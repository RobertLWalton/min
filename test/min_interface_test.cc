// MIN Language Interface Test Program
//
// File:	min_interface_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri May 22 04:16:09 EDT 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Notes
//	Setup
//	Run-Time System for Interface Tests
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
//	Names
//	Packed Structures
//	Packed Vectors
//	Files
//	Identifier Maps
//	UNICODE Name Tables
//	Printers
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level
//	Object Printing
//	Main Program

// Notes
// -----

// 1. This program does not use the garbage collector
//    and therefore does not follow the normal rules
//    for making stubs locatable.


// Setup
// -----

# include <iostream>
# include <iomanip>
# include <sstream>
# include <cstdlib>
# include <cstring>
using std::cout;
using std::endl;
using std::hex;
using std::dec;

bool debug = false;
#define dout if ( debug ) cout
bool memory_debug = false;
#define mout if ( memory_debug ) cout

#define PRINTING_MIN_CHECK(x) \
    min::assert_print = true; \
    MIN_CHECK ( x ); \
    min::assert_print = false;

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
    catch ( min::assert_exception * x ) \
        { }

# include <min.h>
# define MUP min::unprotected
# define MINT min::internal
# define MTEST min::test


# undef MIN_ASSERT
# define MIN_ASSERT MIN_ASSERT_CALL_ALWAYS


// Run-Time System for Interface Tests
// -------- ------ --- --------- -----

// Out of line functions that must be defined to test
// the MIN interface.  These definitions substitute for
// the normal run-time system, and are just for simple
// interface testing.

// Process Interface Functions.

// When this out-of-line interrupt function is called it
// counts the interrupt_count variable.
//
min::uns32 interrupt_count = 0;
bool MINT::acc_interrupt ( void )
{
    min::initialize();
    ++ interrupt_count;
    min::thread_interrupt();
    return true;
}

static void test_initialize ( void )
{
    cout << "TEST INITIALIZE CALLED" << endl;
}
static min::initializer
    test_initializer ( ::test_initialize );

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
    MIN_REQUIRE ( (   ( sizeof (min::stub) - 1 )
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
	MIN_CHECK
	    (    begin_stub_region
	      >= (min::stub *)
	         min::internal::stub_base );
#   endif

    stp += sizeof stub_region;
    min::unsptr n =
        ( stp - p ) / ( sizeof (min::stub) );
    p += ( sizeof (min::stub) ) * n;
    end_stub_region = (min::stub *) p;
    MIN_REQUIRE ( begin_stub_region < end_stub_region );

    MINT::head_stub = begin_stub_region;
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

std::ostream & operator <<
	( std::ostream & out, const min::stub * s )
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
    mout << "MINT::acc_expand_stub_free_list (" << n
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
	MIN_REQUIRE ( s < end_stub_region );
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
char body_region[4000000];

// Place to point deallocated bodies.  All zeros.
//
char deallocated_body_region[4000000];

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
    next_body = begin_body_region;
}

void MINT::new_non_fixed_body
	( min::stub * s, min::unsptr n )
{
    mout << "MINT::new_non_fixed_body ( " << s
         << ", " << n << " ) called" << endl;

    min::unsptr m = n + 7;
    m >>= 3;
    ++ m;
    MIN_CHECK ( next_body + m <= end_body_region );

    * next_body =
        MUP::new_control_with_locator ( 0, s );
    	
    MUP::set_ptr_of ( s, next_body + 1 );

    next_body += m;
}

// Performs MUP::new_body when count of fixed bodies
// is zero.
//
void MINT::new_fixed_body
    ( min::stub * s, min::unsptr n,
      MINT::fixed_block_list * fbl )
{
    mout << "MINT::new_fixed_body ( " << s
         << ", " << n << " ) called" << endl;

    min::unsptr m = fbl->size >> 3;
    min::uns64 * next = next_body;
    MIN_CHECK ( next + 2 * m <= end_body_region );

    dout << "Using fixed_block_lists["
         << fbl - fixed_block_lists << "]"
	 << " and assigning begin_body_region["
	 << next - begin_body_region
	 << " .. "
	 << next - begin_body_region + m - 1
	 << "]" << endl;

    * next =
        MUP::new_control_with_locator ( 0, s );
    MUP::set_ptr_of ( s, next + 1 );
    MUP::set_flags_of ( s, MINT::ACC_FIXED_BODY_FLAG );

    next += m;

    MINT::free_fixed_size_block * fb =
        (MINT::free_fixed_size_block *) next;
    fb->block_control = 0;
    fb->next = fb;
    fbl->last_free = fb;
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

    mout << "MINT::deallocate ( " << s
         << ", " << n << " ) called" << endl;

    MUP::set_ptr_of ( s, deallocated_body_region );
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
    mout << "resize_body ( stub "
         << s - begin_stub_region << ", " << new_size
         << ", " << old_size << " ) called" << endl;
    {
	MUP::resize_body rbody
	    ( s, new_size, old_size );
	min::unsptr length = new_size >= old_size ?
	                  old_size : new_size;

	min::uns64 * from = (min::uns64 *)
	    MUP::ptr_of ( s );
	min::uns64 * to   = (min::uns64 *)
	    MUP::new_body_ptr_ref ( rbody );
	dout << "copying body_region["
	     << from - begin_body_region
	     << " .. "
	     << from - begin_body_region + length/8 - 1
	     << "] to body_region["
	     << to - begin_body_region
	     << " .. "
	     << to - begin_body_region + length/8 - 1
	     << "]" << endl;
	memcpy ( to, from, length );
	dout << "zeroing body_region["
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
    MINT::str_acc_hash =
        new stubp[MINT::str_hash_size];
    MINT::str_aux_hash =
        new stubp[MINT::str_hash_size];
    for ( unsigned i = 0; i < MINT::str_hash_size;
                          ++ i )
	MINT::str_acc_hash[i] = MINT::str_aux_hash[i] =
	    MINT::null_stub;
#   if MIN_IS_COMPACT
	MINT::num_hash_size = 128;
	MINT::num_hash_mask = 128 - 1;
	MINT::num_acc_hash =
	    new stubp[MINT::num_hash_size];
	MINT::num_aux_hash =
	    new stubp[MINT::num_hash_size];
	for ( unsigned i = 0; i < MINT::num_hash_size;
	                      ++ i )
	    MINT::num_acc_hash[i] =
	        MINT::num_aux_hash[i] =
		MINT::null_stub;
#   endif
    MINT::lab_hash_size = 128;
    MINT::lab_hash_mask = 128 - 1;
    MINT::lab_acc_hash =
        new stubp[MINT::lab_hash_size];
    MINT::lab_aux_hash =
        new stubp[MINT::lab_hash_size];
    for ( unsigned i = 0; i < MINT::lab_hash_size;
                          ++ i )
	MINT::lab_acc_hash[i] = MINT::lab_aux_hash[i] =
	    MINT::null_stub;
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

static void * packed_subtypes[MIN_PACKED_SUBTYPE_COUNT];
static void ** packed_subtypes_p = packed_subtypes;

void MINT::allocate_packed_subtypes ( min::uns32 count )
{
    MIN_REQUIRE ( MINT::packed_subtypes == NULL );
    MINT::packed_subtypes = & ::packed_subtypes_p;
    MIN_REQUIRE ( count <= MIN_PACKED_SUBTYPE_COUNT );
    MINT::max_packed_subtype_count = count;
}

void MINT::acc_initializer ( void )
{
    initialize_stub_region();
    initialize_body_region();
    initialize_hash_tables();
    initialize_acc_stack();
}

// Find a values address in a MINT::locatable_var list.
//
bool find_ptr_locator ( void * address )
{
    min::locatable_stub_ptr * locator =
	MINT::locatable_stub_ptr_last;

    while ( locator )
    {
        if ( locator == address )
	    return true;
	locator =
	    MINT::locatable_var_previous ( locator );
    }
    return false;
}

// Find a values address in a MINT::locatable_gen list.
//
bool find_gen_locator ( void * address )
{
    min::locatable_gen * locator =
        MINT::locatable_gen_last;

    while ( locator )
    {
        if ( locator == address )
	    return true;
	locator =
	    MINT::locatable_var_previous ( locator );
    }
    return false;
}

// Count the number of locators on a MINT::locatable_gen
// list.
//
int count_gen_locators ( void )
{
    min::locatable_gen * locator =
        MINT::locatable_gen_last;

    int count = 0;
    while ( locator )
    {
        ++ count;
	locator =
	    MINT::locatable_var_previous ( locator );
    }
    return count;
}

// C++ Number Types
// --- ------ -----

void test_number_types ( void )
{
    cout << endl;
    cout << "Start Number Types Test!" << endl;
    cout << "Check that uns64 is 64 bits long:"
	 << endl;
    min::uns64 u64 = (min::uns64) 1;
    u64 <<= 63;
    u64 >>= 13;
    min::float64 f64 = u64;
    MIN_CHECK ( f64 != 0 );
    u64 <<= 14;
    u64 >>= 14;
    f64 = u64;
    MIN_CHECK ( f64 == 0 );

    cout << endl;
    cout << "Finish Number Types Test!" << endl;
}

// General Value Types and Data
// ------- ----- ----- --- ----

void test_general_value_data ( void )
{
    // There are no general types and data tests.
}

// Stub Types and Data
// ---- ----- --- ----

void test_stub_data ( void )
{
    // There are no stub types and data tests.
}

// Internal Pointer Conversion Functions
// -------- ------- ---------- ---------

void test_ptr_conversions ( void )
{
    cout << endl;
    cout << "Start Internal Pointer Conversion"
	    " Test!" << endl;
    char buffer[1];
    const min::stub * stub = MINT::null_stub;

    cout << endl;
    cout << "Test ptr/uns64 conversions:"
	 << endl;
    min::uns64 u64 =
	MINT::ptr_to_uns64 ( buffer );
    char * b64 = (char *)
	MINT::uns64_to_ptr ( u64 );
    MIN_CHECK ( b64 == buffer );

    cout << endl;
    cout << "Test stub/unsgen conversions:"
	 << endl;
    min::unsgen ugen = MINT::stub_to_unsgen ( stub );
    min::stub * sgen = MINT::unsgen_to_stub ( ugen );
    MIN_CHECK ( sgen == stub );

    cout << endl;
    cout << "Finish Internal Pointer Conversion"
	    " Test!" << endl;
}

// General Value Constructor/Test/Read Functions
// ------- ----- --------------------- ---------

// Count the number of gen_tests that are true.
//
int count_gen_tests ( min::gen v )
{
    return   min::is_stub ( v )
#          if MIN_IS_COMPACT
               + min::is_direct_int ( v )
#          else
               + min::is_direct_float ( v )
#          endif
           + min::is_direct_str ( v )
           + min::is_index ( v )
           + min::is_control_code ( v )
           + min::is_special ( v )
           + min::is_list_aux ( v )
           + min::is_sublist_aux ( v )
           + min::is_indirect_aux ( v )
           + min::is_aux ( v );
}

void test_general_value_functions ( void )
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
    min::gen stubgen = MUP::new_stub_gen ( stub );
    cout << "stubgen: " << stubgen << endl;
    MIN_CHECK ( min::is_stub ( stubgen ) );
    MIN_CHECK ( count_gen_tests ( stubgen ) == 1 );
    MIN_CHECK ( MUP::stub_of ( stubgen ) == stub );
    stubgen = min::new_stub_gen ( stub );
    MIN_CHECK ( min::is_stub ( stubgen ) );
    MIN_CHECK ( count_gen_tests ( stubgen ) == 1 );
    MIN_CHECK (    min::gen_subtype_of ( stubgen )
		 == min::GEN_STUB );
    MIN_CHECK ( MUP::stub_of ( stubgen ) == stub );

#   if MIN_IS_COMPACT
	cout << endl;
	cout << "Test direct integer general"
		" values:" << endl;
	int i = -8434;
	min::gen igen =
	    MUP::new_direct_int_gen ( i );
	cout << "igen: " << igen << endl;
	MIN_CHECK ( min::is_direct_int ( igen ) );
	MIN_CHECK
	    ( count_gen_tests ( igen ) == 1 );
	MIN_CHECK
	    ( MUP::direct_int_of ( igen ) == i );
	igen = min::new_direct_int_gen ( i );
	MIN_CHECK ( min::is_direct_int ( igen ) );
	MIN_CHECK
	    ( count_gen_tests ( igen ) == 1 );
	MIN_CHECK
	    (    min::gen_subtype_of ( igen )
	      == min::GEN_DIRECT_INT );
	MIN_CHECK
	    ( MUP::direct_int_of ( igen ) == i );
	desire_failure (
	    igen = min::new_direct_int_gen
			( 1 << 28 );
	);
	desire_success (
	    igen = min::new_direct_int_gen
			( 1 << 27 );
	);
	desire_failure (
	    igen = min::new_direct_int_gen
			( -1 << 29 );
	);
	desire_success (
	    igen = min::new_direct_int_gen
			( -1 << 28 );
	);
	MIN_CHECK
	    (    MUP::direct_int_of ( igen )
	      == -1 << 28 );
#   endif

#   if MIN_IS_LOOSE
	cout << endl;
	cout << "Test direct float general"
		" values:" << endl;
	min::float64 f = -8.245324897;
	min::gen fgen =
	    MUP::new_direct_float_gen ( f );
	cout << "fgen: " << fgen << endl;
	MIN_CHECK
	    ( min::is_direct_float ( fgen ) );
	MIN_CHECK
	    ( count_gen_tests ( fgen ) == 1 );
	MIN_CHECK
	    ( MUP::direct_float_of ( fgen ) == f );
	fgen = min::new_direct_float_gen ( f );
	MIN_CHECK
	    ( min::is_direct_float ( fgen ) );
	MIN_CHECK
	    ( count_gen_tests ( fgen ) == 1 );
	MIN_CHECK
	    (    min::gen_subtype_of ( fgen )
	      == min::GEN_DIRECT_FLOAT );
	MIN_CHECK
	    ( MUP::direct_float_of ( fgen ) == f );
#   endif

    cout << endl;
    cout << "Test direct string general values:"
	 << endl;
#   if MIN_IS_COMPACT
	const char * str = "ABC";
	const char * overflowstr = "ABCD";
	const char * overflowstrn = "ABCDE";
	int strlimit = 3;
#   elif MIN_IS_LOOSE
	const char * str = "ABCDE";
	const char * overflowstr = "ABCDEF";
	const char * overflowstrn = "ABCDEFG";
	int strlimit = 6;
#   endif
    union {
	min::uns64 u64;
	char str[8];
    } value;

    min::gen strgen =
	MUP::new_direct_str_gen ( str );
    cout << "strgen: " << strgen << endl;
    MIN_CHECK ( min::is_direct_str ( strgen ) );
    MIN_CHECK ( count_gen_tests ( strgen ) == 1 );
    MIN_CHECK (    min::gen_subtype_of ( strgen )
		 == min::GEN_DIRECT_STR );
    value.u64 = MUP::direct_str_of ( strgen );
    MIN_CHECK ( strcmp ( str, value.str ) == 0 );
    desire_success (
	strgen = min::new_direct_str_gen ( str );
    );
    desire_failure (
	strgen = min::new_direct_str_gen
			    ( overflowstr );
    );

    min::gen strngen =
	MUP::new_direct_str_gen ( str, 2 );
    cout << "strngen: " << strngen << endl;
    MIN_CHECK ( min::is_direct_str ( strngen ) );
    MIN_CHECK ( count_gen_tests ( strngen ) == 1 );
    MIN_CHECK (    min::gen_subtype_of ( strngen )
		 == min::GEN_DIRECT_STR );
    value.u64 = MUP::direct_str_of ( strngen );
    MIN_CHECK
	( strncmp ( str, value.str, 2 ) == 0 );
    MIN_CHECK ( value.str[2] == 0 );
    desire_success (
	strngen =
	    min::new_direct_str_gen ( str, 2 );
    );
    desire_failure (
	strngen = min::new_direct_str_gen
			    ( overflowstrn,
			      strlimit + 1 );
    );

    cout << endl;
    cout << "Test list aux general values:"
	 << endl;
    unsigned aux = 734523;
    min::gen listauxgen =
	MUP::new_list_aux_gen ( aux );
    cout << "listauxgen: " << listauxgen << endl;
    MIN_CHECK ( min::is_list_aux ( listauxgen ) );
    MIN_CHECK
	( count_gen_tests ( listauxgen ) == 2 );
    MIN_CHECK (    min::gen_subtype_of
			    ( listauxgen )
		 == min::GEN_LIST_AUX );
    MIN_CHECK
	( min::list_aux_of ( listauxgen ) == aux );
    desire_success (
	listauxgen = min::new_list_aux_gen ( aux );
    );
    desire_failure (
	listauxgen = min::new_list_aux_gen
	    ( (min::unsgen) 1 << min::VSIZE );
    );
    unsigned reaux = 963921;
    listauxgen =
	MUP::renew_gen ( listauxgen, reaux );
    cout << "re-listauxgen: " << listauxgen << endl;
    MIN_CHECK ( min::is_list_aux ( listauxgen ) );
    MIN_CHECK
	( count_gen_tests ( listauxgen ) == 2 );
    MIN_CHECK
	(    min::list_aux_of ( listauxgen )
	  == reaux );

    cout << endl;
    cout << "Test sublist aux general values:"
	 << endl;
    min::gen sublistauxgen =
	MUP::new_sublist_aux_gen ( aux );
    cout << "sublistauxgen: " << sublistauxgen << endl;
    MIN_CHECK
	( min::is_sublist_aux ( sublistauxgen ) );
    MIN_CHECK
	( count_gen_tests ( sublistauxgen ) == 2 );
    MIN_CHECK
	(    min::gen_subtype_of ( sublistauxgen )
	  == min::GEN_SUBLIST_AUX );
    MIN_CHECK
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

    cout << endl;
    cout << "Test indirect aux general values:"
	 << endl;
    min::gen indirectauxgen =
	MUP::new_indirect_aux_gen ( aux );
    cout << "indirectauxgen: "
	 << indirectauxgen << endl;
    MIN_CHECK
	( min::is_indirect_aux
		    ( indirectauxgen ) );
    MIN_CHECK
	( count_gen_tests ( indirectauxgen ) == 2 );
    MIN_CHECK
	(    min::gen_subtype_of ( indirectauxgen )
	  == min::GEN_INDIRECT_AUX );
    MIN_CHECK
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

    cout << endl;
    cout << "Test index general values:" << endl;
    unsigned index = 734523;
    min::gen indexgen =
	MUP::new_index_gen ( index );
    cout << "indexgen: " << indexgen << endl;
    MIN_CHECK ( min::is_index ( indexgen ) );
    MIN_CHECK
	( count_gen_tests ( indexgen ) == 1 );
    MIN_CHECK
	(    min::gen_subtype_of ( indexgen )
	  == min::GEN_INDEX );
    MIN_CHECK
	( min::index_of ( indexgen ) == index );
    desire_success (
	indexgen = min::new_index_gen ( index );
    );
    desire_failure (
	indexgen = min::new_index_gen
	    ( (min::unsgen) 1 << min::VSIZE );
    );

    cout << endl;
    cout << "Test control code general values:"
	 << endl;
    unsigned code = 0x7e005f;
    min::gen codegen =
	MUP::new_control_code_gen ( code );
    cout << "codegen: " << codegen << endl;
    MIN_CHECK ( min::is_control_code ( codegen ) );
    MIN_CHECK ( count_gen_tests ( codegen ) == 1 );
    MIN_CHECK
	(    min::gen_subtype_of ( codegen )
	  == min::GEN_CONTROL_CODE );
    MIN_CHECK
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

    cout << endl;
    cout << "Test special general values:"
	 << endl;

    MIN_CHECK
	( min::is_special ( min::MISSING() ) );
    MIN_CHECK
	( count_gen_tests ( min::MISSING() ) == 1 );
    MIN_CHECK
	( min::is_special ( min::ANY() ) );
    MIN_CHECK
	( count_gen_tests ( min::ANY() ) == 1 );
    MIN_CHECK
	( min::is_special ( min::MULTI_VALUED() ) );
    MIN_CHECK
	( count_gen_tests
	      ( min::MULTI_VALUED() ) == 1 );
    MIN_CHECK
	( min::is_special ( min::UNDEFINED() ) );
    MIN_CHECK
	( count_gen_tests ( min::UNDEFINED() ) == 1 );
    MIN_CHECK
	( min::is_special ( min::SUCCESS() ) );
    MIN_CHECK
	( count_gen_tests ( min::SUCCESS() ) == 1 );
    MIN_CHECK
	( min::is_special ( min::FAILURE() ) );
    MIN_CHECK
	( count_gen_tests ( min::FAILURE() ) == 1 );

    unsigned special = 0x7e005f;
    min::gen specialgen =
	MUP::new_special_gen ( special );
    cout << "specialgen: " << specialgen << endl;
    MIN_CHECK ( min::is_special ( specialgen ) );
    MIN_CHECK
	( count_gen_tests ( specialgen ) == 1 );
    MIN_CHECK
	(    min::gen_subtype_of ( specialgen )
	  == min::GEN_SPECIAL );
    MIN_CHECK
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

    cout << endl;
    cout << "Finish General Value Constructor/"
	    "/Test/Read Function Test!" << endl;
}

// Control Values
// ------- ------

void test_control_values ( void )
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
    MIN_CHECK
	(    MUP::type_of_control ( control1 )
	  == type1 );
    MIN_CHECK
	(    MUP::value_of_control ( control1 )
	  == v1 );
    MIN_CHECK ( control1 & hiflag );
    MIN_CHECK ( control1 & loflag );
    MIN_CHECK ( ! ( control1 & midflag ) );

    control1 =
	MUP::renew_control_type ( control1, type2 );
    cout << "re-control1: " << hex << control1
	 << dec << endl;
    MIN_CHECK
	(    MUP::type_of_control ( control1 )
	  == type2 );
    MIN_CHECK
	(    MUP::value_of_control ( control1 )
	  == v1 );
    MIN_CHECK ( control1 & hiflag );
    MIN_CHECK ( control1 & loflag );
    MIN_CHECK ( ! ( control1 & midflag ) );

    control1 =
	MUP::renew_control_value ( control1, v2 );
    cout << "re-control1: " << hex << control1
	 << dec << endl;
    MIN_CHECK
	(    MUP::type_of_control ( control1 )
	  == type2 );
    MIN_CHECK
	(    MUP::value_of_control ( control1 )
	  == v2 );
    MIN_CHECK ( control1 & hiflag );
    MIN_CHECK ( control1 & loflag );
    MIN_CHECK ( ! ( control1 & midflag ) );

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
    MIN_CHECK
	(    MUP::type_of_control ( control2 )
	  == type1 );
    MIN_CHECK
	(    MUP::stub_of_control ( control2 )
	  == stub1 );
    MIN_CHECK ( control2 & hiflag );
    MIN_CHECK ( ! ( control2 & loflag ) );
    MIN_CHECK ( ! ( control2 & midflag ) );

    control2 =
	MUP::renew_control_stub
	    ( control2, stub2 );
    cout << "re-control2: " << hex
	 << control2 - stubbase << dec << endl;
    MIN_CHECK
	(    MUP::type_of_control ( control2 )
	  == type1 );
    MIN_CHECK
	(    MUP::stub_of_control ( control2 )
	  == stub2 );
    MIN_CHECK ( control2 & hiflag );
    MIN_CHECK ( ! ( control2 & loflag ) );
    MIN_CHECK ( ! ( control2 & midflag ) );

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
    MIN_CHECK
	(    MUP::type_of_control ( control3 )
	  == type1 );
    MIN_CHECK
	(    MUP::stub_of_acc_control ( control3 )
	  == stub1 );
    MIN_CHECK ( control3 & hiflag );
    MIN_CHECK ( ! ( control3 & loflag ) );
    MIN_CHECK ( ! ( control3 & midflag ) );

    control3 =
	MUP::renew_acc_control_stub
	    ( control3, stub2 );
    cout << "re-control3: " << hex
	 << control3 - stubbase << dec << endl;
    MIN_CHECK
	(    MUP::type_of_control ( control3 )
	  == type1 );
    MIN_CHECK
	(    MUP::stub_of_acc_control ( control3 )
	  == stub2 );
    MIN_CHECK ( control3 & hiflag );
    MIN_CHECK ( ! ( control3 & loflag ) );
    MIN_CHECK ( ! ( control3 & midflag ) );

    cout << endl;
    cout << "Finish Control Value Test!" << endl;
}

// Stub Functions
// ---- ---------

void test_stub_functions ( void )
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
    MIN_CHECK ( MUP::value_of ( stub ) == u );

    min::float64 f = 1.4362346234;
    MUP::set_float_of ( stub, f );
    MIN_CHECK ( MUP::float_of ( stub ) == f );

    min::gen g = min::new_stub_gen ( stub );
    MUP::set_gen_of ( stub, g );
    MIN_CHECK ( MUP::gen_of ( stub ) == g );

    void * p = & g;
    MUP::set_ptr_of ( stub, p );
    MIN_CHECK ( MUP::ptr_of ( stub ) == p );

    cout << endl;
    cout << "Test stub control set/read functions:"
	 << endl;
    min::uns64 f1 = min::uns64(1) << 55;
    min::uns64 f2 = min::uns64(1) << 48;
    min::uns64 c = f1 | f2;
    cout << "c: " << hex << c << dec << endl;
    MUP::set_control_of ( stub, c );
    MIN_CHECK ( MUP::control_of ( stub ) == c );
    MIN_CHECK ( min::type_of ( stub ) == 0 );
    MUP::set_type_of ( stub, min::NUMBER );
    c = MUP::renew_control_type ( c, min::NUMBER );
    cout << "c: " << hex << c << dec << endl;
    MIN_CHECK ( MUP::control_of ( stub ) == c );
    MIN_CHECK
	( min::type_of ( stub ) == min::NUMBER );

    cout << endl;
    cout << "Test stub flag set/clear/read"
	    " functions:" << endl;
    MIN_CHECK ( MUP::test_flags_of ( stub, f1 ) );
    MIN_CHECK ( MUP::test_flags_of ( stub, f2 ) );
    MUP::clear_flags_of ( stub, f2 );
    MIN_CHECK ( MUP::test_flags_of ( stub, f1 ) );
    MIN_CHECK
	( ! MUP::test_flags_of ( stub, f2 ) );
    MUP::set_flags_of ( stub, f2 );
    MIN_CHECK ( MUP::test_flags_of ( stub, f1 ) );
    MIN_CHECK ( MUP::test_flags_of ( stub, f2 ) );
    MIN_CHECK ( MUP::control_of ( stub ) == c );

    cout << endl;
    cout << "Test stub ACC related functions:"
	 << endl;
    MIN_CHECK
	( min::is_collectible ( min::NUMBER ) );
    MIN_CHECK
	( ! min::is_collectible ( min::LIST_AUX ) );
    MUP::set_type_of ( stub, min::NUMBER );
    MIN_CHECK ( ! min::is_deallocated ( stub ) );
    MUP::set_type_of ( stub, min::DEALLOCATED );
    MIN_CHECK ( min::is_deallocated ( stub ) );

    cout << endl;
    cout << "Finish Stub Functions Test!" << endl;
}

// Process Interface
// ------- ---------

void test_process_interface ( void )
{
    cout << endl;
    cout << "Start Process Interface Test!" << endl;

    // Process control testing is TBD.

    cout << endl;
    cout << "Test interrupt function:" << endl;
    min::uns32 count = ::interrupt_count;
    min::interrupt();
    MIN_CHECK ( ::interrupt_count == count );
    min::stub ** limit_save = MINT::acc_stack_limit;
    MINT::acc_stack_limit = MINT::acc_stack;
    min::interrupt();
    MIN_CHECK ( ::interrupt_count == count + 1 );
    MINT::acc_stack_limit = limit_save;

    cout << endl;
    cout << "Finish Process Interface Test!"
	 << endl;
}

// Allocator/Collector/Compactor Interface
// ----------------------------- ---------

void test_acc_interface ( void )
{
    cout << endl;
    cout << "Start Allocator/Collector/Compactor"
	    " Interface Test!" << endl;

    cout << endl;
    cout << "optimal_body_size ( 53 ) = "
         << min::unprotected::optimal_body_size ( 53 )
	 << endl;

    bool memory_debug_save = memory_debug;
    memory_debug = true;

    static min::stub s1, s2;
    const min::uns64 unmarked_flag =
	   min::uns64(1)
	<< ( 56 - MIN_ACC_FLAG_BITS + 2
	        + MIN_MAX_EPHEMERAL_LEVELS );
    const min::uns64 scavenged_flag =
	unmarked_flag << MINT::ACC_FLAG_PAIRS;

    cout << endl;
    cout << "Test mutator functions:"
	 << endl;
    MUP::set_control_of ( &s1, 0 );
    MUP::set_flags_of ( &s1, scavenged_flag );
    MUP::set_control_of ( &s2, 0 );
    MUP::set_flags_of ( &s2, unmarked_flag );
    MINT::acc_stack_mask = 0;
    MUP::acc_write_update ( &s1, &s2 );
    MIN_CHECK ( MINT::acc_stack == ::acc_stack );
    MINT::acc_stack_mask = unmarked_flag;
    MUP::acc_write_update ( &s1, &s2 );
    MIN_CHECK
	( MINT::acc_stack == ::acc_stack + 2 );
    MIN_CHECK ( ::acc_stack[0] == &s1 );
    MIN_CHECK ( ::acc_stack[1] == &s2 );
    MUP::clear_flags_of ( &s1, scavenged_flag );
    MUP::acc_write_update ( &s1, &s2 );
    MIN_CHECK
	( MINT::acc_stack == ::acc_stack + 2 );
    MUP::set_flags_of ( &s1, scavenged_flag );
    MUP::clear_flags_of ( &s2, unmarked_flag );
    MUP::acc_write_update ( &s1, &s2 );
    MIN_CHECK
	( MINT::acc_stack == ::acc_stack + 2 );

    cout << endl;
    cout << "Test stub allocator functions:"
	 << endl;
    MINT::new_acc_stub_flags = 0;
    min::unsptr sbase = stubs_allocated;
    cout << "initial stubs allocated = "
	 << sbase << endl;
    min::stub * stub1 = MUP::new_acc_stub();
    MIN_CHECK
	( stub1 == begin_stub_region + sbase  );
    MIN_CHECK
	( stub1 == MINT::last_allocated_stub );
    MIN_CHECK ( stubs_allocated == sbase + 1 );
    MIN_CHECK
	( min::type_of ( stub1 ) == min::ACC_FREE );
    MIN_CHECK
	( ! MUP::test_flags_of
		 ( stub1, unmarked_flag ) );
    MINT::new_acc_stub_flags = unmarked_flag;
    min::stub * stub2 = MUP::new_acc_stub();
    MIN_CHECK
	( stub2 == MINT::last_allocated_stub );
    MIN_CHECK ( stubs_allocated == sbase + 2 );
    MIN_CHECK
	( stub2 == begin_stub_region + sbase + 1 );
    MIN_CHECK
	( min::type_of ( stub2 ) == min::ACC_FREE );
    MIN_CHECK
	( MUP::test_flags_of
		 ( stub2, unmarked_flag ) );
    MINT::acc_expand_stub_free_list ( 2 );
    MIN_CHECK ( stubs_allocated == sbase + 4 );
    MIN_CHECK
	( stub2 == MINT::last_allocated_stub );
    min::stub * stub3 = MUP::new_aux_stub();
    MIN_CHECK
	( stub3 == begin_stub_region + sbase + 3 );
    MIN_CHECK ( stubs_allocated == sbase + 4 );
    MIN_CHECK
	( stub2 == MINT::last_allocated_stub );
    min::stub * stub4 = MUP::new_acc_stub();
    MIN_CHECK
	( stub4 == begin_stub_region + sbase + 2 );
    MIN_CHECK ( stubs_allocated == sbase + 4 );
    MIN_CHECK
	( stub4 == MINT::last_allocated_stub );


    cout << endl;
    cout << "Test body allocator functions:"
	 << endl;
    cout << "MINT::min_fixed_block_size = "
         << MINT::min_fixed_block_size
         << " MINT::max_fixed_block_size = "
         << MINT::max_fixed_block_size
	 << endl;
    MUP::new_body ( stub1, 128 );
    char * p1 = (char *) MUP::ptr_of ( stub1 );
    memset ( p1, 0xBB, 128 );
    MUP::new_body ( stub2, 128 );
    char * p2 = (char *) MUP::ptr_of ( stub2 );
    memset ( p2, 0xBB, 128 );
    MIN_CHECK ( memcmp ( p1, p2, 128 ) == 0 );
    MIN_CHECK ( p1 != p2 );
    MUP::new_body ( stub3, 128 );
    char * p3 = (char *) MUP::ptr_of ( stub3 );
    memset ( p3, 0xCC, 128 );
    MUP::new_body ( stub4, 128 );
    char * p4 = (char *) MUP::ptr_of ( stub4 );
    memset ( p4, 0xCC, 128 );
    MIN_CHECK ( memcmp ( p3, p4, 128 ) == 0 );
    MIN_CHECK ( p3 != p4 );
    resize_body ( stub4, 128, 128 );
    char * p5 = (char *) MUP::ptr_of ( stub4 );
    MIN_CHECK ( memcmp ( p3, p5, 128 ) == 0 );
    MIN_CHECK ( p4 != p5 );
    MUP::deallocate_body ( stub4, 128 );
    MIN_CHECK ( min::type_of ( stub4 )
		 == min::DEALLOCATED );
    char * p6 = (char *) MUP::ptr_of ( stub4 );
    MIN_CHECK ( p5 != p6 );
    MIN_CHECK ( p6[0] == 0
		 &&
		 memcmp ( p6, p6+1, 127 ) == 0 );

    cout << endl;
    cout << "Test General Value Locators:"
	 << endl;

    static min::locatable_gen staticg1[3];
    static min::locatable_num_gen staticg2[2];

    MIN_CHECK
	( find_ptr_locator
	      ( & (const min::stub * &)
	          min::error_message ) );
    MIN_CHECK
	( find_gen_locator
	      ( & (min::gen &) staticg1[0] ) );
    MIN_CHECK
	( find_gen_locator
	      ( & (min::gen &) staticg2[0] )
	  == MIN_IS_COMPACT );

    int locatable_gen_count = count_gen_locators();
    {
        min::locatable_gen g3[5];
        min::locatable_num_gen g4[3];
	g3[0] = g4[0];
	MIN_CHECK
	    ( find_gen_locator
	          ( & (min::gen &) staticg1[2] ) );
	MIN_CHECK
	    ( find_gen_locator
	          ( & (min::gen &) g3[4] ) );
	MIN_CHECK
	    (    count_gen_locators()
	      == 5 + 3 * MIN_IS_COMPACT
	       + locatable_gen_count );
    }
    MIN_CHECK
	(    count_gen_locators()
	  == locatable_gen_count );

    memory_debug = memory_debug_save;

    cout << endl;
    cout << "Finish Allocator/Collector/Compactor"
	    " Interface Test!" << endl;
}

// Numbers
// -------

void test_numbers ( void )
{
    cout << endl;
    cout << "Start Numbers Test!" << endl;

    cout << endl;
    cout << "Test number create/test/read"
	    " functions:" << endl;

    min::gen n1 = min::new_num_gen ( 12345 );
    cout << "n1: " << n1 << endl;
    MIN_CHECK ( min::is_num ( n1 ) );
    MIN_CHECK ( min::is_name ( n1 ) );
    MIN_CHECK ( min::int_of ( n1 ) == 12345 );
    MIN_CHECK ( min::float_of ( n1 ) == 12345 );
    MIN_CHECK ( MUP::float_of ( n1 ) == 12345 );
    min::uns32 n1hash = min::numhash ( n1 );
    cout << "n1hash: " << hex << n1hash << dec
	 << endl;
    MIN_CHECK
	( n1hash == min::floathash ( 12345 ) );
    MIN_CHECK
	( n1hash == min::hash ( n1 ) );
    MIN_CHECK ( min::new_num_gen ( 12345 ) == n1 );

    min::gen n2 = min::new_num_gen ( 1.2345 );
#   if MIN_IS_LOOSE
	cout << "n2: " << n2 << endl;
#   endif
    MIN_CHECK ( min::is_num ( n2 ) );
    MIN_CHECK ( min::is_name ( n2 ) );
    MIN_CHECK ( min::float_of ( n2 ) == 1.2345 );
    MIN_CHECK ( MUP::float_of ( n2 ) == 1.2345 );
    min::uns32 n2hash = min::numhash ( n2 );
    cout << "n2hash: " << hex << n2hash << dec
	 << endl;
    MIN_CHECK
	( n2hash == min::floathash ( 1.2345 ) );
    MIN_CHECK
	( n2hash == min::hash ( n2 ) );
    MIN_CHECK
	( min::new_num_gen ( 1.2345 ) == n2 );

    min::gen n3 = min::new_num_gen ( 1 << 30 );
#   if MIN_IS_LOOSE
	cout << "n3: " << n3 << endl;
#   endif
    MIN_CHECK ( min::is_num ( n3 ) );
    MIN_CHECK ( min::is_name ( n3 ) );
    MIN_CHECK ( min::int_of ( n3 ) == 1 << 30 );
    MIN_CHECK ( min::float_of ( n3 ) == 1 << 30 );
    MIN_CHECK ( MUP::float_of ( n3 ) == 1 << 30 );
    min::uns32 n3hash = min::numhash ( n3 );
    cout << "n3hash: " << hex << n3hash << dec
	 << endl;
    MIN_CHECK
	( n3hash == min::floathash ( 1 << 30 ) );
    MIN_CHECK
	( n3hash == min::hash ( n3 ) );
    MIN_CHECK
	( min::new_num_gen ( 1 << 30 ) == n3 );

    cout << endl;
    cout << "Finish Numbers Test!" << endl;
}

// Strings

void test_strings ( void )
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
    MIN_CHECK
	( min::strnhash ( s13, 8 ) == s8hash );
    MIN_CHECK
	( min::strnhash ( s13, 3 ) == s3hash );

    cout << endl;
    cout << "Test string general values:" << endl;
    min::gen strgen3 = min::new_str_gen ( s3 );
    min::gen strgen7 = min::new_str_gen ( s7 );
    min::gen strgen8 = min::new_str_gen ( s8 );
    min::gen strgen13 = min::new_str_gen ( s13 );
    MIN_CHECK (    min::new_str_gen ( s13, 8 )
		 == strgen8 );
    MIN_CHECK (    min::new_str_gen ( s13, 20 )
		 == strgen13 );

    MIN_CHECK ( min::is_str ( strgen3 ) );
    MIN_CHECK ( min::is_name ( strgen3 ) );
    MIN_CHECK ( min::is_direct_str ( strgen3 ) );
    MIN_CHECK ( min::is_str ( strgen7 ) );
    MIN_CHECK ( min::is_name ( strgen7 ) );
    MIN_CHECK ( min::is_stub ( strgen7 ) );
    MIN_CHECK ( min::is_str ( strgen8 ) );
    MIN_CHECK ( min::is_name ( strgen8 ) );
    MIN_CHECK ( min::is_stub ( strgen8 ) );
    MIN_CHECK ( min::is_str ( strgen13 ) );
    MIN_CHECK ( min::is_name ( strgen13 ) );
    MIN_CHECK ( min::is_stub ( strgen13 ) );

    MIN_CHECK ( min::strlen ( strgen3 ) == 3 );
    MIN_CHECK
	( min::strhash ( strgen3 ) == s3hash );
    MIN_CHECK
	( min::hash ( strgen3 ) == s3hash );
    min::strcpy ( buffer, strgen3 );
    MIN_CHECK ( strcmp ( buffer, s3 ) == 0 );
    MIN_CHECK ( min::strcmp ( s3, strgen3 ) == 0 );
    MIN_CHECK
	( min::new_str_gen ( buffer ) == strgen3 );
    buffer[2] = 0;
    min::strncpy ( buffer, strgen3, 2 );
    MIN_CHECK ( buffer[2] == 0 );
    MIN_CHECK
	(    min::strncmp ( buffer, strgen3, 2 )
	  == 0 );
    MIN_CHECK ( min::strlen ( strgen7 ) == 7 );
    MIN_CHECK
	( min::strhash ( strgen7 ) == s7hash );
    MIN_CHECK
	( min::hash ( strgen7 ) == s7hash );
    min::strcpy ( buffer, strgen7 );
    MIN_CHECK ( strcmp ( buffer, s7 ) == 0 );
    MIN_CHECK ( min::strcmp ( s7, strgen7 ) == 0 );
    MIN_CHECK
	( min::new_str_gen ( buffer ) == strgen7 );
    buffer[6] = 0;
    min::strncpy ( buffer, strgen7, 6 );
    MIN_CHECK ( buffer[6] == 0 );
    MIN_CHECK
	(    min::strncmp ( buffer, strgen7, 6 )
	  == 0 );
    MIN_CHECK ( min::strlen ( strgen8 ) == 8 );
    MIN_CHECK
	( min::strhash ( strgen8 ) == s8hash );
    MIN_CHECK
	( min::hash ( strgen8 ) == s8hash );
    min::strcpy ( buffer, strgen8 );
    MIN_CHECK ( strcmp ( buffer, s8 ) == 0 );
    MIN_CHECK ( min::strcmp ( s8, strgen8 ) == 0 );
    MIN_CHECK
	( min::new_str_gen ( buffer ) == strgen8 );
    buffer[7] = 0;
    min::strncpy ( buffer, strgen8, 7 );
    MIN_CHECK ( buffer[7] == 0 );
    MIN_CHECK
	(    min::strncmp ( buffer, strgen8, 7 )
	  == 0 );
    MIN_CHECK ( min::strlen ( strgen13 ) == 13 );
    MIN_CHECK
	( min::strhash ( strgen13 ) == s13hash );
    MIN_CHECK
	( min::hash ( strgen13 ) == s13hash );
    min::strcpy ( buffer, strgen13 );
    MIN_CHECK ( strcmp ( buffer, s13 ) == 0 );
    MIN_CHECK
	( min::strcmp ( s13, strgen13 ) == 0 );
    MIN_CHECK
	( min::new_str_gen ( buffer ) == strgen13 );
    buffer[12] = 0;
    min::strncpy ( buffer, strgen13, 12 );
    MIN_CHECK ( buffer[12] == 0 );
    MIN_CHECK
	(    min::strncmp ( buffer, strgen13, 12 )
	  == 0 );

    union { min::uns64 u; char s[8]; } v;
    v.u = min::strhead ( strgen3 );
    MIN_CHECK ( strcmp ( v.s, "ABC" ) == 0 );
    v.u = min::strhead ( strgen7 );
    MIN_CHECK ( strcmp ( v.s, "ABCDEFG" ) == 0 );
    v.u = min::strhead ( strgen8 );
    MIN_CHECK ( strncmp ( v.s, "ABCDEFGH", 8 ) == 0 );
    v.u = min::strhead ( strgen13 );
    MIN_CHECK ( strncmp ( v.s, "ABCDEFGH", 8 ) == 0 );

    MIN_CHECK ( min::strhead ( min::MISSING() ) == 0 );

    cout << endl;
    cout << "Test unprotected string functions:"
	 << endl;

    min::stub * stub7 = MUP::stub_of ( strgen7 );
    MIN_CHECK (    min::type_of ( stub7 )
		 == min::SHORT_STR );
    u.str = MUP::short_str_of ( stub7 );
    u.buf[8] = 0;
    MIN_CHECK ( strcmp ( u.buf, s7 ) == 0 );
    min::stub * stub8 = MUP::stub_of ( strgen8 );
    MIN_CHECK (    min::type_of ( stub8 )
		 == min::SHORT_STR );
    u.str = MUP::short_str_of ( stub8 );
    u.buf[8] = 0;
    MIN_CHECK ( strcmp ( u.buf, s8 ) == 0 );

    min::stub * stub13 = MUP::stub_of ( strgen13 );
    MIN_CHECK (    min::type_of ( stub13 )
		 == min::LONG_STR );
    MUP::long_str * lstr13 =
	MUP::long_str_of ( stub13 );
    MIN_CHECK ( MUP::length_of ( lstr13 ) == 13 );
    MIN_CHECK
	( MUP::hash_of ( lstr13 ) == s13hash );
    MIN_CHECK
	(    strcmp ( MUP::str_of ( lstr13 ), s13 )
	  == 0 );

    cout << endl;
    cout << "Test protected string ptrs:" << endl;

    // Test body relocation first.
    MIN_CHECK
	( MUP::body_size_of ( stub13 )
	  ==
	  sizeof ( MUP::long_str ) + 13 + 1 );
    resize_body
	( stub13, MUP::body_size_of ( stub13 ),
		  MUP::body_size_of ( stub13 ) );
    MIN_CHECK ( min::strlen ( strgen13 ) == 13 );
    MIN_CHECK
	( min::strhash ( strgen13 ) == s13hash );
    min::strcpy ( buffer, strgen13 );
    MIN_CHECK ( strcmp ( buffer, s13 ) == 0 );

    min::str_ptr p3 ( strgen3 );
    min::str_ptr p7 ( strgen7 );
    min::str_ptr p8 ( strgen8 );
    min::str_ptr p13 ( strgen13 );

    MIN_CHECK ( min::strcmp ( s3, p3 ) == 0 );
    MIN_CHECK ( min::strcmp ( s7, p7 ) == 0 );
    MIN_CHECK ( min::strcmp ( s8, p8 ) == 0 );
    MIN_CHECK ( min::strcmp ( s13, p13 ) == 0 );

    MIN_CHECK ( s3[0] == p3[0] );
    MIN_CHECK ( s3[1] == p3[1] );
    MIN_CHECK ( s3[2] == p3[2] );
    MIN_CHECK ( s3[3] == p3[3] );
    MIN_CHECK ( s7[0] == p7[0] );
    MIN_CHECK ( s7[6] == p7[6] );
    MIN_CHECK ( s7[7] == p7[7] );
    MIN_CHECK ( s8[0] == p8[0] );
    MIN_CHECK ( s8[7] == p8[7] );
    MIN_CHECK ( s8[8] == p8[8] );
    MIN_CHECK ( s13[0] == p13[0] );
    MIN_CHECK ( s13[12] == p13[12] );
    MIN_CHECK ( s13[13] == p13[13] );

    min::strcpy ( buffer, p3 );
    MIN_CHECK ( strcmp ( buffer, s3 ) == 0 );
    min::strcpy ( buffer, p13 );
    MIN_CHECK ( strcmp ( buffer, s13 ) == 0 );
    buffer[5] = 0;
    MIN_CHECK ( strncmp ( buffer, p13, 5 ) == 0 );
    MIN_CHECK ( strncmp ( buffer, p13, 6 ) != 0 );
    buffer[4] = 0;
    buffer[5] = 'X';
    min::strncpy ( buffer, p13, 5 );
    MIN_CHECK ( buffer[4] == s13[4] );
    MIN_CHECK ( buffer[5] == 'X' );

    MIN_CHECK ( min::strlen ( p3 ) == 3 );
    MIN_CHECK ( min::strlen ( p7 ) == 7 );
    MIN_CHECK ( min::strlen ( p8 ) == 8 );
    MIN_CHECK ( min::strlen ( p13 ) == 13 );

    MIN_CHECK (    min::strhash ( p3 )
		 == min::strhash ( s3 ) );
    MIN_CHECK (    min::strhash ( p7 )
		 == min::strhash ( s7 ) );
    MIN_CHECK (    min::strhash ( p8 )
		 == min::strhash ( s8 ) );
    MIN_CHECK (    min::strhash ( p13 )
		 == min::strhash ( s13 ) );

    const char * p13str_before =
	min::unprotected::str_of ( p13 );
    MIN_CHECK
	( strcmp ( p13str_before, s13 ) == 0 );
    resize_body
	( stub13, MUP::body_size_of ( stub13 ),
		  MUP::body_size_of ( stub13 ) );
    const char * p13str_after =
	min::unprotected::str_of ( p13 );
    MIN_CHECK ( p13str_after != p13str_before );
    MIN_CHECK ( min::strcmp ( s13, p13 ) == 0 );

    min::str_ptr p = strgen13;
    MIN_CHECK ( strcmp ( s13, p ) == 0 );
    min::str_ptr pb = p;
    p = strgen8;
    MIN_CHECK ( strcmp ( s8, p ) == 0 );
    MIN_CHECK ( strcmp ( s13, pb ) == 0 );
    min::str_ptr pc;
    pb = min::NULL_STUB;
    pc = strgen13;
    MIN_CHECK ( strcmp ( s13, pc ) == 0 );

    min::gen sempty = min::new_str_gen ( "" );
    min::gen s1234 = min::new_str_gen ( "1234" );
    min::gen s1234x = min::new_str_gen ( "1234x" );
    min::gen sm1234 = min::new_str_gen ( "-1234" );
    min::gen sm1234x = min::new_str_gen ( "-1234x" );
    min::gen sA8 = min::new_str_gen ( "A8" );

    min::int32 si;
    MIN_CHECK ( min::strto ( si, s1234 ) );
    MIN_CHECK ( si == 1234 );
    MIN_CHECK ( min::strto ( si, sm1234 ) );
    MIN_CHECK ( si == -1234 );
    si = 55;
    MIN_CHECK ( ! min::strto ( si, s1234x ) );
    MIN_CHECK ( ! min::strto ( si, sm1234x ) );
    MIN_CHECK ( ! min::strto ( si, sempty ) );
    MIN_CHECK ( ! min::strto ( si, min::MISSING() ) );
    MIN_CHECK ( si == 55 );
    MIN_CHECK ( min::strto ( si, sA8, 16 ) );
    MIN_CHECK ( si == 168 );


    min::gen s1234567890 =
	min::new_str_gen ( "1234567890" );
    min::gen s12345678900 =
	min::new_str_gen ( "12345678900" );
    min::gen s12345678901234567890 =
	min::new_str_gen ( "12345678901234567890" );
    min::gen s123456789012345678900 =
	min::new_str_gen ( "123456789012345678900" );
    si = 0;
    MIN_CHECK ( min::strto ( si, s1234567890 ) );
    MIN_CHECK ( si == 1234567890 );
    MIN_CHECK ( ! min::strto ( si, s12345678900 ) );
    MIN_CHECK ( si == 1234567890 );

    min::uns64 sli = 0;
    MIN_CHECK
        ( min::strto ( sli, s12345678901234567890 ) );
    MIN_CHECK ( sli == 12345678901234567890ull );
    MIN_CHECK
        ( ! min::strto
		( sli, s123456789012345678900 ) );
    MIN_CHECK ( ! min::strto ( sli, sempty ) );
    MIN_CHECK ( sli == 12345678901234567890ull );

    min::gen s1e38 = min::new_str_gen ( "1e38" );
    min::gen s1e39 = min::new_str_gen ( "1e39" );
    min::gen s1em37 = min::new_str_gen ( "1e-37" );
    // If denormalized numbers are supported, 1e-45 will
    // be convertable with loss of precision, but
    // 1e-46 will not be.
    min::gen s1em46 = min::new_str_gen ( "1e-46" );

    min::float32 sf = 0;
    MIN_CHECK ( min::strto ( sf, s1e38 ) );
    MIN_CHECK ( sf == 1e38f );
    MIN_CHECK ( min::strto ( sf, s1em37 ) );
    MIN_CHECK ( sf == 1e-37f );
    MIN_CHECK ( ! min::strto ( sf, s1e39 ) );
    MIN_CHECK ( ! min::strto ( sf, s1em46 ) );
    MIN_CHECK ( ! min::strto ( sf, sempty ) );
    MIN_CHECK ( sf == 1e-37f );

    min::gen s1e308 = min::new_str_gen ( "1e308" );
    min::gen s1e308x = min::new_str_gen ( "1e308x" );
    min::gen s1e309 = min::new_str_gen ( "1e309" );
    min::gen s1em307 = min::new_str_gen ( "1e-307" );
    // If denormalized numbers are supported, 1e-323
    // will be convertable with loss of precision, but
    // 1e-324 will not be.
    min::gen s1em324 = min::new_str_gen ( "1e-324" );

    min::float64 sd = 0;
    MIN_CHECK ( min::strto ( sd, s1e308 ) );
    MIN_CHECK ( sd == 1e308 );
    MIN_CHECK ( min::strto ( sd, s1em307 ) );
    MIN_CHECK ( sd == 1e-307 );
    MIN_CHECK ( ! min::strto ( sd, s1e308x ) );
    MIN_CHECK ( ! min::strto ( sd, s1e309 ) );
    MIN_CHECK ( ! min::strto ( sd, s1em324 ) );
    MIN_CHECK ( ! min::strto ( sd, sempty ) );
    MIN_CHECK ( sd == 1e-307 );

    min::gen snums =
        min::new_str_gen ( "1 -2e 3e10 -4e-10X5" );
    min::str_ptr snumsp ( snums );
    int j = 0;
    MIN_CHECK ( min::strto ( si, snumsp, j ) );
    MIN_CHECK ( si == 1 );
    MIN_CHECK ( min::strto ( si, snumsp, j ) );
    MIN_CHECK ( si == -2 );
    MIN_CHECK ( snumsp[j] == 'e' );
    ++ j;
    MIN_CHECK ( min::strto ( sd, snumsp, j ) );
    MIN_CHECK ( sd == 3e10 );
    MIN_CHECK ( min::strto ( sd, snumsp, j ) );
    MIN_CHECK ( sd == -4e-10 );
    MIN_CHECK ( snumsp[j] == 'X' );
    ++ j;
    MIN_CHECK ( min::strto ( si, snumsp, j ) );
    MIN_CHECK ( si == 5 );
    MIN_CHECK ( snumsp[j] == 0 );

    min::gen sspace = min::new_str_gen ( " " );
    min::gen spoint = min::new_str_gen ( " ." );
    min::gen splus  = min::new_str_gen ( " +" );
    min::gen sminus = min::new_str_gen ( " -" );
    min::gen sispace = min::new_str_gen ( " -123  " );
    min::gen sdspace =
        min::new_str_gen ( " -123.4e-15  " );
    MIN_CHECK ( ! min::strto ( si, sspace ) );
    MIN_CHECK ( ! min::strto ( sd, spoint ) );
    MIN_CHECK ( ! min::strto ( si, splus ) );
    MIN_CHECK ( ! min::strto ( si, sminus ) );
    MIN_CHECK ( min::strto ( si, sispace ) );
    MIN_CHECK ( si == -123 );
    MIN_CHECK ( min::strto ( sd, sdspace ) );
    MIN_CHECK ( sd == -123.4e-15 );

    // `A' with latin1 diacritics
    //
    const min::Uchar Ubuffer[7] =
        { 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6 };
    {
	char sbuffer[2*7+1];
	char * s = sbuffer;
	const min::Uchar * u = Ubuffer;
	min::unsptr len = min::unicode_to_utf8
	    ( s, s + sizeof ( sbuffer ),
	      u, Ubuffer + 7 );
	MIN_CHECK ( s - sbuffer == 2*7 );
	MIN_CHECK ( len == 2*7 );
	* s = 0;
	cout << sbuffer << endl;

	min::Uchar Ubuffer2[8];
	min::Uchar * u2 = Ubuffer2;
	const char * s2 = sbuffer;
	len = min::utf8_to_unicode
	    ( u2, u2 + 8, s2, s2 + 2 * 7 );
	MIN_CHECK ( len == 7 );
	MIN_CHECK ( u2 - Ubuffer2 == 7 );
	MIN_CHECK ( s == sbuffer + 2 * 7 );
	MIN_CHECK (    memcmp ( Ubuffer, Ubuffer2,
	                         sizeof ( Ubuffer ) )
		     == 0 );

	u2 = Ubuffer2;
	s2 = sbuffer;
	len = min::utf8_to_unicode
	    ( u2, u2 + 8, s2, s2 + 2 * 7 - 1 );
	MIN_CHECK ( len == 7 );
	MIN_CHECK
	    ( Ubuffer2[6] == min::UNKNOWN_UCHAR );

	char sbuffer3[20];
	char * s3 = sbuffer3;
	const min::Uchar * u3 = Ubuffer2;
	len = min::unicode_to_utf8
	    ( s3, s3 + sizeof ( sbuffer3 ),
	      u3, u3 + 7 );
	* s3 = 0;
	cout << sbuffer3 << endl;

    }
    
    cout << endl;
    cout << "Finish Strings Test!" << endl;
}

// Labels
// ------

void test_labels ( void )
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
    MIN_CHECK ( min::is_lab ( lab ) );
    MIN_CHECK ( min::is_name ( lab ) );
    MIN_CHECK ( min::is_stub ( lab ) );
    const min::stub * s = min::stub_of ( lab );
    MIN_CHECK ( min::labhash ( s ) == labhash1 );
    MIN_CHECK ( min::lablen ( s ) == 3 );
    MIN_CHECK ( min::labhash ( lab ) == labhash1 );
    MIN_CHECK ( min::lablen ( lab ) == 3 );
    MIN_CHECK ( MUP::body_size_of ( s )
		 ==
		 3 * sizeof ( min::gen )
		 +
		 sizeof ( MINT::lab_header ) );
    MIN_CHECK ( min::hash ( lab ) == labhash1 );
    MIN_CHECK
	( min::labncpy ( labv2, s, 5 ) == 3 );
    MIN_CHECK
	( min::new_lab_gen ( labv2, 3 ) == lab );
    MIN_CHECK
	( min::labncpy ( labv2, lab, 5 ) == 3 );
    MIN_CHECK
	( min::new_lab_gen ( labv2, 3 ) == lab );

    min::lab_ptr labp ( lab );
    MIN_CHECK ( labp != min::NULL_STUB );
    MIN_CHECK ( labp[0] == labv1[0] );
    MIN_CHECK ( labp[1] == labv1[1] );
    MIN_CHECK ( labp[2] == labv1[2] );
    MIN_CHECK ( min::lablen ( labp ) == 3);
    MIN_CHECK ( min::labhash ( labp ) == labhash1 );
    labp = labv1[0];
    MIN_CHECK ( labp == min::NULL_STUB );
    labp = lab;
    MIN_CHECK ( min::lablen ( labp ) == 3);
    min::lab_ptr labp1 ( labv1[0] );
    MIN_CHECK ( labp1 == min::NULL_STUB );
    min::lab_ptr labp2 ( labv1[0] );
    MIN_CHECK ( labp2 == min::NULL_STUB );

    cout << "LABEL " << lab << endl;

    MIN_CHECK
        ( -1 == min::is_subsequence ( lab, labv1[0] ) );
    MIN_CHECK
        ( 0 == min::is_subsequence
	           ( labv1[0], labv1[0] ));
    MIN_CHECK
        ( -1 == min::is_subsequence
	           ( labv1[0], labv1[1] ));
    MIN_CHECK
        ( 0 == min::is_subsequence ( labv1[0], lab ) );
    MIN_CHECK
        ( 1 == min::is_subsequence ( labv1[1], lab ) );
    MIN_CHECK
        ( 2 == min::is_subsequence ( labv1[2], lab ) );
    MIN_CHECK
        ( 0 == min::is_subsequence ( lab, lab ) );
    MIN_CHECK
        ( -1 == min::is_subsequence
	    ( min::new_str_gen ( "66" ), lab ) );
    MIN_CHECK
        ( 0 == min::is_subsequence
	    ( min::new_lab_gen ( labv1, 2 ), lab ) );
    MIN_CHECK
        ( 1 == min::is_subsequence
	    ( min::new_lab_gen ( labv1 + 1, 2 ),
	      lab ) );
    labv2[0] = labv1[2];
    labv2[1] = labv1[1];
    MIN_CHECK
        ( -1 == min::is_subsequence
	    ( min::new_lab_gen ( labv2, 2 ), lab ) );
    
    cout << endl;
    cout << "Finish Labels Test!" << endl;
}

// Names
// -----

void test_names ( void )
{
    cout << endl;
    cout << "Start Names Test!" << endl;

    min::gen num1 = min::new_num_gen ( 1 );
    min::gen num2 = min::new_num_gen ( 2 );

    min::gen str1 = min::new_str_gen ( "str 1" );
    min::gen str2 = min::new_str_gen ( "str 2" );

    min::gen l11[1] = { num1 };
    min::gen l12[1] = { num2 };
    min::gen l21[2] = { num1, str1 };
    min::gen l22[2] = { num1, str2 };
    min::gen l23[2] = { num2, str1 };
    min::gen l24[2] = { num2, str2 };

    min::gen lab11 = min::new_lab_gen ( l11, 1 );
    min::gen lab12 = min::new_lab_gen ( l12, 1 );
    min::gen lab21 = min::new_lab_gen ( l21, 2 );
    min::gen lab22 = min::new_lab_gen ( l22, 2 );
    min::gen lab23 = min::new_lab_gen ( l23, 2 );
    min::gen lab24 = min::new_lab_gen ( l24, 2 );

    cout << "HASH of new_num_gen ( 1 ) = "
         << min::hash ( num1 ) << endl;
    cout << "HASH of new_num_gen ( 2 ) = "
         << min::hash ( num2 ) << endl;
    cout << "HASH of new_str_gen ( \"str 1\" ) = "
         << min::hash ( str1 ) << endl;
    cout << "HASH of new_str_gen ( \"str 2\" ) = "
         << min::hash ( str2 ) << endl;
    cout << "HASH of new_lab_gen ( { 1.0 }, 1 ) = "
         << min::hash ( lab11 ) << endl;
    cout << "HASH of new_lab_gen ( { 2.0 }, 1 ) = "
         << min::hash ( lab12 ) << endl;
    cout << "HASH of new_lab_gen"
              " ( { 1.0, \"str 1\" }, 2 ) = "
         << min::hash ( lab21 ) << endl;
    cout << "HASH of new_lab_gen"
              " ( { 1.0, \"str 2\" }, 2 ) = "
         << min::hash ( lab22 ) << endl;

    MIN_CHECK ( min::compare ( num1, num1 ) == 0 );
    MIN_CHECK ( min::compare ( num1, num2 ) < 0 );
    MIN_CHECK ( min::compare ( num2, num1 ) > 0 );

    MIN_CHECK ( min::compare ( str1, str1 ) == 0 );
    MIN_CHECK ( min::compare ( str1, str2 ) < 0 );
    MIN_CHECK ( min::compare ( str2, str1 ) > 0 );

    MIN_CHECK ( min::compare ( lab11, lab11 ) == 0 );
    MIN_CHECK ( min::compare ( lab11, lab12 ) < 0 );
    MIN_CHECK ( min::compare ( lab12, lab11 ) > 0 );

    MIN_CHECK ( min::compare ( lab21, lab21 ) == 0 );
    MIN_CHECK ( min::compare ( lab11, lab21 ) < 0 );
    MIN_CHECK ( min::compare ( lab21, lab11 ) > 0 );

    MIN_CHECK ( min::compare ( lab21, lab21 ) == 0 );
    MIN_CHECK ( min::compare ( lab21, lab23 ) < 0 );
    MIN_CHECK ( min::compare ( lab23, lab21 ) > 0 );

    MIN_CHECK ( min::compare ( lab23, lab23 ) == 0 );
    MIN_CHECK ( min::compare ( lab23, lab24 ) < 0 );
    MIN_CHECK ( min::compare ( lab24, lab23 ) > 0 );
    
    cout << endl;
    cout << "Finish Names Test!" << endl;
}

// Packed Structures
// ------ ----------

struct ps1;
typedef min::packed_struct<ps1> ps1t;
typedef min::packed_struct_ptr<ps1> ps1ptr;
typedef min::packed_struct_updptr<ps1> ps1updptr;
struct ps1 {
    const min::uns32 control;
    min::uns32 i;
    min::gen g;
    min::stub * s;
    ps1ptr psp;
};

static min::uns32 ps1_gen_disp[2] =
    { min::DISP ( & ps1::g ), min::DISP_END };
static min::uns32 ps1_stub_disp[3] =
    { min::DISP ( & ps1::s ),
      min::DISP ( & ps1::psp ),
      min::DISP_END };

static ps1t ps1type
    ( "ps1type", ps1_gen_disp, ps1_stub_disp );

struct ps2 {
    const min::uns32 control;
    min::uns32 i;
    min::uns32 j;
};

typedef min::packed_struct<ps2> ps2t;
typedef min::packed_struct_ptr<ps2> ps2ptr;
typedef min::packed_struct_updptr<ps2> ps2updptr;
static ps2t ps2type ( "ps2type" );


void test_packed_structs ( void )
{
    cout << endl;
    cout << "Start Packed Structs Test!" << endl;

    cout << "ps1type.name = " << ps1type.name << endl;

    const min::stub * v1 = ps1type.new_stub();
    MIN_CHECK (    min::packed_subtype_of ( v1 )
                 == ps1type.subtype );
    ps1updptr upv1 ( v1 );
    MIN_CHECK (    min::packed_subtype_of ( upv1 )
                 == ps1type.subtype );
    cout << "upv1->control = " << upv1->control << endl;
    MIN_CHECK ( upv1->i == 0 );
    upv1->i = 88;
    MIN_CHECK ( upv1->i == 88 );

    min::gen v2 = ps2type.new_gen();
    ps2updptr upv2 ( v2 );
    cout << "upv2->control = " << upv2->control << endl;
    MIN_CHECK ( upv2->i == 0 );
    MIN_CHECK ( upv2->j == 0 );
    upv2->i = 55;
    upv2->j = 99;
    MIN_CHECK ( upv2->i == 55 );
    MIN_CHECK ( upv2->j == 99 );
    ps2ptr pv2 ( v2 );
    MIN_CHECK ( pv2->i == 55 );
    MIN_CHECK ( pv2->j == 99 );

    const min::stub * v3 = ps2type.new_stub();
    MIN_CHECK ( upv2 == min::stub_of ( v2 ) );
    upv2 = min::NULL_STUB;
    MIN_CHECK ( upv2 != min::stub_of ( v2 ) );
    upv2 = min::new_stub_gen ( v3 );
    MIN_CHECK ( upv2->i == 0 );
    MIN_CHECK ( upv2->j == 0 );
    upv2->i = 22;
    upv2->j = 44;
    MIN_CHECK ( upv2->i == 22 );
    MIN_CHECK ( upv2->j == 44 );
    upv2 = v2;
    MIN_CHECK ( upv2->i == 55 );
    MIN_CHECK ( upv2->j == 99 );

    ps1updptr upv1b;
    MIN_CHECK ( upv1b == min::NULL_STUB );
    upv1b = upv1;
    MIN_CHECK ( upv1b->i == 88 );

    MIN_CHECK ( upv1->psp == min::NULL_STUB );
    upv1->psp = upv1;
    MIN_CHECK ( upv1->psp->i == 88 );

    cout << endl;
    cout << "Finish Packed Structs Test!" << endl;
}

// Packed Vectors
// ------ -------

struct pve;
struct pvh;
typedef min::packed_vec<pve,pvh> pvt;
typedef min::packed_vec_ptr<pve,pvh> pvptr;
typedef min::packed_vec_updptr<pve,pvh> pvupdptr;
typedef min::packed_vec_insptr<pve,pvh> pvinsptr;
struct pvh {
    const min::uns32 control;
    min::uns32 i;
    const min::uns32 length;
    const min::uns32 max_length;
    pvinsptr pvip;
};

struct pve {

    // Invisible padding causes memcmp problems because
    // the default operator = does NOT copy it, so we
    // must be sure there is no invisible padding.

    min::gen g1;  // 2 min::gen's to avoid padding
    min::gen g2;  // before s.
    min::stub * s;
    min::uns8 j;

    min::uns8 padding
        [32 - 2 * sizeof ( min::gen )
	    - sizeof ( min::stub * ) - 1];
};

static min::uns32 pvh_stub_disp[] =
    { min::DISP ( & pvh::pvip ), min::DISP_END };
static min::uns32 pve_gen_disp[] =
    { min::DISP ( & pve::g1 ),
      min::DISP ( & pve::g2 ),
      min::DISP_END };
static min::uns32 pve_stub_disp[] =
    { min::DISP ( & pve::s ), min::DISP_END };

static pvt pvtype
    ( "pvtype", pve_gen_disp, pve_stub_disp,
                NULL,         pvh_stub_disp );

void test_packed_vectors ( void )
{
    cout << endl;
    cout << "Start Packed Vectors Test!" << endl;

    MIN_CHECK ( sizeof ( pve ) == 32 );

    cout << "pvtype.name = " << pvtype.name << endl;

    const min::stub * v = pvtype.new_stub ( 5 );
    MIN_CHECK (    min::packed_subtype_of ( v )
                 == pvtype.subtype );
    pvinsptr pvip ( v );
    MIN_CHECK (    min::packed_subtype_of ( pvip )
                 == pvtype.subtype );
    MIN_CHECK ( pvip->max_length == 5 );
    MIN_CHECK ( pvip->length == 0 );
    pve e1 = { min::MISSING(), min::ANY(),
               NULL, 88, { 0 } };
    min::push(pvip) = e1;
    MIN_CHECK ( pvip->length == 1 );
    MIN_CHECK ( (&pvip[0])->j == 88 );
    pvptr pvp = min::new_stub_gen ( v );
    MIN_CHECK ( pvp->length == 1 );
    MIN_CHECK ( (&pvp[0])->j == 88 );

    pve e2[3] = { { min::MISSING(), min::NONE(),
                    NULL, 11, { 0 } },
                  { min::MISSING(), min::NONE(),
		    NULL, 22, { 0 } },
                  { min::MISSING(), min::NONE(),
		    NULL, 33, { 0 } } };
    min::push ( pvip, 3, e2 );
    MIN_CHECK ( (&pvp[1])->j == 11 );
    MIN_CHECK ( (&pvp[2])->j == 22 );
    MIN_CHECK ( (&pvp[3])->j == 33 );


    MIN_CHECK ( pvp->length == 4 );
    MIN_CHECK ( pvp->max_length == 5 );
    min::resize ( pvip, 10 );
    MIN_CHECK ( pvp->length == 4 );
    MIN_CHECK ( pvp->max_length == 10 );

    pve e3, e4[3];
    e3 = min::pop ( pvip );
    MIN_CHECK
        ( memcmp ( & e3, & e2[2],
	           sizeof ( pve ) ) == 0 );
    MIN_CHECK ( pvip->length == 3 );
    min::pop ( pvip, 2, e4 );
    MIN_CHECK
        ( memcmp ( e4, e2, 2 * sizeof ( pve ) ) == 0 );
    MIN_CHECK ( pvip->length == 1 );
    min::pop ( pvip, 1, (pve *) NULL );
    MIN_CHECK ( pvip->length == 0 );

    pvtype.increment_ratio = 3.5;
    pvtype.max_increment = 5;
    min::push ( pvip, 3, e2 );
    MIN_CHECK ( pvip->length == 3 );
    MIN_CHECK ( pvip->max_length == 10 );
    min::reserve ( pvip, 10 );
    MIN_CHECK ( pvip->length == 3 );
    MIN_CHECK ( pvip->max_length == 15 );
    MIN_CHECK ( (&pvp[0])->j == 11 );
    MIN_CHECK ( (&pvp[1])->j == 22 );
    MIN_CHECK ( (&pvp[2])->j == 33 );

    pvp = min::NULL_STUB;
    MIN_CHECK ( pvp != v );
    pvp = v;
    MIN_CHECK ( pvp == v );
    MIN_CHECK ( (&pvp[2])->j == 33 );

    pvinsptr pvip2;
    MIN_CHECK ( pvip2 == min::NULL_STUB );
    pvip2 = pvip;
    MIN_CHECK ( (&pvip2[2])->j == 33 );

    MIN_CHECK ( pvip->length == 3 );
    MIN_CHECK ( pvip->pvip == min::NULL_STUB );
    pvip->pvip = pvp;
    min::push(pvip->pvip) = e1;
    MIN_CHECK ( pvip->length == 4 );
    MIN_CHECK ( (&pvip[3])->j == 88 );
    (&pvip->pvip[3])->j = 77;
    MIN_CHECK ( (&pvip[3])->j == 77 );

    min::push ( pvip, 3, &pvip[0] );
    MIN_CHECK ( pvip->length == 7 );
    MIN_CHECK ( (&pvp[4])->j == 11 );
    MIN_CHECK ( (&pvp[5])->j == 22 );
    MIN_CHECK ( (&pvp[6])->j == 33 );

    cout << endl;
    cout << "Finish Packed Vectors Test!" << endl;
}

// Files
// -----

void test_file ( void )
{
    cout << endl;
    cout << "Start File Test!" << endl;

    min::locatable_var<min::file> file1;
    min::init_input_string
        ( file1,
	  min::new_ptr ( "Line 1\nLine 2\nLine 3\n" ) );
    MIN_CHECK
        (    strcmp ( "Line 1",
	              ! & file1->buffer
		              [min::next_line(file1)] )
	  == 0 );
    MIN_CHECK
        (    strcmp ( "Line 2",
	              ! & file1->buffer
		              [min::next_line(file1)] )
	  == 0 );
    MIN_CHECK
        (    strcmp ( "Line 3",
	              ! & file1->buffer
		              [min::next_line(file1)] )
	  == 0 );
    MIN_CHECK
        ( min::NO_LINE == min::next_line ( file1 ) );
    MIN_CHECK
        (    strcmp ( "Line 2",
	              ! & file1->buffer
		              [min::line(file1,1)] )
	  == 0 );

    min::locatable_var<min::file> file2;
    min::init_input_named_file
        ( file2,
	  min::new_str_gen
	      ( "min_interface_test_file.in" ),
	  0 );
    MIN_CHECK
        (    strcmp ( "Line 0",
	              ! & file2->buffer
		              [min::next_line(file2)] )
	  == 0 );
    MIN_CHECK
        (    strcmp ( "Line 1",
	              ! & file2->buffer
		              [min::next_line(file2)] )
	  == 0 );
    MIN_CHECK
        ( min::NO_LINE == min::next_line ( file2 ) );

    const char * data = "Line A\nLine B\nPartial Line";
    unsigned data_length = strlen ( data );

    std::istringstream istream ( data );
    min::locatable_var<min::file> file3, file4;
    min::init_input_stream ( file3, istream );
    min::init_input ( file4 );
    min::init_ofile ( file3, file4 );
    min::flush_file ( file3 );
    MIN_CHECK ( data_length == file3->buffer->length );
    MIN_CHECK ( data_length == file4->buffer->length );
    MIN_CHECK ( strncmp ( ! & file3->buffer[0],
    			   ! & file4->buffer[0],
                           data_length ) == 0 );
    MIN_CHECK
        (    strcmp ( "Line A",
	              ! & file4->buffer
		              [min::next_line(file4)] )
	  == 0 );
    MIN_CHECK
        (    strcmp ( "Line B",
	              ! & file4->buffer
		              [min::next_line(file4)] )
	  == 0 );
    MIN_CHECK
        ( min::NO_LINE == min::next_line ( file4 ) );

    std::ostringstream ostream
        (std::ostringstream::out);
    min::locatable_var<min::file> file5;
    min::init_input_file ( file5, file4 );
    min::init_ostream ( file5, ostream );
    min::rewind ( file4 );
    min::flush_file ( file5 );
    MIN_CHECK ( data == ostream.str() );

    min::rewind ( file4 );
    min::init_ostream
        ( file5, * (std::ostream *) NULL );
    min::flush_file ( file5 );
    MIN_CHECK (    file4->buffer->length
                 == file5->buffer->length );
    MIN_CHECK (    strncmp ( ! & file4->buffer[0],
    			      ! & file5->buffer[0],
                              file4->buffer->length )
		 == 0 );

    min::end_line ( file5 );
    min::rewind ( file5, 1 );
        // We cannot rewind to line 1 as that is ==
	// file5->next_line_number.
    min::next_line ( file5 );
    MIN_CHECK
        (    strcmp ( "Partial Line",
	              ! & file5->buffer
		              [min::next_line(file5)] )
	  == 0 );
    MIN_CHECK
        ( min::NO_LINE == min::next_line ( file5 ) );

    // Tests of files + printers is deferred until
    // test of printers.

    cout << endl;
    cout << "Finish File Test!" << endl;

}

// Identifier Maps
// ---------- ----

static min::locatable_var<min::id_map> id_map;

void test_identifier_map ( void )
{
    cout << endl;
    cout << "Start Identifier Map Test!" << endl;

    min::init ( ::id_map );
    MIN_CHECK ( ::id_map->length == 1 );
    MIN_CHECK ( ::id_map->occupied == 0 );
    MIN_CHECK ( ::id_map->next == 1 );

    min::locatable_gen g1 =
        min::new_str_gen
	    ( "this is a long string" );
    min::locatable_gen g2 =
        min::new_lab_gen ( "a", "label" );
    min::locatable_gen g3 =
        min::new_obj_gen ( 10 );
    const min::stub * s1 = min::stub_of ( g1 );
    const min::stub * s2 = min::stub_of ( g2 );
    const min::stub * s3 = min::stub_of ( g3 );

    MIN_CHECK
        ( min::find ( ::id_map, s1 ) == 0 );
    MIN_CHECK
        ( min::find_or_add ( ::id_map, s1 ) == 1 );
    MIN_CHECK
        ( min::find ( ::id_map, s2 ) == 0 );
    min::insert ( ::id_map, s2, 3 );
    MIN_CHECK
        ( min::find ( ::id_map, s3 ) == 0 );
    MIN_CHECK
        ( min::find_or_add ( ::id_map, s3 ) == 4 );

    MIN_CHECK ( ::id_map->length == 5 );
    MIN_CHECK ( ::id_map[0] == min::NULL_STUB );
    MIN_CHECK ( ::id_map[1] == s1 );
    MIN_CHECK ( ::id_map[2] == min::NULL_STUB );
    MIN_CHECK ( ::id_map[3] == s2 );
    MIN_CHECK ( ::id_map[4] == s3 );

    MIN_CHECK ( ::id_map->occupied == 3 );
    MIN_CHECK ( ::id_map->next == 1 );

    MIN_CHECK
        ( min::find ( ::id_map, s1 ) == 1 );
    MIN_CHECK
        ( min::find ( ::id_map, s2 ) == 3 );
    MIN_CHECK
        ( min::find_or_add ( ::id_map, s3 ) == 4 );


    cout << endl;
    cout << "Finish Identifier Map Test!" << endl;
}

// UNICODE Name Tables
// ------- ---- ------

static min::locatable_var<min::unicode_name_table>
    unicode_table;

void test_unicode_name_table ( void )
{
    cout << endl;
    cout << "Start UNICODE Name Table Test!" << endl;

    min::init ( ::unicode_table );
    min::Uchar c = min::find ( ::unicode_table, "FF" );
    MIN_CHECK ( c == '\f' );
    c = min::find ( ::unicode_table, "DEL" );
    MIN_CHECK ( c == 0x7F );
    c = min::find ( ::unicode_table, "PAD" );
    MIN_CHECK ( c == 0x80 );
    c = min::find ( ::unicode_table, "SHY" );
    MIN_CHECK ( c == 0xAD );
    c = min::find ( ::unicode_table, "XXXXXX" );
    MIN_CHECK ( c == min::NO_UCHAR );
    c = min::find ( ::unicode_table, "YYYYY" );
    MIN_CHECK ( c == min::NO_UCHAR );

    min::add ( ::unicode_table,
               (min::ustring) "\x06\x06XXXXXX",
	       0x1234 );
    c = min::find ( ::unicode_table, "XXXXXX" );
    MIN_CHECK ( c == 0x1234 );
    c = min::find ( ::unicode_table, "YYYYY" );
    MIN_CHECK ( c == min::NO_UCHAR );

    min::add ( ::unicode_table,
               (min::ustring) "\x05\x05YYYYY",
	       0x5678 );
    c = min::find ( ::unicode_table, "XXXXXX" );
    MIN_CHECK ( c == 0x1234 );
    c = min::find ( ::unicode_table, "YYYYY" );
    MIN_CHECK ( c == 0x5678 );

    desire_success (
	min::add ( ::unicode_table,
		   (min::ustring)
		       "\x05\x05YYYYY",
		   0x5678 );
    );
    desire_failure (
	min::add ( ::unicode_table,
		   (min::ustring)
		       "\x05\x05YYYYY",
		   0x1234 );
    );
    c = min::find ( ::unicode_table, "XXXXXX" );
    MIN_CHECK ( c == 0x1234 );
    c = min::find ( ::unicode_table, "YYYYY" );
    MIN_CHECK ( c == 0x5678 );

    cout << endl;
    cout << "Finish UNICODE Name Table Test!" << endl;
}

// Printers
// --------

static min::locatable_var<min::printer> printer;

void test_printer ( void )
{
    cout << endl;
    cout << "Start Printer Test!" << endl;
    min::assert_print = false;

    min::init_ostream ( printer, std::cout );
    min::resize ( printer->file->buffer, 16*1024 );

    printer << min::bom << min::set_indent ( 4 )
            << min::set_line_length ( 16 )
            << "123456 123456789";
    MIN_CHECK
        ( printer->line_break.line_length == 16 );
    MIN_CHECK
        ( printer->line_break.indent == 4 );
    MIN_CHECK ( printer->file->end_offset == 0 );
    MIN_CHECK ( printer->column == 16 );
    MIN_CHECK ( printer->line_break.column == 7 );
    printer << " ";
    MIN_CHECK ( printer->line_break.column == 7 );
    MIN_CHECK ( printer->column == 17 );
    printer << "\t";
    MIN_CHECK ( printer->column == 24 );
    printer << "A";
    MIN_CHECK ( printer->file->end_offset == 17 );
    MIN_CHECK ( printer->column == 5 );
    printer << " B C D E F";
    MIN_CHECK ( printer->column == 15 );
    MIN_CHECK ( printer->line_break.column == 14 );
    printer << "1234";
    MIN_CHECK ( printer->file->end_offset == 31 );
    MIN_CHECK ( printer->column == 9 );
    printer << "\tab\t";
    MIN_CHECK ( printer->file->end_offset == 41 );
    MIN_CHECK ( printer->column == 8 );
    MIN_CHECK ( printer->line_break.column == 4 );
    printer << "123456789012345678901234567890";
    MIN_CHECK ( printer->file->end_offset == 48 );
    MIN_CHECK ( printer->column == 34 );
    printer << " B";
    MIN_CHECK ( printer->file->end_offset == 83 );
    MIN_CHECK ( printer->column == 5 );
    printer << min::eom;
    MIN_CHECK (    printer->file->spool_lines
                 == min::ALL_LINES );
    MIN_CHECK (    printer->file->next_offset
                 == printer->file->buffer->length );
    MIN_CHECK ( printer->column == 0 );
    MIN_CHECK
        ( printer->line_break.line_length == 72 );

    printer << min::bom << min::set_indent ( 4 ) 
            << min::break_before_all
            << min::graphic_only
            << "\300\200\001\002\003\004\005\006\007"
                   "\010\011\012\013\014\015\016\017"
                   "\020\021\022\023\024\025\026\027"
                   "\030\031\032\033\034\035\036\037"
		   "\040\177\xC2\x80\xC2\xA0\200\xC2"
            << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
            << min::break_before_all
            << min::graphic_only
            << min::display_picture
            << "\300\200\001\002\003\004\005\006\007"
                   "\010\011\012\013\014\015\016\017"
                   "\020\021\022\023\024\025\026\027"
                   "\030\031\032\033\034\035\036\037"
		   "\040\177\xC2\x80\xC2\xA0\200\xC2"
            << min::eom;

    char buffer[128*8];
    char * bp = buffer;
    for ( min::Uchar c = 0xA1; c <= 0xFF; ++ c )
    {
        if ( c % 8 == 0 ) * bp ++ = ' ';
	min::unicode_to_utf8 ( bp, c );
    }
    * bp = 0;


    printer << min::bom << min::set_indent ( 4 ) 
            << min::set_line_length ( 40 )
            << buffer
            << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
            << min::set_line_length ( 40 )
	    << min::ascii
            << buffer
            << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
            << min::set_line_length ( 40 )
            << min::graphic_only
            << min::display_picture
            << buffer << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
            << min::set_line_length ( 40 )
	    << min::ascii
            << min::break_before_all
            << min::graphic_only
	    << buffer
            << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
            << min::display_eol
            << min::display_picture
	    << "hello" << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
            << min::display_eol
	    << "hello" << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
            << min::no_auto_break;
    for ( min::uns32 i = 0; i < 100; ++ i )
        printer << i << min::right ( 4 );
    printer << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
            << min::no_auto_break;
    for ( min::uns32 i = 0; i < 100; ++ i )
        printer << i << min::left ( 4 );
    printer << min::eom;

    printer << min::save_indent
            << min::set_indent ( 4 );
    MIN_CHECK ( printer->line_break.indent == 4 );
    printer << min::set_indent ( 8 );
    MIN_CHECK ( printer->line_break.indent == 8 );
    printer << min::restore_indent;
    MIN_CHECK ( printer->line_break.indent == 4 );

    printer << "A" << min::indent << "B"
            << min::indent << "C" << min::eol;
    printer << min::bom << min::set_indent ( 4 ) 
            << min::set_line_length ( 10 )
	    << min::indent << "A B "
	    << min::reserve ( 4 ) << "C D "
	    << min::reserve ( 3 ) << "E F "
	    << min::reserve ( 2 ) << "G H I "
	    << min::reserve ( 1 ) << "J"
	    << min::eom;

    min::Uchar ubuffer[128];
    min::Uchar * ubp = ubuffer;
    for ( min::Uchar c = 0xA1; c <= 0xFF; ++ c )
    {
        if ( c % 8 == 0 ) * ubp ++ = ' ';
	* ubp ++ = c;
    }

    printer << min::bom << min::set_indent ( 4 ) 
    	    << min::set_line_length ( 40 )
            << min::punicode ( ubp - ubuffer, ubuffer )
            << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
    	    << min::punicode ( min::UNKNOWN_UCHAR )
            << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
    	    << min::set_line_length ( 20 )
            << min::no_auto_break
            <<  "int32 -1 = " << (min::int32) -1
	    << " " << min::set_break
            << "int64 -2 = " << (min::int64) -2
	    << " " << min::set_break
            << "uns32 1 = " << (min::uns32) 1
	    << " " << min::set_break
            << "uns64 2 = " << (min::uns64) 2
	    << " " << min::set_break
            << "float64 1.23 = " << (min::float64) 1.23
	    << " " << min::set_break
            << "char 'A' = " << (char) 'A'
            << min::eom;

    printer << min::bom << min::set_indent ( 4 ) 
    	    << min::set_line_length ( 20 )
            << min::no_auto_break
            <<  "pint ( -3, \"%05d\" ) = "
	    <<  min::pint ( -3, "%05d" )
	    << " " << min::set_break
            <<  "puns ( 3u, \"%05u\" ) = "
	    <<  min::puns ( 3u, "%05u" )
	    << " " << min::set_break
            <<  "pfloat ( 1.2345, \"%04.2f\" ) = "
	    <<  min::pfloat ( 1.2345, "%04.2f" )
            << min::eom;

    printer << min::pgen ( min::new_num_gen ( 1 ) )
            << min::eol;
    printer << min::pgen
                    ( min::new_num_gen ( 1.23456789 ) )
            << min::eol;
    printer << min::new_num_gen ( 1.23456789012345 )
            << min::eol;
    min::print_num ( printer, 1.23456789012345,
		     min::fraction_num_format )
            << min::eol;
    min::print_num ( printer, 0.333333334,
		     min::fraction_num_format )
            << min::eol;
    min::print_num ( printer, 0.33333334,
		     min::fraction_num_format )
            << min::eol;
    min::print_num ( printer, 1.333333334,
		     min::fraction_num_format )
            << min::eol;
    min::print_num ( printer, -0.333333334,
		     min::fraction_num_format )
            << min::eol;
    min::print_num ( printer, -1.333333334,
		     min::fraction_num_format )
            << min::eol;

    min::print_cstring
        ( printer, "this is a string" )
	<< min::eol;
    min::print_cstring
        ( printer, "this is a string",
	  min::quote_all_str_format )
	<< min::eol;
    min::print_cstring
        ( printer, "name",
          min::standard_str_format )
	<< min::eol;
    min::print_cstring
        ( printer, "name",
	  min::quote_all_str_format )
	<< min::eol;
    min::print_cstring
        ( printer, "..name",
          min::standard_str_format )
	<< min::eol;
    min::print_cstring
        ( printer, "...",
          min::standard_str_format )
	<< min::eol;

    printer << min::bom;

    printer << min::graphic_only
            << min::pgen
                   ( min::new_str_gen
			 ( "this is a string with a"
			   " quote (\")" ) )
            << min::eol;

    printer << min::display_picture
            << min::pgen
                   ( min::new_str_gen
			 ( "this is a string with a"
			   " quote (\")" ) )
            << min::eol;

    printer << min::graphic_and_hspace
	    << min::nodisplay_picture
            << min::pgen
                   ( min::new_str_gen
			 ( "this is a string with a"
			   " quote (\")" ),
	    	     min::never_quote_gen_format )
            << min::eol;


    printer << min::eom;

    min::gen lab1[2] =
        { min::new_num_gen ( 1.234 ),
	  min::new_str_gen ( "str 1" ) };
    min::gen lab2[3] =
        { min::new_num_gen ( 5.6 ),
	  min::new_lab_gen ( lab1, 2 ),
	  min::new_str_gen ( "str 2" ) };
    min::gen lab3[3] =
        { min::new_str_gen ( "A" ),
	  min::new_str_gen ( "," ),
	  min::new_str_gen ( "B" ) };
    printer << min::pgen
                    ( min::new_lab_gen ( lab2, 3 ) )
            << min::eol;
    printer << min::pgen_name
                    ( min::new_lab_gen ( lab3, 3 ) )
	    << " / "
            << min::pgen
                    ( min::new_lab_gen ( lab3, 3 ) )
            << min::eol;

    printer << min::bom
            << min::set_gen_format
	           ( min::never_quote_gen_format );

    printer << min::pgen ( min::MISSING() ) << min::eol;
    printer << min::pgen ( min::NONE() ) << min::eol;
    printer << min::pgen ( min::ANY() ) << min::eol;
    printer << min::pgen ( min::MULTI_VALUED() )
            << min::eol;
    printer << min::pgen ( min::UNDEFINED() )
            << min::eol;
    printer << min::pgen ( min::SUCCESS() ) << min::eol;
    printer << min::pgen ( min::FAILURE() ) << min::eol;
    printer << min::pgen ( min::ERROR() ) << min::eol;
    printer << min::pgen
                    ( min::new_special_gen (0xABCDEF) )
            << min::eol;
    printer << min::pgen
                    ( min::new_special_gen (0x111),
		      min::top_gen_format )
            << min::eol;
    printer << min::pgen
                    ( min::MISSING(),
		      min::top_gen_format )
            << min::eol;

    printer << min::eom;
    
    min::stub * s = MUP::new_aux_stub();
    printer << min::pgen ( min::new_stub_gen ( s ) )
            << min::eol;
    MUP::set_type_of ( s, min::RELOCATE_BODY );
    printer << min::pgen ( min::new_stub_gen ( s ) )
            << min::eol;
    MUP::set_type_of ( s, 0 );
    printer << min::pgen ( min::new_stub_gen ( s ) )
            << min::eol;

    printer << min::pgen
                    ( min::new_list_aux_gen ( 10 ) )
            << min::eol;
    printer << min::pgen
                    ( min::new_sublist_aux_gen ( 20 ) )
            << min::eol;
    printer << min::pgen
                    ( min::new_indirect_aux_gen ( 30 ) )
            << min::eol;
    printer << min::pgen ( min::new_index_gen ( 40 ) )
            << min::eol;
    printer << min::pgen ( min::new_control_code_gen
                           ( 0xFEDCBA ) )
            << min::eol;

    printer << min::pgen
                    ( min::unprotected::new_gen
                      ( (min::unsgen ) min::GEN_ILLEGAL
		        << min::VSIZE ) )
            << min::eol;

    printer << min::bom
            << min::pgen ( min::MISSING() )
	    << " "
	    << min::pgen_never_quote ( min::MISSING() )
	    << " "
	    << min::pgen ( min::MISSING() )
            << min::eom;

    printer << min::save_print_format
            << min::noflush_on_eol
	    << "The line being flushed" << min::eol;
    std::cout << "A flush is next:" << std::endl;
    printer << min::flush;
    printer << min::restore_print_format;

    printer << min::save_print_format;

    min::uns32 column;
    char ctemp[20];
    char * ctp;
#   define WTEST(c) \
	printer << min::punicode ( c ); \
	ctp = ctemp; \
	min::unicode_to_utf8 ( ctp, c ); \
	* ctp = 0; \
	min::pwidth \
	    ( column, ctemp, strlen ( ctemp ), \
	      printer->print_format ); \
	MIN_CHECK ( printer->column == column );

    MIN_CHECK ( printer->column == 0 );
    column = 0;
    printer << min::display_picture
            << min::graphic_and_vhspace;
    WTEST ( '\f' );
    WTEST ( 'a' );
    WTEST ( '\001' );
    WTEST ( ' ' );
    WTEST ( '\t' );
    WTEST ( 0x7F );
    WTEST ( 0x2400 );
    WTEST ( min::UNKNOWN_UCHAR );
    printer << min::eol;

    printer << min::nodisplay_picture;
    MIN_CHECK ( printer->column == 0 );
    column = 0;
    WTEST ( '\f' );
    WTEST ( 'a' );
    WTEST ( '\001' );
    WTEST ( ' ' );
    WTEST ( '\t' );
    WTEST ( 0x7F );
    WTEST ( 0x2400 );
    WTEST ( min::UNKNOWN_UCHAR );
    printer << min::eol;

    printer << min::break_before_all
            << min::graphic_only
            << min::nodisplay_picture;
    MIN_CHECK ( printer->column == 0 );
    column = 0;
    WTEST ( 'a' );
    WTEST ( '\001' );
    WTEST ( ' ' );
    WTEST ( '\t' );
    WTEST ( 0x7F );
    WTEST ( 0x2400 );
    WTEST ( min::UNKNOWN_UCHAR );
    printer << min::eol;

    printer << min::display_picture;
    column = 0;
    MIN_CHECK ( printer->column == 0 );
    WTEST ( 'a' );
    WTEST ( '\001' );
    WTEST ( ' ' );
    WTEST ( '\t' );
    WTEST ( 0x7F );
    WTEST ( 0x2400 );
    WTEST ( min::UNKNOWN_UCHAR );
    printer << min::eol;

    printer << min::restore_print_format;

    // Test of non-simultaneous line breaks.

    printer << min::bom << min::set_indent ( 2 )
            << min::set_line_length ( 72 )
	    << min::break_after_space
            << "[ "
            << "aaa, "
            << "bbb, "
            << "ccc, "
            << "ddd, "
            << "eee, "
            << "fff, "
            << "ggg"
            << " ]"
	    << min::eom;

    printer << min::bom << min::set_indent ( 2 )
            << min::set_line_length ( 14 )
	    << min::break_after_space
            << "[ "
            << "aaa, "
            << "bbb, "
            << "ccc, "
            << "ddd, "
            << "eee, "
            << "fff, "
            << "ggg"
            << " ]"
	    << min::eom;

    // Test of adjustments with line breaks.

    printer << min::bom << min::set_line_length ( 12 )
            << min::set_break << "0" << min::left ( 5 )
            << min::set_break << "1" << min::left ( 5 )
            << min::set_break << "2" << min::left ( 5 )
            << min::set_break << "3" << min::left ( 5 )
            << min::set_break << "4" << min::left ( 5 )
            << min::set_break << "555"
	                      << min::left ( 5 )
            << min::set_break << "666"
	                      << min::left ( 5 )
            << min::set_break << "777"
	                      << min::left ( 5 )
	    << min::eom;

    printer << min::bom << min::set_line_length ( 12 )
            << min::set_break << "A" << min::right ( 5 )
            << min::set_break << "B" << min::right ( 5 )
            << min::set_break << "C" << min::right ( 5 )
            << min::set_break << "D" << min::right ( 5 )
            << min::set_break << "E" << min::right ( 5 )
            << min::set_break << "FFF"
	                      << min::right ( 5 )
            << min::set_break << "GGG"
	                      << min::right ( 5 )
            << min::set_break << "HHH"
	                      << min::right ( 5 )
            << min::set_break << "III"
	                      << min::right ( 5 )
	    << min::eom;

    // Test of mutiple simultaneous line breaks.

    printer << min::bom << min::set_line_length ( 72 )
            << min::set_break << "{ "
	    << min::save_indent
            << min::set_break << "aaa, "
            << min::set_break << "bbb, "
            << min::set_break << "[ "
	    << min::save_indent 
            << min::set_break << "ccc, "
            << min::set_break << "ddd, "
            << min::set_break << "eee, "
            << min::set_break << "( "
	    << min::save_indent 
            << min::set_break << "fff, "
            << min::set_break << "ggg"
            << " ), " << min::restore_indent 
            << min::set_break << "hhh"
            << " ], " << min::restore_indent 
            << min::set_break << "iii, "
            << min::set_break << "jjj"
            << " }" << min::restore_indent 
            << min::eom;
    printer << min::bom << min::set_line_length ( 34 )
            << min::set_break << "{ "
	    << min::save_indent
            << min::set_break << "aaa, "
            << min::set_break << "bbb, "
            << min::set_break << "[ "
	    << min::save_indent 
            << min::set_break << "ccc, "
            << min::set_break << "ddd, "
            << min::set_break << "eee, "
            << min::set_break << "( "
	    << min::save_indent 
            << min::set_break << "fff, "
            << min::set_break << "ggg"
            << " ), " << min::restore_indent 
            << min::set_break << "hhh"
            << " ], " << min::restore_indent 
            << min::set_break << "iii, "
            << min::set_break << "jjj"
            << " }" << min::restore_indent 
            << min::eom;

    // Tests of min::leading/trailing.
    //
    min::print_item
        ( printer, "(", 1, 1,
	  min::IS_LEADING + min::IS_GRAPHIC );
    min::print_leading ( printer );
    min::print_item
        ( printer, "100", 3, 3, min::IS_GRAPHIC );
    min::print_trailing_always ( printer );
    min::print_item
        ( printer, ",", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    min::print_item
        ( printer, " ", 1, 1, 0 );
    min::print_item
        ( printer, "200", 3, 3, min::IS_GRAPHIC );
    min::print_trailing ( printer );
    min::print_item
        ( printer, ")", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    printer << min::eol;

    printer << min::force_space;
    min::print_item
        ( printer, "(", 1, 1,
	  min::IS_LEADING + min::IS_GRAPHIC );
    min::print_leading ( printer );
    min::print_item
        ( printer, "100", 3, 3, min::IS_GRAPHIC );
    min::print_trailing_always ( printer );
    min::print_item
        ( printer, ",", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    min::print_item
        ( printer, " ", 1, 1, 0 );
    min::print_item
        ( printer, "200", 3, 3, min::IS_GRAPHIC );
    min::print_trailing ( printer );
    min::print_item
        ( printer, ")", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    printer << min::noforce_space << min::eol;

    printer << min::force_pgen
            << "(" << min::leading
	    << "100" << min::trailing_always << ","
	    << min::space
	    << "200" << min::trailing << ")"
	    << min::eol
	    << min::force_space
            << "(" << min::leading
	    << "100" << min::trailing_always << ","
	    << min::space
	    << "200" << min::trailing << ")"
	    << min::eol
	    << min::noforce_space
	    << min::noforce_pgen;

    min::print_item
        ( printer, "(", 1, 1,
	  min::IS_LEADING + min::IS_GRAPHIC );
    min::print_leading ( printer );
    min::print_item
        ( printer, "100", 3, 3, min::IS_GRAPHIC );
    min::print_trailing_always ( printer );
    min::print_item
        ( printer, ",", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    min::print_item
        ( printer, " ", 1, 1, 0 );
    min::print_trailing ( printer );
    min::print_item
        ( printer, ")", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    printer << min::eol;

    printer << min::force_space;
    min::print_item
        ( printer, "(", 1, 1,
	  min::IS_LEADING + min::IS_GRAPHIC );
    min::print_leading ( printer );
    min::print_item
        ( printer, "100", 3, 3, min::IS_GRAPHIC );
    min::print_trailing_always ( printer );
    min::print_item
        ( printer, ",", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    min::print_item
        ( printer, " ", 1, 1, 0 );
    min::print_trailing ( printer );
    min::print_item
        ( printer, ")", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    printer << min::noforce_space << min::eol;

    printer << min::force_pgen
            << "(" << min::leading
	    << "100" << min::trailing_always << ","
	    << min::space
	    << min::trailing << ")"
	    << min::eol
	    << min::force_space
            << "(" << min::leading
	    << "100" << min::trailing_always << ","
	    << min::space
	    << min::trailing << ")"
	    << min::eol
	    << min::noforce_space
	    << min::noforce_pgen;

    // Tests of files and printers.

    min::locatable_var<min::file> file;
    min::init_printer ( file, printer);
    min::init_input_string
        ( file,
	  min::new_ptr ( "Line 1\nLine 2\nLine 3\n\n" )
	 );
    min::init_file_name
        ( file, min::new_str_gen ( "test_file" ) );

    printer << "The file has lines: "
            << min::pline_numbers
		  ( file, 0, file->file_lines - 1 )
	    << min::eol;
    printer << "The last line is: "
            << min::pline_numbers
		  ( file, file->file_lines - 1,
		          file->file_lines - 1 )
	    << min::eol;

    min::flush_file ( file );

    min::print_line ( printer, file, 1 );
    min::print_line ( printer, file, 3 );
    min::print_line ( printer, file, 4 );
    min::print_line ( printer, file, 5 );

    min::phrase_position ppos1 =
        { { 1, 5 }, { 2, 4 } };
    min::phrase_position ppos2 =
	{ { 2, 5 }, { 5, 0 } };

    printer << "Phrase Print Test: "
            << min::pline_numbers ( file, ppos1 )
	    << min::eol;
    min::print_phrase_lines ( printer, file, ppos1 );
    printer << "Phrase Print Test: "
            << min::pline_numbers ( file, ppos2 )
	    << min::eol;
    min::print_phrase_lines
	( printer, file, ppos2,
	  '#', "<EMPTY>", "<EOF>");

    file->line_display =
        min::DISPLAY_PICTURE + min::DISPLAY_EOL;
    min::print_line ( printer, file, 1 );
    min::print_line ( printer, file, 3 );
    min::print_line ( printer, file, 4 );
    min::print_line ( printer, file, 5 );

    printer << "Phrase Print Test: "
            << min::pline_numbers ( file, ppos1 )
	    << min::eol;
    min::print_phrase_lines ( printer, file, ppos1 );
    printer << "Phrase Print Test: "
            << min::pline_numbers ( file, ppos2 )
	    << min::eol;
    min::print_phrase_lines
	( printer, file, ppos2,
	  '#', "<EMPTY>", "<EOF>");

    min::locatable_var<min::file> efile;
    min::init_input_named_file
        ( efile,
	  min::new_str_gen
	      ( "min_non_existent_file" ) );
    std::cout << min::error_message;

    min::assert_print = true;
    cout << endl;
    cout << "Finish Printer Test!" << endl;
}

// Objects
// -------

// Values shared with subsequent object tests.
//
static min::gen short_obj_gen;
static min::gen long_obj_gen;

void test_objects ( void )
{
    cout << endl;
    cout << "Start Objects Test!" << endl;

    cout << endl;
    cout << "Test short objects:" << endl;
    short_obj_gen = min::new_obj_gen ( 500, 100 );
    const min::stub * sstub =
	min::stub_of ( short_obj_gen );
    MIN_CHECK
	(    min::type_of ( sstub )
	  == min::SHORT_OBJ );
    {
	min::obj_vec_ptr svp ( sstub );
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
	MIN_CHECK ( sht >= 100 );
	MIN_CHECK ( sua >= 500 );
	MIN_CHECK ( sav == 0 );
	MIN_CHECK ( saa == 0 );
	MIN_CHECK
	    ( st == sh + sht + sav + sua + saa );
	MIN_CHECK ( MUP::body_size_of ( sstub )
		     ==
		     st * sizeof ( min::gen ) );
    }

    cout << endl;
    cout << "Test long objects:" << endl;
    long_obj_gen = min::new_obj_gen ( 70000, 7000 );
    const min::stub * lstub =
	min::stub_of ( long_obj_gen );
    MIN_CHECK
	( min::type_of ( lstub ) == min::LONG_OBJ );
    {
	min::obj_vec_ptr lvp ( long_obj_gen );
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
	MIN_CHECK ( lht >= 7000 );
	MIN_CHECK ( lua >= 70000 );
	MIN_CHECK ( lav == 0 );
	MIN_CHECK ( laa == 0 );
	MIN_CHECK
	    ( lt == lh + lht + lav + lua + laa );
	MIN_CHECK ( MUP::body_size_of ( lstub )
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

// Test vector level for object v given unused size.
// Attribute vector must be empty.
// Auxiliary area must be empty.
// Unused size must be >= 20.
//
void test_object_vector_level
     ( const char * name, min::gen v )
{
    cout << endl << name << endl;

    min::gen num0 = min::new_num_gen ( 0 );
    min::gen num1 = min::new_num_gen ( 1 );
    min::gen num2 = min::new_num_gen ( 2 );
    min::gen num3 = min::new_num_gen ( 3 );
    min::gen numv[3] = { num1, num2, num3 };
    min::gen fillv[70000];
    for ( unsigned i = 0; i < 70000; ++ i )
	fillv[i] = num0;
    min::gen outv[4];

    min::stub * sstub = MUP::stub_of ( v );

    {
	min::obj_vec_insptr vp ( sstub );
	MIN_CHECK
	    ( min::attr_size_of ( vp ) == 0 );
	MIN_CHECK
	    ( min::unused_size_of ( vp ) >= 20 );
	MIN_CHECK
	    ( min::aux_size_of ( vp ) == 0 );

	min::gen * & base = MUP::base ( vp );
	min::unsptr ht =
	    MUP::hash_offset_of ( vp );
	min::unsptr av =
	    MUP::attr_offset_of ( vp );
	min::unsptr & cua =
	    MUP::unused_offset_of ( vp );
	min::unsptr unused_size =
	    min::unused_size_of ( vp );
	min::unsptr half_unused_size =
	    unused_size / 2;
	min::unsptr total_size =
	    min::total_size_of ( vp );

	MIN_CHECK ( base[ht] == min::LIST_END() );
	min::hash ( vp, 0 ) = min::EMPTY_SUBLIST();
	MIN_CHECK
	    ( base[ht] == min::EMPTY_SUBLIST() );
	MIN_CHECK
	    (    min::hash(vp,0)
	      == min::EMPTY_SUBLIST() );

	base[av] = num0;
	MIN_CHECK
	    ( base[av+0] == num0 );
	MIN_CHECK ( cua == av + 0 );
	min::attr_push(vp) = num1;
	MIN_CHECK ( base[av] == num1 );
	MIN_CHECK ( attr ( vp, 0 ) == num1 );
	MIN_CHECK
	    ( min::attr_size_of ( vp ) == 1 );
	MIN_CHECK ( cua == av + 1 );
	base[av+1] = num0;
	base[av+2] = num0;
	base[av+3] = num0;
	MIN_CHECK ( base[av+1] == num0 );
	MIN_CHECK ( base[av+2] == num0 );
	MIN_CHECK ( base[av+3] == num0 );
	min::attr_push ( vp, 3, numv );
	vp = min::NULL_STUB;
	vp = v;
	MIN_CHECK ( base[av+1] == num1 );
	MIN_CHECK ( base[av+2] == num2 );
	MIN_CHECK ( base[av+3] == num3 );
	MIN_CHECK ( attr ( vp, 3 ) == num3 );
	MIN_CHECK
	    ( min::attr_size_of ( vp ) == 4 );
	MIN_CHECK ( cua == av + 4 );
	MIN_CHECK
	    (    min::unused_size_of ( vp )
	      == unused_size - 4 );

	min::unsptr aa = MUP::aux_offset_of ( vp );
	min::unsptr & caa =
	    MUP::aux_offset_of ( vp );
	base[aa-1] = num0;
	MIN_CHECK ( base[aa-1] == num0 );
	MIN_CHECK ( caa == aa );
	min::aux_push(vp) = num1;
	MIN_CHECK ( base[aa-1] == num1 );
	MIN_CHECK
	    (    min::aux ( vp, total_size-aa+1 )
	      == num1 );
	MIN_CHECK
	    ( min::aux_size_of ( vp ) == 1 );
	MIN_CHECK ( caa == aa - 1 );
	base[aa-2] = num0;
	base[aa-3] = num0;
	base[aa-4] = num0;
	MIN_CHECK ( base[aa-2] == num0 );
	MIN_CHECK ( base[aa-3] == num0 );
	MIN_CHECK ( base[aa-4] == num0 );
	min::aux_push ( vp, 3, numv );
	MIN_CHECK ( base[aa-4] == num1 );
	MIN_CHECK ( base[aa-3] == num2 );
	MIN_CHECK ( base[aa-2] == num3 );
	MIN_CHECK
	    (    min::aux ( vp, total_size-aa+2 )
	      == num3 );
	MIN_CHECK
	    ( min::aux_size_of ( vp ) == 4 );
	MIN_CHECK ( caa == aa - 4 );
	MIN_CHECK
	    (    min::unused_size_of ( vp )
	      == unused_size - 8 );

	min::attr_pop ( vp, 3, outv + 1 );
	outv[0] = min::attr_pop ( vp );
	MIN_CHECK ( outv[0] == num1 );
	MIN_CHECK ( outv[1] == num1 );
	MIN_CHECK ( outv[2] == num2 );
	MIN_CHECK ( outv[3] == num3 );
	MIN_CHECK
	    ( min::attr_size_of ( vp ) == 0 );
	MIN_CHECK ( cua == av + 0 );
	MIN_CHECK
	    (    min::unused_size_of ( vp )
	      == unused_size - 4 );
	desire_failure (
	    outv[0] = min::attr_pop ( vp );
	);
	desire_failure (
	    min::attr_pop ( vp, 3, outv + 1 );
	);
	min::attr_push ( vp, 4, fillv );

	min::aux_pop ( vp, 3, outv + 1 );
	outv[0] = min::aux_pop ( vp );
	MIN_CHECK ( outv[0] == num1 );
	MIN_CHECK ( outv[1] == num1 );
	MIN_CHECK ( outv[2] == num2 );
	MIN_CHECK ( outv[3] == num3 );
	MIN_CHECK
	    (    min::aux_size_of ( vp )
	      == 0 );
	MIN_CHECK ( caa == aa );
	MIN_CHECK
	    (    min::unused_size_of ( vp )
	      == unused_size - 4 );
	desire_failure (
	    outv[0] = min::aux_pop ( vp );
	);
	desire_failure (
	    min::aux_pop ( vp, 3, outv + 1 );
	);
	min::aux_push ( vp, 4, fillv );

	min::attr_push
	    ( vp, half_unused_size - 4, fillv );
	min::aux_push
	    ( vp,
	      unused_size - half_unused_size - 4,
	      fillv );
	MIN_CHECK
	    ( min::unused_size_of ( vp ) == 0 );
	MIN_CHECK ( cua == caa );

	min::unsptr attr_offset =
	    MUP::attr_offset_of ( vp );
	min::unsptr aux_offset =
	    MUP::aux_offset_of ( vp );

	min::attr ( vp, 0 ) = min::MISSING();
	MIN_CHECK
	    ( base[attr_offset] == min::MISSING() );
	MIN_CHECK ( vp[0] == min::MISSING() );

	min::attr ( vp, 0 ) = min::LIST_END();
	MIN_CHECK ( vp[0] == min::LIST_END() );

	vp[1] = min::new_list_aux_gen
		    ( total_size - aux_offset );
	min::aux ( vp, total_size - aux_offset ) =
	    min::LIST_END();
	MIN_CHECK
	    ( base[attr_offset] == min::LIST_END() );
	MIN_CHECK
	    ( base[aux_offset] == min::LIST_END() );
	MIN_CHECK
	    (    base[attr_offset + 1]
	      == min::new_list_aux_gen
		     ( total_size - aux_offset ) );

	MIN_CHECK
	    ( min::unused_size_of ( vp ) == 0 );

	min::resize ( vp, 10, 20 );
    }

    {
	min::obj_vec_ptr vp ( sstub );
	const min::gen * & base = MUP::base ( vp );
	min::unsptr total_size =
	    min::total_size_of ( vp );

	MIN_CHECK
	    ( min::var_size_of ( vp ) == 20 );
	MIN_CHECK
	    ( min::unused_size_of ( vp ) >= 10 );
	min::unsptr attr_offset =
	    MUP::attr_offset_of ( vp );
	min::unsptr aux_offset =
	    MUP::aux_offset_of ( vp );
	vp = min::NULL_STUB;
	MIN_CHECK ( vp == min::NULL_STUB );
	vp = v;
	MIN_CHECK ( vp == sstub );
	MIN_CHECK
	    ( base[attr_offset] == min::LIST_END() );
	MIN_CHECK
	    ( vp[0] == min::LIST_END() );
	MIN_CHECK
	    ( base[aux_offset] == min::LIST_END() );
	MIN_CHECK
	    (    base[attr_offset + 1]
	      == min::new_list_aux_gen
		     ( total_size - aux_offset ) );
    }

    {
	min::obj_vec_insptr vp ( sstub );
	MIN_CHECK
	    ( min::unused_size_of ( vp ) >= 10 );
	min::unsptr attr_size =
	    min::attr_size_of ( vp );
	min::unsptr aux_size =
	    min::aux_size_of ( vp );
	min::gen att_end =
	    min::attr ( vp, attr_size - 1 );
	min::gen aux_begin =
	    min::aux ( vp, aux_size );

	min::resize ( vp, 0 );
	MIN_CHECK ( min::unused_size_of ( vp ) == 0 );
	MIN_CHECK
	    (    att_end
	      == min::attr ( vp, attr_size - 1 ) );
	MIN_CHECK
	    (    aux_begin
	      == min::aux ( vp, aux_size  ) );

	MIN_CHECK ( att_end != min::SUCCESS() );
	MIN_CHECK ( aux_begin != min::SUCCESS() );
	min::attr_push ( vp ) = min::SUCCESS();
	min::aux_push ( vp ) = min::SUCCESS();
	MIN_CHECK
	    (    att_end
	      == min::attr ( vp, attr_size - 1 ) );
	MIN_CHECK
	    (    min::SUCCESS()
	      == min::attr ( vp, attr_size ) );
	MIN_CHECK
	    (    aux_begin
	      == min::aux ( vp, aux_size  ) );
	MIN_CHECK
	    (    min::SUCCESS()
	      == min::aux ( vp, aux_size + 1  ) );
    }

    {
	min::obj_vec_ptr vp ( min::new_num_gen ( 8 ) );
	MIN_CHECK ( vp == min::NULL_STUB );
	vp = v;
	MIN_CHECK ( vp == sstub );
	MIN_CHECK
	    ( min::var_size_of ( vp ) == 20 );
	vp = min::MISSING();
	MIN_CHECK ( vp == min::NULL_STUB );
    }
}

void test_object_vector_level ( void )
{
    cout << endl;
    cout << "Start Object Vector Level Test!"
	 << endl;

    test_object_vector_level
        ( "Test short object vector level:",
	  short_obj_gen );
    test_object_vector_level
        ( "Test long object vector level:",
	  long_obj_gen );

    cout << endl;
    cout << "Finish Object Vector Level Test!"
	 << endl;
}

// Object List Level

// Values shared with previous object tests.
// min::gen short_obj_gen;
// min::gen long_obj_gen;

// List level test for object v using/not-using aux
// stubs or alternate between aux stubs and aux area.
//
// Helper to insert.
//
static bool use_obj_aux_stubs;
static bool alternate_aux;
static bool resize;  // Set true to resize just once.
static void insert
    ( min::list_insptr & wlp,
      bool before, min::gen * p, unsigned n )
{
    min::obj_vec_insptr & vp =
        min::obj_vec_ptr_of ( wlp );
    min::gen numtest = min::new_num_gen ( 123456789 );
    min::gen out;

    bool saved_min_assert_print = min::assert_print;
    min::assert_print = false;
    if ( use_obj_aux_stubs || resize )
    {
	cout << "EMPTYING UNUSED AREA" << endl;
        while ( min::unused_size_of ( vp ) > 0 )
	    min::attr_push(vp) = numtest;
    }
    else
    {
	cout << "ADDING 20 ELEMENTS TO UNUSED AREA"
	     << endl;
        while ( min::unused_size_of ( vp ) < 20 )
	    out = min::attr_pop ( vp );
    }
    min::assert_print = saved_min_assert_print;

    bool resize_happened =
        min::insert_reserve
	      ( wlp, 1, n, use_obj_aux_stubs );
    MIN_CHECK
        ( resize_happened ==
	  ( ! use_obj_aux_stubs && resize ) );
    if ( resize_happened )
    {
        cout << "RESIZE HAPPENED" << endl;
        resize = false;
    }

    use_obj_aux_stubs ^= alternate_aux;
    if ( before )
	min::insert_before ( wlp, p, n );
    else
	min::insert_after ( wlp, p, n );
}
     
void test_object_list_level
    ( const char * name, min::gen v,
      bool use_obj_aux_stubs, bool alternate_aux )
{
#   if ! MIN_USE_OBJ_AUX_STUBS
	if ( use_obj_aux_stubs || alternate_aux )
	    return;
#   endif

    cout << endl << name << endl;

    min::gen numtest = min::new_num_gen ( 123456789 );
    min::gen num100 = min::new_num_gen ( 100 );
    min::gen num101 = min::new_num_gen ( 101 );
    min::gen num102 = min::new_num_gen ( 102 );
    min::gen p[3] = { num100, num101, num102 };
    min::gen out[6];

    ::use_obj_aux_stubs = use_obj_aux_stubs;
    ::alternate_aux = alternate_aux;
    ::resize = false;

    min::obj_vec_insptr vp ( v );

    min::gen * & base = MUP::base ( vp );

    bool saved_min_assert_print = min::assert_print;
    min::assert_print = false;

    // Empty aux area.
    //
    while ( min::aux_size_of ( vp ) > 0 )
        out[0] = min::aux_pop ( vp );

    // Fill attr vector with numbers, consuming all
    // of unused area.
    //
    while ( min::unused_size_of ( vp ) > 0 )
	min::attr_push(vp) = num100;

    min::assert_print = saved_min_assert_print;

    min::unsptr vorg = MUP::attr_offset_of ( vp );
    min::unsptr vsize = min::attr_size_of ( vp );
    min::unsptr usize = min::unused_size_of ( vp );
    min::unsptr tsize = min::total_size_of ( vp );

    cout << " VORG " << vorg << " VSIZE " << vsize
	 << " USIZE " << usize << " TSIZE " << tsize
	 << endl;

    min::list_ptr lp ( vp );
    min::start_vector ( lp, 0 );
    MIN_CHECK
	( min::current ( lp ) == base[vorg+0] );
    MIN_CHECK
	( min::peek ( lp ) == min::LIST_END() );
    MIN_CHECK
	( min::next ( lp ) == min::LIST_END() );
    MIN_CHECK
	( min::current ( lp ) == min::LIST_END() );
    MIN_CHECK
	( min::peek ( lp ) == min::LIST_END() );
    MIN_CHECK
	( min::next ( lp ) == min::LIST_END() );
    base[vorg+0] = numtest;
    min::start_vector ( lp, 0 );
    MIN_CHECK
	( min::current ( lp ) == base[vorg+0] );

    min::list_insptr wlp ( vp );
    min::start_vector ( wlp, 0 );
    insert ( wlp, false, p+2, 1 );
    insert ( wlp, false, p+1, 1 );
    insert ( wlp, false, p+0, 1 );
    //
    // Vector[0] list now is
    //	{ numtest, num100, num101, num102 }

    MIN_CHECK ( min::current ( wlp ) == numtest );
    MIN_CHECK ( min::peek ( wlp ) == num100 );
    MIN_CHECK ( min::next ( wlp ) == num100 );
    MIN_CHECK ( min::peek ( wlp ) == num101 );
    MIN_CHECK ( min::next ( wlp ) == num101 );

    min::update ( wlp, min::EMPTY_SUBLIST() );
    MIN_CHECK (    min::current ( wlp )
		 == min::EMPTY_SUBLIST() );
    //
    // Vector[0] list now is
    //	{ numtest, num100, {}, num102 }

    min::start_vector ( wlp, 0 );
    MIN_CHECK ( min::current ( wlp ) == numtest );
    MIN_CHECK ( min::next ( wlp ) == num100 );
    MIN_CHECK
	( min::is_sublist ( min::next ( wlp ) ) );

    min::list_insptr wslp ( vp );
    min::start_copy ( wslp, wlp );
    min::start_sublist ( wslp );
    insert ( wslp, true, p, 1 );
    MIN_CHECK ( min::current ( wslp ) == num100 );
    MIN_CHECK
        ( min::peek ( wslp ) == min::LIST_END() );
    min::insert_refresh ( wlp );
    MIN_CHECK
        ( min::is_sublist ( min::current ( wlp ) ) );
    ::resize = true;
    min::start_sublist ( wslp, wlp );
    insert ( wslp, false, p+2, 1 );
    MIN_CHECK ( min::peek ( wslp ) == num102 );
    MIN_CHECK ( min::next ( wslp ) == num102 );
    insert ( wslp, true, p+1, 1 );
    min::insert_refresh ( wlp );
    MIN_CHECK
        ( min::is_sublist ( min::current ( wlp ) ) );
    MIN_CHECK ( min::current ( wslp ) == num101 );
    MIN_CHECK ( min::peek ( wslp ) == num102 );
    MIN_CHECK ( min::next ( wslp ) == num102 );
    MIN_CHECK
        ( min::peek ( wslp ) == min::LIST_END() );
    MIN_CHECK
        ( min::next ( wslp ) == min::LIST_END() );
    //
    // Vector[0] list now is
    //	{ numtest, num100,
    //        { num100, num101, num102 }, num102 }

    min::start_sublist ( wslp, wlp );
    MIN_CHECK ( min::current ( wslp ) == num100 );
    MIN_CHECK ( min::peek ( wslp ) == num101 );
    MIN_CHECK ( min::next ( wslp ) == num101 );
    MIN_CHECK ( min::peek ( wslp ) == num102 );
    MIN_CHECK ( min::next ( wslp ) == num102 );
    MIN_CHECK
        ( min::peek ( wslp ) == min::LIST_END() );
    MIN_CHECK
        ( min::next ( wslp ) == min::LIST_END() );

    MIN_CHECK ( min::peek ( wlp ) == num102 );
    MIN_CHECK ( min::next ( wlp ) == num102 );
    MIN_CHECK ( min::peek ( wlp ) == min::LIST_END() );
    MIN_CHECK ( min::next ( wlp ) == min::LIST_END() );

    min::start_vector ( wlp, 0 );
    MIN_CHECK ( min::current ( wlp ) == numtest );
    MIN_CHECK ( min::peek ( wlp ) == num100 );
    MIN_CHECK ( min::next ( wlp ) == num100 );
    MIN_CHECK
	( min::is_sublist ( min::peek ( wlp ) ) );
    MIN_CHECK
	( min::is_sublist ( min::next ( wlp ) ) );

    min::start_sublist ( wslp, wlp );
    MIN_CHECK ( min::current ( wslp ) == num100 );
    MIN_CHECK ( min::peek ( wslp ) == num101 );
    MIN_CHECK ( min::next ( wslp ) == num101 );
    MIN_CHECK ( 1 == min::remove ( wslp, 1 ) );
    min::insert_refresh ( wlp );
    //
    // Vector[0] list now is
    //	{ numtest, num100,
    //        { num100, num102 }, num102 }
    //
    MIN_CHECK ( min::current ( wslp ) == num102 );
    MIN_CHECK
        ( min::peek ( wslp ) == min::LIST_END() );
    MIN_CHECK
        ( min::next ( wslp ) == min::LIST_END() );

    min::start_sublist ( wslp, wlp );
    MIN_CHECK ( min::current ( wslp ) == num100 );
    MIN_CHECK ( min::peek ( wslp ) == num102 );
    MIN_CHECK ( min::next ( wslp ) == num102 );
    MIN_CHECK
        ( min::peek ( wslp ) == min::LIST_END() );
    MIN_CHECK
        ( min::next ( wslp ) == min::LIST_END() );

    min::start_sublist ( wslp, wlp );
    MIN_CHECK ( 1 == min::remove ( wslp, 1 ) );
    min::insert_refresh ( wlp );
    //
    // Vector[0] list now is
    //	{ numtest, num100,
    //        { num102 }, num102 }
    //
    MIN_CHECK
        ( min::current ( wslp ) == num102 );
    MIN_CHECK
        ( min::peek ( wslp ) == min::LIST_END() );
    MIN_CHECK
        ( min::next ( wslp ) == min::LIST_END() );

    min::start_sublist ( wslp, wlp );
    MIN_CHECK
        ( min::current ( wslp ) == num102 );
    MIN_CHECK
        ( min::peek ( wslp ) == min::LIST_END() );
    MIN_CHECK
        ( min::next ( wslp ) == min::LIST_END() );

    min::start_sublist ( wslp, wlp );
    MIN_CHECK ( min::current ( wslp ) == num102 );
    MIN_CHECK ( 1 == min::remove ( wslp, 5 ) );
    min::insert_refresh ( wlp );
    //
    // Vector[0] list now is
    //	{ numtest, num100, { }, num102 }
    //
    MIN_CHECK ( min::is_list_end
                      ( min::current ( wslp ) ) );
    MIN_CHECK ( min::peek ( wlp ) == num102 );
    MIN_CHECK ( min::next ( wlp ) == num102 );
    MIN_CHECK ( min::peek ( wlp ) == min::LIST_END() );
    MIN_CHECK ( min::next ( wlp ) == min::LIST_END() );

    min::start_vector ( wlp, 0 );
    MIN_CHECK ( min::current ( wlp ) == numtest );
    MIN_CHECK ( 3 == min::remove ( wlp, 3 ) );
    //
    // Vector[0] list now is { num102 }
    //
    MIN_CHECK ( min::current ( wlp ) == num102 );
    MIN_CHECK ( min::peek ( wlp ) == min::LIST_END() );
    MIN_CHECK ( min::next ( wlp ) == min::LIST_END() );

    min::start_vector ( wlp, 0 );
    MIN_CHECK ( min::current ( wlp ) == num102 );
    MIN_CHECK ( min::peek ( wlp ) == min::LIST_END() );
    MIN_CHECK ( min::next ( wlp ) == min::LIST_END() );

    min::start_vector ( wlp, 0 );
    MIN_CHECK ( 1 == min::remove ( wlp, 3 ) );
    //
    // Vector[0] list now is { }
    //
    MIN_CHECK
	( min::current ( wlp ) == min::LIST_END() );
    min::start_vector ( wlp, 0 );
    MIN_CHECK
	( min::current ( wlp ) == min::LIST_END() );
}

void test_object_list_level ( void )
{
    cout << endl;
    cout << "Start Object List Level Test!"
	 << endl;

    test_object_list_level
        ( "Test short object aux stubs list level:",
	  short_obj_gen, true, false );
    test_object_list_level
        ( "Test short object aux area list level:",
	  short_obj_gen, false, false );
    test_object_list_level
        ( "Test short object alternate aux list level:",
	  short_obj_gen, false, true );

    cout << endl;
    cout << "Finish Object List Level Test!"
	 << endl;
}

// Object Attribute Level

// Call get_attrs ( ap ), sort the resulting packed
// vec entries by label, and compare to aip[0 .. n-1].
// Print differences and return false if there are any
// differences.  Return true if there are no
// differences.
//
static bool check_attr_info
        ( min::attr_insptr & ap,
	  min::attr_info * aip, unsigned n,
	  bool include_attr_vec = true )
{
    bool save_min_assert_print = min::assert_print;
    min::assert_print = false;
    min::attr_info aiv[n+100];
    min::unsptr m = min::get_attrs
        ( aiv, n+100, ap, include_attr_vec );
    min::sort_attr_info ( aiv, m );

    if ( m != n )
        cout << "ACTUAL NUMBER OF ATTRIBUTES " << m
	     << " != EXPECTED NUMBER " << n << endl;

    bool ok = true;
    for ( unsigned i = 0; i < m; ++ i )
    {
        if ( i >= n ) continue;

        if ( aiv[i].name != aip[i].name )
	{
	    cout << i << ": BAD NAME: "
	         << aiv[i].name
		 << " != "
		 << aip[i].name
		 << endl;
	    ok = false;
	    continue;
	}
        if ( aiv[i].value_count != aip[i].value_count )
	{
	    cout << i << ": "
	         << aiv[i].name
	         << ": BAD VALUE COUNT: "
	         << aiv[i].value_count << " != "
		 << aip[i].value_count << endl;
	    ok = false;
	}
        if ( aiv[i].flag_count != aip[i].flag_count )
	{
	    cout << i << ": "
	         << aiv[i].name
	         << ": BAD FLAG COUNT: "
	         << aiv[i].flag_count << " != "
		 << aip[i].flag_count << endl;
	    ok = false;
	}
        if (    aiv[i].reverse_attr_count
	     != aip[i].reverse_attr_count )
	{
	    cout << i << ": "
	         << aiv[i].name
	         << ": BAD REVERSE ATTR COUNT: "
	         << aiv[i].reverse_attr_count << " != "
		 << aip[i].reverse_attr_count << endl;
	    ok = false;
	}
        if ( aiv[i].value != aip[i].value )
	{
	    cout << i << ": "
	         << aiv[i].name
	         << ": BAD VALUE: "
	         << aiv[i].value << " != "
		 << aip[i].value << endl;
	    ok = false;
	}
        if ( aiv[i].flags != aip[i].flags )
	{
	    cout << i << ": "
	         << aiv[i].name
	         << ": BAD FLAGS: " << hex
	         << aiv[i].flags << " != "
		 << aip[i].flags
		 << dec << endl;
	    ok = false;
	}
    }
    if ( m > n )
        cout << "AN EXTRA ATTRIBUTE MAY BE "
	     << aiv[n].name << endl;
    else if ( m < n )
        cout << "A MISSING ATTRIBUTE MAY BE "
	     << aip[m].name << endl;

    min::assert_print = save_min_assert_print;
    return ok;
}

// Compare function to qsort vector of min::gen values.
//
static int compare_gen
	( const void * vp1, const void * vp2 )
{
    return min::compare ( * (min::gen *) vp1,
                          * (min::gen *) vp2 );
}

// Use get ( ap ) to get a vector of values and compare
// this to p[0 .. n-1].  Print differences.  Return
// true if no differences, and false if there are
// differences.
//
static bool check_values
        ( min::attr_insptr & ap,
	  min::gen * p, unsigned n )
{
    bool save_min_assert_print = min::assert_print;
    min::assert_print = false;
    bool ok = true;

    min::gen values[n], sortedp[n];
    min::unsptr m = min::get ( values, n, ap );

    if ( m != n )
    {
        cout << "BAD NUMBER OF VALUES: "
	     << m << " != " << n << endl;
	ok = false;
    }
    else
    {
        qsort ( values, n, sizeof ( min::gen ),
	        compare_gen );
	memcpy ( sortedp, p, n * sizeof ( min::gen ) );
        qsort ( sortedp, n, sizeof ( min::gen ),
	        compare_gen );
	for ( min::unsptr i = 0; i < n; ++ i )
	{
	    if ( values[i] != sortedp[i] )
	    {
	        cout << i << ": BAD VALUE: "
		    << values[i]
		    << " != "
		    << sortedp[i]
		    << endl;
		ok = false;
		break;
	    }
	}
    }

    min::assert_print = save_min_assert_print;
    return ok;
}

// Set, add, and remove from the value set of the
// attribute with label1.  Exit with 6 values for
// the attribute.  Switch temporarily to the
// attribute with label2 occassionally, but do not
// change that attribute.
//
void test_attribute_values
	( min::attr_insptr & ap,
	  min::gen label1, min::gen label2 )
{
    min::assert_print = false;

    cout << "TEST ATTRIBUTE VALUES ( "
         << label1 << ", "
         << label2 << ")" << endl;

    min::gen val1 = min::new_num_gen ( 1 );
    min::gen val2 = min::new_num_gen ( 2 );

    min::gen val5 = min::new_str_gen ( "value5" );
    min::gen val6 = min::new_str_gen ( "value6" );

    min::gen values1[8] = { val1, val1, val2, val2,
                            val5, val6, val6, val6 };
    min::locate ( ap, label1 );
    min::set ( ap, values1, 3 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 3 ) );
    min::add_to_multiset ( ap, values1 + 3, 3 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 6 ) );
    min::add_to_set ( ap, values1 + 4, 2 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 6 ) );
    min::add_to_multiset ( ap, values1 + 6, 2 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 8 ) );
    min::add_to_set ( ap, values1, 8 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 8 ) );
    cout << "REMOVED "
         << min::remove_one ( ap, values1+7, 1 )
	 << endl;
    min::locate ( ap, label2 );
    min::locate ( ap, label1 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 7 ) );
    cout << "REMOVED "
         << min::remove_one ( ap, values1, 1 )
	 << endl;
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1+1, 6 ) );
    cout << "REMOVED "
         << min::remove_all ( ap, values1+7, 1 )
	 << endl;
    min::locate ( ap, label2 );
    min::locate ( ap, label1 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1+1, 4 ) );
    cout << "REMOVED "
         << min::remove_all ( ap, val2 )
	 << endl;
    min::gen values2[6] = { val1, val1,
                            val5, val6, val6, val6 };
    PRINTING_MIN_CHECK
        ( check_values ( ap, values2+1, 2 ) );
    min::add_to_set ( ap, values2+1, 3 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values2+1, 3 ) );
    min::add_to_multiset ( ap, values2+3, 2 );
    min::add_to_multiset ( ap, val1 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values2, 6 ) );

    min::assert_print = true;
}

// Use get_flags ( ap ) to get a vector of control codes
// and compare this to p[0 .. n-1].  Print differences.
// Return true if no differences, and false if there are
// differences.
//
static bool check_flags
        ( min::attr_insptr & ap,
	  min::gen * p, unsigned n )
{
    bool save_min_assert_print = min::assert_print;
    min::assert_print = false;
    bool ok = true;

    min::gen flags[n];
    unsigned m = min::get_flags ( flags, n, ap );

    if ( m != n )
    {
        cout << "BAD NUMBER OF FLAG CONTROL CODES: "
	     << m << " != " << n << endl;
	ok = false;
    }
    else
    {
	for ( unsigned i = 0; i < n; ++ i )
	for ( unsigned j = 0; j < min::VSIZE; ++ j )
	{
	    bool flag =
		( (      min::control_code_of
		             ( flags[i] )
		       & ( (min::unsgen) 1 << j ) )
		    != 0 );
	    bool pflag =
		( (      min::control_code_of ( p[i] )
		       & ( (min::unsgen) 1 << j ) )
		    != 0 );
	    if ( flag == pflag ) continue;
	    cout << "(" << i << "," << j
		 << "): BAD FLAG: " << flag
		 << " != " << pflag << endl;
	    ok = false;
	}
    }

    min::assert_print = save_min_assert_print;
    return ok;
}

// Set, clear, flip, and test flags for the attributes
// with label1 and label2.  Exit with 4 control non-zero
// flag control words for label1 and 0 for label 2.
// The low order flags for label1 on exit are
//
static min::uns64 test_flags =
	( 1 << 0 ) + ( 1ull << (   min::VSIZE + 3 ) )
#		if MIN_IS_COMPACT
		// This avoids warning about shift
		// being too long.
		   + ( 1ull << ( 2*min::VSIZE + 5 ) )
#		endif
		;
//	
// Switch temporarily to the attribute with label3
// occassionally, but do not change that attribute.
//
void test_attribute_flags
	( min::attr_insptr & ap,
	  min::gen label1, min::gen label2,
	  min::gen label3 )
{
    min::assert_print = false;

    cout << "TEST ATTRIBUTE FLAGS ( "
         << label1 << ", "
         << label2 << ", "
         << label3 << ")" << endl;

    min::gen cc0 = min::new_control_code_gen ( 1 << 0 );
    min::gen cc1 = min::new_control_code_gen ( 1 << 1 );
    min::gen cc3 = min::new_control_code_gen ( 1 << 3 );
    min::gen cc5 = min::new_control_code_gen ( 1 << 5 );
    min::gen cc10 =
        min::new_control_code_gen ( 1 << 10 );

    min::gen codes1[5] = { cc0, cc3, cc5, cc10, cc1 };

    min::locate ( ap, label1 );

    min::set_flags ( ap, codes1, 2 );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 2 ) );
    min::set_flags ( ap, codes1, 0 );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 0 ) );
    min::set_some_flags ( ap, codes1, 3 );
    locate ( ap, label3 );
    locate ( ap, label1 );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 3 ) );
    min::clear_some_flags ( ap, codes1, 4 );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 0 ) );
    min::flip_some_flags ( ap, codes1, 4 );
    locate ( ap, label3 );
    locate ( ap, label1 );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 4 ) );
    min::flip_some_flags ( ap, codes1, 4 );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 0 ) );

    bool flag[5][min::VSIZE];
    flag[0][0] = min::set_flag ( ap, 0 );
    flag[1][3] = min::set_flag ( ap, min::VSIZE + 3 );
    flag[2][5] = min::set_flag ( ap, 2*min::VSIZE + 5 );
    flag[3][10] =
        min::set_flag ( ap, 3*min::VSIZE + 10 );
    flag[4][1] =
        min::set_flag ( ap, 4*min::VSIZE + 1 );
    PRINTING_MIN_CHECK
        ( ! flag[0][0] && ! flag[4][1] );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 5 ) );
    flag[0][0] = test_flag ( ap, 0 );
    flag[0][3] = test_flag ( ap, 1 );
    flag[1][2] = test_flag ( ap, min::VSIZE + 2 );
    flag[1][3] = test_flag ( ap, min::VSIZE + 3 );
    flag[1][4] = test_flag ( ap, min::VSIZE + 4 );
    locate ( ap, label3 );
    locate ( ap, label1 );
    flag[4][0] = test_flag ( ap, 4*min::VSIZE + 0 );
    flag[4][1] = test_flag ( ap, 4*min::VSIZE + 1 );
    flag[1][2] = test_flag ( ap, 4*min::VSIZE + 2 );
    PRINTING_MIN_CHECK ( flag[0][0] );
    PRINTING_MIN_CHECK ( ! flag[0][3] );
    PRINTING_MIN_CHECK ( ! flag[1][2] );
    PRINTING_MIN_CHECK ( flag[1][3] );
    PRINTING_MIN_CHECK ( ! flag[1][4] );
    PRINTING_MIN_CHECK ( ! flag[4][0] );
    PRINTING_MIN_CHECK ( flag[4][1] );
    PRINTING_MIN_CHECK ( ! flag[1][2]);
    flag[4][1] =
        min::set_flag ( ap, 4*min::VSIZE + 1 );
    PRINTING_MIN_CHECK ( flag[4][1] );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 5 ) );
    flag[4][1] =
        min::flip_flag ( ap, 4*min::VSIZE + 1 );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 4 ) );
    PRINTING_MIN_CHECK ( flag[4][1] );

    locate ( ap, label2 );
    flag[0][0] = min::flip_flag ( ap, 0 );
    flag[1][3] = min::flip_flag ( ap, min::VSIZE + 3 );
    PRINTING_MIN_CHECK
        ( ! flag[0][0] && ! flag[1][3] );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 2 ) );
    flag[0][0] = min::clear_flag ( ap, min::VSIZE + 3 );
    flag[4][0] = min::clear_flag ( ap, 4*min::VSIZE );
    PRINTING_MIN_CHECK
        ( flag[0][0] && ! flag[4][0] );
    locate ( ap, label3 );
    locate ( ap, label2 );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 1 ) );
    flag[0][0] = min::flip_flag ( ap, 0 );
    PRINTING_MIN_CHECK ( flag[0][0] );
    PRINTING_MIN_CHECK
        ( check_flags ( ap, codes1, 0 ) );

    min::assert_print = true;
}

// Set, add, and remove reverse attribute values with
// attribute name label1 and reverse attribute name
// rlabel1.  Exit with 3 reverse attribute values for
// these.  Temporary switch to attribute name label2,
// but do not change that attribute.  Use obj1, obj2,
// and obj3 as values and leave each with one value
// with attribute name rlabel1 and reverse attribute
// name label1.
//
void test_reverse_attribute_values
	( min::attr_insptr & ap,
	  min::gen label1, min::gen rlabel1,
	  min::gen label2,
	  min::gen obj1, min::gen obj2, min::gen obj3 )
{
    min::assert_print = false;

    cout << "TEST REVERSE ATTRIBUTE VALUES ( "
         << label1 << ", "
         << rlabel1 << ", "
         << label2 << ")" << endl;

    min::gen values1[8] =
        { obj1, obj2, obj3,
	  obj3, obj2, obj1,
	  obj1, obj2 };
    min::locate ( ap, label1 );
    min::locate_reverse ( ap, rlabel1 );
    min::set ( ap, values1, 3 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 3 ) );
    min::add_to_multiset ( ap, values1, 3 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 6 ) );
    min::add_to_set ( ap, values1, 2 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 6 ) );
    min::add_to_multiset ( ap, values1, 2 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 8 ) );
    // min::locate ( ap, label1 );
    // min::locate_reverse ( ap, rlabel1 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 8 ) );
    min::add_to_set ( ap, values1, 8 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 8 ) );
    cout << "REMOVED "
         << min::remove_one ( ap, values1+7, 1 )
	 << endl;
    min::locate ( ap, label2 );
    min::locate ( ap, label1 );
    min::locate_reverse ( ap, rlabel1 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 7 ) );
    cout << "REMOVED "
         << min::remove_one ( ap, values1, 1 )
	 << endl;
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1+1, 6 ) );
    cout << "REMOVED "
         << min::remove_all ( ap, values1, 1 )
	 << endl;
    // min::locate ( ap, label2 );
    // min::locate ( ap, label1 );
    // min::locate_reverse ( ap, rlabel1 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1+1, 4 ) );
    cout << "REMOVED "
         << min::remove_all ( ap, obj2 )
	 << endl;
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1+2, 2 ) );
    min::add_to_set ( ap, values1, 2 );
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 4 ) );
    cout << "REMOVED "
         << min::remove_one ( ap, obj3 )
	 << endl;
    PRINTING_MIN_CHECK
        ( check_values ( ap, values1, 3 ) );

    min::reverse_attr_info rinfo;
    MIN_CHECK
        (    min::get_reverse_attrs ( & rinfo, 1, ap )
	  == 1 );
    MIN_CHECK ( rinfo.name == rlabel1 );
    MIN_CHECK ( rinfo.value_count == 3 );
    MIN_CHECK ( rinfo.value == min::MULTI_VALUED() );

    min::obj_vec_ptr vp1 ( obj1 );
    min::attr_ptr ap1 ( vp1 );
    min::locate ( ap1, rlabel1 );
    min::gen obj = min::new_stub_gen
        ( (const min::stub * )
	  min::obj_vec_ptr_of ( ap ) );

    MIN_CHECK
        (    min::get_reverse_attrs ( & rinfo, 1, ap1 )
	  == 1 );
    MIN_CHECK ( rinfo.name == label1 );
    MIN_CHECK ( rinfo.value_count == 1 );
    MIN_CHECK ( rinfo.value == obj  );

    min::assert_print = true;
}

void test_object_attribute_level ( void )
{
    cout << endl;
    cout << "Start Object Attribute Level Test!"
	 << endl;

    min::gen obj_gen = min::new_obj_gen ( 500, 100 );
    min::obj_vec_insptr vp ( obj_gen );
    min::attr_insptr ap ( vp );

    min::gen lab1 = min::new_str_gen ( "label1" );
    min::gen lab2 = min::new_str_gen ( "label2" );
    min::gen lab3 = min::new_str_gen ( "label3" );
    min::gen lab4 = min::new_str_gen ( "label4" );

    min::gen int1 = min::new_num_gen ( 1 );
    min::gen int2 = min::new_num_gen ( 2 );
    min::gen int3 = min::new_num_gen ( 3 );
    min::gen int4 = min::new_num_gen ( 4 );

    min::gen int1lab1v[2] = { int1, lab1 };
    min::gen int1lab2v[2] = { int1, lab2 };
    min::gen lab1int1v[2] = { lab1, int1 };
    min::gen lab1int2v[2] = { lab1, int2 };
    min::gen int1lab1 =
        min::new_lab_gen ( int1lab1v, 2 );
    min::gen int1lab2 =
        min::new_lab_gen ( int1lab2v, 2 );
    min::gen lab1int1 =
        min::new_lab_gen ( lab1int1v, 2 );
    min::gen lab1int2 =
        min::new_lab_gen ( lab1int2v, 2 );

    min::locate ( ap, lab1 );
    min::set ( ap, int1 );
    MIN_CHECK ( min::get ( ap ) == int1 );

    min::locate ( ap, lab2 );
    min::set ( ap, int2 );
    min::locate ( ap, lab3 );
    min::set ( ap, int3 );
    min::locate ( ap, lab4 );
    min::set ( ap, int4 );

    min::locate ( ap, lab1 );
    MIN_CHECK ( min::get ( ap ) == int1 );
    min::locate ( ap, lab2 );
    MIN_CHECK ( min::get ( ap ) == int2 );
    min::locate ( ap, lab3 );
    MIN_CHECK ( min::get ( ap ) == int3 );
    min::locate ( ap, lab4 );
    MIN_CHECK ( min::get ( ap ) == int4 );

    min::assert_print = false;
    for ( unsigned i = 0; i < 50; ++ i )
        min::attr_push(vp) = min::EMPTY_SUBLIST();
    min::assert_print = true;
    MIN_CHECK ( min::attr_size_of ( vp ) == 50 );
    MIN_CHECK
        (    min::attr ( vp, 21 )
	  == min::EMPTY_SUBLIST() );

    min::locatei ( ap, 1 );
    MIN_CHECK ( min::get ( ap ) == min::NONE() );
    min::set ( ap, lab1 );
    MIN_CHECK ( min::get ( ap ) == lab1 );
    min::locatei ( ap, 2 );
    min::set ( ap, lab2 );
    min::locatei ( ap, 3 );
    min::set ( ap, lab3 );
    min::locatei ( ap, 4 );
    min::set ( ap, lab4 );

    min::locate ( ap, int1 );
    MIN_CHECK ( min::get ( ap ) == lab1 );
    min::locate ( ap, int2 );
    MIN_CHECK ( min::get ( ap ) == lab2 );
    min::locate ( ap, int3 );
    MIN_CHECK ( min::get ( ap ) == lab3 );
    min::locate ( ap, int4 );
    MIN_CHECK ( min::get ( ap ) == lab4 );

    min::gen MULTI = min::MULTI_VALUED();
    min::attr_info ai[12] = {
        { int1, lab1, 0, 1, 0, 0 },
        { int2, lab2, 0, 1, 0, 0 },
        { int3, lab3, 0, 1, 0, 0 },
        { int4, lab4, 0, 1, 0, 0 },
        { lab1, int1, 0, 1, 0, 0 },
        { lab2, int2, 0, 1, 0, 0 },
        { lab3, int3, 0, 1, 0, 0 },
        { lab4, int4, 0, 1, 0, 0 },
	{ int1lab1, MULTI, 0, 0, 0, 0 },
	{ int1lab2, MULTI, 0, 0, 0, 0 },
	{ lab1int1, MULTI, 0, 0, 0, 0 },
	{ lab1int2, MULTI, 0, 0, 0, 0 } };

    MIN_CHECK ( check_attr_info ( ap, ai, 8 ) );

    test_attribute_values ( ap, lab1, lab2 );
    ai[4].value_count = 6;
    ai[4].value = MULTI;
    MIN_CHECK ( check_attr_info ( ap, ai, 8 ) );

    test_attribute_values ( ap, int3, lab1 );
    ai[2].value_count = 6;
    ai[2].value = MULTI;
    MIN_CHECK ( check_attr_info ( ap, ai, 8 ) );

    test_attribute_values ( ap, int1lab1, int1lab2 );
    ai[8].value_count = 6;
    ai[8].value = MULTI;
    MIN_CHECK ( check_attr_info ( ap, ai, 9 ) );
    test_attribute_values ( ap, int1lab2, int1lab1 );
    ai[9].value_count = 6;
    ai[9].value = MULTI;
    MIN_CHECK ( check_attr_info ( ap, ai, 10 ) );
    test_attribute_values ( ap, lab1int1, lab1int2 );
    ai[10].value_count = 6;
    ai[10].value = MULTI;
    MIN_CHECK ( check_attr_info ( ap, ai, 11 ) );
    test_attribute_values ( ap, lab1int2, lab1int1 );
    ai[11].value_count = 6;
    ai[11].value = MULTI;
    MIN_CHECK ( check_attr_info ( ap, ai, 12 ) );

    test_attribute_flags ( ap, lab1, lab2, lab3 );
    ai[4].flag_count = 4;
    ai[4].flags = ::test_flags;
    test_attribute_flags ( ap, lab2, int1, int2 );
    ai[5].flag_count = 4;
    ai[5].flags = ::test_flags;
    MIN_CHECK ( check_attr_info ( ap, ai, 12 ) );

    min::gen obj1 = min::new_obj_gen ( 40, 10 );
    min::gen obj2 = min::new_obj_gen ( 40, 10 );
    min::gen obj3 = min::new_obj_gen ( 40, 10 );

    test_reverse_attribute_values
	( ap, lab1, lab2, int1, obj1, obj2, obj3 );
    ai[4].reverse_attr_count = 1;
    MIN_CHECK ( check_attr_info ( ap, ai, 12 ) );

    cout << endl;
    cout << "Finish Object Attribute Level Test!"
	 << endl;
}

// Object Printing
// ------ --------

void test_object_printing ( void )
{
    cout << endl;
    cout << "Start Object Printing Test!"
	 << endl << endl;
    min::assert_print = false;
    // min::assert_throw = false;
    // min::assert_abort = true;

    min::gen obj = min::new_obj_gen ( 5, 5 );

    printer << min::pgen ( obj ) << min::eol;

    {
	min::obj_vec_insptr vp ( obj );
	min::attr_push ( vp, 3 );
	vp[0] = min::new_num_gen ( 1 );
	vp[1] = min::new_num_gen ( 2 );
	vp[2] = min::new_num_gen ( 3 );
    }
    printer << min::pgen ( obj ) << min::eol;

    {
	min::obj_vec_insptr vp ( obj );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_initiator );
	min::set ( ap, min::new_lab_gen ( "{", "*" ) );
	min::locate ( ap, min::dot_separator );
	min::set ( ap, min::new_str_gen ( "," ) );
	min::locate ( ap, min::dot_terminator );
	min::set ( ap, min::new_lab_gen ( "*", "}" ) );
	min::attr_push ( vp, 2 );
	vp[3] = min::new_num_gen ( 4 );
	vp[4] = min::new_num_gen ( 5 );
    }
    printer << min::pgen ( obj ) << min::eol;

    min::gen obj2 = min::new_obj_gen ( 5, 5 );
    {
	min::obj_vec_insptr vp ( obj2 );
	min::attr_push ( vp, 5 );
	vp[0] = obj;
	vp[1] = obj;
	vp[2] = obj;
	vp[3] = obj;
	vp[4] = obj;
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_initiator );
	min::set ( ap, min::new_str_gen ( "[" ) );
	min::locate ( ap, min::dot_separator );
	min::set ( ap, min::new_str_gen ( ";" ) );
	min::locate ( ap, min::dot_terminator );
	min::set ( ap, min::new_str_gen ( "]" ) );
    }
    printer << min::pgen ( obj2 ) << min::eol;

    min::gen obj3 = min::new_obj_gen ( 5, 5 );
    {
	min::obj_vec_insptr vp ( obj3 );
	min::attr_push ( vp, 3 );
	vp[0] = min::new_str_gen ( "A" );
	vp[1] = min::new_str_gen ( "B" );
	vp[2] = min::new_str_gen ( "C" );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_str_gen ( "T" ) );
    }
    printer << min::pgen ( obj3 ) << min::eol;
    min::print_obj ( printer, obj3,
                       min::ENABLE_COMPACT
		     + min::NO_TRAILING_TYPE );
    min::print_obj ( printer, obj3,
                     min::embedded_line_obj_format );
    min::print_obj ( printer, obj3,
                     min::embedded_line_obj_format,
                       min::EMBEDDED_LINE
		     + min::NO_TRAILING_TYPE );

    {
	min::obj_vec_insptr vp ( obj3 );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_separator );
	min::set ( ap, min::new_str_gen ( ";" ) );
    }
    printer << min::pgen ( obj3 ) << min::eol;
    min::print_obj ( printer, obj3,
		     min::embedded_line_obj_format );
    min::print_obj ( printer, obj3,
		     min::isolated_line_obj_format );

    {
	min::obj_vec_insptr vp ( obj3 );
	min::attr_insptr ap  ( vp );
	min::locatable_gen number =
	    min::new_str_gen ( "number" );
	min::locate ( ap, number );
	min::set ( ap, min::new_num_gen ( 123 ) );
    }
    printer << min::pgen ( obj3 ) << min::eol;
    min::print_obj ( printer, obj3,
                     min::embedded_line_obj_format );

    {
	min::obj_vec_insptr vp ( obj3 );
	min::attr_insptr ap  ( vp );
	min::locatable_gen fraction =
	    min::new_str_gen ( "fraction" );
	min::locate ( ap, fraction );
	min::set ( ap, min::new_str_gen ( "TRUE" ) );
	min::locatable_gen scientific =
	    min::new_str_gen ( "scientific" );
	min::locate ( ap, scientific );
	min::set ( ap, min::new_str_gen ( "FALSE" ) );
    }
    printer << min::pgen ( obj3 ) << min::eol;

    {
	min::obj_vec_insptr vp ( obj );
	min::attr_insptr ap  ( vp );
	min::locatable_gen myattr =
	    min::new_str_gen ( "myattr" );
	min::locate ( ap, myattr );
	min::set ( ap, min::new_lab_gen
	                   ( "MYATTR", "VALUE" ) );
    }
    printer << min::pgen ( obj ) << min::eol;
    min::print_id ( printer, obj );
    printer << min::eol << min::flush_one_id;

    {
	min::obj_vec_insptr vp ( obj );
	min::attr_insptr ap  ( vp );
	min::locatable_gen myattr =
	    min::new_str_gen ( "myattr" );
	min::locate ( ap, myattr );
	min::add_to_set
	    ( ap, min::new_lab_gen
		      ( "MYATTR", "SECOND", "VALUE" ) );
	min::add_to_set
	    ( ap, min::new_lab_gen
		      ( "MYATTR", "THIRD", "VALUE" ) );
    }
    printer << min::pgen ( obj ) << min::eol;

    {
	min::obj_vec_insptr vp ( obj3 );
	min::attr_insptr ap  ( vp );
	min::locatable_gen myforward =
	    min::new_str_gen ( "myforward" );
	min::locatable_gen myreverse =
	    min::new_str_gen ( "myreverse" );
	min::locate ( ap, myforward );
	min::locate_reverse ( ap, myreverse );
	min::set ( ap, obj );
    }
    printer << min::pgen ( obj3 ) << min::eol;
    printer << min::pgen ( obj ) << min::eol;
    printer << min::eol << min::flush_one_id;

    {
	min::obj_vec_insptr vp ( obj3 );
	min::attr_insptr ap  ( vp );
	min::locatable_gen myforward =
	    min::new_str_gen ( "myforward" );
	min::locate ( ap, myforward );
	min::set_flag ( ap, 6 );
	min::set_flag ( ap, 12 );
	min::set_flag ( ap, 63 );
    }
    printer << min::pgen ( obj3 ) << min::eol;

    {
	min::obj_vec_insptr vp ( obj );
	min::attr_insptr ap  ( vp );
	min::locatable_gen myattr =
	    min::new_str_gen ( "myattr" );
	min::locate ( ap, myattr );
	min::set_flag ( ap, 6 );
	min::set_flag ( ap, 63 );
	min::set_flag ( ap, 64 );
	min::set_flag ( ap, 512 );
    }
    printer << min::pgen ( obj ) << min::eol;
    min::print_obj ( printer, obj,
                     min::embedded_line_obj_format );
    min::print_obj ( printer, obj,
                     min::isolated_line_obj_format );

    {
	min::obj_vec_insptr vp ( obj );
	min::attr_insptr ap  ( vp );
	min::locatable_gen myattr =
	    min::new_str_gen ( "myattr" );
	min::locate ( ap, myattr );
	min::set_flag
	    ( ap,
	        min::standard_attr_A_flag
	      + ( 'H' - 'A' ) );
    }
    printer << min::pgen ( obj ) << min::eol;
    min::print_obj ( printer, obj,
                     min::embedded_line_obj_format );
    min::print_obj ( printer, obj,
                     min::isolated_line_obj_format );

    min::gen obj4 = min::new_obj_gen ( 5, 5 );
    {
	min::obj_vec_insptr vp ( obj4 );
	min::attr_push ( vp, 3 );
	vp[0] = min::new_str_gen ( "X" );
	vp[1] = min::new_str_gen ( "Y" );
	vp[2] = min::new_str_gen ( "Z" );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_str_gen ( "T" ) );
    }
    printer << min::pgen ( obj4 ) << min::eol;
    {
	min::obj_vec_insptr vp ( obj4 );
	// min::attr_push ( vp, 3 );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_str_gen ( "+" ) );
    }
    printer << min::pgen ( obj4 ) << min::eol;
    {
	min::obj_vec_insptr vp ( obj4 );
	// min::attr_push ( vp, 3 );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_lab_gen ( "<", ">" ) );
    }
    printer << min::pgen ( obj4 ) << min::eol;
    {
	min::obj_vec_insptr vp ( obj4 );
	min::attr_push ( vp, 3 );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_lab_gen ( "{", "}" ) );
    }
    printer << min::pgen ( obj4 ) << min::eol;
    {
	min::obj_vec_insptr vp ( obj4 );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::doublequote );
    }
    printer << min::pgen ( obj4 ) << min::eol;

    min::gen line1 = min::new_obj_gen ( 10, 10 );
    {
	min::obj_vec_insptr vp ( line1 );
	min::attr_push ( vp, 6 );
	vp[0] = min::new_str_gen ( "This" );
	vp[1] = min::new_str_gen ( "is" );
	vp[2] = min::new_str_gen ( "the" );
	vp[3] = min::new_str_gen ( "first" );
	vp[4] = min::new_str_gen ( "line" );
	vp[5] = min::new_str_gen ( "." );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_str_gen ( "\n" ) );
    }
    min::gen line2 = min::new_obj_gen ( 10, 10 );
    {
	min::obj_vec_insptr vp ( line2 );
	min::attr_push ( vp, 5 );
	vp[0] = min::new_str_gen ( "This" );
	vp[1] = min::new_str_gen ( "is" );
	vp[2] = min::new_str_gen ( "the" );
	vp[3] = min::new_str_gen ( "second" );
	vp[4] = min::new_str_gen ( "line" );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_str_gen ( ";" ) );
    }
    min::gen line3 = min::new_obj_gen ( 10, 10 );
    {
	min::obj_vec_insptr vp ( line3 );
	min::attr_push ( vp, 5 );
	vp[0] = min::new_str_gen ( "This" );
	vp[1] = min::new_str_gen ( "is" );
	vp[2] = min::new_str_gen ( "the" );
	vp[3] = min::new_str_gen ( "third" );
	vp[4] = min::new_str_gen ( "line" );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_str_gen ( "\n" ) );
	min::locate ( ap, min::dot_terminator );
	min::set ( ap, min::new_str_gen ( "!" ) );
    }
    min::gen line4 = min::new_obj_gen ( 10, 10 );
    {
	min::obj_vec_insptr vp ( line4 );
	min::attr_push ( vp, 6 );
	vp[0] = min::new_str_gen ( "This" );
	vp[1] = min::new_str_gen ( "is" );
	vp[2] = min::new_str_gen ( "the" );
	vp[3] = min::new_str_gen ( "fourth" );
	vp[4] = min::new_str_gen ( "line" );
	vp[5] = min::new_str_gen ( "." );
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_str_gen ( "\n" ) );
    }
    min::gen paragraph = min::new_obj_gen ( 10, 10 );
    {
	min::obj_vec_insptr vp ( paragraph );
	min::attr_push ( vp, 4 );
	vp[0] = line1;
	vp[1] = line2;
	vp[2] = line3;
	vp[3] = line4;
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_str_gen ( ":" ) );
    }
    min::gen top_line = min::new_obj_gen ( 10, 10 );
    {
	min::obj_vec_insptr vp ( top_line );
	min::attr_push ( vp, 6 );
	vp[0] = min::new_str_gen ( "This" );
	vp[1] = min::new_str_gen ( "is" );
	vp[2] = min::new_str_gen ( "the" );
	vp[3] = min::new_str_gen ( "top" );
	vp[4] = min::new_str_gen ( "line" );
	vp[5] = paragraph;
	min::attr_insptr ap  ( vp );
	min::locate ( ap, min::dot_type );
	min::set ( ap, min::new_str_gen ( "\n" ) );
    }
    printer << min::bom << min::place_indent ( 4 )
            << min::pgen ( top_line,
	                   min::line_gen_format )
	    << min::eom;

    min::assert_print = true;
    // min::assert_throw = true;
    // min::assert_abort = false;
    cout << endl;
    cout << "Finish Object Printing Test!" << endl;
}


// Main Program
// ---- -------

int main ( int argc, const char * argv[] )
{
    min::assert_print = true;
    min::assert_throw = true;

    debug = ( argc > 1 );
    cout << endl;
    cout << "Initialize!" << endl;
    MIN_REQUIRE ( ::interrupt_count == 0 );
    MIN_REQUIRE ( ! MINT::initialization_done );
    min::interrupt();
    MIN_REQUIRE ( ::interrupt_count == 1 );
    MIN_REQUIRE ( MINT::initialization_done );
    cout << endl;
    cout << "Start Test!" << endl;

    try {

	test_number_types();
	test_general_value_data();
	test_stub_data();
	test_ptr_conversions();
	test_general_value_functions();
	test_control_values();
	test_stub_functions();
	test_process_interface();
	test_acc_interface();
	test_numbers();
	test_strings();
	test_labels();
	test_names();
	test_packed_structs();
	test_packed_vectors();
	test_file();
	test_identifier_map();
	test_unicode_name_table();
	test_printer();
	test_objects();
	test_object_vector_level();
	test_object_list_level();
	test_object_attribute_level();
	test_object_printing();

	// Check that deallocated_body_region is still
	// zero.
	//
	MIN_CHECK
	    ( deallocated_body_region[0] == 0
	      &&
	      memcmp ( deallocated_body_region,
		       deallocated_body_region + 1,
		         sizeof
		           ( deallocated_body_region )
		       - 1 )
	      == 0 );

    } catch ( min::assert_exception * x ) {
        cout << "EXITING BECAUSE OF FAILED MIN_CHECK"
	     << endl;
	exit ( 1 );
    }

    cout << endl;
    cout << "Finished Test!" << endl;
}
