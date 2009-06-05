// MIN OS Interface Test Program
//
// File:	min_os_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Fri Jun  5 13:58:55 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/06/05 17:59:02 $
//   $RCSfile: min_os_test.cc,v $
//   $Revision: 1.3 $

// Table of Contents:
//
//	Setup
//	Memory Management Tests

// Setup
// -----

#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;

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

// Note: we include min_os.cc because we want to use
// its static functions and data.
//
# include "../src/min_os.cc"

MINT::initializer::initializer ( void ) {}

int main ( )
{
    cout << "Start min_os Test" << endl;

// Memory Management Tests
// ------ ---------- -----

    bool print = false;

    read_used_pools ( "read_used_pools", print );
    unsigned pages = 10;
    void * start = MOS::new_pool ( pages );
    const char * error = MOS::pool_error ( start );
    if ( error != NULL ) cout << error << endl;
    MIN_ASSERT ( error == NULL );

    void * end = (void *)
	( (char *) start + pages * MOS::pagesize() );

    unsigned new_count, old_count;
    compare_pools ( new_count, old_count, print );

    MIN_ASSERT ( new_count == 1 );
    if ( old_count == 0 )
    {
        assert ( used_pools[0].start == start );
        assert ( used_pools[0].end   == end );
    }
    else
    {
	assert ( old_count == 1 );
	used_pool & n = used_pools[0];
	used_pool & o = used_pools[1];
        if ( n.start == o.start )
	{
	    assert ( o.end == start );
	    assert ( n.end == end );
	}
	else if ( n.end == o.end )
	{
	    assert ( n.start == start );
	    assert ( o.end == end );
	}
	else
	{
	    assert ( ! "allocation gap exists" );
	}
    }


// Finish
// ------

    cout << "Finish min_os Test" << endl;
    return 0;
}
