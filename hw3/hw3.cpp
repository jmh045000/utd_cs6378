
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

    Mutex m(processid, hosts, 14000);
    vector<string> sentence;
    sentence.push_back("The");
    sentence.push_back("quick");
    sentence.push_back("brown");
    sentence.push_back("fox");
    sentence.push_back("jumped");
    sentence.push_back("over");
    sentence.push_back("the");
    sentence.push_back("lazy");
    sentence.push_back("dog");

    for(int i = 0; i < 10; i++)
    {
        //Enter critical section
        m.requestCS();
        {
            ofstream out("dmutextest", ios_base::app);
            for(vector<string>::iterator it = sentence.begin(); it != sentence.end(); ++it)
            {
                out << *it << " " << flush;
                sleep(1);
            }
            out << endl << flush;
            out.close();

        }
        m.releaseCS();
    }

    m.finish();

    return 0;
}
