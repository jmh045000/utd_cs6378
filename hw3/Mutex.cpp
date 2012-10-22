
#ifndef FAKEMUTEX
#include <iostream>
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

static pthread_mutex_t mutex_;

class LocalMutex
{
    pthread_mutex_t *m_;
public:
    LocalMutex(pthread_mutex_t *m) : m_(m) { pthread_mutex_lock(m_); }
    ~LocalMutex() { pthread_mutex_unlock(m_); }
};

void *Mutex::listener(void *p)
{
    listenerparams *params = (listenerparams*)p;
    Socket *socket = params->socket;
    Mutex *mutex = params->mutex;

    while(true)
    {
        string str = "";

        try
        {
            while(str == "")
            {
                {
                    LocalMutex _(&mutex_);  
                    //cerr << "Reading from socket..." << endl;
                    str = socket->read();
                }
                usleep(1);
            }
        } catch(ClosedException e) {
            return NULL;
        }

        Message m(str);

        {
            LocalMutex _(&mutex_);
            switch(m.type())
            {
            case REQ:
                {
                    cerr << pthread_self() << ": " << "RECEIVED REQ" << endl;
                    mutex->highestsequence_ = max(mutex->highestsequence_, m.seqno());
                    bool defer = mutex->requestingcs_ && ( (m.seqno() > mutex->sequenceno_) || (m.seqno() == mutex->sequenceno_ && m.processid() > mutex->processid_ ) );
                    if(defer)
                    {
                        cerr << "DEFER REPLY" << endl;
                        mutex->deferred_.push_back(socket);
                    }
                    else
                    {
                        cerr << "SENDING REPLY" << endl;
                        mutex->outgoing_.push_back( OutgoingMessage(socket, Message(REPLY, mutex->processid_, m.seqno())) );
                        pthread_cond_signal(&mutex->cond_);
                    }
                }
                break;
            case REPLY:
                cerr << pthread_self() << ": " << "RECEIVED REPLY" << endl;
                mutex->outstandingreplies_--;
                break;
            case DONE:
                cerr << pthread_self() << ": " << "RECEIVED DONE" << endl;
                mutex->done_.push_back(socket);
                break;
            default:
                cerr << "UNKNOWN MESSAGE TYPE..." << endl;
            }
        }
    }

    return NULL;
}

void *Mutex::sender(void *p)
{
    vector<OutgoingMessage> *outgoing = ((senderparams*)p)->outgoing;
    Mutex *mutex = ((senderparams*)p)->mutex;

    while(mutex->ready_)
    {
        pthread_mutex_lock(&mutex_);
        pthread_cond_wait(&mutex->cond_, &mutex_);
        for(vector<OutgoingMessage>::iterator it = outgoing->begin(); it != outgoing->end(); ++it)
        {
            it->first->write(it->second);
        }
        outgoing->clear();
        pthread_mutex_unlock(&mutex_);
    }

    return NULL;
}

void Mutex::initialize(vector<host> &hosts, uint16_t port)
{
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&cond_, NULL);

    vector<pthread_t*> lthreadids;
    pthread_t senderid;
    senderparams *sparams = new senderparams;

    for(vector<host>::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        if(it->port < port)
        {
            sockets_.push_back(new Socket(it->hostname, it->port));
            cerr << processid_ << ": Connected to " << it->hostname << ":" << it->port << endl;
        }
    }

    while(sockets_.size() < numhosts_)
    {
        sockets_.push_back(new Socket(serversocket_.acceptConnection()));
    }

    vector<listenerparams*> lparams;
    for(vector<Socket*>::iterator it = sockets_.begin(); it != sockets_.end(); ++it)
    {
        lthreadids.push_back(new pthread_t);
        lparams.push_back(new listenerparams);
        lparams.back()->socket = *it;
        lparams.back()->mutex = this;

        pthread_create(lthreadids.back(), 0, listener, lparams.back());
    }

    cerr << processid_ << ": is connected to " << sockets_.size() << " hosts" << endl;

    sleep(1);

    if( sockets_.size() == numhosts_ )
    {
        ready_ = true;
    }

    sparams->outgoing = &outgoing_;
    sparams->mutex = this;
    pthread_create(&senderid, NULL, sender, sparams);
}

void Mutex::requestCS()
{
    usleep(1000000);
    {
        LocalMutex m(&mutex_);
        requestingcs_ = true;
        sequenceno_ = highestsequence_ + 1;
        outstandingreplies_ = numhosts_;

        cerr << "Sending REQs, seqno=" << sequenceno_ << endl;
        for(vector<Socket*>::iterator it = sockets_.begin(); it != sockets_.end(); ++it)
        {
            outgoing_.push_back( OutgoingMessage((*it), Message(REQ, processid_, sequenceno_)) );
        }
        pthread_cond_signal(&cond_);
    }

    cerr << "Waiting for " << outstandingreplies_ << " replies" << endl;

    while(true) 
    {
        {
            LocalMutex m(&mutex_);
            if (outstandingreplies_ == 0)
                break;
        }
    }

    cerr << "Received all replies, entering CS" << endl;
}

void Mutex::releaseCS()
{
    LocalMutex m(&mutex_);

    requestingcs_ = false;
    highestsequence_ = max(highestsequence_, sequenceno_);
    
    for(list<Socket*>::iterator it = deferred_.begin(); it != deferred_.end(); ++it)
    {
        outgoing_.push_back( OutgoingMessage((*it), Message(REPLY, processid_, sequenceno_)) );
    }
    pthread_cond_signal(&cond_);
    deferred_.clear();
}

void Mutex::finish()
{ 
    cerr << processid_ << ": Is done, sending DONE message" << endl;
    for(vector<Socket*>::iterator it = sockets_.begin(); it != sockets_.end(); ++it)
    {
        LocalMutex _(&mutex_);
        outgoing_.push_back( OutgoingMessage((*it), Message(DONE, processid_)) );
    }
    pthread_cond_signal(&cond_);

    while( numhosts_ > 0 ) 
    {
        sleep(1);
        {
            LocalMutex _(&mutex_);
            for(list<Socket*>::iterator it = done_.begin(); it != done_.end(); it = done_.begin())
            {
                numhosts_--;
                (*it)->closeSock();
                delete *it;
                done_.erase(it);
            }
        }
    }

    ready_ = false;
}

#endif /*FAKEMUTEX*/
