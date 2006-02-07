// MIN Language Interface Test Program
//
// File:	min_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Feb  7 02:38:06 EST 2006
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2006/02/07 08:22:09 $
//   $RCSfile: min_interface_test.cc,v $
//   $Revision: 1.2 $

// Table of Contents:
//
//	Setup
//	C++ Number Types
//	Internal Pointer Conversion Functions
//	General Value Types and Data
//	General Value Constructor Functions
//	General Value Test Functions
//	General Value Read Functions
//	Control Values
//	Stub Types and Data
//	Stub Functions
//	Process Interface
//	Garbage Collector Interface
//	Numbers
//	Strings
//	Labels
//	Atom Functions
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level
//	Finish

// Setup
// -----

void min_assert
	( bool value,
	  const char * file, unsigned line,
	  const char * expression );
# define MIN_ASSERT(expr) \
    min_assert ( expr ? true : false, \
    		 __FILE__, __LINE__, #expr );

# include <min.h>
# include <iostream>
using std::cout;
using std::endl;

bool min_assert_enable = true;
bool min_assert_desired = true;
bool min_assert_print = true;
void min_assert
	( bool value,
	  const char * file, unsigned line,
	  const char * expression )
{
    bool print = min_assert_print;
    bool exit = false;
    if ( min_assert_enable
         && ( min_assert_desired != value ) )
    {
	print = true;
	exit = true;
    }
    if ( print )
	cout << "Line " << line
             << " assert " << expression
	     << ( value ? " true." : " false." )
	     << endl;
    if ( exit )
    {
        cout << "EXITING BECAUSE OF BAD ASSERT VALUE"
	     << endl;
    }
}

int main ()
{
    cout << "Start Test!" << endl;

// C++ Number Types
// --- ------ -----

    {
	cout << "Check that uns64 is 64 bits long:"
	     << endl;
    	min::uns64 u64 = (min::uns64) 1;
	u64 <<= 63;
	u64 >>= 13;
	min::float64 f64 = u64;
	MIN_ASSERT ( f64 != 0 );
	u64 <<= 14;
	u64 >>= 14;
	f64 = u64;
	MIN_ASSERT ( f64 == 0 );
	cout << "Number Types Done!" << endl;
    }


// Internal Pointer Conversion Functions
// General Value Types and Data
// General Value Constructor Functions
// General Value Test Functions
// General Value Read Functions
// Control Values
// Stub Types and Data
// Stub Functions
// Process Interface
// Garbage Collector Interface
// Numbers
// Strings
// Labels
// Atom Functions
// Objects
// Object Vector Level
// Object List Level
// Object Attribute Level

// Finish
// ------

    cout << "Finished Test!" << endl;
}
