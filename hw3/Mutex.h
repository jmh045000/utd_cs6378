
#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "Socket.h"

#ifndef FAKEMUTEX

class Mutex;

typedef struct
{
    ListenSocket *socket;
} inparams;

typedef struct
{
    std::string host;
    uint16_t    port;
} outparams;

typedef struct
{
    Socket *socket;
    Mutex *mutex;
} listenerparams;

class Mutex
{
    ListenSocket            serversocket_;    
    std::vector<Socket*>    outsockets_;
    std::vector<Socket*>    insockets_;

    int     processid_;
    int     sequenceno_;
    bool    ready_;

    static void *inconnector(void *);
    static void *outconnector(void *);
    static void *listener(void *);

    void initialize(std::vector<std::string> &hosts, uint16_t port);

public:
    Mutex(int processid, std::vector<std::string> &hosts, uint16_t port) : 
        serversocket_(ListenSocket(port)), processid_(processid), sequenceno_(0), ready_(false)
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
