// MIN Language Out-of-Line Code
//
// File:	min.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Nov 22 06:30:29 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/22 11:44:28 $
//   $RCSfile: min.cc,v $
//   $Revision: 1.17 $

// Table of Contents:
//
//	Setup
//	Process Management
//	Garbage Collector Management
//	Numbers
//	Strings
//	Labels
//	Objects
//	Object List Level

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
    min::uns32 min::unprotected::
                    gc_stub_expand_free_list ( void )
# else // if MIN_IS_LOOSE
    min::uns64 min::unprotected::
                    gc_stub_expand_free_list ( void )
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
	        MUP::stub_p_of_control
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
	    MUP::stub_p_of_control
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
	    ( sizeof ( MUP::long_str ) + length + 1 );
	b->control = MUP::pointer_to_uns64 ( s );
	s->v.u64 = MUP::pointer_to_uns64 ( b + 1 );
	MUP::long_str * ls =
	    (MUP::long_str *) ( b + 1 );
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
	    MUP::stub_p_of_control
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
	    MUP::uns64_to_pointer
	        ( MUP::value_of ( s ) );
	unsigned i = 0;
	while ( q )
	{
	    if ( i >= n ) break;
	    if (    p[i++]
	         != (min::gen) MUP::value_of ( q ) )
	        break;
	    q = (min::stub *)
		MUP::stub_p_of_control
		    ( MUP::control_of ( q ) );
	}
	if ( q == NULL && i >= n )
	    return min::new_gen ( s );
	s = (min::stub *)
	    MUP::stub_p_of_control
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



// Object List Level
// ------ ---- -----

// Allocate n consecutive aux cells and return the index
// of the first such cell.  If there are insufficient
// cells available, return 0 is list auxiliary stubs can
// be used, and proclaim an assert violation otherwise.
//
inline unsigned MUP::allocate_aux_list
	( min::unprotected::list_pointer & lp,
	  unsigned n)
{
    unsigned unused_area_offset;
    unsigned aux_area_offset;
    if ( lp.so )
    {
	unused_area_offset =
	    lp.so->unused_area_offset;
	aux_area_offset =
	    lp.so->aux_area_offset;
    }
    else
    {
	unused_area_offset =
	    lp.lo->unused_area_offset;
	aux_area_offset =
	    lp.lo->aux_area_offset;
    }
#   if MIN_USES_LIST_AUX_STUBS
	if (   unused_area_offset + n
	     > aux_area_offset )
	    return 0;
#   else
	assert (    unused_area_offset + n
	         <= aux_area_offset );
#   endif
    aux_area_offset -= n;
    if ( lp.so )
    {
	lp.so->aux_area_offset =
	    aux_area_offset;
    }
    else
    {
	lp.lo->aux_area_offset =
	    aux_area_offset;
    }
    return aux_area_offset;
}

// Allocate a chain of stubs containing the n min::gen
// values in p.  The type of the first stub is given
// and the other stubs have type min::LIST_AUX.  Each
// stub but the last points at the next stub.  The
// control of the last contains the end value, which
// may be a list aux value or a pointer to a stub.
// This function returns a pointer to the first stub
// allocated, or returns `end' if n == 0.
//
// This function asserts that the relocated flag is
// off both before and after any stub allocations
// this function performs.  Sufficient stubs should
// have been reserved in advance.
//
min::gen MUP::allocate_stub_list
	( int type, const min::gen * p, unsigned n,
	  min::uns64 end )
{
    if ( n == 0 )
    {
	if (   MUP::flags_of_control ( end )
	     & MUP::LIST_AUX_STUB )
	    return min::new_gen
		( MUP::stub_p_of_control ( end ) );
	else
	    return min::new_list_aux_gen
	    	( MUP::value_of_control ( end ) );
    }

    // Check for failure to use min::insert_reserve
    // properly.
    //
    assert ( ! relocated_flag () );

    min::stub * first = MUP::new_stub ();
    MUP::set_gen_of ( first, * p ++ );
    min::stub * previous = first;
    min::stub * last = first;
    while ( -- n )
    {
	min::stub * last = MUP::new_stub ();
	MUP::set_gen_of ( last, * p ++ );
	MUP::set_control_of
	     ( previous,
	       MUP::stub_control
	           ( type, MUP::LIST_AUX_STUB, last ) );
	type = MUP::LIST_AUX_STUB;
	previous = last;
    }

    // Check for failure to use min::insert_reserve
    // properly.
    //
    assert ( ! relocated_flag () );
    MUP::set_control_of ( last, end );
    MUP::set_type_of ( last, type );

    return min::new_gen ( first );
}

// Copy n elements from the vector at p to the vector at
// q, reversing the order of the elements.  Check that
// none of the copied elements are list or sublist
// pointers.
//
inline void copy_elements
	( min::gen * q, const min::gen * p, unsigned n )
{
    q += n;
    while ( n -- )
    {
	assert (    ! min::is_list_aux ( * p )
		 && ! min::is_sublist_aux ( * p ) );
	* -- q = * p ++;
    }
}



void min::unprotected::insert_reserve
	( min::unprotected::list_pointer & lp,
	  unsigned insertions,
	  unsigned elements,
	  bool use_aux )
{
}

void min::insert_before
	( min::unprotected::list_pointer & lp,
	  const min::gen * p, unsigned n )
{
    if (    lp.current_index == 0
#	 if MIN_USES_LIST_AUX_STUBS
	 && lp.current_stub == NULL
#	 endif
       )
    {
        // Inserting before a min::LIST_END that
	// is not a specific auxilary element is
	// treated as inserting after the previous
	// element.
        assert ( lp.current == min::LIST_END );
        lp.current_index = lp.previous_index;
#	if MIN_USES_LIST_AUX_STUBS
	    lp.current_stub = lp.previous_stub;
#	endif
	lp.current = 0;
	min::insert_after ( lp, p, n );
	return;
    }

    assert ( lp.reserved_insertions >= 1 );
    assert ( lp.reserved_elements >= n );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    unsigned aux_area_offset;
    if ( lp.so )
	aux_area_offset = lp.so->aux_area_offset;
    else
	aux_area_offset = lp.lo->aux_area_offset;

    if ( lp.current == min::LIST_END )
    {
	assert ( lp.current_index != 0 );

	bool contiguous =
	    ( lp.current_index == aux_area_offset );
	unsigned index =
	    MUP::allocate_aux_list
	    	( lp, n + ! contiguous );
#	if MIN_USES_LIST_AUX_STUBS
	    if ( index == 0 )
	    {
	    	lp.base[lp.current_index] =
		    MUP::allocate_stub_list
		        ( min::LIST_AUX,
			  p, n,
			  MUP::stub_control
			      ( 0, 0, (unsigned) 0 ) );
		return;
	    }
#	endif
	copy_elements ( lp.base + index + 1, p, n );
	lp.base[index] = min::LIST_END;
	if ( ! contiguous )
	    lp.base[lp.current_index] =
	        min::new_list_aux_gen ( index + n );
	lp.current_index = index;
	lp.previous_index = 0;
	return;
    }

    bool previous = ( lp.previous_index != 0 );
#   if MIN_USES_LIST_AUX_STUBS
	if ( lp.previous_stub != NULL ) previous = true;
#   endif

    unsigned index = MUP::allocate_aux_list
    	( lp, n + 1 + ! previous );

#   if MIN_USES_LIST_AUX_STUBS
	if ( index == 0 )
	{
	    lp.base[lp.current_index] =
		MUP::allocate_stub_list
		    ( min::LIST_AUX,
		      p, n, min::LIST_END );
	    return;
	}
#   endif

    copy_elements ( lp.base + index + 1, p, n );

#   if MIN_USES_LIST_AUX_STUBS
    if ( lp.current_stub != NULL )
        lp.base[index] =
	    min::new_gen ( lp.current_stub );
	    // TBD What about type of current stub
    else
#   endif
        lp.base[index] =
	    min::new_list_aux_gen
	        ( lp.current_index - ! previous );

    if ( lp.previous_index != 0 )
	lp.base[lp.previous_index] =
	    min::unprotected::new_aux_gen
		( lp.base[lp.previous_index],
		  index + n + 1 );
#   if MIN_USES_LIST_AUX_STUBS
	else if ( lp.previous_stub != NULL )
	    MUP::set_control_of
	        ( lp.previous_stub,
		  MUP::stub_control
		      ( min::type_of
		            ( lp.previous_stub ),
			0, index + n ) );
#   endif
    else
    {
	lp.base[index + n + 1] = lp.current;
	lp.base[lp.current_index] =
	    min::new_list_aux_gen ( index + n + 1 );
	lp.previous_index = lp.current_index;
	lp.current_index = index + n + 1;
    }
}

void min::insert_after
	( min::unprotected::list_pointer & lp,
	  const min::gen * p, unsigned n )
{
    assert ( lp.current != min::LIST_END );

    assert ( lp.reserved_insertions >= 1 );
    assert ( lp.reserved_elements >= n );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

#   if MIN_USES_LIST_AUX_STUBS
	bool in_aux = ( lp.current_index != 0 );
#   else
	bool in_aux = true;
#   endif

    unsigned index =
        MUP::allocate_aux_list ( lp, n + 1 + in_aux );

    copy_elements ( lp.base + index + 1, p, n );

    if ( in_aux )
    {

	unsigned unused_area_offset;
	if ( lp.so )
	    unused_area_offset =
	        lp.so->unused_area_offset;
	else
	    unused_area_offset =
	        lp.lo->unused_area_offset;

	if ( lp.current_index < unused_area_offset )
	    lp.base[index] = min::LIST_END;
	else if ( min::is_list_aux
	              ( lp.base[lp.current_index - 1] ) )
	    lp.base[index] =
	        lp.base[lp.current_index - 1];
	else
	    lp.base[index] =
	        min::unprotected::new_list_aux_gen
			( lp.current_index - 1 );

        lp.base[index + n + 1] = lp.current;
	lp.base[lp.current_index] =
	    min::unprotected::new_list_aux_gen
		    ( index + n + 1 );
	lp.forward ( lp.current_index );
    }
#   if MIN_USES_LIST_AUX_STUBS
    else
    {
    	assert ( lp.current_stub != NULL );
    }
#   endif
}


// Numbers
// -------


// TBD
// ---
