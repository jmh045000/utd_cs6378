
#include <pthread.h>
#include <stdint.h>
#include <time.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <boost/lexical_cast.hpp>

#include "Message.h"
#include "Mutex.h"
#include "Socket.h"

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        cerr << "Wrong usage:" << endl
            << argv[0] << " <configuration file>" << endl;
        return -1;
    }

    int processid = -1, port = 14000;
    vector<host> hosts;

    ifstream infile(argv[1]);

    infile >> processid;
    infile >> port;
    while(infile)
    {
        host h;
        infile >> h.hostname >> h.port;
        if(h.hostname != "")
            hosts.push_back(h);
    }

    Mutex m(processid, hosts, port);
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

    cout << "Starting at next 5 second boundary" << endl;
    time_t starttime = ((time(0)/5)*5)+5;

    while(time(0) < starttime);

    for(int i = 0; i < 10; i++)
    {
        //Enter critical section
        m.requestCS();
        {
            for(vector<string>::iterator it = sentence.begin(); it != sentence.end(); ++it)
            {
                ofstream out("dmutextest", ios_base::app);
                out << *it << " " << flush;
                out.close();
            }
            ofstream out("dmutextest", ios_base::app);
            out << endl << flush;
            out.close();
        }
        m.releaseCS();
    }

    m.finish();

    return 0;
}
