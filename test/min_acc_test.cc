// MIN Language Allocator/Collector/Compactor Test
// Program
//
// File:	min_acc_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Mar 16 16:07:46 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Setup

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
# include <min_acc.h>
# define MUP min::unprotected
# define MINT min::internal
# define MACC min::acc

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
    bool print_save = min_assert_print;
    min_assert_print = false;

    min::unsptr size = 2 + random_uns32() % m;

    min::gen obj = min::new_obj_gen ( size );
    min::obj_vec_insptr ep ( obj );

    for ( min::unsptr j = 0; j < size; ++ j )
	min::attr_push(ep) = min::new_num_gen ( j );

    min_assert_print = print_save;

    return obj;
}

// Create an object that is a vector of n new objects
// each created by calling create_object(m).
//
static min::gen create_vec_of_objects
    ( min::unsptr n, min::unsptr m )
{
    bool print_save = min_assert_print;
    min_assert_print = false;

    min::gen obj = min::new_obj_gen ( n );

    min::obj_vec_insptr vp ( obj );

    for ( min::unsptr i = 0; i < n; ++ i )
        min::attr_push(vp) = create_object ( m );

    min_assert_print = print_save;

    return obj;
}

// Given an object created by create_vec_of_objects,
// randomly deallocate an element and allocate a
// replacement using create_object(m).  Do this n times.
//
static void random_deallocate
	( min::gen obj, min::unsptr n, min::unsptr m )
{
    bool print_save = min_assert_print;
    min_assert_print = false;

    min::obj_vec_updptr vp ( obj );
    min::unsptr size = min::attr_size_of ( vp );
    while ( n -- )
    {
        min::unsptr i = random_uns32() % size;
	min::deallocate
	    ( MUP::stub_of ( min::attr ( vp, i ) ) );
	min::set_attr ( vp, i, create_object ( m ) );
    }
    min_assert_print = print_save;
}


// Check an object created by create_vec_of_objects.
// Return true if object checks and false if it does
// not.
//
static bool check_vec_of_objects ( min::gen obj )
{
    bool print_save = min_assert_print;
    min_assert_print = false;

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
		     << " != " << min::attr ( ep, j )
		     << " = object[" << i << "][" << j
		     << "]" << endl;
	        checks = false;
		break;
	    }
	}
    }

    min_assert_print = print_save;
    return checks;
}

// Main Program
// ---- -------

int main ()
{
    cout << endl;
    cout << "Start Test!" << endl;

    try {

	cout << "Before Allocation" << endl;
	MACC::print_acc_statistics ( cout );

    	min::gen v =
	    create_vec_of_objects ( 1000, 300 );
	MIN_ASSERT ( check_vec_of_objects ( v ) );
	cout << "After Allocation" << endl;
	MACC::print_acc_statistics ( cout );

	random_deallocate ( v, 100000, 300 );
	MIN_ASSERT ( check_vec_of_objects ( v ) );
	cout << "After Random Deallocation" << endl;
	MACC::print_acc_statistics ( cout );

	MACC::collect ( MACC::ephemeral_levels );
	MIN_ASSERT ( check_vec_of_objects ( v ) );
	cout << "After Highest Level GC" << endl;
	MACC::print_acc_statistics ( cout );


    } catch ( min_assert_exception * x ) {
        cout << "EXITING BECAUSE OF FAILED MIN_ASSERT"
	     << endl;
	exit ( 1 );
    }

    cout << endl;
    cout << "Finished Test!" << endl;
}

