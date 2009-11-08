// MIN Language Builtin Function Test Program
//
// File:	min_builtin_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sun Nov  8 08:00:37 EST 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/11/08 13:26:39 $
//   $RCSfile: min_builtin_test.cc,v $
//   $Revision: 1.1 $

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
    for ( int i = 0; i <= 30; ++ i )
    {
	assert
	  ( min::internal
	       ::fixed_bodies_log ( 1u << i )
	    == i );
	assert
	  ( min::internal
	       ::fixed_bodies_log
	           ( ( 1u << ( i + 1 ) ) - 1 )
	    == i );
    }

    return 0;
}
