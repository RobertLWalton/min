// MIN Allocator/Collector/Compactor
//
// File:	min_acc.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sun Jun  6 20:03:38 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/06/07 03:24:57 $
//   $RCSfile: min_acc.cc,v $
//   $Revision: 1.42 $

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
static min::unsptr page_size;
inline min::unsptr number_of_pages
    ( min::unsptr number_of_bytes )
{
    return ( number_of_bytes + page_size - 1 )
           / page_size;
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
#if MIN_POINTER_BITS > 32
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

	    assert ( MIN_STUB_BASE + page_size * pages
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
min::unsptr MACC::subregion_size;
min::unsptr MACC::superregion_size;
min::unsptr MACC::max_paged_body_size;
min::unsptr MACC::paged_body_region_size;
min::unsptr MACC::stub_stack_region_size;
min::unsptr MACC::stub_stack_segment_size;

MACC::region * MACC::region_table;
MACC::region * MACC::region_next;
MACC::region * MACC::region_end;

MACC::region * MACC::last_superregion = NULL;
MACC::region * MACC::last_free_subregion = NULL;
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
                MACC::space_factor, 8,
		( MIN_POINTER_BITS <= 32 ? 32 : 256 ),
		1, true );

    min::unsptr F = MACC::space_factor;

    get_param ( "cache_line_size",
                MACC::cache_line_size,
		8, 4096, 1, true );

    MACC::subregion_size = F * F * page_size;
    get_param ( "subregion_size",
                MACC::subregion_size,
		4 * F * page_size,
		F * F * F * page_size,
		page_size );

    MACC::superregion_size = 64 * MACC::subregion_size;
    get_param ( "superregion_size",
                MACC::superregion_size,
		MACC::subregion_size,
		( MIN_POINTER_BITS <= 32 ?
		  1 << 28 : 1ull << 44 ),
		MACC::subregion_size );

    MACC::max_paged_body_size = F * F * page_size;
    get_param ( "max_paged_body_size",
                MACC::max_paged_body_size,
		0,
		( MIN_POINTER_BITS <= 32 ?
		  1 << 24 : 1ull << 32 ),
		page_size );

    MACC::paged_body_region_size =
        F * MACC::max_paged_body_size;
    get_param ( "paged_body_region_size",
                MACC::paged_body_region_size,
		MACC::max_paged_body_size,
		( MIN_POINTER_BITS <= 32 ?
		  1 << 28 : 1ull << 44 ),
		page_size );

    MACC::stub_stack_segment_size = 4 * page_size;
    get_param ( "stub_stack_segment_size",
                MACC::stub_stack_segment_size,
		page_size, 64 * page_size,
		page_size );

    MACC::stub_stack_region_size =
        16 * MACC::stub_stack_segment_size;
    get_param ( "stub_stack_region_size",
                MACC::stub_stack_region_size,
		MACC::stub_stack_segment_size,
		64 * MACC::stub_stack_segment_size,
		page_size );

    if ( MINT::max_fixed_block_size > F * page_size )
	MINT::max_fixed_block_size = F * page_size;

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

// Allocate a new multi-page region with the given
// number of pages.
//
static MACC::region * new_multi_page_block_region
	( min::unsptr size, int type )
{
    if ( MOS::trace_pools >= 1 )
        cout << "TRACE: new_multi_page_block_region ("
	     << size << ", " << type << ")" << endl;

    // Round up to a multiple of the page size and
    // allocate.
    //
    min::unsptr pages = number_of_pages ( size );
    size = pages * page_size;
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

    return r;
}

// Call this when we are out of superregions.
//
static void allocate_new_superregion ( void )
{
    if ( MOS::trace_pools >= 1 )
        cout << "TRACE: allocate_new_superregion()"
	     << endl;

    // Allocate as multi page block region.  If not the
    // first superregion, try downsizing if necessary.
    //
    MACC::region * r = new_multi_page_block_region
	( MACC::superregion_size,
	  MACC::SUPERREGION );
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
		 << ( MACC::superregion_size
		      / page_size )
		 << " page initial superregion."
		 << endl;
	else
	    cout << "ERROR: out of virtual memory"
	            " trying to allocate "
		 << ( MACC::subregion_size
		      / page_size )
	         << " page superregion." << endl;
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

    // Search current region and then next regions
    // in the circular list of regions until one
    // found with free blocks, and return it as r.
    // But set r = NULL if no region has free blocks.
    //
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

	// Allocate new subregion.
	//
        r = MACC::last_free_subregion;
	if ( r != NULL )
	{
	    // Found new subregion on the list of free
	    // subregions.

	    int locator = MUP::locator_of_control
	                       ( r->block_control );
	    sr = MACC::region_table + locator;
	    assert ( MACC::region_table <= sr
	             &&
		     sr < MACC::region_next );

	    MACC::remove
	        ( MACC::last_free_subregion, r );
	}
	else
	{
	    // Try to allocate to last superregion.
	    //
	    MACC::region * sr = MACC::last_superregion;
	    min::uns8 * new_next = sr->next
	                         + MACC::subregion_size;

	    if ( new_next > sr->end )
	    {
		// Need a new superregion.

	        allocate_new_superregion();
		sr = MACC::last_superregion;
		new_next = sr->next
	                 + MACC::subregion_size;
		assert ( new_next <= sr->end );
	    }

	    r = (MACC::region *) sr->next;
	    sr->next = new_next;
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
		  
	r->block_size = fbl->size;
	r->free_count = 0;
	r->free_first = r->free_last = NULL;

	// Insert the region at the end of the fixed
	// size region list of the given block size.
	//
	r->region_previous = r->region_next = r;
	MACC::insert_after ( fblext->last_region, r );
    }

    // Make the found region current.
    //
    fblext->current_region = r;

    if ( r->free_count > 0 )
    {
        // Move region free list to fbl.
	//
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
		assert ( r->next == r->end );
		break;
	    }

	    MINT::free_fixed_size_block * b =
	        (MINT::free_fixed_size_block *) r->next;
	    b->block_control =
	        MUP::new_control_with_locator
		    (   ( (min::uns8 *) r - r->next )
		      / page_size,
		      MINT::null_stub );

	    if ( last == NULL )
	        fbl->first = b;
	    else
	        last->next = b;

	    last = b;

	    r->next += fbl->size;
	    ++ fbl->count;
	}
	assert ( last != NULL );
	last->next = NULL;
    }

    assert ( fbl->count > 0 );

    // Allocate first block from refurbished fbl free
    // list.
    //
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
	cout << "ERROR: out of virtual memory"
		" trying to allocate "
	     << ( MACC::max_paged_body_size
		  / page_size )
	     << " page paged body region." << endl;
	MOS::dump_error_info ( cout );
	exit ( 1 );
    }

    r->block_size = MACC::max_paged_body_size;

    MACC::insert ( MACC::last_paged_body_region, r );
}

// Execute new_non_fixed_body for bodies of size at most
// MACC::max_paged_body_size.  n is size of body + body
// control rounded up to a multiple of page_size.  n
// should be larger than MINT::max_fixed_block_size.
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
	cout << "ERROR: out of virtual memory"
		" trying to allocate "
	     << ( n / page_size )
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
    // multiple of page_size.
    //
    n += sizeof ( min::uns64 );
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
	    r = r->region_next;
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

	r = new_multi_page_block_region
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
	insert ( last_stub_stack_region, r );
	current_stub_stack_region = r;
    }

    // Now r has a free segment or room to allocate
    // a new segment.

    stub_stack_segment * sss;
    if ( r->free_count > 0 )
    {
        MINT::free_fixed_size_block * b =
	    r->free_first;
	r->free_first = b->next;
	if ( r->free_first == NULL )
	    r->free_last = NULL;
	-- r->free_count;
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
    if ( input_segment != last_segment )
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
	    region * r = & region_table[locator];
	    if ( r->free_count ++ == 0 )
		r->free_first = r->free_last = b;
	    else
	    {
		r->free_last->next = b;
		r->free_last = b;
	    }

	    // TBD: if all segments in r are free,
	    // free r.
	}
    }
    else
	is_at_end = true;
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
	region * r = & region_table[locator];
	if ( r->free_count ++ == 0 )
	    r->free_first = r->free_last = b;
	else
	{
	    r->free_last->next = b;
	    r->free_last = b;
	}

	// TBD: if all segments in r are free, free r.
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
static MACC::generation generations[MAX_GENERATIONS];
MACC::generation * MACC::generations = ::generations;

static const unsigned MAX_LEVELS =
    1 + MIN_MAX_EPHEMERAL_LEVELS;
static MACC::level levels_vector[MAX_GENERATIONS];
MACC::level * MACC::levels = levels_vector;

min::unsptr  MACC::acc_stack_size;
min::stub ** MACC::acc_stack_begin;
min::stub ** MACC::acc_stack_end;

static void collector_initializer ( void )
{
    get_param ( "ephemeral_levels",
                MACC::ephemeral_levels,
		0, MIN_MAX_EPHEMERAL_LEVELS );

    MACC::generation * g = MACC::generations;

    levels[0].g = g ++;
    levels[0].number_of_sublevels = 1;

    char name[80];
    for ( unsigned i = 1;
          i < MACC::ephemeral_levels; ++ i )
    {
        sprintf ( name, "ephemeral_sublevels[%d]", i );
	get_param ( name,
	            MACC::ephemeral_sublevels[i],
		    1, MIN_MAX_EPHEMERAL_SUBLEVELS );

	levels[i].g = g;
	levels[i].number_of_sublevels =
	    MACC::ephemeral_sublevels[i];

	for ( unsigned j = 0;
	      j < MACC::ephemeral_sublevels[i]; ++ j )
	{
	    g->level = i;
	    g->sublevel = j;
	    g->last_before = MINT::first_allocated_stub;
	    g->count = 0;

	    ++ g;
	}
    }

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

	unsigned non_root =
	    (unsigned) ( f1 >> ( M - 1 ) );
	unsigned unmarked = non_root >> ( E + 1 );
	non_root &= ( ( ( 1 << E ) - 1 ) << 1 );

	while ( unmarked != 0 )
	{
	    unsigned i = MINT::log2floor ( unmarked );
	    unmarked ^= 1 << i;
	    levels[i].to_be_scavenged.push ( s2 );
	    MUP::clear_flags_of
	        ( s2, UNMARKED ( i ) );
	}
	while ( non_root != 0 )
	{
	    unsigned i = MINT::log2floor ( non_root );
	    non_root ^= 1 << i;
	    level & lev = levels[i];
	    lev.root.push ( s1 );
	    MUP::clear_flags_of
	        ( s1, NON_ROOT ( i ) );

	    if (   lev.collector_state
	         > INITING_ROOT_FLAGS )
	    {
	        // After initializing root flags, new
		// roots are marked scavenged.  This is
		// an efficiency measure, as new roots
		// only contain a single pointer to
		// a collectible stub.
		//
		MUP::set_flags_of
		    ( s1, SCAVENGED ( i ) );
		if (   MUP::test_flags_of
		            ( s2, UNMARKED ( i ) )
		     & UNMARKED ( i ) )
		{
		    // After termination unmarked
		    // s2's should be unreachable.
		    //
		    assert (    lev.collector_state
		             <= COLLECTOR_TERMINATING );

		    lev.to_be_scavenged.push ( s2 );
		    MUP::clear_flags_of
			( s2, UNMARKED ( i ) );
		}
	    }
	}
    }
}

// Perform one increment of the current collector execu-
// tion.  To start a collector execution just set the
// level state to COLLECTOR_START.  The collector
// execution is finished when it returns with the level
// collector_state set to COLLECTOR_FINISHED.
//
static void collector_increment ( unsigned level )
{

    MACC::level & lev = levels[level];

    switch ( lev.collector_state )
    {
    case COLLECTOR_START:
	MINT::new_acc_stub_flags |=
	    UNMARKED ( level );
	MINT::new_acc_stub_flags &=
	    ~ SCAVENGED ( level );
	lev.last_allocated_stub =
	    MINT::last_allocated_stub;

	lev.last_stub = lev.g->last_before;
	lev.collector_state =
	    INITING_COLLECTIBLE_FLAGS;
	//
	// Fall throught to next collector_state.

    case INITING_COLLECTIBLE_FLAGS:
        {
	    min::uns64 scanned = 0;   
	    min::uns64 c =
	        MUP::control_of ( lev.last_stub );
	    while (    lev.last_stub
	            != lev.last_allocated_stub
	            &&
		    scanned < MACC::scan_limit )
	    {
		min::stub * s =
		    MUP::stub_of_acc_control ( c );
		c = MUP::control_of ( s );
		c |= UNMARKED ( level );
		c &= ~ SCAVENGED ( level );
		MUP::set_control_of ( s, c );
		lev.last_stub = s;
	    }
	    lev.root_flag_set_count += scanned;
	    if (    lev.last_stub
	         == lev.last_allocated_stub )
	    {
		lev.root.rewind();
	        lev.collector_state =
		    INITING_ROOT_FLAGS;
	    }
	}
        break;

    case INITING_ROOT_FLAGS:
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
	    lev.root_flag_set_count += scanned;
	    if ( lev.root.at_end() )
	    {
		lev.root.rewind();
		MINT::scavenge_control & sc =
		    MINT::scavenge_controls[level];
		sc.state = 0;
		sc.stub_flag = UNMARKED ( level );
		sc.level = level;
		sc.gen_limit = MACC::scan_limit;
	        lev.collector_state =
		    SCAVENGING;
	    }
	}
	break;

    case SCAVENGING:
        {
	    MINT::scavenge_control & sc =
		MINT::scavenge_controls[level];
	    sc.gen_count = 0;
	    sc.stub_count = 0;
	    lev.to_be_scavenged.begin_push
	        ( sc.to_be_scavenged,
	          sc.to_be_scavenged_limit );
	    min::uns64 scavenged = 0;   
	    while ( true )
	    {
		if ( sc.state == 0 )
		{
		    if ( ! lev.to_be_scavenged
		              .at_end() )
		    {
		        sc.s1 = lev.to_be_scavenged
			           .current();
		        lev.to_be_scavenged.remove();
			lev.root_scavenge = false;
			MUP::set_flags_of
			    ( sc.s1,
			      SCAVENGED ( level ) );
		    }
		    else if ( ! lev.root.at_end() )
		    {
			sc.stub_flag_accumulator = 0;
		        sc.s1 = lev.root.current();

			if ( MUP::test_flags_of
			        ( sc.s1, SCAVENGED
				           ( level ) ) )
			{
			    lev.collector_state =
			        COLLECTOR_TERMINATING;
			    break;
			}

			lev.root_scavenge = true;
			MUP::set_flags_of
			    ( sc.s1,
			      SCAVENGED ( level )
			      +
			      NON_ROOT ( level ) );
		    }
		    else
		    {
		        lev.collector_state =
			    COLLECTOR_TERMINATING;
			break;
		    }
		}

		int type = min::type_of ( sc.s1 );
		assert ( type >= 0 );
		MINT::scavenger_routine scav =
		    MINT::scavenger_routines[type];
		if ( scav != NULL) (* scav) ( sc );

		if ( sc.state != 0 ) break;

		++ scavenged;
		if ( lev.root_scavenge )
		{
		    if ( ! MUP::test_flags_of
		               ( sc.s1, NON_ROOT
			                  ( level ) ) )
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
	    lev.scanned_count += sc.gen_count;
	    lev.scavenged_count += scavenged;

	    if (    lev.collector_state
	         == COLLECTOR_TERMINATING )
	        lev.termination_count = 0;
	}
	break;

    case COLLECTOR_TERMINATING:
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

	    while ( true )
	    {
		if ( sc.state == 0 )
		{
		    if ( ! lev.to_be_scavenged
		              .at_end() )
		    {
		        sc.s1 = lev.to_be_scavenged
			           .current();
		        lev.to_be_scavenged.remove();
			MUP::set_flags_of
			    ( sc.s1,
			      SCAVENGED ( level ) );
		    }
		    else if ( ! thread_scavenged )
		    {
			sc.state = 0;
			MINT::thread_scavenger_routine
			    ( sc );

			if ( sc.state != 0 ) break;

			thread_scavenged = true;
			if ( lev.termination_count
			     == 0 )
			    sc.gen_limit +=
			        sc.gen_count;
			else if ( lev.termination_count
			          % 4 == 0 )
			    sc.gen_limit *= 2;
			++ lev.termination_count;
			continue;
		    }
		    else
		    {
			lev.root_removal_level =
			    level;
		        lev.collector_state =
			    ROOT_REMOVAL;
			break;
		    }
		}

		int type = min::type_of ( sc.s1 );
		assert ( type >= 0 );
		MINT::scavenger_routine scav =
		    MINT::scavenger_routines[type];
		if ( scav != NULL) (* scav) ( sc );

		if ( sc.state != 0 ) break;

		++ scavenged;
	    }

	    lev.to_be_scavenged.end_push
		( sc.to_be_scavenged );
	    lev.scanned_count += sc.gen_count;
	    lev.scavenged_count += scavenged;
	}
	break;

    case ROOT_REMOVAL:
        {
	    bool force_rewind = false;
	    if ( lev.root_removal_level == level )
	    {
		++ lev.root_removal_level;
		force_rewind = true;
	    }

	    min::uns64 unmarked_flag =
	        UNMARKED ( level );
	    min::uns64 scanned = 0;
	    while ( true )
	    {
	        if (   lev.root_removal_level
		     > MINT::number_of_acc_levels )
		{
		    lev.last_stub = lev.g->last_before;
		    lev.collector_state =
		        level == 0 ? COLLECTING :
		                     PROMOTING;
		    break;
		}

		MACC::level & rrlev =
		    MACC::levels[lev.root_removal_level];

		if ( force_rewind )
		{
		    rrlev.root.rewind();
		    force_rewind = false;
		}

		while ( scanned < MACC::scan_limit
		        &&
			! rrlev.root.at_end() )
		{
		    min::stub * s =
		        rrlev.root.current();
		    if ( MUP::test_flags_of
		             ( s, unmarked_flag ) )
		        rrlev.root.remove();
		    else
		        rrlev.root.keep();
		    ++ scanned;
		}

		if ( scanned >= MACC::scan_limit )
		    break;

		if ( rrlev.root.at_end() )
		{
		    rrlev.root.flush();
		    ++ lev.root_removal_level;
		    force_rewind = true;
		}

	    }
	    lev.scanned_count += scanned;
	}
	break;

    case PROMOTING:
        {
	    min::stub * end_stub =
	        lev.gnext == NULL ?
		lev.last_allocated_stub :
		lev.gnext->last_before;
	        
	    min::uns64 collected = 0;   
	    min::uns64 promoted = 0;   
	    min::uns64 last_c =
	        MUP::control_of ( lev.last_stub );
	    while ( lev.last_stub != end_stub
	            &&
		      collected + promoted
		    < MACC::scan_limit )
	    {
		min::stub * s =
		    MUP::stub_of_acc_control ( last_c );
		min::uns64 c = MUP::control_of ( s );
		if ( c & UNMARKED ( level ) )
		{
		    if ( s == end_stub )
		        end_stub = lev.last_stub;


		    // TBD FREE BODY and STUB

		    last_c = MUP::renew_control_stub
				( last_c,
				  MUP::stub_of_control
				      ( c ) );
		    MUP::set_control_of
		        ( lev.last_stub, last_c );

		    ++ collected;
		}
		else
		{
		    c &= ~ COLLECTIBLE ( level );
		    c &= ~ NON_ROOT ( level );
		    lev.root.push ( s );
		    MUP::set_control_of ( s, c );
		    last_c = c;
		    lev.last_stub = s;
		    ++ promoted;
		}
	    }
	    lev.collected_count += collected;
	    lev.promoted_count += promoted;

	    if ( lev.last_stub == end_stub )
	    {
	        if ( lev.gnext != NULL )
		    lev.gnext->last_before = end_stub;
		// TBD
	        lev.collector_state = COLLECTING;
	    }
	}
	break;

    default:
        MIN_ABORT ( "bad collector state" );
    }
}
