
#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <sstream>
#include <string>

static const int REQ=0;
static const int REPLY=1;
static const int HELLO=2;
static const int DONE=3;

class Message
{
    int type_;
    int processid_;
    int seqno_;

public:
    Message(int type, int processid, int seqno = 0) : processid_(processid), seqno_(seqno), type_(type) {}
    Message(std::string s)
    {
        *this << s;
    }

    const int type()        { return type_; }
    const int processid()   { return processid_; }
    const int seqno()       { return seqno_; }
    
    friend std::ostream& operator<< (std::ostream &out, Message &m);

    void operator<< (std::string s)
    {
        std::stringstream ss(s);
        ss >> type_;
        ss >> processid_;
        if(type_ != HELLO)
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
