// MIN Language Out-of-Line Code
//
// File:	min.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Fri Nov 11 21:58:47 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/12 02:58:34 $
//   $RCSfile: min.cc,v $
//   $Revision: 1.5 $

// Table of Contents:
//
//	Setup
//	Process Management
//	Garbage Collector Management
//	Numbers
//	Strings
//	Labels
//	Objects

// Setup
// -----

# include "../include/min.h"
# define MUP min::unprotected

// Stub Functions
// ---- ---------

min::uns32 min::hash ( min::gen v )
{
    if ( min::is_num ( v ) )
        return min::numhash ( v );
    else if ( min::is_str ( v ) )
        return min::strhash ( v );
    else if ( min::is_lab ( v ) )
        return min::labhash ( v );
    else
	return 0;
}

void min::deallocate ( min::stub * s )
    { }


// Process Management
// ------- ----------

// Out of line function to execute interrupt.
// Returns true.
//
bool min::unprotected::interrupt ( void )
	{ return true; }


// Garbage Collector Management
// ------- --------- ----------

// Data.

// Hash tables for atoms.
//
min::stub ** string_hash;
min::stub ** number_hash;
min::stub ** label_hash;
unsigned string_hash_size;
unsigned number_hash_size;
unsigned label_hash_size;

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
    {
	unsigned hash = floathash ( v );
	unsigned h = hash % number_hash_size;
	min::stub * s = number_hash[h];
	while ( s )
	{
	    if ( MUP::float_of ( s ) == v )
		return min::new_gen ( s );
	    s = (min::stub *)
	        MUP::pointer_of_control
		    ( MUP::control_of ( s ) );
	}

	s = MUP::new_stub ();
	MUP::set_float_of ( s, v );
	MUP::set_control_of
	    ( s,
	      MUP::stub_control ( min::NUMBER,
				  gc_new_stub_flags,
				  number_hash[h] ));
	number_hash[h] = s;
	return min::new_gen ( s );
    }
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
{
    unsigned length = ::strlen ( p );
    unsigned hash = strhash ( p, length );
    unsigned h = hash % string_hash_size;
    min::stub * s = string_hash[h];
    while ( s )
    {
        if (    length <= 8
	     && min::type_of ( s ) == min::SHORT_STR
	     && strncmp ( p, s->v.c8, 8 ) == 0 )
	    return min::new_gen ( s );
	else if (    length > 8
	          && min::type_of ( s ) == min::LONG_STR
	          && strcmp
		       ( p, MUP::str_of (
			      MUP::long_str_of ( s ) ) )
		     == 0 )
	    return min::new_gen ( s );
	s = (min::stub *)
	    MUP::pointer_of_control
		( MUP::control_of ( s ) );
    }

    s = MUP::new_stub ();
    int type;
    if ( length <= 8 )
    {
	type = min::SHORT_STR;
	::strncpy ( s->v.c8, p, 8 );
    }
    else
    {
	type = min::LONG_STR;
	MUP::body_control * b = MUP::new_body
	    ( sizeof ( min::long_str ) + length + 1 );
	b->control = MUP::pointer_to_uns64 ( s );
	s->v.u64 = MUP::pointer_to_uns64 ( b + 1 );
	min::long_str * ls = (min::long_str *) ( b + 1 );
	ls->length = length;
	ls->hash = hash;
	::strcpy ( MUP::writable_str_of ( ls ), p );
    }
    MUP::set_control_of
	( s,
	  MUP::stub_control ( type,
			      gc_new_stub_flags,
			      string_hash[h] ));
    string_hash[h] = s;
    return min::new_gen ( s );
}

// Labels
// ------

// 65599**8:
//
const min::uns32 lab_multiplier =
	  min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 );

min::uns32 labhash ( const min::gen * p, unsigned n )
{
    min::uns32 m = 1;
    min::uns32 hash = 0;
    while ( n -- )
    {
        assert ( min::is_atom ( * p ) );
        hash = m * hash + min::hash ( * p ++ );
	m *= lab_multiplier;
    }
    return hash;
}

min::uns32 labhash ( min::stub * s )
{
    assert ( min::type_of ( s ) == min::LABEL );
    min::uns32 m = 1;
    min::uns32 hash = 0;
    min::stub * p = (min::stub *)
        MUP::uns64_to_pointer ( MUP::value_of ( s ) );
    while ( p )
    {
        hash = m * hash
	     + min::hash
		    ( (min::gen) MUP::value_of ( p ) );
	m *= lab_multiplier;
        p = (min::stub *)
	    MUP::pointer_of_control
		( MUP::control_of ( p ) );
    }
    return hash;
}

min::gen new_gen ( const min::gen * p, unsigned n )
{
    unsigned hash = labhash ( p, n );
    unsigned h = hash % label_hash_size;
    min::stub * s = label_hash[h];
    while ( s )
    {
	assert ( min::type_of ( s ) == min::LABEL );
	min::stub * q = (min::stub *)
	    MUP::uns64_to_pointer ( MUP::value_of ( s ) );
	unsigned i = 0;
	while ( q )
	{
	    if ( i >= n ) break;
	    if (    p[i++]
	         != (min::gen) MUP::value_of ( q ) )
	        break;
	    q = (min::stub *)
		MUP::pointer_of_control
		    ( MUP::control_of ( q ) );
	}
	if ( q == NULL && i >= n )
	    return min::new_gen ( s );
	s = (min::stub *)
	    MUP::pointer_of_control
		( MUP::control_of ( s ) );
    }

    s = MUP::new_stub ();
    if ( n == 0 )
        MUP::set_value_of ( s, 0 );
    else
    {
        min::stub * lastq = NULL;
	for ( unsigned i = 0; i < n; ++ i )
	{
	    min::stub * q = MUP::new_stub ();
	    MUP::set_gen_of ( q, p[i] );
	    if ( lastq )
		MUP::set_control_of
		    ( s, MUP::stub_control
		           ( min::LABEL_AUX, 0,
			     lastq ) );
	    else
		MUP::set_value_of
		    ( s, MUP::pointer_to_uns64 ( q ) );
	    lastq = q;
	}
	MUP::set_control_of
	    ( lastq, MUP::stub_control
	    	         ( min::LABEL_AUX, 0 ) );
    }
    MUP::set_control_of
	( s,
	  MUP::stub_control ( min::LABEL,
			      MUP::gc_new_stub_flags,
			      label_hash[h] ));
    label_hash[h] = s;
    return min::new_gen ( s );
}


// Objects
// -------


// Numbers
// -------


// TBD
// ---
