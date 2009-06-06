// MIN OS Interface Test Program
//
// File:	min_os_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Jun  6 10:38:54 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/06/06 14:39:20 $
//   $RCSfile: min_os_test.cc,v $
//   $Revision: 1.5 $

// Table of Contents:
//
//	Setup
//	Memory Management Functions
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

bool print = false;

// Memory Management Functions
// ------ ---------- ---------

// Check that the last memory operation allocated a
// segment with given start address and number of
// pages.  Start address is checked for error.
//
void check_allocation ( min::uns64 pages, void * start )
{
    const char * error = MOS::pool_error ( start );
    if ( error != NULL )
    {
        cout << "ERROR:" << pages
	     << " pages NOT successfully allocated"
	     << endl
	     << "    " << error << endl;
        return;
    }

    void * end = (void *)
	( (char *) start + pages * MOS::pagesize() );

    // New segment must include allocated memory
    // and any old segment it replaces.  However,
    // the /proc/#/maps entry may extend beyond
    // memory acctually allocated.
    //
    bool ok = true;
    if ( new_count != 1 ) ok = false;
    else if ( old_count == 0 )
    {
        if ( start != used_pools[0].start
	     ||
             end != used_pools[0].end )
	    ok = false;
    }
    else if ( old_count == 1 )
    {
	used_pool & n = used_pools[0];
	used_pool & o = used_pools[1];
	if ( o.start == n.start )
	{
	    if ( o.end != start
	         ||
		 n.end != end )
		ok = false;
	}
	else if ( o.end == n.end )
	{
	    if ( n.start != start
	         ||
		 o.end != end )
		ok = false;
	}
	else
	    ok = false;
    }
    else
        ok = false;

    if ( ok )
        cout << pages << " pages successfully allocated"
	     << endl;
    else
    {
        cout << "ERROR:" << pages
	     << " pages NOT successfully allocated"
	     << endl;
	cout << "Allocated: "
	     << used_pool ( start, end ) << endl;
	dump_compare_pools();
    }
}

int main ( )
{
    cout << "Start min_os Test" << endl;

// Memory Management Tests
// ------ ---------- -----

    cout << endl
         << "Start Memory Management Test" << endl;

    MOS::trace_pools = false;
    create_compare = true;

    void * limit = (void *) 0xFFFF0000;

    void * start10 =
        MOS::new_pool_below ( 10, limit );
    check_allocation ( 10, start10 );

    void * start100 =
        MOS::new_pool ( 100 );
    check_allocation ( 100, start100 );

    void * start1000 =
        MOS::new_pool_below ( 1000, limit );
    check_allocation ( 1000, start1000 );

    void * start10000 =
        MOS::new_pool_below ( 10000, limit );
    check_allocation ( 10000, start10000 );

    cout << "Finish Memory Management Test" << endl
         << endl;


// Finish
// ------

    cout << "Finish min_os Test" << endl;
    return 0;
}
