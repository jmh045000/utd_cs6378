
#ifndef _MUTEX_H
#define _MUTEX_H

#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

class host
{
public:
    std::string hostname;
    uint16_t port;
};

#ifndef FAKEMUTEX

#include "Message.h"

class Mutex
{
private:
    typedef std::pair<int, Message> DeferredMessage;   

    //Internal functions
    void initialize(std::vector<host> hosts, uint16_t port);
    static void *listener(void *param);
    
    //Special variables
    int serversocket_;
    std::vector<int> sockets_;
    std::vector<int> done_;
    std::vector<DeferredMessage> deferred_;
    pthread_t listenerid_;

    //Variables for control of mutex
    int processid_;
    int numhosts_;
    
    //Variables for Ricart & Agrawala's algo
    bool requestingcs_;
    int outstandingreplies_;
    int sequenceno_;
    int highestsequenceno_;

public:

    Mutex(int processid, std::vector<host> hosts, uint16_t port);
    void requestCS();
    void releaseCS();
    void finish();
};

#else

class Mutex
{
public:
    Mutex(int, std::vector<host>, uint16_t) {}
    void requestCS()    {}
    void releaseCS()    {}
    void finish()       {}
};

#endif /*FAKEMUTEX*/

#endif /*_MUTEX_H*/
