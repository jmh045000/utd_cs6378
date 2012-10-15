
#ifndef FAKEMUTEX
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <list>
#include <string>
#include <vector>

using std::list;
using std::string;
using std::vector;

#include "Message.h"
#include "Mutex.h"

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

void *Mutex::inconnector(void *p)
{
    inparams *params = (inparams*)p;
    Socket *s;
    {
        LocalMutex m;
        s = new Socket( params->socket->acceptConnection() );
    }
    return s;
}

void *Mutex::outconnector(void *p)
{
    outparams *params = (outparams*)p;
    return new Socket(params->host, params->port);
}

void *Mutex::listener(void *p)
{
    listenerparams *params = (listenerparams*)p;
    Socket *socket = params->socket;
    Mutex *mutex = params->mutex;

    while(true)
    {
        Message m(socket->read());

        {
            LocalMutex _;
            switch(m.type())
            {
            case HELLO:
                socket->write( Message(HELLO, mutex->processid_) );
                break;
            case REQ:
                {
                    cout << "RECEIVED REQ" << endl;
                    mutex->highestsequence_ = max(mutex->highestsequence_, m.seqno());
                    bool defer = mutex->requestingcs_ && ( (m.seqno() > mutex->sequenceno_) || (m.seqno() == mutex->sequenceno_ && m.processid() > mutex->processid_ ) );
                    if(defer)
                    {
                        cout << "DEFER REPLY" << endl;
                        mutex->deferred_.push_back(mutex->idtosockets_[m.processid()]);
                    }
                    else
                    {
                        cout << "SENDING REPLY" << endl;
                        mutex->idtosockets_[m.processid()]->write( Message(REPLY, mutex->processid_, m.seqno()) );
                    }
                }
                break;
            case REPLY:
                cout << "RECEIVED REPLY" << endl;
                mutex->outstandingreplies_--;
                break;
            case DONE:
                cout << "RECEIVED DONE" << endl;
                mutex->done_.push_back(socket);
                mutex->done_.push_back(mutex->idtosockets_[m.processid()]);
                break;
            default:
                cerr << "UNKNOWN MESSAGE TYPE..." << endl;
            }
        }
    }

    return NULL;
}

void Mutex::initialize(vector<string> &hosts, uint16_t port)
{
    vector<pthread_t*> lthreadids;
    vector<pthread_t*> cthreadids;
    vector<outparams*> cparams;
    inparams p;
    p.socket = &serversocket_;

    cout << __LINE__ << endl;

    for(vector<string>::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        lthreadids.push_back(new pthread_t);
        pthread_create( lthreadids.back(), 0, inconnector, &p );
    }
    cout << __LINE__ << endl;

    for(vector<string>::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        cthreadids.push_back(new pthread_t);
        cparams.push_back(new outparams);
        cparams.back()->host = (*it);
        cparams.back()->port = port;

        pthread_create( cthreadids.back(), 0, outconnector, cparams.back() );
    }
    cout << __LINE__ << endl;

    for(vector<pthread_t*>::iterator it = lthreadids.begin(); it != lthreadids.end(); ++it)
    {
        void *s;
        pthread_join( *(*it), &s );
        insockets_.push_back((Socket*)s);

        delete *it;
    }
    cout << __LINE__ << endl;
    lthreadids.clear();

    for(vector<pthread_t*>::iterator it = cthreadids.begin(); it != cthreadids.end(); ++it)
    {
        void *s;
        pthread_join( *(*it), &s );
        outsockets_.push_back((Socket*)s);

        delete *it;
    }
    cout << __LINE__ << endl;
    cthreadids.clear();


    for(vector<outparams*>::iterator it = cparams.begin(); it != cparams.end(); ++it)
    {
        delete *it;
    }
    cout << __LINE__ << endl;
    cparams.clear();

    vector<listenerparams*> lparams;
    for(vector<Socket*>::iterator it = insockets_.begin(); it != insockets_.end(); ++it)
    {
        lthreadids.push_back(new pthread_t);
        lparams.push_back(new listenerparams);
        lparams.back()->socket = *it;
        lparams.back()->mutex = this;

        pthread_create(lthreadids.back(), 0, listener, lparams.back());
    }
    cout << __LINE__ << endl;

    for(vector<Socket*>::iterator it = outsockets_.begin(); it != outsockets_.end(); ++it)
    {
        (*it)->write( Message(HELLO, processid_) );
        Message m((*it)->read());
        idtosockets_[m.processid()] = (*it);
    }
    cout << __LINE__ << endl;

    sleep(1);

    cout << __LINE__ << endl;
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

        cout << "Sending REQs, seqno=" << sequenceno_ << endl;
        for(vector<Socket*>::iterator it = outsockets_.begin(); it != outsockets_.end(); ++it)
        {
            (*it)->write( Message(REQ, processid_, sequenceno_) );
        }
    }

    while(true) 
    {
        {
            LocalMutex m;
            if (outstandingreplies_ == 0)
                break;
        }
        usleep(50);
    }
}

void Mutex::releaseCS()
{
    LocalMutex m;

    requestingcs_ = false;
    highestsequence_ = max(highestsequence_, sequenceno_);
    
    for(list<Socket*>::iterator it = deferred_.begin(); it != deferred_.end(); it = deferred_.begin())
    {
        cout << "SENDING DEFERRED REPLY, seqno=" << sequenceno_ << endl;
        (*it)->write( Message(REPLY, processid_, sequenceno_) );
        deferred_.erase(it);
    }
}

void Mutex::finish()
{ 
    for(vector<Socket*>::iterator it = outsockets_.begin(); it != outsockets_.end(); ++it)
    {
        (*it)->write( Message(DONE, processid_) );
    }

    while( numhosts_ > 0 ) 
    {
        sleep(1);
        for(list<Socket*>::iterator it = done_.begin(); it != done_.end(); it = done_.begin())
        {
            (*it)->closeSock();
            delete *it;
            done_.erase(it);
        }
    }
}

#endif /*FAKEMUTEX*/
