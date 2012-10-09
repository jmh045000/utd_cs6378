
#include <exception>
#include <iostream>
#include <sstream>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::exception;
using std::string;
using std::vector;

extern "C"
{
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netdb.h>
#   include <arpa/inet.h>


#   include <errno.h>
#   include <string.h>
#   include <stdio.h>
#   include <sys/select.h>
#   include <time.h>
}

#include "Socket.h"

#ifdef DEBUG
#define SOCKET_DEBUG
#endif

//copy constructor
Socket::Socket(const Socket &copy)
{
    this->connected=copy.connected;
    this->sockFD=copy.sockFD;
}

//constructor from host:port
Socket::Socket(string host, uint16_t port) :
    connected(false), sockFD(-1)
{
    if(host != "")
    {
        struct hostent      *h_server;
        union {
        struct sockaddr_in  saddr_in;
        struct sockaddr     saddr; };
        if( (h_server = gethostbyname(host.c_str())) == NULL) {
            cerr << __LINE__ << endl; throw exception();
        }

        saddr_in.sin_family = AF_INET;
        saddr_in.sin_port   = htons(port);
        saddr_in.sin_addr   = *((struct in_addr *) h_server->h_addr);

        connectFD(&saddr);;
    }
}

Socket::~Socket()
{
#ifdef SOCKET_DEBUG
    cerr << "~Socket: " << *this << endl;
#endif
}

void Socket::closeSock()
{
    if(connected) {
        close(sockFD);
        sockFD=-1;
    }
}

//open the socket
//  throw on failure
void Socket::createFD(void)
{
    if(sockFD != -1)
    {
        cerr << __LINE__ << endl; throw exception();
    }
    if( (sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        cerr << __LINE__ << endl; throw exception();
    }
}

//connect the socket
//  cerr << __LINE__ << endl; thrown on failure
void Socket::connectFD(struct sockaddr * saddr, uint32_t retries) //default to retry 4 times
{
    if(connected)
    {
        cerr << __LINE__ << endl; throw exception();
    }
    if(sockFD == -1) {
        createFD();
    }

    //retry <retries> times...
    for(uint32_t i=0; i <= retries; i++) {
        if( connect( sockFD, saddr, sizeof(sockaddr)) < 0) {
#ifdef SOCKET_DEBUG
            perror("connect()");
#endif
        }
        else {
            connected = true;
            break;
        }
        if(retries>0)   //only sleep if we're going around again
            sleep(1);
    }
    if (!connected)
    {
        cerr << __LINE__ << endl; throw exception();
    }
}

//send current contents of myBuf to peer
int Socket::output()
{
    //don't forget to add 1 for \0
    int length = myBuf.str().length()+1;
#ifdef SOCKET_DEBUG
    cerr << "Sending: '" << myBuf.str() << "'" << endl;
#endif
    int retVal = send(sockFD, myBuf.str().c_str(), length , 0);
#ifdef SOCKET_DEBUG
    cerr << "Sent " << retVal << " bytes" << endl;
#endif

    if (retVal < 0) {
        std::stringstream ss;
        ss << "error sending: " << errno;
        perror("send()");
        cerr << __LINE__ << endl; throw exception();
    }
    else if (retVal != length) {
        cerr << "Odd send...  expected: " << length  << " sent: " << retVal << endl;
    }

    return retVal;
}

//receive from peer into myBuf
int Socket::input()
{
    char buffer[4096];


#ifdef SOCKET_DEBUG
    cout << "IN input()" << endl;
#endif
    if (!connected)
    {
        return 0;
    }

    int retVal;

RECVAGAIN:
#ifdef SOCKET_DEBUG
    cout << "calling recv()" << endl;
#endif
    retVal = recv(sockFD, buffer, 4096, 0);
#ifdef SOCKET_DEBUG
    cout << "done with recv()" << endl;
#endif

    if(retVal > 0) {
#ifdef SOCKET_DEBUG
        cout << "Adding received to myBuf" << endl;
#endif
        myBuf.str(buffer);
    }
    else if(retVal < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            cout << "input() TIMEDOUT" << endl;
            goto RECVAGAIN;
        }
        cout << strerror(errno) << endl;
        cerr << errno << ":" << __LINE__ << endl; throw exception();
    }
    else {
        connected=false;
        close(sockFD);
        sockFD=-1;
    }

#ifdef SOCKET_DEBUG
    cerr << "Received: " << buffer << endl;
#endif

    return retVal;
}

SocketArray::~SocketArray()
{
    for(SocketVector::iterator it = sockets_.begin(); it != sockets_.end(); ++it)
        it->closeSock();
    sockets_.clear();
}

SocketReadData SocketArray::read()
{
    fd_set          fds;
    char            buffer[4096];
    int             max = 0;
    SocketReadData  ret(NULL, "");

    FD_ZERO(&fds);
    for(SocketVector::iterator it = sockets_.begin(); it != sockets_.end(); ++it)
    {
        if(it->sockFD > max) max = it->sockFD;
        FD_SET(it->sockFD, &fds);
#ifdef SOCKET_DEBUG
        cerr << "Adding " << *it << " to fdset" << endl;
#endif
    }

    int rc = select(max+1, &fds, NULL, NULL, 0);
    if (rc < 0)
    {
#ifdef SOCKET_DEBUG
        perror(" select()");
#endif
        return SocketReadData(NULL, "");
    }

    for(int i = 0; i < max; i++)
    {
        if (FD_ISSET(i, &fds))
        {
            for(SocketVector::iterator it = sockets_.begin(); it != sockets_.end(); ++it)
            {
                if(it->sockFD == i) 
                {
                    ret.first = &(*it);
                    ret.second = it->read();
                    break;
                }
            }
        }
        if(ret.first != NULL) return ret;
    }
}

void SocketArray::write(string msg)
{
    for(SocketVector::iterator it = sockets_.begin(); it != sockets_.end(); ++it)
    {
        it->write(msg);
    }
}

void SocketArray::removeSocket(Socket& s)
{
    for(SocketVector::iterator it = sockets_.begin(); it != sockets_.end(); ++it)
    {
        if(*it == s)
        {
            it->closeSock();
            sockets_.erase(it);
        }
    }
}

//open a listen socket on the specified port
ListenSocket::ListenSocket(uint16_t port)
{
    struct sockaddr_in  servaddr;
    servaddr.sin_family         = AF_INET;
    servaddr.sin_port           = htons(port);
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);

    if( (sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << __LINE__ << endl; throw exception();
    }

    int       reuse=1;
    /* set SO_REUSEADDR so echoserv will be able to be re-run instantly on shutdown */
    if(setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        close(sockFD);
        cerr << __LINE__ << endl; throw exception();
    }

    if( bind( sockFD, (const sockaddr *)&servaddr, sizeof(servaddr) ) ) {
        close(sockFD);
        cerr << __LINE__ << endl; throw exception();
    }
    if( listen(sockFD, 20) < 0 )
    {
        close(sockFD);
        cerr << __LINE__ << endl; throw exception();
    }
}

ListenSocket::~ListenSocket()
{
    //cout << "~ListenSocket()" << endl;
    if(connected)
    {
        close(sockFD);
        connected=false;
    }
}

//accept an income TCP connection
//  blocking
Socket ListenSocket::acceptConnection()
{
    int newFD;
    struct sockaddr addr;
    socklen_t len = 0;
    newFD = accept(sockFD, &addr, &len);
    if(newFD < 0)
        perror("accept()");
    Socket s(newFD);
    return s;
}

