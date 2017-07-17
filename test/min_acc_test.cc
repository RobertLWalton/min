// MIN Language Allocator/Collector/Compactor Test
// Program
//
// File:	min_acc_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jul 17 14:02:54 EDT 2017
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Setup
//	ACC Interface Test
//	ACC Garbage Collector Test

// Setup
// -----

# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <cstring>
using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::ostream;

# define MIN_ASSERT MIN_ASSERT_CALL_ALWAYS
# include <min.h>
# include <min_acc.h>
# define MUP min::unprotected
# define MINT min::internal
# define MACC min::acc

// ACC Interface Test
//
void test_acc_interface ( void )
{
    cout << endl;
    cout << "Start Allocator/Collector/Compactor"
	    " Interface Test!" << endl;

    cout << endl;
    cout << "Test stub allocator functions:"
	 << endl;
    min::unsptr sbase = MUP::acc_stubs_allocated;
    cout << "initial stubs allocated = "
	 << sbase << endl;
    min::stub * stub1 = MUP::new_acc_stub();
    MIN_CHECK ( stub1 == MINT::last_allocated_stub );
    MIN_CHECK ( MUP::acc_stubs_allocated == sbase + 1 );
    MIN_CHECK
	( min::type_of ( stub1 ) == min::ACC_FREE );
    min::stub * stub2 = MUP::new_acc_stub();
    MIN_CHECK ( stub2 == MINT::last_allocated_stub );
    MIN_CHECK ( MUP::acc_stubs_allocated == sbase + 2 );
    MIN_CHECK
	( min::type_of ( stub2 ) == min::ACC_FREE );

    min::unsptr free_stubs = MINT::number_of_free_stubs;
    MINT::acc_expand_stub_free_list ( free_stubs + 2 );
    MIN_CHECK (    MINT::number_of_free_stubs
                >= free_stubs + 2 );

    MIN_CHECK ( MUP::acc_stubs_allocated == sbase + 2 );
    MIN_CHECK ( stub2 == MINT::last_allocated_stub );
    min::stub * stub3 = MUP::new_acc_stub();
    MIN_CHECK ( MUP::acc_stubs_allocated == sbase + 3 );
    MIN_CHECK ( stub3 == MINT::last_allocated_stub );
    min::stub * stub4 = MUP::new_acc_stub();
    MIN_CHECK ( MUP::acc_stubs_allocated == sbase + 4 );
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

    min::stub * stub5 = MUP::new_acc_stub();
    MUP::new_body ( stub5, 128 );
    char * p5 = (char *) MUP::ptr_of ( stub5 );
    memset ( p5, 0xCC, 128 );
    MIN_CHECK ( memcmp ( p3, p5, 128 ) == 0 );
    {
	MUP::resize_body rb ( stub5, 128, 128 );
	memcpy ( MUP::new_body_ptr_ref ( rb ),
	         MUP::ptr_of ( stub5 ), 128 );
    }
    char * p6 = (char *) MUP::ptr_of ( stub5 );
    MIN_CHECK ( memcmp ( p3, p6, 128 ) == 0 );
    MIN_CHECK ( p5 != p6 );
    MUP::deallocate_body ( stub5, 128 );
    MIN_CHECK ( min::type_of ( stub5 )
		 == min::DEALLOCATED );
    char * p7 = (char *) MUP::ptr_of ( stub5 );
    MIN_CHECK ( p6 != p7 );

    min::deallocate ( stub1 );
    min::deallocate ( stub2 );
    min::deallocate ( stub3 );
    min::deallocate ( stub4 );

    cout << endl;
    cout << "Test stub swap function:"
	 << endl;

    bool print_save = min::assert_print;
    min::assert_print = false;

    min::locatable_gen num1 ( min::new_num_gen ( 1 ) );
    min::locatable_gen num2 ( min::new_num_gen ( 2 ) );
    min::locatable_gen obj1
        ( min::new_obj_gen ( 10 ) );
    {
	min::obj_vec_insptr vp1 ( obj1 );
	min::attr_push(vp1) = num1;
    }
    min::locatable_gen obj2
        ( min::new_obj_gen ( 10 ) );
    {
	min::obj_vec_insptr vp2 ( obj2 );
	min::attr_push(vp2) = num2;
    }
    min::locatable_gen gen_pre
        ( min::new_preallocated_gen ( 55 ) );

    min::assert_print = print_save;

    MIN_CHECK ( min::is_preallocated ( gen_pre ) );
    MIN_CHECK
        ( min::id_of_preallocated ( gen_pre ) == 55 );

    const min::stub * stub_obj1 = MUP::stub_of ( obj1 );
    const min::stub * stub_obj2 = MUP::stub_of ( obj2 );

    {
	min::obj_vec_insptr vp1 ( obj1 );
	min::obj_vec_insptr vp2 ( obj2 );

	MIN_CHECK ( vp1[0] == num1 );
	MIN_CHECK ( vp2[0] == num2 );
    }

    {
        min::uns64 * bp1 =
	  (min::uns64 *) MUP::ptr_of ( stub_obj1 ) - 1;
	MIN_CHECK
	    ( stub_obj1 == MACC::stub_of_body ( bp1 ) );
        min::uns64 * bp2 =
	  (min::uns64 *) MUP::ptr_of ( stub_obj2 ) - 1;
	MIN_CHECK
	    ( stub_obj2 == MACC::stub_of_body ( bp2 ) );
    }

    void * p_obj1 = MUP::ptr_of ( stub_obj1 );
    void * p_obj2 = MUP::ptr_of ( stub_obj2 );
    MIN_CHECK ( p_obj1 != p_obj2 );

    MUP::stub_swap ( stub_obj1, stub_obj2 );

    MIN_CHECK ( p_obj1 == MUP::ptr_of ( stub_obj2 ) );
    MIN_CHECK ( p_obj2 == MUP::ptr_of ( stub_obj1 ) );

    {
        min::uns64 * bp1 =
	  (min::uns64 *) MUP::ptr_of ( stub_obj1 ) - 1;
	MIN_CHECK
	    ( stub_obj1 == MACC::stub_of_body ( bp1 ) );
        min::uns64 * bp2 =
	  (min::uns64 *) MUP::ptr_of ( stub_obj2 ) - 1;
	MIN_CHECK
	    ( stub_obj2 == MACC::stub_of_body ( bp2 ) );
    }

    {
	min::obj_vec_insptr vp1 ( obj1 );
	min::obj_vec_insptr vp2 ( obj2 );

	MIN_CHECK ( vp1[0] == num2 );
	MIN_CHECK ( vp2[0] == num1 );
    }

    const min::stub * stub_pre =
        min::stub_of ( gen_pre );
    MIN_CHECK
        (    min::type_of ( stub_pre )
	  == min::PREALLOCATED );
    MIN_CHECK
        ( MUP::body_size_of ( stub_pre ) == 0 );

    MUP::stub_swap ( stub_pre, stub_obj1 );

    MIN_CHECK
        (    min::type_of ( stub_obj1 )
	  == min::PREALLOCATED );
    MIN_CHECK
        ( MUP::body_size_of ( stub_obj1 ) == 0 );

    {
	min::obj_vec_insptr vp1 ( stub_pre );

	MIN_CHECK ( vp1[0] == num2 );
    }

    {
        min::uns64 * bp =
	  (min::uns64 *) MUP::ptr_of ( stub_pre ) - 1;
	MIN_CHECK
	    ( stub_pre == MACC::stub_of_body ( bp ) );
    }

    cout << endl;
    cout << "Finish Allocator/Collector/Compactor"
	    " Interface Test!" << endl;
}

// ACC Garbage Collector Test
// --- ------- --------- ----

static min::locatable_gen teststr;
    // Set to "this is a test str" before GC and
    // checked after GC.

// Helper functions for tests.

struct print_gen {
    min::gen g;
    print_gen ( min::gen g ) : g ( g ) {}
    friend ostream & operator <<
    	    ( ostream & s, print_gen pg )
    {
	return s << hex
	         << min::unprotected::value_of ( pg.g )
		 << dec;
    }
};

// Compute a random number.
// Multiply with carry algorithm of George Marsaglia.
//
static min::uns32 m_z = 789645;
static min::uns32 m_w = 6793764;
static min::uns32 random_uns32 ( void )
{
    m_z = 36969 * ( m_z & 0xFFFF ) + ( m_z >> 16 );
    m_w = 18000 * ( m_w & 0xFFFF ) + ( m_w >> 16 );
    return ( m_z << 16 ) + m_w;
}

// Create an object of size
//
//	2 + ( random_uns32() % m ).
//
// Set element j equal to j, fo j = 0, 1, ...
//
static min::gen create_object ( min::unsptr m )
{
    bool print_save = min::assert_print;
    min::assert_print = false;

    min::unsptr size = 2 + random_uns32() % m;

    min::locatable_gen obj;
    obj = min::new_obj_gen ( size );
    min::obj_vec_insptr ep ( obj );

    for ( min::unsptr j = 0; j < size; ++ j )
	min::attr_push(ep) = min::new_num_gen ( j );

    min::assert_print = print_save;

    return obj;
}

// Create an object that is a vector of n new objects
// each created by calling create_object(m).
//
static min::gen create_vec_of_objects
    ( min::unsptr n, min::unsptr m )
{
    bool print_save = min::assert_print;
    min::assert_print = false;

    min::locatable_gen obj;
    obj = min::new_obj_gen ( n );

    min::obj_vec_insptr vp ( obj );

    for ( min::unsptr i = 0; i < n; ++ i )
        min::attr_push(vp) = create_object ( m );

    min::assert_print = print_save;

    return obj;
}

// Given an object created by create_vec_of_objects,
// randomly deallocate an element and allocate a
// replacement using create_object(m).  Do this n times.
//
static void random_deallocate
	( min::gen obj, min::unsptr n, min::unsptr m )
{
    bool print_save = min::assert_print;
    min::assert_print = false;

    min::obj_vec_updptr vp ( obj );
    min::unsptr size = min::attr_size_of ( vp );
    while ( n -- )
    {
        min::unsptr i = random_uns32() % size;
	min::deallocate
	    ( MUP::stub_of ( min::attr ( vp, i ) ) );
	min::attr ( vp, i ) = create_object ( m );
    }
    min::assert_print = print_save;
}


// Check an object created by create_vec_of_objects.
// Return true if object checks and false if it does
// not.
//
static bool check_vec_of_objects ( min::gen obj )
{
    bool print_save = min::assert_print;
    min::assert_print = false;

    bool checks = true;

    min::obj_vec_ptr vp ( obj );

    for ( min::unsptr i = 0;
          checks && i < min::attr_size_of ( vp );
	  ++ i )
    {
        min::gen element = min::attr ( vp, i );
	min::obj_vec_ptr ep ( element );

	for ( min::unsptr j = 0;
	      j < min::attr_size_of ( ep ); ++ j )
	{
	    min::gen value = min::new_num_gen ( j );
	    if ( min::attr ( ep, j ) != value )
	    {
		cout << "check_vec_of_objects FAILURE:"
		     << endl
		     << "   object[" << i << "][0] = "
		     << value
		     << " != "
		     << min::attr ( ep, j )
		     << " = object[" << i << "][" << j
		     << "]" << endl;
	        checks = false;
		break;
	    }
	}
    }

    min::assert_print = print_save;
    return checks;
}

void test_acc_garbage_collector ( void )
{
    cout << endl;
    cout << "Start ACC Garbage Collector Test!" << endl;

    try {

	cout << "Before Allocation" << endl;
	MACC::print_acc_statistics ( cout );

	::teststr = min::new_str_gen
	                ( "this is a test str" );

    	min::locatable_gen v;
	v = create_vec_of_objects ( 1000, 300 );
	MIN_CHECK ( check_vec_of_objects ( v ) );
	cout << "After Allocation" << endl;
	MACC::print_acc_statistics ( cout );

	random_deallocate ( v, 100000, 300 );
	MIN_CHECK ( check_vec_of_objects ( v ) );
	cout << "After Random Deallocation" << endl;
	MACC::print_acc_statistics ( cout );

	MACC::collect ( MACC::ephemeral_levels );
	MIN_CHECK ( check_vec_of_objects ( v ) );
	cout << "After Highest Level GC" << endl;
	MACC::print_acc_statistics ( cout );

	MIN_CHECK
	    (    ::teststr
	      == min::new_str_gen
	                ( "this is a test str" ) );


    } catch ( min::assert_exception * x ) {
        cout << "EXITING BECAUSE OF FAILED MIN_CHECK"
	     << endl;
	exit ( 1 );
    }

    cout << endl;
    cout << "Finish ACC Garbage Collector Test!"
         << endl;
}

// Main Program
// ---- -------

int main ()
{
    min::assert_err = stdout;
    min::assert_print = true;
    cout << endl;
    cout << "Initialize!" << endl;
    min::interrupt();

    test_acc_interface();
    test_acc_garbage_collector();
}
