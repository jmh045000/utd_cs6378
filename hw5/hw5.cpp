
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

    Message() : l(BOTTOM) {}
    Message(int label) : l(label) {}

    Message(const Message &m) : l(m.l) {}

    Message &operator= (Message &m)
    {
        l = m.l;
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

    MessageMap last_received_other_;

    MessageMap last_received_mine_;
    MessageMap first_sent_mine_;
    ProcessList neighbors_;

    bool willing_to_ckpt;

    ostream *OUT;
    
public:

    Process() : id_(0) {}
    Process(char p, ostream &out = cout) : id_(p), willing_to_ckpt(false), OUT(&out) {}

    void add_neighbor(Process other_process);

    void send_message(Message m, Process other_process);
    void recv_message(Message m, Process other_process);

    void start_ckpt();
    void send_ckpt_request(Message m, Process other_process);
    void recv_ckpt_request(Message m, Process other_process);

    char id() { return id_; }

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
    *OUT << "SENDING: " << id_ << "->" << other_process.id_ << ": APPL(" << m.l << ")" << endl;
    if(first_sent_mine_[other_process.id_].l == BOTTOM)
    {
        first_sent_mine_[other_process.id_].l = m.l;
    }
    willing_to_ckpt = true;
}

void Process::recv_message(Message m, Process other_process)
{
    *OUT << "RECEIVING: " << other_process.id_ << "->" << id_ << ": APPL(" << m.l << ")" << endl;
    last_received_mine_[other_process.id_].l = m.l;
    willing_to_ckpt = true;
}

void Process::start_ckpt()
{
    *OUT << id_ << ": START CKPT" << endl;
    for(MessageMap::iterator it = last_received_mine_.begin(); it != last_received_mine_.end(); ++it)
    {
        if(it->second.l > BOTTOM)
        {
            *OUT << id_ << ": SEND CKPT -> " << it->first << "(" << it->second.l << ")" << endl;
        }
    }

}

void Process::send_ckpt_request(Message m, Process other_process)
{
    *OUT << "SENDING: " << id_ << "->" << other_process.id_ << ": CKPT(" << m.l << ")" << endl;
}

void Process::recv_ckpt_request(Message m, Process other_process)
{
    *OUT << "RECEIVING: " << other_process.id_ << "->" << id_ << ": CKPT(" << m.l << ")" << endl;
    // PROCESS CKPT REQUEST
    // The label of the message received is the last message received by other_process from me
    if(willing_to_ckpt)
    {
        if(m.l >= first_sent_mine_[other_process.id_].l && first_sent_mine_[other_process.id_].l > BOTTOM)
        {
            *OUT << id_ << ": TAKE TENTATIVE" << endl;
        }
    }
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

    ifstream topology("topology.txt");

    while(topology)
    {
        string line;
        getline(topology, line);
        if(line == "" || line[0] == '#') // ignore comments and blank lines
            continue;

        cout << "Processing line: " << line << endl;
        stringstream ss(line); // String streams are easier than strings...
        char one, two;
        ss >> one >> two;
    
        if(processes[one].id() == 0) processes[one] = Process(one);
        if(processes[two].id() == 0) processes[two] = Process(two);
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

        cout << "Processing line: " << line << endl;
        stringstream ss(line);
        char action, type, one, two;
        int id;
        ss >> action;
        switch(action)
        {
        case 'S':
            ss >> type >> one >> two >> id;
            switch(type)
            {
            case 'A':
                processes[one].send_message(Message(id), processes[two]);
                break;
            case 'C':
                processes[one].send_ckpt_request(Message(id), processes[two]);
                break;
            }
            break;
        case 'R':
            ss >> type >> one >> two >> id;
            switch(type)
            {
            case 'A':
                processes[two].recv_message(Message(id), processes[one]);
                break;
            case 'C':
                processes[two].recv_ckpt_request(Message(id), processes[one]);
                break;
            }
            break;
        case 'C':
            ss >> one;
            processes[one].start_ckpt();
            break;
        default:
            cout << "INVALID ACTION in line=" << line << endl;
        }
    }

    for(map<char, Process>::iterator it = processes.begin(); it != processes.end(); ++it)
    {
        cout << it->second << endl;
    }

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

