// MIN Allocator/Collector/Compactor
//
// File:	min_acc.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Jul 28 13:53:11 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/07/28 17:53:19 $
//   $RCSfile: min_acc.cc,v $
//   $Revision: 1.2 $

// Table of Contents:
//
//	Setup
//	Initializer
//	Stub Allocator
//	Block Allocator
//	Collector

// Setup
// -----

# include <min.h>
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

    const char * p;
    min::int64 v = ::strtoll ( s, & p, 10 );
    if ( * p && ! isspace ( * p ) )
    {
        for ( p = s; * p && ! isspace ( * p ); ++ p );
        cout << "ERROR: bad " << name
	     << " program parameter value: "
	     << setw ( p - s ) << s 
	     << endl;
	exit ( 1 );
    }
    if ( v < minimum )
    {
        cout << "ERROR: " << name
	     << " program parameter value too small: "
	     << v << " < " minimum << endl;
	exit ( 1 );
    }
    if ( v > maximum )
    {
        cout << "ERROR: " << name
	     << " program parameter value too large: "
	     << v << " > " maximum << endl;
	exit ( 1 );
    }
    if ( power_of_type
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


static void stub_allocator_initializer ( void );
static void block_allocator_initializer ( void );
static void collector_initializer ( void );
MINT::acc_initializer ( void )
{
    stub_allocator_initializer();
    collector_initializer();
    block_allocator_initializer();
}


// Stub Allocator
// ---- ---------

min::uns64 MACC::max_stubs;
unsigned MACC::stub_increment;

static void stub_allocator_initializer ( void )
{
    get_param ( "max_stubs",
                MACC::max_stubs );
    get_param ( "stub_increment",
                MACC::stub_increment, 100 );
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

unsigned MACC::ephemeral_levels;

static void collector_initializer ( void )
{
    get_param ( "ephemeral_levels",
                MACC::ephemeral_levels,
		0, MIN_MAX_EPHEMERAL_LEVELS );
}
