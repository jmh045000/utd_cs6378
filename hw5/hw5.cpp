
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>

using namespace std;

static const int BOTTOM = 0;
static const int TOP = -2;


class Message
{
public:
    int l; // The only important data for this algorithm is the message label
    string reply;

    Message() : l(BOTTOM), reply("NO") {}
    Message(int label) : l(label), reply("NO") {}
    Message(string r) : l(BOTTOM), reply(r) {}

    Message(const Message &m) : l(m.l), reply(m.reply) {}

    Message &operator= (Message &m)
    {
        l = m.l;
        reply = m.reply;
        return *this;
    }

};

class Process
{
    // This is the type that will be used for last_rmsg and first_smsg
    typedef map<char, Message> MessageMap;
    typedef list<char>         ProcessList;
private:
    // The *_other_ maps are the last received and first sent AT the other process
    char id_;

    MessageMap last_received_mine_;
    MessageMap first_sent_mine_;
    ProcessList neighbors_;

    bool willing_to_ckpt;
    bool tentative_taken;
    char ckpt_initiator;
    ProcessList ckpt_cohorts;
    ProcessList await_replys;

    ostream *OUT;

    void zero_counters()
    {
        for(MessageMap::iterator it = last_received_mine_.begin(); it != last_received_mine_.end(); ++it)
        {
            it->second.l = BOTTOM;
        }
        for(MessageMap::iterator it = first_sent_mine_.begin(); it != first_sent_mine_.end(); ++it)
        {
            it->second.l = BOTTOM;
        }
    }

    void add_to_cohorts(char id)
    {
        for(ProcessList::iterator it = ckpt_cohorts.begin(); it != ckpt_cohorts.end(); ++it)
        {
            if(*it == id)
            {
                return;
            }
        }
        ckpt_cohorts.push_back(id);
    }

    void remove_from_cohorts(char id)
    {
        for(ProcessList::iterator it = ckpt_cohorts.begin(); it != ckpt_cohorts.end(); ++it)
        {
            if(*it == id)
            {
                ckpt_cohorts.erase(it);
                return;
            }
        }
    }

    void add_to_replys(char id)
    {
        for(ProcessList::iterator it = await_replys.begin(); it != await_replys.end(); ++it)
        {
            if(*it == id)
            {
                return;
            }
        }
        await_replys.push_back(id);
    }

    void remove_from_replys(char id)
    {
        for(ProcessList::iterator it = await_replys.begin(); it != await_replys.end(); ++it)
        {
            if(*it == id)
            {
                await_replys.erase(it);
                return;
            }
        }
    }
    
public:

    Process() : id_(0) {}
    Process(char p, ostream &out = cout) : id_(p), willing_to_ckpt(false), tentative_taken(false), OUT(&out) {}

    void add_neighbor(Process other_process);

    void send_message(Message m, Process other_process);
    void recv_message(Message m, Process other_process);

    void start_ckpt();
    void send_ckpt_requests();
    void recv_ckpt_request(Message m, Process other_process);

    void send_ckpt_reply(char receiver);
    void recv_ckpt_reply(Message m, Process other_process);

    void send_ckpt_decision(Message m);
    void recv_ckpt_decision(Message m, Process other_process);

    char id() { return id_; }

    bool operator== (Process &right)
    {
        if(id_ == right.id_)    return true;
        else                    return false;
    }

    friend ostream& operator<< (ostream &out, Process &right);
};

void Process::add_neighbor(Process other_process)
{
    neighbors_.push_back(other_process.id_);
    last_received_mine_[other_process.id_].l = BOTTOM;
    first_sent_mine_[other_process.id_].l = BOTTOM;
}

void Process::send_message(Message m, Process other_process)
{
    *OUT << id_ << ":S\t" << other_process.id_ << "\tA\t" << m.l  << endl;
    if(first_sent_mine_[other_process.id_].l == BOTTOM)
    {
        first_sent_mine_[other_process.id_].l = m.l;
    }
    willing_to_ckpt = true;
}

void Process::recv_message(Message m, Process other_process)
{
    *OUT << id_ << ":R\t" << other_process.id_ << "\tA\t" << m.l << endl;
    last_received_mine_[other_process.id_].l = m.l;

    willing_to_ckpt = true;
    add_to_cohorts(other_process.id_);
}

void Process::start_ckpt()
{
    *OUT << id_ << ":C" << endl;
    if(willing_to_ckpt)
    {
        if(ckpt_cohorts.size() != 0)
        {
            *OUT << id_ << ":C\tT" << endl;
            tentative_taken = true;
            send_ckpt_requests();
            ckpt_initiator = 0;
            zero_counters();
        }
    }
}

void Process::send_ckpt_requests()
{
    for(ProcessList::iterator it = ckpt_cohorts.begin(); it != ckpt_cohorts.end(); ++it)
    {
        Message m = last_received_mine_[*it];
        *OUT << id_ << ":S\t" << *it << "\tC\t" << m.l << endl;
        await_replys.push_back(*it);
    }
}

void Process::recv_ckpt_request(Message m, Process other_process)
{
    *OUT << id_ << ":R\t" << other_process.id_ << "\tC\t" << m.l << endl;
    remove_from_cohorts(other_process.id_);
    // The label of the message received is the last message received by other_process from me
    if(willing_to_ckpt)
    {
        if(!tentative_taken)
        {
            if(m.l >= first_sent_mine_[other_process.id_].l && first_sent_mine_[other_process.id_].l > BOTTOM)
            {
                *OUT << id_ << ":C\tT" << endl;
                tentative_taken = true;
                ckpt_initiator = other_process.id_;
            }
            send_ckpt_requests();
            zero_counters();
        }
        else
        {
            send_ckpt_reply(other_process.id_);
        }
    }

    if(ckpt_cohorts.size() == 0)
    {
        if(ckpt_initiator != 0)
        {
            send_ckpt_reply(ckpt_initiator);
        }
    }
}

void Process::send_ckpt_reply(char receiver)
{
    if(ckpt_initiator != 0)
    {
        *OUT << id_ << ":S\t" << receiver << "\tR\t" << (willing_to_ckpt ? "YES" : "NO") << endl;
    }
    else
    {
        for(ProcessList::iterator it = ckpt_cohorts.begin(); it != ckpt_cohorts.end(); ++it)
        {
            *OUT << id_ << ":S\t" << *it << "\t" << (willing_to_ckpt ? "PERM" : "UNDO") << endl;
        }
    }
}

void Process::recv_ckpt_reply(Message m, Process other_process)
{
    if(m.reply == "NO")
    {
        willing_to_ckpt = false;
    }
    remove_from_replys(other_process.id_);

    if(await_replys.size() == 0)
    {
        send_ckpt_reply(ckpt_initiator);
    }
}

void Process::send_ckpt_decision(Message m)
{
    for(ProcessList::iterator it = ckpt_cohorts.begin(); it != ckpt_cohorts.end(); ++it)
    {
        *OUT << id_ << ":S\t" << *it << "\t" << m.reply << endl;
    }
}
    

void Process::recv_ckpt_decision(Message m, Process other_process)
{
    if(m.reply == "UNDO")
    {
        *OUT << id_ << ":UNDO" << endl;
    }
    else if(m.reply == "PERM")
    {
        *OUT << id_ << ":PERM" << endl;
    }
    send_ckpt_decision(m);
    tentative_taken = false;
}

ostream &operator<< (ostream &out, Process &right)
{
    out << "<-----" << endl << "Process " << right.id_ << endl;
    for(Process::MessageMap::iterator it = right.last_received_mine_.begin(); it != right.last_received_mine_.end(); ++it)
    {
        out << "Last received from <Process " << it->first << ">: " << it->second.l << endl;
    }
    for(Process::MessageMap::iterator it = right.first_sent_mine_.begin(); it != right.first_sent_mine_.end(); ++it)
    {
        out << "First sent to <Process " << it->first << ">: " << it->second.l << endl;
    }
    out << "Neighbors:" << endl;
    for(Process::ProcessList::iterator it = right.neighbors_.begin(); it != right.neighbors_.end(); ++it)
    {
        out << "<Process " << *it << ">" << endl;
    }
    out << "----->" << endl;
    return out;
}

int main()
{
    map<char, Process> processes;

    ofstream report("report.txt");

    ifstream topology("topology.txt");

    while(topology)
    {
        string line;
        getline(topology, line);
        if(line == "" || line[0] == '#') // ignore comments and blank lines
            continue;

        //cout << "Processing line: " << line << endl;
        stringstream ss(line); // String streams are easier than strings...
        char one, two;
        ss >> one >> two;
    
        if(processes[one].id() == 0) processes[one] = Process(one, report);
        if(processes[two].id() == 0) processes[two] = Process(two, report);
        processes[one].add_neighbor(processes[two]);
        processes[two].add_neighbor(processes[one]);
    }
    topology.close();

    ifstream events("events.txt");
    
    while(events)
    {
        string line;
        getline(events, line);
        if(line == "" || line[0] == '#') // ignore blank lines and comments
            continue;

        //cout << "Processing line: " << line << endl;
        stringstream ss(line);
        char action, type, one, two;
        int id;
        string reply;
        ss >> action;
        switch(action)
        {
        case 'S': // Send a message
            ss >> type >> one >> two;
            switch(type)
            {
            case 'A': // Sending application message
                ss >> id;
                processes[one].send_message(Message(id), processes[two]);
                break;
            }
            break;
        case 'R': // Receive a message
            ss >> type >> one >> two;
            switch(type)
            {
            case 'A': // Receive application message
                ss >> id;
                processes[two].recv_message(Message(id), processes[one]);
                break;
            case 'C': // Receive checkpoint request
                ss >> id;
                processes[two].recv_ckpt_request(Message(id), processes[one]);
                break;
            case 'R': // Receive checkpoint reply
                ss >> reply;
                processes[two].recv_ckpt_reply(Message(reply), processes[one]);
                break;
            case 'D': // Receive checkpoint decision (undo or make permanent)
                ss >> reply;
                processes[two].recv_ckpt_decision(Message(reply), processes[one]);
                break;
            }
            break;
        case 'C': // Start checkpoint algorithm
            ss >> one;
            processes[one].start_ckpt();
            break;
        default:
            cout << "INVALID ACTION in line=" << line << endl;
        }
    }

    /*
    for(map<char, Process>::iterator it = processes.begin(); it != processes.end(); ++it)
    {
        cout << it->second << endl;
    }
    */

    /*
    Process p('P');
    Process q('Q');
    Process r('R');

    p.add_neighbor(q);
    p.add_neighbor(r);
    
    q.add_neighbor(p);
    q.add_neighbor(r);

    r.add_neighbor(p);
    r.add_neighbor(q);

    p.send_message(Message(1), q);
    p.send_message(Message(1), r);

    q.recv_message(Message(1), p);
    r.recv_message(Message(1), r);

    q.send_message(Message(2), r);

    r.recv_message(Message(2), q);

    p.send_ckpt_request();

    cout << p;
    cout << q;
    cout << r;
    */

    return 0;
}

