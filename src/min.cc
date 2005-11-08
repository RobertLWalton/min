// MIN Language Out-of-Line Code
//
// File:	min.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Nov  8 00:43:26 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/08 06:30:40 $
//   $RCSfile: min.cc,v $
//   $Revision: 1.1 $

// Table of Contents:
//
//	Setup
//	Process Interface
//	Garbage Collector Interface
//	Numbers
//	Strings
//	Labels
//	Objects

// Setup
// -----

# include "../include/min.h"


// Stub Functions
// ---- ---------

void min::deallocate ( min::stub * s )
    { }


// Process Interface
// ------- ---------

// Out of line function to execute interrupt.
// Returns true.
//
bool min::unprotected::interrupt ( void )
	{ return true; }


// Garbage Collector Interface
// ------- --------- ---------

# if MIN_IS_COMPACT
    min::uns32 min::unprotected::gc_stub_expand_free_list
	    ( void )
# else // if MIN_IS_LOOSE
    min::uns64 min::unprotected::gc_stub_expand_free_list
	    ( void )
# endif
	    { return 0; }

min::unprotected::body_control *
    min::unprotected::gc_body_stack_expand
	( unsigned n )
        { return 0; }

// Numbers
// -------

# if MIN_IS_COMPACT
    min::gen min::unprotected::new_num_stub_gen
	    ( min::float64 v )
	    { return 0; }
# endif

min::uns32 min::floathash ( min::float64 f )
{
    min::uns32 hash = 0;
    unsigned char * p = (unsigned char *) & f;
    int size = 8;
#   if MIN_IS_LITTLE_ENDIAN
	p += 8;
#   endif
    while ( -- size )
    {
#	if MIN_IS_BIG_ENDIAN
	    hash = ( hash * 65599 ) + * p ++;
#	else // if MIN_IS_LITTLE_ENDIAN
	    hash = ( hash * 65599 ) + * -- p;
#	endif
    }
    return hash;
}

// Strings
// -------

min::uns32 min::strhash
	( const char * p, unsigned size )
{
    min::uns32 hash = 0;
    const unsigned char * q = (const unsigned char *) p;
    while ( size -- )
    {
        hash = ( hash * 65599 ) + * q ++;
    }
    if ( hash == 0 ) hash = 0xFFFFFFFF;
}

// Labels
// ------


// Objects
// -------


// Numbers
// -------


// TBD
// ---
