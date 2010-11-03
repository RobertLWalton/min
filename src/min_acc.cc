// MIN Allocator/Collector/Compactor
//
// File:	min_acc.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Nov  3 01:18:11 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/08/15 05:59:13 $
//   $RCSfile: min_acc.cc,v $
//   $Revision: 1.83 $

// Table of Contents:
//
//	Setup
//	Initializer
//	Stub Allocator
//	Block Allocator
//	Packed Type Allocator
//	Stub Stack Manager
//	Collector
//	Compactor
//	Statistics

// Setup
// -----

# include <min_acc.h>
# include <min_os.h>
# define MUP min::unprotected
# define MOS min::os
# define MINT min::internal
# define MACC min::acc

using namespace MACC;

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
static bool trace_parameters = false;
static bool trace_fixed_block_allocation = false;
static bool trace_variable_block_allocation = false;
// MOS::trace_pools >= 1 traces all pool related
// allocations.

// Find the number of characters in a string before
// the first white space or end of string.
//
static int before_space ( const char * p )
{
    const char * q = p;
    for ( ; * q && ! isspace ( * q ); ++ q );
    return ( q - p );
}

// Convert a size in number of bytes into a size in
// number of pages, rounding up.
//
min::unsptr MACC::page_size;
inline min::unsptr number_of_pages
    ( min::unsptr number_of_bytes )
{
    return   ( number_of_bytes + MACC::page_size - 1 )
           / MACC::page_size;
}

// Get and set parameter.  Return false if nothing
// done and true if parameter set.  Announce error
// and exit program if parameter too small, too
// large, not a multiple of `unit', or not a power
// of two when that is required.
//
static bool get_param
	( const char * name, min::int64 & parameter,
	  min::int64 minimum = 0,
	  min::int64 maximum = 0x7FFFFFFFFFFFFFFFll,
	  min::int64 unit = 1,
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
    if ( v % unit != 0 )
    {
        cout << "ERROR: " << name
	     << " program parameter " << v
	     << " not a multiple of " << unit << endl;
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
	  min::unsptr unit = 1,
	  bool power_of_two = false,
	  bool trace = trace_parameters )
{
    assert ( maximum <= 0x7FFFFFFFFFFFFFFFull );
    min::int64 v;
    if ( ! get_param ( name, v,
                       minimum, maximum,
		       unit, power_of_two, trace ) )
         return false;
    parameter = (min::unsptr) v;
    return true;
}

// Ditto but for `unsigned' parameter if `unsigned'
// not the same as `min::unsptr'.
//
#if MIN_PTR_BITS > 32
static bool get_param
	( const char * name, unsigned & parameter,
	  unsigned minimum = 0,
	  unsigned maximum = 0xFFFFFFFFu,
	  unsigned unit = 1,
	  bool power_of_two = false,
	  bool trace = trace_parameters )
{
    min::int64 v;
    if ( ! get_param ( name, v,
                       minimum, maximum,
		       unit, power_of_two, trace ) )
         return false;
    parameter = (unsigned) v;
    return true;
}
#endif

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
	    case 'f':
	        trace_fixed_block_allocation = true;
	    case 'v':
	        trace_variable_block_allocation = true;
		break;
	    case 'm':
	        MOS::trace_pools = 1;
		break;
	    case 'M':
	        MOS::trace_pools = 2;
		break;
	    case 'D':
	        MOS::trace_pools = 3;
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
    MACC::page_size = MOS::pagesize();

    stub_allocator_initializer();
    collector_initializer();
    block_allocator_initializer();

    MINT::acc_initialize_resize_body();
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
    if ( MOS::trace_pools >= 1 )
        cout << "TRACE: stub_allocator_initializer()"
	     << endl;

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

	    assert (   MIN_STUB_BASE
	             + MACC::page_size * pages
		     <=
	             MIN_MAX_ABSOLUTE_STUB_ADDRESS );

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
	        MINT::null_stub ) );

    MINT::first_allocated_stub = MACC::stub_next;
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
	MOS::dump_memory_layout ( cout );
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
min::unsptr MACC::deallocated_body_size;
min::unsptr MACC::subregion_size;
min::unsptr MACC::superregion_size;
min::unsptr MACC::max_paged_body_size;
min::unsptr MACC::paged_body_region_size;
min::unsptr MACC::stub_stack_region_size;
min::unsptr MACC::stub_stack_segment_size;

void * MACC::deallocated_body;

MACC::region * MACC::region_table;
MACC::region * MACC::region_next;
MACC::region * MACC::region_end;

MACC::region * MACC::last_free_region = NULL;
MACC::region * MACC::last_superregion = NULL;
MACC::region * MACC::current_superregion = NULL;
MACC::region * MACC::last_variable_body_region = NULL;
MACC::region * MACC::last_paged_body_region = NULL;
MACC::region * MACC::last_mono_body_region = NULL;
MACC::region * MACC::last_stub_stack_region = NULL;
MACC::region * MACC::current_stub_stack_region = NULL;

static MINT::fixed_block_list_extension
    fixed_block_extensions
	[MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG-2];

static void allocate_new_superregion ( void );

static void block_allocator_initializer ( void )
{
    if ( MOS::trace_pools >= 1 )
        cout << "TRACE: block_allocator_initializer()"
	     << endl;

    // Set parameters.

    get_param ( "space_factor",
                MACC::space_factor,
		4, 128, 1, true );

    min::unsptr F = MACC::space_factor;

    get_param ( "cache_line_size",
                MACC::cache_line_size,
		8, 4096, 1, true );

    MACC::subregion_size = F * F * MACC::page_size;
    get_param ( "subregion_size",
                MACC::subregion_size,
		4 * F * MACC::page_size,
		F * F * F * MACC::page_size,
		MACC::page_size );

    MACC::superregion_size = 64 * MACC::subregion_size;
    get_param ( "superregion_size",
                MACC::superregion_size,
		MACC::subregion_size,
		( MIN_PTR_BITS <= 32 ?
		  1 << 28 : 1ull << 44 ),
		MACC::subregion_size );

    MACC::max_paged_body_size = F * F * MACC::page_size;
    get_param ( "max_paged_body_size",
                MACC::max_paged_body_size,
		0,
		( MIN_PTR_BITS <= 32 ?
		  1 << 24 : 1ull << 32 ),
		MACC::page_size );

#   ifdef MIN_DEALLOCATED_BODY_SIZE
	MACC::deallocated_body_size =
	    MIN_DEALLOCATED_BODY_SIZE;
#   else
	MACC::deallocated_body_size =
	    MACC::max_paged_body_size;
#   endif
    get_param ( "deallocated_body_size",
                MACC::deallocated_body_size,
		1 << 20,
		( MIN_PTR_BITS <= 32 ?
		  1 << 30 : 1ull << 40 ),
		MACC::page_size );

    MACC::paged_body_region_size =
        F * MACC::max_paged_body_size;
    get_param ( "paged_body_region_size",
                MACC::paged_body_region_size,
		MACC::max_paged_body_size,
		( MIN_PTR_BITS <= 32 ?
		  1 << 28 : 1ull << 44 ),
		MACC::page_size );

    MACC::stub_stack_segment_size = 4 * MACC::page_size;
    get_param ( "stub_stack_segment_size",
                MACC::stub_stack_segment_size,
		MACC::page_size, 64 * MACC::page_size,
		MACC::page_size );

    MACC::stub_stack_region_size =
        16 * MACC::stub_stack_segment_size;
    get_param ( "stub_stack_region_size",
                MACC::stub_stack_region_size,
		MACC::stub_stack_segment_size,
		64 * MACC::stub_stack_segment_size,
		MACC::page_size );

    if (   MINT::max_fixed_block_size
         > F * MACC::page_size )
	MINT::max_fixed_block_size =
	    F * MACC::page_size;

    // Allocate MACC::deallocated_body.
    //
    {
	min::unsptr pages =
	    number_of_pages
	        ( MACC::deallocated_body_size );
	void * p = MOS::new_pool ( pages );

	const char * error = MOS::pool_error ( p );
	if ( error != NULL )
	{
	    cout << "ERROR: " << error << endl
		 << "       while allocating"
		    " deallocated body of size"
		 << MACC::deallocated_body_size
		 << " bytes"
		 << endl
		 << "       Suggest decreasing"
		    " deallocated_body_size."
		 << endl;
	    MOS::dump_error_info ( cout );
	    exit ( 1 );
	}
	MOS::inaccess_pool ( pages, p );
	MACC::deallocated_body = (void *)
	    ( (min::uns64 *) p + 1 );
    }

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

    MACC::region_table = (MACC::region *) rt;
    MACC::region_next = MACC::region_table + 1;
    MACC::region_end =
          MACC::region_table
	+ MACC::MAX_MULTI_PAGE_BLOCK_REGIONS;

    allocate_new_superregion();

    for ( unsigned j = 0;
          j < MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG-2;
	  ++ j )
    {
        MINT::fixed_blocks[j].extension =
	    fixed_block_extensions + j;
        fixed_block_extensions[j].fbl =
	    MINT::fixed_blocks + j;
        fixed_block_extensions[j].last_region = NULL;
        fixed_block_extensions[j].current_region = NULL;
    }
}

// Allocate a new non-sub region with the given
// number of pages.  Note that block_size and max_free_
// count are set to 0 by this function, and must be
// set by caller if they are used.
//
static MACC::region * new_paged_block_region
	( min::unsptr size, int type )
{
    if ( MOS::trace_pools >= 1 )
        cout << "TRACE: new_paged_block_region ("
	     << size << ", " << type << ")" << endl;

    // Round up to a multiple of the page size and
    // allocate.
    //
    min::unsptr pages = number_of_pages ( size );
    size = pages * MACC::page_size;
    void * m = MOS::new_pool ( pages );

    const char * error = MOS::pool_error ( m );
    if ( error != NULL ) return NULL;

    // Allocate region struct and fill it in.
    //
    if ( MACC::region_next == MACC::region_end )
    {
        cout << "ERROR: trying to allocate more than "
	     << (   MACC::region_end
	          - MACC::region_table )
	     << " paged block regions" << endl;
	exit ( 1 );
    }

    MACC::region * r = MACC::last_free_region;
    if ( r == NULL ) r = MACC::region_next ++;
    else remove ( MACC::last_free_region, r );

    r->block_control = 0;
    r->block_subcontrol = MUP::new_control_with_type
        ( type, size );
    r->begin = (min::uns8 *) m;
    r->next = r->begin;
    r->end = r->begin + size;
    r->round_size = MACC::page_size;
    r->round_mask = MACC::page_size - 1;
    r->block_size = 0;
    r->free_size = 0;
    r->free_count = 0;
    r->max_free_count = 0;
    r->region_previous = r;
    r->region_next = r;
    r->last_free = NULL;

    if ( MOS::trace_pools >= 1 )
        cout << "TRACE: new_paged_block_region"
	        " returns & region_table["
	     << r - MACC::region_table
	     << "]" << endl;

    return r;
}

// Free new paged block region.  Caller must first
// remove region from any list it is on.
//
static void free_paged_block_region
	( MACC::region * r )
{
    if ( MOS::trace_pools >= 1 )
        cout << "TRACE: free_paged_block_region"
	        " ( & region_table["
	     << r - MACC::region_table
	     << "] )" << endl;

    min::uns64 length = MACC::size_of ( r );
    assert ( length % MACC::page_size == 0 );
    min::uns64 pages = length / MACC::page_size;

    MOS::free_pool ( pages, r->begin );

    r->block_control = 0;
    r->block_subcontrol = MUP::new_control_with_type
        ( MACC::FREE, (min::uns64) 0 );
    r->begin = NULL;
    r->next = NULL;
    r->end = NULL;
    r->round_size = MACC::page_size;
    r->round_mask = MACC::page_size - 1;
    r->block_size = 0;
    r->free_size = 0;
    r->free_count = 0;
    r->last_free = NULL;

    assert ( r->region_previous == r->region_next );

    insert ( MACC::last_free_region, r );
}

// Call this when we are out of superregions.
//
static void allocate_new_superregion ( void )
{
    if ( MOS::trace_pools >= 1 )
        cout << "TRACE: allocate_new_superregion()"
	     << endl;

    // Allocate as new region.  If not the first
    // superregion, try downsizing if necessary.
    //
    MACC::region * r = new_paged_block_region
	( MACC::superregion_size,
	  MACC::SUPERREGION );
    if ( r == NULL && MACC::last_superregion != NULL )
	r = new_paged_block_region
	    ( 4 * MACC::subregion_size,
	      MACC::SUPERREGION );
    if ( r == NULL && MACC::last_superregion != NULL )
	r = new_paged_block_region
	    ( MACC::subregion_size,
	      MACC::SUPERREGION );

    if ( r == NULL )
    {
        if ( MACC::last_superregion == NULL )
	    cout << "ERROR: could not allocate "
		 << ( MACC::superregion_size
		      / MACC::page_size )
		 << " page initial superregion."
		 << endl;
	else
	    cout << "ERROR: out of virtual memory"
	            " trying to allocate "
		 << ( MACC::subregion_size
		      / MACC::page_size )
	         << " page superregion." << endl;
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }
    r->round_size = MACC::page_size;
    r->round_mask = MACC::page_size - 1;
    r->block_size = MACC::subregion_size;
    r->max_free_count = size_of ( r ) / r->block_size;

    MACC::insert ( MACC::last_superregion, r );
}

void MINT::new_fixed_body
    ( min::stub * s, min::unsptr n,
      MINT::fixed_block_list * fbl )
{
    MINT::fixed_block_list_extension * fblext =
        fbl->extension;

    // Use the current region if possible.  Otherwise
    // search regions oldest first for new current
    // region.
    //
    MACC::region * r = fblext->current_region;
    if ( r != NULL
         &&
	 r->free_count == 0
	 &&
	 r->next == r->end )
        for ( r = fblext->last_region->region_next;
	      r != NULL; )
	{
	    if ( r->free_count > 0 ) break;
	    if ( r->next  < r->end ) break;
	    if ( r == fblext->last_region ) r = NULL;
	    else r = r->region_next;
	}

    if ( r == NULL )
    {
        // No fixed size block region with the given
	// block size has any free blocks.  Must
	// allocate a new subregion.

	MACC::region * sr = MACC::current_superregion;

	if ( sr == NULL
	     ||
	     sr->free_count > 0
	     ||
	     sr->next < sr->end )
	    ; // do nothing
	else for ( sr = MACC::last_superregion; ; )
	{
	    sr = sr->region_next;
	    if ( sr->free_count > 0
		 ||
		 sr->next < sr->end )
		break;
	    else if ( sr == MACC::last_superregion )
	    {
	        sr = NULL;
		break;
	    }
	}

	if ( sr == NULL )
	{
	    allocate_new_superregion();
	    sr = MACC::current_superregion
	       = MACC::last_superregion;
	}

	if ( sr->free_count > 0 )
	{
	    MINT::free_fixed_size_block * b =
	        sr->last_free->next;
	    r = (MACC::region *) b;
	    sr->last_free->next = b->next;
	    if ( -- sr->free_count == 0 )
	        sr->last_free = NULL;
	}
	else
	{
	    assert ( sr->next < sr->end );
	    r = (MACC::region *) sr->next;
	    sr->next += MACC::subregion_size;
	}

	// Fill in region struct at beginning of new
	// region.

	r->block_control =
	    MUP::new_control_with_locator
	        ( sr - MACC::region_table,
		  MINT::null_stub );
	r->block_subcontrol =
	    MUP::new_control_with_type
	        ( MACC::FIXED_SIZE_BLOCK_REGION,
		  MACC::subregion_size );

	// Set the region to begin at the next block
	// sized boundary if this is <= the cache line
	// size, or at the next cache line boundary
	// otherwise.  Set the end to the maximum
	// integral number of block sizes after the
	// beginning in the region.
	//
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

	r->round_size = fbl->size;
	r->round_mask = fbl->size - 1;
	r->block_size = fbl->size;
	r->free_count = 0;
	r->last_free = NULL;

	if ( MOS::trace_pools >= 1 )
	    cout << "TRACE: allocating new subregion"
	            " for " << fbl->size
		 << " byte blocks" << endl;

	// Insert the region at the end of the fixed
	// size region list of the given block size.
	//
	r->region_previous = r->region_next = r;
	MACC::insert ( fblext->last_region, r );
    }

    // Make the found region current.
    //
    fblext->current_region = r;

    if ( r->free_count > 0 )
    {
        // Move region free list to fbl.
	//
        fbl->count = r->free_count;
	fbl->last_free = r->last_free;
	r->free_count = 0;
	r->last_free = NULL;
    }
    else
    {
	// Add up to 16 pages worth of bodies to the
	// fbl free list.
	//
    	min::unsptr count = 16 * MACC::page_size
	                  / fbl->size;
	if ( count == 0 ) count = 1;
	MINT::free_fixed_size_block * first = NULL;
	assert ( fbl->last_free == NULL );
	while ( count -- > 0 )
	{
	    if ( r->next + fbl->size > r->end )
	    {
		assert ( r->next == r->end );
		break;
	    }

	    MINT::free_fixed_size_block * b =
	        (MINT::free_fixed_size_block *) r->next;
	    int locator = - (int)
	        (   ( r->next - (min::uns8 *) r )
		  / MACC::page_size );
	    b->block_control =
	        MUP::new_control_with_locator
		    ( locator, MINT::null_stub );

	    if ( first == NULL )
	        first = b;
	    else
	        fbl->last_free->next = b;
	    fbl->last_free = b;

	    r->next += fbl->size;
	    ++ fbl->count;
	}
	assert ( fbl->last_free != NULL );
	fbl->last_free->next = first;
    }

    assert ( fbl->count > 0 );

    // Allocate first block from refurbished fbl free
    // list.
    //
    MINT::free_fixed_size_block * b =
        fbl->last_free->next;
    fbl->last_free->next = b->next;
    if ( -- fbl->count == 0 )
        fbl->last_free = NULL;

    b->block_control = MUP::renew_control_stub
        ( b->block_control, s );
    MUP::set_ptr_of ( s, & b->block_control + 1 );
    MUP::set_flags_of ( s, ACC_FIXED_BODY_FLAG );
}

// Call this when we are out of paged body regions.
//
static void allocate_new_paged_body_region ( void )
{
    MACC::region * r = new_paged_block_region
	( MACC::paged_body_region_size,
	  MACC::PAGED_BODY_REGION );
    if ( r == NULL )
	r = new_paged_block_region
		( 4 * MACC::max_paged_body_size,
		  MACC::PAGED_BODY_REGION );
    if ( r == NULL )
	r = new_paged_block_region
	        ( MACC::max_paged_body_size,
		  MACC::PAGED_BODY_REGION );
    if ( r == NULL )
    {
	cout << "ERROR: out of virtual memory"
		" trying to allocate "
	     << (   MACC::max_paged_body_size
		  / MACC::page_size )
	     << " page paged body region." << endl;
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }

    r->block_size = MACC::max_paged_body_size;

    MACC::insert ( MACC::last_paged_body_region, r );
}

// Execute new_non_fixed_body for bodies of size at most
// MACC::max_paged_body_size.  n is size of body + body
// control rounded up to a multiple of MACC::page_size.
// n should be larger than MINT::max_fixed_block_size.
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
	MIN_ASSERT ( r->next + n <= r->end );
    }


    min::uns64 * b = (min::uns64 *) r->next;
    r->next += n;

    int locator = r - MACC::region_table;
    * b = MUP::new_control_with_locator
	    ( locator, s );

    return b + 1;
}

// Execute new_non_fixed_body for bodies of size greater
// than MACC::max_paged_body_size.  n is size of body
// + body control rounded up to a multiple of MACC::
// page_size.  Return address to store in stub.
//
inline void * new_mono_body
    ( min::stub * s, min::unsptr n )
{
    MACC::region * r = new_paged_block_region
	( n, MACC::MONO_BODY_REGION );
    if ( r == NULL )
    {
	cout << "ERROR: out of virtual memory"
		" trying to allocate "
	     << ( n / MACC::page_size )
	     << " page mono body region." << endl;
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }

    MACC::insert ( MACC::last_mono_body_region, r );

    min::uns64 * b = (min::uns64 *) r->next;
    r->next += n;
    assert ( r->next <= r->end );

    int locator = r - MACC::region_table;
    * b = MUP::new_control_with_locator
	    ( locator, s );

    return b + 1;
}

void MINT::new_non_fixed_body
    ( min::stub * s, min::unsptr n )
{
    // Add space for body control and round up to
    // multiple of MACC::page_size.
    //
    n += sizeof ( min::uns64 );
    n = MACC::page_size
      * (   ( n + MACC::page_size - 1 )
          / MACC::page_size );

    void * body =
	n <= MACC::max_paged_body_size ?
	    new_paged_body ( s, n ) :
	    new_mono_body ( s, n );

    MUP::set_ptr_of ( s, body );
}

void MUP::deallocate_body
    ( min::stub * s, min::unsptr n )
{
    if ( n == 0 ) return;

    min::uns64 * bp =
        (min::uns64 *) MUP::ptr_of ( s ) - 1;
    MACC::region * r = MACC::region_of_body ( bp );
    assert ( s == MACC::stub_of_body ( bp ) );

    * bp = MUP::renew_control_stub
		( * bp, MINT::null_stub );

    int type = MACC::type_of ( r );
    if ( type == MACC::FIXED_SIZE_BLOCK_REGION )
    {
	MINT::free_fixed_size_block * p =
	    (MINT::free_fixed_size_block *) bp;
	if ( r->last_free == NULL )
	    r->last_free = p->next = p;
	else
	{
	    p->next = r->last_free->next;
	    r->last_free->next = p;
	    r->last_free = p;
	}
	++ r->free_count;
	MUP::clear_flags_of
	    ( s, MINT::ACC_FIXED_BODY_FLAG );
    }
    else
    {
	MACC::free_variable_size_block * p =
	    (MACC::free_variable_size_block *) bp;
	n = ( n + 8 + r->round_mask ) & ~ r->round_mask;
	p->block_subcontrol =
	    MUP::new_control_with_type
	        ( MACC::FREE, n );
	r->free_size += n;
    }

    MUP::set_ptr_of ( s, MACC::deallocated_body );
    MUP::set_type_of ( s, min::DEALLOCATED );
}

// Packed Type Allocator
// ------ ---- ---------

// MINT::packed_types points at this word which points
// at the packed types vector.
//
static void ** packed_types_p;

void MINT::allocate_packed_types ( min::uns32 count )
{
    min::unsptr new_pages =
        number_of_pages ( count * sizeof ( void * ) );
    void ** new_packed_types = (void **)
        MOS::new_pool ( new_pages );
    if ( MINT::max_packed_type_count != 0 )
    {
        min::unsptr old_pages =
	    number_of_pages
	        (   MINT::max_packed_type_count
		  * sizeof ( void * ) );
	memcpy ( new_packed_types, * packed_types_p,
	           MINT::packed_type_count
		 * sizeof ( void * ) );
	MOS::free_pool ( old_pages, * packed_types_p );
    }
    packed_types_p = new_packed_types;
    MINT::max_packed_type_count = count;
    MINT::packed_types = & ::packed_types_p;
}

// Stub Stack Manager
// ---- ----- -------


void MACC::stub_stack::rewind ( void )
{
    if ( last_segment == NULL )
    {
	input = output = NULL;
	input_segment = output_segment = NULL;
	is_at_end = true;
    }
    else
    {
	input_segment =
	    last_segment->next_segment;
	input = input_segment->begin;
	output_segment = input_segment;
	output = input;
	is_at_end =
	    ( input_segment == last_segment
	      &&
	      input == input_segment->next );
    }
}

// Called by push() when we are at the end of the
// current segment, or there is no current segment
// (stack has no segments yet).  Allocates another
// segment for the stack.
//
void MACC::stub_stack
         ::allocate_stub_stack_segment ( void )
{
    region * r = current_stub_stack_region;
    if ( r != NULL
         &&
	 r->free_count == 0
	 &&
	 r->next == r->end )
    {
        // Current stack region exists and has no free
	// segments or room to allocate a new segment.

	r = last_stub_stack_region->region_next;
	while ( r->free_count == 0
		&&
		r != last_stub_stack_region )
	{
	    assert ( r->next == r->end );
	    r = r->region_next;
	}

	if ( r->free_count == 0
	     &&
	     r->next == r->end )
	    r = NULL;
	else
	    current_stub_stack_region = r;
    }

    if ( r == NULL )
    {
        // No existing stack region has any free
	// segments or room to allocate a new segment.

	r = new_paged_block_region
		( stub_stack_region_size,
		  STUB_STACK_REGION );
	if ( r == NULL )
	{
	    cout << "ERROR: out of virtual"
		    " memory while attempting to"
		    " allocate a stub stack"
		    " region." << endl;
	    MOS::dump_error_info ( cout );
	    exit ( 1 );
	}

	r->round_size = MACC::page_size;
	r->round_mask = MACC::page_size - 1;
	r->block_size = MACC::stub_stack_segment_size;
	r->max_free_count =
	    size_of ( r ) / r->block_size;

	insert ( last_stub_stack_region, r );
	current_stub_stack_region = r;
    }

    // Now r has a free segment or room to allocate
    // a new segment.

    stub_stack_segment * sss;
    if ( r->free_count > 0 )
    {
        MINT::free_fixed_size_block * b =
	    r->last_free->next;
	r->last_free->next = b->next;
	if ( -- r->free_count == 0 )
	    r->last_free = NULL;
	sss = (stub_stack_segment *) b;
    }
    else
    {
        sss = (stub_stack_segment *) r->next;
	r->next += stub_stack_segment_size;
	assert ( r->next <= r->end );
    }

    sss->block_control = MUP::new_control_with_locator
        ( r - region_table, MINT::null_stub );
    sss->block_subcontrol = MUP::new_control_with_type
        ( STUB_STACK_SEGMENT, stub_stack_segment_size );

    sss->next = sss->begin;
    sss->end =
        (min::stub **)
        ( (uns8 *) sss + stub_stack_segment_size );

    if ( last_segment == NULL )
    {
        sss->previous_segment = sss->next_segment = sss;
	last_segment = sss;
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

void MACC::stub_stack::remove_jump ( void )
{
    if ( input_segment == last_segment )
	is_at_end = true;
    else
    {
	input_segment =
	    input_segment->next_segment;
	input = input_segment->begin;

	is_at_end =
	    ( input == input_segment->next );

	if (    output_segment
	     != input_segment->previous_segment )
	{
	    // Remove segment we just left.
	    //
	    MACC::stub_stack_segment * sss =
	        input_segment->previous_segment;
	    assert ( sss != sss->next_segment );
	    sss->next_segment->previous_segment =
	        sss->previous_segment;
	    sss->previous_segment->next_segment =
	        sss->next_segment;

	    MINT::free_fixed_size_block * b =
		(MINT::free_fixed_size_block *) sss;
	    b->next = NULL;

	    int locator = MUP::locator_of_control
		( sss->block_control );
	    assert ( locator > 0 );
	    region * r = & region_table[locator];
	    if ( r->free_count ++ == 0 )
		r->last_free = b->next = b;
	    else
	    {
		b->next = r->last_free->next;
		r->last_free->next = b;
		r->last_free = b;
	    }

	    if ( r->free_count == r->max_free_count )
	    {
		// If all segments in r are free,
		// free r.  But not if r is the current
		// stub stack region, in order to
		// prevent thrashing.
		//
		if ( r != MACC::
		            current_stub_stack_region )
		    MACC::remove
		        ( MACC::last_stub_stack_region,
			  r );
	    }
	}
    }
}

void MACC::stub_stack::flush ( void )
{
    if ( output_segment == NULL ) return;

    if ( output == output_segment->begin )
    {
	if ( output_segment
	     ==
	     output_segment->previous_segment )
	    output_segment = NULL;
	else
	{
	    output_segment =
		output_segment->previous_segment;
	    output = output_segment->next;
	}
    }

    // Note: if we are to empty the stack at this
    // point output_segment == NULL.

    while ( output_segment != last_segment )
    {
        stub_stack_segment * sss = last_segment;
	if ( sss->next_segment == sss )
	    last_segment = NULL;
	else
	{
	    last_segment = sss->previous_segment;

	    sss->previous_segment->next_segment =
		sss->next_segment;
	    sss->next_segment->previous_segment =
		sss->previous_segment;
	}

	MINT::free_fixed_size_block * b =
	    (MINT::free_fixed_size_block *) sss;
	b->next = NULL;

	int locator = MUP::locator_of_control
	    ( sss->block_control );
	assert ( locator > 0 );
	region * r = & region_table[locator];
	if ( r->free_count ++ == 0 )
	    r->last_free = b->next = b;
	else
	{
	    b->next = r->last_free->next;
	    r->last_free->next = b;
	    r->last_free = b;
	}

	if ( r->free_count == r->max_free_count )
	{
	    // If all segments in r are free,
	    // free r.  But not if r is the current
	    // stub stack region, in order to
	    // prevent thrashing.
	    //
	    if ( r != MACC::
			current_stub_stack_region )
		MACC::remove
		    ( MACC::last_stub_stack_region, r );
	}
    }

    if ( output_segment != NULL )
        output_segment->next = output;

    rewind();
}


// Collector
// ---------

unsigned MACC::ephemeral_levels =
    MIN_DEFAULT_EPHEMERAL_LEVELS;
static unsigned ephemeral_sublevels
                    [MIN_MAX_EPHEMERAL_LEVELS];
unsigned * MACC::ephemeral_sublevels =
    ::ephemeral_sublevels;

static const unsigned MAX_GENERATIONS =
    1 +   MIN_MAX_EPHEMERAL_LEVELS
        * MIN_MAX_EPHEMERAL_SUBLEVELS;
static MACC::generation generations[MAX_GENERATIONS+1];
MACC::generation * MACC::generations = ::generations;
MACC::generation * MACC::end_g;

static const unsigned MAX_LEVELS =
    1 + MIN_MAX_EPHEMERAL_LEVELS;
static MACC::level levels_vector[MAX_GENERATIONS];
MACC::level * MACC::levels = levels_vector;

min::unsptr  MACC::acc_stack_size;
min::stub ** MACC::acc_stack_begin;
min::stub ** MACC::acc_stack_end;
min::uns64   MACC::acc_stack_scavenge_mask;

min::uns64 MACC::scan_limit;
min::uns64 MACC::scavenge_limit;
min::uns64 MACC::collection_limit;

static void collector_initializer ( void )
{
    get_param ( "ephemeral_levels",
                MACC::ephemeral_levels,
		0, MIN_MAX_EPHEMERAL_LEVELS );

    MACC::generation * g = MACC::generations;

    levels[0].g = g ++;
    levels[0].number_of_sublevels = 1;

    char name[80];

    MINT::new_acc_stub_flags = 0;
    for ( unsigned i = 1;
          i < MACC::ephemeral_levels; ++ i )
    {
	MINT::new_acc_stub_flags |= COLLECTIBLE ( i );

        sprintf ( name, "ephemeral_sublevels[%d]", i );
	MACC::ephemeral_sublevels[i] =
	    MIN_DEFAULT_EPHEMERAL_SUBLEVELS;
	get_param ( name,
	            MACC::ephemeral_sublevels[i],
		    1, MIN_MAX_EPHEMERAL_SUBLEVELS );

	levels[i].g = g;
	levels[i].number_of_sublevels =
	    MACC::ephemeral_sublevels[i];

	for ( unsigned j = 0;
	      j < MACC::ephemeral_sublevels[i]; ++ j )
	{
	    g->level = & levels[i];
	    g->last_before = MINT::first_allocated_stub;
	    g->count = 0;
	    g->lock = false;

	    ++ g;
	}
	MACC::end_g = g;
	MACC::end_g->lock = false;
    }

    MACC::acc_stack_size =
          MACC::page_size
	* number_of_pages
	      ( MIN_DEFAULT_ACC_STACK_SIZE );
    get_param ( "acc_stack_size",
                MACC::acc_stack_size,
		1 << 12, 1 << 24, MACC::page_size );
    min::unsptr np =
        number_of_pages ( MACC::acc_stack_size );
    MACC::acc_stack_begin = (min::stub **)
        MOS::new_pool ( np + 1 );
    const char * error =
        MOS::pool_error ( MACC::acc_stack_begin );
    if ( error != NULL )
    {
        cout << "ERROR: " << error << endl
	     << "       while allocating " << np
	     << "page acc stack."
	     << endl
	     << "       Suggest decreasing"
	        " acc_stack_size."
	     << endl;
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }
    MACC::acc_stack_end = (min::stub **)
        (   (min::uns8 *) MACC::acc_stack_begin
	  + ( np - 1 ) * MACC::page_size );
    MOS::inaccess_pool ( 1, MACC::acc_stack_end );
        // Allocate an inaccessible page after the
	// acc stack to catch overflows.

    MACC::scan_limit = MIN_DEFAULT_ACC_SCAN_LIMIT;
    get_param ( "scan_limit",
                MACC::scan_limit,
		100, 1 << 30 );

    MACC::scavenge_limit =
        MIN_DEFAULT_ACC_SCAVENGE_LIMIT;
    get_param ( "scavenge_limit",
                MACC::scavenge_limit,
		10, 1 << 30 );

    MACC::collection_limit =
        MIN_DEFAULT_ACC_COLLECTION_LIMIT;
    get_param ( "collection_limit",
                MACC::collection_limit,
		10, 1 << 30 );

}

void MACC::process_acc_stack ( min::stub ** acc_lower )
{
    unsigned const M = 56 - MIN_ACC_FLAG_BITS + 2;
    unsigned const E = MIN_MAX_EPHEMERAL_LEVELS;

    while ( MINT::acc_stack > acc_lower )
    {
	// Stub s1 contains a pointer to stub s2.
	//
        min::stub * s2 = * -- MINT::acc_stack;
        min::stub * s1 = * -- MINT::acc_stack;

        uns64 f1 = (    min::unprotected
	                   ::control_of ( s1 )
	             >> MINT::ACC_FLAG_PAIRS )
	         & ( min::unprotected
		        ::control_of ( s2 ) )
		 & min::internal::acc_stack_mask;

	unsigned make_root =
	    (unsigned) ( f1 >> ( M - 1 ) );
	unsigned mark = make_root >> ( E + 1 );
	make_root &= ( ( ( 1 << E ) - 1 ) << 1 );

	while ( mark != 0 )
	{
	    unsigned level =
	        MINT::log2floor ( mark );
	    mark ^= 1 << level;
	    levels[level].to_be_scavenged.push ( s2 );
	    MUP::clear_flags_of
	        ( s2, UNMARKED ( level ) );
	}
	while ( make_root != 0 )
	{
	    unsigned level =
	        MINT::log2floor ( make_root );
	    make_root ^= 1 << level;
	    MACC::level & lev = levels[level];
	    lev.root.push ( s1 );
	    MUP::clear_flags_of
	        ( s1, NON_ROOT ( level ) );

	    if ( MACC::acc_stack_scavenge_mask
	         & COLLECTIBLE ( level ) )
	    {
	        // During scavenging, new roots are
		// marked scavenged.  This is an
		// efficiency measure, as new roots
		// only contain a single pointer to
		// a collectible stub.
		//
		MUP::set_flags_of
		    ( s1, SCAVENGED ( level ) );
		if ( MUP::test_flags_of
		            ( s2, UNMARKED ( level ) ) )
		{
		    MUP::clear_flags_of
			( s2, UNMARKED ( level ) );
		    int type = min::type_of ( s2 );
		    assert ( type >= 0 );
		    if ( MINT::scavenger_routines[type]
		         != NULL )
			lev.to_be_scavenged.push ( s2 );
		}
	    }
	    else
	    {
	        // Stubs of level L < level have their
		// SCAVENGED ( level ) flags clear
		// unless they are on the level L root
		// list.
		//
	        assert
		    ( ! MUP::test_flags_of
			  ( s1, SCAVENGED ( level ) ) );
	    }
	}
    }
}

// Helper function that removes a stub s with control c
// from any aux hash table it might be in.
//
inline void remove_from_aux_hash_table
	( min::uns64 c, min::stub * s )
{

    int t = MUP::type_of_control ( c );
    min::stub ** head = NULL;
    min::uns32 h;
    switch ( t )
    {
#   if MIN_IS_COMPACT
	case min::NUMBER:
	    h = min::floathash
		    ( MUP::float_of ( s ) );
	    h &= MINT::num_hash_mask;
	    head = MINT::num_aux_hash + h;
	    break;
#   endif
    case min::SHORT_STR:
	h = min::strnhash ( s->v.c8, 8 );
	h &= MINT::str_hash_mask;
	head = MINT::str_aux_hash + h;
	break;
    case min::LONG_STR:
	h = MUP::hash_of ( MUP::long_str_of ( s ) );
	h &= MINT::str_hash_mask;
	head = MINT::str_aux_hash + h;
	break;
    case min::LABEL:
	h = min::labhash ( s );
	h &= MINT::lab_hash_mask;
	head = MINT::lab_aux_hash + h;
	break;
    default:
	return;
    }

    MINT::remove_aux_hash ( head, s );
}

// Perform one increment of the current collector execu-
// tion.  To start a collector execution just set the
// level state to COLLECTOR_START.  The collector
// execution is finished when it returns with the level
// collector_state set to COLLECTOR_NOT_RUNNING.
//
// Returns `false' if the collector increment could not
// start because it was locked out by the generation
// struct or level struct locks.
//
static bool collector_increment ( unsigned level )
{

    MACC::level & lev = levels[level];

    switch ( lev.collector_state )
    {
    case COLLECTOR_START:
    	{
	    if ( lev.lock ) return false;
	    lev.lock = true;

	    MINT::new_acc_stub_flags |=
	        UNMARKED ( level );

	    // Check that to-be-scavenged stack is
	    // empty.
	    //
	    lev.to_be_scavenged.rewind();
	    assert ( lev.to_be_scavenged.at_end() );

	    // Check that flags are properly cleared.
	    //
	    assert (    ( MINT::acc_stack_mask
	                  & UNMARKED ( level ) )
		     == 0 );
	    assert (    ( MACC::acc_stack_scavenge_mask
	                  & SCAVENGED ( level ) )
		     == 0 );
	    assert (    ( MACC::removal_request_flags
	                  & UNMARKED ( level ) )
		     == 0 );
	    assert (    ( MINT::hash_acc_clear_flags
	                  & UNMARKED ( level ) )
		     == 0 );

	    lev.first_g = NULL;
	    lev.collector_state =
		PRE_INITING_COLLECTIBLE;
	}
	//
	// Fall throught to next collector_state.

    case PRE_INITING_COLLECTIBLE:
	{
	    if ( lev.first_g == NULL )
	    {
	        if ( lev.g->lock ) return false;
		lev.g->lock = true;
		lev.first_g = lev.last_g = lev.g;
		lev.last_stub = lev.g->last_before;
	    }

	    assert ( lev.first_g == lev.last_g );
	    if ( lev.last_g[1].lock ) return false;

	    lev.last_g[1].lock = true;
	    ++ lev.last_g;

	    if ( lev.last_g == end_g )
		end_g->last_before =
		    MINT::last_allocated_stub;

	    lev.collector_state = INITING_COLLECTIBLE;
	}

    case INITING_COLLECTIBLE:
        {
	    assert ( lev.first_g + 1 == lev.last_g );

	    min::uns64 scanned = 0;
	    min::uns64 c =
		MUP::control_of ( lev.last_stub );

	    while ( scanned < MACC::scan_limit )
	    {
		if (    lev.last_stub
		     == lev.last_g->last_before )
		{
		    lev.first_g->lock = false;
		    lev.first_g = lev.last_g;

		    if ( lev.last_g == end_g )
		    {
		        lev.last_g->lock = false;
			if ( level == 0 )
			{
			    lev.hash_table_id = 0;
			    lev.hash_table_index = 0;
			    lev.collector_state =
				INITING_HASH;
			}
			else
			{
			    lev.root.rewind();
			    lev.collector_state =
				INITING_ROOT;
			}
			break;
		    }
		    else if ( lev.last_g[1].lock )
		    {
			lev.collector_state =
			    PRE_INITING_COLLECTIBLE;
			break;
		    }
		    else
		    {
			lev.last_g[1].lock = true;
			++ lev.last_g;
			if ( lev.last_g == end_g )
			    end_g->last_before =
				MINT::
				  last_allocated_stub;
		    }
		}

		min::stub * s =
		    MUP::stub_of_acc_control ( c );
		c = MUP::control_of ( s );
		c |= UNMARKED ( level );
		c &= ~ SCAVENGED ( level );
		MUP::set_control_of ( s, c );
		lev.last_stub = s;
		++ scanned;
	    }

	    lev.collectible_init_count += scanned;
	}
        break;

    case INITING_HASH:
        {
	    min::uns64 scanned = 0;

	    min::stub ** hash_table = NULL;
	    min::uns32 hash_table_size;
	    while ( scanned < MACC::scan_limit )
	    {
	        if ( hash_table == NULL )
		    switch ( lev.hash_table_id )
		{
		case 0:
		    hash_table =
			MINT::str_acc_hash;
		    hash_table_size =
			MINT::str_hash_size;
		    break;
		case 1:
		    hash_table =
			MINT::lab_acc_hash;
		    hash_table_size =
			MINT::lab_hash_size;
		    break;
#		if MIN_IS_COMPACT
		    case 2:
			hash_table =
			    MINT::num_acc_hash;
			hash_table_size =
			    MINT::num_hash_size;
			break;
#		endif
		}

		if ( hash_table == NULL )
		{
		    lev.root.rewind();
		    lev.collector_state = INITING_ROOT;
		    break;
		}

		if (    lev.hash_table_index
		     >= hash_table_size )
		{
		    ++ lev.hash_table_id;
		    hash_table = NULL;
		    continue;
		}

		min::stub * s =
		    hash_table [lev.hash_table_index++];
		while ( s != MINT::null_stub )
		{
		    min::uns64 c =
		        MUP::control_of ( s );
		    c |= UNMARKED ( level );
		    c &= ~ SCAVENGED ( level );
		    MUP::set_control_of ( s, c );
		    s = MUP::stub_of_acc_control ( c );
		    ++ scanned;
		}
	    }

	    lev.hash_init_count += scanned;
	}
	break;

    case INITING_ROOT:
	{
	    min::uns64 scanned = 0;

	    while ( ! lev.root.at_end()
	            &&
		    scanned < MACC::scan_limit )
	    {
	        min::stub * s = lev.root.current();
		lev.root.next();
		++ scanned;
		MUP::clear_flags_of
		    ( s, SCAVENGED ( level ) );
	    }

	    lev.root_init_count += scanned;

	    if ( lev.root.at_end() )
	    {
		lev.root.rewind();
		MINT::scavenge_control & sc =
		    MINT::scavenge_controls[level];
		sc.state = 0;
		sc.stub_flag = UNMARKED ( level );
		sc.gen_limit = MACC::scan_limit;
		MINT::acc_stack_mask |=
		    UNMARKED ( level );
	        MACC::acc_stack_scavenge_mask |=
		    SCAVENGED ( level );
		lev.collector_state =
		    SCAVENGING_ROOT;
		break;
	    }
	}
	break;

    case SCAVENGING_ROOT:
        {
	    MINT::scavenge_control & sc =
		MINT::scavenge_controls[level];

	    sc.gen_count = 0;
	    sc.stub_count = 0;
	    lev.to_be_scavenged.begin_push
	        ( sc.to_be_scavenged,
	          sc.to_be_scavenged_limit );
	    min::uns64 scavenged = 0;

	    min::uns64 remove =
	        MACC::removal_request_flags;
	    min::uns64 c;
	    while ( true )
	    {
		if ( sc.state == 0 )
		{
		    if (    scavenged
		         >= MACC::scavenge_limit )
		        break;
		    else if ( ! lev.to_be_scavenged
		                   .at_end() )
		    {
		        sc.s1 = lev.to_be_scavenged
			           .current();
		        lev.to_be_scavenged.remove();
			c = MUP::control_of ( sc.s1 );
			if ( c & remove ) continue;

			lev.root_scavenge = false;
			c |= SCAVENGED ( level );
		    }
		    else if ( ! lev.root.at_end() )
		    {
			sc.stub_flag_accumulator = 0;
		        sc.s1 = lev.root.current();
			c = MUP::control_of ( sc.s1 );
			if ( c & remove )
			{
			    lev.root.remove();
			    c |= NON_ROOT ( level );
			    c &= ~ SCAVENGED ( level );
			    MUP::set_control_of
			        ( sc.s1, c );
			    continue;
			}

			if ( c & SCAVENGED ( level ) )
			{
			    lev.collector_state =
			        SCAVENGING_THREAD;
			    break;
			}

			lev.root_scavenge = true;
			c |= SCAVENGED ( level )
			     |
			     NON_ROOT ( level );
		    }
		    else
		    {
		        lev.collector_state =
			    SCAVENGING_THREAD;
			break;
		    }
		}
		MUP::set_control_of ( sc.s1, c );

		int type = MUP::type_of_control ( c );
		assert ( type >= 0 );
		MINT::scavenger_routine scav =
		    MINT::scavenger_routines[type];
		assert ( scav != NULL );
		(* scav) ( sc );

		if ( sc.state != 0 ) break;

		++ scavenged;
		if ( lev.root_scavenge )
		{
		    if ( ! MUP::test_flags_of
		               ( sc.s1,
			         NON_ROOT ( level ) ) )
		        lev.root.keep();
		    else if ( sc.stub_flag_accumulator
		              &
			      COLLECTIBLE ( level ) )
		    {
		        MUP::clear_flags_of
			    ( sc.s1,
			      NON_ROOT ( level ) );
			lev.root.keep();
		    }
		    else
		    {
		        MUP::clear_flags_of
			    ( sc.s1,
			      SCAVENGED ( level ) );
			lev.root.remove();
		    }
		}
	    }

	    lev.to_be_scavenged.end_push
		( sc.to_be_scavenged );

	    lev.stub_scanned_count += sc.stub_count;
	    lev.scanned_count += sc.gen_count;
	    lev.scavenged_count += scavenged;

	    if (    lev.collector_state
	         == SCAVENGING_THREAD )
	    {
	        lev.restart_count = 0;
	        lev.scavenge_limit =
		    MACC::scavenge_limit;
	    }
	}
	break;

    case SCAVENGING_THREAD:
        {
	    MINT::scavenge_control & sc =
		MINT::scavenge_controls[level];
	    sc.gen_count = 0;
	    sc.stub_count = 0;
	    lev.to_be_scavenged.begin_push
	        ( sc.to_be_scavenged,
	          sc.to_be_scavenged_limit );
	    min::uns64 scavenged = 0;

	    bool thread_scavenged = false;
	    min::uns64 remove =
	        MACC::removal_request_flags;
	    while ( true )
	    {
		min::uns64 c;
		if ( sc.state == 0 )
		{
		    if ( ! lev.to_be_scavenged
		              .at_end() )
		    {
			if (    scavenged
			     >= lev.scavenge_limit )
			    break;

		        sc.s1 = lev.to_be_scavenged
			           .current();
		        lev.to_be_scavenged.remove();
			c = MUP::control_of ( sc.s1 );
			if ( c & remove )
			    continue;

			c |= SCAVENGED ( level );
		    }
		    else if ( ! thread_scavenged )
		    {
			min::uns32 init_gen_count =
			    sc.gen_count;
			MINT::thread_scavenger_routine
			    ( sc );

			if ( sc.state != 0 )
			{
			    // Thread scavenger ran out
			    // of to-be-scavenged  list.
			    //
			    sc.state = 0;
			    if (   lev.restart_count
			         < 10 )
				break;

			    // If we are thrashing,
			    // expand the to_be_
			    // scavenged list.
			    //
			    lev.to_be_scavenged.end_push
				( sc.to_be_scavenged );
			    lev.to_be_scavenged
			       .begin_push
				( sc.to_be_scavenged,
				  sc
				  .to_be_scavenged_limit
				);
			    continue;
			}

			thread_scavenged = true;

			if ( lev.restart_count == 0 )
			    sc.gen_limit +=
			          sc.gen_count
				- init_gen_count;
			else if ( lev.restart_count
			          % 4 == 0 )
			{
			    sc.gen_limit *= 2;
			    lev.scavenge_limit *= 2;
			}
			++ lev.restart_count;

			continue;
		    }
		    else
		    {
		        // Scavenging is done.

			MINT::new_acc_stub_flags &=
			    ~ UNMARKED ( level );
			MACC::removal_request_flags |=
			    UNMARKED ( level );
			MINT::hash_acc_clear_flags |=
			    UNMARKED ( level );
			MINT::acc_stack_mask &=
			    ~ UNMARKED ( level );
			MACC::acc_stack_scavenge_mask &=
			    ~ SCAVENGED ( level );

			// Allocate as stub to mark the
			// end of level L collected
			// stubs.  ACC_MARK stubs are
			// not collected if they are in
			// the last generation, and
			// during promotion, they serve
			// to delimit the last genera-
			// tion.
			//
			min::stub * s =
			    MUP::new_acc_stub();
			MUP::set_type_of
			    ( s, min::ACC_MARK );

			lev.first_g = NULL;
			if ( level == 0 )
			{
			    lev.hash_table_id = 0;
			    lev.hash_table_index = 0;
			    lev.collector_state =
				    COLLECTING_HASH;
			}
			else
			    lev.collector_state =
				    PRE_COLLECTING;
			break;
		    }
		}

		MUP::set_control_of ( sc.s1, c );
		int type = MUP::type_of_control ( c );
		assert ( type >= 0 );
		MINT::scavenger_routine scav =
		    MINT::scavenger_routines[type];
		assert ( scav != NULL);
		(* scav) ( sc );

		if ( sc.state != 0 ) break;

		++ scavenged;
	    }

	    lev.to_be_scavenged.end_push
		( sc.to_be_scavenged );
	    lev.stub_scanned_count += sc.stub_count;
	    lev.scanned_count += sc.gen_count;
	    lev.scavenged_count += scavenged;
	}
	break;

    case COLLECTING_HASH:
        {
	    min::uns64 collected = 0;
	    min::uns64 kept = 0;

	    min::stub ** hash_table = NULL;
	    min::uns32 hash_table_size;
	    while (   collected + kept
	            < MACC::collection_limit )
	    {
	        if ( hash_table == NULL )
		    switch ( lev.hash_table_id )
		{
		case 0:
		    hash_table =
			MINT::str_acc_hash;
		    hash_table_size =
			MINT::str_hash_size;
		    break;
		case 1:
		    hash_table =
			MINT::lab_acc_hash;
		    hash_table_size =
			MINT::lab_hash_size;
		    break;
#		if MIN_IS_COMPACT
		    case 2:
			hash_table =
			    MINT::num_acc_hash;
			hash_table_size =
			    MINT::num_hash_size;
			break;
#		endif
		}

		if ( hash_table == NULL )
		{
		    lev.collector_state =
			PRE_COLLECTING;
		    break;
		}

		if (    lev.hash_table_index
		     >= hash_table_size )
		{
		    ++ lev.hash_table_id;
		    hash_table = NULL;
		    continue;
		}

		min::stub * last_s = NULL;
		min::uns64 last_c;
		min::stub * s =
		    hash_table [lev.hash_table_index];
		while ( s != MINT::null_stub )
		{
		    min::uns64 c =
		        MUP::control_of ( s );
		    min::stub * next_s =
			MUP::stub_of_acc_control ( c );
		    if ( c & UNMARKED ( level ) )
		    {
			// Remove s from hash list.
			//
			if ( last_s != NULL )
			{
			    last_c =
			      MUP::
			        renew_acc_control_stub
				    ( last_c, next_s );
			    MUP::set_control_of
			      ( last_s, last_c );
			}
			else
			    hash_table
			        [lev.hash_table_index] =
				next_s;

			// Deallocate body of s.
			//
			min::unsptr size =
			    MUP::body_size_of ( s );
			if ( size != 0 )
			    MUP::deallocate_body
				( s, size );

			// Free stub s.
			//
			MINT::free_acc_stub ( s );
			++ collected;
		    }
		    else
		    {
		        last_c = c;
			last_s = s;
			++ kept;
		    }
		    s = next_s;
		}
		++ lev.hash_table_index;
	    }

	    lev.hash_collected_count += collected;
	    lev.hash_kept_count += kept;
	}
	break;

    case PRE_COLLECTING:
	{
	    if ( lev.first_g == NULL )
	    {
	        if ( lev.g->lock ) return false;
		lev.g->lock = true;
		lev.first_g = lev.last_g = lev.g;
		lev.last_stub = lev.g->last_before;
	    }

	    // We must make last_g > first_g and
	    //		last_g[1].last_before
	    //	     != last_g[0].last_before
	    //    unless last_g == end_g.

	    while ( lev.last_g != end_g
	            &&
		    ( lev.last_g == lev.first_g
		      ||
		         lev.last_g->last_before
		      == lev.last_g[1].last_before ) )
	    {
	        if ( lev.last_g[1].lock ) return false;
		lev.last_g[1].lock = true;
		++ lev.last_g;
	    }

	    lev.collector_state = COLLECTING;
	}

    case COLLECTING:
        {
	    end_g->last_before =
	        MINT::last_allocated_stub;

	    min::uns64 collected = 0;
	    min::uns64 kept = 0;

	    min::uns64 last_c =
	        MUP::control_of ( lev.last_stub );
	    while (   collected + kept
	            < MACC::collection_limit )
	    {
		if (    lev.last_stub
		     == lev.first_g[1].last_before )
		{
		    lev.first_g->lock = false;
		    ++ lev.first_g;
		    if ( lev.first_g == end_g )
		    {
			do lev.first_g->lock = false;
			while (    lev.first_g ++
			        != lev.last_g );

			lev.first_g = NULL;
			lev.collector_state =
			    PRE_PROMOTING;
			break;
		    }

		    // We must make last_g > first_g and
		    //		last_g[1].last_before
		    //	     != last_g[0].last_before
		    //    unless last_g == end_g.

		    while ( lev.last_g != end_g
			    &&
			    ( lev.last_g == lev.first_g
			      ||
				 lev.last_g->last_before
			      == lev.last_g[1]
			            .last_before ) )
		    {
			if ( lev.last_g[1].lock )
			{
			    lev.collector_state =
				PRE_INITING_COLLECTIBLE;
			    break;
			}
			lev.last_g[1].lock = true;
			++ lev.last_g;
		    }
		}

		min::stub * s =
		    MUP::stub_of_acc_control ( last_c );
		min::uns64 c = MUP::control_of ( s );
		if ( ( c & UNMARKED ( level )
		     &&
		     ( lev.first_g + 1 != end_g
		       ||
		          MUP::type_of_control ( c )
		       != min::ACC_MARK ) ) )
		{
		    // Remove s from acc list.
		    //
		    if ( s == lev.first_g[1]
		                 .last_before )
		    {
		        // If we are removing the last
			// stub of a generation we must
			// update the last_before's of
			// succeeding generations.
			//
		        MACC::generation * g =
			    lev.first_g + 1;
			do g->last_before =
			       lev.last_stub;
			while ( g ++ != lev.last_g );
		    }

		    last_c =
		        MUP::renew_control_stub
			    ( last_c,
			      MUP::stub_of_acc_control
				  ( c ) );
		    MUP::set_control_of
		        ( lev.last_stub, last_c );

		    // Remove s from aux hash table.
		    //
		    remove_from_aux_hash_table ( c, s );

		    // Deallocate body of s.
		    //
		    min::unsptr size =
		        MUP::body_size_of ( s );
		    if ( size != 0 )
		        MUP::deallocate_body
			    ( s, size );

		    // Free stub s.
		    //
		    MINT::free_acc_stub ( s );
		    ++ collected;
		}
		else
		{
		    last_c = c;
		    lev.last_stub = s;
		    ++ kept;
		}
	    }

	    lev.collected_count += collected;
	    lev.kept_count += kept;
	}
        break;

    case PRE_PROMOTING:
	{
	    if ( MACC::ephemeral_levels = 0 )
	    {
	        // If there are no ephemeral levels,
		// no promoting is needed and we are
		// done.

		MACC::removal_request_flags &=
		    ~ UNMARKED ( level );
		MINT::hash_acc_clear_flags &=
		    ~ UNMARKED ( level );
		lev.collector_state =
		    COLLECTOR_NOT_RUNNING;
		break;
	    }

	    if ( lev.first_g == NULL )
	    {
	        // First time for any promoting
		// phase increment.

		assert ( lev.lock );

		if ( level == 0 )
		{
		    if ( levels[1].lock ) return false;
		    if ( levels[1].g->lock )
		        return false;
		    levels[0].lock = false;
		    levels[1].lock = true;
		    lev.first_g = levels[1].g;
		}
		else
		{
		    if ( lev.g->lock ) return false;
		    lev.first_g = lev.g;
		}
		lev.first_g->lock = true;
		lev.last_stub =
		    lev.first_g->last_before;
		lev.root.rewind();
		lev.collector_state = REMOVING_ROOT;
	    }
	    else
	    {
	        // We need to lock the next generation.
		//
		assert ( lev.first_g + 1 != end_g );
		bool level_change =
		       lev.first_g[0].level
		    != lev.first_g[1].level;

		if ( lev.first_g[1].lock ) return false;
		if (    level_change
		     && lev.first_g[1].level->lock )
		    return false;
		lev.first_g[0].lock = false;
		lev.first_g[1].lock = true;
		if ( level_change )
		{
		    lev.first_g[1].level->lock = true;
		    lev.first_g[1].level->root.rewind();
		    lev.last_stub =
		        lev.first_g[1].last_before;
		    lev.collector_state = REMOVING_ROOT;
		}
		else
		    lev.collector_state = PROMOTING;
		++ lev.first_g;
	    }
	}

	if ( lev.collector_state == PROMOTING )
	    goto promoting;
	// Fall through to REMOVING_ROOT.

    case REMOVING_ROOT:
        {
	    min::uns64 root_kept = 0;
	    min::uns64 root_removed = 0;
	    min::uns64 remove =
	        MACC::removal_request_flags;
	    MACC::level * rrlev =
		lev.first_g->level;
	    while (   root_kept + root_removed
		    < MACC::scan_limit )
	    {
		if ( rrlev->root.at_end() )
		{
		    rrlev->root.flush();
		    lev.collector_state = PROMOTING;
		    break;
		}

		min::stub * s = rrlev->root.current();
		if ( MUP::test_flags_of ( s, remove ) )
		{
		    rrlev->root.remove();
		    ++ root_removed;
		}
		else
		{
		    rrlev->root.keep();
		    ++ root_kept;
		}
	    }
	    lev.root_kept_count += root_kept;
	    lev.root_removed_count += root_removed;
	}

	if ( lev.collector_state != PROMOTING )
	    break;
	// Fall through to PROMOTING.

    case PROMOTING:
    promoting:
        {
	    min::uns64 promoted = 0;
	    min::uns64 scanned = 0;

	    while ( true )
	    {
		if (    lev.first_g
		     == lev.first_g->level->g )
		{
		    // First_g is sublevel 0.
		    // Let glevel be first_g's level.
		    // Clear COLLECTIBLE ( glevel )
		    // and NON_ROOT ( glevel ) of all
		    // g's stubs flags and put all 
		    // these stubs on glevel's root
		    // list.  Clear glevel's lock
		    // when done.
		    // 
		    unsigned glevel =
			lev.first_g->level - levels;
		    end_g->last_before = NULL;

		    min::uns64 c =
			MUP::control_of
			    ( lev.last_stub );
		    while (   promoted + scanned
		            < MACC::scan_limit
			    &&
			       lev.last_stub
			    != lev.first_g[1]
			          .last_before )
		    {
			min::stub * s =
			    MUP::stub_of_acc_control
			        ( c );
			c = MUP::control_of ( s );
			c &= ~ COLLECTIBLE ( glevel );
			int type =
			    MUP::type_of_control ( c );
			if ( MINT::scavenger_routines
			         [type]
			     != NULL )
			{
			    c &= ~ NON_ROOT ( glevel );
			    lev.first_g->level
				       ->root.push
				             ( s );
			}
			MUP::set_control_of ( s, c );
			++ promoted;
			lev.last_stub = s;
			if ( lev.first_g + 1 == end_g
			     &&
			     type == min::ACC_MARK )
			    end_g->last_before = s;
		    }

		    if (    lev.last_stub
		         != lev.first_g[1].last_before )
			break;

		    lev.first_g->level->lock = false;
		}
		else if ( lev.first_g + 1 == end_g )
		{
		    end_g->last_before = NULL;
		    min::uns64 c =
			MUP::control_of
			    ( lev.last_stub );
		    while ( scanned < MACC::scan_limit )
		    {
			min::stub * s =
			    MUP::stub_of_acc_control
			        ( c );
			assert ( s != MINT::null_stub );
			c = MUP::control_of ( s );
			++ scanned;
			lev.last_stub = s;
			int type =
			    MUP::type_of_control ( c );
			if ( type == min::ACC_MARK )
			{
			    end_g->last_before = s;
			    break;
			}
			assert
			    ( type != min::ACC_FREE );
		    }

		    if (    lev.last_stub
		         != end_g->last_before )
			break;
		}

		// Move all the stubs in the first_g
		// generation to the first_g-1 genera-
		// tion.  If we are at the last genera-
		// tion, all the stubs up to and
		// including the first ACC_MARKed stub
		// in the last generation are moved to
		// the previous generation.
		//
		lev.first_g->last_before =
		    lev.first_g[1].last_before;

		if ( lev.first_g + 1 == end_g )
		{
		    // We are done with the last genera-
		    // tion.

		    lev.first_g->lock = false;
		    MACC::removal_request_flags &=
			~ UNMARKED ( level );
		    MINT::hash_acc_clear_flags &=
			~ UNMARKED ( level );
		    lev.collector_state =
		        COLLECTOR_NOT_RUNNING;
		    break;
		}
		else if (    lev.first_g[0].level
		          != lev.first_g[1].level
			  ||
			  lev.first_g[1].lock )
		{
		    lev.collector_state =
		        PRE_PROMOTING;
		    break;
		}
		lev.first_g[0].lock = false;
		lev.first_g[1].lock = true;
		++ lev.first_g;
	    }
	    lev.promoted_count += promoted;
	    lev.acc_mark_count += scanned;
	}
        break;

    default:
        MIN_ABORT ( "bad collector state" );
    }
    return true;
}

// Compactor
// ---------

// Statistics
// ----------

void MACC::print_acc_statistics ( std::ostream & s )
{
    min::unsptr nstubs =
        MACC::stub_next - MACC::stub_begin;
    cout << std::setw ( 32 ) << "Numbers of"
         << std::setw ( 14 ) << "Used"
         << std::setw ( 14 ) << "Free"
         << std::setw ( 14 ) << "Total"
	 << std::endl;
    cout << std::setw ( 32 ) << "Stubs:"
         << std::setw ( 14 )
         << nstubs - MINT::number_of_free_stubs
         << std::setw ( 14 )
         << MINT::number_of_free_stubs
         << std::setw ( 14 )
         << nstubs
	 << std::endl;
    
    min::unsptr total_bytes = 0;
    min::unsptr free_bytes = 0;
    for ( MINT::fixed_block_list * fbl =
	      MINT::fixed_blocks;
          fbl < MINT::fixed_blocks
	      + MINT::number_fixed_blocks;
	  ++ fbl )
    {
	char buffer [40];
	sprintf ( buffer, "%d Byte Blocks:",
	                  fbl->size );
        MINT::fixed_block_list_extension * fblex =
	    fbl->extension;
	min::unsptr free = fbl->count;
	min::unsptr total = 0;
	MACC::region * r = fblex->last_region;
	if ( r != NULL ) do {
	    total += ( r->next - r->begin )
	           / fbl->size;
	    free += r->free_count;
	    r = r->region_next;
	} while ( r != fblex->last_region );
	if ( total == 0 ) continue;
	cout << std::setw ( 32 ) << buffer
	     << std::setw ( 14 ) << total - free
	     << std::setw ( 14 ) << free
	     << std::setw ( 14 ) << total
	     << std::endl;
	total_bytes += total * fbl->size;
	free_bytes += free * fbl->size;
    }
    cout << std::setw ( 32 )
         << "Bytes in Fixed Size Blocks:"
	 << std::setw ( 14 ) << total_bytes - free_bytes
	 << std::setw ( 14 ) << free_bytes
	 << std::setw ( 14 ) << total_bytes
	 << std::endl;
}
