
#ifndef MESSAGE_H
#define MESSAGE_H

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

static const int REQ=0;
static const int REPLY=1;
static const int HELLO=2;
static const int DONE=3;

typedef struct {
    int type;
    int processid;
    int seqno;
} Message;

#endif /*MESSAGE_H*/
