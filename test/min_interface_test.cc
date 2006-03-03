// MIN Language Interface Test Program
//
// File:	min_interface_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Fri Mar  3 05:18:50 EST 2006
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2006/03/03 10:47:10 $
//   $RCSfile: min_interface_test.cc,v $
//   $Revision: 1.45 $

// Table of Contents:
//
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
//	Garbage Collector Interface
//	Numbers
//	Strings
//	Labels
//	Atom Functions
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

// Artificial increase in pointer size for stretch test.
//
// If MIN_STRETCH defines, set
//
//	MIN_MAXIMUM_ABSOLUTE_STUB_ADDRESS = 2**60 - 1
//	MIN_MAXIMUM_RELATIVE_STUB_ADDRESS =
//		2**36 - 2**33 - 1  if MIN_IS_COMPACT
//		2**48 - 1  if not MIN_IS_COMPACT
//
# ifdef MIN_STRETCH
#   define MIN_MAXIMUM_ABSOLUTE_STUB_ADDRESS \
 	       0xFFFFFFFFFFFFFFF
#   if MIN_IS_COMPACT
#	define MIN_MAXIMUM_RELATIVE_STUB_ADDRESS \
 	       0xDFFFFFFFF
#   else
#	define MIN_MAXIMUM_RELATIVE_STUB_ADDRESS \
 	       0xFFFFFFFFFFFF
#   endif
# endif

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
    		 __FILE__, __LINE__, #expr );

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
bool MUP::interrupt ( void )
{
    interrupt_called = true;
    return true;
}

// Garbage Collector Interface Functions.

// Place to allocate stubs.  Stubs must be allocated on
// a `sizeof (min::stub)' boundary.
//
char stub_region[10000];

// Number of stubs allocated to stub_region so far,
// address of the first stub in the region, and the
// address of the first location beyond the last
// possible stub in the region.
//
unsigned region_stubs_allocated = 0;
min::stub * begin_stub_region;
min::stub * end_stub_region;

// Initialize stub_region, MUP::last_allocated_stub (to
// a dummy), and number_free_stubs (to zero).
//
void initialize_stub_region ( void )
{
    MINT::pointer_uns stp =
	(MINT::pointer_uns) stub_region;
    // Check that sizeof min::stub is a power of 2.
    assert ( (   sizeof (min::stub) - 1
	       & sizeof (min::stub) )
	     == 0 );
    MINT::pointer_uns p = stp;
    p += sizeof (min::stub) - 1;
    p &= ~ (sizeof (min::stub) - 1 ) ;
    begin_stub_region = (min::stub *) p;
    stp += sizeof stub_region;
    unsigned n = ( stp - p ) / ( sizeof (min::stub) );
    p += ( sizeof (min::stub) ) * n;
    end_stub_region = (min::stub *) p;
    assert ( begin_stub_region < end_stub_region );

    MUP::last_allocated_stub = begin_stub_region;
    MUP::number_of_free_stubs = 0;
    ++ region_stubs_allocated;
    min::uns64 c = MUP::new_gc_control
	( min::FREE, (min::stub *) NULL );
    MUP::set_control_of
	( MUP::last_allocated_stub, c );
    MUP::set_value_of
	( MUP::last_allocated_stub, 0 );
}

// Function to allocate n - number_of_free_stubs more
// stubs to the stub_region and attach them to the
// free list just after the last_allocated_stub.
//
void MUP::gc_expand_stub_free_list ( unsigned n )
{
    cout << "MUP::gc_expand_stub_free_list (" << n
         << ") called" << endl;
    if ( n <= MUP::number_of_free_stubs ) return;
    n -= MUP::number_of_free_stubs;

    min::uns64 lastc = MUP::control_of
	( MUP::last_allocated_stub );
    min::stub * free =
	MUP::stub_of_gc_control ( lastc );
    while ( n -- > 0 )
    {
        min::stub * s = begin_stub_region
	              + region_stubs_allocated;
	assert ( s < end_stub_region );
	++ region_stubs_allocated;
	++ MUP::number_of_free_stubs;
	min::uns64 c = MUP::new_gc_control
	    ( min::FREE, free );
	MUP::set_control_of ( s, c );
	MUP::set_value_of ( s, 0 );
	free = s;
    }
    lastc = MUP::renew_gc_control_stub ( lastc, free );
    MUP::set_control_of
	( MUP::last_allocated_stub, lastc );
}

// Place to allocate bodies.  Bodes must be allocated on
// 8 byte boundaries.
//
char body_region[10000];

// Address of the first body control block in the
// region, and the address of the first location beyond
// the last possible body control block in the region.
//
MUP::body_control * begin_body_region;
MUP::body_control * end_body_region;

// Initialize body_region and MUP::free_body_control.
// The initial free body is of zero length.
//
void initialize_body_region ( void )
{
    MINT::pointer_uns stp =
	(MINT::pointer_uns) body_region;
    MINT::pointer_uns p = stp;
    p += 7;
    p &= ~ 7;
    begin_body_region = (MUP::body_control *) p;
    stp += sizeof body_region;
    unsigned n = ( stp - p ) / 8;
    p += 8 * n;
    end_body_region = (MUP::body_control *) p;
    assert ( begin_body_region + 2 <= end_body_region );

    begin_body_region->control =
        MUP::new_control ( min::FREE, (min::uns64) 0);
    begin_body_region->size_difference = 0;
    MUP::body_control * end = begin_body_region + 1;
    end->control =
        MUP::new_control ( min::FREE, (min::uns64) 0 );
    end->size_difference = 0;

    MUP::free_body_control = begin_body_region;
}

// Function to add m bytes to the free area following
// free_body_control.  M must be a multiple of 8.
//
void add_to_free_body ( unsigned m )
{
    cout << "add_to_free_body (" << m << ") called"
         << endl;
    MUP::body_control * free = MUP::free_body_control;
    MINT::pointer_uns size =
        MUP::value_of_control ( free->control );
    assert ( ( size & 7 ) == 0 );
    assert ( ( m & 7 ) == 0 );
    MINT::pointer_uns new_size = size + m;

    MINT::pointer_uns p =
	(MINT::pointer_uns) free;
    assert ( ( p & 7 ) == 0 );
    p += size + sizeof ( MUP::body_control );
    MUP::body_control * tail = (MUP::body_control *) p;

    // Tail points at body_control following old free
    // body.  We read it here, as otherwise it would
    // never be read.
    //
    assert (    tail->control
	     == MUP::new_control
	            ( min::FREE, (min::uns64) 0 ) );
    assert (    tail->size_difference
	     == - min::int64 ( size ) );
    p += m;
    tail = (MUP::body_control *) p;
    assert ( tail + 1 <= end_body_region );

    // Now tail points at body_control following new
    // free body, which we must write.

    free->size_difference += min::int64 ( m );
    free->control =
        MUP::new_control ( min::FREE, new_size );
    tail->control =
        MUP::new_control ( min::FREE, (min::uns64) 0 );
    tail->size_difference = - min::int64 ( new_size );
}

// Function to execute MUP::new_body when the free_
// body_control free body is too small.  n is the
// argument to MUP::new_body rounded up to a multiple
// of 8.
//
MUP::body_control * MUP::gc_new_body ( unsigned n )
{
    assert ( ( n & 7 ) == 0 );

    // Expand the free body to be large enough.

    MINT::pointer_uns size =
        MUP::value_of_control
	     ( MUP::free_body_control->control );
    MINT::pointer_uns new_size =
	n + sizeof ( body_control );
    assert ( ( size & 7 ) == 0 );
    assert ( size < new_size );
    add_to_free_body ( new_size - size );

    // Done expanding free area.  Now rerun new_stub.

    return MUP::new_body ( n );
}

// Function to relocate a body.  Just allocates a new
// body and copies the contents of the old body to the
// new body, zeros the old body, and returns the
// body control before the new body.  Body control is
// copied from old to new body, and body control of
// old body is set to free body.
//
// Length rounded up to multiple of 8 is length of the
// old and new bodies.
//
MUP::body_control * relocate_body
	( MUP::body_control * bc, unsigned length )
{
    length += 7;
    length &= ~ 7;
    MUP::body_control * newbc =
    	MUP::new_body ( length );
    newbc->control = bc->control;
    bc->control =
        MUP::new_control ( min::FREE, length );
    memcpy ( newbc+1, bc+1, length );
    memset ( bc+1, 0, length );
    return newbc;
}

// Function to initialize GC hash tables.
//
void initialize_hash_tables ( void )
{
    MUP::str_hash_size = 101;
    MUP::str_hash =
        new (min::stub *)[MUP::str_hash_size];
    for ( int i = 0; i < MUP::str_hash_size; ++ i )
        MUP::str_hash[i] = NULL;
    MUP::num_hash_size = 101;
    MUP::num_hash =
        new (min::stub *)[MUP::num_hash_size];
    for ( int i = 0; i < MUP::num_hash_size; ++ i )
        MUP::num_hash[i] = NULL;
    MUP::lab_hash_size = 101;
    MUP::lab_hash =
        new (min::stub *)[MUP::lab_hash_size];
    for ( int i = 0; i < MUP::lab_hash_size; ++ i )
        MUP::lab_hash[i] = NULL;
}

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
	min::stub * stub =
	    (min::stub *) (sizeof (min::stub));

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
	        MINT::general_stub_to_uns32 ( stub );
	    min::stub * s32 =
		MINT::general_uns32_to_stub ( u32 );
	    MIN_ASSERT ( s32 == stub );
#	elif MIN_IS_LOOSE
	    cout << endl;
	    cout << "Test general stub/uns64"
	            " conversions:" << endl;
	    u64 =
	        MINT::general_stub_to_uns64 ( stub );
	    u64 += min::uns64(min::GEN_STUB) << 44;
	    min::stub * s64 =
		MINT::general_uns64_to_stub ( u64 );
	    MIN_ASSERT ( s64 == stub );
	    min::uns64 g64 =
		MINT::general_stub_into_uns64
		   	( u64, stub );
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
	min::stub * stub =
	    (min::stub *) (sizeof (min::stub));

	cout << endl;
	cout << "Test stub general values:" << endl;
	cout << "stub: " << hex
	     << MINT::pointer_uns ( stub )
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
#	elif MIN_IS_LOOSE
	    char * str = "ABCDE";
	    char * overflowstr = "ABCDEF";
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
	    			( 1 << 24 );
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
	    			( 1 << 24 );
	);
	MIN_ASSERT
	    ( ! min::is_stub ( sublistauxgen ) );
	MIN_ASSERT
	    ( ! min::is_direct_str ( sublistauxgen ) );
 
        cout << endl;
	cout << "Test indirect pair aux general values:"
	     << endl;
	min::gen pairauxgen =
	    MUP::new_indirect_pair_aux_gen ( aux );
	cout << "pairauxgen: "
	     << print_gen ( pairauxgen ) << endl;
	MIN_ASSERT
	    ( min::is_indirect_pair_aux
	    		( pairauxgen ) );
	MIN_ASSERT
	    (    min::gen_subtype_of ( pairauxgen )
	      == min::GEN_INDIRECT_PAIR_AUX );
	MIN_ASSERT
	    (    min::indirect_pair_aux_of
	              ( pairauxgen )
	      == aux );
	desire_success (
	    pairauxgen =
	    	min::new_indirect_pair_aux_gen ( aux );
	);
	desire_failure (
	    pairauxgen = min::new_indirect_pair_aux_gen
	    			( 1 << 24 );
	);
	MIN_ASSERT
	    ( ! min::is_stub ( pairauxgen ) );
	MIN_ASSERT
	    ( ! min::is_direct_str ( pairauxgen ) );
 
        cout << endl;
	cout << "Test indirect indexed aux general"
	        " values:" << endl;
	unsigned indirect_aux = 637;
	unsigned indirect_index = 281;
	min::gen indirectgen =
	    MUP::new_indirect_indexed_aux_gen
		( indirect_aux, indirect_index );
	cout << "indirectgen: "
	     << print_gen ( indirectgen ) << endl;
	MIN_ASSERT
	    ( min::is_indirect_indexed_aux
		  ( indirectgen ) );
	MIN_ASSERT
	    (    min::gen_subtype_of ( indirectgen )
	      == min::GEN_INDIRECT_INDEXED_AUX );
	MIN_ASSERT
	    (    min::indirect_aux_of ( indirectgen )
	      == indirect_aux );
	MIN_ASSERT
	    (    min::indirect_index_of ( indirectgen )
	      == indirect_index );
	desire_success (
	    indirectgen =
	    	min::new_indirect_indexed_aux_gen
		    ( indirect_aux, indirect_index );
	);
	desire_failure (
	    indirectgen =
	        min::new_indirect_indexed_aux_gen
		    ( 1 << 20, indirect_index );
	);
	desire_failure (
	    indirectgen =
	        min::new_indirect_indexed_aux_gen
		    ( indirect_aux, 1 << 20 );
	);
	MIN_ASSERT
	    ( ! min::is_stub ( indirectgen ) );
	MIN_ASSERT
	    ( ! min::is_direct_str ( indirectgen ) );
 
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
	    indexgen = min::new_index_gen ( 1 << 24 );
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
	    			( 1 << 24 );
	);
	MIN_ASSERT ( ! min::is_stub ( codegen ) );
	MIN_ASSERT ( ! min::is_direct_str ( codegen ) );
 
#	if MIN_IS_LOOSE
	    cout << endl;
	    cout << "Test long control code general"
	            " values:" << endl;
	    min::uns64 longcode = 0xF123456789;
	    min::gen longcodegen =
		MUP::new_long_control_code_gen
		    ( longcode );
	    cout << "longcodegen: "
		 << print_gen ( longcodegen ) << endl;
	    MIN_ASSERT
	        ( min::is_control_code
		      ( longcodegen ) );
	    MIN_ASSERT
		(    min::gen_subtype_of ( longcodegen )
		  == min::GEN_CONTROL_CODE );
	    MIN_ASSERT
		(    min::long_control_code_of
			 ( longcodegen )
		  == longcode );
	    desire_success (
		longcodegen =
		    min::new_long_control_code_gen
		        ( longcode );
	    );
	    desire_failure (
		longcodegen =
		    min::new_long_control_code_gen
			( min::uns64(1) << 40 );
	    );
	    MIN_ASSERT
	        ( ! min::is_stub ( longcodegen ) );
	    MIN_ASSERT
	        ( ! min::is_direct_str
		        ( longcodegen ) );
#	endif
 
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
	        min::new_special_gen ( 1 << 24 );
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
	min::stub * stub1 =
	    (min::stub *) (sizeof (min::stub));
	min::stub * stub2 =
	    (min::stub *) (5 * sizeof (min::stub));

	cout << endl;
	cout << "Test controls sans stub addresses:"
	     << endl;
	min::uns64 control1 =
	    MUP::new_control ( type1, v1, flags );
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
	cout << "Test non-gc controls with stub"
	        " addresses:" << endl;
	min::uns64 control2 =
	    MUP::new_control ( type1, stub1, hiflag );
	cout << "control2: " << hex << control2 << dec
	     << endl;
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
	cout << "re-control2: " << hex << control2
	     << dec << endl;
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
	cout << "Test gc controls with stub"
	        " addresses:" << endl;
	min::uns64 control3 =
	    MUP::new_gc_control
	    	( type1, stub1, hiflag );
	cout << "control3: " << hex << control3 << dec
	     << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control3 )
	      == type1 );
        MIN_ASSERT
	    (    MUP::stub_of_gc_control ( control3 )
	      == stub1 );
        MIN_ASSERT ( control3 & hiflag );
        MIN_ASSERT ( ! ( control3 & loflag ) );
        MIN_ASSERT ( ! ( control3 & midflag ) );

	control3 =
	    MUP::renew_control_stub
		( control3, stub2 );
	cout << "re-control3: " << hex << control3
	     << dec << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control3 )
	      == type1 );
        MIN_ASSERT
	    (    MUP::stub_of_control ( control3 )
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
	min::stub stub_struct;
	min::stub * stub = & stub_struct;

        cout << endl;
	cout << "Test stub value set/read functions:"
	     << endl;
	min::uns64 u = 0x9047814326432464;
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
	cout << "Test stub GC related functions:"
	     << endl;
	MIN_ASSERT
	    ( min::is_collectible ( min::NUMBER ) );
	MIN_ASSERT
	    ( ! min::is_collectible ( min::LIST_AUX ) );
	MUP::set_type_of ( stub, min::NUMBER );
	MIN_ASSERT ( ! min::is_deallocated ( stub ) );
	desire_success (
	    assert_allocated
	        ( stub, MIN_DEALLOCATED_LIMIT + 1 )
	);
	MUP::set_type_of ( stub, min::DEALLOCATED );
	MIN_ASSERT ( min::is_deallocated ( stub ) );
	desire_failure (
	    assert_allocated
	        ( stub, MIN_DEALLOCATED_LIMIT + 1 )
	);

	cout << endl;
	cout << "Finish Stub Functions Test!" << endl;
    }

// Process Interface
// ------- ---------
    {
	cout << endl;
	cout << "Start Process Interface Test!" << endl;
	MUP::interrupt_flag = false;
	MUP::relocated_flag = false;

	// Process control testing is TBD.

        cout << endl;
	cout << "Test interrupt function:" << endl;
	interrupt_called = false;
	min::interrupt();
	MIN_ASSERT ( ! interrupt_called );
	MUP::interrupt_flag = true;
	min::interrupt();
	MIN_ASSERT ( interrupt_called );

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
	    MIN_ASSERT ( ! min::relocated_flag() )
	    min::set_relocated_flag ( true );
	    MIN_ASSERT ( r );
	    MIN_ASSERT ( ! min::relocated_flag() )
	}
	MIN_ASSERT ( min::relocated_flag() );

	// Now relocated flag is true.
	{
	    min::relocated r;
	    MIN_ASSERT ( ! min::relocated_flag() )
	    MIN_ASSERT ( ! r );
	    MIN_ASSERT ( ! min::relocated_flag() )
	}
	MIN_ASSERT ( min::relocated_flag() )

	cout << endl;
	cout << "Finish Process Interface Test!"
	     << endl;
    }

// Garbage Collector Interface
// ------- --------- ---------

    {
	cout << endl;
	cout << "Start Garbage Collector"
	        " Interface Test!" << endl;
	initialize_stub_region();
	initialize_body_region();
	initialize_hash_tables();
	min::stub * stack[2];
	MUP::gc_stack = stack;
	MUP::gc_stack_end = stack + 2;
	min::stub s1, s2;
	const min::uns64 marked_flag =
	       min::uns64(1)
	    << ( 56 - MIN_GC_FLAG_BITS );
	const min::uns64 scavenged_flag =
	    marked_flag << 1;
	MUP::gc_stack_marks = marked_flag;

        cout << endl;
	cout << "Test mutator functions:"
	     << endl;
	MUP::set_control_of ( &s1, 0 );
	MUP::set_flags_of ( &s1, scavenged_flag );
	MUP::set_control_of ( &s2, 0 );
	MUP::gc_stack = stack;
	MUP::gc_stack_marks = 0;
	MUP::gc_write_update ( &s1, &s2 );
	MIN_ASSERT
	    ( MUP::test_flags_of ( &s2, marked_flag ) );
	MIN_ASSERT
	    ( MUP::gc_stack == stack );
	MUP::set_control_of ( &s2, 0 );
	MUP::gc_stack_marks = marked_flag;
	MUP::gc_write_update ( &s1, &s2 );
	MIN_ASSERT
	    ( MUP::test_flags_of ( &s2, marked_flag ) );
	MIN_ASSERT
	    ( MUP::gc_stack == stack + 1 );
	MIN_ASSERT ( stack[0] == &s2 );
	MUP::gc_write_update ( &s1, &s2 );
	MIN_ASSERT
	    ( MUP::test_flags_of ( &s2, marked_flag ) );
	MIN_ASSERT
	    ( MUP::gc_stack == stack + 1 );
	MIN_ASSERT
	    ( ! MUP::test_flags_of
	    	     ( &s1, marked_flag ) );
	MIN_ASSERT
	    ( ! MUP::test_flags_of
	    	     ( &s2, scavenged_flag ) );
	MUP::clear_flags_of ( &s2, marked_flag );
	MUP::gc_write_update ( &s1, &s2 );
	MIN_ASSERT
	    ( MUP::gc_stack == stack + 2 );
	MIN_ASSERT ( stack[1] == &s2 );
	MUP::clear_flags_of ( &s2, marked_flag );
	MUP::gc_write_update ( &s1, &s2 );
	MIN_ASSERT
	    ( MUP::gc_stack == stack + 2 );

        cout << endl;
	cout << "Test stub allocator functions:"
	     << endl;
	MUP::gc_new_stub_flags = 0;
	min::stub * stub1 = MUP::new_stub();
	MIN_ASSERT ( stub1 == begin_stub_region + 1 );
	MIN_ASSERT
	    ( stub1 == MUP::last_allocated_stub );
	MIN_ASSERT
	    ( region_stubs_allocated == 2 );
	MIN_ASSERT
	    ( min::type_of ( stub1 ) == min::FREE );
	MIN_ASSERT
	    ( ! MUP::test_flags_of
	    	     ( stub1, marked_flag ) );
	MUP::gc_new_stub_flags = marked_flag;
	min::stub * stub2 = MUP::new_stub();
	MIN_ASSERT
	    ( stub2 == MUP::last_allocated_stub );
	MIN_ASSERT
	    ( region_stubs_allocated == 3 );
	MIN_ASSERT ( stub2 == begin_stub_region + 2 );
	MIN_ASSERT
	    ( min::type_of ( stub2 ) == min::FREE );
	MIN_ASSERT
	    ( MUP::test_flags_of
	    	     ( stub2, marked_flag ) );
	MUP::gc_expand_stub_free_list ( 2 );
	MIN_ASSERT
	    ( region_stubs_allocated == 5 );
	MIN_ASSERT
	    ( stub2 == MUP::last_allocated_stub );
	min::stub * stub3 = MUP::new_aux_stub();
	MIN_ASSERT ( stub3 == begin_stub_region + 4 );
	MIN_ASSERT
	    ( region_stubs_allocated == 5 );
	MIN_ASSERT
	    ( stub2 == MUP::last_allocated_stub );
	min::stub * stub4 = MUP::new_stub();
	MIN_ASSERT ( stub4 == begin_stub_region + 3 );
	MIN_ASSERT
	    ( region_stubs_allocated == 5 );
	MIN_ASSERT
	    ( stub4 == MUP::last_allocated_stub );


        cout << endl;
	cout << "Test body allocator functions:"
	     << endl;
	MUP::body_control * body1 =
	    MUP::new_body ( 128 );
	char * p1 = (char *) body1
	          + sizeof ( * body1 );
	memset ( p1, 0xBB, 128 );
	MUP::body_control * body2 =
	    MUP::new_body ( 128 );
	char * p2 = (char *) body2
	          + sizeof ( * body2 );
	memset ( p2, 0xBB, 128 );
	MIN_ASSERT ( memcmp ( p1, p2, 128 ) == 0 );
	MIN_ASSERT ( p1 + sizeof ( MUP::body_control )
	                + 128 == p2 );
	add_to_free_body ( 200 );
	MUP::body_control * body3 =
	    MUP::new_body ( 128 );
	char * p3 = (char *) body3
	          + sizeof ( * body3 );
	memset ( p3, 0xCC, 128 );
	MUP::body_control * body4 =
	    MUP::new_body ( 128 );
	char * p4 = (char *) body4
	          + sizeof ( * body4 );
	memset ( p4, 0xCC, 128 );
	MIN_ASSERT ( memcmp ( p3, p4, 128 ) == 0 );
	MIN_ASSERT ( p3 + sizeof ( MUP::body_control )
	                + 128 == p4 );

	cout << endl;
	cout << "Finish Garbage Collector"
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

	min::gen n1 = min::new_gen ( 12345 );
	cout << "n1: " << print_gen ( n1 ) << endl;
	MIN_ASSERT ( min::is_num ( n1 ) );
	MIN_ASSERT ( min::is_atom ( n1 ) );
	MIN_ASSERT ( min::int_of ( n1 ) == 12345 );
	MIN_ASSERT ( min::float_of ( n1 ) == 12345 );
	min::uns32 n1hash = min::numhash ( n1 );
	cout << "n1hash: " << hex << n1hash << dec
	     << endl;
	MIN_ASSERT
	    ( n1hash == min::floathash ( 12345 ) );
	MIN_ASSERT
	    ( n1hash == min::hash ( n1 ) );
	MIN_ASSERT ( min::new_gen ( 12345 ) == n1 );

	min::gen n2 = min::new_gen ( 1.2345 );
	cout << "n2: " << print_gen ( n2 ) << endl;
	MIN_ASSERT ( min::is_num ( n2 ) );
	MIN_ASSERT ( min::is_atom ( n2 ) );
	MIN_ASSERT ( min::float_of ( n2 ) == 1.2345 );
	min::uns32 n2hash = min::numhash ( n2 );
	cout << "n2hash: " << hex << n2hash << dec
	     << endl;
	MIN_ASSERT
	    ( n2hash == min::floathash ( 1.2345 ) );
	MIN_ASSERT
	    ( n2hash == min::hash ( n2 ) );
	MIN_ASSERT ( min::new_gen ( 1.2345 ) == n2 );

	min::gen n3 = min::new_gen ( 1 << 30 );
	cout << "n3: " << print_gen ( n3 ) << endl;
	MIN_ASSERT ( min::is_num ( n3 ) );
	MIN_ASSERT ( min::is_atom ( n3 ) );
	MIN_ASSERT ( min::int_of ( n3 ) == 1 << 30 );
	MIN_ASSERT ( min::float_of ( n3 ) == 1 << 30 );
	min::uns32 n3hash = min::numhash ( n3 );
	cout << "n3hash: " << hex << n3hash << dec
	     << endl;
	MIN_ASSERT
	    ( n3hash == min::floathash ( 1 << 30 ) );
	MIN_ASSERT
	    ( n3hash == min::hash ( n3 ) );
	MIN_ASSERT ( min::new_gen ( 1 << 30 ) == n3 );

	cout << endl;
	cout << "Finish Numbers Test!" << endl;
    }

// Strings

    {
	cout << endl;
	cout << "Start Strings Test!" << endl;
	struct min::stub * sstub = MUP::new_stub();
	struct min::stub * lstub = MUP::new_stub();
	MUP::set_control_of
	    ( sstub,
	      MUP::new_gc_control
	          ( min::SHORT_STR, 0 ) );
	MUP::set_control_of
	    ( lstub,
	      MUP::new_gc_control
	          ( min::LONG_STR, 0 ) );
	union {
	    min::uns64 u64;
	    char str[9];
	} in, out;
	char buffer[20];
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
	cout << "Test short strings:" << endl;
	strcpy ( in.str, s7 );
	MUP::set_short_str_of ( sstub, in.u64 );
	out.u64 = MUP::short_str_of ( sstub );
	out.str[8] = 0;
	MIN_ASSERT ( strcmp ( in.str, out.str ) == 0 );
	MIN_ASSERT ( min::strhash ( sstub ) == s7hash );
	MIN_ASSERT ( min::strlen ( sstub ) == 7 );
	min::strcpy ( buffer, sstub );
	MIN_ASSERT ( strcmp ( buffer, s7 ) == 0 );

	strcpy ( in.str, s8 );
	MUP::set_short_str_of ( sstub, in.u64 );
	out.u64 = MUP::short_str_of ( sstub );
	out.str[8] = 0;
	MIN_ASSERT ( strcmp ( in.str, out.str ) == 0 );
	MIN_ASSERT ( min::strhash ( sstub ) == s8hash );
	MIN_ASSERT ( min::strlen ( sstub ) == 8 );
	min::strcpy ( buffer, sstub );
	MIN_ASSERT ( strcmp ( buffer, s8 ) == 0 );
	buffer[7] = 0;
	min::strncpy ( buffer, sstub, 7 );
	MIN_ASSERT ( strcmp ( buffer, s7 ) == 0 );

	cout << endl;
	cout << "Test long strings:" << endl;
	MUP::body_control * bc =
	    MUP::new_body
		( sizeof (MUP::long_str) + 14 );
	MUP::set_pointer_of ( lstub, bc + 1 );
	MUP::long_str * lstr =
	    MUP::long_str_of ( lstub );
	MUP::set_length_of ( lstr, 13 );
	MUP::set_hash_of ( lstr, 0 );
	char * wp = MUP::writable_str_of ( lstr );
	const char * rp = MUP::str_of ( lstr );
	strcpy ( wp, s13 );
	MIN_ASSERT ( wp == rp );
	cout << "MUP::str_of (long_str_of ( lstub )): "
	     << rp << endl;
	MIN_ASSERT
	    ( (void * ) wp == (void *) (lstr + 1 ) );
	MIN_ASSERT ( min::length_of ( lstr ) == 13 );
	MIN_ASSERT ( MUP::hash_of ( lstr ) == 0 );
	MIN_ASSERT ( min::hash_of ( lstr ) == s13hash );
	MIN_ASSERT
	    ( min::strhash ( lstub ) == s13hash );
	MIN_ASSERT ( min::strlen ( lstub ) == 13 );
	min::strcpy ( buffer, lstub );
	MIN_ASSERT ( strcmp ( buffer, s13 ) == 0 );
	buffer[8] = 0;
	min::strncpy ( buffer, lstub, 8 );
	MIN_ASSERT ( strcmp ( buffer, s8 ) == 0 );

	cout << endl;
	cout << "Test string general values:" << endl;
	min::gen strgen3 = min::new_gen ( s3 );
	min::gen strgen7 = min::new_gen ( s7 );
	min::gen strgen8 = min::new_gen ( s8 );
	min::gen strgen13 = min::new_gen ( s13 );

	MIN_ASSERT ( min::is_str ( strgen3 ) );
	MIN_ASSERT ( min::is_direct_str ( strgen3 ) );
	MIN_ASSERT ( min::is_str ( strgen7 ) );
	MIN_ASSERT ( min::is_stub ( strgen7 ) );
	min::stub * stub7 = MUP::stub_of ( strgen7 );
	MIN_ASSERT (    min::type_of ( stub7 )
	             == min::SHORT_STR );
	MIN_ASSERT ( min::is_str ( strgen8 ) );
	MIN_ASSERT ( min::is_stub ( strgen8 ) );
	min::stub * stub8 = MUP::stub_of ( strgen8 );
	MIN_ASSERT (    min::type_of ( stub8 )
	             == min::SHORT_STR );
	MIN_ASSERT ( min::is_str ( strgen13 ) );
	MIN_ASSERT ( min::is_stub ( strgen13 ) );
	min::stub * stub13 = MUP::stub_of ( strgen13 );
	MIN_ASSERT (    min::type_of ( stub13 )
	             == min::LONG_STR );

	MIN_ASSERT ( min::strlen ( strgen3 ) == 3 );
	MIN_ASSERT
	    ( min::strhash ( strgen3 ) == s3hash );
	min::strcpy ( buffer, strgen3 );
	MIN_ASSERT ( strcmp ( buffer, s3 ) == 0 );
	MIN_ASSERT ( min::strlen ( strgen7 ) == 7 );
	MIN_ASSERT
	    ( min::strhash ( strgen7 ) == s7hash );
	min::strcpy ( buffer, strgen7 );
	MIN_ASSERT ( strcmp ( buffer, s7 ) == 0 );
	MIN_ASSERT ( min::strlen ( strgen8 ) == 8 );
	MIN_ASSERT
	    ( min::strhash ( strgen8 ) == s8hash );
	min::strcpy ( buffer, strgen8 );
	MIN_ASSERT ( strcmp ( buffer, s8 ) == 0 );
	MIN_ASSERT ( min::strlen ( strgen13 ) == 13 );
	MIN_ASSERT
	    ( min::strhash ( strgen13 ) == s13hash );
	min::strcpy ( buffer, strgen13 );
	MIN_ASSERT ( strcmp ( buffer, s13 ) == 0 );

	cout << endl;
	cout << "Test string pointers:" << endl;
	// Test body relocation first.
	MUP::set_pointer_of
	    ( stub13,
	        relocate_body
		    (   (MUP::body_control *)
		        MUP::pointer_of ( stub13 )
		      - 1,
		        min::strlen ( stub13 )
		      + sizeof (MUP::long_str) )
		+ 1 );
	MIN_ASSERT ( min::strlen ( strgen13 ) == 13 );
	MIN_ASSERT
	    ( min::strhash ( strgen13 ) == s13hash );
	min::strcpy ( buffer, strgen13 );
	MIN_ASSERT ( strcmp ( buffer, s13 ) == 0 );
	MUP::str_pointer p3 ( strgen3 );
	MUP::str_pointer p7 ( strgen7 );
	MUP::str_pointer p8 ( strgen8 );
	MUP::str_pointer p13 ( strgen13 );
	MIN_ASSERT
	    ( strcmp ( min::str_of ( p3 ), s3 ) == 0 );
	MIN_ASSERT
	    ( strcmp ( min::str_of ( p7 ), s7 ) == 0 );
	MIN_ASSERT
	    ( strcmp ( min::str_of ( p8 ), s8 ) == 0 );
	const char * p13str_before = min::str_of ( p13 );
	cout << "p13str_before: " << hex
	     << (MINT::pointer_uns) p13str_before
	     << dec << endl;
	MIN_ASSERT
	    ( strcmp ( p13str_before, s13 ) == 0 );
	MUP::set_pointer_of
	    ( stub13,
	        relocate_body
		    (   (MUP::body_control *)
		        MUP::pointer_of ( stub13 )
		      - 1,
		        min::strlen ( stub13 )
		      + sizeof (MUP::long_str) )
		+ 1 );
	const char * p13str_mid = min::str_of ( p13 );
	cout << "p13str_mid: " << hex
	     << (MINT::pointer_uns) p13str_mid
	     << dec << endl;
	MIN_ASSERT
	    (    strcmp ( min::str_of ( p13 ), s13 )
	      != 0 );
	min::relocate ( p13 );
	const char * p13str_after = min::str_of ( p13 );
	cout << "p13str_after: " << hex
	     << (MINT::pointer_uns) p13str_after
	     << dec << endl;
	MIN_ASSERT
	    (    strcmp ( min::str_of ( p13 ), s13 )
	      == 0 );
	
	cout << endl;
	cout << "Finish Strings Test!" << endl;
    }

// Labels
// Atom Functions
// Objects
// Object Vector Level
// Object List Level
// Object Attribute Level

// Finish
// ------

    } catch ( min_assert_exception * x ) {
        cout << "EXITING BECAUSE OF FAILED MIN_ASSERT"
	     << endl;
	exit ( 1 );
    }

    cout << endl;
    cout << "Finished Test!" << endl;
}
