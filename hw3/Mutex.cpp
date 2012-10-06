
#ifndef FAKEMUTEX
#include <iostream>
using std::cout;
using std::endl;

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "Message.h"
#include "Mutex.h"

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

    Message m(socket->read());

    cout << "Received message: " << m << endl;

    return NULL;
}

void Mutex::initialize(vector<string> &hosts, uint16_t port)
{
    vector<pthread_t*> lthreadids;
    vector<pthread_t*> cthreadids;
    vector<outparams*> cparams;
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
        insockets_.push_back((Socket*)s);
        cout << *(insockets_.back()) << endl;

        delete *it;
    }
    lthreadids.clear();

    for(vector<pthread_t*>::iterator it = cthreadids.begin(); it != cthreadids.end(); ++it)
    {
        void *s;
        pthread_join( *(*it), &s );
        outsockets_.push_back((Socket*)s);
        cout << *(outsockets_.back()) << endl;

        delete *it;
    }
    cthreadids.clear();


    for(vector<outparams*>::iterator it = cparams.begin(); it != cparams.end(); ++it)
    {
        delete *it;
    }
    cparams.clear();

    for(vector<Socket*>::iterator it = outsockets_.begin(); it != outsockets_.end(); ++it)
    {
        (*it)->write( Message(HELLO, processid_) );
    }

    if( outsockets_.size() == insockets_.size() && insockets_.size() == hosts.size() )
    {
        ready_ = true;
    }
}

void requestCS()
{

}

void releaseCS()
{

}

#endif /*FAKEMUTEX*/
