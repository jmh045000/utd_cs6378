
#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "Socket.h"

typedef struct
{
    size_t                  numhosts;
    ListenSocket           *serversocket;
    std::vector<Socket*>    *sockets;
} listenerparams;

typedef struct
{
    std::string host;
    uint16_t    port;
    std::vector<Socket*> *sockets;
} connectorparams;

class Mutex
{
    ListenSocket        serversocket_;    
    std::vector<Socket*> outsockets_;
    std::vector<Socket*> insockets_;

    bool ready_;

    void initialize(std::vector<std::string> &hosts, uint16_t port);

public:
    Mutex(std::vector<std::string> &hosts, uint16_t port) : serversocket_(ListenSocket(port)) 
    { 
        initialize(hosts, port); 
        if(!ready_) throw std::exception(); 
    }

    void requestCS();
    void releaseCS();

};

// This is a fake mutex locker, that just immediately returns.
// It will be used to show that there is contention among the processes.
class FakeMutex
{
public:
    void requestCS() { return; }
    void releaseCS() { return; }
};

#endif /*MUTEX_H*/
