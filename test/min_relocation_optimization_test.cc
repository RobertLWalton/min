// MIN System Relocation Optimization Test Program
//
// File:	min_relocation_optimization_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Mar 16 16:08:25 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This program tests when a pointer indirecting through
// a stub to a relocatable body is recomputed.  The
// pointer should be put into a register and should be
// recomputed when and only when out of line code is
// called.
//
// This code is intended to be complied into an assembly
// language file that is checked manually.  This code
// is not intended to be executed.

# include <iostream>
using std::cout;
using std::endl;
# include <min.h>
# define MUP min::unprotected

// Out of line function.
//
void do_something ( void );

inline int * vec ( const min::stub * s )
{
    return (int *) MUP::ptr_of ( s );
}

// Function receiving const min:stub *.
//
int test ( const min::stub * s, int n )
{
    int x = vec(s)[0];
    for ( int i = 1; i < n; ++ i ) x += vec(s)[i];
    x += vec(s)[10];
    do_something();
    x += vec(s)[0];
    for ( int i = 1; i < n; ++ i ) x += vec(s)[i];
    x += vec(s)[10];
    return x;
}

