// MIN Language Allocator/Collector/Compactor Test
// Program
//
// File:	min_acc_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Aug  4 06:56:19 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/08/04 13:37:13 $
//   $RCSfile: min_acc_test.cc,v $
//   $Revision: 1.7 $

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

// Create an object that is a vector of n new objects
// whose sizes are 2 + ( random_uns32() % m ).  Fill
// each object with a different number (i.e., all
// elements of each object will be equal).
//
static min::unsptr obj_number = 0;
static min::gen create_vec_of_objects
    ( min::unsptr n, min::unsptr m )
{
    bool print_save = min_assert_print;
    min_assert_print = false;

    min::gen obj = min::new_obj_gen ( n );

    min::insertable_vec_pointer vp ( obj );

    for ( min::unsptr i = 0; i < n; ++ i )
    {
	min::unsptr size = 2 + random_uns32() % m;
        min::gen element = min::new_obj_gen ( size );
	min::gen num =
	    min::new_num_gen ( ++ obj_number );
	min::insertable_vec_pointer ep ( element );
	for ( min::unsptr j = 0; j < size; ++ j )
	    min::attr_push ( ep, num );
        min::attr_push ( vp, element );
    }

    min_assert_print = print_save;

    return obj;
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

    min::vec_pointer vp ( obj );

    for ( min::unsptr i = 0;
          checks && i < min::attr_size_of ( vp );
	  ++ i )
    {
        min::gen element = min::attr ( vp, i );
	min::vec_pointer ep ( element );
	min::gen value = min::attr ( ep, 0 );
	for ( min::unsptr j = 1;
	      j < min::attr_size_of ( ep ); ++ j )
	{
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

    	min::gen v =
	    create_vec_of_objects ( 1000, 300 );
	MIN_ASSERT ( check_vec_of_objects ( v ) );


    } catch ( min_assert_exception * x ) {
        cout << "EXITING BECAUSE OF FAILED MIN_ASSERT"
	     << endl;
	exit ( 1 );
    }

    cout << endl;
    cout << "Finished Test!" << endl;
}

