// MIN OS Interface Test Program
//
// File:	min_os_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Jun  6 02:43:51 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/06/06 12:24:18 $
//   $RCSfile: min_os_test.cc,v $
//   $Revision: 1.4 $

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

    unsigned new_count, old_count;
    compare_pools ( new_count, old_count, print );

    // New segment must include allocated memory
    // and any old segment it replaces.  However,
    // the /proc/#/maps entry may extend beyond
    // memory acctually allocated.
    //
    bool ok = true;
    if ( new_count != 1 ) ok = false;
    else if ( old_count == 0 )
    {
        if ( start < used_pools[0].start
	     ||
             end > used_pools[0].end )
	    ok = false;
    }
    else if ( old_count == 1 )
    {
	used_pool & n = used_pools[0];
	used_pool & o = used_pools[1];
	if ( o.start < n.start
	     ||
	     o.end > n.end )
	    ok = false;
	if ( start < n.start
	     ||
	     end > n.end )
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
	cout << "Allocated: " << hex
	     << (MINT::pointer_uns) start << '-'
	     << (MINT::pointer_uns) end
	     << dec << endl;
	cout << "New map entries:" << endl;
	dump_used_pools ( 0, new_count );
	cout << "Old map entries:" << endl;
	dump_used_pools ( new_count, old_count );
    }
}

int main ( )
{
    cout << "Start min_os Test" << endl;

// Memory Management Tests
// ------ ---------- -----

    bool print = false;

    void * limit = (void *) 0xFFFF0000;
    read_used_pools ( "read_used_pools", print );
    void * start10 = MOS::new_pool_below ( 10, limit );
    check_allocation ( 10, start10 );
    read_used_pools ( "read_used_pools", print );
    void * start100 = MOS::new_pool_below ( 100, limit );
    check_allocation ( 100, start10 );
    read_used_pools ( "read_used_pools", print );
    void * start1000 = MOS::new_pool_below ( 1000, limit );
    check_allocation ( 1000, start100 );



// Finish
// ------

    cout << "Finish min_os Test" << endl;
    return 0;
}
