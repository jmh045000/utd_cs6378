#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdint.h>
#include <iostream>
#include <map> // For std::pair
#include <sstream>
#include <string>
#include <vector>

//simple wrapper around BSD tcp sockets
//  basic idea is to write the code to deal with opening/closing sockets once
class Socket
{
protected:
    // Vars:
    std::stringstream myBuf;

    bool connected;
    int sockFD;
    
    //Functions:
    void createFD(void);
    void connectFD(struct sockaddr * psaddr, uint32_t retries=50);   //default to retry 50 times
    int output();
    int input();

public:
    Socket() {}
    Socket(const Socket &copy);
    Socket(std::string host, uint16_t port);
    Socket(int sockFD) : connected(true), sockFD(sockFD) {}
    ~Socket();
    bool isConnected() { return connected; }

    void write(std::string data) { myBuf << data; output(); myBuf.str(""); }
    std::string read() {std::cout << "calling input()" << std::endl; input(); std::string str = myBuf.str(); myBuf.str(""); return str;}

    void closeSock();

    Socket  operator=(Socket &rhs) {this->connected = rhs.connected; this->sockFD = rhs.sockFD;}
    bool    operator==(Socket &rhs) { return this->sockFD == rhs.sockFD; }

    //print if socket is connected
    friend std::ostream & operator<< (std::ostream &ostr, Socket &s)
    {
        ostr << "<Socket: " << s.sockFD << ">";
        return ostr;
    }

    friend class SocketArray;
};

typedef std::pair<Socket*, std::string> SocketReadData;
class SocketArray
{
    typedef std::vector<Socket>             SocketVector;
    SocketVector sockets_;
public:
    SocketArray() {}
    SocketArray(SocketVector sockets) : sockets_(sockets) {}
    ~SocketArray();

    SocketReadData  read();
    void            write(std::string msg);
    void            addSocket(Socket &s) { sockets_.push_back( Socket(s) ); }
    void            removeSocket(Socket &s);

    size_t          size() { return sockets_.size(); }
};

//simple listen socket
class ListenSocket : public Socket
{
public:
    ~ListenSocket();
    ListenSocket(uint16_t port);
    Socket acceptConnection();
};

#endif /* __SOCKET_H__ */

