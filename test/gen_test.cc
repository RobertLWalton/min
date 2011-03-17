// MIN Language min::gen Optimization Test Program
//
// File:	gen_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Mar 16 19:59:55 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This test program shows that the gen struct below is
// treated in assmebly code as if it were an unsigned
// long long, and therefore is as efficient as an
// unsigned long long.
//
// Compile with g++ -c [-O] -S gen_test.cc

#include <iostream>
#include <cassert>
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

struct sizetest {
    unsgen x;
    gen y;
    unsgen z;
};

int main()
{
    assert ( sizeof ( unsgen ) == sizeof ( gen ) );
    assert (    10 * sizeof ( unsgen )
             == sizeof ( gen[10] ) );
    assert (    3 * sizeof ( unsgen )
             == sizeof ( sizetest ) );
    gen x = unprotected::new_gen ( 5 );
    x = addtwo ( x );
    assert ( unprotected::value_of ( x ) == 7 );
    cout << "OK" << endl;
}
