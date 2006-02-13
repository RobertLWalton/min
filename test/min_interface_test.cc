// MIN Language Interface Test Program
//
// File:	min_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Feb 13 11:57:53 EST 2006
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2006/02/13 16:55:17 $
//   $RCSfile: min_interface_test.cc,v $
//   $Revision: 1.11 $

// Table of Contents:
//
//	Setup
//	C++ Number Types
//	Internal Pointer Conversion Functions
//	General Value Types and Data
//	General Value Constructor/Test/Read Functions
//	Control Values
//	Stub Types and Data
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

void min_assert
	( bool value,
	  const char * file, unsigned line,
	  const char * expression );
# define MIN_ASSERT(expr) \
    min_assert ( expr ? true : false, \
    		 __FILE__, __LINE__, #expr );

# include <min.h>
# define MUP min::unprotected
# include <iostream>
# include <iomanip>
# include <cstring>
using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::ostream;

struct print_gen {
    min::gen g;
    print_gen ( min::gen g ) : g ( g ) {}
    friend ostream & operator << ( ostream & s, print_gen pg )
    {
	return s << hex << pg.g << dec;
    }
};

struct min_assert_exception {};

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

#define desire_success(statement) \
    { cout << __FILE__ << ":" << __LINE__ \
	   << " desire success:" << endl \
	   << "    " << #statement << endl; \
      statement; }

#define desire_failure(statement) \
    try { cout << __FILE__ << ":" << __LINE__ \
               << " desire failure:" << endl \
	       << "    " << #statement << endl; \
	  statement; \
          cout << "EXITING BECAUSE OF SUCCESSFUL" \
	          " MIN_ASSERT" << endl; \
	  exit (1 ); } \
    catch ( min_assert_exception * x ) {}

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

	cout << "Finish Number Types Test!" << endl;
    }


// Internal Pointer Conversion Functions
// -------- ------- ---------- ---------

    {
	cout << endl;
	cout << "Start Internal Pointer Conversion"
	        " Test!" << endl;
    	char buffer[1];
	min::stub * stub =
	    (min::stub *) (sizeof (min::stub));

	cout << "Test pointer/uns64 conversions:"
	     << endl;
	min::uns64 u64 =
	    min::internal::pointer_to_uns64 ( buffer );
	char * b64 = (char *)
	    min::internal::uns64_to_pointer ( u64 );
	MIN_ASSERT ( b64 == buffer );

#	if MIN_IS_COMPACT
	    cout << "Test pointer/uns32 conversions:"
		 << endl;
	    min::uns32 u32 =
		min::internal::pointer_to_uns32
		    ( buffer );
	    char * b32 = (char *)
		min::internal::uns32_to_pointer ( u32 );
	    MIN_ASSERT ( b32 == buffer );
	    cout << "Test stub/uns32 conversions:"
		 << endl;
	    u32 = min::internal::stub_to_uns32 ( stub );
	    min::stub * s32 =
		min::internal::uns32_to_stub ( u32 );
	    MIN_ASSERT ( s32 == stub );
#	elif MIN_IS_LOOSE
	    cout << "Test stub/uns64 conversions:"
		 << endl;
	    u64 = min::internal::stub_to_uns64 ( stub );
	    min::stub * s64 =
		min::internal::uns64_to_stub ( u64 );
	    MIN_ASSERT ( s64 == stub );
#	endif

	cout << "Finish Internal Pointer Conversion"
	        " Test!" << endl;
    }

// General Value Types and Data
// ------- ----- ----- --- ----

    // There are no general value types and data tests.


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
	min::gen stubgen = MUP::new_gen ( stub );
	cout << "stubgen: " << print_gen ( stubgen )
	     << endl;
	MIN_ASSERT ( min::is_stub ( stubgen ) );
	MIN_ASSERT ( MUP::stub_of ( stubgen ) == stub );
	stubgen = min::new_gen ( stub );
	MIN_ASSERT ( min::is_stub ( stubgen ) );
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
	    MIN_ASSERT ( min::is_direct_float ( fgen ) );
	    MIN_ASSERT
	        ( MUP::direct_float_of ( fgen ) == f );
	    fgen = min::new_direct_float_gen ( f );
	    MIN_ASSERT ( min::is_direct_float ( fgen ) );
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
	    ( min::list_aux_of ( listauxgen ) == reaux );
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
	    ( min::is_indirect_pair_aux ( pairauxgen ) );
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
	    ( min::control_code_of ( codegen ) == code );
	desire_success (
	    codegen = min::new_control_code_gen ( code );
	);
	desire_failure (
	    codegen = min::new_control_code_gen
	    			( 1 << 24 );
	);
	MIN_ASSERT ( ! min::is_stub ( codegen ) );
	MIN_ASSERT ( ! min::is_direct_str ( codegen ) );
	


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

	min::uns64 control2 =
	    MUP::new_control ( type1, stub1, hiflag );
	cout << "control2: " << hex << control2 << dec
	     << endl;
        MIN_ASSERT
	    (    MUP::type_of_control ( control2 )
	      == type1 );
        MIN_ASSERT
	    (    MUP::pointer_of_control ( control2 )
	      == stub1 );
        MIN_ASSERT
	    (    MUP::stub_of_control ( control2 )
	      == stub1 );
        MIN_ASSERT ( control2 & hiflag );
        MIN_ASSERT ( ! ( control2 & loflag ) );
        MIN_ASSERT ( ! ( control2 & midflag ) );

	control2 =
	    MUP::renew_control_pointer
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
	cout << "Finish Control Value Test!" << endl;
    }

// Stub Types and Data
// ---- ----- --- ----

    // There are no stub types and data tests.

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
	cout << "Finish Stub Functions Test!" << endl;
    }
// Process Interface
// Garbage Collector Interface
// Numbers
// Strings
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
