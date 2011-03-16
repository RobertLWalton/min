// This test program shows that the gen struct below is
// treated in assmebly code as if it were an unsigned long
// long, and therefore is as efficient as an unsigned long
// long.
//
// Compile with g++ -c [-O] -S gen_test.cc

#include <iostream>
using namespace std;

struct gen
{
    unsigned long long value;
};

inline gen make_gen ( unsigned long long value )
{
    gen x;
    x.value = value;
    return x;
}

inline unsigned long long value ( gen g )
{
    return g.value;
}

inline bool test ( gen x )
{
    return value ( x ) == 0;
}

inline gen inc ( gen x )
{
    return make_gen ( value ( x ) + 1 );
}

gen addtwo ( gen v )
{
    return test ( v ) ? v : inc ( inc ( v ) );
}
