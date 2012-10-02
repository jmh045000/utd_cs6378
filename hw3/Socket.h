#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdint.h>
#include <iostream>
#include <sstream>
#include <string>

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
    void connectFD(struct sockaddr * psaddr, uint32_t retries=4);   //default to retry 4 times
    int output();
    int input();

public:
    Socket() {}
    Socket(const Socket &copy);
    Socket(std::string host, uint16_t port);
    Socket(int sockFD) : connected(true), sockFD(sockFD) {}
    Socket(struct sockaddr * psaddr, uint32_t retries=4);           //default to retry 4 times
    ~Socket();
    bool isConnected() { return connected; }

    //print if socket is connected
    friend std::ostream & operator<< (std::ostream &ostr, Socket s)
    {
        if(s.connected)
            ostr << "Connected with FD: " << s.sockFD;
        else
            ostr << "Not Connected";
        return ostr;
    }
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

