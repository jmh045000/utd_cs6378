
#ifndef FAKEMUTEX

#include <list>
#include <string>
#include <vector>

using std::list;
using std::pair;
using std::string;
using std::vector;

#include "Message.h"
#include "Mutex.h"

#ifdef DEBUG
#define MUTEX_DEBUG
#endif

#ifdef MUTEX_DEBUG
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#endif

template <typename T>
T max(T left, T right)
{
    if(left > right)    return left;
    else                return right;
}

pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

class LocalMutex
{
public:
    LocalMutex() { pthread_mutex_lock(&mutex_); }
    ~LocalMutex() { pthread_mutex_unlock(&mutex_); }
};

Mutex::~Mutex()
{
    pthread_join(listenerid_, NULL);
    serversocket_.closeSock();
}

void *Mutex::inconnector(void *p)
{
    inparams *params = (inparams*)p;
    return new Socket(params->socket->acceptConnection());
}

void *Mutex::outconnector(void *p)
{
    outparams *params = (outparams*)p;
    return new Socket(params->host, params->port);
}

void *Mutex::listener(void *p)
{
    listenerparams *params = (listenerparams*)p;
    Mutex *mutex = params->mutex;

    while(true)
    {
#ifdef MUTEX_DEBUG
        cout << "Calling read()" << endl;
#endif
        SocketReadData data = mutex->insockets_.read();
        if(data.second == "")
        {
            mutex->insockets_.removeSocket(*data.first);
            return NULL;
        }
        Socket *socket = data.first;
        Message m(data.second);


#ifdef MUTEX_DEBUG
        cout << "Received message of type: " << m.type() << endl;
#endif

        {
            LocalMutex _;
            switch(m.type())
            {
            case HELLO:
                socket->write( Message(HELLO, mutex->processid_) );
#ifdef MUTEX_DEBUG
                cout << "Received HELLO, sending response to " << m.processid() << endl;
#endif
                break;
            case REQ:
                {
                    mutex->highestsequence_ = max(mutex->highestsequence_, m.seqno());
                    bool defer = mutex->requestingcs_ && ( (m.seqno() > mutex->sequenceno_) || (m.seqno() == mutex->sequenceno_ && m.processid() > mutex->processid_ ) );
                    if(defer)
                    {
#ifdef MUTEX_DEBUG
                        cout << "Deferring the reply to " << m.processid() << endl;
#endif
                        mutex->deferred_.push_back(DeferredMessage(mutex->idtosockets_[m.processid()], Message(REPLY, m.processid(), m.seqno()) ) );
                    }
                    else
                    {
#ifdef MUTEX_DEBUG
                        cout << "Sending the reply to " << m.processid() << endl;
#endif
                        mutex->idtosockets_[m.processid()]->write( Message(REPLY, m.processid(), m.seqno()) );
                    }
                }
                break;
            case REPLY:
                mutex->outstandingreplies_--;
                break;
            case DONE:
                mutex->done_.push_back(socket);
                mutex->done_.push_back(mutex->idtosockets_[m.processid()]);
                mutex->numhosts_--;
                break;
            default:
#ifdef MUTEX_DEBUG
                cerr << "UNKNOWN MESSAGE TYPE..." << endl;
#endif
                break;
            }
        }
    }

    return NULL;
}

void Mutex::initialize(vector<string> &hosts, uint16_t port)
{
    vector<pthread_t*> cthreadids;
    vector<pthread_t*>      lthreadids;
    vector<outparams*> cparams;
    vector<listenerparams*> lparams;
    inparams p;
    p.socket = &serversocket_;

    for(vector<string>::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        lthreadids.push_back(new pthread_t);
        pthread_create( lthreadids.back(), 0, inconnector, &p );
    }

    for(vector<string>::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        cthreadids.push_back(new pthread_t);
        cparams.push_back(new outparams);
        cparams.back()->host = (*it);
        cparams.back()->port = port;

        pthread_create( cthreadids.back(), 0, outconnector, cparams.back() );
    }

    for(vector<pthread_t*>::iterator it = lthreadids.begin(); it != lthreadids.end(); ++it)
    {
        void *s;
        pthread_join( *(*it), &s );
        insockets_.addSocket( *((Socket*)s) );

        delete *it;
    }
    lthreadids.clear();

    for(vector<pthread_t*>::iterator it = cthreadids.begin(); it != cthreadids.end(); ++it)
    {
        void *s;
        pthread_join( *(*it), &s );
        outsockets_.addSocket( *((Socket*)s) );

        delete *it;
    }
    cthreadids.clear();


    for(vector<outparams*>::iterator it = cparams.begin(); it != cparams.end(); ++it)
    {
        delete *it;
    }
    cparams.clear();

    listenerparams_.mutex = this;
    pthread_create(&listenerid_, 0, listener, &listenerparams_);

    outsockets_.write( Message(HELLO, processid_) );

    sleep(1);

    for(int i = 0; i < numhosts_; i++)
    {
        SocketReadData data = outsockets_.read();
        Message m(data.second);
        idtosockets_[m.processid()] = data.first;
#ifdef MUTEX_DEBUG
        cout << "Processid " << m.processid() << " is on socket " << *data.first << endl;
#endif
    }


    if( outsockets_.size() == insockets_.size() && insockets_.size() == hosts.size() )
    {
        ready_ = true;
    }
}

void Mutex::requestCS()
{
    {
        LocalMutex m;
        requestingcs_ = true;
        sequenceno_ = highestsequence_ + 1;
        outstandingreplies_ = numhosts_;

        outsockets_.write( Message(REQ, processid_, sequenceno_) );
    }

#ifdef MUTEX_DEBUG
    cout << "Waiting for all " << outstandingreplies_ << " replies" << endl;
#endif

    while(true) 
    {
        {
            LocalMutex m;
            if (outstandingreplies_ == 0)
                break;
        }
    }
#ifdef MUTEX_DEBUG
    cout << "Granting access to CS" << endl;
#endif
}

void Mutex::releaseCS()
{
    LocalMutex m;

    requestingcs_ = false;
    highestsequence_ = max(highestsequence_, sequenceno_);
    
    for(list<DeferredMessage>::iterator it = deferred_.begin(); it != deferred_.end(); it = deferred_.begin())
    {
        it->first->write( it->second );
        deferred_.erase(it);
    }
}

void Mutex::finish()
{ 
    outsockets_.write( Message(DONE, processid_) );

    while( numhosts_ > 0 ) 
    {
        for(list<Socket*>::iterator it = done_.begin(); it != done_.end(); it = done_.begin())
        {
            (*it)->closeSock();
            done_.erase(it);
        }
    }
}

#endif /*FAKEMUTEX*/
