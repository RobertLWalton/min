// MIN OS Interface Test Program
//
// File:	min_os_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Apr 11 14:27:25 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Setup
//	Memory Management Functions
//	Memory Management Tests

// Setup
// -----

#include <iostream>
#include <iomanip>
#include <cstring>
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


// Memory Management Functions
// ------ ---------- ---------

inline void * page ( void * start, int i )
{
    return (void *)
           ( (char *) start + i * ::pagesize() );
}

extern "C" {
#   include <signal.h>
#   include <setjmp.h>

    // Note: an attempt to throw an exception from a
    // SIGSEGV handler failed (as if the handler
    // search routine could not find handlers
    // installed before the signal), so we have had
    // to use longjmp.  (Possibly this could have
    // been fixed by adding SA_RESTART and SA_NODEFER
    // flags, which we did not do till later.)

    // Return true if a read of a byte from the address
    // does NOT cause a segment violation, and false if
    // it does.
    //
    static char test_address_value;
    static void test_address_handler ( int );
    static jmp_buf test_address_env;
    bool test_address ( void * address )
    {
	fname = "test_address";

	struct sigaction sa, old_sa;

	sa.sa_handler = test_address_handler;
	sigemptyset ( & sa.sa_mask );
	sa.sa_flags = SA_RESTART + SA_NODEFER;
	    // These flags are necessary or else the
	    // second call to test_address with an
	    // inaccessible address fails to catch
	    // the SIGSEGV and exits the program.

	if (    sigaction ( SIGSEGV, & sa, & old_sa )
	     == -1 )
	    fatal_error ( errno );

	bool result = true;

	if ( setjmp ( test_address_env ) != 0 )
	    result = false;
	else
	{
	    // Save value read so this cannot be
	    // optimized away.
	    //
	    test_address_value = * (char *) address;
	}

	if (    sigaction ( SIGSEGV, & old_sa, NULL )
	     == -1 )
	    fatal_error ( errno );

	return result;
    }

    static void test_address_handler ( int )
    {
        longjmp ( test_address_env, 1 );
    }
}

// Given a sequence of pages, store i, i+1, i+2, ...
// in the 1st, 2nd, 3rd, page, in the first and
// last `int' in each page.  But if i is 0, store
// 0's instead.
//
void set_pages ( min::uns64 pages, void * start, int i )
{
    unsigned length = ::pagesize() / sizeof ( int );
    int * p = (int *) start;
    while ( pages -- )
    {
        p[0] = p[length-1] = i;
	p += length;
	if ( i != 0 ) ++ i;
    }
}

// Return true if and only if a sequence of pages
// contains the `int' values set by set_pages.
//
bool check_pages
	( min::uns64 pages, void * start, int i )
{
    unsigned length = ::pagesize() / sizeof ( int );
    int * p = (int *) start;
    while ( pages -- )
    {
	if ( p[0] != i ) return false;
	if ( p[length-1] != i ) return false;
	p += length;
	if ( i != 0 ) ++ i;
    }
    return true;
}

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

    // New segment must be allocated segment or
    // concatenation of that and old segment.
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
		 o.start != end )
		ok = false;
	}
	else
	    ok = false;
    }
    else if ( old_count == 2 )
    {
	used_pool & n = used_pools[0];
	unsigned i = start == used_pools[1].end ? 1 : 2;
	used_pool & olow = used_pools[i];
	used_pool & ohigh = used_pools[3-i];
	if ( n.start != olow.start
	     ||
	     n.end != ohigh.end
	     ||
	     start != olow.end
	     ||
	     end != ohigh.start )
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

// Check that the last memory operation deallocated a
// segment with given start address and number of
// pages.
//
void check_deallocation
	( min::uns64 pages, void * start )
{
    void * end = (void *)
	( (char *) start + pages * MOS::pagesize() );

    // New segments concatenated with deallocated
    // segment must equal old segment.
    //
    bool ok = true;
    if ( old_count != 1 ) ok = false;
    else if ( new_count == 0 )
    {
        if ( start != used_pools[0].start
	     ||
             end != used_pools[0].end )
	    ok = false;
    }
    else if ( new_count == 1 )
    {
	used_pool & n = used_pools[0];
	used_pool & o = used_pools[1];
	if ( o.start == n.start )
	{
	    if ( n.end != start
	         ||
		 o.end != end )
		ok = false;
	}
	else if ( o.end == n.end )
	{
	    if ( o.start != start
	         ||
		 n.start != end )
		ok = false;
	}
	else
	    ok = false;
    }
    else if ( new_count == 2 )
    {
	used_pool & o = used_pools[2];
	unsigned i = start == used_pools[0].end ? 0 : 1;
	used_pool & nlow = used_pools[i];
	used_pool & nhigh = used_pools[1-i];
	if ( o.start != nlow.start
	     ||
	     o.end != nhigh.end
	     ||
	     start != nlow.end
	     ||
	     end != nhigh.start )
	    ok = false;
    }
    else
        ok = false;

    if ( ok )
        cout << pages
	     << " pages successfully deallocated"
	     << endl;
    else
    {
        cout << "ERROR:" << pages
	     << " pages NOT successfully deallocated"
	     << endl;
	cout << "Deallocated: "
	     << used_pool ( start, end ) << endl;
	dump_compare_pools();
    }
}

// Check that there has been no change in the pages
// that are in segments, though there may be changes
// in the segments (some pages may have changed access
// status or been moved within existing segments).
// Return true if no change, false if change.
//
inline bool check_no_change ( void )
{
    if ( new_count + old_count > 0 )
    {
        // Check that new entries concatenate to a
	// contiguous segment and old entries
	// concatenate to the same segment.
	//
	// Note that new entries are in ascending
	// address order, and ditto old entries.
	//
        if ( new_count == 0 || old_count == 0 )
	    return false;
	else
	{
	    void * start = used_pools[0].start;
	    void * end   = used_pools[0].end;
	    unsigned i = 0;
	    while ( ++ i < new_count )
	    {
	        if ( used_pools[i].start != end )
		    return false;
		else
		    end = used_pools[i].end;
	    }
	    if ( start != used_pools[i].start )
	        return false;
	    void * next   = used_pools[i].end;
	    while ( ++ i < new_count + old_count )
	    {
	        if ( used_pools[i].start != next )
		    return false;
		else
		    next = used_pools[i].end;
	    }
	    if ( next != end )
	        return false;
	}
    }

    return true;
}

// Check that there was REALLY no change.
//
bool inline check_really_no_change ( void )
{
    return old_count + new_count == 0;
}

// Check that the last move memory operation but did
// not change allocated segments.
//
void check_move
	( min::uns64 pages,
	  void * new_start, void * old_start )
{
    bool ok = check_no_change();

    if ( ok )
        cout << pages
	     << " pages successfully moved"
	     << endl;
    else
    {
        cout << "ERROR:" << pages
	     << " pages NOT successfully moved"
	     << endl;
	cout << "Moved: "
	     << used_pool ( pages, old_start ) << " to "
	     << used_pool ( pages, new_start ) << endl;
	dump_compare_pools();
    }
}

// Check that the last memory operation access change
// properly changed allocated segments.  Type is
// "accessible" or "inaccessible".
//
void check_reaccess ( min::uns64 pages, void * start,
                      const char * type )
{
    bool ok = check_no_change();

    if ( ok )
        cout << pages
	     << " pages successfully made " << type
	     << endl;
    else
    {
        cout << "ERROR:" << pages
	     << " pages NOT successfully made "
	     << type << endl;
	cout << "Made " << type << ": "
	     << used_pool ( pages, start ) << endl;
	dump_compare_pools();
    }
}

int main ( int argc, const char ** argv )
{
    cout << "Start min_os Test" << endl;

// Parameter Tests
// --------- -----

    cout << endl
         << "Start Parameter Test" << endl;

    const char * parameter1 =
        MOS::get_parameter ( "parameter1" );
    const char * parameter2 =
        MOS::get_parameter ( "parameter2" );
    const char * parameter3 =
        MOS::get_parameter ( "parameter3" );
    MIN_ASSERT ( parameter1 != NULL );
    MIN_ASSERT ( parameter2 != NULL );
    MIN_ASSERT ( parameter3 == NULL );
    MIN_ASSERT
        ( strncmp ( parameter1, "123 ", 4 ) == 0 );
    MIN_ASSERT
        ( strcmp ( parameter2, "3.21" ) == 0 );

    cout << "Finish Parameter Test" << endl
         << endl;

// Memory Management Tests
// ------ ---------- -----

    cout << endl
         << "Start Memory Management Test" << endl;

    MOS::trace_pools = 0;
    create_compare = true;

    void * lower = (void *) 0x00FF0000;
    void * upper = (void *) 0xFFFF0000;

    void * start10 =
        MOS::new_pool_between ( 10, lower, upper );
    check_allocation ( 10, start10 );

    void * start100 =
        MOS::new_pool ( 100 );
    check_allocation ( 100, start100 );

    void * start1000 =
        MOS::new_pool_between ( 1000, lower, upper );
    check_allocation ( 1000, start1000 );

    void * start10000 =
        MOS::new_pool_between ( 10000, lower, upper );
    check_allocation ( 10000, start10000 );

#   define P(i) page ( start1000, i )
    set_pages ( 1000, P(0), 1000000 );

    MOS::purge_pool ( 100, P(200) );
    MIN_ASSERT ( check_really_no_change() );
    MIN_ASSERT ( check_pages ( 200, P(0), 1000000 ) );
    MIN_ASSERT ( check_pages ( 100, P(200), 0 ) );
    MIN_ASSERT ( check_pages ( 700, P(300), 1000300 ) );

    MOS::free_pool ( 300, P(200) );
    check_deallocation ( 300, P(200) );

    MIN_ASSERT ( test_address ( P(0) ) );
    MIN_ASSERT ( ! test_address ( P(200) ) );
    MIN_ASSERT ( ! test_address ( P(499) ) );

    void * start1200 = MOS::new_pool_at ( 250, P(200) );
    check_allocation ( 250, start1200 );

    void * start1450 = MOS::new_pool_at ( 50, P(450) );
    check_allocation ( 50, start1450 );

    MIN_ASSERT ( check_pages ( 200, P(0), 1000000 ) );
    MIN_ASSERT ( check_pages ( 300, P(200), 0 ) );
    MIN_ASSERT ( check_pages ( 500, P(500), 1000500 ) );

    set_pages ( 1000, P(0), 1000000 );

    MOS::move_pool ( 100, P(0), P(200) );
    check_move ( 100, P(0), P(200) );

    MIN_ASSERT ( check_pages ( 100, P(0), 1000200 ) );
    MIN_ASSERT ( check_pages ( 100, P(100), 1000100 ) );
    MIN_ASSERT ( check_pages ( 100, P(200), 0 ) );
    MIN_ASSERT ( check_pages ( 700, P(300), 1000300 ) );

    set_pages ( 1000, P(0), 1000000 );

    MOS::move_pool ( 500, P(0), P(100) );
    check_move ( 500, P(0), P(100) );

    MIN_ASSERT ( check_pages ( 500, P(0), 1000100 ) );
    MIN_ASSERT ( check_pages ( 100, P(500), 0 ) );
    MIN_ASSERT ( check_pages ( 400, P(600), 1000600 ) );

    set_pages ( 1000, P(0), 1000000 );

    MOS::move_pool ( 500, P(100), P(0) );
    check_move ( 500, P(100), P(0) );

    MIN_ASSERT ( check_pages ( 100, P(0), 0 ) );
    MIN_ASSERT ( check_pages ( 500, P(100), 1000000 ) );
    MIN_ASSERT ( check_pages ( 400, P(600), 1000600 ) );

    set_pages ( 1000, P(0), 1000000 );

    MOS::inaccess_pool ( 100, P(100) );
    check_reaccess ( 100, P(100), "inaccessible" );
    MIN_ASSERT ( check_pages ( 100, P(0), 1000000 ) );
    MIN_ASSERT ( ! test_address ( P(100) ) );
    MIN_ASSERT ( ! test_address ( P(199) ) );
    MIN_ASSERT ( check_pages ( 800, P(200), 1000200 ) );

    MOS::access_pool ( 100, P(100) );
    check_reaccess ( 100, P(100), "accessible" );
    MIN_ASSERT ( check_pages ( 100, P(0), 1000000 ) );
    MIN_ASSERT ( check_pages ( 100, P(100), 0 ) );
    MIN_ASSERT ( check_pages ( 800, P(200), 1000200 ) );

    // Find largest size of memory that can be
    // allocated.
    //
    MOS::trace_pools = 0;
    {
        unsigned n = 0;
	while ( true )
	{
	    min::uns64 pages = 1ull << n;
	    void * start = MOS::new_pool ( pages );
	    const char * error =
	        MOS::pool_error ( start );
	    if ( error != NULL ) break;
	    MOS::free_pool ( pages, start );
	    ++ n;
	}
	-- n;
	cout << "new_pool will allocate at most 1 << "
	     << n << " pages." << endl;
	cout << "virtual memory has "
	     << MOS::virtualsize() / ::pagesize()
	     << " = 0x" << std::hex
	     << MOS::virtualsize() / ::pagesize()
	     << std::dec << " pages." << endl;
    }

    if ( argc > 1 ) MOS::dump_error_info ( cout );

    cout << "Finish Memory Management Test" << endl
         << endl;

// Memory File Tests
// ------ ---- -----

    cout << endl
         << "Start File Management Test" << endl;

    min::uns64 fsize;
    char error_message[512];

    if ( !  ( MOS::file_size
	          ( fsize, "min_os_test.cc",
	            error_message ) ) )
    {
        cout << error_message << endl;
	abort();
    }
    cout << "min_os_test.cc has "
         << fsize << " bytes" << endl;

    if ( !  ( MOS::file_size
	          ( fsize, "min_non_existent_file",
	            error_message ) ) )
        cout << error_message << endl;

    cout << "Finish File Management Test" << endl
         << endl;



// Finish
// ------

    cout << "Finish min_os Test" << endl;
    return 0;
}
