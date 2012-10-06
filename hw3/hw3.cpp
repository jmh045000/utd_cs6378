
#include <pthread.h>
#include <stdint.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <boost/lexical_cast.hpp>

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
    if(argc < 3)
    {
        cerr << "Wrong usage:" << endl
            << argv[0] << " <processid> <othernode1> [ <othernode2> ... ]" << endl;
        return -1;
    }

    int processid;
    processid = boost::lexical_cast<int>(argv[1]);

    vector<string> hosts;
    for(int i = 2; i < argc; i++)
        hosts.push_back(argv[i]);

    try
    {
        Mutex m(processid, hosts, 14000);
    } catch(...) {
        cout << "woops" << endl;
        return -1;
    }

    for(int i = 0; i < 100; i++)
    {
        //Enter critical section
        {
            fstream file("dmutextest");
            int number;
            file >> number;
            number++;
            cout << number;
            file << number;
            file.close();
        }
    }

    return 0;
}
