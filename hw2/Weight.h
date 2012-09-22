#ifndef _WEIGHT_H
#define _WEIGHT_H

#include <gmpxx.h>

#include <iostream>

class Weight: public mpz_class
{
public:
    Weight() : mpz_class() { this->set_str("8000000000000000000000000000000000000000000000000000000000000000", 16); }
    Weight(int w) : mpz_class(w) {}

    friend std::ostream& operator<< (std::ostream& out, Weight &right);
};

#endif /*_WEIGHT_H*/
