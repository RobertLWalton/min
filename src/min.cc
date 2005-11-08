// MIN Language Out-of-Line Code
//
// File:	min.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Nov  8 03:07:25 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/08 14:15:41 $
//   $RCSfile: min.cc,v $
//   $Revision: 1.2 $

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

unsigned min::strlen ( min::gen v )
{
    if ( min::is_direct_str ( v ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = min::direct_str_of ( v );
	return ::strlen ( u.buf );
    }
    else
    {
        return min::strlen ( min::stub_of ( v ) );
    }
}

min::uns32 min::strhash ( min::gen v )
{
    if ( min::is_direct_str ( v ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = min::direct_str_of ( v );
	return min::strhash
	    ( u.buf, ::strlen ( u.buf ) );
    }
    else
    {
        return min::strhash ( min::stub_of ( v ) );
    }
}

char * min::strcpy ( char * p, min::gen v )
{
    if ( min::is_direct_str ( v ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = min::direct_str_of ( v );
	return ::strcpy ( p, u.buf );
    }
    else
    {
        return min::strcpy ( p, min::stub_of ( v ) );
    }
}

char * min::strncpy ( char * p, min::gen v, unsigned n )
{
    if ( min::is_direct_str ( v ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = min::direct_str_of ( v );
	return ::strncpy ( p, u.buf, n );
    }
    else
    {
        return min::strncpy
	    ( p, min::stub_of ( v ), n );
    }
}

min::gen min::unprotected::new_str_stub_gen
	( const char * p )
	{ return 0; }

// Labels
// ------


// Objects
// -------


// Numbers
// -------


// TBD
// ---
