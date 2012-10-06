
#include <iostream>

#include "Message.h"

using std::ostream;

ostream& operator<<(ostream &out, Message &m)
{
    out << m.type_ << " " << m.processid_;
    if(m.type_ != HELLO)
        out << " " << m.seqno_;
}
