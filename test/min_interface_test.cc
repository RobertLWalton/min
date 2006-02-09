// MIN Language Interface Test Program
//
// File:	min_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Feb  8 19:20:23 EST 2006
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2006/02/09 00:33:40 $
//   $RCSfile: min_interface_test.cc,v $
//   $Revision: 1.6 $

// Table of Contents:
//
//	Setup
//	C++ Number Types
//	Internal Pointer Conversion Functions
//	General Value Types and Data
//	General Value Constructor/Test/Read Functions
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
# define MUP min::unprotected
# include <iostream>
# include <cstring>
using std::cout;
using std::endl;

struct min_assert_exception {};

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

#define desire_success(statement) \
    { cout << __FILE__ << ":" << __LINE__ \
	   << " desire success:" << endl \
	   << "    " << #statement << endl; \
      statement; }

#define desire_failure(statement) \
    try { cout << __FILE__ << ":" << __LINE__ \
               << " desire failure:" << endl \
	       << "    " << #statement << endl; \
	  statement; \
          cout << "EXITING BECAUSE OF SUCCESSFUL" \
	          " MIN_ASSERT" << endl; \
	  exit (1 ); } \
    catch ( min_assert_exception * x ) {}

int main ()
{
    cout << endl;
    cout << "Start Test!" << endl;

    try {

// C++ Number Types
// --- ------ -----

    {
	cout << endl;
	cout << "Start Number Types Test!" << endl;
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

	cout << "Finish Number Types Test!" << endl;
    }


// Internal Pointer Conversion Functions
// -------- ------- ---------- ---------

    {
	cout << endl;
	cout << "Start Internal Pointer Conversion"
	        " Test!" << endl;
    	char buffer[1];
	min::stub * stub =
	    (min::stub *) (sizeof (min::stub));

	cout << "Test pointer/uns64 conversions:"
	     << endl;
	min::uns64 u64 =
	    min::internal::pointer_to_uns64 ( buffer );
	char * b64 = (char *)
	    min::internal::uns64_to_pointer ( u64 );
	MIN_ASSERT ( b64 == buffer );

#	if MIN_IS_COMPACT
	    cout << "Test pointer/uns32 conversions:"
		 << endl;
	    min::uns32 u32 =
		min::internal::pointer_to_uns32
		    ( buffer );
	    char * b32 = (char *)
		min::internal::uns32_to_pointer ( u32 );
	    MIN_ASSERT ( b32 == buffer );
	    cout << "Test stub/uns32 conversions:"
		 << endl;
	    u32 = min::internal::stub_to_uns32 ( stub );
	    min::stub * s32 =
		min::internal::uns32_to_stub ( u32 );
	    MIN_ASSERT ( s32 == stub );
#	elif MIN_IS_LOOSE
	    cout << "Test stub/uns64 conversions:"
		 << endl;
	    u64 = min::internal::stub_to_uns64 ( stub );
	    min::stub * s64 =
		min::internal::uns64_to_stub ( u64 );
	    MIN_ASSERT ( s64 == stub );
#	endif

	cout << "Finish Internal Pointer Conversion"
	        " Test!" << endl;
    }

// General Value Types and Data
// ------- ----- ----- --- ----

    // There are no general value types and data tests.


// General Value Constructor/Test/Read Functions
// ------- ----- --------------------- ---------

    {
	cout << endl;
	cout << "Start General Value Constructor/"
	        "/Test/Read Function Test!" << endl;
	min::stub * stub =
	    (min::stub *) (sizeof (min::stub));

	cout << "Test stub general values:" << endl;
	min::gen sgen = MUP::new_gen ( stub );
	MIN_ASSERT ( min::is_stub ( sgen ) );
	MIN_ASSERT ( MUP::stub_of ( sgen ) == stub );
	sgen = min::new_gen ( stub );
	MIN_ASSERT ( min::is_stub ( sgen ) );
	MIN_ASSERT ( MUP::stub_of ( sgen ) == stub );

#	if MIN_IS_COMPACT
	    cout << "Test direct integer general"
	    	    " values:" << endl;
	    int i = -8434;
	    min::gen igen =
	    	MUP::new_direct_int_gen ( i );
	    MIN_ASSERT ( min::is_direct_int ( igen ) );
	    MIN_ASSERT
	        ( MUP::direct_int_of ( igen ) == i );
	    igen = min::new_direct_int_gen ( i );
	    MIN_ASSERT ( min::is_direct_int ( igen ) );
	    MIN_ASSERT
	        ( MUP::direct_int_of ( igen ) == i );
	    desire_failure (
	        igen = min::new_direct_int_gen
			    ( 1 << 27 );
	    );
	    desire_success (
		igen = min::new_direct_int_gen
			    ( 1 << 26 );
	    );
	    desire_failure (
	        igen = min::new_direct_int_gen
			    ( -1 << 28 );
	    );
	    desire_success (
		igen = min::new_direct_int_gen
			    ( -1 << 27 );
	    );
	    MIN_ASSERT
	        (    MUP::direct_int_of ( igen )
		  == -1 << 27 );
#	endif

#	if MIN_IS_LOOSE
	    cout << "Test direct float general"
	    	    " values:" << endl;
	    min::float64 f = -8.245324897;
	    min::gen fgen =
	    	MUP::new_direct_float_gen ( f );
	    MIN_ASSERT ( min::is_direct_float ( fgen ) );
	    MIN_ASSERT
	        ( MUP::direct_float_of ( fgen ) == f );
	    fgen = min::new_direct_float_gen ( f );
	    MIN_ASSERT ( min::is_direct_float ( fgen ) );
	    MIN_ASSERT
	        ( MUP::direct_float_of ( fgen ) == f );
#	endif
    
	cout << "Test direct string general values:"
	     << endl;
#	if MIN_IS_COMPACT
	    char * str = "ABC";
	    char * overflowstr = "ABCD";
#	elif MIN_IS_LOOSE
	    char * str = "ABCDE";
	    char * overflowstr = "ABCDEF";
#	endif
	union {
	    min::uns64 u64;
	    char str[8];
	} value;
	min::gen strgen =
	    MUP::new_direct_str_gen ( str );
	value.u64 = MUP::direct_str_of ( strgen );
	MIN_ASSERT ( strcmp ( str, value.str ) == 0 );
	desire_success (
	    strgen = min::new_direct_str_gen ( str );
	);
	desire_failure (
	    strgen = min::new_direct_str_gen
	    			( overflowstr );
	);
 



	cout << "Finish General Value Constructor/"
	        "/Test/Read Function Test!" << endl;
    }


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

    } catch ( min_assert_exception * x ) {
        cout << "EXITING BECAUSE OF FAILED MIN_ASSERT"
	     << endl;
	exit ( 1 );
    }

    cout << endl;
    cout << "Finished Test!" << endl;
}
