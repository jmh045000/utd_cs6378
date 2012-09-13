
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>

//fstream
using std::ifstream;
using std::ofstream;

//iostream
using std::cerr;
using std::cout;
using std::endl;

//list
using std::list;

//map
using std::map;

//sstream
using std::stringstream;

//string
using std::string;

//utility
using std::pair;

class Event;

typedef pair<char, int> SendPair;
map<SendPair, int> messages;

#ifdef DEBUG
std::ostream & operator<<(std::ostream &, SendPair);
#endif

class Action
{
protected:
    char type_;
public:
    Action(char t) : type_(t) {}
    virtual operator string() = 0;
    friend std::ostream & operator<< (std::ostream &out, Action *a);
    friend bool compareEvents(Event *, Event *);
};

class AtomicAction : public Action
{
public:
    AtomicAction() : Action('A') {}
    operator string() {return "<AtomicAction>"; }
};

class SendAction : public Action
{
public:
    list<SendPair> receivers;
    int timestamp;
    SendAction(string send, int ts) : Action('S'), timestamp(ts)
    {
        stringstream stream(send);
        while(stream)
        {
            char p = '\0';
            int id = -1;
            stream >> p >> id;
            if( p != '\0' && id != -1 )
            {
                SendPair receiver(p, id);
                receivers.push_back(receiver);
                messages[receiver] = timestamp;
            }
        }
    }

    bool receiverscontains(SendPair p)
    {
        for(list<SendPair>::iterator it = receivers.begin(); it != receivers.end(); ++it)
        {
            if(*it == p) return true;
        }
        return false;
    }

    operator string()
    {
        stringstream stream;
        stream << "<SendAction: ";
        for (list<SendPair>::iterator it = receivers.begin(); it != receivers.end(); ++it)
        {
            stream << "(" << it->first << "," << it->second << "),";
        }
        stream << ">";
        return stream.str();
    }
};

class ReceiveAction : public Action
{
public:
    ReceiveAction() : Action('R') {}
    operator string() { return "<ReceiveAction>"; }
};

class Event
{
    char process_;
    int eventid_;
    Action *action_;
    int clock_;
public:
    Event(char proc, int id, Action *action, int clock) : process_(proc), eventid_(id), action_(action), clock_(clock) {}
    ~Event() { delete action_; }

    int clock() { return clock_; }

    friend std::ostream & operator<< (std::ostream & out, Event *e);
    friend Event* findevent(SendPair);
    friend bool compareEvents(Event *, Event *);
};

map<char, int> processes;
list<Event*> events;

Event *findevent(SendPair p)
{
    for (list<Event*>::iterator it = events.begin(); it != events.end(); ++it)
    {
        if(p.first == (*it)->process_ && p.second == (*it)->eventid_)
        {
            return *it;
        }
    }
    return NULL;
}

bool compareEvents(Event* left, Event* right)
{

    if(left->process_ == right->process_)
    {
        if(left->eventid_ < right->eventid_)
            return true;
        else
            return false;
    }
    else
    {
        if(left->clock() < right->clock())
        {
            return true;
        }
        else if(left->clock() == right->clock())
        {
            if (processes[left->process_] < processes[right->process_])
                return true;
            else
                return false;
        }
        else
        {
            return false;
        }
    }
}

int main(int argc, char *argv[])
{
    ifstream inprocs("processes.txt");
    if(!inprocs)
    {
        cerr << "Failed to open processes.txt" << endl;
        return -1;
    }
    int pos = 0;
    while(inprocs)
    {
        char p, lessthan;
        inprocs >> p >> lessthan;

        processes[p] = pos++;
    }

    ifstream inevents("events.txt");
    if(!inevents)
    {
        cerr << "Failed to open events.txt" << endl;
        return -1;
    }

    while(inevents)
    {
        Event *e;
        Action *a;
        char proc = '\0', action = '\0';
        int eventid = -1;
        inevents >> proc >> eventid >> action;

        if ( proc == '\0' || action == '\0' || eventid == -1 ) continue;

        int clock = 0;
        Event *event = findevent(SendPair(proc, eventid-1));
        if(event)
        {
            clock = event->clock()+1;
#ifdef DEBUG
            cout << event;
#endif
        }
        else
        {
#ifdef DEBUG
            cout << "Did not find event (" << proc << ", " << eventid-1 << endl;
#endif
        }
        
        string line;
        switch(action)
        {
        case 'A':
            a = new AtomicAction();
            break;
        case 'S':
            getline(inevents, line);
            a = new SendAction(line, clock);
            break;
        case 'R':
            a = new ReceiveAction();
            int msgts = messages[SendPair(proc, eventid)];
#ifdef DEBUG
            cout << "pair: " << SendPair(proc, eventid) << ", ts=" << messages[SendPair(proc, eventid)] << endl;
#endif
            if(msgts >= clock)
            {
                clock = msgts+1;
            }
            break;
        }

        e = new Event(proc, eventid, a, clock);

        events.push_back(e);
#ifdef DEBUG
        cout << "Added event: " << e;
#endif
    }

    events.sort(compareEvents);
    stringstream stream;
    for (list<Event*>::iterator it = events.begin(); it != events.end(); ++it)
    {
        stream << *it << "\t=>\t";
    }

    string output = stream.str();
    output = output.substr(0, output.rfind("\t=>"));
#ifdef DEBUG
    cout << output;
#else
    ofstream total("totalordering.txt");
    total << output;
#endif
    events.clear();
    return 0;
}

#ifndef DEBUG
std::ostream & operator<<(std::ostream &out, Action *a)
{
    return out;
}

std::ostream & operator<< (std::ostream & out, Event *e)
{
    out << e->process_ << e->eventid_;
    return out;
}

#else
std::ostream & operator<<(std::ostream &out, SendPair p)
{
    out << "(" << p.first << ", " << p.second << ")";
    return out;
}
std::ostream & operator<<(std::ostream &out, Action *a)
{
    out << string(*a);
    return out;
}

std::ostream & operator<< (std::ostream & out, Event *e)
{
    out << "<Event: Process=" << e->process_ << e->eventid_ << ", clock=" << e->clock_ << ", " << e->action_ << endl;
    return out;
}

#endif

