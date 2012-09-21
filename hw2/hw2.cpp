
#include <iostream>
#include <gmp.h>
#include <stdint.h>
#include <unistd.h>

using namespace std;

class Weight
{
    uint64_t word0;
    uint64_t word1;
    uint64_t word2;
    uint64_t word3;
public:

    Weight() : word0(0x8000000000000000), word1(0), word2(0), word3(0) {}
    Weight(const Weight &w) : word0(w.word0), word1(w.word1), word2(w.word2), word3(w.word3) {}

    uint64_t getword(int i)
    {
        switch(i)
        {
        case 0: return word0;
        case 1: return word1;
        case 2: return word2;
        case 3: return word3;
        default: return ~0;
        }
    }
    
    void setword(int i, uint64_t w)
    {
        switch(i)
        {
        case 0: word0 = w; break;
        case 1: word1 = w; break;
        case 2: word2 = w; break;
        case 3: word3 = w; break;
        }
    }

    Weight operator= (Weight right)
    {
        this->word0 = right.word0;
        this->word1 = right.word1;
        this->word2 = right.word2;
        this->word3 = right.word3;
        return *this;
    }

    friend Weight operator/ (Weight &w, int divisor);
    friend Weight operator+ (Weight &left, Weight &right);
    friend ostream& operator<< (ostream& out, Weight &right);
};

Weight operator/ (Weight &left, int divisor)
{
    Weight w(left);
    bool carry = false;
    for(int i = 0; i < 4; i++)
    {
        uint64_t oldval = w.getword(i);
        w.setword(i, oldval>>1);

        if(carry) 
        {
            w.setword(i, w.getword(i) | 0x8000000000000000);
        }

        if(oldval & 1) carry = true;
        else carry = false;
    }

    return w;
}

Weight operator+ (Weight &left, Weight &right)
{
    return left;
}

ostream& operator<< (ostream& out, Weight &right)
{
    uint32_t numbits = 0;
    for(int i = 0; i < 4; i++)
    {
        for(uint64_t v = right.getword(i); v; v >>= 1)
        {
            numbits += (v&1);
        }
    }

    uint32_t i = 0;
    do
    {
        uint32_t bit = (right.getword( (i/64) ) >> ( 63 - (i%64) ) ) & 1;
        out << bit;
        numbits -= bit;
        i++;
    }while(numbits);

    return out;
}


int main()
{
    Weight w;
    cout << w << endl;

    for(int i = 0; i < 64; i++)
    {
        w = w / 2;
        cout << w << endl;
    }
    return 0;
}
