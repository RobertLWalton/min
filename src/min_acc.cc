// MIN Allocator/Collector/Compactor
//
// File:	min_acc.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Jan 23 03:42:15 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/01/23 10:28:39 $
//   $RCSfile: min_acc.cc,v $
//   $Revision: 1.27 $

// Table of Contents:
//
//	Setup
//	Initializer
//	Stub Allocator
//	Block Allocator
//	Stub Stack Manager
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

// Ditto but for `min::unsptr' parameter.  maximum
// may not be larger than 2**63 - 1.
//
static bool get_param
	( const char * name, min::unsptr & parameter,
	  min::unsptr minimum = 0,
	  min::unsptr maximum = 0x7FFFFFFFFFFFFFFFull,
	  bool power_of_two = false,
	  bool trace = trace_parameters )
{
    assert ( maximum <= 0x7FFFFFFFFFFFFFFFull );
    min::int64 v;
    if ( ! get_param ( name, v,
                       minimum, maximum,
		       power_of_two, trace ) )
         return false;
    parameter = (min::unsptr) v;
    return true;
}

// Ditto but for `unsigned' parameter if `unsigned'
// not the same as `min::unsptr'.
//
#if MIN_POINTER_BITS > 32
static bool get_param
	( const char * name, unsigned & parameter,
	  unsigned minimum = 0,
	  unsigned maximum = 0xFFFFFFFFu,
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
#endif

// Convert a size in number of bytes into a size in
// number of pages, rounding up.
//
static min::unsptr page_size;
inline min::unsptr number_of_pages
    ( min::unsptr number_of_bytes )
{
    return ( number_of_bytes + page_size - 1 )
           / page_size;
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
	        trace_multi_page_block_allocation
		    = true;
	    case 'f':
	        trace_fixed_block_allocation = true;
	    case 'v':
	        trace_variable_block_allocation = true;
		break;
	    case 'o':
	        MOS::trace_pools = true;
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
    page_size = MOS::pagesize();

    stub_allocator_initializer();
    collector_initializer();
    block_allocator_initializer();
}


// Stub Allocator
// ---- ---------

min::unsptr MACC::max_stubs =
    MIN_DEFAULT_MAX_STUBS;
min::unsptr MACC::stub_increment =
    MIN_DEFAULT_STUB_INCREMENT;
min::stub * MACC::stub_begin;
min::stub * MACC::stub_next;
min::stub * MACC::stub_end;

static void stub_allocator_initializer ( void )
{
    get_param ( "max_stubs",
                MACC::max_stubs, 1000,
		MIN_MAX_NUMBER_OF_STUBS );
    get_param ( "stub_increment",
                MACC::stub_increment, 100, 1000000 );

    min::unsptr pages =
        number_of_pages ( 16 * MACC::max_stubs );
    void * stubs = MOS::new_pool_between
        ( pages, NULL,
	  (void *) MIN_MAX_ABSOLUTE_STUB_ADDRESS );

    const char * error = MOS::pool_error ( stubs );
    if ( error != NULL )
    {
        cout << "ERROR: " << error << endl
	     << "       while allocating vector to"
	        " hold max_stubs = " << MACC::max_stubs
	     << " stubs" << endl
	     << "       below address "
	     << (min::unsptr)
	        MIN_MAX_ABSOLUTE_STUB_ADDRESS
	     << endl
	     << "       Suggest decreasing max_stubs."
	     << endl;
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }

#   ifndef MIN_STUB_BASE
	MINT::null_stub = (min::stub *) stubs;
	MINT::stub_base = (min::unsptr) stubs;
#   else
	if ( (min::unsptr) stubs < MIN_STUB_BASE )
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
		     << (min::unsptr) MIN_STUB_BASE
		     << endl
		     << "       Suggest decreasing"
			" max_stubs."
		     << endl;
		MOS::dump_error_info ( cout );
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

void MINT::acc_expand_stub_free_list ( min::unsptr n )
{
    if ( n == 0 ) return;

    min::unsptr max_n =
        MACC::stub_end - MACC::stub_next;
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
	MOS::dump_error_info ( cout );
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
	          ( min::ACC_FREE,
		    MACC::stub_next + 1 ) );
	++ MACC::stub_next;
	-- n;
	++ MINT::number_of_free_stubs;
    }
    MUP::set_control_of
	( MACC::stub_next - 1 ,
	  MUP::new_acc_control
	      ( min::ACC_FREE, MINT::null_stub ) );
}


// Block Allocator
// ----- ---------

min::unsptr MACC::space_factor =
    MIN_DEFAULT_SPACE_FACTOR;
min::unsptr MACC::cache_line_size =
    MIN_DEFAULT_CACHE_LINE_SIZE;
min::unsptr MACC::subregion_size;
min::unsptr MACC::superregion_size;
min::unsptr MACC::max_paged_body_size;
min::unsptr MACC::paged_body_region_size;
min::unsptr MACC::stack_region_size;
min::unsptr MACC::stack_segment_size;

MACC::region * MACC::region_table;
MACC::region * MACC::region_next;
MACC::region * MACC::region_end;
MACC::region * MACC::last_superregion = NULL;
MACC::region * MACC::last_variable_body_region = NULL;
MACC::region * MACC::last_paged_body_region = NULL;
MACC::region * MACC::last_mono_body_region = NULL;
MACC::region * MACC::last_stack_region = NULL;
MACC::region * MACC::current_stack_region = NULL;

MACC::region *
    MACC::last_free_subregion = NULL;


static void allocate_new_superregion ( void );

static void block_allocator_initializer ( void )
{
    get_param ( "space_factor",
                MACC::space_factor, 8,
		( MIN_POINTER_BITS <= 32 ? 32 : 256 ),
		true );
    get_param ( "cache_line_size",
                MACC::cache_line_size,
		8, 4096, true );

    MACC::subregion_size = MACC::space_factor
                         * MACC::space_factor
	                 * page_size;

    // We allocate a maximum sized region table, on the
    // grounds that it is < 4 megabytes, its silly not
    // to do this for a 64 bit computer, and affordable
    // for a 32 bit computer.

    min::unsptr rtpages =
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
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }

    if ( trace_multi_page_block_allocation )
        cout << "TRACE: allocated " << rtpages
	     << " page region table" << endl;

    MACC::region_table = (MACC::region *) rt;
    MACC::region_next = MACC::region_table + 1;
    MACC::region_end =
          MACC::region_table
	+ MACC::MAX_MULTI_PAGE_BLOCK_REGIONS;

    min::unsptr M = 0;
    for ( min::unsptr t =
              MACC::space_factor * page_size;
    	  t >= 2; t /= 2 ) ++ M;
    MACC::superregion_size = (min::unsptr) M
                           * MACC::space_factor
			   * MACC::space_factor
			   * page_size;

    allocate_new_superregion();
}

// Allocate a new multi-page region with the given
// number of pages.
//
static MACC::region * new_multi_page_block_region
	( min::unsptr size, int type )
{
    min::unsptr pages = number_of_pages ( size );
    size = pages * page_size;
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
        ( type, size );
    r->begin = (min::uns8 *) m;
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
    MACC::region * r = new_multi_page_block_region
	( MACC::superregion_size, MACC::SUPERREGION );
    if ( r == NULL && MACC::last_superregion != NULL )
	r = new_multi_page_block_region
	        ( 4 * MACC::subregion_size,
		  MACC::SUPERREGION );
    if ( r == NULL && MACC::last_superregion != NULL )
	r = new_multi_page_block_region
	        ( MACC::subregion_size,
		  MACC::SUPERREGION );
    if ( r == NULL )
    {
        if ( MACC::last_superregion == NULL )
	    cout << "ERROR: could not allocate "
		 << ( MACC::subregion_size
		      / page_size )
		 << " page initial heap." << endl;
	else
	    cout << "ERROR: out of virtual memory."
	         << endl;
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }

    MACC::insert ( MACC::last_superregion, r );
}

void MINT::new_fixed_body
    ( min::stub * s, min::unsptr n,
      MINT::fixed_block_list * fbl )
{
    MINT::fixed_block_list_extension * fblext =
        fbl->extension;
    MACC::region * r = fblext->current_region;
    while ( r != NULL )
    {
        if ( r->free_first != NULL ) break;
	if ( r->next + fbl->size <= r->end ) break;
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
	    min::uns8 * new_next = sr->next
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

	min::unsptr s = fbl->size;
	if ( s > MACC::cache_line_size )
	    s = MACC::cache_line_size;
	s = s
	  * ( ( sizeof ( MACC::region ) + s - 1 ) / s );
	r->begin = r->next = (min::uns8 *) r + s;
	r->end = (min::uns8 *) r + MACC::subregion_size
	       -   ( MACC::subregion_size - s )
	         % fbl->size;

	assert ( r->end > r->begin );
	assert
	    ( ( r->end - r->begin ) % fbl->size == 0 );
		  
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
	// Add up to 16 pages worth of bodies to the
	// fbl free list.
	//
    	min::unsptr count = 16 * page_size / fbl->size;
	if ( count == 0 ) count = 1;
	MINT::free_fixed_size_block * last = NULL;
	while ( count -- > 0 )
	{
	    if ( r->next + fbl->size > r->end )
	    {
		assert
		    ( r->next + fbl->size == r->end );
		break;
	    }

	    MINT::free_fixed_size_block * b =
	        (MINT::free_fixed_size_block *) r->next;
	    b->block_control =
	        MUP::new_control_with_locator
		    (   ( (min::uns8 *) r - r->next )
		      / page_size,
		      MINT::null_stub );
	    b->block_subcontrol =
	        MUP::new_control_with_type
		    ( MACC::FREE, fbl->size );

	    if ( last == NULL )
	        fbl->first = b;
	    else
	        last->next = b;

	    last = b;

	    r->next += fbl->size;
	    ++ fbl->count;
	}
	if ( last != NULL ) last->next = NULL;
    }

    assert ( fbl->count > 0 );

    MINT::free_fixed_size_block * b = fbl->first;
    fbl->first = b->next;
    -- fbl->count;

    b->block_control = MUP::renew_control_stub
        ( b->block_control, s );
    MUP::set_pointer_of ( s, & b->block_control + 1 );
    MUP::set_flags_of ( s, ACC_FIXED_BODY_FLAG );
}

// Call this when we are out of paged body regions.
//
static void allocate_new_paged_body_region ( void )
{
    MACC::region * r = new_multi_page_block_region
	( MACC::paged_body_region_size,
	  MACC::PAGED_BODY_REGION );
    if ( r == NULL )
	r = new_multi_page_block_region
	        ( 4 * MACC::max_paged_body_size,
		  MACC::PAGED_BODY_REGION );
    if ( r == NULL )
	r = new_multi_page_block_region
	        ( MACC::max_paged_body_size,
		  MACC::PAGED_BODY_REGION );
    if ( r == NULL )
    {
	cout << "ERROR: out of virtual memory."
	     << endl;
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }

    r->block_size = MACC::max_paged_body_size;

    if ( MACC::last_paged_body_region != NULL )
        MACC::insert_after
	    ( r, MACC::last_paged_body_region );
    MACC::last_paged_body_region = r;
}

// Execute new_non_fixed_body for bodies of size at most
// MACC::max_paged_body_size.  n is size of body + body
// control rounded up to a multiple of page_size.
// Return address to store in stub.
//
inline void * new_paged_body
    ( min::stub * s, min::unsptr n )
{
    MACC::region * r = MACC::last_paged_body_region;
    if ( r == NULL || r->next + n > r->end )
    {
        allocate_new_paged_body_region();
	r = MACC::last_paged_body_region;
    }

    int locator = r - MACC::region_table;

    min::unsptr * b = (min::unsptr *) r->next;
    r->next += n;
    assert ( r->next <= r->end );

    * b = MUP::new_control_with_locator
	    ( locator, s );

    return ++ b;
}
// Execute new_non_fixed_body for bodies of size greater
// than MACC::max_paged_body_size.  n is size of body
// + body control rounded up to a multiple of page_size.
// Return address to store in stub.
//
inline void * new_mono_body
    ( min::stub * s, min::unsptr n )
{
    MACC::region * r = new_multi_page_block_region
	( n, MACC::MONO_BODY_REGION );
    if ( r == NULL )
    {
	cout << "ERROR: out of virtual memory."
	     << endl;
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }

    if ( MACC::last_mono_body_region != NULL )
        MACC::insert_after
	    ( r, MACC::last_mono_body_region );
    MACC::last_mono_body_region = r;

    int locator = r - MACC::region_table;

    min::unsptr * b = (min::unsptr *) r->next;
    r->next += n;
    assert ( r->next <= r->end );

    * b = MUP::new_control_with_locator
	    ( locator, s );

    return ++ b;
}

void MINT::new_non_fixed_body
    ( min::stub * s, min::unsptr n )
{
    // Add space for body control and round up to
    // multiple of page_size.
    //
    n += sizeof ( min::unsptr );
    n = page_size
      * ( ( n + page_size - 1 ) / page_size );

    void * body =
	n <= MACC::max_paged_body_size ?
	    new_paged_body ( s, n ) :
	    new_mono_body ( s, n );

    MUP::set_pointer_of ( s, body );
}

// Stub Stack Manager
// ---- ----- -------

void MACC::stub_stack
         ::allocate_stub_stack_segment ( void )
{
    region * r = current_stack_region;
    if ( r != NULL
         &&
	 r->free_count == 0
	 &&
	 r->next == r->end )
    {
	r = last_stack_region->region_previous;
	while ( r->free_count == 0
		&&
		r != last_stack_region )
	    r = r->region_next;
	if ( r->free_count == 0
	     &&
	     r->next == r->end )
	    r = NULL;
	else
	    current_stack_region = r;
    }
    if ( r == NULL )
    {
	r = new_multi_page_block_region
	    ( MACC::stack_region_size,
	      MACC::STACK_REGION );
	if ( r == NULL )
	{
	    cout << "ERROR: out of virtual"
		    " memory." << endl;
	    MOS::dump_error_info ( cout );
	    exit ( 1 );
	}
	insert ( last_stack_region, r );
	current_stack_region = r;
    }

    // Now r has a free segment or room to allocate
    // a new segment.

    stub_stack_segment * sss;
    if ( r->free_count > 0 )
    {
        MINT::free_fixed_size_block * b =
	    r->free_first;
	if ( ( r->free_first = b->next ) == NULL )
	    r->free_last = NULL;
	-- r->free_count;
	sss = (stub_stack_segment *) b;
    }
    else
    {
        sss = (stub_stack_segment *) r->next;
	r->next += stack_segment_size;
    }

    sss->block_control = MUP::new_control_with_locator
        ( r - region_table, MINT::null_stub );
    sss->block_subcontrol = MUP::new_control_with_type
        ( STACK_SEGMENT, stack_segment_size );

    sss->next = sss->begin;
    sss->end = (min::stub **)
               ( (char *) sss + stack_segment_size );

    if ( last_segment == NULL )
    {
        sss->previous_segment = sss->next_segment = sss;
	last_segment = input_segment
		     = output_segment = sss;
	input = output = sss->begin;
    }
    else
    {
	sss->previous_segment =
	    last_segment;
	sss->next_segment =
	    last_segment->next_segment;
	sss->previous_segment
	   ->next_segment = sss;
	sss->next_segment
	   ->previous_segment = sss;
	if ( input_segment == last_segment
	     &&
	     input == last_segment->next )
	{
	    input_segment == sss;
	    input = sss->begin;
	}
	if ( output_segment == last_segment
	     &&
	     output == last_segment->next )
	{
	    output_segment == sss;
	    output = sss->begin;
	}
	last_segment = sss;
    }
}

void MACC::stub_stack::flush ( void )
{
    while ( output_segment != last_segment )
    {
        stub_stack_segment * sss = last_segment;
	assert ( sss->next_segment != sss );
	last_segment = sss->previous_segment;

	sss->previous_segment->next_segment =
	    sss->next_segment;
	sss->next_segment->previous_segment =
	    sss->previous_segment;

	MINT::free_fixed_size_block * b =
	    (MINT::free_fixed_size_block *) sss;
	b->next = NULL;
	b->block_subcontrol = MUP::new_control_with_type
	    ( MACC::FREE, stack_segment_size );

	int locator = MUP::locator_of_control
	    ( sss->block_control );
	region * r = & region_table[locator];
	if ( r->free_count ++ == 0 )
	    r->free_first = r->free_last = b;
	else
	    r->free_last->next = b;

	// TBD: if all segments in r are free, free r.
    }
    rewind();
}


// Collector
// ---------

unsigned MACC::ephemeral_levels =
    MIN_DEFAULT_EPHEMERAL_LEVELS;

static void collector_initializer ( void )
{
    get_param ( "ephemeral_levels",
                MACC::ephemeral_levels,
		0, MIN_MAX_EPHEMERAL_LEVELS );
}
