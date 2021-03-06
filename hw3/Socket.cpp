
#include <exception>
#include <iostream>
#include <sstream>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
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
#   include <netinet/tcp.h>


#   include <errno.h>
#   include <string.h>
#   include <stdio.h>
}

#include "Socket.h"

#define SOCKET_DEBUG

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

//constructor with struct sockaddr
Socket::Socket(struct sockaddr *saddr, uint32_t retries) : connected(false), sockFD(-1) {
    connectFD(saddr, retries);
}

Socket::~Socket()
{
    //cerr << "~Socket: " << sockFD << endl;
}

void Socket::closeSock()
{
    if(connected) {
        ::close(sockFD);
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
/*
            sockaddr_in *sin = (sockaddr_in *) saddr;
            printf("%d %x %s\n", ntohs( sin -> sin_port),
                    sin->sin_family,
                    inet_ntoa(sin->sin_addr)
                    );
*/
            perror("connect()");
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

    int flag = 1;
    int rc = setsockopt(sockFD, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
    if(rc != 0)
    {
        perror("setsockopt(TCP_NODELAY)");
    }
}

//send current contents of myBuf to peer
int Socket::output(string data)
{
    //don't forget to add 1 for \0
    int length = data.length()+1;
#ifdef SOCKET_DEBUG
    cerr << "Sending: '" << data << "' on socket (" << sockFD << ")" << endl;
#endif
    const char *buf = data.c_str();
    int retVal = send(sockFD, buf, length , 0);

    if(retVal == length) {
        cerr << "Finished sending '" << data << "' on socket (" << sockFD << ")" << endl;
    }
    else if (retVal < 0) {
        std::stringstream ss;
        ss << "error sending: " << errno;
        perror("send()");
        cerr << __LINE__ << endl; throw exception();
    }
    else {
        cerr << "Odd send...  expected: " << length  << " sent: " << retVal << endl;
    }

    return retVal;
}

//receive from peer into myBuf
string Socket::input()
{
    char buffer[4096];
    memset(buffer, 0, 4096);

    if (!connected)
    {
        //cerr << __FILE__"," << __LINE__ << endl;
        cerr << __LINE__ << endl; throw exception();
    }

        struct timeval tv;
        memset(&tv, 0, sizeof(struct timeval));
        tv.tv_sec = 0;
        tv.tv_usec = 50;

        setsockopt(sockFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

    int retVal;
        retVal = recv(sockFD, buffer, 4096, 0);

    if(retVal > 0) {
#ifdef SOCKET_DEBUG
        cerr << "Received: " << buffer << endl;
#endif
        return string(buffer);
    }
    else if(retVal < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return "";
        //cout << strerror(errno) << endl;
        cerr << errno << ":" << __LINE__ << endl; throw exception();
    }
    if (retVal == 0) {
        connected=false;
        close(sockFD);
        sockFD=-1;
        throw ClosedException();
    }


    return string("");
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
    cout << "~ListenSocket()" << endl;
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

    int flag = 1;
    int rc = setsockopt(newFD, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
    if(rc != 0)
    {
        perror("setsockopt(TCP_NODELAY)");
    }

    Socket s(newFD);
    return s;
}

