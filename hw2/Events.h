#ifndef _EVENTS_H
#define _EVENTS_H

#include <exception>
#include <iostream>
#include <vector>

#include "Weight.h"

class Channel;

class Process
{
    char id_;
    Weight w_;
public:
    Process() : id_(0), w_(0) {}
    Process(char id) : id_(id), w_() {}
    Process(char id, Weight w) : id_(id), w_(w) {}

    Weight send(Process &master);
    void receive(Weight w);
    Weight idle();

    bool operator< (Process &right);
    bool operator!= (Process &right);
    bool operator== (Process &right);
    friend std::ostream& operator<< (std::ostream &out, Process &right);
    friend std::ostream& operator<< (std::ostream &out, Channel &right);
    friend class Channel;
};

class Channel
{
    Process *sender_;
    Process *receiver_;
    std::vector<Weight> weights_;
public:
    Channel(Process &sender, Process &receiver) : sender_(&sender), receiver_(&receiver) {}

    void sendMessage(Weight w);
    Weight receiveMessage();

    bool operator== (Channel &right);
    friend bool compareChannels(Channel &left, Channel &right);
    friend std::ostream& operator<< (std::ostream& out, Channel &right);
};

template <class T>
class EmptyException : public std::exception 
{
public:
    EmptyException(T what) : std::exception() { std::cerr << what << std::endl; }
};

bool compareChannels(Channel &left, Channel &right);

#endif /*_EVENTS_H*/
