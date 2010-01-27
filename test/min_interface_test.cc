// MIN Language Interface Test Program
//
// File:	min_interface_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Jan 27 01:38:39 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/01/27 07:31:52 $
//   $RCSfile: min_interface_test.cc,v $
//   $Revision: 1.127 $

// Table of Contents:
//
//	Setup
//	Run-Time System for Interface Tests
//	Main Program
//	C++ Number Types
//	General Value Types and Data
//	Stub Types and Data
//	Internal Pointer Conversion Functions
//	General Value Constructor/Test/Read Functions
//	Control Values
//	Stub Functions
//	Process Interface
//	Allocator/Collector/Compactor Interface
//	Numbers
//	Strings
//	Labels
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level
//	Finish

// Setup
// -----

# include <iostream>
# include <iomanip>
# include <cstring>
using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::ostream;

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

# include <min.h>
# define MUP min::unprotected
# define MINT min::internal

// Helper functions for tests.

struct print_gen {
    min::gen g;
    print_gen ( min::gen g ) : g ( g ) {}
    friend ostream & operator <<
    	    ( ostream & s, print_gen pg )
    {
	return s << hex << pg.g << dec;
    }
};

// Run-Time System for Interface Tests
// -------- ------ --- --------- -----

// Out of line functions that must be defined to test
// the MIN interface.  These definitions substitute for
// the normal run-time system, and are just for simple
// interface testing.

// Process Interface Functions.

// When this out-of-line interrupt function is called it
// sets the `interrupt_called' variable to true.
//
bool interrupt_called;
bool MINT::interrupt ( void )
{
    interrupt_called = true;
    return true;
}

// Allocator/Collector/Compactor Interface Functions.

// Place to allocate stubs.  Stubs must be allocated on
// a `sizeof (min::stub)' boundary.
//
char stub_region[10000];

// Number of stubs allocated to stub_region so far,
// address of the first stub in the region, and the
// address of the first location beyond the last
// possible stub in the region.
//
min::unsptr stubs_allocated = 0;
min::stub * begin_stub_region;
min::stub * end_stub_region;

// Initialize stub_region, MINT::last_allocated_stub (to
// a dummy), and number_free_stubs (to zero).
//
void initialize_stub_region ( void )
{
    min::unsptr stp = (min::unsptr) stub_region;
    // Check that sizeof min::stub is a power of 2.
    assert ( (   sizeof (min::stub) - 1
	       & sizeof (min::stub) )
	     == 0 );
    min::unsptr p = stp;
    p += sizeof (min::stub) - 1;
    p &= ~ (sizeof (min::stub) - 1 ) ;
    begin_stub_region = (min::stub *) p;

#   ifndef MIN_STUB_BASE
	min::internal::stub_base =
	    (min::unsptr) begin_stub_region;
	min::internal::null_stub = begin_stub_region;
#   else
	MIN_ASSERT
	    (    begin_stub_region
	      >= (min::stub *)
	         min::internal::stub_base );
#   endif

    stp += sizeof stub_region;
    min::unsptr n =
        ( stp - p ) / ( sizeof (min::stub) );
    p += ( sizeof (min::stub) ) * n;
    end_stub_region = (min::stub *) p;
    assert ( begin_stub_region < end_stub_region );

    MINT::last_allocated_stub = begin_stub_region;
    MINT::number_of_free_stubs = 0;
    ++ stubs_allocated;
    min::uns64 c = MUP::new_acc_control
	( min::ACC_FREE, MINT::null_stub );
    MUP::set_control_of
	( MINT::last_allocated_stub, c );
    MUP::set_value_of
	( MINT::last_allocated_stub, 0 );
}

ostream & operator <<
	( ostream & out, const min::stub * s )
{
    min::stub locate;
    out << "stub ";

    if ( s > & locate ) out << "in stack";
    else if ( begin_stub_region <= s
	      &&
	      s < end_stub_region )
	out << s - begin_stub_region;
    else
        MIN_ABORT ( "bad stub" );

    return out;
}

// Function to allocate n - number_of_free_stubs more
// stubs to the stub_region and attach them to the
// free list just after the last_allocated_stub.
//
void MINT::acc_expand_stub_free_list ( min::unsptr n )
{
    cout << "MINT::acc_expand_stub_free_list (" << n
         << ") called" << endl;
    if ( n <= MINT::number_of_free_stubs ) return;
    n -= MINT::number_of_free_stubs;

    min::uns64 lastc = MUP::control_of
	( MINT::last_allocated_stub );
    min::stub * free =
	MUP::stub_of_acc_control ( lastc );
    while ( n -- > 0 )
    {
        min::stub * s = begin_stub_region
	              + stubs_allocated;
	assert ( s < end_stub_region );
	++ stubs_allocated;
	++ MINT::number_of_free_stubs;
	min::uns64 c = MUP::new_acc_control
	    ( min::ACC_FREE, free );
	MUP::set_control_of ( s, c );
	MUP::set_value_of ( s, 0 );
	free = s;
    }
    lastc = MUP::renew_acc_control_stub ( lastc, free );
    MUP::set_control_of
	( MINT::last_allocated_stub, lastc );
}

// Place to allocate bodies.  Bodes must be allocated on
// 8 byte boundaries.
//
char body_region[2000000];

// Place to point deallocated bodies.  All zeros.
//
char deallocated_body_region[2000000];

// Address of the first body control block in the
// region, and the address of the first location beyond
// the last possible body control block in the region.
//
min::uns64 * begin_body_region;
min::uns64 * end_body_region;
min::uns64 * next_body;

// Initialize body_region.
//
void initialize_body_region ( void )
{
    MINT::max_fixed_block_size = 1 << 17;

    min::unsptr stp = (min::unsptr) body_region;
    min::unsptr p = stp;
    p += 7;
    p &= ~ 7;
    begin_body_region = (min::uns64 *) p;
    stp += sizeof body_region;
    min::unsptr n = ( stp - p ) / 8;
    p += 8 * n;
    end_body_region = (min::uns64 *) p;

    for ( min::unsptr j = 0;
          j < MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG-2;
	  ++ j )
    {
	MINT::fixed_blocks[j].size = 1 << ( j + 3 );
	MINT::fixed_blocks[j].count = 0;
	MINT::fixed_blocks[j].first = NULL;
    }
    next_body = begin_body_region;
}

void MINT::new_non_fixed_body
	( min::stub * s, min::unsptr n )
{
    cout << "MINT::new_non_fixed_body ( " << s
         << ", " << n << " ) called" << endl;

    min::unsptr m = n + 7;
    m >>= 3;
    MIN_ASSERT ( next_body + m <= end_body_region );

    min::unprotected
       ::set_pointer_of ( s, next_body );

    next_body += m;
}

// Performs MUP::new_body when count of fixed bodies
// is zero.
//
void MINT::new_fixed_body
    ( min::stub * s, min::unsptr n,
      MINT::fixed_block_list * fbl )
{
    cout << "MINT::new_fixed_body ( " << s
         << ", " << n << " ) called" << endl;

    min::unsptr m = fbl->size >> 3;
    min::uns64 * next = next_body;
    MIN_ASSERT ( next + 2 * m <= end_body_region );

    cout << "Using fixed_blocks["
         << fbl - fixed_blocks << "]"
	 << " and assigning begin_body_region["
	 << next - begin_body_region
	 << " .. "
	 << next - begin_body_region + m - 1
	 << "]" << endl;

    * next = 0;
    min::unprotected
       ::set_pointer_of ( s, next + 1 );
    min::unprotected
       ::set_flags_of ( s, MINT::ACC_FIXED_BODY_FLAG );

    next += m;

    MINT::free_fixed_size_block * fb =
        (MINT::free_fixed_size_block *) next;
    fb->block_control = 0;
    fb->block_subcontrol = 0;
    fb->next = NULL;
    fbl->first = fb;
    fbl->count = 1;

    next_body = next + m;
}

// Deallocate the stub body.  Repoints the body to the
// all zero deallocated_body_region.
//
void MUP::deallocate_body
	( min::stub * s, min::unsptr n )
{
    if ( n == 0 ) return;

    cout << "MINT::deallocate ( " << s
         << " ) called" << endl;

    MUP::set_pointer_of ( s, deallocated_body_region );
    MUP::set_type_of ( s, min::DEALLOCATED );
}

// Function to relocate a body.  Just allocates a new
// body, copies the contents of the old body to the
// new body, and zeros the old body, and deallocates
// the old body.  Sizes of new and old bodies are given.
// The minimum of these is copied.
//
static void resize_body
	( min::stub * s, min::unsptr new_size,
	                 min::unsptr old_size )
{
    cout << "resize_body ( stub "
         << s - begin_stub_region << ", " << new_size
         << ", " << old_size << " ) called" << endl;
    {
	MUP::resize_body rbody
	    ( s, new_size, old_size );
	min::unsptr length = new_size >= old_size ?
	                  old_size : new_size;

	min::uns64 * from = (min::uns64 *)
	    MUP::pointer_of ( s );
	min::uns64 * to   = (min::uns64 *)
	    MUP::new_body_pointer_ref ( rbody );
	cout << "copying body_region["
	     << from - begin_body_region
	     << " .. "
	     << from - begin_body_region + length/8 - 1
	     << "] to body_region["
	     << to - begin_body_region
	     << " .. "
	     << to - begin_body_region + length/8 - 1
	     << "]" << endl;
	memcpy ( to, from, length );
	cout << "zeroing body_region["
	     << from - begin_body_region
	     << " .. "
	     << from - begin_body_region
	             + old_size/8 - 1
	     << "]" << endl;
	memset ( from, 0, old_size );
    }

}

// Function to initialize ACC hash tables.
//
void initialize_hash_tables ( void )
{
    typedef min::stub * stubp;
    MINT::str_hash_size = 128;
    MINT::str_hash_mask = 128 - 1;
    MINT::str_hash =
        new stubp[MINT::str_hash_size];
    for ( int i = 0; i < MINT::str_hash_size; ++ i )
        MINT::str_hash[i] = MINT::null_stub;
    MINT::num_hash_size = 128;
    MINT::num_hash_mask = 128 - 1;
    MINT::num_hash =
        new stubp[MINT::num_hash_size];
    for ( int i = 0; i < MINT::num_hash_size; ++ i )
        MINT::num_hash[i] = MINT::null_stub;
    MINT::lab_hash_size = 128;
    MINT::lab_hash_mask = 128 - 1;
    MINT::lab_hash =
        new stubp[MINT::lab_hash_size];
    for ( int i = 0; i < MINT::lab_hash_size; ++ i )
        MINT::lab_hash[i] = MINT::null_stub;
}

// Acc stack.
//
static min::stub * acc_stack[1000];
static min::stub ** acc_stack_end = acc_stack + 1000;

// Initialize the acc_stack.
//
void initialize_acc_stack ( void )
{
    MINT::acc_stack = ::acc_stack;
    MINT::acc_stack_limit = ::acc_stack_end - 6;
}

void MINT::acc_initializer ( void )
{
    initialize_stub_region();
    initialize_body_region();
    initialize_hash_tables();
    initialize_acc_stack();

    MINT::acc_initialize_resize_body();
}

// Find a values address in a MINT::gen_locator list.
//
bool find_locator
    ( min::gen * address, MINT::gen_locator * locator )
{
    while ( locator )
    {
        if ( locator->values == address )
	    return true;
	locator = locator->previous;
    }
    return false;
}

// Count the number of locators on a MINT::gen_locator
// list.
//
int count_locators ( MINT::gen_locator * locator )
{
    int count = 0;
    while ( locator )
    {
        ++ count;
	locator = locator->previous;
    }
    return count;
}

// Main Program
// ---- -------

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

	cout << endl;
	cout << "Finish Number Types Test!" << endl;
    }

// General Value Types and Data
// ------- ----- ----- --- ----

    // There are no general types and data tests.

// Stub Types and Data
// ---- ----- --- ----

    // There are no stub types and data tests.

// Internal Pointer Conversion Functions
// -------- ------- ---------- ---------

    {
	cout << endl;
	cout << "Start Internal Pointer Conversion"
	        " Test!" << endl;
    	char buffer[1];
	const min::stub * stub = MINT::null_stub;

	cout << endl;
	cout << "Test pointer/uns64 conversions:"
	     << endl;
	min::uns64 u64 =
	    MINT::pointer_to_uns64 ( buffer );
	char * b64 = (char *)
	    MINT::uns64_to_pointer ( u64 );
	MIN_ASSERT ( b64 == buffer );

#	if MIN_IS_COMPACT
	    cout << endl;
	    cout << "Test stub/uns32 conversions:"
		 << endl;
	    min::uns32 u32 =
	        MINT::stub_to_unsgen ( stub );
	    min::stub * s32 =
		MINT::unsgen_to_stub ( u32 );
	    MIN_ASSERT ( s32 == stub );
#	elif MIN_IS_LOOSE
	    cout << endl;
	    cout << "Test general stub/uns64"
	            " conversions:" << endl;
	    u64 = MINT::stub_to_unsgen ( stub );
	    min::stub * s64 =
		MINT::unsgen_to_stub ( u64 );
	    MIN_ASSERT ( s64 == stub );
	    u64 += min::uns64(min::GEN_STUB) << 44;
	    min::uns64 g64 =
		MINT::stub_into_gen ( u64, stub );
	    MIN_ASSERT ( u64 == g64 );
#	endif

	cout << endl;
	cout << "Finish Internal Pointer Conversion"
	        " Test!" << endl;
    }

// General Value Constructor/Test/Read Functions
// ------- ----- --------------------- ---------

    {
	cout << endl;
	cout << "Start General Value Constructor/"
	        "/Test/Read Function Test!" << endl;
	const min::stub * stub = MINT::null_stub;

	cout << endl;
	cout << "Test stub general values:" << endl;
	cout << "stub: " << hex
	     << min::unsptr ( stub )
	     << dec << endl;
	min::gen stubgen = MUP::new_gen ( stub );
	cout << "stubgen: " << print_gen ( stubgen )
	     << endl;
	MIN_ASSERT ( min::is_stub ( stubgen ) );
	MIN_ASSERT ( MUP::stub_of ( stubgen ) == stub );
	stubgen = min::new_gen ( stub );
	MIN_ASSERT ( min::is_stub ( stubgen ) );
	MIN_ASSERT (    min::gen_subtype_of ( stubgen )
	             == min::GEN_STUB );
	MIN_ASSERT ( MUP::stub_of ( stubgen ) == stub );
	MIN_ASSERT ( ! min::is_direct_str ( stubgen ) );

#	if MIN_IS_COMPACT
	    cout << endl;
	    cout << "Test direct integer general"
	    	    " values:" << endl;
	    int i = -8434;
	    min::gen igen =
	    	MUP::new_direct_int_gen ( i );
	    cout << "igen: " << print_gen ( igen )
		 << endl;
	    MIN_ASSERT ( min::is_direct_int ( igen ) );
	    MIN_ASSERT
	        ( MUP::direct_int_of ( igen ) == i );
	    igen = min::new_direct_int_gen ( i );
	    MIN_ASSERT ( min::is_direct_int ( igen ) );
	    MIN_ASSERT
	        (    min::gen_subtype_of ( igen )
		  == min::GEN_DIRECT_INT );
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
	    MIN_ASSERT ( ! min::is_stub ( igen ) );
	    MIN_ASSERT
	    	( ! min::is_direct_str ( igen ) );
#	endif

#	if MIN_IS_LOOSE
	    cout << endl;
	    cout << "Test direct float general"
	    	    " values:" << endl;
	    min::float64 f = -8.245324897;
	    min::gen fgen =
	    	MUP::new_direct_float_gen ( f );
	    cout << "fgen: " << print_gen ( fgen )
		 << endl;
	    MIN_ASSERT
	        ( min::is_direct_float ( fgen ) );
	    MIN_ASSERT
	        ( MUP::direct_float_of ( fgen ) == f );
	    fgen = min::new_direct_float_gen ( f );
	    MIN_ASSERT
	        ( min::is_direct_float ( fgen ) );
	    MIN_ASSERT
	        (    min::gen_subtype_of ( fgen )
		  == min::GEN_DIRECT_FLOAT );
	    MIN_ASSERT
	        ( MUP::direct_float_of ( fgen ) == f );
	    MIN_ASSERT ( ! min::is_stub ( fgen ) );
	    MIN_ASSERT
	    	( ! min::is_direct_str ( fgen ) );
#	endif
    
        cout << endl;
	cout << "Test direct string general values:"
	     << endl;
#	if MIN_IS_COMPACT
	    char * str = "ABC";
	    char * overflowstr = "ABCD";
	    char * overflowstrn = "ABCDE";
	    int strlimit = 3;
#	elif MIN_IS_LOOSE
	    char * str = "ABCDE";
	    char * overflowstr = "ABCDEF";
	    char * overflowstrn = "ABCDEFG";
	    int strlimit = 6;
#	endif
	union {
	    min::uns64 u64;
	    char str[8];
	} value;

	min::gen strgen =
	    MUP::new_direct_str_gen ( str );
	cout << "strgen: " << print_gen ( strgen )
	     << endl;
	MIN_ASSERT ( min::is_direct_str ( strgen ) );
	MIN_ASSERT (    min::gen_subtype_of ( strgen )
	             == min::GEN_DIRECT_STR );
	value.u64 = MUP::direct_str_of ( strgen );
	MIN_ASSERT ( strcmp ( str, value.str ) == 0 );
	desire_success (
	    strgen = min::new_direct_str_gen ( str );
	);
	desire_failure (
	    strgen = min::new_direct_str_gen
	    			( overflowstr );
	);
	MIN_ASSERT ( ! min::is_stub ( strgen ) );

	min::gen strngen =
	    MUP::new_direct_str_gen ( str, 2 );
	cout << "strngen: " << print_gen ( strngen )
	     << endl;
	MIN_ASSERT ( min::is_direct_str ( strngen ) );
	MIN_ASSERT (    min::gen_subtype_of ( strngen )
	             == min::GEN_DIRECT_STR );
	value.u64 = MUP::direct_str_of ( strngen );
	MIN_ASSERT
	    ( strncmp ( str, value.str, 2 ) == 0 );
	MIN_ASSERT ( value.str[2] == 0 );
	desire_success (
	    strngen =
	        min::new_direct_str_gen ( str, 2 );
	);
	desire_failure (
	    strngen = min::new_direct_str_gen
	    			( overflowstrn,
				  strlimit + 1 );
	);
	MIN_ASSERT ( ! min::is_stub ( strngen ) );
 
        cout << endl;
	cout << "Test list aux general values:"
	     << endl;
	unsigned aux = 734523;
	min::gen listauxgen =
	    MUP::new_list_aux_gen ( aux );
	cout << "listauxgen: "
	     << print_gen ( listauxgen ) << endl;
	MIN_ASSERT ( min::is_list_aux ( listauxgen ) );
	MIN_ASSERT (    min::gen_subtype_of
				( listauxgen )
	             == min::GEN_LIST_AUX );
	MIN_ASSERT
	    ( min::list_aux_of ( listauxgen ) == aux );
	desire_success (
	    listauxgen = min::new_list_aux_gen ( aux );
	);
	desire_failure (
	    listauxgen = min::new_list_aux_gen
		( (min::unsgen) 1 << min::VSIZE );
	);
	MIN_ASSERT
	    ( ! min::is_stub ( listauxgen ) );
	MIN_ASSERT
	    ( ! min::is_direct_str ( listauxgen ) );
	unsigned reaux = 963921;
	listauxgen =
	    MUP::renew_gen ( listauxgen, reaux );
	cout << "re-listauxgen: "
	     << print_gen ( listauxgen ) << endl;
	MIN_ASSERT ( min::is_list_aux ( listauxgen ) );
	MIN_ASSERT
	    (    min::list_aux_of ( listauxgen )
	      == reaux );
	MIN_ASSERT
	    ( ! min::is_sublist_aux ( listauxgen ) );
	MIN_ASSERT
	    ( ! min::is_stub ( listauxgen ) );
	MIN_ASSERT
	    ( ! min::is_direct_str ( listauxgen ) );
 
        cout << endl;
	cout << "Test sublist aux general values:"
	     << endl;
	min::gen sublistauxgen =
	    MUP::new_sublist_aux_gen ( aux );
	cout << "sublistauxgen: "
	     << print_gen ( sublistauxgen ) << endl;
	MIN_ASSERT
	    ( min::is_sublist_aux ( sublistauxgen ) );
	MIN_ASSERT
	    (    min::gen_subtype_of ( sublistauxgen )
	      == min::GEN_SUBLIST_AUX );
	MIN_ASSERT
	    (    min::sublist_aux_of ( sublistauxgen )
	      == aux );
	desire_success (
	    sublistauxgen =
	        min::new_sublist_aux_gen ( aux );
	);
	desire_failure (
	    sublistauxgen = min::new_sublist_aux_gen
		( (min::unsgen) 1 << min::VSIZE );
	);
	MIN_ASSERT
	    ( ! min::is_stub ( sublistauxgen ) );
	MIN_ASSERT
	    ( ! min::is_direct_str ( sublistauxgen ) );
 
        cout << endl;
	cout << "Test indirect aux general values:"
	     << endl;
	min::gen indirectauxgen =
	    MUP::new_indirect_aux_gen ( aux );
	cout << "indirectauxgen: "
	     << print_gen ( indirectauxgen ) << endl;
	MIN_ASSERT
	    ( min::is_indirect_aux
	    		( indirectauxgen ) );
	MIN_ASSERT
	    (    min::gen_subtype_of ( indirectauxgen )
	      == min::GEN_INDIRECT_AUX );
	MIN_ASSERT
	    (    min::indirect_aux_of
	              ( indirectauxgen )
	      == aux );
	desire_success (
	    indirectauxgen =
	    	min::new_indirect_aux_gen ( aux );
	);
	desire_failure (
	    indirectauxgen = min::new_indirect_aux_gen
		( (min::unsgen) 1 << min::VSIZE );
	);
	MIN_ASSERT
	    ( ! min::is_stub ( indirectauxgen ) );
	MIN_ASSERT
	    ( ! min::is_direct_str ( indirectauxgen ) );
 
        cout << endl;
	cout << "Test index general values:" << endl;
	unsigned index = 734523;
	min::gen indexgen =
	    MUP::new_index_gen ( index );
	cout << "indexgen: "
	     << print_gen ( indexgen ) << endl;
	MIN_ASSERT ( min::is_index ( indexgen ) );
	MIN_ASSERT
	    (    min::gen_subtype_of ( indexgen )
	      == min::GEN_INDEX );
	MIN_ASSERT
	    ( min::index_of ( indexgen ) == index );
	desire_success (
	    indexgen = min::new_index_gen ( index );
	);
	desire_failure (
	    indexgen = min::new_index_gen
		( (min::unsgen) 1 << min::VSIZE );
	);
	MIN_ASSERT ( ! min::is_stub ( indexgen ) );
	MIN_ASSERT
	    ( ! min::is_direct_str ( indexgen ) );
 
        cout << endl;
	cout << "Test control code general values:"
	     << endl;
	unsigned code = 734523;
	min::gen codegen =
	    MUP::new_control_code_gen ( code );
	cout << "codegen: "
	     << print_gen ( codegen ) << endl;
	MIN_ASSERT ( min::is_control_code ( codegen ) );
	MIN_ASSERT
	    (    min::gen_subtype_of ( codegen )
	      == min::GEN_CONTROL_CODE );
	MIN_ASSERT
	    (    min::control_code_of ( codegen )
	      == code );
	desire_success (
	    codegen =
	        min::new_control_code_gen ( code );
	);
	desire_failure (
	    codegen = min::new_control_code_gen
		( (min::unsgen) 1 << min::VSIZE );
	);
	MIN_ASSERT ( ! min::is_stub ( codegen ) );
	MIN_ASSERT ( ! min::is_direct_str ( codegen ) );
 
        cout << endl;
	cout << "Test special general values:"
	     << endl;

	MIN_ASSERT
	    ( min::is_special ( min::MISSING ) );
	MIN_ASSERT
	    ( min::is_special ( min::ANY ) );
	MIN_ASSERT
	    ( min::is_special ( min::MULTI_VALUED ) );
	MIN_ASSERT
	    ( min::is_special ( min::UNDEFINED ) );
	MIN_ASSERT
	    ( min::is_special ( min::SUCCESS ) );
	MIN_ASSERT
	    ( min::is_special ( min::FAILURE ) );

	unsigned special = 542492;
	min::gen specialgen =
	    MUP::new_special_gen ( special );
	cout << "specialgen: "
	     << print_gen ( specialgen ) << endl;
	MIN_ASSERT ( min::is_special ( specialgen ) );
	MIN_ASSERT
	    (    min::gen_subtype_of ( specialgen )
	      == min::GEN_SPECIAL );
	MIN_ASSERT
	    (    min::special_index_of ( specialgen )
	      == special );
	desire_success (
	    specialgen =
	        min::new_special_gen ( special );
	);
	desire_failure (
	    specialgen =
	        min::new_special_gen
		    ( (min::unsgen) 1 << min::VSIZE );
	);
	MIN_ASSERT ( ! min::is_stub ( specialgen ) );
	MIN_ASSERT
	    ( ! min::is_direct_str ( specialgen ) );

	cout << endl;
	cout << "Finish General Value Constructor/"
	        "/Test/Read Function Test!" << endl;
    }

// Control Values
// ------- ------

    {
	cout << endl;
	cout << "Start Control Value Test!" << endl;
    	min::uns64 hiflag = min::uns64(1) << 55;
    	min::uns64 midflag = min::uns64(1) << 51;
    	min::uns64 loflag = min::uns64(1) << 48;
    	min::uns64 flags  = hiflag | loflag;
    	min::uns64 v1 = 73458439;
    	min::uns64 v2 = 83670280;
	int type1 = -123;
	int type2 = 127;
	void * p1 = (void *) 353456321;
	void * p2 = (void *) 651946503;
	static char stubsarea[3*sizeof(min::stub)];
        min::unsptr stubp = min::unsptr ( stubsarea );
	stubp += sizeof (min::stub) - 1;
	stubp &= ~ (sizeof (min::stub) - 1 ) ;
	min::stub * stubs = (min::stub * ) stubp;
	min::stub * stub1 = & stubs[0];
	min::stub * stub2 = & stubs[1];

	cout << endl;
	cout << "Test controls sans stub addresses:"
	     << endl;
	min::uns64 control1 =
	    MUP::new_control_with_type
	        ( type1, v1, flags );
	cout << "control1: " << hex << control1 << dec
	     << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control1 )
	      == type1 );
        MIN_ASSERT
	    (    MUP::value_of_control ( control1 )
	      == v1 );
        MIN_ASSERT ( control1 & hiflag );
        MIN_ASSERT ( control1 & loflag );
        MIN_ASSERT ( ! ( control1 & midflag ) );

	control1 =
	    MUP::renew_control_type ( control1, type2 );
	cout << "re-control1: " << hex << control1
	     << dec << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control1 )
	      == type2 );
        MIN_ASSERT
	    (    MUP::value_of_control ( control1 )
	      == v1 );
        MIN_ASSERT ( control1 & hiflag );
        MIN_ASSERT ( control1 & loflag );
        MIN_ASSERT ( ! ( control1 & midflag ) );

	control1 =
	    MUP::renew_control_value ( control1, v2 );
	cout << "re-control1: " << hex << control1
	     << dec << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control1 )
	      == type2 );
        MIN_ASSERT
	    (    MUP::value_of_control ( control1 )
	      == v2 );
        MIN_ASSERT ( control1 & hiflag );
        MIN_ASSERT ( control1 & loflag );
        MIN_ASSERT ( ! ( control1 & midflag ) );

	cout << endl;
	cout << "Test non-acc controls with stub"
	        " addresses:" << endl;
	min::uns64 control2 =
	    MUP::new_control_with_type
	        ( type1, stub1, hiflag );
	min::uns64 stubbase =
	    control2 & MIN_CONTROL_VALUE_MASK;
	cout << "control2: " << hex
	     << control2 - stubbase << dec << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control2 )
	      == type1 );
        MIN_ASSERT
	    (    MUP::stub_of_control ( control2 )
	      == stub1 );
        MIN_ASSERT ( control2 & hiflag );
        MIN_ASSERT ( ! ( control2 & loflag ) );
        MIN_ASSERT ( ! ( control2 & midflag ) );

	control2 =
	    MUP::renew_control_stub
		( control2, stub2 );
	cout << "re-control2: " << hex
	     << control2 - stubbase << dec << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control2 )
	      == type1 );
        MIN_ASSERT
	    (    MUP::stub_of_control ( control2 )
	      == stub2 );
        MIN_ASSERT ( control2 & hiflag );
        MIN_ASSERT ( ! ( control2 & loflag ) );
        MIN_ASSERT ( ! ( control2 & midflag ) );

	cout << endl;
	cout << "Test acc controls with stub"
	        " addresses:" << endl;
	min::uns64 control3 =
	    MUP::new_acc_control
	    	( type1, stub1, hiflag );
	stubbase =
	    control3 & MIN_ACC_CONTROL_VALUE_MASK;
	cout << "control3: " << hex
	     << control3 - stubbase << dec << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control3 )
	      == type1 );
        MIN_ASSERT
	    (    MUP::stub_of_acc_control ( control3 )
	      == stub1 );
        MIN_ASSERT ( control3 & hiflag );
        MIN_ASSERT ( ! ( control3 & loflag ) );
        MIN_ASSERT ( ! ( control3 & midflag ) );

	control3 =
	    MUP::renew_acc_control_stub
		( control3, stub2 );
	cout << "re-control3: " << hex
	     << control3 - stubbase << dec << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control3 )
	      == type1 );
        MIN_ASSERT
	    (    MUP::stub_of_acc_control ( control3 )
	      == stub2 );
        MIN_ASSERT ( control3 & hiflag );
        MIN_ASSERT ( ! ( control3 & loflag ) );
        MIN_ASSERT ( ! ( control3 & midflag ) );

	cout << endl;
	cout << "Finish Control Value Test!" << endl;
    }

// Stub Functions
// ---- ---------

    {
	cout << endl;
	cout << "Start Stub Functions Test!" << endl;
	static min::stub stubs[1];
	min::stub * stub = & stubs[0];

        cout << endl;
	cout << "Test stub value set/read functions:"
	     << endl;
	min::uns64 u = 0x9047814326432464ull;
	cout << "u: " << hex << u << dec << endl;
	MUP::set_value_of ( stub, u );
	MIN_ASSERT ( MUP::value_of ( stub ) == u );

	min::float64 f = 1.4362346234;
	MUP::set_float_of ( stub, f );
	MIN_ASSERT ( MUP::float_of ( stub ) == f );

	min::gen g = min::new_gen ( stub );
	MUP::set_gen_of ( stub, g );
	MIN_ASSERT ( MUP::gen_of ( stub ) == g );

	void * p = & g;
	MUP::set_pointer_of ( stub, p );
	MIN_ASSERT ( MUP::pointer_of ( stub ) == p );

        cout << endl;
	cout << "Test stub control set/read functions:"
	     << endl;
	min::uns64 f1 = min::uns64(1) << 55;
	min::uns64 f2 = min::uns64(1) << 48;
	min::uns64 c = f1 | f2;
	cout << "c: " << hex << c << dec << endl;
	MUP::set_control_of ( stub, c );
	MIN_ASSERT ( MUP::control_of ( stub ) == c );
	MIN_ASSERT ( min::type_of ( stub ) == 0 );
	MUP::set_type_of ( stub, min::NUMBER );
	c = MUP::renew_control_type ( c, min::NUMBER );
	cout << "c: " << hex << c << dec << endl;
	MIN_ASSERT ( MUP::control_of ( stub ) == c );
	MIN_ASSERT
	    ( min::type_of ( stub ) == min::NUMBER );

        cout << endl;
	cout << "Test stub flag set/clear/read"
	        " functions:" << endl;
	MIN_ASSERT ( MUP::test_flags_of ( stub, f1 ) );
	MIN_ASSERT ( MUP::test_flags_of ( stub, f2 ) );
	MUP::clear_flags_of ( stub, f2 );
	MIN_ASSERT ( MUP::test_flags_of ( stub, f1 ) );
	MIN_ASSERT
	    ( ! MUP::test_flags_of ( stub, f2 ) );
	MUP::set_flags_of ( stub, f2 );
	MIN_ASSERT ( MUP::test_flags_of ( stub, f1 ) );
	MIN_ASSERT ( MUP::test_flags_of ( stub, f2 ) );
	MIN_ASSERT ( MUP::control_of ( stub ) == c );

        cout << endl;
	cout << "Test stub ACC related functions:"
	     << endl;
	MIN_ASSERT
	    ( min::is_collectible ( min::NUMBER ) );
	MIN_ASSERT
	    ( ! min::is_collectible ( min::LIST_AUX ) );
	MUP::set_type_of ( stub, min::NUMBER );
	MIN_ASSERT ( ! min::is_deallocated ( stub ) );
	MUP::set_type_of ( stub, min::DEALLOCATED );
	MIN_ASSERT ( min::is_deallocated ( stub ) );

	cout << endl;
	cout << "Finish Stub Functions Test!" << endl;
    }

// Process Interface
// ------- ---------
    {
	cout << endl;
	cout << "Start Process Interface Test!" << endl;
	MINT::relocated_flag = false;

	// Process control testing is TBD.

        cout << endl;
	cout << "Test interrupt function:" << endl;
	interrupt_called = false;
	min::interrupt();
	MIN_ASSERT ( ! interrupt_called );
	min::stub ** limit_save = MINT::acc_stack_limit;
	MINT::acc_stack_limit = MINT::acc_stack;
	min::interrupt();
	MIN_ASSERT ( interrupt_called );
	MINT::acc_stack_limit = limit_save;

        cout << endl;
	cout << "Test relocate flag functions:"
	     << endl;
	MIN_ASSERT ( ! min::relocated_flag() );
	MIN_ASSERT
	    ( ! min::set_relocated_flag ( true ) );
	MIN_ASSERT ( min::relocated_flag() );
	MIN_ASSERT
	    ( min::set_relocated_flag ( false ) );
	MIN_ASSERT ( ! min::relocated_flag() );

	// Now relocated flag is false.
	{
	    min::relocated r;
	    MIN_ASSERT ( ! r );
	    MIN_ASSERT ( ! min::relocated_flag() );
	    min::set_relocated_flag ( true );
	    MIN_ASSERT ( r );
	    MIN_ASSERT ( ! min::relocated_flag() );
	}
	MIN_ASSERT ( min::relocated_flag() );

	// Now relocated flag is true.
	{
	    min::relocated r;
	    MIN_ASSERT ( ! min::relocated_flag() );
	    MIN_ASSERT ( ! r );
	    MIN_ASSERT ( ! min::relocated_flag() );
	}
	MIN_ASSERT ( min::relocated_flag() );

	cout << endl;
	cout << "Finish Process Interface Test!"
	     << endl;
    }

// Allocator/Collector/Compactor Interface
// ----------------------------- ---------

    {
	cout << endl;
	cout << "Start Allocator/Collector/Compactor"
	        " Interface Test!" << endl;

	static min::stub s1, s2;
	const min::uns64 unmarked_flag =
	       min::uns64(1)
	    << ( 56 - MIN_ACC_FLAG_BITS );
	const min::uns64 scavenged_flag =
	    unmarked_flag << 1;
	MINT::acc_stack_mask = unmarked_flag;

        cout << endl;
	cout << "Test mutator functions:"
	     << endl;
	MUP::set_control_of ( &s1, 0 );
	MUP::set_flags_of ( &s1, scavenged_flag );
	MUP::set_control_of ( &s2, 0 );
	MUP::set_flags_of ( &s2, unmarked_flag );
	MINT::acc_stack_mask = 0;
	MUP::acc_write_update ( &s1, &s2 );
	MIN_ASSERT ( MINT::acc_stack == ::acc_stack );
	MINT::acc_stack_mask = unmarked_flag;
	MUP::acc_write_update ( &s1, &s2 );
	MIN_ASSERT
	    ( MINT::acc_stack == ::acc_stack + 2 );
	MIN_ASSERT ( ::acc_stack[0] == &s1 );
	MIN_ASSERT ( ::acc_stack[1] == &s2 );
	MUP::clear_flags_of ( &s1, scavenged_flag );
	MUP::acc_write_update ( &s1, &s2 );
	MIN_ASSERT
	    ( MINT::acc_stack == ::acc_stack + 2 );
	MUP::set_flags_of ( &s1, scavenged_flag );
	MUP::clear_flags_of ( &s2, unmarked_flag );
	MUP::acc_write_update ( &s1, &s2 );
	MIN_ASSERT
	    ( MINT::acc_stack == ::acc_stack + 2 );

        cout << endl;
	cout << "Test stub allocator functions:"
	     << endl;
	MINT::new_acc_stub_flags = 0;
	min::unsptr sbase = stubs_allocated;
	cout << "initial stubs allocated = "
	     << sbase << endl;
	min::stub * stub1 = MUP::new_acc_stub();
	MIN_ASSERT
	    ( stub1 == begin_stub_region + sbase  );
	MIN_ASSERT
	    ( stub1 == MINT::last_allocated_stub );
	MIN_ASSERT ( stubs_allocated == sbase + 1 );
	MIN_ASSERT
	    ( min::type_of ( stub1 ) == min::ACC_FREE );
	MIN_ASSERT
	    ( ! MUP::test_flags_of
	    	     ( stub1, unmarked_flag ) );
	MINT::new_acc_stub_flags = unmarked_flag;
	min::stub * stub2 = MUP::new_acc_stub();
	MIN_ASSERT
	    ( stub2 == MINT::last_allocated_stub );
	MIN_ASSERT ( stubs_allocated == sbase + 2 );
	MIN_ASSERT
	    ( stub2 == begin_stub_region + sbase + 1 );
	MIN_ASSERT
	    ( min::type_of ( stub2 ) == min::ACC_FREE );
	MIN_ASSERT
	    ( MUP::test_flags_of
	    	     ( stub2, unmarked_flag ) );
	MINT::acc_expand_stub_free_list ( 2 );
	MIN_ASSERT ( stubs_allocated == sbase + 4 );
	MIN_ASSERT
	    ( stub2 == MINT::last_allocated_stub );
	min::stub * stub3 = MUP::new_aux_stub();
	MIN_ASSERT
	    ( stub3 == begin_stub_region + sbase + 3 );
	MIN_ASSERT ( stubs_allocated == sbase + 4 );
	MIN_ASSERT
	    ( stub2 == MINT::last_allocated_stub );
	min::stub * stub4 = MUP::new_acc_stub();
	MIN_ASSERT
	    ( stub4 == begin_stub_region + sbase + 2 );
	MIN_ASSERT ( stubs_allocated == sbase + 4 );
	MIN_ASSERT
	    ( stub4 == MINT::last_allocated_stub );


        cout << endl;
	cout << "Test body allocator functions:"
	     << endl;
	MUP::new_body ( stub1, 128 );
	char * p1 = (char *) MUP::pointer_of ( stub1 );
	memset ( p1, 0xBB, 128 );
	MUP::new_body ( stub2, 128 );
	char * p2 = (char *) MUP::pointer_of ( stub2 );
	memset ( p2, 0xBB, 128 );
	MIN_ASSERT ( memcmp ( p1, p2, 128 ) == 0 );
	MIN_ASSERT ( p1 != p2 );
	MUP::new_body ( stub3, 128 );
	char * p3 = (char *) MUP::pointer_of ( stub3 );
	memset ( p3, 0xCC, 128 );
	MUP::new_body ( stub4, 128 );
	char * p4 = (char *) MUP::pointer_of ( stub4 );
	memset ( p4, 0xCC, 128 );
	MIN_ASSERT ( memcmp ( p3, p4, 128 ) == 0 );
	MIN_ASSERT ( p3 != p4 );
	resize_body ( stub4, 128, 128 );
	char * p5 = (char *) MUP::pointer_of ( stub4 );
	MIN_ASSERT ( memcmp ( p3, p5, 128 ) == 0 );
	MIN_ASSERT ( p4 != p5 );
	MUP::deallocate_body ( stub4, 128 );
	MIN_ASSERT ( min::type_of ( stub4 )
	             == min::DEALLOCATED );
	char * p6 = (char *) MUP::pointer_of ( stub4 );
	MIN_ASSERT ( p5 != p6 );
	MIN_ASSERT ( p6[0] == 0
	             &&
		     memcmp ( p6, p6+1, 127 ) == 0 );

        cout << endl;
	cout << "Test General Value Locators:"
	     << endl;

	static min::static_gen<3> staticg1;
	static min::static_num_gen<5> staticg2;

	MIN_ASSERT
	    ( find_locator ( & staticg1[0],
	                     MINT::static_gen_last ) );
	MIN_ASSERT
	    ( find_locator ( & staticg2[0],
	                     MINT::static_gen_last )
	      == MIN_IS_COMPACT );
	MIN_ASSERT
	    ( count_locators ( MINT::static_gen_last )
	      == 1 + MIN_IS_COMPACT );
	{
	    MIN_ASSERT ( MINT::stack_gen_last == NULL );
	    min::stack_gen<2> stackg1;
	    MIN_ASSERT
		( find_locator
		      ( & stackg1[0],
			MINT::stack_gen_last ) );
	    MIN_ASSERT
		( count_locators
		      ( MINT::stack_gen_last )
		  == 1 );
	    {
		min::stack_num_gen<10> stackg2;
		MIN_ASSERT
		    ( find_locator
			  ( & stackg1[0],
			    MINT::stack_gen_last ) );
		MIN_ASSERT
		    ( find_locator
			  ( & stackg2[0],
			    MINT::stack_gen_last )
		      == MIN_IS_COMPACT );
		MIN_ASSERT
		    ( count_locators
			  ( MINT::stack_gen_last )
		      == 1 + MIN_IS_COMPACT );
	    }
	    MIN_ASSERT
		( find_locator
		      ( & stackg1[0],
			MINT::stack_gen_last ) );
	    MIN_ASSERT
		( count_locators
		      ( MINT::stack_gen_last )
		  == 1 );
	}

	cout << endl;
	cout << "Finish Allocator/Collector/Compactor"
	        " Interface Test!" << endl;
    }

// Numbers
// -------

    {
	cout << endl;
	cout << "Start Numbers Test!" << endl;

	cout << endl;
	cout << "Test number create/test/read"
	        " functions:" << endl;

	min::gen n1 = min::new_num_gen ( 12345 );
	cout << "n1: " << print_gen ( n1 ) << endl;
	MIN_ASSERT ( min::is_num ( n1 ) );
	MIN_ASSERT ( min::is_name ( n1 ) );
	MIN_ASSERT ( min::int_of ( n1 ) == 12345 );
	MIN_ASSERT ( min::float_of ( n1 ) == 12345 );
	MIN_ASSERT ( MUP::float_of ( n1 ) == 12345 );
	min::uns32 n1hash = min::numhash ( n1 );
	cout << "n1hash: " << hex << n1hash << dec
	     << endl;
	MIN_ASSERT
	    ( n1hash == min::floathash ( 12345 ) );
	MIN_ASSERT
	    ( n1hash == min::hash ( n1 ) );
	MIN_ASSERT ( min::new_num_gen ( 12345 ) == n1 );

	min::gen n2 = min::new_num_gen ( 1.2345 );
#	if MIN_IS_LOOSE
	    cout << "n2: " << print_gen ( n2 ) << endl;
#	endif
	MIN_ASSERT ( min::is_num ( n2 ) );
	MIN_ASSERT ( min::is_name ( n2 ) );
	MIN_ASSERT ( min::float_of ( n2 ) == 1.2345 );
	MIN_ASSERT ( MUP::float_of ( n2 ) == 1.2345 );
	min::uns32 n2hash = min::numhash ( n2 );
	cout << "n2hash: " << hex << n2hash << dec
	     << endl;
	MIN_ASSERT
	    ( n2hash == min::floathash ( 1.2345 ) );
	MIN_ASSERT
	    ( n2hash == min::hash ( n2 ) );
	MIN_ASSERT
	    ( min::new_num_gen ( 1.2345 ) == n2 );

	min::gen n3 = min::new_num_gen ( 1 << 30 );
#	if MIN_IS_LOOSE
	    cout << "n3: " << print_gen ( n3 ) << endl;
#	endif
	MIN_ASSERT ( min::is_num ( n3 ) );
	MIN_ASSERT ( min::is_name ( n3 ) );
	MIN_ASSERT ( min::int_of ( n3 ) == 1 << 30 );
	MIN_ASSERT ( min::float_of ( n3 ) == 1 << 30 );
	MIN_ASSERT ( MUP::float_of ( n3 ) == 1 << 30 );
	min::uns32 n3hash = min::numhash ( n3 );
	cout << "n3hash: " << hex << n3hash << dec
	     << endl;
	MIN_ASSERT
	    ( n3hash == min::floathash ( 1 << 30 ) );
	MIN_ASSERT
	    ( n3hash == min::hash ( n3 ) );
	MIN_ASSERT
	    ( min::new_num_gen ( 1 << 30 ) == n3 );

	cout << endl;
	cout << "Finish Numbers Test!" << endl;
    }

// Strings

    {
	cout << endl;
	cout << "Start Strings Test!" << endl;

	char buffer[20];
	union { min::uns64 str; char buf[9]; } u;
	const char * s13 = "ABCDEFGHIJKLM";
	const char * s8 = "ABCDEFGH";
	const char * s7 = "ABCDEFG";
	const char * s3 = "ABC";

	cout << endl;
	cout << "Test string hash:" << endl;
	min::uns32 s13hash = min::strhash ( s13 );
	min::uns32 s8hash = min::strhash ( s8 );
	min::uns32 s7hash = min::strhash ( s7 );
	min::uns32 s3hash = min::strhash ( s3 );
	cout << "s13hash: " << hex << s13hash << dec
	     << endl;
	cout << "s8hash: " << hex << s8hash << dec
	     << endl;
	cout << "s7hash: " << hex << s7hash << dec
	     << endl;
	cout << "s3hash: " << hex << s3hash << dec
	     << endl;
	MIN_ASSERT
	    ( min::strnhash ( s13, 8 ) == s8hash );
	MIN_ASSERT
	    ( min::strnhash ( s13, 3 ) == s3hash );

	cout << endl;
	cout << "Test string general values:" << endl;
	min::gen strgen3 = min::new_str_gen ( s3 );
	min::gen strgen7 = min::new_str_gen ( s7 );
	min::gen strgen8 = min::new_str_gen ( s8 );
	min::gen strgen13 = min::new_str_gen ( s13 );
	MIN_ASSERT (    min::new_str_gen ( s13, 8 )
	             == strgen8 );
	MIN_ASSERT (    min::new_str_gen ( s13, 20 )
	             == strgen13 );

	MIN_ASSERT ( min::is_str ( strgen3 ) );
	MIN_ASSERT ( min::is_name ( strgen3 ) );
	MIN_ASSERT ( min::is_direct_str ( strgen3 ) );
	MIN_ASSERT ( min::is_str ( strgen7 ) );
	MIN_ASSERT ( min::is_name ( strgen7 ) );
	MIN_ASSERT ( min::is_stub ( strgen7 ) );
	MIN_ASSERT ( min::is_str ( strgen8 ) );
	MIN_ASSERT ( min::is_name ( strgen8 ) );
	MIN_ASSERT ( min::is_stub ( strgen8 ) );
	MIN_ASSERT ( min::is_str ( strgen13 ) );
	MIN_ASSERT ( min::is_name ( strgen13 ) );
	MIN_ASSERT ( min::is_stub ( strgen13 ) );

	MIN_ASSERT ( min::strlen ( strgen3 ) == 3 );
	MIN_ASSERT
	    ( min::strhash ( strgen3 ) == s3hash );
	MIN_ASSERT
	    ( min::hash ( strgen3 ) == s3hash );
	min::strcpy ( buffer, strgen3 );
	MIN_ASSERT ( strcmp ( buffer, s3 ) == 0 );
	MIN_ASSERT ( min::strcmp ( s3, strgen3 ) == 0 );
	MIN_ASSERT
	    ( min::new_str_gen ( buffer ) == strgen3 );
	buffer[2] = 0;
	min::strncpy ( buffer, strgen3, 2 );
	MIN_ASSERT ( buffer[2] == 0 );
	MIN_ASSERT
	    (    min::strncmp ( buffer, strgen3, 2 )
	      == 0 );
	MIN_ASSERT ( min::strlen ( strgen7 ) == 7 );
	MIN_ASSERT
	    ( min::strhash ( strgen7 ) == s7hash );
	MIN_ASSERT
	    ( min::hash ( strgen7 ) == s7hash );
	min::strcpy ( buffer, strgen7 );
	MIN_ASSERT ( strcmp ( buffer, s7 ) == 0 );
	MIN_ASSERT ( min::strcmp ( s7, strgen7 ) == 0 );
	MIN_ASSERT
	    ( min::new_str_gen ( buffer ) == strgen7 );
	buffer[6] = 0;
	min::strncpy ( buffer, strgen7, 6 );
	MIN_ASSERT ( buffer[6] == 0 );
	MIN_ASSERT
	    (    min::strncmp ( buffer, strgen7, 6 )
	      == 0 );
	MIN_ASSERT ( min::strlen ( strgen8 ) == 8 );
	MIN_ASSERT
	    ( min::strhash ( strgen8 ) == s8hash );
	MIN_ASSERT
	    ( min::hash ( strgen8 ) == s8hash );
	min::strcpy ( buffer, strgen8 );
	MIN_ASSERT ( strcmp ( buffer, s8 ) == 0 );
	MIN_ASSERT ( min::strcmp ( s8, strgen8 ) == 0 );
	MIN_ASSERT
	    ( min::new_str_gen ( buffer ) == strgen8 );
	buffer[7] = 0;
	min::strncpy ( buffer, strgen8, 7 );
	MIN_ASSERT ( buffer[7] == 0 );
	MIN_ASSERT
	    (    min::strncmp ( buffer, strgen8, 7 )
	      == 0 );
	MIN_ASSERT ( min::strlen ( strgen13 ) == 13 );
	MIN_ASSERT
	    ( min::strhash ( strgen13 ) == s13hash );
	MIN_ASSERT
	    ( min::hash ( strgen13 ) == s13hash );
	min::strcpy ( buffer, strgen13 );
	MIN_ASSERT ( strcmp ( buffer, s13 ) == 0 );
	MIN_ASSERT
	    ( min::strcmp ( s13, strgen13 ) == 0 );
	MIN_ASSERT
	    ( min::new_str_gen ( buffer ) == strgen13 );
	buffer[12] = 0;
	min::strncpy ( buffer, strgen13, 12 );
	MIN_ASSERT ( buffer[12] == 0 );
	MIN_ASSERT
	    (    min::strncmp ( buffer, strgen13, 12 )
	      == 0 );

	cout << endl;
	cout << "Test unprotected string functions:"
	     << endl;

	min::stub * stub7 = MUP::stub_of ( strgen7 );
	MIN_ASSERT (    min::type_of ( stub7 )
	             == min::SHORT_STR );
	u.str = MUP::short_str_of ( stub7 );
	u.buf[8] = 0;
	MIN_ASSERT ( strcmp ( u.buf, s7 ) == 0 );
	min::stub * stub8 = MUP::stub_of ( strgen8 );
	MIN_ASSERT (    min::type_of ( stub8 )
	             == min::SHORT_STR );
	u.str = MUP::short_str_of ( stub8 );
	u.buf[8] = 0;
	MIN_ASSERT ( strcmp ( u.buf, s8 ) == 0 );

	min::stub * stub13 = MUP::stub_of ( strgen13 );
	MIN_ASSERT (    min::type_of ( stub13 )
	             == min::LONG_STR );
	MUP::long_str * lstr13 =
	    MUP::long_str_of ( stub13 );
	MIN_ASSERT ( MUP::length_of ( lstr13 ) == 13 );
	MIN_ASSERT
	    ( MUP::hash_of ( lstr13 ) == s13hash );
	MIN_ASSERT
	    (    strcmp ( MUP::str_of ( lstr13 ), s13 )
	      == 0 );

	cout << endl;
	cout << "Test protected string pointers:"
	     << endl;

	// Test body relocation first.
	MIN_ASSERT
	    ( MUP::body_size_of ( stub13 )
	      ==
	      sizeof ( MUP::long_str ) + 13 + 1 );
	resize_body
	    ( stub13, MUP::body_size_of ( stub13 ),
	              MUP::body_size_of ( stub13 ) );
	MIN_ASSERT ( min::strlen ( strgen13 ) == 13 );
	MIN_ASSERT
	    ( min::strhash ( strgen13 ) == s13hash );
	min::strcpy ( buffer, strgen13 );
	MIN_ASSERT ( strcmp ( buffer, s13 ) == 0 );

	min::str_pointer p3 ( strgen3 );
	min::str_pointer p7 ( strgen7 );
	min::str_pointer p8 ( strgen8 );
	min::str_pointer p13 ( strgen13 );

	MIN_ASSERT ( min::strcmp ( s3, p3 ) == 0 );
	MIN_ASSERT ( min::strcmp ( s7, p7 ) == 0 );
	MIN_ASSERT ( min::strcmp ( s8, p8 ) == 0 );
	MIN_ASSERT ( min::strcmp ( s13, p13 ) == 0 );

	MIN_ASSERT ( s3[0] == p3[0] );
	MIN_ASSERT ( s3[1] == p3[1] );
	MIN_ASSERT ( s3[2] == p3[2] );
	MIN_ASSERT ( s3[3] == p3[3] );
	MIN_ASSERT ( s7[0] == p7[0] );
	MIN_ASSERT ( s7[6] == p7[6] );
	MIN_ASSERT ( s7[7] == p7[7] );
	MIN_ASSERT ( s8[0] == p8[0] );
	MIN_ASSERT ( s8[7] == p8[7] );
	MIN_ASSERT ( s8[8] == p8[8] );
	MIN_ASSERT ( s13[0] == p13[0] );
	MIN_ASSERT ( s13[12] == p13[12] );
	MIN_ASSERT ( s13[13] == p13[13] );

	min::strcpy ( buffer, p3 );
	MIN_ASSERT ( strcmp ( buffer, s3 ) == 0 );
	min::strcpy ( buffer, p13 );
	MIN_ASSERT ( strcmp ( buffer, s13 ) == 0 );
	buffer[5] = 0;
	MIN_ASSERT ( strncmp ( buffer, p13, 5 ) == 0 );
	MIN_ASSERT ( strncmp ( buffer, p13, 6 ) != 0 );
	buffer[4] = 0;
	buffer[5] = 'X';
	min::strncpy ( buffer, p13, 5 );
	MIN_ASSERT ( buffer[4] == s13[4] );
	MIN_ASSERT ( buffer[5] == 'X' );

	MIN_ASSERT ( min::strlen ( p3 ) == 3 );
	MIN_ASSERT ( min::strlen ( p7 ) == 7 );
	MIN_ASSERT ( min::strlen ( p8 ) == 8 );
	MIN_ASSERT ( min::strlen ( p13 ) == 13 );

	MIN_ASSERT (    min::strhash ( p3 )
	             == min::strhash ( s3 ) );
	MIN_ASSERT (    min::strhash ( p7 )
	             == min::strhash ( s7 ) );
	MIN_ASSERT (    min::strhash ( p8 )
	             == min::strhash ( s8 ) );
	MIN_ASSERT (    min::strhash ( p13 )
	             == min::strhash ( s13 ) );

	const char * p13str_before =
	    min::unprotected::str_of ( p13 );
	MIN_ASSERT
	    ( strcmp ( p13str_before, s13 ) == 0 );
	resize_body
	    ( stub13, MUP::body_size_of ( stub13 ),
	              MUP::body_size_of ( stub13 ) );
	const char * p13str_after =
	    min::unprotected::str_of ( p13 );
	MIN_ASSERT ( p13str_after != p13str_before );
	MIN_ASSERT ( min::strcmp ( s13, p13 ) == 0 );

	min::str_pointer p ( strgen13 );
	MIN_ASSERT ( strcmp ( s13, p ) == 0 );
	min::initialize ( p, strgen8 );
	MIN_ASSERT ( strcmp ( s8, p ) == 0 );
	
	cout << endl;
	cout << "Finish Strings Test!" << endl;
    }

// Labels
// ------

    {
	cout << endl;
	cout << "Start Labels Test!" << endl;
	min::gen labv1[3], labv2[5];
	labv1[0] = min::new_str_gen ( "Hello" );
	labv1[1] = min::new_num_gen ( 55 );
	labv1[2] = min::new_str_gen ( "End" );

	cout << endl;
	cout << "Test label hash:" << endl;
	min::uns32 labhash1 = min::labhash (labv1, 3);
	cout << "labhash1: " << hex << labhash1 << dec
	     << endl;

	cout << endl;
	cout << "Test labels:" << endl;
	min::gen lab = min::new_lab_gen ( labv1, 3 );
	MIN_ASSERT ( min::is_lab ( lab ) );
	MIN_ASSERT ( min::is_name ( lab ) );
	MIN_ASSERT ( min::is_stub ( lab ) );
	const min::stub * s = min::stub_of ( lab );
	MIN_ASSERT ( min::labhash ( s ) == labhash1 );
	MIN_ASSERT ( min::lablen ( s ) == 3 );
	MIN_ASSERT ( min::labhash ( lab ) == labhash1 );
	MIN_ASSERT ( min::lablen ( lab ) == 3 );
	MIN_ASSERT ( MUP::body_size_of ( s )
	             ==
		     3 * sizeof ( min::gen )
		     +
		     sizeof ( MINT::lab_header ) );
	MIN_ASSERT ( min::hash ( lab ) == labhash1 );
	MIN_ASSERT
	    ( min::lab_of ( labv2, 5, s ) == 3 );
	MIN_ASSERT
	    ( min::new_lab_gen ( labv2, 3 ) == lab );
	MIN_ASSERT
	    ( min::lab_of ( labv2, 5, lab ) == 3 );
	MIN_ASSERT
	    ( min::new_lab_gen ( labv2, 3 ) == lab );
	
	cout << endl;
	cout << "Finish Labels Test!" << endl;
    }

// Objects
// -------

    // Values shared with subsequent object tests.
    //
    min::gen short_obj_gen;
    min::gen long_obj_gen;
    {
	cout << endl;
	cout << "Start Objects Test!" << endl;

	cout << "SHORT_OBJ_MAX_TOTAL_SIZE = 0x"
	    << hex << MINT::SHORT_OBJ_MAX_TOTAL_SIZE
	    << dec << endl;
	cout << "LONG_OBJ_MAX_TOTAL_SIZE = 0x"
	    << hex << MINT::LONG_OBJ_MAX_TOTAL_SIZE
	    << dec << endl;

	cout << endl;
	cout << "Test object size functions:" << endl;

	min::unsptr lastht = 0;
	min::unsptr maxht =
	    min::obj_hash_size ( min::unsptr(-1) );
	cout << "maxht: " << maxht << endl;
	MIN_ASSERT ( maxht >= 4000000 );
	// Note: maxht < ( 1 << 22 ) is possible.
	for ( min::unsptr u = 0; u <= 4000000; ++ u )
	{
	    // Use `assert' to avoid to many .out
	    // lines.

	    min::unsptr t = min::obj_total_size ( u );
	    assert ( u <= t );
	    min::unsptr ht = min::obj_hash_size ( u );
	    if ( u > ht )
	        assert ( ht == maxht );
	    else
	        assert ( lastht < u || lastht == ht );
	    lastht = ht;
	}

	cout << endl;
	cout << "Test short objects:" << endl;
	short_obj_gen = min::new_obj_gen ( 500, 100 );
	const min::stub * sstub =
	    min::stub_of ( short_obj_gen );
	MIN_ASSERT
	    (    min::type_of ( sstub )
	      == min::SHORT_OBJ );
	{
	    min::vec_pointer svp ( sstub );
	    min::unsptr sh = MUP::var_offset_of ( svp );
	    min::unsptr sht = min::hash_size_of ( svp );
	    min::unsptr sav = min::attr_size_of ( svp );
	    min::unsptr sua =
	        min::unused_size_of ( svp );
	    min::unsptr saa = min::aux_size_of ( svp );
	    min::unsptr st = min::total_size_of ( svp );
	    cout << "sh: " << sh << " sht: " << sht
		 << " sua: " << sua
		 << " sav: " << sav
		 << " saa: " << saa
		 << " st: " << st << endl;
	    MIN_ASSERT ( sht >= 100 );
	    MIN_ASSERT ( sua >= 500 );
	    MIN_ASSERT ( sav == 0 );
	    MIN_ASSERT ( saa == 0 );
	    MIN_ASSERT
	        ( st == sh + sht + sav + sua + saa );
	    MIN_ASSERT ( MUP::body_size_of ( sstub )
			 ==
			 st * sizeof ( min::gen ) );
	}

	cout << endl;
	cout << "Test long objects:" << endl;
	long_obj_gen = min::new_obj_gen ( 70000, 7000 );
	const min::stub * lstub =
	    min::stub_of ( long_obj_gen );
	MIN_ASSERT
	    ( min::type_of ( lstub ) == min::LONG_OBJ );
	{
	    min::vec_pointer lvp ( long_obj_gen );
	    min::uns32 lh = MUP::var_offset_of ( lvp );
	    min::uns32 lht = min::hash_size_of ( lvp );
	    min::uns32 lav = min::attr_size_of ( lvp );
	    min::uns32 lua =
	        min::unused_size_of ( lvp );
	    min::uns32 laa = min::aux_size_of ( lvp );
	    min::uns32 lt = min::total_size_of ( lvp );
	    cout << "lh: " << lh << " lht: " << lht
		 << " lua: " << lua
		 << " lav: " << lav
		 << " laa: " << laa
		 << " lt: " << lt << endl;
	    MIN_ASSERT ( lht >= 7000 );
	    MIN_ASSERT ( lua >= 70000 );
	    MIN_ASSERT ( lav == 0 );
	    MIN_ASSERT ( laa == 0 );
	    MIN_ASSERT
	        ( lt == lh + lht + lav + lua + laa );
	    MIN_ASSERT ( MUP::body_size_of ( lstub )
			 ==
			 lt * sizeof ( min::gen ) );
	}

	cout << endl;
	cout << "Finish Objects Test!" << endl;
    }

// Object Vector Level

    // Values shared with previous object tests.
    // min::gen short_obj_gen;
    // min::gen long_obj_gen;
    {
	cout << endl;
	cout << "Start Object Vector Level Test!"
	     << endl;
	min::gen num0 = min::new_num_gen ( 0 );
	min::gen num1 = min::new_num_gen ( 1 );
	min::gen num2 = min::new_num_gen ( 2 );
	min::gen num3 = min::new_num_gen ( 3 );
	min::gen numv[3] = { num1, num2, num3 };
	min::gen fillv[70000];
	for ( int i = 0; i < 70000; ++ i )
	    fillv[i] = num0;
	min::gen outv[4];

	cout << endl;
	cout << "Test short object vector level:"
	     << endl;
	min::stub * sstub =
	    MUP::stub_of ( short_obj_gen );
	{
	    min::insertable_vec_pointer svp ( sstub );
	    min::gen * & sbase = MUP::base ( svp );
	    min::unsptr ht =
	        MUP::hash_offset_of ( svp );
	    min::unsptr av =
	        MUP::attr_offset_of ( svp );
	    min::unsptr & cua =
	        MUP::unused_offset_of ( svp );
	    min::unsptr unused_size =
		min::unused_size_of ( svp );
	    MIN_ASSERT ( sbase[ht] == min::LIST_END );
	    min::set_hash
	        ( svp, 0, min::EMPTY_SUBLIST );
	    MIN_ASSERT
	        ( sbase[ht] == min::EMPTY_SUBLIST );
	    MIN_ASSERT
	        (    min::hash(svp,0)
		  == min::EMPTY_SUBLIST );
	    MIN_ASSERT ( unused_size >= 500 );
	    sbase[av] = num0;
	    MIN_ASSERT
	        ( sbase[av+0] == num0 );
	    MIN_ASSERT
	        ( min::attr_size_of ( svp ) == 0 );
	    MIN_ASSERT ( cua == av + 0 );
	    min::attr_push ( svp, num1 );
	    MIN_ASSERT ( sbase[av] == num1 );
	    MIN_ASSERT ( attr ( svp, 0 ) == num1 );
	    MIN_ASSERT
	        ( min::attr_size_of ( svp ) == 1 );
	    MIN_ASSERT ( cua == av + 1 );
	    sbase[av+1] = num0;
	    sbase[av+2] = num0;
	    sbase[av+3] = num0;
	    MIN_ASSERT ( sbase[av+1] == num0 );
	    MIN_ASSERT ( sbase[av+2] == num0 );
	    MIN_ASSERT ( sbase[av+3] == num0 );
	    min::attr_push ( svp, numv, 3 );
	    MIN_ASSERT ( sbase[av+1] == num1 );
	    MIN_ASSERT ( sbase[av+2] == num2 );
	    MIN_ASSERT ( sbase[av+3] == num3 );
	    MIN_ASSERT ( attr ( svp, 3 ) == num3 );
	    MIN_ASSERT
	        ( min::attr_size_of ( svp ) == 4 );
	    MIN_ASSERT ( cua == av + 4 );
	    MIN_ASSERT
		(    min::unused_size_of ( svp )
		  == unused_size - 4 );

	    min::unsptr aa = MUP::aux_offset_of ( svp );
	    min::unsptr & caa =
	        MUP::aux_offset_of ( svp );
	    sbase[aa-1] = num0;
	    MIN_ASSERT ( sbase[aa-1] == num0 );
	    MIN_ASSERT
		( min::aux_size_of ( svp ) == 0 );
	    MIN_ASSERT ( caa == aa );
	    min::aux_push ( svp, num1 );
	    MIN_ASSERT ( sbase[aa-1] == num1 );
	    MIN_ASSERT
	        ( min::aux ( svp, aa-1 ) == num1 );
	    MIN_ASSERT
		( min::aux_size_of ( svp ) == 1 );
	    MIN_ASSERT ( caa == aa - 1 );
	    sbase[aa-2] = num0;
	    sbase[aa-3] = num0;
	    sbase[aa-4] = num0;
	    MIN_ASSERT ( sbase[aa-2] == num0 );
	    MIN_ASSERT ( sbase[aa-3] == num0 );
	    MIN_ASSERT ( sbase[aa-4] == num0 );
	    min::aux_push ( svp, numv, 3 );
	    MIN_ASSERT ( sbase[aa-4] == num1 );
	    MIN_ASSERT ( sbase[aa-3] == num2 );
	    MIN_ASSERT ( sbase[aa-2] == num3 );
	    MIN_ASSERT ( aux ( svp, aa-2 ) == num3 );
	    MIN_ASSERT
		( min::aux_size_of ( svp ) == 4 );
	    MIN_ASSERT ( caa == aa - 4 );
	    MIN_ASSERT
		(    min::unused_size_of ( svp )
		  == unused_size - 8 );

	    min::attr_pop ( svp, outv + 1, 3 );
	    min::attr_pop ( svp, outv[0] );
	    MIN_ASSERT ( outv[0] == num1 );
	    MIN_ASSERT ( outv[1] == num1 );
	    MIN_ASSERT ( outv[2] == num2 );
	    MIN_ASSERT ( outv[3] == num3 );
	    MIN_ASSERT
	        ( min::attr_size_of ( svp ) == 0 );
	    MIN_ASSERT ( cua == av + 0 );
	    MIN_ASSERT
		(    min::unused_size_of ( svp )
		  == unused_size - 4 );
	    desire_failure (
		min::attr_pop ( svp, outv[0] );
	    );
	    desire_failure (
		min::attr_pop ( svp, outv + 1, 3 );
	    );
	    min::attr_push ( svp, fillv, 4 );

	    min::aux_pop ( svp, outv + 1, 3 );
	    min::aux_pop ( svp, outv[0] );
	    MIN_ASSERT ( outv[0] == num1 );
	    MIN_ASSERT ( outv[1] == num1 );
	    MIN_ASSERT ( outv[2] == num2 );
	    MIN_ASSERT ( outv[3] == num3 );
	    MIN_ASSERT
		(    min::aux_size_of ( svp )
		  == 0 );
	    MIN_ASSERT ( caa == aa );
	    MIN_ASSERT
		(    min::unused_size_of ( svp )
		  == unused_size - 4 );
	    desire_failure (
		min::aux_pop ( svp, outv[0] );
	    );
	    desire_failure (
		min::aux_pop ( svp, outv + 1, 3 );
	    );
	    min::aux_push ( svp, fillv, 4 );

	    min::attr_push ( svp, fillv, 250 - 4 );
	    min::aux_push ( svp, fillv,
	                    unused_size - 250 - 4 );
	    MIN_ASSERT
	        ( min::unused_size_of ( svp ) == 0 );
	    MIN_ASSERT ( cua == caa );
	    desire_failure (
		min::attr_push ( svp, num3 );
	    );
	    desire_failure (
		min::aux_push ( svp, num3 );
	    );
	}

	cout << endl;
	cout << "Test long object vector level:"
	     << endl;
	{
	    min::insertable_vec_pointer lvp
	        ( long_obj_gen );
	    min::gen * & lbase = MUP::base ( lvp );
	    min::unsptr ht =
	        MUP::hash_offset_of ( lvp );
	    min::unsptr av =
	        MUP::attr_offset_of ( lvp );
	    min::unsptr & cua =
	        MUP::unused_offset_of ( lvp );
	    min::unsptr unused_size =
		min::unused_size_of ( lvp );
	    MIN_ASSERT ( lbase[ht] == min::LIST_END );
	    min::set_hash
	        ( lvp, 0, min::EMPTY_SUBLIST );
	    MIN_ASSERT
	        ( lbase[ht] == min::EMPTY_SUBLIST );
	    MIN_ASSERT
	        (    min::hash(lvp,0)
		  == min::EMPTY_SUBLIST );
	    MIN_ASSERT ( unused_size >= 70000 );
	    lbase[av] = num0;
	    MIN_ASSERT ( lbase[av + 0] == num0 );
	    MIN_ASSERT
	        ( min::attr_size_of ( lvp ) == 0 );
	    min::attr_push ( lvp, num1 );
	    MIN_ASSERT ( lbase[av] == num1 );
	    MIN_ASSERT
	        ( min::attr_size_of ( lvp ) == 1 );
	    MIN_ASSERT ( cua == av + 1 );
	    lbase[av+1] = num0;
	    lbase[av+2] = num0;
	    lbase[av+3] = num0;
	    MIN_ASSERT ( lbase[av+1] == num0 );
	    MIN_ASSERT ( lbase[av+2] == num0 );
	    MIN_ASSERT ( lbase[av+3] == num0 );
	    min::attr_push ( lvp, numv, 3 );
	    MIN_ASSERT ( lbase[av+1] == num1 );
	    MIN_ASSERT ( lbase[av+2] == num2 );
	    MIN_ASSERT ( lbase[av+3] == num3 );
	    MIN_ASSERT ( attr ( lvp, 3 ) == num3 );
	    MIN_ASSERT
	        ( min::attr_size_of ( lvp ) == 4 );
	    MIN_ASSERT ( cua == av + 4 );
	    MIN_ASSERT
		(    min::unused_size_of ( lvp )
		  == unused_size - 4 );
	    min::unsptr aa = MUP::aux_offset_of ( lvp );
	    min::unsptr & caa =
	        MUP::aux_offset_of ( lvp );
	    lbase[aa-1] = num0;
	    MIN_ASSERT ( lbase[aa-1] == num0 );
	    MIN_ASSERT
		( min::aux_size_of ( lvp ) == 0 );
	    MIN_ASSERT ( caa == aa );
	    min::aux_push ( lvp, num1 );
	    MIN_ASSERT ( lbase[aa-1] == num1 );
	    MIN_ASSERT
		( min::aux_size_of ( lvp ) == 1 );
	    lbase[aa-2] = num0;
	    lbase[aa-3] = num0;
	    lbase[aa-4] = num0;
	    MIN_ASSERT ( lbase[aa-2] == num0 );
	    MIN_ASSERT ( lbase[aa-3] == num0 );
	    MIN_ASSERT ( lbase[aa-4] == num0 );
	    min::aux_push ( lvp, numv, 3 );
	    MIN_ASSERT ( lbase[aa-4] == num1 );
	    MIN_ASSERT ( lbase[aa-3] == num2 );
	    MIN_ASSERT ( lbase[aa-2] == num3 );
	    MIN_ASSERT ( aux ( lvp, aa-2 ) == num3 );
	    MIN_ASSERT
		( min::aux_size_of ( lvp ) == 4 );
	    MIN_ASSERT ( caa == aa - 4 );
	    MIN_ASSERT
		(    min::unused_size_of ( lvp )
		  == unused_size - 8 );

	    min::attr_pop ( lvp, outv + 1, 3 );
	    min::attr_pop ( lvp, outv[0] );
	    MIN_ASSERT ( outv[0] == num1 );
	    MIN_ASSERT ( outv[1] == num1 );
	    MIN_ASSERT ( outv[2] == num2 );
	    MIN_ASSERT ( outv[3] == num3 );
	    MIN_ASSERT
	        ( min::attr_size_of ( lvp ) == 0 );
	    MIN_ASSERT ( cua == av );
	    MIN_ASSERT
		(    min::unused_size_of ( lvp )
		  == unused_size - 4 );
	    desire_failure (
		min::attr_pop ( lvp, outv[0] );
	    );
	    desire_failure (
		min::attr_pop ( lvp, outv + 1, 3 );
	    );
	    min::attr_push ( lvp, fillv, 4 );

	    min::aux_pop ( lvp, outv + 1, 3 );
	    min::aux_pop ( lvp, outv[0] );
	    MIN_ASSERT ( outv[0] == num1 );
	    MIN_ASSERT ( outv[1] == num1 );
	    MIN_ASSERT ( outv[2] == num2 );
	    MIN_ASSERT ( outv[3] == num3 );
	    MIN_ASSERT
		(    min::aux_size_of ( lvp )
		  == 0 );
	    MIN_ASSERT ( caa == aa );
	    MIN_ASSERT
		(    min::unused_size_of ( lvp )
		  == unused_size - 4 );
	    desire_failure (
		min::aux_pop ( lvp, outv[0] );
	    );
	    desire_failure (
		min::aux_pop ( lvp, outv + 1, 3 );
	    );
	    min::aux_push ( lvp, fillv, 4 );

	    min::attr_push ( lvp, fillv, 35000 - 4 );
	    min::aux_push ( lvp, fillv,
	                    unused_size - 35000 - 4 );
	    MIN_ASSERT
	        ( min::unused_size_of ( lvp ) == 0 );
	    MIN_ASSERT ( cua == caa );
	    desire_failure (
		min::attr_push ( lvp, num3 );
	    );
	    desire_failure (
		min::attr_push ( lvp, numv, 3 );
	    );
	    desire_failure (
		min::aux_push ( lvp, num3 );
	    );
	    desire_failure (
		min::aux_push ( lvp, numv, 3 );
	    );
	}

	cout << endl;
	cout << "Finish Object Vector Level Test!"
	     << endl;
    }

// Object List Level

    // Values shared with previous object tests.
    // min::gen short_obj_gen;
    // min::gen long_obj_gen;
    {
	cout << endl;
	cout << "Start Object List Level Test!"
	     << endl;

	min::insertable_vec_pointer short_vp
		( short_obj_gen );
	min::gen * & sbase = MUP::base ( short_vp );
	min::unsptr vsize =
	    min::attr_size_of ( short_vp );
	min::unsptr vorg =
	    MUP::attr_offset_of ( short_vp );
	min::unsptr usize =
	    min::unused_size_of ( short_vp );
	cout << "VSIZE " << vsize << " VORG " << vorg
	     << " USIZE " << usize << endl;

	min::set_relocated_flag ( false );

	min::list_pointer lp ( short_vp );
	min::start_vector ( lp, 0 );
	MIN_ASSERT
	    ( min::current ( lp ) == sbase[vorg+0] );
	MIN_ASSERT
	    ( min::next ( lp ) == min::LIST_END );
	MIN_ASSERT
	    ( min::current ( lp ) == min::LIST_END );
	MIN_ASSERT
	    ( min::next ( lp ) == min::LIST_END );
	min::gen numtest =
	    min::new_num_gen ( 123456789 );
	sbase[vorg+0] = numtest;
	min::start_vector ( lp, 0 );
	MIN_ASSERT
	    ( min::current ( lp ) == sbase[vorg+0] );

	min::insertable_list_pointer wlp ( short_vp );
	min::start_vector ( wlp, 0 );
	min::insert_reserve ( wlp, 1, 3, true );
	MIN_ASSERT ( ! min::relocated_flag() );

	min::gen num100 = min::new_num_gen ( 100 );
	min::gen num101 = min::new_num_gen ( 101 );
	min::gen num102 = min::new_num_gen ( 102 );
	min::gen p[3] = { num100, num101, num102 };
	min::insert_after ( wlp, p, 3 );

	// Vector[0] list now is
	//	{ numtest, num100, num101, num102 }
	//
	MIN_ASSERT ( min::current ( wlp ) == numtest );
	MIN_ASSERT ( min::next ( wlp ) == num100 );
	MIN_ASSERT ( min::next ( wlp ) == num101 );
	min::set ( wlp, min::EMPTY_SUBLIST );
	MIN_ASSERT (    min::current ( wlp )
	             == min::EMPTY_SUBLIST );

	// Vector[0] list now is
	//	{ numtest, num100, {}, num102 }
	//
	min::insertable_list_pointer wslp ( short_vp );
	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	min::insert_reserve ( wslp, 1, 3, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_before ( wslp, p, 3 );
	min::refresh ( wlp );

	// Vector[0] list now is
	//	{ numtest, num100,
	//        { num100, num101, num102 }, num102 }
	//
	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num100 );
	MIN_ASSERT ( min::next ( wslp ) == num101 );
	MIN_ASSERT ( min::next ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	MIN_ASSERT ( min::next ( wlp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	min::start_vector ( wlp, 0 );
	MIN_ASSERT ( min::current ( wlp ) == numtest );
	MIN_ASSERT ( min::next ( wlp ) == num100 );
	MIN_ASSERT
	    ( min::is_sublist ( min::next ( wlp ) ) );

	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num100 );
	MIN_ASSERT ( min::next ( wslp ) == num101 );
	MIN_ASSERT ( 1 == min::remove ( wslp, 1 ) );
	min::refresh ( wlp );
	MIN_ASSERT ( min::current ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	// Vector[0] list now is
	//	{ numtest, num100,
	//        { num100, num102 }, num102 }
	//
	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num100 );
	MIN_ASSERT ( min::next ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( 1 == min::remove ( wslp, 1 ) );
	min::refresh ( wlp );
	MIN_ASSERT ( min::current ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	// Vector[0] list now is
	//	{ numtest, num100,
	//        { num102 }, num102 }
	//
	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num102 );
	MIN_ASSERT ( 1 == min::remove ( wslp, 5 ) );
	min::refresh ( wlp );

	// Vector[0] list now is
	//	{ numtest, num100, { }, num102 }
	//
	MIN_ASSERT (    min::current ( wlp )
	             == min::EMPTY_SUBLIST );
	MIN_ASSERT ( min::next ( wlp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	min::start_vector ( wlp, 0 );
	MIN_ASSERT ( min::current ( wlp ) == numtest );
	MIN_ASSERT ( 3 == min::remove ( wlp, 3 ) );
	MIN_ASSERT ( min::current ( wlp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	// Vector[0] list now is { num102 }
	//
	min::start_vector ( wlp, 0 );
	MIN_ASSERT ( min::current ( wlp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	min::start_vector ( wlp, 0 );
	MIN_ASSERT ( 1 == min::remove ( wlp, 3 ) );
	MIN_ASSERT
	    ( min::current ( wlp ) == min::LIST_END );

	// Vector[0] list now is { }
	//
	min::start_vector ( wlp, 0 );
	MIN_ASSERT
	    ( min::current ( wlp ) == min::LIST_END );

	// Repeat the above using aux area.

	min::gen tmp;
	for ( int i = 0; i < 20; ++ i )
	    min::attr_pop ( short_vp, tmp );

	cout << "USIZE BEFORE USING AUX "
	     << min::unused_size_of ( short_vp )
	     << endl;
	sbase[vorg+0] = numtest;
	min::start_vector ( lp, 0 );
	MIN_ASSERT
	    ( min::current ( lp ) == sbase[vorg+0] );

	min::start_vector ( wlp, 0 );
	min::insert_reserve ( wlp, 1, 3, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_after ( wlp, p, 3 );

	// Vector[0] list now is
	//	{ numtest, num100, num101, num102 }
	//
	MIN_ASSERT ( min::current ( wlp ) == numtest );
	MIN_ASSERT ( min::next ( wlp ) == num100 );
	MIN_ASSERT ( min::next ( wlp ) == num101 );
	min::set ( wlp, min::EMPTY_SUBLIST );
	MIN_ASSERT (    min::current ( wlp )
	             == min::EMPTY_SUBLIST );

	// Vector[0] list now is
	//	{ numtest, num100, {}, num102 }
	//
	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	min::insert_reserve ( wslp, 1, 3, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_before ( wslp, p, 3 );
	min::refresh ( wlp );

	// Vector[0] list now is
	//	{ numtest, num100,
	//        { num100, num101, num102 }, num102 }
	//
	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num100 );
	MIN_ASSERT ( min::next ( wslp ) == num101 );
	MIN_ASSERT ( min::next ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	MIN_ASSERT ( min::next ( wlp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	min::start_vector ( wlp, 0 );
	MIN_ASSERT ( min::current ( wlp ) == numtest );
	MIN_ASSERT ( min::next ( wlp ) == num100 );
	MIN_ASSERT
	    ( min::is_sublist ( min::next ( wlp ) ) );

	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num100 );
	MIN_ASSERT ( min::next ( wslp ) == num101 );
	MIN_ASSERT ( 1 == min::remove ( wslp, 1 ) );
	min::refresh ( wlp );
	MIN_ASSERT ( min::current ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	// Vector[0] list now is
	//	{ numtest, num100,
	//        { num100, num102 }, num102 }
	//
	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num100 );
	MIN_ASSERT ( min::next ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( 1 == min::remove ( wslp, 1 ) );
	min::refresh ( wlp );
	MIN_ASSERT ( min::current ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	// Vector[0] list now is
	//	{ numtest, num100,
	//        { num102 }, num102 }
	//
	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wslp ) == min::LIST_END );

	min::start_copy ( wslp, wlp );
	min::start_sublist ( wslp );
	MIN_ASSERT ( min::current ( wslp ) == num102 );
	MIN_ASSERT ( 1 == min::remove ( wslp, 5 ) );
	min::refresh ( wlp );

	// Vector[0] list now is
	//	{ numtest, num100, { }, num102 }
	//
	MIN_ASSERT (    min::current ( wlp )
	             == min::EMPTY_SUBLIST );
	MIN_ASSERT ( min::next ( wlp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	min::start_vector ( wlp, 0 );
	MIN_ASSERT ( min::current ( wlp ) == numtest );
	MIN_ASSERT ( 3 == min::remove ( wlp, 3 ) );
	MIN_ASSERT ( min::current ( wlp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	// Vector[0] list now is { num102 }
	//
	min::start_vector ( wlp, 0 );
	MIN_ASSERT ( min::current ( wlp ) == num102 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	min::start_vector ( wlp, 0 );
	MIN_ASSERT ( 1 == min::remove ( wlp, 3 ) );
	MIN_ASSERT
	    ( min::current ( wlp ) == min::LIST_END );

	// Vector[0] list now is { }
	//
	min::start_vector ( wlp, 0 );
	MIN_ASSERT
	    ( min::current ( wlp ) == min::LIST_END );

	cout << "USIZE AFTER USING AUX "
	     << min::unused_size_of ( short_vp )
	     << endl;

	// Now use a mixture of aux area and aux stubs.

	 while ( min::unused_size_of ( short_vp ) != 0 )
	    min::attr_push ( short_vp, numtest );

	sbase[vorg+0] = numtest;
	min::start_vector ( lp, 0 );
	MIN_ASSERT
	    ( min::current ( lp ) == sbase[vorg+0] );

	min::gen num103 = min::new_num_gen ( 103 );
	min::gen num104 = min::new_num_gen ( 104 );
	min::gen num105 = min::new_num_gen ( 105 );
	min::gen num106 = min::new_num_gen ( 106 );
	min::gen num107 = min::new_num_gen ( 107 );

	min::gen pv[8] =
	    { num100, num101, num102, num103,
	      num104, num105, num106, num107 };
	min::gen psplit[2] = { num104, num107 };

	// Build the following list:
	//
	//	{ numtest		In list head
	//	  num100		In aux stub
	//	  num101		In aux stub
	//	  num102		In aux stub
	//	  num103		In aux stub
	//	  num104		In aux area
	//	  num105		In aux area
	//	  num106		In aux stub
	//	  num107 }		In aux area


        MIN_ASSERT
	    ( min::unused_size_of ( short_vp ) == 0 );
	min::gen tmpv [20];
	min::start_vector ( wlp, 0 );
	min::insert_reserve ( wlp, 1, 2, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_after ( wlp, pv, 2 );
	MIN_ASSERT ( min::current ( wlp ) == numtest );
	MIN_ASSERT ( min::next ( wlp ) == num100 );
	MIN_ASSERT ( min::next ( wlp ) == num101 );
	min::attr_pop ( short_vp, tmpv, 3 );
        MIN_ASSERT
	    ( min::unused_size_of ( short_vp ) == 3 );
	min::insert_reserve ( wlp, 1, 2, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_after ( wlp, psplit, 2 );
        MIN_ASSERT
	    ( min::unused_size_of ( short_vp ) == 0 );
	MIN_ASSERT ( min::next ( wlp ) == num104 );
	min::insert_reserve ( wlp, 1, 2, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_before ( wlp, pv + 2, 2 );
	min::attr_pop ( short_vp, tmpv, 3 );
        MIN_ASSERT
	    ( min::unused_size_of ( short_vp ) == 3 );
	MIN_ASSERT ( min::next ( wlp ) == num107 );
	min::insert_reserve ( wlp, 2, 2, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_before ( wlp, pv + 5, 1 );
        MIN_ASSERT
	    ( min::unused_size_of ( short_vp ) == 0 );
	min::insert_before ( wlp, pv + 6, 1 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	min::start_vector ( wlp, 0 );
	MIN_ASSERT ( min::current ( wlp ) == numtest );
	MIN_ASSERT ( min::next ( wlp ) == num100 );
	MIN_ASSERT ( min::next ( wlp ) == num101 );
	MIN_ASSERT ( min::next ( wlp ) == num102 );
	MIN_ASSERT ( min::next ( wlp ) == num103 );
	MIN_ASSERT ( min::next ( wlp ) == num104 );
	MIN_ASSERT ( min::next ( wlp ) == num105 );
	MIN_ASSERT ( min::next ( wlp ) == num106 );
	MIN_ASSERT ( min::next ( wlp ) == num107 );
	MIN_ASSERT
	    ( min::next ( wlp ) == min::LIST_END );

	// Repeat the test using a mixture of aux area
	// and aux stubs but using long object instead
	// of short object.

	min::insertable_vec_pointer long_vp
		( long_obj_gen );
	min::gen * & lbase = MUP::base ( long_vp );
	vsize = min::attr_size_of ( long_vp );
	vorg = MUP::attr_offset_of ( long_vp );
	usize = min::unused_size_of ( long_vp );
	cout << "VSIZE " << vsize << " VORG " << vorg
	     << " USIZE " << usize << endl;

	min::list_pointer llp ( long_vp );
	min::insertable_list_pointer wllp ( long_vp );

	lbase[vorg+0] = numtest;
	min::start_vector ( llp, 0 );
	MIN_ASSERT
	    ( min::current ( llp ) == lbase[vorg+0] );

	// Build the following list:
	//
	//	{ numtest		In list head
	//	  num100		In aux stub
	//	  num101		In aux stub
	//	  num102		In aux stub
	//	  num103		In aux stub
	//	  num104		In aux area
	//	  num105		In aux area
	//	  num106		In aux stub
	//	  num107 }		In aux area


        MIN_ASSERT
	    ( min::unused_size_of ( long_vp ) == 0 );
	min::start_vector ( wllp, 0 );
	min::insert_reserve ( wllp, 1, 2, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_after ( wllp, pv, 2 );
	MIN_ASSERT ( min::current ( wllp ) == numtest );
	MIN_ASSERT ( min::next ( wllp ) == num100 );
	MIN_ASSERT ( min::next ( wllp ) == num101 );
	min::attr_pop ( long_vp, tmpv, 3 );
        MIN_ASSERT
	    ( min::unused_size_of ( long_vp ) == 3 );
	min::insert_reserve ( wllp, 1, 2, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_after ( wllp, psplit, 2 );
        MIN_ASSERT
	    ( min::unused_size_of ( long_vp ) == 0 );
	MIN_ASSERT ( min::next ( wllp ) == num104 );
	min::insert_reserve ( wllp, 1, 2, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_before ( wllp, pv + 2, 2 );
	min::attr_pop ( long_vp, tmpv, 3 );
        MIN_ASSERT
	    ( min::unused_size_of ( long_vp ) == 3 );
	MIN_ASSERT ( min::next ( wllp ) == num107 );
	min::insert_reserve ( wllp, 2, 2, true );
	MIN_ASSERT ( ! min::relocated_flag() );
	min::insert_before ( wllp, pv + 5, 1 );
        MIN_ASSERT
	    ( min::unused_size_of ( long_vp ) == 0 );
	min::insert_before ( wllp, pv + 6, 1 );
	MIN_ASSERT
	    ( min::next ( wllp ) == min::LIST_END );

	min::start_vector ( wllp, 0 );
	MIN_ASSERT ( min::current ( wllp ) == numtest );
	MIN_ASSERT ( min::next ( wllp ) == num100 );
	MIN_ASSERT ( min::next ( wllp ) == num101 );
	MIN_ASSERT ( min::next ( wllp ) == num102 );
	MIN_ASSERT ( min::next ( wllp ) == num103 );
	MIN_ASSERT ( min::next ( wllp ) == num104 );
	MIN_ASSERT ( min::next ( wllp ) == num105 );
	MIN_ASSERT ( min::next ( wllp ) == num106 );
	MIN_ASSERT ( min::next ( wllp ) == num107 );
	MIN_ASSERT
	    ( min::next ( wllp ) == min::LIST_END );

	cout << endl;
	cout << "Finish Object List Level Test!"
	     << endl;
    }

// Object Attribute Level

// Finish
// ------

    // Check that deallocated_body_region is still zero.
    //
    MIN_ASSERT
        ( deallocated_body_region[0] == 0
	  &&
	  memcmp ( deallocated_body_region,
		   deallocated_body_region + 1,
		   sizeof ( deallocated_body_region )
		   - 1 ) == 0 );

    } catch ( min_assert_exception * x ) {
        cout << "EXITING BECAUSE OF FAILED MIN_ASSERT"
	     << endl;
	exit ( 1 );
    }

    cout << endl;
    cout << "Finished Test!" << endl;
}
