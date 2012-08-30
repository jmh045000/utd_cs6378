
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

#include <boost/lexical_cast.hpp>

//boost/lexical_cast
using boost::lexical_cast;
using boost::bad_lexical_cast;

class Event;

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

typedef pair<char, int> SendPair;
class SendAction : public Action
{
public:
    list<SendPair> receivers;
    SendAction(string send) : Action('S')
    {
        stringstream stream(send);
        while(stream)
        {
            char p = '\0';
            int id = -1;
            stream >> p >> id;
            if( p != '\0' && id != -1 )
                receivers.push_back(SendPair(p, id));
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
public:
    Event(char proc, int id, Action *action) : process_(proc), eventid_(id), action_(action) {}
    ~Event() { delete action_; }

    friend std::ostream & operator<< (std::ostream & out, Event *e);
    friend bool compareEvents(Event *, Event *);
};

map<char, int> processes;
list<Event*> eventlist;

bool compareEvents(Event* left, Event* right)
{

    if(left->process_ == right->process_
        && left->eventid_ < right->eventid_) 
    {
        return true;
    }
    else if(left->action_->type_ == 'R' && right->action_->type_ == 'S')
    {
        // left is a receiver
        if( ( (SendAction*)right->action_)->receiverscontains(SendPair(left->process_, left->eventid_)))
        {
            return false;
        }

    }
    else if(left->action_->type_ == 'S' && right->action_->type_ == 'R')
    {
        // right is a receiver
        if( ( (SendAction*)left->action_)->receiverscontains(SendPair(right->process_, right->eventid_)))
        {
            return true;
        }
    }
    else if(left->eventid_ == right->eventid_
        && processes[left->process_] < processes[right->process_])
    {
        return true;
    }
    else return false;
}

int main(int argc, char *argv[])
{
    ifstream inprocs("processes.txt");
    int pos = 0;
    while(inprocs)
    {
        char p, lessthan;
        inprocs >> p >> lessthan;

        processes[p] = pos++;
    }
    ifstream inevents("events.txt");

    while(inevents)
    {
        Event *e;
        Action *a;
        char proc = '\0', action = '\0';
        int event = -1;
        inevents >> proc >> event >> action;

        if ( proc == '\0' || action == '\0' || event == -1 ) continue;
        string line;
        switch(action)
        {
        case 'A':
            a = new AtomicAction();
            break;
        case 'S':
            getline(inevents, line);
            a = new SendAction(line);
            break;
        case 'R':
            a = new ReceiveAction();
            break;
        }

        e = new Event(proc, event, a);

        eventlist.push_back(e);
    }

    eventlist.sort(compareEvents);
    stringstream stream;
    for (list<Event*>::iterator it = eventlist.begin(); it != eventlist.end(); ++it)
    {
        stream << *it << "\t=>\t";
    }

    string output = stream.str();
    output = output.substr(0, output.rfind("\t=>"));
    cout << output << endl;
    eventlist.clear();
    return 0;
}

std::ostream & operator<<(std::ostream &out, Action *a)
{
    out << string(*a);
    return out;
}

std::ostream & operator<< (std::ostream & out, Event *e)
{
    out << e->process_ << e->eventid_;
    return out;
}

