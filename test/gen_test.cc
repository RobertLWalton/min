// MIN Language min::gen Optimization Test Program
//
// File:	gen_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Mar 16 16:05:46 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This test program shows that the gen struct below is
// treated in assmebly code as if it were an unsigned long
// long, and therefore is as efficient as an unsigned long
// long.
//
// Compile with g++ -c [-O] -S gen_test.cc

#include <iostream>
using namespace std;

typedef unsigned long long unsgen ;

struct gen;
namespace unprotected 
{
    gen new_gen ( unsgen value );
    unsgen value_of ( gen g );
}

struct gen
{

private:

    unsgen value;

    friend gen unprotected::new_gen ( unsgen value );
    friend unsgen unprotected::value_of ( gen g );
};

namespace unprotected {

    inline gen new_gen ( unsgen value )
    {
	gen x;
	x.value = value;
	return x;
    }

    inline unsgen value_of ( gen g )
    {
	return g.value;
    }

}
inline bool test ( gen x )
{
    return unprotected::value_of ( x ) == 0;
}

inline gen inc ( gen x )
{
    return unprotected::new_gen
        ( unprotected::value_of ( x ) + 1 );
}

gen addtwo ( gen v )
{
    return test ( v ) ? v : inc ( inc ( v ) );
}
