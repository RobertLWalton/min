// MIN Language Allocator/Collector/Compactor Test
// Program
//
// File:	min_acc_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sun Jan 24 10:11:17 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/01/24 15:11:36 $
//   $RCSfile: min_acc_test.cc,v $
//   $Revision: 1.2 $

// Table of Contents:
//
//	Setup

// Setup
// -----

# include <iostream>
# include <iomanip>
# include <cstring>
using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::ostream;

// Redefinition of MIN_ASSERT for use in min.h.
//
struct min_assert_exception {};
//
bool min_assert_print = true;
void min_assert
	( bool value,
	  const char * file, unsigned line,
	  const char * expression )
{
    if ( min_assert_print )
    {
	cout << file << ":" << line
             << " assert:" << endl 
	     << "    " << expression
	     << ( value ? " true." : " false." )
	     << endl;
    }
    if ( ! value )
	throw ( new min_assert_exception );
}

# define desire_success(statement) \
    { cout << __FILE__ << ":" << __LINE__ \
	   << " desire success:" << endl \
	   << "    " << #statement << endl; \
      statement; }

# define desire_failure(statement) \
    try { cout << __FILE__ << ":" << __LINE__ \
               << " desire failure:" << endl \
	       << "    " << #statement << endl; \
	  statement; \
          cout << "EXITING BECAUSE OF SUCCESSFUL" \
	          " MIN_ASSERT" << endl; \
	  exit ( 1 ); } \
    catch ( min_assert_exception * x ) {}
//
# define MIN_ASSERT(expr) \
    min_assert ( expr ? true : false, \
    		 __FILE__, __LINE__, #expr )

# include <min.h>
# include <min_acc.h>
# define MUP min::unprotected
# define MINT min::internal
# define MACC min::acc

// Helper functions for tests.

struct print_gen {
    min::gen g;
    print_gen ( min::gen g ) : g ( g ) {}
    friend ostream & operator <<
    	    ( ostream & s, print_gen pg )
    {
	return s << hex << pg.g << dec;
    }
};

// Temporary functions until real acc installed.
// --------------------------------------------

void MUP::deallocate_body
	( min::stub * s, min::unsptr size )
{}

// Main Program
// ---- -------

int main ()
{
    cout << endl;
    cout << "Start Test!" << endl;

    try {


    } catch ( min_assert_exception * x ) {
        cout << "EXITING BECAUSE OF FAILED MIN_ASSERT"
	     << endl;
	exit ( 1 );
    }

    cout << endl;
    cout << "Finished Test!" << endl;
}

