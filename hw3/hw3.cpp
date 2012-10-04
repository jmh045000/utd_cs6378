
#include <pthread.h>
#include <stdint.h>

#include <iostream>
#include <string>
using namespace std;

#include "Message.h"
#include "Socket.h"

typedef struct
{
    uint16_t    port;
} listenerparams_t;

typedef struct
{
    string      hostname;
    uint16_t    port;
} senderparams_t;

void *listener(void *p)
{
    listenerparams_t params = *( (listenerparams_t*)p );

    ListenSocket ls(params.port);
    cout << "Accepting a connection!" << endl;
    Socket s(ls.acceptConnection());
    cout << "reading..." << endl;
    Message m(s.read());
    cout << m << endl;
}

void *sender(void *p)
{
    senderparams_t params = *( (senderparams_t*)p );

    Socket s(params.hostname, params.port);
    Message m(REQ, 0, 1);
    s.write(m);
    cout << s << endl;
}

int main(int argc, char *argv[])
{
    listenerparams_t lparams;
    lparams.port = 14000;

    senderparams_t sparams;
    sparams.port = 14000;

    if(argc > 1)
    {
        sparams.hostname = "net02";
    }
    else
    {
        sparams.hostname = "net01";
    }

    pthread_t lid, sid;

    pthread_create(&lid, 0, listener, &lparams);
    pthread_create(&sid, 0, sender, &sparams);

    sleep(4);
    return 0;
}
