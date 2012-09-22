
#include "Events.h"

using namespace std;

Weight Process::send(Process &master)
{
    if( (w_/2) > 0)
    {
        w_ /= 2;
        Weight w(w_);
        return w;
    }
    else
    {
        Weight w(0);
        return w;
    }

}

void Process::receive(Weight w)
{
    w_ += w;
}

Weight Process::idle()
{
    Weight w = w_;
    w_ = 0;
    return w;
}

bool Process::operator< (Process& right)
{
    if(this->id_ < right.id_)
        return true;
    else
        return false;
}

bool Process::operator!= (Process& right)
{
    if(this->id_ != right.id_)
        return true;
    else
        return false;
}

bool Process::operator== (Process& right)
{
    if(this->id_ == right.id_)
        return true;
    else
        return false;
}

ostream& operator<< (ostream &out, Process &right)
{
    out << right.id_ << "=" << right.w_;
    return out;
}

void Channel::sendMessage(Weight w)
{
    weights_.push_back(w);
}

Weight Channel::receiveMessage()
{
    if(weights_.empty()) throw EmptyException<Channel>(*this);
    
    Weight w(weights_[0]);

    weights_.erase(weights_.begin());

    return w;
}

bool Channel::operator== (Channel &right)
{
    if(*this->sender_ == *right.sender_ && *this->receiver_ == *right.receiver_)
        return true;
    else
        return false;
}

bool compareChannels(Channel &left, Channel &right)
{
    if(*left.sender_ < *right.sender_)
    {
        return true;
    }
    else if(*right.sender_ < *left.sender_)
    {
        return false;
    }
    else
    {
        if(*left.receiver_ < *right.receiver_)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

ostream& operator<< (ostream &out, Channel &right)
{
    Weight w(0);
    for(vector<Weight>::iterator it = right.weights_.begin(); it != right.weights_.end(); ++it)
    {
        w += *it;
    }

    if( w != 0 )
    {
        out << "c" << (*right.sender_).id_ << (*right.receiver_).id_;
        out << "=" << w;
    }
    return out;
}
