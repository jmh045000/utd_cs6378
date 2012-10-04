
#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <sstream>
#include <string>

static const int REQ=0;
static const int REPLY=1;

class Message
{
    int processid_;
    int seqno_;
    int type_;

public:
    Message(int processid, int seqno, int type) : processid_(processid), seqno_(seqno), type_(type) {}
    Message(std::string s)
    {
        *this << s;
    }
    
    friend std::ostream& operator<< (std::ostream &out, Message &m);

    void operator<< (std::string s)
    {
        std::stringstream ss(s);
        ss >> type_;
        ss >> processid_;
        ss >> seqno_;
    }

    operator const std::string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

};

#endif /*MESSAGE_H*/
