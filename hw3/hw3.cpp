
#include <pthread.h>
#include <stdint.h>

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "Message.h"
#include "Mutex.h"
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

/*
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
*/

int main(int argc, char *argv[])
{
    vector<string> hosts;
    for(int i = 1; i < argc; i++)
        hosts.push_back(argv[i]);

    try
    {
        Mutex m(hosts, 14000);
    } catch(...) {
        cout << "woops" << endl;
    }
    return 0;
}
