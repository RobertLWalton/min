// MIN Language min::ref Optimization Test Program
//
// File:	ref_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Mar 19 09:25:12 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This test program shows that the ref struct below is
// treated in assmebly code as if it were a reference
// value, and is therefore as efficient as a T & value.
//
// Compile with g++ [-O] -S ref_test.cc

#include <iostream>
using namespace std;

template < typename T >
struct ref
{
    unsigned char ** p;
    unsigned long long offset;

    ref ( void ** p, const T & location )
        : p ( (unsigned char **) p ),
	offset (   (unsigned char *) & location
	         - (unsigned char *) * p ) {}

    operator T ( void )
    {
        return * (T *) ( * p + offset );
    }
    T operator = ( T value )
    {
        return * (T *) ( * p + offset ) = value;
    }
};

class foo
{
public:
    const double x, y, z;
};

foo f1 = { 0, 0, 0 };
foo * f1p = & f1;

inline ref<double> x ( foo ** p )
{
    return ref<double> ( (void **) p, (*p)->x );
}

inline ref<double> y ( foo ** p )
{
    return ref<double> ( (void **) p, (*p)->y );
}

inline ref<double> z ( foo ** p )
{
    return ref<double> ( (void **) p, (*p)->z );
}

double get_z ( foo ** p )
{
    return z(p);
}

void set_z ( foo ** p, double v )
{
    z(p) = v;
}

void test ( foo ** p )
{
    double w = x(p);
    x(p) = w + 1.0;
    z(p) = z(p) + y(p);
}
