// MIN Allocator/Collector/Compactor
//
// File:	min_acc.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Thu Jul 30 22:02:21 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/07/31 21:03:07 $
//   $RCSfile: min_acc.cc,v $
//   $Revision: 1.4 $

// Table of Contents:
//
//	Setup
//	Initializer
//	Stub Allocator
//	Block Allocator
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

// Get and set parameter.  Return false if nothing
// done and true if parameter set.  Announce error
// and exit program if parameter too small, too
// large, or not a power of two when that is required.
//
static bool get_param
	( const char * name, min::int64 & parameter,
	  min::int64 minimum = 0,
	  min::int64 maximum = 0x7FFFFFFFFFFFFFFFll,
	  bool power_of_two = false )
{
    const char * s = MOS::get_parameter ( name );
    if ( s == NULL ) return false;

    char * p;
    min::int64 v = ::strtoll ( s, & p, 10 );
    if ( * p && ! isspace ( * p ) )
    {
	const char * q;
        for ( q = s; * q && ! isspace ( * q ); ++ q );
        cout << "ERROR: bad " << name
	     << " program parameter value: "
	     << setw ( q - s ) << s 
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
    parameter = v;
    return true;
}

// Ditto but for `min::uns64' parameter.  maximum
// may not be larger than 2**63 - 1.
//
static bool get_param
	( const char * name, min::uns64 & parameter,
	  min::uns64 minimum = 0,
	  min::uns64 maximum = 0x7FFFFFFFFFFFFFFFull,
	  bool power_of_two = false )
{
    assert ( maximum <= 0x7FFFFFFFFFFFFFFFull );
    min::int64 v;
    if ( ! get_param ( name, v,
                       minimum, maximum,
		       power_of_two ) )
         return false;
    parameter = (min::uns64) v;
    return true;
}

// Ditto but for `unsigned' parameter.
//
static bool get_param
	( const char * name, unsigned & parameter,
	  unsigned minimum = 0,
	  unsigned maximum = unsigned ( -1 ),
	  bool power_of_two = false )
{
    min::int64 v;
    if ( ! get_param ( name, v,
                       minimum, maximum,
		       power_of_two ) )
         return false;
    parameter = (unsigned) v;
    return true;
}

// Convert a size in number of bytes into a size in
// number of pages, rounding up.
//
static min::uns64 pagesize;
inline min::uns64 number_of_pages
    ( min::uns64 number_of_bytes )
{
    return ( number_of_bytes + pagesize - 1 )
           / pagesize;
}

static void stub_allocator_initializer ( void );
static void block_allocator_initializer ( void );
static void collector_initializer ( void );
void MINT::acc_initializer ( void )
{
    pagesize = MOS::pagesize();

    stub_allocator_initializer();
    collector_initializer();
    block_allocator_initializer();
}


// Stub Allocator
// ---- ---------

min::uns64 MACC::max_stubs =
    MIN_DEFAULT_MAX_STUBS;
unsigned MACC::stub_increment =
    MIN_DEFAULT_STUB_INCREMENT;
min::stub * MACC::stub_begin;
min::stub * MACC::stub_next;
min::stub * MACC::stub_end;

static void stub_allocator_initializer ( void )
{
    get_param ( "max_stubs",
                MACC::max_stubs );
    get_param ( "stub_increment",
                MACC::stub_increment, 100 );

    void * stubs = MOS::new_pool_below
        ( number_of_pages ( 16 * max_stubs ),
	  (void *) MIN_MAX_ABSOLUTE_STUB_ADDRESS );

    const char * error = MOS::pool_error ( stubs );
    if ( error != NULL )
    {
        cout << "ERROR: " << error << endl
	     << "       while allocating vector to"
	        " hold" << max_stubs << " stubs"
		" below address "
	     << (min::uns64)
	        MIN_MAX_ABSOLUTE_STUB_ADDRESS
	     << endl;
	exit ( 1 );
    }

    MACC::stub_begin = (min::stub *) stubs;
    MACC::stub_next = MACC::stub_begin;
    MACC::stub_end = MACC::stub_begin + max_stubs;

    MINT::null_stub = MACC::stub_next ++;
    MUP::set_control_of
        ( MINT::null_stub,
	  MUP::new_acc_control
	      ( min::DEALLOCATED,
	        MINT::null_stub ) );

    MINT::last_allocated_stub = MINT::null_stub;

}

void MINT::acc_expand_stub_free_list ( unsigned n )
{
}


// Block Allocator
// ----- ---------

unsigned MACC::space_factor;

static void block_allocator_initializer ( void )
{
    get_param ( "space_factor",
                MACC::space_factor, 8,
		( MIN_POINTER_BITS <= 32 ? 32 : 256 ),
		true );
}


// Collector
// ---------

unsigned MACC::ephemeral_levels =
    MIN_MAX_EPHEMERAL_LEVELS;

static void collector_initializer ( void )
{
    get_param ( "ephemeral_levels",
                MACC::ephemeral_levels,
		0, MIN_MAX_EPHEMERAL_LEVELS );
}
