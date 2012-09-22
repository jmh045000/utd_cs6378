
#include <exception>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>

#include "Weight.h"
#include "Events.h"

using namespace std;

typedef map<char, Process> ProcessMap;
typedef list<Channel> ChannelList;

ostream& operator<< (ostream &out, ChannelList &channels)
{

}

Channel* findChannel(ChannelList &channels, Channel c)
{
    for(ChannelList::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if( (*it) == c )
        {
            return &(*it);
        }
    }

    channels.push_back(c);
    return &channels.back();
}

void parse_events()
{
    ProcessMap processes;
    Process *master;
    ChannelList channels;
    ifstream events("events.txt");
    stringstream reports;

    while(events)
    {
        string line;
        getline(events, line);

        if( line == "" )
        {
            continue;
        }

        if(line == "report")
        {
            //do report
            stringstream report;
            for(ProcessMap::iterator it = processes.begin(); it != processes.end(); ++it)
            {
                report << it->second << '\t';
            }
            channels.sort(compareChannels);
            for(ChannelList::iterator it= channels.begin(); it != channels.end(); ++it)
            {
                stringstream channel;
                channel << (*it);
                if( channel.str() != "" )
                    report << channel.str() << '\t';
            }
            reports << report.str().substr(0, report.str().length()-1) << endl;
        }
        else
        {
            stringstream parser(line);

            char proc;
            parser >> proc;

            ProcessMap::iterator it = processes.find(proc);
            if(processes.empty())
            {
                //This is the master process
                processes[proc] = Process(proc);
                master = &(processes[proc]);
            }

            int eventid; //Not needed?
            parser >> eventid;

            char action;
            parser >> action;

            switch(action)
            {
            case 'A':
                //Nothing happens on atomic events
                break;
            case 'S':
                //Send event
                {
                    char receiver;
                    parser >> receiver;

                    if ( processes.find(receiver) == processes.end() )
                    {
                        processes[receiver] = Process(receiver, 0);
                    }

                    Channel *c = findChannel(channels, Channel(processes[proc], processes[receiver]));
                    c->sendMessage(processes[proc].send(*master));

                }
                break;
            case 'R':
                {
                    char sender;
                    parser >> sender;

                    Channel *c = findChannel(channels, Channel(processes[sender], processes[proc]));
                    processes[proc].receive(c->receiveMessage());
                }
                //Receive event
                break;
            case 'I':
                //Went IDLE
                {
                    if((*master) != processes[proc])
                    {
                        Channel *c = findChannel(channels, Channel(processes[proc], *master));
                        c->sendMessage( processes[proc].idle() );
                    }
                }
                break;
            default:
                //What..
                break;
            }

        }

    }
    ofstream out("reports.txt");
    out << reports.str().substr(0, reports.str().length()-1);
}

int main()
{
    //test_sort();
    parse_events();
    return 0;
}
