
#include <sstream>
#include <string>

#include "Weight.h"

using namespace std;

ostream& operator<< (ostream& out, Weight &right)
{
    if(right == 0)
    {
        out << '0';
    }
    else
    {
        stringstream big;
        big.width(256); big.fill('0');
        big << internal << right.get_str(2);
        string weight = big.str();
        size_t lastone = weight.find_last_of("1");
        out << weight.substr(0, lastone+1);
    }

    return out;
}

