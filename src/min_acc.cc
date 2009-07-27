// MIN Allocator/Collector/Compactor
//
// File:	min_acc.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Jul 27 05:57:12 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/07/27 13:01:17 $
//   $RCSfile: min_acc.cc,v $
//   $Revision: 1.1 $

// Table of Contents:
//
//	Setup
//	Initializer
//	Stub Allocator
//	Block Allocator
//	Scavenger
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

static void get_param
	( const char * name, min::uns64 & parameter )
{
    const char * s = MOS::get_parameter ( name );
    if ( s == NULL ) return;
    const char * p;
    min::uns64 v = ::strtoull ( s, & p, 10 );
    if ( * p && ! isspace ( * p ) )
    {
        for ( p = s; * p && ! isspace ( * p ); ++ p );
        cout << "ERROR: bad " << name
	     << " program parameter value: "
	     << setw ( p - s ) << s 
	     << endl;
	exit ( 1 );
    }
    parameter = v;
}
    

MINT::acc_initializer ( void )
{
    get_param ( "max_stubs", max_stubs );
}


// Stub Allocator
// ---- ---------


// Block Allocator
// ----- ---------


// Scavenger
// ---------


// Collector
// ---------

