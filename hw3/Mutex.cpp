
#include <string>
#include <vector>

using std::string;
using std::vector;

#include "Mutex.h"

pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

class LocalMutex
{
public:
    LocalMutex() { pthread_mutex_lock(&mutex_); }
    ~LocalMutex() { pthread_mutex_unlock(&mutex_); }
};

void *listener(void *p)
{
    listenerparams *params = (listenerparams*)p;
    while ( params->sockets->size() !=  params->numhosts)
    {
        Socket s( params->serversocket->acceptConnection() );
        {
            LocalMutex m;
            params->sockets->push_back( new Socket(s) );
        }
    }

    return NULL;
}

void *connector(void *p)
{
    connectorparams *params = (connectorparams*)p;

    Socket s(params->host, params->port);
    {
        LocalMutex m;
        params->sockets->push_back( new Socket(s) );
    }

    return NULL;
}

void Mutex::initialize(vector<string> &hosts, uint16_t port)
{
    vector<pthread_t*> threadids;
    vector<connectorparams*> cparams;
    threadids.push_back(new pthread_t);

    listenerparams lparam;
    lparam.numhosts = hosts.size();
    lparam.serversocket = &serversocket_;
    lparam.sockets = &insockets_;
    pthread_create(threadids.back(), 0, listener, &lparam);

    for(vector<string>::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        threadids.push_back(new pthread_t);
        cparams.push_back(new connectorparams);
        cparams.back()->host = (*it);
        cparams.back()->port = port;
        cparams.back()->sockets = &outsockets_;

        pthread_create( threadids.back(), 0, connector, cparams.back() );
    }

    for(vector<pthread_t*>::iterator it = threadids.begin(); it != threadids.end(); ++it)
    {
        pthread_join( *(*it), NULL );
    }

    if( outsockets_.size() == insockets_.size() && insockets_.size() == hosts.size() )
    {
        ready_ = true;
    }
}
