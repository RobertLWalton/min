// MIN System Relocation Optimization Test Program
//
// File:	min_relocation_optimization_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Feb  9 09:44:01 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This code is intended to be complied into an assembly
// language file that is checked manually.  This code
// is not intended to be executed.

# include <iostream>
using std::cout;
using std::endl;
# include <min.h>
# define MUP min::unprotected
# define MINT min::internal

// Test1 tests when a pointer indirecting through
// a stub to a relocatable body is recomputed.  The
// pointer should be put into a register and should be
// recomputed when and only when out of line code is
// called.

// Out of line function.
//
void do_something ( void );

inline int * vec ( const min::stub * s )
{
    return (int *) MUP::ptr_of ( s );
}

// Function receiving const min:stub *.
//
int test1 ( const min::stub * s, int n )
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

// Test2 tests whether returning a ref<int> and then
// dereferencing it is less efficient that returning
// an int.  The answer should be no.
//
inline min::ref<int const> ref_index
    ( const min::stub * s, int i )
{
    return MUP::new_ref<int const>
        ( s, ( (int const *) MUP::ptr_of ( s ) ) [i] );
}
inline int index
    ( const min::stub * s, int i )
{
    return ( (int const *) MUP::ptr_of ( s ) ) [i];
}
int test2 ( const min::stub * s, int i1, int i2 )
{
    return ref_index ( s, i1 ) * index ( s, i2 );
}

// Test 3 tests whether the interrupt() function checks
// for asynchronous changes to MINT::acc_stack_limit.
//
void test3 ( int & n )
{
    min::interrupt();
    ++ n;
    min::interrupt();
}
