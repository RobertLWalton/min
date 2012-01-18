// MIN Language Builtin Function Test Program
//
// File:	min_builtin_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jan 18 06:57:54 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Setup
//	Main Program

// Setup
// -----

# include <min_parameters.h>
# include <iostream>
# include <cassert>
using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::ostream;



// Main Program
// ---- -------

int main ()
{
    cout << endl;
    cout << "Builtin Test!" << endl;
    cout << endl;
    cout << "MIN_USE_GNUC_BUILTINS = "
         << MIN_USE_GNUC_BUILTINS << endl;
    assert ( sizeof ( unsigned ) >= 4 );
    for ( unsigned i = 0; i <= 30; ++ i )
    {
	assert
	  ( min::internal ::log2floor ( 1u << i )
	    == i );
	assert
	  ( min::internal
	       ::log2floor ( ( 1u << ( i + 1 ) ) - 1 )
	    == i );
    }

    return 0;
}
