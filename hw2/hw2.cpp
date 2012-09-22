
#include <iostream>
#include <sstream>
#include <string>

#include <gmpxx.h>
#include <stdint.h>
#include <unistd.h>

using namespace std;

class Weight : public mpz_class
{
public:
    Weight() : mpz_class() { this->set_str("8000000000000000000000000000000000000000000000000000000000000000", 16); }

    friend ostream& operator<< (ostream& out, Weight &right);
};

ostream& operator<< (ostream& out, Weight &right)
{
    stringstream big;
    big.width(256); big.fill('0');
    big << internal << right.get_str(2);
    string weight = big.str();
    size_t lastone = weight.find_last_of("1");
    out << weight.substr(0, lastone+1);

    return out;
}

int main()
{
    Weight w;

    w /= 64;
    cout << w << endl;

    Weight w2(w);

    w2 /= 64;
    cout << w2 << endl;


    w += w2;
    cout << w << endl;

    return 0;
}
