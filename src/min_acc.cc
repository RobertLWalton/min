// MIN Allocator/Collector/Compactor
//
// File:	min_acc.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Aug 10 09:49:43 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/08/10 20:38:12 $
//   $RCSfile: min_acc.cc,v $
//   $Revision: 1.12 $

// Table of Contents:
//
//	Setup
//	Initializer
//	Stub Allocator
//	Block Allocator
//	Collector

// Setup
// -----

# include <min_acc.h>
# include <min_os.h>
# define MUP min::unprotected
# define MOS min::os
# define MINT min::internal
# define MACC min::acc

// For debugging.
//
# include <iostream>
# include <iomanip>
# include <cctype>
using std::hex;
using std::dec;
using std::cout;
using std::endl;
using std::setw;


// Initializer
// -----------

// Trace switches.
//
bool trace_parameters = false;
bool trace_fixed_block_allocation = false;
bool trace_variable_block_allocation = false;
bool trace_multi_page_block_allocation = false;

// Find the number of characters in a string before
// the first white space or end of string.
//
static int before_space ( const char * p )
{
    const char * q = p;
    for ( ; * q && ! isspace ( * q ); ++ q );
    return ( q - p );
}

// Get and set parameter.  Return false if nothing
// done and true if parameter set.  Announce error
// and exit program if parameter too small, too
// large, or not a power of two when that is required.
//
static bool get_param
	( const char * name, min::int64 & parameter,
	  min::int64 minimum = 0,
	  min::int64 maximum = 0x7FFFFFFFFFFFFFFFll,
	  bool power_of_two = false,
	  bool trace = trace_parameters )
{
    const char * s = MOS::get_parameter ( name );
    if ( s == NULL ) return false;

    char * p;
    min::int64 v = ::strtoll ( s, & p, 10 );
    if ( * p && ! isspace ( * p ) )
    {
        cout << "ERROR: bad " << name
	     << " program parameter value: "
	     << setw ( before_space ( s ) ) << s 
	     << endl;
	exit ( 1 );
    }
    if ( v < minimum )
    {
        cout << "ERROR: " << name
	     << " program parameter value too small: "
	     << v << " < " << minimum << endl;
	exit ( 1 );
    }
    if ( v > maximum )
    {
        cout << "ERROR: " << name
	     << " program parameter value too large: "
	     << v << " > " << maximum << endl;
	exit ( 1 );
    }
    if ( power_of_two
         &&
	 ( v & ( v - 1 ) ) != 0 )
    {
        cout << "ERROR: " << name
	     << " program parameter value is not a"
	        " power of two: " << v << endl;
	exit ( 1 );
    }

    if ( trace )
        cout << "TRACE: setting " << name
	     << "=" << v << endl;

    parameter = v;
    return true;
}

// Ditto but for `min::uns64' parameter.  maximum
// may not be larger than 2**63 - 1.
//
static bool get_param
	( const char * name, min::uns64 & parameter,
	  min::uns64 minimum = 0,
	  min::uns64 maximum = 0x7FFFFFFFFFFFFFFFull,
	  bool power_of_two = false,
	  bool trace = trace_parameters )
{
    assert ( maximum <= 0x7FFFFFFFFFFFFFFFull );
    min::int64 v;
    if ( ! get_param ( name, v,
                       minimum, maximum,
		       power_of_two, trace ) )
         return false;
    parameter = (min::uns64) v;
    return true;
}

// Ditto but for `unsigned' parameter.
//
static bool get_param
	( const char * name, unsigned & parameter,
	  unsigned minimum = 0,
	  unsigned maximum = unsigned ( -1 ),
	  bool power_of_two = false,
	  bool trace = trace_parameters )
{
    min::int64 v;
    if ( ! get_param ( name, v,
                       minimum, maximum,
		       power_of_two, trace ) )
         return false;
    parameter = (unsigned) v;
    return true;
}

// Convert a size in number of bytes into a size in
// number of pages, rounding up.
//
static min::uns64 pagesize;
inline min::uns64 number_of_pages
    ( min::uns64 number_of_bytes )
{
    return ( number_of_bytes + pagesize - 1 )
           / pagesize;
}

static void stub_allocator_initializer ( void );
static void block_allocator_initializer ( void );
static void collector_initializer ( void );
void MINT::acc_initializer ( void )
{
    const char * deb =
        MOS::get_parameter ( "debug" );
    if ( deb != NULL )
    {
        for ( const char * p = deb;
	      * p && ! isspace ( * p ); ++ p )
	{
	    switch ( * p )
	    {
	    case 'p':
	        trace_parameters = true;
		break;
	    case 'm':
	        trace_multi_page_block_allocation = true;
	    case 'f':
	        trace_fixed_block_allocation = true;
	    case 'v':
	        trace_variable_block_allocation = true;
		break;
	    default:
	        cout << "ERROR: cannot understand debug"
		        " = ..." << *p << "..." << endl;
		exit ( 1 );
	    }
	}
	if ( trace_parameters )
	    cout << "TRACE: debug="
	         << setw ( before_space ( deb ) ) << deb
		 << endl;
    }
    pagesize = MOS::pagesize();

    stub_allocator_initializer();
    collector_initializer();
    block_allocator_initializer();
}


// Stub Allocator
// ---- ---------

min::uns64 MACC::max_stubs =
    MIN_DEFAULT_MAX_STUBS;
unsigned MACC::stub_increment =
    MIN_DEFAULT_STUB_INCREMENT;
min::stub * MACC::stub_begin;
min::stub * MACC::stub_next;
min::stub * MACC::stub_end;

static void stub_allocator_initializer ( void )
{
    get_param ( "max_stubs",
                MACC::max_stubs, 1000,
		MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS );
    get_param ( "stub_increment",
                MACC::stub_increment, 100, 1000000 );

    min::uns64 pages =
        number_of_pages ( 16 * MACC::max_stubs );
    void * stubs = MOS::new_pool_below
        ( pages,
	  (void *) MIN_MAX_ABSOLUTE_STUB_ADDRESS );

    const char * error = MOS::pool_error ( stubs );
    if ( error != NULL )
    {
        cout << "ERROR: " << error << endl
	     << "       while allocating vector to"
	        " hold max_stubs = " << MACC::max_stubs
	     << " stubs" << endl
	     << "       below address "
	     << (min::uns64)
	        MIN_MAX_ABSOLUTE_STUB_ADDRESS
	     << endl
	     << "       Suggest decreasing max_stubs."
	     << endl;
	exit ( 1 );
    }

#   ifndef MIN_STUB_BASE
	MINT::null_stub = (min::stub *) stubs;
	MINT::stub_base = (MINT::pointer_uns) stubs;
#   else
	if ( (MINT::pointer_uns) stubs < MIN_STUB_BASE )
	{
	    // Oops, stub vector too low in memory.
	    // Assume that we are supposed to allocate
	    // vector beginning at MIN_STUB_BASE.

	    // Note: in this case MIN_MAX_ABSOLUTE_STUB_
	    // ADDRESS is defined to be large enough
	    // to accommodate stub vector.

	    MOS::free_pool ( pages, stubs );

	    stubs = MOS::new_pool_at
	        ( pages, (void *) MIN_STUB_BASE );

	    error = MOS::pool_error ( stubs );
	    if ( error != NULL )
	    {
		cout << "ERROR: " << error << endl
		     << "       while allocating vector"
		        " to hold max_stubs = "
		     << MACC::max_stubs << " stubs"
		     << endl
		     << "       at address"
		        " MIN_STUB_BASE = "
		     << (min::uns64) MIN_STUB_BASE
		     << endl
		     << "       Suggest decreasing"
			" max_stubs."
		     << endl;
		exit ( 1 );
	    }

	}
#   endif

    MACC::stub_begin = (min::stub *) stubs;
    MACC::stub_next = MACC::stub_begin;
    MACC::stub_end = MACC::stub_begin + MACC::max_stubs;

    MUP::set_control_of
        ( MACC::stub_next,
	  MUP::new_acc_control
	      ( min::DEALLOCATED,
	        MACC::stub_next ) );

    MINT::last_allocated_stub = MACC::stub_next;
    MINT::number_of_free_stubs = 0;

    ++ MACC::stub_next;

}

void MINT::acc_expand_stub_free_list ( unsigned n )
{
    if ( n == 0 ) return;

    min::uns64 max_n = MACC::stub_end - MACC::stub_next;
    if ( n > max_n )
    {
        cout << "ERROR: too many stubs required."
	     << endl
             << "       At most max_stubs = "
	     << MACC::max_stubs << " are allowed."
	     << endl
	     << "       Increase max_stubs or make"
	        " the garbage collector more efficient."
	     << endl;
	exit ( 1 );
    }
    n += MACC::stub_increment;
    if ( n > max_n ) n = max_n;
    MUP::set_control_of
        ( MINT::last_allocated_stub,
	  MUP::renew_acc_control_stub
	      ( MUP::control_of
	            ( MINT::last_allocated_stub ),
		MACC::stub_next ) );
    while ( n > 0 )
    {
	MUP::set_value_of ( MACC::stub_next, 0ull );
        MUP::set_control_of
	    ( MACC::stub_next,
	      MUP::new_acc_control
	          ( min::FREE, MACC::stub_next + 1 ) );
	++ MACC::stub_next;
	-- n;
	++ MINT::number_of_free_stubs;
    }
    MUP::set_control_of
	( MACC::stub_next - 1 ,
	  MUP::new_acc_control
	      ( min::FREE, MINT::null_stub ) );
}


// Block Allocator
// ----- ---------

unsigned MACC::space_factor;
unsigned MACC::subregion_size;
unsigned MACC::superregion_size;

MACC::region * MACC::region_table;
MACC::region * MACC::region_next;
MACC::region * MACC::region_end;
MACC::region * MACC::last_superregion;

MACC::region *
    MACC::last_free_subregion = NULL;


static void allocate_new_superregion ( void );

static void block_allocator_initializer ( void )
{
    get_param ( "space_factor",
                MACC::space_factor, 8,
		( MIN_POINTER_BITS <= 32 ? 32 : 256 ),
		true );

    MACC::subregion_size = MACC::space_factor
                         * MACC::space_factor
	                 * pagesize;

    // We allocate a maximum sized region table, on the
    // grounds that it is < 4 megabytes, its silly not
    // to do this for a 64 bit computer, and affordable
    // for a 32 bit computer.

    min::uns64 rtpages =
        number_of_pages
	    (   MACC::MAX_MULTI_PAGE_BLOCK_REGIONS
	      * sizeof ( MACC::region ) );
    void * rt = MOS::new_pool ( rtpages );
    const char * rterror = MOS::pool_error ( rt );
    if ( rterror != NULL )
    {
    	cout << "ERROR: could not allocate "
	     << rtpages << " page region table"
	     << endl;
	exit ( 1 );
    }

    if ( trace_multi_page_block_allocation )
        cout << "TRACE: allocated " << rtpages
	     << " page region table" << endl;

    MACC::region_table = (MACC::region *) rt;
    MACC::region_next = MACC::region_table;
    MACC::region_end =
          MACC::region_table
	+ MACC::MAX_MULTI_PAGE_BLOCK_REGIONS;

    min::uns64 M = 0;
    for ( unsigned t = MACC::space_factor * pagesize;
    	  t >= 2; t /= 2 ) ++ M;
    MACC::superregion_size = (min::uns64) M
                           * MACC::space_factor
			   * MACC::space_factor
			   * pagesize;

    allocate_new_superregion();
}

// Allocate a new multi-page region with the given
// number of pages.
//
static MACC::region * new_multi_page_region
	( min::uns64 size )
{
    min::uns64 pages = number_of_pages ( size );
    size = pages * pagesize;
    void * m = MOS::new_pool ( pages );
    const char * error = MOS::pool_error ( m );
    if ( error != NULL ) return NULL;

    if ( MACC::region_next == MACC::region_end )
    {
        cout << "ERROR: trying to allocate more than "
	     << (   MACC::region_end
	          - MACC::region_table )
	     << " multi-page block regions" << endl;
	exit ( 1 );
    }

    MACC::region * r = MACC::region_next ++;
    r->block_control = 0;
    r->block_subcontrol = MUP::new_control_with_type
        ( MACC::MULTI_PAGE_REGION, size );
    r->begin = (char *) m;
    r->next = r->begin;
    r->end = r->begin + size;
    r->block_size = 0;
    r->free_count = 0;
    r->region_previous = r;
    r->region_next = r;
    r->free_first = NULL;
    r->free_last = NULL;

    if ( trace_multi_page_block_allocation )
        cout << "TRACE: allocated " << pages
	     << " page ( " << size << " byte)"
	     << " multi-page block region"
	     << endl;

    return r;
}

// Call this when we are out of superregions.
//
static void allocate_new_superregion ( void )
{
    MACC::region * r = new_multi_page_region
                           ( MACC::superregion_size );
    if ( r == NULL && MACC::last_superregion != NULL )
	r = new_multi_page_region
	        ( 4 * MACC::subregion_size );
    if ( r == NULL && MACC::last_superregion != NULL )
	r = new_multi_page_region
	        ( MACC::subregion_size );
    if ( r == NULL )
    {
        if ( MACC::last_superregion == NULL )
	    cout << "ERROR: could not allocate "
		 << MACC::superregion_size / pagesize
		 << " page initial heap." << endl;
	else
	    cout << "ERROR: out of virtual memory."
	         << endl;
	exit ( 1 );
    }

    if ( MACC::last_superregion != NULL )
        MACC::insert_after ( r, MACC::last_superregion );
    MACC::last_superregion = r;
}

void MINT::new_fixed_body
    ( min::stub * s, unsigned n,
      MINT::fixed_body_list * fbl )
{
    MINT::fixed_body_list_extension * fblext =
        fbl->extension;
    MACC::region * r = fblext->current_region;
    while ( r != NULL )
    {
        if ( r->free_first != NULL ) break;
	if ( r->next < r->end ) break;
	r = r->region_next;
	if ( r == fblext->current_region ) r = NULL;
    }
    if ( r == NULL )
    {
	MACC::region * sr;	// Superregion of r.

        r = MACC::last_free_subregion;
	if ( r != NULL )
	{
	    int locator = MUP::locator_of_control
	                       ( r->block_control );
	    sr = MACC::region_table + locator;
	    assert ( MACC::region_table <= sr
	             &&
		     sr < MACC::region_end );

	    if ( r == r->region_previous )
		MACC::last_free_subregion = NULL;
	    else
	    {
		MACC::last_free_subregion =
		    r->region_previous;
		MACC::remove ( r );
	    }
	}
	else
	{
	    MACC::region * sr = MACC::last_superregion;
	    char * new_next = sr->next
	                    + MACC::subregion_size;

	    if ( new_next > sr->end )
	    {
	        allocate_new_superregion();
		sr = MACC::last_superregion;
		new_next = sr->next
	                 + MACC::subregion_size;
		assert ( new_next <= sr->end );
	    }

	    r = (MACC::region *) sr->next;
	    sr->next = new_next;
	}

	r->block_control =
	    MUP::new_control_with_locator
	        ( sr - MACC::region_table,
		  MINT::null_stub );
	r->block_subcontrol =
	    MUP::new_control_with_type
	        ( MACC::FIXED_SIZE_BLOCK_REGION,
		  MACC::subregion_size );

	unsigned s = fbl->size;
	s = s
	  * ( ( sizeof ( MACC::region ) + s - 1 ) / s );
	r->begin = r->next = (char *) r + s;
	r->end = (char *) r + MACC::subregion_size;

	assert
	    ( ( r->end - r->begin ) % fbl->size == 0 );
		  

	r->region_next = fblext->first_region;
	r->region_previous =
	    fblext->first_region->region_previous;
	r->region_next->region_previous = r;

	r->block_size = fbl->size;
	r->free_count = 0;
	r->region_previous = r->region_next = r;
	if ( fblext->current_region != NULL )
	    MACC::insert_after
	        ( r, fblext->current_region );

	r->free_first = r->free_last = NULL;
    }

    fblext->current_region = r;

    if ( r->free_count > 0 )
    {
        fbl->count = r->free_count;
	fbl->first = r->free_first;
	r->free_count = 0;
	r->free_first = r->free_last = NULL;
    }
    else
    {
    	unsigned count = 16 * pagesize / fbl->size;
	if ( count == 0 ) count = 1;
	void * last = NULL;
	while ( count -- > 0 )
	{
	    if ( r->next + fbl->size > r->end )
	    {
	        r->next = r->end;
		break;
	    }
	    if ( last == NULL )
	        fbl->first = r->next;
	    else
	        * ( void **) last = r->next;
	    r->next += fbl->size;
	    ++ fbl->count;
	}
	if ( last != NULL )
	        * ( void **) last = NULL;
    }
}



// Collector
// ---------

unsigned MACC::ephemeral_levels =
    MIN_MAX_EPHEMERAL_LEVELS;

static void collector_initializer ( void )
{
    get_param ( "ephemeral_levels",
                MACC::ephemeral_levels,
		0, MIN_MAX_EPHEMERAL_LEVELS );
}
