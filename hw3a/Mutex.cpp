
#ifndef FAKEMUTEX
#include <iostream>

extern "C"
{
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netdb.h>
#   include <arpa/inet.h>
#   include <netinet/tcp.h>
#   include <sys/select.h>

#   include <errno.h>
#   include <string.h>
#   include <stdio.h>
#   include <pthread.h>
}

#include "Mutex.h"
#include "Message.h"

using namespace std;

//Utility functions:

template <typename T>
T max(T &left, T &right)
{
    if(left > right)    return left;
    else                return right;
}

int createserversocket(uint16_t port)
{
    struct sockaddr_in  servaddr;
    servaddr.sin_family         = AF_INET;
    servaddr.sin_port           = htons(port);
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);

    int servfd;

    if( (servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
    }

    int       reuse=1;
    if(setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        close(servfd);
        perror("setsockopt()");
    }

    if( bind( servfd, (const sockaddr *)&servaddr, sizeof(servaddr) ) ) {
        close(servfd);
        perror("bind()");
    }
    if( listen(servfd, 20) < 0 )
    {
        close(servfd);
        perror("listen()");
    }

    return servfd;
}

int acceptconnection(int servfd)
{
    struct sockaddr addr;
    socklen_t len = 0;
    int fd = accept(servfd, &addr, &len);
    if(fd < 0)
        perror("accept()");

    int flag = 1;
    int rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
    if(rc != 0)
    {
        perror("setsockopt(TCP_NODELAY)");
    }
    return fd;
}

int connect(string host, uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket()");
    }

    if(host != "")
    {
        struct hostent      *h_server;
        union {
        struct sockaddr_in  saddr_in;
        struct sockaddr     saddr; };
        if( (h_server = gethostbyname(host.c_str())) == NULL) {
            perror("gethostbyname()");
        }

        saddr_in.sin_family = AF_INET;
        saddr_in.sin_port   = htons(port);
        saddr_in.sin_addr   = *((struct in_addr *) h_server->h_addr);

        for(int i = 50; i > 0; i--)
        {
            if(connect(fd, &saddr, sizeof(sockaddr)) < 0)
            {
                perror("connect()");
            }
            else
                break;

            if(i > 0) sleep(1);  //Some time for the other host to come up
        }
    }

    int flag = 1;
    int rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
    if(rc != 0)
    {
        perror("setsockopt(TCP_NODELAY)");
    }

    return fd;
}

static pthread_mutex_t pmutex_;

//Private functions:

void *Mutex::listener(void *param)
{
    Mutex *mutex = static_cast<Mutex*>(param);
    fd_set fds;
    int maxfd = -1;
    int numfds = mutex->sockets_.size();


    while(numfds > 0)
    {
        usleep(1);
        FD_ZERO(&fds);
        
        pthread_mutex_lock(&pmutex_);
        for(vector<int>::iterator it = mutex->sockets_.begin(); it != mutex->sockets_.end(); ++it)
        {
            FD_SET(*it, &fds);
            maxfd = max(*it, maxfd);
        }
        pthread_mutex_unlock(&pmutex_);
        maxfd++;

        int rc = select(maxfd, &fds, NULL, NULL, 0);
        if(rc < 0)
        {
            perror("select()");
            continue;
        }

        cerr << "Done with select(), rc=" << rc << endl;

        for(int i = 0; i < maxfd; i++)
        {
            if(FD_ISSET(i, &fds))
            {
                pthread_mutex_lock(&pmutex_);
                cerr << "fd=" << i << " HAS DATA" << endl;
                Message m;
                memset(&m, 0, sizeof(Message));

                    int rc1 = recv(i, &m, sizeof(Message), 0);
                    if(rc1 < 0)
                    {
                        perror("recv()");
                        pthread_mutex_unlock(&pmutex_);
                        continue;
                    }
                    if(rc1 == 0)
                    {
                        FD_CLR(i, &fds);
                        close(i);
                    }


                cerr << "Received message=" << m.type << " " << m.processid << " " << m.seqno << " from fd=" << i << endl;

                switch(m.type)
                {
                case REQ:
                    {
                        mutex->highestsequenceno_ = max(mutex->highestsequenceno_, m.seqno);
                        bool defer = mutex->requestingcs_ && ( (m.seqno > mutex->sequenceno_) || (m.seqno == mutex->sequenceno_ && m.processid > mutex->processid_ ) );
                        if(defer)
                        {
                            cerr << "DEFER" << endl;
                            Message dmsg = { REPLY, mutex->processid_, m.seqno };
                            mutex->deferred_.push_back( DeferredMessage(i, dmsg) );
                        }
                        else
                        {
                            cerr << "REPLY" << endl;
                            Message msg = { REPLY, mutex->processid_, m.seqno };
                            int rc1 = send(i, &msg, sizeof(Message), 0);
                            if(rc1 < 0)
                            {
                                perror("send()");
                            }
                        }
                    }
                    break;
                case REPLY:
                    mutex->outstandingreplies_--;
                    break;
                case DONE:
                    mutex->done_.push_back(i);
                    break;
                default:
                    cerr << "what..." << endl;
                }
            }
            pthread_mutex_unlock(&pmutex_);
        }
    }

    return NULL;
}

void Mutex::initialize(vector<host> hosts, uint16_t port)
{
    pthread_mutex_init(&pmutex_, NULL);

    for(vector<host>::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        if(it->port < port)
        {
            sockets_.push_back(connect(it->hostname, it->port));
        }
    }

    serversocket_ = createserversocket(port);
    while(sockets_.size() < numhosts_)
    {
        sockets_.push_back(acceptconnection(serversocket_));
        cerr << "Got a new connection" << endl;
    }

    pthread_create(&listenerid_, 0, listener, this);

    sleep(1);
}

//Public functions:

Mutex::Mutex(int processid, vector<host> hosts, uint16_t port) :
    processid_(processid), numhosts_(hosts.size()),
    sequenceno_(65535), highestsequenceno_(0)
{
    initialize(hosts, port);
}

void Mutex::requestCS()
{
    pthread_mutex_lock(&pmutex_);
    requestingcs_ = true;
    outstandingreplies_ = numhosts_;
    sequenceno_ = highestsequenceno_ + 1;

    for(vector<int>::iterator it = sockets_.begin(); it != sockets_.end(); ++it)
    {
        Message m = { REQ, processid_, sequenceno_ };
        int rc = send(*it, &m, sizeof(Message), 0);
        if(rc < 0)
            perror("send()");
        cerr << "Sent REQ to fd=" << *it << endl;
    }

    pthread_mutex_unlock(&pmutex_);

    while(true)
    {
        pthread_mutex_lock(&pmutex_);
        if(outstandingreplies_ == 0)
        {
            pthread_mutex_unlock(&pmutex_);
            break;
        }
        pthread_mutex_unlock(&pmutex_);
        usleep(1);
    }
    cerr << "ENTERING CS" << endl;
}

void Mutex::releaseCS()
{
    pthread_mutex_lock(&pmutex_);
    requestingcs_ = false;
    for(vector<DeferredMessage>::iterator it = deferred_.begin(); it != deferred_.end(); ++it)
    {
        int rc = send(it->first, &it->second, sizeof(Message), 0);
        if(rc < 0)
            perror("send()");
        cerr << "Sent Deferred to fd=" << it->first << endl;
    }
    deferred_.clear();
    pthread_mutex_unlock(&pmutex_);
}

void Mutex::finish()
{
    pthread_mutex_lock(&pmutex_);
    for(vector<int>::iterator it = sockets_.begin(); it!= sockets_.end(); ++it)
    {
        cerr << "Sending DONE to fd=" << *it << endl;
        Message m = { DONE, processid_, sequenceno_ };
        int rc = send(*it, &m, sizeof(Message), 0);
        if(rc < 0)
            perror("send()");
    }
    pthread_mutex_unlock(&pmutex_);

    while(numhosts_ > 0)
    {
        pthread_mutex_lock(&pmutex_);
        for(vector<int>::iterator it = done_.begin(); it != done_.end(); ++it)
        {
            close(*it);
            for(vector<int>::iterator it1 = sockets_.begin(); it1 != sockets_.end(); ++it1)
            {
                if(*it1 == *it)
                {
                    cerr << "Erasing fd=" << *it1 << " from sockets_" << endl;
                    sockets_.erase(it1);
                    break;
                }
            }
            numhosts_--;
        }
        done_.clear();
        pthread_mutex_unlock(&pmutex_);
        usleep(1);
    }

}

#endif /*FAKEMUTEX*/
