
#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "Socket.h"

#ifndef FAKEMUTEX

typedef struct
{
    ListenSocket *socket;
} listenerparams;

typedef struct
{
    std::string host;
    uint16_t    port;
} connectorparams;

class Mutex
{
    ListenSocket        serversocket_;    
    std::vector<Socket*> outsockets_;
    std::vector<Socket*> insockets_;

    bool ready_;
    int sequenceno_;

    static void *listener(void *);
    static void *connector(void *);

    void initialize(std::vector<std::string> &hosts, uint16_t port);

public:
    Mutex(std::vector<std::string> &hosts, uint16_t port) : 
        serversocket_(ListenSocket(port)), ready_(false), sequenceno_(0)
    { 
        initialize(hosts, port); 
        if(!ready_) throw std::exception(); 
    }

    void requestCS();
    void releaseCS();

    friend class Receiver;
};

#else /*FAKEMUTEX*/

// This is a fake mutex locker, that just immediately returns.
// It will be used to show that there is contention among the processes.
class Mutex
{
public:
    Mutex(std::vector<std::string> &hosts, uint16_t port) {}
    void requestCS() { return; }
    void releaseCS() { return; }
};

#endif /*FAKEMUTEX*/

#endif /*MUTEX_H*/
