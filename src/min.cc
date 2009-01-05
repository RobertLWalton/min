// MIN Language Out-of-Line Code
//
// File:	min.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Jan  5 00:06:48 EST 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/01/05 06:08:33 $
//   $RCSfile: min.cc,v $
//   $Revision: 1.62 $

// Table of Contents:
//
//	Setup
//	Initialization
//	Stub Functions
//	Process Management
//	Garbage Collector Management
//	Numbers
//	Strings
//	Labels
//	Objects
//	Object List Level

// Setup
// -----

# include <min.h>
# define MUP min::unprotected
# define MINT min::internal

// For debugging.
//
# include <iostream>
# include <iomanip>
# include <cmath>
using std::hex;
using std::dec;
using std::cout;
using std::endl;

// Initialization
// --------------

static bool initializer_done = false;
MINT::initializer::initializer ( void )
{
    if ( initializer_done ) return;

    assert ( sizeof ( MIN_INT32_TYPE ) == 4 );
    assert ( sizeof ( MIN_INT64_TYPE ) == 8 );
    assert
        ( sizeof ( void * ) == MIN_POINTER_BITS / 8 );

    min::uns32 u = 1;
    char * up = (char *) & u;
    bool big_endian = ( up[3] == 1 );
    bool little_endian = ( up[0] == 1 );
    assert ( MIN_IS_BIG_ENDIAN == big_endian );
    assert ( MIN_IS_LITTLE_ENDIAN == little_endian );

#   if MIN_IS_LOOSE 

	// Tests of MIN_FLOAT64_SIGNALLING_NAN
	//
	min::gen missing = min::MISSING;
	min::float64 v = * (min::float64 *) & missing;

	assert ( isnan ( v ) );

	// Attemps to get any kind of NaN to raise an
	// exception failed, so we cannot test for that.

	// However, we can test that hardware does not
	// generate non-signalling NaNs with high order
	// 16 bits identical to v.

	min::float64 v2 = v + 1.0;

	assert ( isnan ( v2 ) );
	min::uns16 * vp = (min::uns16 *) & v;
	min::uns16 * v2p = (min::uns16 *) & v2;
	assert ( vp[3*little_endian]
		 !=
		 v2p[3*little_endian] );

	v2 = 0.0;
	v2 = v2 / v2;
	assert ( isnan ( v2 ) );
	assert ( vp[3*little_endian]
		 !=
		 v2p[3*little_endian] );
#   endif

    initializer_done = true;
}

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

namespace min { namespace unprotected {

    bool interrupt_flag;
    bool relocated_flag;
    process_control * current_process;

} }

// Garbage Collector Management
// ------- --------- ----------

// Data.

namespace min { namespace unprotected {

    min::uns64 gc_stack_marks;
    min::uns64 gc_new_stub_flags;
    min::stub ** gc_stack;
    min::stub ** gc_stack_end;
    min::stub * last_allocated_stub;
    unsigned number_of_free_stubs;
    body_control * free_body_control;

    min::stub ** str_hash;
    unsigned str_hash_size;

    min::stub ** num_hash;
    unsigned num_hash_size;

    min::stub ** lab_hash;
    unsigned lab_hash_size;

} }


// Numbers
// -------

# if MIN_IS_COMPACT
    min::gen MUP::new_num_stub_gen
	    ( min::float64 v )
    {
	unsigned hash = floathash ( v );
	unsigned h = hash % MUP::num_hash_size;
	min::stub * s = MUP::num_hash[h];
	while ( s )
	{
	    if ( MUP::float_of ( s ) == v )
		return min::new_gen ( s );
	    s = (min::stub *)
	        MUP::stub_of_gc_control
		    ( MUP::control_of ( s ) );
	}

	s = MUP::new_aux_stub ();
	MUP::set_float_of ( s, v );
	MUP::set_control_of
	    ( s,
	      MUP::new_gc_control
	          ( min::NUMBER, MUP::num_hash[h],
		    gc_new_stub_flags ));
	MUP::num_hash[h] = s;
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
    while ( size -- )
    {
#	if MIN_IS_BIG_ENDIAN
	    hash = ( hash * 65599 ) + * p ++;
#	elif MIN_IS_LITTLE_ENDIAN
	    hash = ( hash * 65599 ) + * -- p;
#	endif
    }
    return hash;
}

// Strings
// -------

min::uns32 min::strnhash
	( const char * p, unsigned size )
{
    min::uns32 hash = 0;
    const unsigned char * q = (const unsigned char *) p;
    unsigned char c;
    while ( size -- && ( c = * q ++ ) )
    {
        hash = ( hash * 65599 ) + c;
    }
    if ( hash == 0 ) hash = 0xFFFFFFFF;
    return hash;
}

min::uns32 min::strhash ( const char * p )
{
    min::uns32 hash = 0;
    const unsigned char * q = (const unsigned char *) p;
    unsigned char c;
    while ( c = * q ++ )
    {
        hash = ( hash * 65599 ) + c;
    }
    if ( hash == 0 ) hash = 0xFFFFFFFF;
    return hash;
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
	return min::strhash ( u.buf );
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

// Perform the new_str_stub_gen operation where n is the
// exact number of characters in p that are to be used
// (instead of the maximum).  There must be no NULs in
// the first n characters of p.
//
min::gen MUP::new_str_stub_gen_internal
	( const char * p, unsigned n )
{
    unsigned hash = min::strnhash ( p, n );
    unsigned h = hash % MUP::str_hash_size;
    min::stub * s = MUP::str_hash[h];
    const char * q;
    while ( s )
    {
        if (    n <= 8
	     && min::type_of ( s ) == min::SHORT_STR
	     && strncmp ( p, s->v.c8, n ) == 0
	     && (    n == 8
	          || s->v.c8[n] == 0 ) )
	    return min::new_gen ( s );
	else if (    n > 8
	          && min::type_of ( s ) == min::LONG_STR
	          && strncmp
		       ( p, q = MUP::str_of (
			            MUP::long_str_of
				        ( s ) ),
			    n )
		     == 0
		  && q[n] == 0 )
	    return min::new_gen ( s );
	s = (min::stub *)
	    MUP::stub_of_gc_control
		( MUP::control_of ( s ) );
    }

    s = MUP::new_aux_stub ();
    int type;
    if ( n <= 8 )
    {
	type = min::SHORT_STR;
	s->v.u64 = 0;
	::strncpy ( s->v.c8, p, n );
    }
    else
    {
	type = min::LONG_STR;
	MUP::body_control * b = MUP::new_body
	    ( sizeof ( MUP::long_str ) + n + 1 );
	b->control = MINT::pointer_to_uns64 ( s );
	s->v.u64 = MINT::pointer_to_uns64 ( b + 1 );
	MUP::long_str * ls =
	    (MUP::long_str *) ( b + 1 );
	ls->length = n;
	ls->hash = hash;
	::strcpy ( MUP::writable_str_of ( ls ), p );
    }
    MUP::set_control_of
	( s,
	  MUP::new_gc_control
	      ( type, MUP::str_hash[h],
	        MUP::gc_new_stub_flags ));
    MUP::str_hash[h] = s;
    return min::new_gen ( s );
}

// Labels
// ------

// 65599**8:
//
const min::uns32 lab_multiplier =	// 65599**10
	  min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 )
	* min::uns32 ( 65599 );

min::uns32 min::labhash
	( const min::gen * p, unsigned n )
{
    min::uns32 hash = 0;
    while ( n -- )
    {
        MIN_ASSERT ( min::is_atom ( * p ) );
        hash = lab_multiplier * hash
	     + min::hash ( * p ++ );
    }
    return hash;
}

min::uns32 min::labhash ( min::stub * s )
{
    MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
    min::uns64 c = MUP::value_of ( s );
    min::uns32 hash = 0;
    while ( true )
    {
	s = MUP::stub_of_control ( c );
	if ( s == NULL ) break;
        hash = lab_multiplier * hash
	     + min::hash ( MUP::gen_of ( s ) );
	c = MUP::control_of ( s );
    }
    return hash;
}

min::gen min::new_lab_gen
	( const min::gen * p, unsigned n )
{
    unsigned hash = labhash ( p, n );
    unsigned h = hash % MUP::lab_hash_size;
    min::stub * s = MUP::lab_hash[h];
    while ( s )
    {
	MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
	min::uns64 c = MUP::value_of ( s );
	unsigned i = 0;
	while ( true )
	{
	    min::stub * aux =
	        MUP::stub_of_control ( c );
	    if ( aux == NULL )
	    {
	        if ( i == n )
		    return min::new_gen ( s );
		else
		    break;
	    }
	    if ( i >= n ) break;
	    if ( p[i++] != MUP::gen_of ( aux ) )
	        break;
	    c = MUP::control_of ( aux );
	}
	s = MUP::stub_of_gc_control
		( MUP::control_of ( s ) );
    }

    s = MUP::new_aux_stub ();
    if ( n == 0 )
        MUP::set_value_of ( s, 0 );
    else
    {
        min::stub * lastq = NULL;
	for ( unsigned i = 0; i < n; ++ i )
	{
	    min::stub * q = MUP::new_aux_stub ();
	    MUP::set_gen_of ( q, p[i] );
	    min::uns64 c = MUP::new_control
		             ( min::LABEL_AUX, q );
	    if ( lastq )
		MUP::set_control_of ( lastq, c );
	    else
		MUP::set_value_of ( s, c );
	    lastq = q;
	}
	MUP::set_control_of
	    ( lastq, MUP::new_control
	    	         ( min::LABEL_AUX,
			   min::uns64 (0) ) );
    }
    MUP::set_control_of
	( s,
	  MUP::new_gc_control
	      ( min::LABEL, MUP::lab_hash[h],
	        MUP::gc_new_stub_flags ));
    MUP::lab_hash[h] = s;
    return min::new_gen ( s );
}

// Objects
// -------

namespace min { namespace unprotected {

    min::uns32 hash_table_size
	[HASH_TABLE_SIZE_CODE_SIZE] =
    {
	// Except for the first 2 values, these
	// sizes are primes chosen so that none
	// is greater than 105% of the previous
	// one, where possible.
	//
		0,        1,        2,        3,
		5,        7,       11,       13,
	       17,       19,       23,       29,
	       31,       37,       41,       43,
	       47,       53,       59,       61,
	       67,       71,       73,       79,
	       83,       89,       97,      101,
	      103,      107,      109,      113,
	      127,      131,      137,      139,
	      149,      151,      157,      163,
	      167,      173,      181,      191,
	      199,      211,      223,      233,
	      241,      251,      263,      271,
	      283,      293,      307,      317,
	      331,      347,      359,      373,
	      389,      401,      421,      439,
	      457,      479,      499,      523,
	      547,      571,      599,      619,
	      647,      677,      709,      743,
	      773,      811,      839,      877,
	      919,      953,      997,     1039,
	     1087,     1129,     1181,     1237,
	     1297,     1361,     1429,     1499,
	     1571,     1637,     1709,     1789,
	     1877,     1951,     2039,     2137,
	     2243,     2351,     2467,     2579,
	     2707,     2837,     2971,     3119,
	     3271,     3433,     3593,     3769,
	     3947,     4139,     4339,     4549,
	     4759,     4993,     5237,     5483,
	     5749,     6029,     6329,     6637,
	     6967,     7309,     7673,     8053,
	     8447,     8867,     9293,     9749,
	    10223,    10733,    11261,    11821,
	    12409,    13009,    13649,    14327,
	    15031,    15773,    16561,    17389,
	    18257,    19163,    20117,    21121,
	    22171,    23279,    24439,    25657,
	    26927,    28229,    29633,    31091,
	    32633,    34261,    35969,    37747,
	    39631,    41611,    43691,    45869,
	    48157,    50551,    53077,    55721,
	    58481,    61403,    64453,    67651,
	    71023,    74573,    78301,    82207,
	    86311,    90619,    95143,    99881,
	   104869,   110083,   115571,   121349,
	   127403,   133769,   140453,   147457,
	   154823,   162563,   170689,   179213,
	   188171,   197573,   207443,   217793,
	   228677,   240109,   252101,   264697,
	   277919,   291791,   306377,   321679,
	   337759,   354643,   372371,   390989,
	   410519,   431029,   452579,   475207,
	   498961,   523907,   550073,   577573,
	   606449,   636763,   668599,   702017,
	   737111,   773953,   812641,   853241,
	   895903,   940691,   987713,  1037089,
	  1088933,  1143371,  1200527,  1260551,
	  1323577,  1389749,  1459217,  1532173,
	  1608773,  1689211,  1773671,  1862347,
	  1955417,  2053127,  2155781,  2263561,
	  2376721,  2495557,  2620313,  2751323,
	  2888887,  3033323,  3184969,  3344213,
	  3511421,  3686989,  3871331,  4064881
    };

} }

bool min::use_obj_aux_stubs = false;

unsigned min::short_obj_hash_table_size ( unsigned u )
{
    if ( u >= ( 1 << 16 ) - MUP::short_obj_header_size )
        u = ( 1 << 16 ) - MUP::short_obj_header_size;

    // Invariant:
    //
    //    MUP::hash_table_size[hi] >= u || hi = 255
    //
    int lo = 0, hi = 255;
    if ( u <= 1 ) hi = u;
    else while ( true )
    {
        int mid = ( lo + hi ) / 2;
	if ( MUP::hash_table_size[mid] >= u ) hi = mid;
	else if ( lo == mid ) break;
	else lo = mid;
    }
    min::uns32 size = MUP::hash_table_size[hi];
    if ( size >= ( 1 << 16 )
               - MUP::short_obj_header_size )
	return MUP::hash_table_size[--hi];
    else
	return size;
}

unsigned min::short_obj_total_size ( unsigned u )
{
    if ( u < ( 1 << 16 ) ) return u;
    else return ( 1 << 16 ) - 1;
}

unsigned min::long_obj_hash_table_size ( unsigned u )
{
    // Invariant:
    //
    //    MUP::hash_table_size[hi] >= u || hi = 255
    //
    int lo = 0, hi = 255;
    if ( u <= 1 ) hi = u;
    else while ( true )
    {
        int mid = ( lo + hi ) / 2;
	if ( MUP::hash_table_size[mid] >= u ) hi = mid;
	else if ( lo == mid ) break;
	else lo = mid;
    }
    return MUP::hash_table_size[hi];
}

unsigned min::long_obj_total_size ( unsigned u )
{
    if ( sizeof (min::uns32) < sizeof (unsigned) )
    {
        if ( u < min::uns32(-1) ) return u;
	else return min::uns32(-1);
    } else return u;
}

min::gen min::new_obj_gen
	    ( unsigned hash_table_size,
	      unsigned unused_area_size )
{
    MIN_ASSERT (    hash_table_size
                 <= MUP::hash_table_size[255] );

    // Invariant:
    //
    //    MUP::hash_table_size[hi] >= u || hi = 255
    //
    int lo = 0, hi = 255;
    if ( hash_table_size <= 1 ) hi = hash_table_size;
    else while ( true )
    {
        int mid = ( lo + hi ) / 2;
	if (    MUP::hash_table_size[mid]
	     >= hash_table_size )
	    hi = mid;
	else if ( lo == mid ) break;
	else lo = mid;
    }
    hash_table_size = MUP::hash_table_size[hi];

    MIN_ASSERT (    unused_area_size
                 <=   min::uns32(-1)
		    - hash_table_size
		    - MUP::long_obj_header_size );
    unsigned total_size =
        unused_area_size + hash_table_size;
    min::stub * s = MUP::new_stub();
    min::gen * p;
    if (   total_size + MUP::short_obj_header_size
         < ( 1 << 16 ) )
    {
        total_size += MUP::short_obj_header_size;
	MUP::body_control * bc =
	    MUP::new_body
		( sizeof (min::gen) * total_size );
	bc->control =
	    MUP::new_control ( min::SHORT_OBJ, s );
	MUP::short_obj * so =
	    (MUP::short_obj *) ( bc + 1 );
	MUP::set_pointer_of ( s, so );
	MUP::set_type_of ( s, min::SHORT_OBJ );
	so->flags = hi;
	so->unused_area_offset =
	       MUP::short_obj_header_size
	     + hash_table_size;
	so->aux_area_offset = total_size;
	so->total_size	    = total_size;
	p = (min::gen *) so
	  + MUP::short_obj_header_size;
    }
    else
    {
        total_size += MUP::long_obj_header_size;
	MUP::body_control * bc =
	    MUP::new_body
		( sizeof (min::gen) * total_size );
	bc->control =
	    MUP::new_control ( min::LONG_OBJ, s );
	MUP::long_obj * lo =
	    (MUP::long_obj *) ( bc + 1 );
	MUP::set_pointer_of ( s, lo );
	MUP::set_type_of ( s, min::LONG_OBJ );
	lo->flags = hi;
	lo->unused_area_offset =
	       MUP::long_obj_header_size
	     + hash_table_size;
	lo->aux_area_offset = total_size;
	lo->total_size	    = total_size;
	p = (min::gen *) lo
	  + MUP::long_obj_header_size;
    }
    min::gen * endp = p + hash_table_size;
    while ( p < endp ) * p ++ = min::LIST_END;
    return min::new_gen ( s );
}


// Object List Level
// ------ ---- -----

# if MIN_USES_OBJ_AUX_STUBS

// Allocate a chain of stubs containing the n min::gen
// values in p.  The type of the first stub is given
// and the other stubs have type min::LIST_AUX.  Each
// stub but the last points at the next stub.  The
// control of the last, except for its type field,
// equals the end value, which may be a list aux value
// or a pointer to a stub.
//
// This function returns pointers to the first and last
// stubs allocated.  n > 0 is required.
//
// This function asserts that the relocated flag is
// off both before and after any stub allocations
// this function performs.  Sufficient stubs should
// have been reserved in advance.
//
void MUP::allocate_stub_list
	( min::stub * & first,
	  min::stub * & last,
	  int type,
	  const min::gen * p, unsigned n,
	  min::uns64 end )
{
    MIN_ASSERT ( n > 0 );

    // Check for failure to use min::insert_reserve
    // properly.
    //
    MIN_ASSERT ( ! min::relocated_flag () );

    first = MUP::new_aux_stub ();
    MUP::set_gen_of ( first, * p ++ );
    min::stub * previous = first;
    last = first;
    while ( -- n )
    {
	last = MUP::new_aux_stub ();
	MUP::set_gen_of ( last, * p ++ );
	MUP::set_control_of
	     ( previous,
	       MUP::new_control
	           ( type, last, MUP::STUB_POINTER ) );
	type = min::LIST_AUX;
	previous = last;
    }
    MUP::set_control_of
        ( last, MUP::renew_control_type ( end, type ) );

    // Check for failure to use min::insert_reserve
    // properly.
    //
    MIN_ASSERT ( ! min::relocated_flag () );
}

# endif // MIN_USES_OBJ_AUX_STUBS

void min::insert_before
	( MUP::insertable_list_pointer & lp,
	  const min::gen * p, unsigned n )
{
    unsigned unused_area_offset;
    unsigned aux_area_offset;

    if ( n == 0 ) return;
    else if ( lp.so )
    {
	unused_area_offset = lp.so->unused_area_offset;
	aux_area_offset = lp.so->aux_area_offset;
    }
    else if ( lp.lo )
    {
	unused_area_offset = lp.lo->unused_area_offset;
	aux_area_offset = lp.lo->aux_area_offset;
    }
    else
	MIN_ABORT ( "lp list has not been started" );

    MIN_ASSERT ( lp.reserved_insertions >= 1 );
    MIN_ASSERT ( lp.reserved_elements >= n );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    MUP::gc_write_update ( lp.s, p, n );

    if ( lp.current == min::LIST_END )
    {
	// Contiguous means the previous pointer does
	// not exists and current_index == aux_area_
	// offset so we can add elements by copying them
	// into tha aux area just before current_index.
	//
	bool contiguous = false;

	// Previous_is_list_head means previous is in
	// the hash table or attribute vector.  This
	// can only happen if previous_index != 0 and
	// previous_is_sublist_head is false.
	//
	bool previous_is_list_head = false;

	// Pointer to the first new element; replaces
	// LIST_END in current.
	//
	min::gen fgen;

	if ( lp.previous_index != 0 )
	    previous_is_list_head =
	        ! lp.previous_is_sublist_head;
	else if ( lp.previous_stub == NULL )
	    contiguous =
		( lp.current_index == aux_area_offset );

#	if MIN_USES_OBJ_AUX_STUBS
	    if (    lp.use_obj_aux_stubs
		 &&     unused_area_offset
		      + n + ( ! contiguous )
		      + previous_is_list_head
		    > aux_area_offset )
	    {
	        // Not enough aux area available for
		// all the new elements, and aux stubs
		// are allowed.
		//
		min::stub * first, * last;
		MUP::allocate_stub_list
		    ( first, last,
		      lp.previous_is_sublist_head ?
			  min::SUBLIST_AUX :
		          min::LIST_AUX,
		      p, n,
		      MUP::new_control
			( 0, (min::uns64) 0 ) );

		fgen = min::new_gen ( first );
		if ( lp.previous_stub != NULL )
		{
		    if ( lp.previous_is_sublist_head )
		       MUP::set_gen_of
		           ( lp.previous_stub, fgen );
		    else
		    {
			int type =
			    min::type_of
				( lp.previous_stub );
			MUP::set_control_of
			    ( lp.previous_stub,
			      MUP::new_control
				  ( type, first,
				    MUP::STUB_POINTER )
			    );
		    }
		}
		else if ( lp.previous_index != 0 )
		{
		    if ( previous_is_list_head )
		    {
		        min::stub * s =
			    MUP::new_aux_stub();
			MUP::set_value_of
			    ( s,
			      lp.base
				[lp.previous_index] );
			MUP::set_control_of
			    ( s,
			      MUP::new_control
			        ( min::LIST_AUX,
				  first,
				  MUP::STUB_POINTER ) );
			fgen = min::new_gen ( s );
		    }
		    lp.base[lp.previous_index] = fgen;
		    lp.previous_index = 0;
		}
		else
		{
		    lp.base[lp.current_index] = fgen;
		    lp.current_index = 0;
		}
		lp.previous_stub = last;
		lp.previous_is_sublist_head = false;
		return;
	    }
	    else
#	endif
		MIN_ASSERT (      unused_area_offset
			        + n + ( ! contiguous )
		                + previous_is_list_head
			     <= aux_area_offset );

#	if MIN_USES_OBJ_AUX_STUBS
	    if ( lp.previous_stub != NULL )
	    {
	        if ( lp.previous_is_sublist_head )
		{
		    fgen = min::new_list_aux_gen
			       ( aux_area_offset - 1 );
		    MUP::set_gen_of
			( lp.previous_stub, fgen );
		}
		else
		{
		    int type =
		        min::type_of
			    ( lp.previous_stub );
		    MUP::set_control_of
		        ( lp.previous_stub,
			  MUP::new_control
			      ( type,
			        aux_area_offset - 1 ) );
		}
		lp.previous_stub = NULL;
	    }
	    else
#	endif
	if ( lp.previous_index != 0 )
	{
	    if ( previous_is_list_head )
	    {
	        lp.base[-- aux_area_offset] =
		    lp.base[lp.previous_index];
		fgen = min::new_list_aux_gen
		    ( aux_area_offset );
	    }
	    else
		fgen = min::new_sublist_aux_gen
		    ( aux_area_offset - 1 );
	    lp.base[lp.previous_index] = fgen;
	}
	else if ( contiguous )
	    ++ aux_area_offset;
	else
	{
	    fgen = min::new_list_aux_gen
		       ( aux_area_offset - 1 );
	    lp.base[lp.current_index] = fgen;
	}

	while ( n -- )
	    lp.base[-- aux_area_offset] = * p ++;
	    
	lp.base[-- aux_area_offset] = min::LIST_END;
	lp.current_index = aux_area_offset;
	lp.previous_index = 0;
	lp.previous_is_sublist_head = false;
	if ( lp.so )
	    lp.so->aux_area_offset = aux_area_offset;
	else
	    lp.lo->aux_area_offset = aux_area_offset;
	return;
    }

    // lp.current != min::LIST_END

    // If there is no previous, we must move the current
    // element so we can replace it with a list pointer.
    // If the current element is in an aux stub, there
    // has to be a previous.
    //
    bool previous = ( lp.previous_index != 0 );

#   if MIN_USES_OBJ_AUX_STUBS
        if ( lp.previous_stub != NULL )
	    previous = true;
	if (    lp.use_obj_aux_stubs
	     &&     unused_area_offset
		  + n + 1 + ( ! previous )
		> aux_area_offset )
	{
	    // Not enough aux area available for all the
	    // new elements, and aux stubs are allowed.
	    // Prepare to call allocate_stub_list.
	    //
	    min::uns64 end;
	    min::stub * s;
	    int type = min::LIST_AUX;
	    if ( lp.current_stub != NULL )
	    {
		type = min::type_of ( lp.current_stub );
		MUP::set_type_of
		    ( lp.current_stub, min::LIST_AUX );
		end = MUP::new_control
		   ( 0, lp.current_stub,
		     MUP::STUB_POINTER );
		MIN_ASSERT ( previous );
	    }
	    else if ( ! previous )
	    {
	        s = MUP::new_aux_stub();
		MUP::set_gen_of ( s, lp.current );
		end = MUP::new_control
		    ( 0, s, MUP::STUB_POINTER );
		unsigned next = lp.current_index;
		if ( next < unused_area_offset )
		    next = 0;
		else if (    lp.base[-- next]
		          == min::LIST_END )
		{
		    // Next element (the one immediately
		    // before the current element in the
		    // aux area) is LIST_END will not
		    // be needed any more, hence we free
		    // it.
		    //
		    lp.base[next] = min::NONE;

		    next = 0;
		}
		MUP::set_control_of
		    ( s,
		      MUP::new_control
		        ( min::LIST_AUX, next ) );
	    }
	    else
	    {
	        if ( lp.previous_is_sublist_head )
		    type == min::SUBLIST_AUX;
		end = MUP::new_control
		   ( 0, lp.current_index );
	    }

	    min::stub * first, * last;
	    MUP::allocate_stub_list
		( first, last, type, p, n, end );

	    if ( lp.previous_index != 0 )
	    {
		lp.base[lp.previous_index] =
		    min::new_gen ( first );
		lp.previous_index = 0;
	    }
	    else if ( lp.previous_stub != NULL )
	    {
		if ( lp.previous_is_sublist_head )
		    MUP::set_gen_of
			( lp.previous_stub,
			  min::new_gen ( first ) );
		else
		{
		   type = min::type_of
			      ( lp.previous_stub );
		    MUP::set_control_of
			( lp.previous_stub,
			  MUP::new_control
			      ( type, first,
				MUP::STUB_POINTER ) );
		}
	    }
	    else
	    {
	        MIN_ASSERT ( lp.current_index != 0 );

		lp.base[lp.current_index] =
		    min::new_gen ( first );
		lp.current_index = 0;
		lp.current_stub = s;
	    }
	    lp.previous_stub = last;
	    lp.previous_is_sublist_head = false;
	    return;
	}
	else
#   endif
	    MIN_ASSERT (      unused_area_offset
			    + n + 1 + ( ! previous )
			 <= aux_area_offset );

    unsigned first = aux_area_offset - 1;

    while ( n -- )
	lp.base[-- aux_area_offset] = * p ++;

#   if MIN_USES_OBJ_AUX_STUBS
	if ( lp.current_stub != NULL )
	{
	    MIN_ASSERT ( previous );
	    lp.base[-- aux_area_offset] =
		min::new_gen ( lp.current_stub );
	    MUP::set_type_of
		( lp.current_stub, min::LIST_AUX );
	}
	else
#   endif
    {
	unsigned next = lp.current_index;
        if ( ! previous )
	{
	    lp.base[-- aux_area_offset] = lp.current;
	    if ( next < unused_area_offset )
	        next = 0;
	    else if (    lp.base[-- next]
	              == min::LIST_END )
	    {
		// Next element (the one immediately
		// before the current element in the aux
		// area) is LIST_END will not be needed
		// any more, hence we free it.
		//
		lp.base[next] = min::NONE;

		next = 0;
	    }
	}
        lp.base[-- aux_area_offset] =
	    min::new_list_aux_gen ( next );
    }

#   if MIN_USES_OBJ_AUX_STUBS
	if ( lp.previous_stub != NULL )
	{
	    if ( lp.previous_is_sublist_head )
	    {
	        MUP::set_gen_of
		    ( lp.previous_stub,
		      min::new_sublist_aux_gen
			  ( first ) );
	    }
	    else
	    {
	        int type =
		    min::type_of ( lp.previous_stub );
		MUP::set_control_of
		    ( lp.previous_stub,
		      MUP::new_control
			  ( type, first ) );
	    }
	    lp.previous_index = aux_area_offset;
	    lp.previous_stub = NULL;
	    lp.previous_is_sublist_head = false;
	}
	else
#   endif
    if ( lp.previous_index != 0 )
    {
	lp.base[lp.previous_index] =
	    MUP::renew_gen
		( lp.base[lp.previous_index],
		  first );
	lp.previous_index = aux_area_offset;
	lp.previous_is_sublist_head = false;
    }
    else
    {
	MIN_ASSERT ( lp.current_index != 0 );
	lp.base[lp.current_index] =
	    min::new_list_aux_gen ( first );
	lp.current_index = aux_area_offset + 1;
    }

    if ( lp.so )
	lp.so->aux_area_offset = aux_area_offset;
    else
	lp.lo->aux_area_offset = aux_area_offset;
}

void min::insert_after
	( MUP::insertable_list_pointer & lp,
	  const min::gen * p, unsigned n )
{
    unsigned unused_area_offset;
    unsigned aux_area_offset;

    if ( n == 0 ) return;
    else if ( lp.so )
    {
	unused_area_offset = lp.so->unused_area_offset;
	aux_area_offset = lp.so->aux_area_offset;
    }
    else if ( lp.lo )
    {
	unused_area_offset = lp.lo->unused_area_offset;
	aux_area_offset = lp.lo->aux_area_offset;
    }
    else
	MIN_ABORT ( "lp list has not been started" );

    MIN_ASSERT ( lp.reserved_insertions >= 1 );
    MIN_ASSERT ( lp.reserved_elements >= n );
    MIN_ASSERT ( lp.current != min::LIST_END );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    MUP::gc_write_update ( lp.s, p, n );

    bool previous = ( lp.previous_index != 0 );
#   if MIN_USES_OBJ_AUX_STUBS
	if ( lp.previous_stub != NULL )
	    previous = true;

	if (    lp.use_obj_aux_stubs
	     &&     unused_area_offset
		  + ( n + 1 + ! previous )
		> aux_area_offset )
	{
	    // Not enough aux area available for
	    // all the new elements, and aux stubs
	    // are allowed.
	    //
	    min::stub * first, * last;

	    if ( lp.current_stub != NULL )
	    {
		MUP::allocate_stub_list
		    ( first, last, min::LIST_AUX,
		      p, n,
		      MUP::control_of
		          ( lp.current_stub ) );
		MUP::set_control_of
		    ( lp.current_stub,
		      MUP::new_control
			 ( min::type_of
			       ( lp.current_stub ),
			   first,
			   MUP::STUB_POINTER ) );
		return;
	    }

	    min::stub * s = MUP::new_aux_stub();
	    MUP::set_gen_of ( s, lp.current );
	    int type = lp.previous_is_sublist_head ?
	    	       min::SUBLIST_AUX :
		       min::LIST_AUX;
	    unsigned next =
	        lp.current_index - ! previous;
	    if ( lp.current_index < unused_area_offset )
	        next = 0;
	    min::uns64 end =
		MUP::new_control ( type, next );

	    if ( n > previous )
		MUP::allocate_stub_list
		    ( first, last, min::LIST_AUX,
		      p, n - previous, end );

	    if ( previous )
	    {
		// Given previous, we can copy the last
		// new element to the old current
		// element.
		//
		if ( n > 1 )
		    end = MUP::new_control
		              ( type, first,
			        MUP::STUB_POINTER );
		MUP::set_control_of ( s, end );
		lp.base[lp.current_index] = p[n-1];
		lp.current_index = 0;
		lp.current_stub = s;

		if ( lp.previous_stub != NULL )
		{
		    if ( lp.previous_is_sublist_head )
			MUP::set_gen_of
			    ( lp.previous_stub,
			      min::new_gen ( s ) );
		    else
		    {
			unsigned type =
			    min::type_of
				( lp.previous_stub );
			MUP::set_control_of
			  ( lp.previous_stub,
			    MUP::new_control
			      ( type, s,
			        MUP::STUB_POINTER ) );
		    }
		}
		else
		    lp.base[lp.previous_index] =
			MUP::new_gen ( s );
	    }
	    else
	    {
	        // No previous.  Current aux element
		// has been moved to stub s and current
		// aux element is used to point at s.
		//
		MIN_ASSERT ( lp.current_index != 0 );

		MUP::set_control_of
		    ( s,
		      MUP::new_control
		          ( min::LIST_AUX, first,
			    MUP::STUB_POINTER ) );
		lp.base[lp.current_index] =
		    min::new_gen ( s );
		lp.current_index = 0;
		lp.current_stub = s;
	    }
	    return;
	}
#   endif

    // Insertion will use aux area.

    unsigned first = aux_area_offset - 1;

    if ( lp.current_index != 0 )
	lp.base[-- aux_area_offset] = lp.current;

    // Copy all the new elements BUT the last new
    // element.
    //
    while ( -- n )
	lp.base[-- aux_area_offset] = * p ++;

#   if MIN_USES_OBJ_AUX_STUBS
    if ( lp.current_stub != NULL )
    {
	MIN_ASSERT ( previous );
	lp.base[-- aux_area_offset] = * p ++;
	min::uns64 c =
	    MUP::control_of ( lp.current_stub );
	if ( c & MUP::STUB_POINTER )
	    lp.base[-- aux_area_offset] =
		min::new_gen
		    ( MUP::stub_of_control ( c ) );
	else
	    lp.base[-- aux_area_offset] =
	        min::new_list_aux_gen
		    ( MUP::value_of_control ( c ) );
	MUP::set_control_of
	    ( lp.current_stub,
	      MUP::new_control
	         ( min::type_of ( lp.current_stub ),
		   first ) );
    }
    else
#   endif
    if ( previous )
    {
	// Given previous, we can copy the last new
	// element to the old current element.
	//

	lp.base[-- aux_area_offset] =
	    min::new_list_aux_gen ( lp.current_index );
	lp.base[lp.current_index] = * p ++;
	lp.current_index = first;

#	if MIN_USES_OBJ_AUX_STUBS
	    if ( lp.previous_stub != NULL )
	    {
		if ( lp.previous_is_sublist_head )
		{
		    MUP::set_gen_of
			( lp.previous_stub,
			  min::new_sublist_aux_gen
			      ( first ) );
		}
		else
		{
		    MUP::set_control_of
			( lp.previous_stub,
			  MUP::new_control
			      ( min::type_of
				  ( lp.previous_stub ),
				first ) );
		}
	    }
	    else
#	endif
	{
	    lp.base[lp.previous_index] =
		MUP::renew_gen
		    ( lp.base[lp.previous_index],
		      first );
	}
    }
    else
    {
	// With no previous we must use current aux
	// element as pointer to copied of current
	// element.
	//
	MIN_ASSERT ( lp.current_index != 0 );

	lp.base[-- aux_area_offset] = * p ++;
	unsigned next = lp.current_index;
	if ( next < unused_area_offset )
	    next = 0;
	else if ( lp.base[-- next] == min::LIST_END )
	{
	    // Next element (the one immediately
	    // before the current element in the aux
	    // area) is LIST_END will not be needed
	    // any more, hence we free it.
	    //
	    lp.base[next] = min::NONE;

	    next = 0;
	}
	lp.base[-- aux_area_offset] =
	    min::new_list_aux_gen ( next );

	lp.base[lp.current_index] =
	    min::new_list_aux_gen ( first );
	lp.current_index = first;
    }

    if ( lp.so )
	lp.so->aux_area_offset = aux_area_offset;
    else
	lp.lo->aux_area_offset = aux_area_offset;
}

unsigned min::remove
	( MUP::insertable_list_pointer & lp, unsigned n )
{
    // Note: current code does NOT set orphaned sublist
    // elements to NONE.

    unsigned unused_area_offset;
    unsigned aux_area_offset;
    if ( lp.so )
    {
	unused_area_offset = lp.so->unused_area_offset;
	aux_area_offset = lp.so->aux_area_offset;
    }
    else if ( lp.lo )
    {
	unused_area_offset = lp.lo->unused_area_offset;
	aux_area_offset = lp.lo->aux_area_offset;
    }
    else
	MIN_ABORT ( "lp list has not been started" );

    if ( n == 0 || lp.current == min::LIST_END )
        return 0;

    // Count of items removed; to be returned as result.
    //
    unsigned count = 0;

    unsigned previous_index = lp.previous_index;
    bool previous_is_sublist_head =
	lp.previous_is_sublist_head;
    unsigned current_index = lp.current_index;
#   if ! MIN_USES_OBJ_AUX_STUBS
	if ( lp.current_index < unused_area_offset )
	{
	    // Special case: deleting list head of a
	    // a list with just 1 element.
	    //
	    lp.base[lp.current_index] = min::LIST_END;
	    return 1;
	}
	while ( n -- )
	{
	    if ( lp.current == min::LIST_END ) break;
	    ++ count;
	    lp.base[lp.current_index] = min::NONE;
	    lp.current = lp.base[-- lp.current.index];
	    if ( min::is_list_aux ( lp.current ) )
	    {
	        lp.base[lp.current_index] = min::NONE;
		lp.current_index =
		    min::list_aux_of ( lp.current );
		lp.current = lp.base[lp.current.index];
	    }
	}
#   elif MIN_USES_OBJ_AUX_STUBS
	min::stub * previous_stub = lp.previous_stub;
	while ( n -- )
	{
	    if ( lp.current == min::LIST_END ) break;
	    ++ count;
	    if ( lp.current_stub != NULL )
	    {
		min::stub * last_stub = lp.current_stub;
		next ( lp );
		MUP::free_stub ( last_stub );
	    }
	    else
	    {
	        MIN_ASSERT ( lp.current_index != 0 );
		lp.base[lp.current_index] = min::NONE;
		lp.current =
		    lp.base[-- lp.current_index];
		if ( min::is_list_aux ( lp.current ) )
		{
		    lp.base[lp.current_index] =
			min::NONE;
		    lp.current_index =
			min::list_aux_of ( lp.current );
		    lp.current =
			lp.base[lp.current_index];
		}
		else if ( min::is_stub ( lp.current ) )
		    lp.forward ( lp.current_index );
	    }
	}
#   endif

    // Now lp.current_index/lp.current_stub are the new
    // current element and we must either set the old
    // previous element to point to this, or if there
    // was no old previous element, we must make the
    // old current element into a previous element.

#   if MIN_USES_OBJ_AUX_STUBS

	if ( previous_stub != NULL )
	{
	    if ( lp.current_stub != NULL )
	    {
		if ( previous_is_sublist_head )
		{
		    MUP::set_type_of
			( lp.current_stub,
			  min::SUBLIST_AUX );
		    MUP::set_gen_of
		        ( previous_stub,
			  min::new_gen
			      ( lp.current_stub ) );
		}
		else
		{
		    int type =
		        min::type_of ( previous_stub );
		    MUP::set_control_of
		        ( previous_stub,
			  MUP::new_control
			      ( type, lp.current_stub,
			        MUP::STUB_POINTER ) );
		}
	    }
	    else if ( lp.current == min::LIST_END )
	    {
	        if ( previous_is_sublist_head )
		    MUP::set_gen_of
		        ( previous_stub,
			  min::EMPTY_SUBLIST );
		else
		{
		    int type =
		        min::type_of ( previous_stub );
		    MUP::set_control_of
		        ( previous_stub,
			  MUP::new_control
			      ( type, min::uns64(0) ) );
		}
		if ( lp.current_index != 0 )
		{
		    lp.base[lp.current_index] =
		        min::NONE;
		    lp.current_index = 0;
		}
	    }
	    else
	    {
	        MIN_ASSERT ( lp.current_index != 0 );

	        if ( previous_is_sublist_head )
		    MUP::set_gen_of
		        ( previous_stub,
			  min::new_sublist_aux_gen
			      ( lp.current_index ) );
		else
		{
		    int type =
		        min::type_of ( previous_stub );
		    MUP::set_control_of
		        ( previous_stub,
			  MUP::new_control
			      ( type,
			        lp.current_index ) );
		}
	    }

	    lp.previous_stub = previous_stub;
	    lp.previous_index = previous_index;
	    lp.previous_is_sublist_head =
		previous_is_sublist_head;

	}
	else
#   endif
    if ( previous_index != 0 )
    {
	if ( lp.current == min::LIST_END )
	{
	    if ( lp.current_index != 0 )
		lp.base[lp.current_index] = min::NONE;

	    if ( previous_is_sublist_head )
	    {
		lp.base[previous_index] =
		    min::EMPTY_SUBLIST;
		lp.current_index = 0;
	    }
	    else
	    {
		lp.base[previous_index] =
		    min::LIST_END;
		lp.current_index = previous_index;
		lp.previous_index = 0;
	    }
	}
#	if MIN_USES_OBJ_AUX_STUBS
	    else if ( lp.current_stub != NULL )
	    {
		if ( previous_is_sublist_head )
		    MUP::set_type_of
			( lp.current_stub,
			  min::SUBLIST_AUX );
		lp.base[previous_index] =
		    min::new_gen ( lp.current_stub );
	    }
#	endif
	else
	{
	    MIN_ASSERT ( lp.current_index != 0 );

	    if ( previous_is_sublist_head )
		lp.base[previous_index] =
		    min::new_sublist_aux_gen
			( lp.current_index );
	    else
		lp.base[previous_index] =
		    min::new_list_aux_gen
			( lp.current_index );
	}
    }
    else
    {
    	// No previous.  Note that lp.current_stub
	// == NULL as stubs always have a previous.

	MIN_ASSERT ( current_index != 0 );

	if ( lp.current == min::LIST_END )
	{
	    if ( lp.current_index != 0 )
		lp.base[lp.current_index] = min::NONE;
	    lp.base[current_index] = min::LIST_END;
	}
	else if ( current_index < unused_area_offset
	          &&
		     lp.base[current_index - 1]
		  == min::LIST_END )
    	{
	    // Now list has just one element in the list
	    // head.

	    lp.base[current_index] = lp.current;
	    lp.base[lp.current_index] = min::NONE;
	    lp.base[-- lp.current_index] = min::NONE;
	    lp.current_index = current_index;
	}
	else
	{
	    lp.base[current_index] =
		min::new_list_aux_gen ( lp.current_index );
	    lp.previous_index = current_index;
	    lp.previous_is_sublist_head = false;
	}
    }

    return count;
}

void MINT::insert_reserve
	( MUP::insertable_list_pointer & lp,
	  unsigned insertions,
	  unsigned elements,
	  bool use_obj_aux_stubs )
{
#   if MIN_USES_OBJ_AUX_STUBS
	if ( use_obj_aux_stubs )
	    MUP::gc_expand_stub_free_list
		( insertions + elements );
	else
#   endif
    {
	MIN_ABORT ( "insert reserve not implemented" );
    }

    lp.reserved_insertions = insertions;
    lp.reserved_elements = elements;
#   if MIN_USES_OBJ_AUX_STUBS
	lp.use_obj_aux_stubs = use_obj_aux_stubs;
#   endif
}


// Numbers
// -------


// TBD
// ---
