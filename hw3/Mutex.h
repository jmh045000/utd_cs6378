
#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <stdint.h>

#include <list>
#include <map>
#include <string>
#include <vector>

#include "Socket.h"

typedef struct
{
    std::string hostname;
    int         port;
} host;

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
    std::map<int, Socket*>  idtosockets_;

    int     processid_;
    int     sequenceno_;
    bool    ready_;

    size_t              numhosts_;
    volatile bool       requestingcs_;
    volatile int        highestsequence_;
    volatile size_t     outstandingreplies_;
    std::list<Socket*>  deferred_;
    std::list<Socket*>  done_;

    static void *inconnector(void *);
    static void *outconnector(void *);
    static void *listener(void *);

    void initialize(std::vector<host> &hosts, uint16_t port);

public:
    Mutex(int processid, std::vector<host> &hosts, uint16_t port) : 
        serversocket_(ListenSocket(port)), processid_(processid), sequenceno_(0), ready_(false),
        numhosts_(hosts.size()), requestingcs_(false), highestsequence_(0), outstandingreplies_(false)
    { 
        initialize(hosts, port); 
        if(!ready_) throw std::exception(); 
    }

    void requestCS();
    void releaseCS();

    void finish();

    friend class Receiver;
};

#else /*FAKEMUTEX*/

// This is a fake mutex locker, that just immediately returns.
// It will be used to show that there is contention among the processes.
class Mutex
{
public:
    Mutex(int processid, std::vector<host> &hosts, uint16_t port) {}
    void requestCS()    { return; }
    void releaseCS()    { return; }
    void finish()       { return; }
};

#endif /*FAKEMUTEX*/

#endif /*MUTEX_H*/
