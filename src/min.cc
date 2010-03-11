// MIN Language Out-of-Line Code
//
// File:	min.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Thu Mar 11 11:44:22 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/03/11 16:44:33 $
//   $RCSfile: min.cc,v $
//   $Revision: 1.185 $

// Table of Contents:
//
//	Setup
//	Initialization
//	Stub Functions
//	Process Management
//	Allocator/Collector/Compactor
//	Numbers
//	Strings
//	Labels
//	Raw Vectors
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level

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

static bool initializer_called = false;
MINT::initializer::initializer ( void )
{
    if ( initializer_called ) return;
    initializer_called = true;

    assert ( sizeof ( MIN_INT32_TYPE ) == 4 );
    assert ( sizeof ( MIN_INT64_TYPE ) == 8 );
    assert
        ( sizeof ( void * ) == MIN_POINTER_BITS / 8 );

    assert
      ( MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG <= 33 );

    assert
      (   MINT::SHORT_OBJ_FLAG_BITS
        + MINT::SHORT_OBJ_MANTISSA_BITS
	+ MINT::SHORT_OBJ_EXPONENT_BITS
	== 16 );
    assert
      (   MINT::SHORT_OBJ_HASH_CODE_BITS
        + MINT::SHORT_OBJ_VAR_CODE_BITS
	== 16 );
    assert
      (   MINT::LONG_OBJ_FLAG_BITS
        + MINT::LONG_OBJ_MANTISSA_BITS
	+ MINT::LONG_OBJ_EXPONENT_BITS
	== 32 );
    assert
      (   MINT::LONG_OBJ_HASH_CODE_BITS
        + MINT::LONG_OBJ_VAR_CODE_BITS
	== 32 );

    // Check that max total sizes are representable
    // and small enough for representable offsets.
    //
    min::uns64 short_max_representable =
    	(    (min::uns64) 1
	  << MINT::SHORT_OBJ_MANTISSA_BITS )
	<<
	( ( 1 << MINT::SHORT_OBJ_EXPONENT_BITS ) - 1 );
    min::uns64 long_max_representable =
    	(    (min::uns64) 1
	  << MINT::LONG_OBJ_MANTISSA_BITS )
	<<
	( ( 1 << MINT::LONG_OBJ_EXPONENT_BITS ) - 1 );
    assert
      ( MINT::SHORT_OBJ_MAX_TOTAL_SIZE <= ( 1 << 16 ) );
    assert
      (    MINT::SHORT_OBJ_MAX_TOTAL_SIZE
        <= short_max_representable );
    assert
      (    MINT::LONG_OBJ_MAX_TOTAL_SIZE
        <= ( 1ull << 32 ) );
    assert
      (    MINT::LONG_OBJ_MAX_TOTAL_SIZE
        <= long_max_representable );

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

    MINT::acc_initializer();
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

// Process Management
// ------- ----------

namespace min { namespace internal {

    bool relocated_flag;

} }

// Allocator/Collector/Compactor 
// -----------------------------

MINT::gen_locator * MINT::static_gen_last;
MINT::gen_locator * MINT::stack_gen_last;

namespace min { namespace internal {

    min::unsptr number_of_free_stubs;

    min::stub ** str_hash;
    min::unsptr str_hash_size;
    min::unsptr str_hash_mask;

    min::stub ** num_hash;
    min::unsptr num_hash_size;
    min::unsptr num_hash_mask;

    min::stub ** lab_hash;
    min::unsptr lab_hash_size;
    min::unsptr lab_hash_mask;

#   ifndef MIN_STUB_BASE
	min::unsptr stub_base;
	min::stub * null_stub;
#   endif

    min::uns64 acc_stack_mask;
    min::stub ** acc_stack;
    min::stub ** acc_stack_limit;

    min::uns64 new_acc_stub_flags;
    min::stub * last_allocated_stub;

    MINT::fixed_block_list fixed_blocks
	    [MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG-2];

    min::unsptr max_fixed_block_size;

} }

namespace min { namespace unprotected {

    min::stub * resize_body::last_allocated;

    min::stub * resize_body::rstub_allocate ( void )
    {
	min::stub * endstub = last_allocated;
	for ( int i = 0; i < 5; ++ i )
	{
	    min::stub * rstub = MUP::new_aux_stub();
	    MUP::set_control_of
	         ( endstub, MUP::renew_control_stub
	                        ( MUP::control_of
				    ( endstub ),
				  rstub ) );
	    endstub = rstub;
	}
	return MUP::stub_of_control
	            ( MUP::control_of
		        ( last_allocated ) );
    }

} }

namespace min { namespace internal {

    void acc_initialize_resize_body ( void )
    {
	min::stub * s = MUP::new_aux_stub();
	MUP::resize_body::last_allocated = s;
	MUP::set_control_of ( s,
	    MUP::renew_control_stub
		( MUP::control_of ( s ),
		  MINT::null_stub ) );
    }

} }

// Numbers
// -------

# if MIN_IS_COMPACT
    min::gen MINT::new_num_stub_gen
	    ( min::float64 v )
    {
	min::uns32 hash = floathash ( v );
	min::uns32 h = hash & MINT::num_hash_mask;
	min::stub * s = MINT::num_hash[h];
	while ( s != MINT::null_stub )
	{
	    min::stub * s2;
	    if (    min::type_of ( s )
	         == min::HASHTABLE_AUX )
	    {
	        s2 = (min::stub *)
		     MUP::pointer_of ( s );
		s = (min::stub *)
		    MUP::stub_of_control
			( MUP::control_of ( s ) );
	    }
	    else
	    {
	    	s2 = s;
		s = (min::stub *)
		    MUP::stub_of_acc_control
			( MUP::control_of ( s ) );
	    }

	    if ( MUP::float_of ( s2 ) == v )
		return min::new_gen ( s2 );

	}

	min::stub * s2 = MUP::new_acc_stub();
	MUP::set_float_of ( s2, v );
	MUP::set_type_of ( s2, min::NUMBER );

	s = MUP::new_aux_stub ();
	MUP::set_pointer_of ( s, s2 );
	MUP::set_control_of
	    ( s,
	      MUP::new_control_with_type
	          ( min::HASHTABLE_AUX,
		    MINT::num_hash[h] ) );
	MINT::num_hash[h] = s;

	return min::new_gen ( s2 );
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
	( const char * p, min::unsptr size )
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

min::unsptr min::strlen ( min::gen v )
{
    if ( min::is_direct_str ( v ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = min::direct_str_of ( v );
	return ::strlen ( u.buf );
    }

    const min::stub * s = min::stub_of ( v );
    if ( type_of ( s ) == min::SHORT_STR )
    {
	const char * p = s->v.c8;
	const char * endp = p + 8;
	while ( * p && p < endp ) ++ p;
	return p - s->v.c8;
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
	return unprotected::long_str_of ( s )->length;
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

    const min::stub * s = min::stub_of ( v );
    if ( type_of ( s ) == min::SHORT_STR )
	return min::strnhash ( s->v.c8, 8 );
    else
    {
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
	return unprotected::long_str_of ( s )->hash;
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

    const min::stub * s = min::stub_of ( v );
    if ( type_of ( s ) == min::SHORT_STR )
    {
	if ( s->v.c8[7] )
	{
	    p[8] = 0;
	    return ::strncpy ( p, s->v.c8, 8 );
	}
	return ::strcpy ( p, s->v.c8 );
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
	return ::strcpy
	    ( p, min::unprotected::str_of
		   ( unprotected::long_str_of ( s ) ) );
    }
}

char * min::strncpy
	( char * p, min::gen v, min::unsptr n )
{
    if ( min::is_direct_str ( v ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = min::direct_str_of ( v );
	return ::strncpy ( p, u.buf, n );
    }

    const min::stub * s = min::stub_of ( v );
    if ( type_of ( s ) == min::SHORT_STR )
    {
	if ( s->v.c8[7] && n >= 9 )
	    p[8] = 0;
	return ::strncpy
		 ( p, s->v.c8, n < 8 ? n : 8 );
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
	return ::strncpy
	    ( p, min::unprotected::str_of
		   ( unprotected::long_str_of ( s ) ),
		 n );
    }
}

int min::strcmp ( const char * p, min::gen v )
{
    if ( min::is_direct_str ( v ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = min::direct_str_of ( v );
	return ::strcmp ( p, u.buf );
    }

    const min::stub * s = min::stub_of ( v );
    if ( type_of ( s ) == min::SHORT_STR )
    {
	if ( s->v.c8[7] )
	{
	    int r = ::strncmp ( p, s->v.c8, 8 );
	    if ( r != 0 )
	        return r;
	    else
	        return ::strcmp ( p + 8, "" );
	}
	return ::strcmp ( p, s->v.c8 );
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
	return ::strcmp
	    ( p, min::unprotected::str_of
		   ( unprotected::long_str_of ( s ) ) );
    }
}

int min::strncmp
	( const char * p, min::gen v, min::unsptr n )
{
    if ( min::is_direct_str ( v ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = min::direct_str_of ( v );
	return ::strncmp ( p, u.buf, n );
    }

    const min::stub * s = min::stub_of ( v );
    if ( type_of ( s ) == min::SHORT_STR )
    {
	if ( s->v.c8[7] && n >= 9 )
	{
	    int r = ::strncmp ( p, s->v.c8, 8 );
	    if ( r != 0 )
	        return r;
	    else
	        return ::strcmp ( p + 8, "" );
	}
	return ::strncmp ( p, s->v.c8, n );
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
	return ::strncmp
	    ( p, min::unprotected::str_of
		   ( unprotected::long_str_of ( s ) ),
		 n );
    }
}

// Perform the new_str_stub_gen operation where n is the
// exact number of characters in p that are to be used
// (instead of the maximum).  There must be no NULs in
// the first n characters of p.
//
min::gen MINT::new_str_stub_gen
	( const char * p, min::unsptr n )
{
    min::uns32 hash = min::strnhash ( p, n );
    min::uns32 h = hash & MINT::str_hash_mask;
    min::stub * s = MINT::str_hash[h];
    const char * q;
    while ( s != MINT::null_stub )
    {
	min::stub * s2;
	if ( min::type_of ( s ) == min::HASHTABLE_AUX )
	{
	    s2 = (min::stub *) MUP::pointer_of ( s );
	    s = (min::stub *)
		MUP::stub_of_control
		    ( MUP::control_of ( s ) );
	}
	else
	{
	    s2 = s;
	    s = (min::stub *)
		MUP::stub_of_acc_control
		    ( MUP::control_of ( s ) );
	}

        if (    n <= 8
	     && min::type_of ( s2 ) == min::SHORT_STR
	     && ::strncmp ( p, s2->v.c8, n ) == 0
	     && (    n == 8
	          || s2->v.c8[n] == 0 ) )
	    return min::new_gen ( s2 );
	else if (    n > 8
	          &&    min::type_of ( s2 )
		     == min::LONG_STR
	          && ::strncmp
		       ( p, q = MUP::str_of (
			            MUP::long_str_of
				        ( s2 ) ),
			    n )
		     == 0
		  && q[n] == 0 )
	    return min::new_gen ( s2 );
    }

    min::stub * s2 = MUP::new_acc_stub();
    if ( n <= 8 )
    {
	MUP::set_type_of ( s2, min::SHORT_STR );
	s2->v.u64 = 0;
	::strncpy ( s2->v.c8, p, n );
    }
    else
    {
	MUP::set_type_of ( s2, min::LONG_STR );
	MUP::new_body
	    ( s2, sizeof ( MUP::long_str ) + n + 1 );
	MUP::long_str * ls = MUP::long_str_of ( s2 );
	ls->length = n;
	ls->hash = hash;
	::strcpy ( (char *) MUP::str_of ( ls ), p );
    }

    s = MUP::new_aux_stub ();
    MUP::set_pointer_of ( s, s2 );
    MUP::set_control_of
	( s,
	  MUP::new_control_with_type
	      ( min::HASHTABLE_AUX,
		MINT::str_hash[h] ));
    MINT::str_hash[h] = s;

    return min::new_gen ( s2 );
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
	( const min::gen * p, min::unsptr n )
{
    min::uns32 hash = 1009;
    while ( n -- )
    {
        MIN_ASSERT ( min::is_name ( * p ) );
        hash = lab_multiplier * hash
	     + min::hash ( * p ++ );
    }
    return hash;
}

min::gen min::new_lab_gen
	( const min::gen * p, min::unsptr n )
{
    min::uns32 hash = labhash ( p, n );
    min::uns32 h = hash & MINT::lab_hash_mask;

    // Search for existing label stub with given
    // elements.
    //
    min::stub * s = MINT::lab_hash[h];
    while ( s != MINT::null_stub )
    {
        min::stub * s2;
	if ( min::type_of ( s ) == min::HASHTABLE_AUX )
	{
	    s2 = (min::stub *) MUP::pointer_of ( s );
            s = MUP::stub_of_control
			( MUP::control_of ( s ) );
	}
	else
	{
	    s2 = s;
            s = MUP::stub_of_acc_control
			( MUP::control_of ( s ) );
	}

	MIN_ASSERT
	    ( min::type_of ( s2 ) == min::LABEL );
	MINT::lab_header * lh =
	    MINT::lab_header_of ( s2 );

	if ( hash != MINT::labhash ( lh ) ) continue;

	if ( n != MINT::lablen ( lh ) ) continue;
	const min::gen * q = MINT::vector_of ( lh );
	int i;
	for ( i = 0; i < n && p[i] == q[i]; ++ i );
	if ( i == n ) return new_gen ( s2 );
    }

    // Allocate new label.
    //
    min::stub * s2 = MUP::new_acc_stub ();
    MUP::new_body ( s2,   sizeof ( MINT::lab_header )
	                + n * sizeof (min::gen) );
    MINT::lab_header * lh = MINT::lab_header_of ( s2 );
    lh->length = n;
    lh->hash = hash;
    memcpy ( (min::gen *) lh + MINT::lab_header_size,
             p, n * sizeof ( min::gen ) );

    s = MUP::new_aux_stub ();
    MUP::set_pointer_of ( s, s2 );
    MUP::set_control_of
	( s,
	  MUP::new_control_with_type
	      ( min::HASHTABLE_AUX,
		MINT::lab_hash[h] ));
    MINT::lab_hash[h] = s;

    MUP::set_type_of ( s2, min::LABEL );
    return min::new_gen ( s2 );
}

// Raw Vectors
// --- -------

min::gen min::internal::new_raw_vec_gen
	( const min::raw_vec_type_info & type_info )
{
    min::stub * s = unprotected::new_acc_stub();
    unprotected::new_body
	( s,
	    sizeof ( internal::raw_vec_header )
	  +   type_info.element_size
	    * type_info.initial_max_length );
    internal::raw_vec_header & h =
	* (internal::raw_vec_header *)
	  unprotected::pointer_of ( s );
    h.type_info = & type_info;
    h.length = 0;
    h.max_length = type_info.initial_max_length;
    unprotected::set_type_of ( s, min::RAW_VEC );
    return new_gen ( s );
}

void min::internal::resize 
	( min::stub * s,
	  min::unsptr new_max_length,
	  const min::raw_vec_type_info & type_info )
{
    raw_vec_header * & oldhp =
        * (raw_vec_header **) &
	unprotected::pointer_ref_of ( s );
    min::unsptr old_size =
	  sizeof ( raw_vec_header )
	+   oldhp->max_length
	  * type_info.element_size;
    min::unsptr new_size =
	  sizeof ( raw_vec_header )
	+ new_max_length * type_info.element_size;
    min::unsptr min_size =
	old_size < new_size ? old_size :
			      new_size;
    unprotected::resize_body r
	( s, new_size, old_size );
    raw_vec_header * & newhp =
	* ( raw_vec_header **) &
	unprotected::new_body_pointer_ref ( r );
    memcpy ( newhp, oldhp, min_size );
    newhp->max_length = new_max_length;
    if ( newhp->length > new_max_length )
	newhp->length = new_max_length;
}

void min::internal::expand 
	( min::stub * s,
	  min::unsptr required_increment,
	  const min::raw_vec_type_info & type_info )
{
    raw_vec_header * & oldhp =
        * (raw_vec_header **) &
	unprotected::pointer_ref_of ( s );
    min::unsptr max_length = oldhp->max_length;
    min::unsptr increment =
	(min::unsptr) (   type_info.increment_ratio
			* max_length );
    if ( increment > type_info.max_increment )
	increment = type_info.max_increment;
    if ( increment < required_increment )
	increment = required_increment;
    resize ( s, max_length + increment, type_info );
}

// Objects
// -------

namespace min { namespace internal {

    min::uns32 hash_size [] =
    {
	// [0 .. 15]
		  0,          1,          2,          3,
		  4,          5,          6,          7,
		  8,          9,         10,         11,
		 12,         13,         14,         15,
	// [16 .. 31]
		 16,         17,         18,         19,
		 20,         21,         22,         23,
		 24,         25,         26,         27,
		 28,         29,         30,         31,
	// [32 .. 47]
		 32,         33,         34,         35,
		 36,         37,         38,         39,
		 40,         41,         42,         43,
		 44,         45,         46,         47,
	// [48 .. 63]
		 48,         49,         50,         51,
		 52,         53,         54,         55,
		 56,         57,         58,         59,
		 60,         61,         62,         63,
	// [64 .. 79]
		 64,         65,         66,         67,
		 68,         69,         70,         71,
		 72,         73,         74,         75,
		 76,         77,         78,         79,
	// [80 .. 95]
		 80,         81,         82,         83,
		 84,         85,         86,         87,
		 88,         89,         90,         91,
		 92,         93,         94,         95,
	// [96 .. 111]
		 96,         97,         98,         99,
		100,        101,        102,        103,
		104,        105,        106,        107,
		108,        109,        110,        111,
	// [112 .. 127]
		112,        113,        114,        115,
		116,        117,        118,        119,
		120,        121,        122,        123,
		124,        125,        126,        127,
	// [128 .. 143]
		128,        129,        130,        131,
		132,        133,        134,        135,
		136,        137,        138,        139,
		140,        141,        142,        143,
	// [144 .. 159]
		144,        145,        146,        147,
		148,        149,        150,        151,
		152,        153,        154,        155,
		156,        157,        158,        159,
	// [160 .. 175]
		160,        161,        162,        163,
		164,        165,        166,        167,
		168,        169,        170,        171,
		172,        173,        174,        175,
	// [176 .. 191]
		176,        177,        178,        179,
		180,        181,        182,        183,
		184,        185,        186,        187,
		188,        189,        190,        191,
	// [192 .. 207]
		192,        193,        194,        195,
		196,        197,        198,        199,
		200,        201,        202,        203,
		204,        205,        206,        207,
	// [208 .. 223]
		208,        209,        210,        211,
		212,        213,        214,        215,
		216,        217,        218,        219,
		220,        221,        222,        223,
	// [224 .. 239]
		224,        225,        226,        227,
		228,        229,        230,        231,
		232,        233,        234,        235,
		236,        237,        238,        239,
	// [240 .. 255]
		240,        241,        242,        243,
		244,        245,        246,        247,
		248,        249,        250,        251,
		252,        253,        254,        255,
	// [256 .. 271]
		256,        257,        258,        259,
		260,        261,        262,        263,
		264,        265,        266,        267,
		268,        269,        270,        271,
	// [272 .. 287]
		272,        273,        274,        275,
		276,        277,        278,        279,
		280,        281,        282,        283,
		284,        285,        286,        287,
	// [288 .. 303]
		288,        289,        290,        291,
		292,        293,        294,        295,
		296,        297,        298,        299,
		300,        301,        302,        303,
	// [304 .. 319]
		304,        305,        306,        307,
		308,        309,        310,        311,
		312,        313,        314,        315,
		316,        317,        318,        319,
	// [320 .. 335]
		320,        321,        322,        323,
		324,        325,        326,        327,
		328,        329,        330,        331,
		332,        333,        334,        335,
	// [336 .. 351]
		336,        337,        338,        339,
		340,        341,        342,        343,
		344,        345,        346,        347,
		348,        349,        350,        351,
	// [352 .. 367]
		352,        353,        354,        355,
		356,        357,        358,        359,
		360,        361,        362,        363,
		364,        365,        366,        367,
	// [368 .. 383]
		368,        369,        370,        371,
		372,        373,        374,        375,
		376,        377,        378,        379,
		380,        381,        382,        383,
	// [384 .. 399]
		384,        385,        386,        387,
		388,        389,        390,        391,
		392,        393,        394,        395,
		396,        397,        398,        399,
	// [400 .. 415]
		400,        401,        402,        403,
		404,        405,        406,        407,
		408,        409,        410,        411,
		412,        413,        414,        415,
	// [416 .. 431]
		416,        417,        418,        419,
		420,        421,        422,        423,
		424,        425,        426,        427,
		428,        429,        430,        431,
	// [432 .. 447]
		432,        433,        434,        435,
		436,        437,        438,        439,
		440,        441,        442,        443,
		444,        445,        446,        447,
	// [448 .. 463]
		448,        449,        450,        451,
		452,        453,        454,        455,
		456,        457,        458,        459,
		460,        461,        462,        463,
	// [464 .. 479]
		464,        465,        466,        467,
		468,        469,        470,        471,
		472,        473,        474,        475,
		476,        477,        478,        479,
	// [480 .. 495]
		480,        481,        482,        483,
		484,        485,        486,        487,
		488,        489,        490,        491,
		492,        493,        494,        495,
	// [496 .. 511]
		496,        497,        498,        499,
		500,        501,        502,        503,
		504,        505,        506,        507,
		508,        509,        510,        511,
	// [512 .. 527]
		512,        513,        514,        515,
		516,        517,        518,        519,
		520,        521,        522,        523,
		524,        525,        526,        527,
	// [528 .. 543]
		528,        529,        530,        531,
		532,        533,        534,        535,
		536,        537,        538,        539,
		540,        541,        542,        543,
	// [544 .. 559]
		544,        545,        546,        547,
		548,        549,        550,        551,
		552,        553,        554,        555,
		556,        557,        558,        559,
	// [560 .. 575]
		560,        561,        562,        563,
		564,        565,        566,        567,
		568,        569,        570,        571,
		572,        573,        574,        575,
	// [576 .. 591]
		576,        577,        578,        579,
		580,        581,        582,        583,
		584,        585,        586,        587,
		588,        589,        590,        591,
	// [592 .. 607]
		592,        593,        594,        595,
		596,        597,        598,        599,
		600,        601,        602,        603,
		604,        605,        606,        607,
	// [608 .. 623]
		608,        609,        610,        611,
		612,        613,        614,        615,
		616,        617,        618,        619,
		620,        621,        622,        623,
	// [624 .. 639]
		624,        625,        626,        627,
		628,        629,        630,        631,
		632,        633,        634,        635,
		636,        637,        638,        639,
	// [640 .. 655]
		640,        641,        642,        643,
		644,        645,        646,        647,
		648,        649,        650,        651,
		652,        653,        654,        655,
	// [656 .. 671]
		656,        657,        658,        659,
		660,        661,        662,        663,
		664,        665,        666,        667,
		668,        669,        670,        671,
	// [672 .. 687]
		672,        673,        674,        675,
		676,        677,        678,        679,
		680,        681,        682,        683,
		684,        685,        686,        687,
	// [688 .. 703]
		688,        689,        690,        691,
		692,        693,        694,        695,
		696,        697,        698,        699,
		700,        701,        702,        703,
	// [704 .. 719]
		704,        705,        706,        707,
		708,        709,        710,        711,
		712,        713,        714,        715,
		716,        717,        718,        719,
	// [720 .. 735]
		720,        721,        722,        723,
		724,        725,        726,        727,
		728,        729,        730,        731,
		732,        733,        734,        735,
	// [736 .. 751]
		736,        737,        738,        739,
		740,        741,        742,        743,
		744,        745,        746,        747,
		748,        749,        750,        751,
	// [752 .. 767]
		752,        753,        754,        755,
		756,        757,        758,        759,
		760,        761,        762,        763,
		764,        765,        766,        767,
	// [768 .. 783]
		768,        769,        770,        771,
		772,        773,        774,        775,
		776,        777,        778,        779,
		780,        781,        782,        783,
	// [784 .. 799]
		784,        785,        786,        787,
		788,        789,        790,        791,
		792,        793,        794,        795,
		796,        797,        798,        799,
	// [800 .. 815]
		800,        801,        802,        803,
		804,        805,        806,        807,
		808,        809,        810,        811,
		812,        813,        814,        815,
	// [816 .. 831]
		816,        817,        818,        819,
		820,        821,        822,        823,
		824,        825,        826,        827,
		828,        829,        830,        831,
	// [832 .. 847]
		832,        833,        834,        835,
		836,        837,        838,        839,
		840,        841,        842,        843,
		844,        845,        846,        847,
	// [848 .. 863]
		848,        849,        850,        851,
		852,        853,        854,        855,
		856,        857,        858,        859,
		860,        861,        862,        863,
	// [864 .. 879]
		864,        865,        866,        867,
		868,        869,        870,        871,
		872,        873,        874,        875,
		876,        877,        878,        879,
	// [880 .. 895]
		880,        881,        882,        883,
		884,        885,        886,        887,
		888,        889,        890,        891,
		892,        893,        894,        895,
	// [896 .. 911]
		896,        897,        898,        899,
		900,        901,        902,        903,
		904,        905,        906,        907,
		908,        909,        910,        911,
	// [912 .. 927]
		912,        913,        914,        915,
		916,        917,        918,        919,
		920,        921,        922,        923,
		924,        925,        926,        927,
	// [928 .. 943]
		928,        929,        930,        931,
		932,        933,        934,        935,
		936,        937,        938,        939,
		940,        941,        942,        943,
	// [944 .. 959]
		944,        945,        946,        947,
		948,        949,        950,        951,
		952,        953,        954,        955,
		956,        957,        958,        959,
	// [960 .. 975]
		960,        961,        962,        963,
		964,        965,        966,        967,
		968,        969,        970,        971,
		972,        973,        974,        975,
	// [976 .. 991]
		976,        977,        978,        979,
		980,        981,        982,        983,
		984,        985,        986,        987,
		988,        989,        990,        991,
	// [992 .. 1007]
		992,        993,        994,        995,
		996,       1003,       1010,       1017,
	       1024,       1031,       1038,       1046,
	       1053,       1060,       1067,       1075,
	// [1008 .. 1023]
	       1082,       1090,       1097,       1105,
	       1113,       1121,       1128,       1136,
	       1144,       1152,       1160,       1168,
	       1176,       1184,       1193,       1201,
	// [1024 .. 1039]
	       1209,       1218,       1226,       1235,
	       1243,       1252,       1261,       1269,
	       1278,       1287,       1296,       1305,
	       1314,       1323,       1333,       1342,
	// [1040 .. 1055]
	       1351,       1361,       1370,       1380,
	       1389,       1399,       1409,       1418,
	       1428,       1438,       1448,       1458,
	       1468,       1479,       1489,       1499,
	// [1056 .. 1071]
	       1510,       1520,       1531,       1541,
	       1552,       1563,       1574,       1585,
	       1596,       1607,       1618,       1629,
	       1641,       1652,       1663,       1675,
	// [1072 .. 1087]
	       1687,       1698,       1710,       1722,
	       1734,       1746,       1758,       1771,
	       1783,       1795,       1808,       1820,
	       1833,       1846,       1859,       1872,
	// [1088 .. 1103]
	       1885,       1898,       1911,       1924,
	       1938,       1951,       1965,       1978,
	       1992,       2006,       2020,       2034,
	       2048,       2062,       2077,       2091,
	// [1104 .. 1119]
	       2106,       2120,       2135,       2150,
	       2165,       2180,       2195,       2210,
	       2226,       2241,       2257,       2272,
	       2288,       2304,       2320,       2336,
	// [1120 .. 1135]
	       2353,       2369,       2385,       2402,
	       2419,       2435,       2452,       2469,
	       2487,       2504,       2521,       2539,
	       2557,       2574,       2592,       2610,
	// [1136 .. 1151]
	       2628,       2647,       2665,       2684,
	       2702,       2721,       2740,       2759,
	       2778,       2798,       2817,       2837,
	       2856,       2876,       2896,       2916,
	// [1152 .. 1167]
	       2937,       2957,       2978,       2998,
	       3019,       3040,       3061,       3083,
	       3104,       3126,       3148,       3169,
	       3191,       3214,       3236,       3259,
	// [1168 .. 1183]
	       3281,       3304,       3327,       3350,
	       3373,       3397,       3421,       3444,
	       3468,       3492,       3517,       3541,
	       3566,       3591,       3616,       3641,
	// [1184 .. 1199]
	       3666,       3692,       3717,       3743,
	       3769,       3795,       3822,       3848,
	       3875,       3902,       3929,       3956,
	       3984,       4012,       4040,       4068,
	// [1200 .. 1215]
	       4096,       4124,       4153,       4182,
	       4211,       4240,       4270,       4300,
	       4330,       4360,       4390,       4421,
	       4451,       4482,       4513,       4545,
	// [1216 .. 1231]
	       4576,       4608,       4640,       4673,
	       4705,       4738,       4771,       4804,
	       4837,       4871,       4905,       4939,
	       4973,       5008,       5043,       5078,
	// [1232 .. 1247]
	       5113,       5149,       5185,       5221,
	       5257,       5293,       5330,       5367,
	       5405,       5442,       5480,       5518,
	       5557,       5595,       5634,       5673,
	// [1248 .. 1263]
	       5713,       5753,       5793,       5833,
	       5873,       5914,       5955,       5997,
	       6039,       6081,       6123,       6165,
	       6208,       6252,       6295,       6339,
	// [1264 .. 1279]
	       6383,       6427,       6472,       6517,
	       6562,       6608,       6654,       6700,
	       6747,       6794,       6841,       6889,
	       6937,       6985,       7033,       7082,
	// [1280 .. 1295]
	       7132,       7181,       7231,       7281,
	       7332,       7383,       7434,       7486,
	       7538,       7591,       7643,       7697,
	       7750,       7804,       7858,       7913,
	// [1296 .. 1311]
	       7968,       8023,       8079,       8135,
	       8192,       8249,       8306,       8364,
	       8422,       8481,       8540,       8599,
	       8659,       8719,       8780,       8841,
	// [1312 .. 1327]
	       8903,       8964,       9027,       9090,
	       9153,       9216,       9281,       9345,
	       9410,       9476,       9541,       9608,
	       9675,       9742,       9810,       9878,
	// [1328 .. 1343]
	       9947,      10016,      10086,      10156,
	      10226,      10297,      10369,      10441,
	      10514,      10587,      10661,      10735,
	      10809,      10885,      10960,      11037,
	// [1344 .. 1359]
	      11113,      11191,      11268,      11347,
	      11426,      11505,      11585,      11666,
	      11747,      11829,      11911,      11994,
	      12077,      12161,      12246,      12331,
	// [1360 .. 1375]
	      12417,      12503,      12590,      12678,
	      12766,      12855,      12944,      13034,
	      13125,      13216,      13308,      13401,
	      13494,      13588,      13682,      13777,
	// [1376 .. 1391]
	      13873,      13970,      14067,      14165,
	      14263,      14362,      14462,      14563,
	      14664,      14766,      14869,      14972,
	      15076,      15181,      15287,      15393,
	// [1392 .. 1407]
	      15500,      15608,      15717,      15826,
	      15936,      16047,      16158,      16271,
	      16384,      16498,      16613,      16728,
	      16845,      16962,      17080,      17199,
	// [1408 .. 1423]
	      17318,      17439,      17560,      17682,
	      17805,      17929,      18054,      18179,
	      18306,      18433,      18561,      18690,
	      18820,      18951,      19083,      19216,
	// [1424 .. 1439]
	      19349,      19484,      19619,      19756,
	      19893,      20032,      20171,      20311,
	      20453,      20595,      20738,      20882,
	      21028,      21174,      21321,      21469,
	// [1440 .. 1455]
	      21619,      21769,      21921,      22073,
	      22227,      22381,      22537,      22694,
	      22851,      23010,      23170,      23332,
	      23494,      23657,      23822,      23988,
	// [1456 .. 1471]
	      24154,      24322,      24492,      24662,
	      24834,      25006,      25180,      25355,
	      25532,      25709,      25888,      26068,
	      26249,      26432,      26616,      26801,
	// [1472 .. 1487]
	      26987,      27175,      27364,      27554,
	      27746,      27939,      28133,      28329,
	      28526,      28725,      28924,      29126,
	      29328,      29532,      29738,      29944,
	// [1488 .. 1503]
	      30153,      30362,      30574,      30786,
	      31000,      31216,      31433,      31652,
	      31872,      32094,      32317,      32542,
	      32768,      32996,      33225,      33457,
	// [1504 .. 1519]
	      33689,      33924,      34160,      34397,
	      34636,      34877,      35120,      35364,
	      35610,      35858,      36107,      36358,
	      36611,      36866,      37122,      37381,
	// [1520 .. 1535]
	      37641,      37902,      38166,      38431,
	      38699,      38968,      39239,      39512,
	      39787,      40063,      40342,      40623,
	      40905,      41190,      41476,      41765,
	// [1536 .. 1551]
	      42055,      42348,      42642,      42939,
	      43238,      43538,      43841,      44146,
	      44453,      44762,      45074,      45387,
	      45703,      46021,      46341,      46663,
	// [1552 .. 1567]
	      46988,      47315,      47644,      47975,
	      48309,      48645,      48983,      49324,
	      49667,      50012,      50360,      50711,
	      51063,      51419,      51776,      52136,
	// [1568 .. 1583]
	      52499,      52864,      53232,      53602,
	      53975,      54350,      54728,      55109,
	      55492,      55878,      56267,      56658,
	      57052,      57449,      57849,      58251,
	// [1584 .. 1599]
	      58656,      59064,      59475,      59889,
	      60305,      60725,      61147,      61573,
	      62001,      62432,      62866,      63304,
	      63744,      64187,      64634,      65083,
	// [1600 .. 1615]
	      65536,      65992,      66451,      66913,
	      67378,      67847,      68319,      68794,
	      69273,      69755,      70240,      70728,
	      71220,      71716,      72214,      72717,
	// [1616 .. 1631]
	      73223,      73732,      74245,      74761,
	      75281,      75805,      76332,      76863,
	      77398,      77936,      78478,      79024,
	      79573,      80127,      80684,      81245,
	// [1632 .. 1647]
	      81811,      82380,      82953,      83530,
	      84111,      84696,      85285,      85878,
	      86475,      87077,      87682,      88292,
	      88906,      89525,      90148,      90775,
	// [1648 .. 1663]
	      91406,      92042,      92682,      93327,
	      93976,      94629,      95288,      95950,
	      96618,      97290,      97966,      98648,
	      99334,     100025,     100721,     101421,
	// [1664 .. 1679]
	     102127,     102837,     103552,     104273,
	     104998,     105728,     106464,     107204,
	     107950,     108701,     109457,     110218,
	     110985,     111757,     112534,     113317,
	// [1680 .. 1695]
	     114105,     114898,     115698,     116502,
	     117313,     118129,     118950,     119778,
	     120611,     121450,     122295,     123145,
	     124002,     124864,     125733,     126607,
	// [1696 .. 1711]
	     127488,     128375,     129267,     130167,
	     131072,     131984,     132902,     133826,
	     134757,     135694,     136638,     137588,
	     138545,     139509,     140479,     141457,
	// [1712 .. 1727]
	     142441,     143431,     144429,     145433,
	     146445,     147464,     148489,     149522,
	     150562,     151609,     152664,     153726,
	     154795,     155872,     156956,     158048,
	// [1728 .. 1743]
	     159147,     160254,     161369,     162491,
	     163621,     164759,     165905,     167059,
	     168221,     169391,     170569,     171756,
	     172951,     174154,     175365,     176585,
	// [1744 .. 1759]
	     177813,     179050,     180295,     181549,
	     182812,     184083,     185364,     186653,
	     187951,     189259,     190575,     191901,
	     193235,     194579,     195933,     197296,
	// [1760 .. 1775]
	     198668,     200050,     201441,     202842,
	     204253,     205674,     207105,     208545,
	     209996,     211456,     212927,     214408,
	     215899,     217401,     218913,     220436,
	// [1776 .. 1791]
	     221969,     223513,     225068,     226633,
	     228210,     229797,     231395,     233005,
	     234625,     236257,     237901,     239555,
	     241222,     242900,     244589,     246290,
	// [1792 .. 1807]
	     248003,     249728,     251465,     253214,
	     254976,     256749,     258535,     260333,
	     262144,     263967,     265803,     267652,
	     269514,     271388,     273276,     275177,
	// [1808 .. 1823]
	     277091,     279018,     280959,     282913,
	     284881,     286863,     288858,     290867,
	     292890,     294927,     296979,     299044,
	     301124,     303219,     305328,     307452,
	// [1824 .. 1839]
	     309590,     311744,     313912,     316095,
	     318294,     320508,     322737,     324982,
	     327242,     329519,     331810,     334118,
	     336442,     338783,     341139,     343512,
	// [1840 .. 1855]
	     345901,     348307,     350730,     353169,
	     355626,     358099,     360590,     363098,
	     365624,     368167,     370728,     373306,
	     375903,     378517,     381150,     383801,
	// [1856 .. 1871]
	     386471,     389159,     391866,     394591,
	     397336,     400100,     402883,     405685,
	     408507,     411348,     414209,     417090,
	     419991,     422913,     425854,     428816,
	// [1872 .. 1887]
	     431799,     434802,     437827,     440872,
	     443938,     447026,     450136,     453266,
	     456419,     459594,     462791,     466010,
	     469251,     472515,     475801,     479111,
	// [1888 .. 1903]
	     482443,     485799,     489178,     492581,
	     496007,     499457,     502931,     506429,
	     509951,     513498,     517070,     520666,
	     524288,     527935,     531607,     535304,
	// [1904 .. 1919]
	     539028,     542777,     546552,     550354,
	     554182,     558037,     561918,     565826,
	     569762,     573725,     577716,     581734,
	     585780,     589855,     593957,     598089,
	// [1920 .. 1935]
	     602249,     606438,     610656,     614903,
	     619180,     623487,     627824,     632191,
	     636588,     641016,     645474,     649964,
	     654485,     659037,     663621,     668237,
	// [1936 .. 1951]
	     672885,     677565,     682278,     687024,
	     691802,     696614,     701459,     706338,
	     711251,     716199,     721180,     726196,
	     731247,     736334,     741455,     746612,
	// [1952 .. 1967]
	     751806,     757035,     762300,     767603,
	     772942,     778318,     783732,     789183,
	     794672,     800199,     805765,     811370,
	     817013,     822696,     828418,     834180,
	// [1968 .. 1983]
	     839983,     845825,     851708,     857632,
	     863598,     869605,     875653,     881744,
	     887877,     894052,     900271,     906533,
	     912838,     919188,     925581,     932019,
	// [1984 .. 1999]
	     938502,     945030,     951603,     958222,
	     964887,     971598,     978356,     985161,
	     992013,     998913,    1005861,    1012858,
	    1019903,    1026997,    1034140,    1041333,
	// [2000 .. 2015]
	    1048576,    1055869,    1063214,    1070609,
	    1078055,    1085554,    1093105,    1100708,
	    1108364,    1116073,    1123836,    1131653,
	    1139524,    1147450,    1155431,    1163468,
	// [2016 .. 2031]
	    1171560,    1179709,    1187915,    1196177,
	    1204498,    1212875,    1221312,    1229807,
	    1238361,    1246974,    1255647,    1264381,
	    1273176,    1282031,    1290948,    1299928,
	// [2032 .. 2047]
	    1308969,    1318074,    1327242,    1336474,
	    1345770,    1355130,    1364556,    1374047,
	    1383604,    1393228,    1402919,    1412677,
	    1422503,    1432397,    1442360,    1452393,
	// [2048 .. 2063]
	    1462495,    1472667,    1482910,    1493225,
	    1503611,    1514070,    1524601,    1535205,
	    1545883,    1556636,    1567463,    1578366,
	    1589344,    1600399,    1611530,    1622740,
	// [2064 .. 2079]
	    1634027,    1645392,    1656837,    1668361,
	    1679965,    1691650,    1703417,    1715265,
	    1727196,    1739209,    1751306,    1763488,
	    1775754,    1788105,    1800542,    1813066,
	// [2080 .. 2095]
	    1825677,    1838375,    1851162,    1864038,
	    1877004,    1890059,    1903206,    1916443,
	    1929773,    1943196,    1956712,    1970322,
	    1984027,    1997827,    2011723,    2025715,
	// [2096 .. 2111]
	    2039805,    2053993,    2068280,    2082666,
	    2097152,    2111739,    2126427,    2141218,
	    2156111,    2171108,    2186209,    2201415,
	    2216727,    2232146,    2247672,    2263306,
	// [2112 .. 2127]
	    2279048,    2294900,    2310863,    2326936,
	    2343121,    2359419,    2375830,    2392355,
	    2408995,    2425751,    2442623,    2459613,
	    2476721,    2493948,    2511295,    2528762,
	// [2128 .. 2143]
	    2546351,    2564063,    2581897,    2599855,
	    2617939,    2636148,    2654484,    2672947,
	    2691539,    2710260,    2729112,    2748094,
	    2767209,    2786456,    2805837,    2825354,
	// [2144 .. 2159]
	    2845005,    2864794,    2884720,    2904785,
	    2924989,    2945334,    2965821,    2986450,
	    3007222,    3028139,    3049201,    3070410,
	    3091767,    3113272,    3134926,    3156731,
	// [2160 .. 2175]
	    3178688,    3200798,    3223061,    3245479,
	    3268053,    3290784,    3313673,    3336722,
	    3359931,    3383301,    3406833,    3430530,
	    3454391,    3478418,    3502613,    3526975,
	// [2176 .. 2191]
	    3551507,    3576210,    3601084,    3626132,
	    3651354,    3676751,    3702325,    3728076,
	    3754007,    3780118,    3806411,    3832887,
	    3859547,    3886392,    3913424,    3940644,
	// [2192 .. 2207]
	    3968053,    3995653,    4023445,    4051431,
	    4079611,    4107986,    4136560,    4165332,
	    4194304,    4223478,    4252854,    4282435,
	    4312222,    4342216,    4372418,    4402831,
	// [2208 .. 2223]
	    4433455,    4464292,    4495344,    4526611,
	    4558096,    4589800,    4621725,    4653872,
	    4686242,    4718837,    4751659,    4784710,
	    4817990,    4851502,    4885247,    4919226,
	// [2224 .. 2239]
	    4953442,    4987896,    5022590,    5057525,
	    5092702,    5128125,    5163794,    5199711,
	    5235878,    5272296,    5308968,    5345895,
	    5383078,    5420521,    5458223,    5496188,
	// [2240 .. 2255]
	    5534417,    5572912,    5611675,    5650707,
	    5690011,    5729588,    5769441,    5809570,
	    5849979,    5890669,    5931642,    5972899,
	    6014444,    6056278,    6098403,    6140820,
	// [2256 .. 2271]
	    6183533,    6226543,    6269852,    6313462,
	    6357376,    6401595,    6446122,    6490958,
	    6536106,    6581568,    6627347,    6673444,
	    6719861,    6766602,    6813667,    6861060,
	// [2272 .. 2287]
	    6908782,    6956837,    7005225,    7053950,
	    7103015,    7152420,    7202169,    7252264,
	    7302707,    7353502,    7404649,    7456153,
	    7508014,    7560237,    7612822,    7665774,
	// [2288 .. 2303]
	    7719093,    7772784,    7826848,    7881288,
	    7936107,    7991307,    8046891,    8102861,
	    8159221,    8215973,    8273120,    8330664,
	    8388608,    8446955,    8505709,    8564870,
	// [2304 .. 2319]
	    8624444,    8684432,    8744837,    8805662,
	    8866910,    8928584,    8990687,    9053223,
	    9116193,    9179601,    9243450,    9307743,
	    9372484,    9437675,    9503319,    9569420,
	// [2320 .. 2335]
	    9635980,    9703004,    9770493,    9838453,
	    9906884,    9975792,   10045179,   10115049,
	   10185405,   10256250,   10327588,   10399422,
	   10471756,   10544592,   10617936,   10691789,
	// [2336 .. 2351]
	   10766157,   10841041,   10916447,   10992377,
	   11068835,   11145824,   11223350,   11301414,
	   11380022,   11459176,   11538881,   11619140,
	   11699958,   11781338,   11863283,   11945799,
	// [2352 .. 2367]
	   12028888,   12112556,   12196805,   12281641,
	   12367067,   12453086,   12539704,   12626925,
	   12714752,   12803190,   12892243,   12981916,
	   13072212,   13163137,   13254694,   13346887,
	// [2368 .. 2383]
	   13439722,   13533203,   13627334,   13722120,
	   13817564,   13913673,   14010450,   14107901,
	   14206029,   14304840,   14404338,   14504528,
	   14605415,   14707004,   14809299,   14912306,
	// [2384 .. 2399]
	   15016029,   15120474,   15225645,   15331548,
	   15438187,   15545568,   15653696,   15762576,
	   15872213,   15982613,   16093781,   16205722,
	   16318442,   16431946,   16546239,   16661327,
	// [2400 .. 2415]
	   16777216,   16893911,   17011417,   17129741,
	   17248888,   17368863,   17489673,   17611324,
	   17733820,   17857168,   17981375,   18106445,
	   18232386,   18359202,   18486900,   18615487,
	// [2416 .. 2431]
	   18744968,   18875349,   19006638,   19138839,
	   19271960,   19406008,   19540987,   19676905,
	   19813769,   19951585,   20090359,   20230098,
	   20370810,   20512500,   20655176,   20798844,
	// [2432 .. 2447]
	   20943511,   21089185,   21235872,   21383579,
	   21532314,   21682083,   21832893,   21984753,
	   22137669,   22291649,   22446700,   22602829,
	   22760044,   22918352,   23077762,   23238281,
	// [2448 .. 2463]
	   23399916,   23562675,   23726566,   23891598,
	   24057777,   24225112,   24393611,   24563282,
	   24734133,   24906173,   25079409,   25253850,
	   25429504,   25606380,   25784487,   25963832,
	// [2464 .. 2479]
	   26144425,   26326274,   26509387,   26693775,
	   26879445,   27066406,   27254668,   27444239,
	   27635129,   27827346,   28020901,   28215802,
	   28412058,   28609679,   28808676,   29009056,
	// [2480 .. 2495]
	   29210830,   29414007,   29618598,   29824611,
	   30032058,   30240947,   30451290,   30663095,
	   30876374,   31091136,   31307392,   31525152,
	   31744427,   31965227,   32187563,   32411445,
	// [2496 .. 2511]
	   32636884,   32863892,   33092478,   33322655,
	   33554432,   33787822,   34022834,   34259482,
	   34497775,   34737726,   34979346,   35222647,
	   35467640,   35714337,   35962750,   36212890,
	// [2512 .. 2527]
	   36464771,   36718404,   36973800,   37230973,
	   37489935,   37750698,   38013275,   38277679,
	   38543921,   38812015,   39081974,   39353811,
	   39627538,   39903169,   40180718,   40460197,
	// [2528 .. 2543]
	   40741620,   41025000,   41310351,   41597688,
	   41887023,   42178370,   42471744,   42767158,
	   43064627,   43364165,   43665787,   43969506,
	   44275338,   44583298,   44893399,   45205657,
	// [2544 .. 2559]
	   45520088,   45836705,   46155524,   46476561,
	   46799832,   47125350,   47453133,   47783195,
	   48115554,   48450224,   48787222,   49126564,
	   49468266,   49812345,   50158817,   50507700,
	// [2560 .. 2575]
	   50859008,   51212761,   51568974,   51927664,
	   52288850,   52652548,   53018775,   53387550,
	   53758889,   54132812,   54509336,   54888478,
	   55270258,   55654693,   56041802,   56431603,
	// [2576 .. 2591]
	   56824116,   57219359,   57617351,   58018111,
	   58421659,   58828014,   59237195,   59649223,
	   60064116,   60481895,   60902580,   61326191,
	   61752748,   62182272,   62614784,   63050304,
	// [2592 .. 2607]
	   63488854,   63930454,   64375125,   64822890,
	   65273769,   65727784,   66184956,   66645309,
	   67108864,   67575643,   68045669,   68518964,
	   68995551,   69475453,   69958693,   70445294,
	// [2608 .. 2623]
	   70935280,   71428674,   71925500,   72425781,
	   72929542,   73436807,   73947601,   74461947,
	   74979871,   75501397,   76026551,   76555357,
	   77087842,   77624030,   78163948,   78707621,
	// [2624 .. 2639]
	   79255076,   79806339,   80361436,   80920394,
	   81483239,   82050000,   82620703,   83195375,
	   83774045,   84356740,   84943487,   85534316,
	   86129254,   86728330,   87331574,   87939013,
	// [2640 .. 2655]
	   88550677,   89166596,   89786798,   90411315,
	   91040175,   91673410,   92311049,   92953123,
	   93599663,   94250700,   94906266,   95566391,
	   96231108,   96900448,   97574444,   98253128,
	// [2656 .. 2671]
	   98936532,   99624690,  100317635,  101015399,
	  101718017,  102425522,  103137948,  103855329,
	  104577700,  105305095,  106037550,  106775099,
	  107517779,  108265624,  109018671,  109776956,
	// [2672 .. 2687]
	  110540515,  111309385,  112083603,  112863206,
	  113648232,  114438718,  115234702,  116036223,
	  116843319,  117656028,  118474391,  119298445,
	  120128232,  120963789,  121805159,  122652381,
	// [2688 .. 2703]
	  123505496,  124364544,  125229568,  126100609,
	  126977708,  127860908,  128750251,  129645779,
	  130547537,  131455567,  132369913,  133290618,
	  134217728,  135151286,  136091338,  137037928,
	// [2704 .. 2719]
	  137991102,  138950906,  139917386,  140890588,
	  141870560,  142857348,  143850999,  144851562,
	  145859084,  146873614,  147895201,  148923894,
	  149959741,  151002794,  152053101,  153110714,
	// [2720 .. 2735]
	  154175683,  155248060,  156327896,  157415242,
	  158510152,  159612677,  160722871,  161840787,
	  162966479,  164100000,  165241406,  166390751,
	  167548090,  168713479,  169886974,  171068632,
	// [2736 .. 2751]
	  172258508,  173456661,  174663147,  175878025,
	  177101354,  178333191,  179573597,  180822630,
	  182080351,  183346820,  184622098,  185906246,
	  187199326,  188501400,  189812531,  191132782,
	// [2752 .. 2767]
	  192462215,  193800896,  195148888,  196506256,
	  197873065,  199249381,  200635270,  202030799,
	  203436034,  204851043,  206275895,  207710657,
	  209155399,  210610190,  212075100,  213550199,
	// [2768 .. 2783]
	  215035558,  216531248,  218037342,  219553912,
	  221081030,  222618770,  224167206,  225726413,
	  227296464,  228877436,  230469404,  232072446,
	  233686637,  235312057,  236948781,  238596890,
	// [2784 .. 2799]
	  240256463,  241927579,  243610318,  245304762,
	  247010992,  248729089,  250459137,  252201218,
	  253955416,  255721815,  257500501,  259291559,
	  261095074,  262911134,  264739826,  266581237,
	// [2800 .. 2815]
	  268435456,  270302572,  272182675,  274075856,
	  275982204,  277901812,  279834772,  281781177,
	  283741120,  285714695,  287701998,  289703124,
	  291718168,  293747229,  295790402,  297847787,
	// [2816 .. 2831]
	  299919482,  302005587,  304106202,  306221428,
	  308351367,  310496120,  312655791,  314830484,
	  317020304,  319225354,  321445742,  323681574,
	  325932957,  328200000,  330482812,  332781502,
	// [2832 .. 2847]
	  335096180,  337426958,  339773948,  342137263,
	  344517016,  346913321,  349326294,  351756051,
	  354202708,  356666382,  359147193,  361645260,
	  364160701,  366693639,  369244195,  371812492,
	// [2848 .. 2863]
	  374398652,  377002801,  379625062,  382265564,
	  384924431,  387601792,  390297776,  393012511,
	  395746130,  398498762,  401270540,  404061597,
	  406872068,  409702087,  412551790,  415421315,
	// [2864 .. 2879]
	  418310798,  421220380,  424150200,  427100398,
	  430071116,  433062497,  436074685,  439107824,
	  442162061,  445237541,  448334413,  451452825,
	  454592928,  457754872,  460938809,  464144892,
	// [2880 .. 2895]
	  467373275,  470624113,  473897563,  477193781,
	  480512926,  483855158,  487220637,  490609524,
	  494021983,  497458178,  500918273,  504402435,
	  507910832,  511443631,  515001003,  518583118,
	// [2896 .. 2911]
	  522190149,  525822268,  529479652,  533162474,
	  536870912,  540605145,  544365351,  548151711,
	  551964408,  555803624,  559669544,  563562353,
	  567482239,  571429391,  575403996,  579406248,
	// [2912 .. 2927]
	  583436337,  587494457,  591580804,  595695574,
	  599838965,  604011175,  608212405,  612442857,
	  616702733,  620992240,  625311583,  629660969,
	  634040607,  638450708,  642891484,  647363148,
	// [2928 .. 2943]
	  651865915,  656400001,  660965624,  665563003,
	  670192360,  674853917,  679547897,  684274526,
	  689034032,  693826643,  698652589,  703512102,
	  708405415,  713332765,  718294387,  723290519,
	// [2944 .. 2959]
	  728321402,  733387278,  738488390,  743624983,
	  748797304,  754005601,  759250125,  764531127,
	  769848862,  775203584,  780595551,  786025023,
	  791492259,  796997523,  802541079,  808123194,
	// [2960 .. 2975]
	  813744135,  819404173,  825103580,  830842629,
	  836621597,  842440760,  848300399,  854200795,
	  860142232,  866124994,  872149370,  878215648,
	  884324121,  890475082,  896668826,  902905651,
	// [2976 .. 2991]
	  909185856,  915509744,  921877618,  928289784,
	  934746550,  941248226,  947795125,  954387562,
	  961025852,  967710316,  974441273,  981219048,
	  988043966,  994916356, 1001836546, 1008804870,
	// [2992 .. 3007]
	 1015821663, 1022887262, 1030002005, 1037166236,
	 1044380297, 1051644537, 1058959303, 1066324947,
	 1073741824, 1081210289, 1088730701, 1096303422,
	 1103928816, 1111607248, 1119339088, 1127124707,
	// [3008 .. 3023]
	 1134964479, 1142858781, 1150807993, 1158812495,
	 1166872673, 1174988915, 1183161609, 1191391149,
	 1199677930, 1208022349, 1216424809, 1224885713,
	 1233405467, 1241984480, 1250623166, 1259321938,
	// [3024 .. 3039]
	 1268081214, 1276901417, 1285782968, 1294726296,
	 1303731830, 1312800002, 1321931248, 1331126007,
	 1340384721, 1349707834, 1359095794, 1368549053,
	 1378068064, 1387653286, 1397305178, 1407024204,
	// [3040 .. 3055]
	 1416810831, 1426665530, 1436588773, 1446581038,
	 1456642805, 1466774557, 1476976781, 1487249967,
	 1497594608, 1508011203, 1518500250, 1529062254,
	 1539697724, 1550407168, 1561191103, 1572050046,
	// [3056 .. 3071]
	 1582984518, 1593995046, 1605082159, 1616246388,
	 1627488271, 1638808347, 1650207160, 1661685259,
	 1673243194, 1684881521, 1696600798, 1708401590,
	 1720284463, 1732249988, 1744298739, 1756431296,
	// [3072 .. 3087]
	 1768648242, 1780950164, 1793337652, 1805811301,
	 1818371712, 1831019488, 1843755235, 1856579567,
	 1869493099, 1882496452, 1895590251, 1908775123,
	 1922051704, 1935420631, 1948882546, 1962438096,
	// [3088 .. 3103]
	 1976087933, 1989832711, 2003673092, 2017609741,
	 2031643326, 2045774523, 2060004010, 2074332471,
	 2088760595, 2103289074, 2117918606, 2132649895,
	 2147483648, 2162420578, 2177461403, 2192606844,
	// [3104 .. 3119]
	 2207857631, 2223214495, 2238678175, 2254249413,
	 2269928958, 2285717562, 2301615985, 2317624990,
	 2333745347, 2349977830, 2366323218, 2382782298,
	 2399355859, 2416044699, 2432849619, 2449771426,
	// [3120 .. 3135]
	 2466810934, 2483968961, 2501246331, 2518643875,
	 2536162429, 2553802834, 2571565937, 2589452593,
	 2607463660, 2625600004, 2643862496, 2662252014,
	 2680769441, 2699415667, 2718191588, 2737098106,
	// [3136 .. 3151]
	 2756136128, 2775306571, 2794610355, 2814048407,
	 2833621662, 2853331059, 2873177546, 2893162076,
	 2913285610, 2933549114, 2953953562, 2974499933,
	 2995189217, 3016022405, 3037000500, 3058124509,
	// [3152 .. 3167]
	 3079395447, 3100814336, 3122382206, 3144100091,
	 3165969037, 3187990093, 3210164318, 3232492776,
	 3254976542, 3277616694, 3300414321, 3323370518,
	 3346486388, 3369763041, 3393201597, 3416803181,
	// [3168 .. 3183]
	 3440568926, 3464499975, 3488597478, 3512862593,
	 3537296484, 3561900327, 3586675303, 3611622603,
	 3636743425, 3662038976, 3687510471, 3713159135,
	 3738986199, 3764992905, 3791180501, 3817550247,
	// [3184 .. 3199]
	 3844103409, 3870841262, 3897765093, 3924876193,
	 3952175866, 3979665423, 4007346185, 4035219482,
	 4063286653, 4091549047, 4120008021, 4148664943,
	 4177521189, 4206578147, 4235837212, 4265299790
    };

} }

static int hash_size_length =
    sizeof ( MINT::hash_size ) / sizeof ( min::uns32 );

const min::unsptr min::SHORT_OBJ_MAX_VAR_SIZE =
    MINT::SHORT_OBJ_MAX_VAR_SIZE;
const min::unsptr min::SHORT_OBJ_MAX_HASH_SIZE =
    MINT::hash_size
        [MINT::SHORT_OBJ_MAX_HASH_SIZE_CODE];
const min::unsptr min::SHORT_OBJ_MAX_TOTAL_SIZE =
    MINT::SHORT_OBJ_MAX_TOTAL_SIZE;
const min::unsptr min::LONG_OBJ_MAX_VAR_SIZE =
    MINT::LONG_OBJ_MAX_VAR_SIZE;
const min::unsptr min::LONG_OBJ_MAX_HASH_SIZE =
    MINT::hash_size[hash_size_length - 1];
const min::unsptr min::LONG_OBJ_MAX_TOTAL_SIZE =
    MINT::LONG_OBJ_MAX_TOTAL_SIZE;

bool min::use_obj_aux_stubs = false;

min::unsptr min::obj_var_size ( min::unsptr u )
{
    if ( u > MINT::LONG_OBJ_MAX_VAR_SIZE )
        return MINT::LONG_OBJ_MAX_VAR_SIZE;
    else
        return u;
}

min::unsptr min::obj_hash_size ( min::unsptr u )
{
    int lo = 0, hi = hash_size_length - 1;
    if ( u <= 1 ) hi = u;
    else if ( u < min::LONG_OBJ_MAX_HASH_SIZE )
	while ( true )
	{
	    // Invariants:
	    //
	    //    MINT::hash_size[hi] >= u
	    //    MINT::hash_size[lo] < u
	    //
	    int mid = ( lo + hi ) / 2;
	    if ( MINT::hash_size[mid] >= u ) hi = mid;
	    else if ( lo == mid ) break;
	    else lo = mid;
	}
    return MINT::hash_size[hi];
}

min::unsptr min::obj_total_size ( min::unsptr u )
{
    unsigned exponent = 0;
    if ( u > MINT::LONG_OBJ_MAX_TOTAL_SIZE )
        u = MINT::LONG_OBJ_MAX_TOTAL_SIZE;
    else
    {
	while ( u > MINT::LONG_OBJ_MANTISSA_MASK )
	    ++ u, u >>= 1, ++ exponent;
	u <<= exponent;
    }
    return u;
}

min::gen min::new_obj_gen
	    ( min::unsptr unused_size,
	      min::unsptr hash_size,
	      min::unsptr var_size )
{
    int lo = 0, hi = hash_size_length - 1;
    if ( hash_size <= 1 ) hi = hash_size;
    else if ( hash_size < min::LONG_OBJ_MAX_HASH_SIZE )
	while ( true )
	{
	    // Invariants:
	    //
	    //    MINT::hash_size[hi] >= u
	    //    MINT::hash_size[lo] < u
	    //
	    int mid = ( lo + hi ) / 2;
	    if ( MINT::hash_size[mid] >= hash_size )
	        hi = mid;
	    else if ( lo == mid )
	        break;
	    else
	        lo = mid;
	}

    hash_size = MINT::hash_size[hi];

    min::unsptr total_size =
        unused_size + hash_size + var_size;

    min::stub * s = MUP::new_acc_stub();
    min::gen * p;
    int type;
    if ( var_size <= MINT::SHORT_OBJ_MAX_VAR_SIZE
         &&
	 hi <= MINT::SHORT_OBJ_MAX_HASH_SIZE_CODE
	 &&
	    total_size + MINT::SHORT_OBJ_HEADER_SIZE
         <= MINT::SHORT_OBJ_MAX_TOTAL_SIZE )
    {
        total_size += MINT::SHORT_OBJ_HEADER_SIZE;

	unsigned exponent = 0;
	min::unsptr mantissa = total_size;
	while (   mantissa
	        > MINT::SHORT_OBJ_MANTISSA_MASK )
	    ++ mantissa, mantissa >>= 1, ++ exponent;
	total_size = mantissa << exponent;

	type = min::SHORT_OBJ;
	MUP::new_body
	    ( s, sizeof (min::gen) * total_size );
	MINT::short_obj * so = MINT::short_obj_of ( s );

	so->flags =
	      (   (    exponent
	            << MINT::SHORT_OBJ_MANTISSA_BITS )
	        + mantissa - 1 )
	    << MINT::SHORT_OBJ_FLAG_BITS;
	so->codes =
	      (    var_size
	        << MINT::SHORT_OBJ_HASH_CODE_BITS )
	    + hi;
	so->unused_offset = MINT::SHORT_OBJ_HEADER_SIZE
	                  + var_size
			  + hash_size;
	so->aux_offset = total_size;

	p = (min::gen *) so
	  + MINT::SHORT_OBJ_HEADER_SIZE;
    }
    else
    {
        total_size += MINT::LONG_OBJ_HEADER_SIZE;

	MIN_ASSERT
	    ( var_size <= MINT::LONG_OBJ_MAX_VAR_SIZE
	      &&
	      hi <= MINT::LONG_OBJ_MAX_HASH_SIZE_CODE
	      &&
	         total_size
	      <= MINT::LONG_OBJ_MAX_TOTAL_SIZE );

	unsigned exponent = 0;
	min::unsptr mantissa = total_size;
	while (   mantissa
	        > MINT::LONG_OBJ_MANTISSA_MASK )
	    ++ mantissa, mantissa >>= 1, ++ exponent;
	total_size = mantissa << exponent;

	type = min::LONG_OBJ;
	MUP::new_body
	    ( s, sizeof (min::gen) * total_size );
	MINT::long_obj * lo = MINT::long_obj_of ( s );

	lo->flags =
	      (   (    exponent
	            << MINT::LONG_OBJ_MANTISSA_BITS )
	        + mantissa - 1 )
	    << MINT::LONG_OBJ_FLAG_BITS;
	lo->codes =
	      (    var_size
	        << MINT::LONG_OBJ_HASH_CODE_BITS )
	    + hi;
	lo->unused_offset = MINT::LONG_OBJ_HEADER_SIZE
	                  + var_size
			  + hash_size;
	lo->aux_offset = total_size;

	p = (min::gen *) lo
	  + MINT::LONG_OBJ_HEADER_SIZE;
    }

    min::gen * endp = p + var_size;
    while ( p < endp ) * p ++ = min::UNDEFINED;
    endp += hash_size;
    while ( p < endp ) * p ++ = min::LIST_END;

    MUP::set_type_of ( s, type );
    return min::new_gen ( s );
}

// Object Vector Level
// ------ ------ -----

static const min::unsptr OBJ_HEADER_SIZE_DIFFERENCE =
       sizeof ( min::gen )
     * (   MINT::LONG_OBJ_HEADER_SIZE
	 - MINT::SHORT_OBJ_HEADER_SIZE );

bool min::resize
    ( min::insertable_vec_pointer & vp,
      min::unsptr unused_size,
      min::unsptr var_size )
{
    min::stub * s = MUP::stub_of ( vp );
    min::unsptr old_size = min::total_size_of ( vp );
    min::unsptr new_size =
        ( unused_size + var_size + old_size )
	-
	(   min::var_size_of ( vp )
	  + min::unused_size_of ( vp ) );

    int new_type = min::type_of ( s );
    min::unsptr hash_code;
    min::uns32 flags;
    if ( new_type == min::SHORT_OBJ )
    {
	MINT::short_obj * so = MINT::short_obj_of ( s );
	hash_code =   so->codes
	            & MINT::SHORT_OBJ_HASH_CODE_MASK;
        flags =   so->flags
	        & MINT::SHORT_OBJ_FLAG_MASK;

	if ( var_size > MINT::SHORT_OBJ_MAX_VAR_SIZE
	     ||
		new_size
	     > MINT::SHORT_OBJ_MAX_TOTAL_SIZE )
	{
	     new_type = min::LONG_OBJ;
	     new_size += OBJ_HEADER_SIZE_DIFFERENCE;
	}
    }
    else
    {
	MIN_ASSERT ( new_type == min::LONG_OBJ );
	MINT::long_obj * lo = MINT::long_obj_of ( s );
	hash_code =   lo->codes
	            & MINT::LONG_OBJ_HASH_CODE_MASK;
        flags =   lo->flags
	        & MINT::LONG_OBJ_FLAG_MASK;

	if ( var_size <= MINT::SHORT_OBJ_MAX_VAR_SIZE
	     &&
	        hash_code
	     <= MINT::SHORT_OBJ_MAX_HASH_SIZE_CODE
	     &&
		new_size
	     <=   MINT::SHORT_OBJ_MAX_TOTAL_SIZE
	        + OBJ_HEADER_SIZE_DIFFERENCE )
	{
	     new_type = min::SHORT_OBJ;
	     new_size -= OBJ_HEADER_SIZE_DIFFERENCE;
	}
    }

    unsigned exponent = 0;
    min::unsptr mantissa = new_size;
    min::unsptr new_var_offset;
    if ( new_type == min::SHORT_OBJ )
    {
	MIN_ASSERT
	    (    flags
	      == ( flags & MINT::SHORT_OBJ_FLAG_MASK )
	    );
	new_var_offset = MINT::SHORT_OBJ_HEADER_SIZE;
	while (   mantissa
	        > MINT::SHORT_OBJ_MANTISSA_MASK )
	    ++ mantissa, mantissa >>= 1, ++ exponent;
    }
    else
    {
	MIN_ASSERT
	    ( var_size <= MINT::LONG_OBJ_MAX_VAR_SIZE
	      &&
	         new_size
	      <= MINT::LONG_OBJ_MAX_TOTAL_SIZE );
	new_var_offset = MINT::LONG_OBJ_HEADER_SIZE;
	while (   mantissa
	        > MINT::LONG_OBJ_MANTISSA_MASK )
	    ++ mantissa, mantissa >>= 1, ++ exponent;
    }
    min::unsptr initial_new_size = new_size;
    new_size = mantissa << exponent;
    unused_size += new_size - initial_new_size;

    MUP::resize_body r
        ( s, new_size * sizeof ( min::gen),
	     old_size * sizeof ( min::gen) );
    MUP::retype_resize_body ( r, new_type );

    min::gen * & oldb = MUP::base ( vp );
    min::gen * & newb = * ( min::gen **) &
	MUP::new_body_pointer_ref ( r );

    // Compute aux pointer offset.
    //
    min::unsgen aux_offset = (min::unsgen) new_size
	                   - (min::unsgen) old_size;

    // Initialize copy pointers.
    //
    min::unsptr from = MUP::var_offset_of ( vp );
    min::unsptr to = new_var_offset;

    // Copy variables vector.
    //
    min::unsptr from_end =
        from + min::var_size_of ( vp );
    min::unsptr to_end = to + var_size;

    while ( from < from_end && to < to_end )
        newb[to++] = oldb[from++];
    while ( to < to_end )
        newb[to++] = min::UNDEFINED;
    from = from_end;

    // Copy hash table and attribute vector.
    //
    from_end = from + min::hash_size_of ( vp )
                    + min::attr_size_of ( vp );
    while ( from < from_end )
        newb[to++] = oldb[from++];

    // Initialize unused area.
    //
    from += min::unused_size_of ( vp );
    memset ( & newb[to], 0, unused_size );
    to += unused_size;

    // Copy auxiliary area.
    //
    from_end = from + min::aux_size_of ( vp );
    while ( from < from_end )
        newb[to++] = oldb[from++];

    MIN_ASSERT ( from == old_size );
    MIN_ASSERT ( to == new_size );

    // Fix vector pointer.
    //
    vp.var_offset = new_var_offset;
    vp.hash_offset = vp.var_offset + var_size;
    min::unsptr attr_size = vp.unused_offset
                          - vp.attr_offset;
    vp.attr_offset = vp.hash_offset
                   + vp.hash_size;
    vp.unused_offset = vp.attr_offset
                     + attr_size;
    vp.aux_offset = vp.unused_offset
		  + unused_size;
    vp.total_size = new_size;

    // Fill in header.
    //
    if ( new_type == min::SHORT_OBJ )
    {
        MINT::short_obj * so = (MINT::short_obj *) newb;
	so->codes =
	      (    var_size
	        << MINT::SHORT_OBJ_HASH_CODE_BITS )
	    + hash_code;
	flags +=
	       (  (    exponent
	            << MINT::SHORT_OBJ_MANTISSA_BITS )
	        + mantissa - 1 )
	    << MINT::SHORT_OBJ_FLAG_BITS;
	so->flags = (min::uns16) flags;
	so->unused_offset =
	    (min::uns16) vp.unused_offset;
	so->aux_offset =
	    (min::uns16) vp.aux_offset;
    }
    else
    {
        MINT::long_obj * lo = (MINT::long_obj *) newb;
	lo->codes =
	      (    var_size
	        << MINT::LONG_OBJ_HASH_CODE_BITS )
	    + hash_code;
	flags +=
	       (  (    exponent
	            << MINT::LONG_OBJ_MANTISSA_BITS )
	        + mantissa - 1 )
	    << MINT::LONG_OBJ_FLAG_BITS;
	lo->flags = flags;
	lo->unused_offset = vp.unused_offset;
	lo->aux_offset = vp.aux_offset;
    }

    // Currently `resize' always relocates.
    //
    return true;
}

bool min::resize
    ( min::insertable_vec_pointer & vp,
      min::unsptr unused_size )
{
    return min::resize ( vp, unused_size,
                         min::var_size_of ( vp ) );
}

// Object List Level
// ------ ---- -----

# if MIN_USE_OBJ_AUX_STUBS

// Allocate a chain of aux stubs containing the n
// min::gen values in p.  The type of the first stub is
// given and the other stubs have type min::LIST_AUX.
// Each stub but the last points at the next stub.  The
// control of the last, except for its type field,
// equals the end value, which may be a list aux value
// or a pointer to a stub.
//
// This function returns pointers to the first and last
// stubs allocated.  n > 0 is required.
//
// This function asserts that the relocated flag is not
// set during the execution of this function.  Suffi-
// cient stubs should have been reserved in advance.
//
void MINT::allocate_stub_list
	( min::stub * & first,
	  min::stub * & last,
	  int type,
	  const min::gen * p, min::unsptr n,
	  min::uns64 end )
{
    MIN_ASSERT ( n > 0 );

    // Check for failure to use min::insert_reserve
    // properly.
    //
    bool saved_relocated_flag =
        min::set_relocated_flag ( false );

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
	       MUP::new_control_with_type
	           ( type, last, MUP::STUB_POINTER ) );
	type = min::LIST_AUX;
	previous = last;
    }
    MUP::set_control_of
        ( last, MUP::renew_control_type ( end, type ) );

    // Check for failure to use min::insert_reserve
    // properly.
    //
    MIN_ASSERT ( ! min::set_relocated_flag
                     ( saved_relocated_flag ) );
}

# endif // MIN_USE_OBJ_AUX_STUBS

// Remove a list.  Free any aux stubs used and
// set any auxiliary area elements use to min::NONE.
// Index/s point at the first element of the list.
// Either index != 0 and s == NULL or index == 0
// and s != NULL.  Base is the base of a vector
// pointer and total_size is its total size.
//
void MINT::remove_list
	( min::gen * & base,
	  min::unsptr total_size,
	  min::unsptr index
#	if MIN_USE_OBJ_AUX_STUBS
	  , min::stub * s // = NULL
#	endif
	)
{
    while ( true )
    {
#	if MIN_USE_OBJ_AUX_STUBS
	    if ( s != NULL )
	    {
		MINT::remove_sublist
		    ( base, total_size,
		      min::unprotected
		         ::value_of ( s ) );
		min::uns64 c = MUP::control_of ( s );
		MUP::free_aux_stub ( s );
		if ( c & MUP::STUB_POINTER)
		    s = MUP::stub_of ( c );
		else
		{
		    min::unsptr vc =
			MUP::value_of_control ( c );
		    if ( vc == 0 ) return;
		    index = total_size - c;
		    s = NULL;
		}
	    }
	    else
#	endif
	{
	    MIN_ASSERT ( index != 0 );
	    MINT::remove_sublist
	        ( base, total_size, base[index] );
	    base[index] = min::NONE;
	    min::gen v = base[--index];
#	    if MIN_USE_OBJ_AUX_STUBS
		if ( min::is_stub ( v ) )
		{
		    min::stub * s2 =
			min::unprotected::stub_of ( v );
		    if (    min::type_of ( s2 )
			 == min::LIST_AUX )
		        s = s2, index = 0;
		}
		else
#	    endif
	    if ( min::is_list_aux ( v ) )
	    {
		index = min::list_aux_of ( v  );
		if ( index == 0 ) return;
		index = total_size - index;
	    }
	}
    }
}

void min::insert_before
	( min::insertable_list_pointer & lp,
	  const min::gen * p, min::unsptr n )
{
    if ( n == 0 ) return;

    min::unsptr unused_offset =
        unprotected::unused_offset_of ( lp.vecp );
    min::unsptr aux_offset =
        unprotected::aux_offset_of ( lp.vecp );
    min::unsptr total_size = lp.total_size;
    MIN_ASSERT (    total_size
                 == min::total_size_of ( lp.vecp ) );

    MIN_ASSERT ( lp.reserved_insertions >= 1 );
    MIN_ASSERT ( lp.reserved_elements >= n );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    MUP::acc_write_update
            ( MUP::stub_of ( lp.vecp ), p, n );

    if ( lp.current == min::LIST_END )
    {
	// Contiguous means the previous pointer does
	// not exist and current_index == aux_
	// offset so we can add elements by copying them
	// into the aux area at and just before current_
	// index.
	//
	bool contiguous = false;

	// Previous_is_list_head means previous is in
	// the hash table or attribute vector.  This
	// can only happen if previous_index != 0 and
	// previous_is_sublist_head is false.
	//
	bool previous_is_list_head = false;

	// Pointer to the first new element; may replace
	// LIST_END in current or previous pointer.
	//
	min::gen fgen;

	if ( lp.previous_index != 0 )
	    previous_is_list_head =
	        ! lp.previous_is_sublist_head;
		// If previous_index != 0 and current ==
		// LIST_END then only two cases are
		// possible:
		//    1) previous is a list head
		//    2) previous is a sublist head

	else
#	if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.previous_stub == NULL )
#       endif
		contiguous =
		    ( lp.current_index == aux_offset );

#	if MIN_USE_OBJ_AUX_STUBS
	    if (    lp.use_obj_aux_stubs
		 &&     unused_offset
		      + n + ( ! contiguous )
		      + previous_is_list_head
		    > aux_offset )
	    {
	        // Not enough aux area available for
		// all the new elements, and aux stubs
		// are allowed.
		//
		min::stub * first, * last;
		MINT::allocate_stub_list
		    ( first, last,
		      lp.previous_is_sublist_head ?
			  min::SUBLIST_AUX :
		          min::LIST_AUX,
		      p, n,
		      MUP::new_control_with_type
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
			      MUP::new_control_with_type
				  ( type, first,
				    MUP::STUB_POINTER )
			    );
		    }
		}
		else if ( lp.previous_index != 0 )
		{
		    // previous is list head or sublist
		    // head, as noted above.

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
			      MUP::new_control_with_type
			        ( min::LIST_AUX,
				  first,
				  MUP::STUB_POINTER ) );
			lp.base[lp.previous_index] =
			    min::new_gen ( s );
			lp.previous_index = 0;
			lp.previous_stub = s;
		    }
		    else
			lp.base[lp.previous_index] =
			    fgen;
		}
		else
		{
		    lp.base[lp.current_index] = fgen;
		    lp.previous_index =
		        lp.current_index; 
		    lp.previous_is_sublist_head = false;
		}
		lp.current_stub = first;
		lp.current = MUP::gen_of ( first );
		lp.current_index = 0;
		return;
	    }
#	endif

	// Insertion will use aux area.

	MIN_ASSERT (      unused_offset
			+ n + ( ! contiguous )
			+ previous_is_list_head
		     <= aux_offset );

#	if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.previous_stub != NULL )
	    {
	        if ( lp.previous_is_sublist_head )
		{
		    fgen = min::new_list_aux_gen
			       (   total_size
			         - aux_offset + 1 );
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
			  MUP::new_control_with_type
			      ( type,
			          total_size
				- aux_offset + 1 ) );
		}
	    }
	    else
#	endif
	if ( lp.previous_index != 0 )
	{
	    if ( previous_is_list_head )
	    {
	        lp.base[-- aux_offset] =
		    lp.base[lp.previous_index];
		lp.base[lp.previous_index] =
		    min::new_list_aux_gen
			( total_size - aux_offset );
		lp.previous_index = 0;
	    }
	    else
	    {
		lp.base[lp.previous_index] =
		    min::new_sublist_aux_gen
			( total_size - aux_offset + 1 );
	    }
	}
	else if ( contiguous )
	    ++ aux_offset;
	else
	{
	    lp.base[lp.current_index] =
		min::new_list_aux_gen
		   ( total_size - aux_offset + 1 );
	    lp.previous_index = lp.current_index;
	    lp.previous_is_sublist_head = false;
	}

	lp.current_index = aux_offset - 1;
	lp.current = p[0];
	while ( n -- )
	    lp.base[-- aux_offset] = * p ++;
	lp.base[-- aux_offset] = min::LIST_END;

	unprotected::aux_offset_of ( lp.vecp ) =
	    aux_offset;
	return;
    }

    // lp.current != min::LIST_END

    // If there is no previous, we must move the current
    // element so we can replace it with a list pointer.
    // If the current element is in an aux stub, there
    // has to be a previous.
    //
    bool previous = ( lp.previous_index != 0 );

#   if MIN_USE_OBJ_AUX_STUBS
        if ( lp.previous_stub != NULL )
	    previous = true;
	if (    lp.use_obj_aux_stubs
	     &&     unused_offset
		  + n + 1 + ( ! previous )
		> aux_offset )
	{
	    // Not enough aux area available for all the
	    // new elements, and aux stubs are allowed.
	    // Prepare to call allocate_stub_list.

	    min::stub * s;
	        // Stub to which current value is moved
		// if previous pointer does not exist.

	    min::uns64 end;
	    int type = min::LIST_AUX;
	        // Parameters for call to allocate_stub_
		// list below.

	    if ( lp.current_stub != NULL )
	    {
		type = min::type_of ( lp.current_stub );
		MUP::set_type_of
		    ( lp.current_stub, min::LIST_AUX );
		end = MUP::new_control_with_type
		   ( 0, lp.current_stub,
		     MUP::STUB_POINTER );
		MIN_ASSERT ( previous );
	    }
	    else if ( ! previous )
	    {
	        s = MUP::new_aux_stub();
		MUP::set_gen_of ( s, lp.current );
		end = MUP::new_control_with_type
		    ( 0, s, MUP::STUB_POINTER );
		min::unsptr next = lp.current_index;
		if ( next == lp.head_index )
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
		else
		    next = total_size - next;
		MUP::set_control_of
		    ( s,
		      MUP::new_control_with_type
		        ( min::LIST_AUX, next ) );
	    }
	    else
	    {
	        if ( lp.previous_is_sublist_head )
		    type = min::SUBLIST_AUX;
		end = MUP::new_control_with_type
		   ( 0, total_size - lp.current_index );
	    }

	    min::stub * first, * last;
	    MINT::allocate_stub_list
		( first, last, type, p, n, end );

	    if ( lp.previous_index != 0 )
		lp.base[lp.previous_index] =
		    min::new_gen ( first );
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
			  MUP::new_control_with_type
			      ( type, first,
				MUP::STUB_POINTER ) );
		}
	    }
	    else
	    {
	        MIN_ASSERT ( lp.current_index != 0 );

		lp.base[lp.current_index] =
		    min::new_gen ( first );
		lp.previous_index = lp.current_index;
		lp.previous_is_sublist_head = false;
	    }
	    lp.current_stub = first;
	    lp.current_index = 0;
	    lp.current = MUP::gen_of ( first );
	    return;
	}
#   endif

    // Insertion will use aux area.

    MIN_ASSERT (      unused_offset
		    + n + 1 + ( ! previous )
		 <= aux_offset );

    min::unsptr first = aux_offset - 1;
    min::unsptr aux_first = total_size - first;

    while ( n -- )
	lp.base[-- aux_offset] = * p ++;

#   if MIN_USE_OBJ_AUX_STUBS
	if ( lp.current_stub != NULL )
	{
	    MIN_ASSERT ( previous );
	    lp.base[-- aux_offset] =
		min::new_gen ( lp.current_stub );
	    MUP::set_type_of
		( lp.current_stub, min::LIST_AUX );
	}
	else
#   endif
    {
	min::unsptr next = lp.current_index;
        if ( ! previous )
	{
	    lp.base[-- aux_offset] = lp.current;
	    if ( next == lp.head_index )
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
	if ( next != 0 ) next = total_size - next;
        lp.base[-- aux_offset] =
	    min::new_list_aux_gen ( next );
    }

#   if MIN_USE_OBJ_AUX_STUBS
	if ( lp.previous_stub != NULL )
	{
	    if ( lp.previous_is_sublist_head )
	    {
	        MUP::set_gen_of
		    ( lp.previous_stub,
		      min::new_sublist_aux_gen
			  ( aux_first ) );
	    }
	    else
	    {
	        int type =
		    min::type_of ( lp.previous_stub );
		MUP::set_control_of
		    ( lp.previous_stub,
		      MUP::new_control_with_type
			  ( type, aux_first ) );
	    }
	}
	else
#   endif
    if ( lp.previous_index != 0 )
    {
	lp.base[lp.previous_index] =
	    lp.previous_is_sublist_head ?
	    min::new_sublist_aux_gen ( aux_first ) :
	    min::new_list_aux_gen ( aux_first );
    }
    else
    {
	MIN_ASSERT ( lp.current_index != 0 );
	lp.base[lp.current_index] =
	    min::new_list_aux_gen ( aux_first );
	lp.previous_index = lp.current_index;
	lp.previous_is_sublist_head = false;
    }

    lp.current_index = first;
    lp.current = lp.base[first];
#   if MIN_USE_OBJ_AUX_STUBS
	lp.current_stub = NULL;
#   endif

    unprotected::aux_offset_of ( lp.vecp ) = aux_offset;
}

void min::insert_after
	( min::insertable_list_pointer & lp,
	  const min::gen * p, min::unsptr n )
{
    if ( n == 0 ) return;

    min::unsptr unused_offset =
        unprotected::unused_offset_of ( lp.vecp );
    min::unsptr aux_offset =
        unprotected::aux_offset_of ( lp.vecp );
    min::unsptr total_size = lp.total_size;
    MIN_ASSERT (    total_size
                 == min::total_size_of ( lp.vecp ) );

    MIN_ASSERT ( lp.reserved_insertions >= 1 );
    MIN_ASSERT ( lp.reserved_elements >= n );
    MIN_ASSERT ( lp.current != min::LIST_END );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    MUP::acc_write_update
	    ( MUP::stub_of ( lp.vecp ), p, n );

    bool previous = ( lp.previous_index != 0 );
#   if MIN_USE_OBJ_AUX_STUBS
	if ( lp.previous_stub != NULL )
	    previous = true;

	if (    lp.use_obj_aux_stubs
	     &&     unused_offset
		  + ( n + 1 + ! previous )
		> aux_offset )
	{
	    // Not enough aux area available for
	    // all the new elements, and aux stubs
	    // are allowed.
	    //
	    min::stub * first, * last;

	    if ( lp.current_stub != NULL )
	    {
		MINT::allocate_stub_list
		    ( first, last, min::LIST_AUX,
		      p, n,
		      MUP::control_of
		          ( lp.current_stub ) );
		MUP::set_control_of
		    ( lp.current_stub,
		      MUP::new_control_with_type
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

	    // If previous, we can copy the last new
	    // element to the old current element.

	    min::unsptr next =
	        lp.current_index == lp.head_index ?
	        0 :
	          total_size
	        - lp.current_index + ! previous;
	    min::uns64 end =
		MUP::new_control_with_type
		    ( type, next );

	    if ( n > previous )
		MINT::allocate_stub_list
		    ( first, last, min::LIST_AUX,
		      p, n - previous, end );

	    if ( previous )
	    {
		if ( n > 1 )
		    end = MUP::new_control_with_type
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
			int type =
			    min::type_of
				( lp.previous_stub );
			MUP::set_control_of
			  ( lp.previous_stub,
			    MUP::new_control_with_type
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
		      MUP::new_control_with_type
		          ( min::LIST_AUX, first,
			    MUP::STUB_POINTER ) );
		lp.base[lp.current_index] =
		    min::new_gen ( s );
		lp.previous_index = lp.current_index;
		lp.current_index = 0;
		lp.current_stub = s;
	    }
	    return;
	}
#   endif

    // Insertion will use aux area.

    MIN_ASSERT (      unused_offset
		    + ( n + 1 + ! previous )
		 <= aux_offset );

    min::unsptr first = aux_offset - 1;

    if ( lp.current_index != 0 )
	lp.base[-- aux_offset] = lp.current;

    // If previous, we can copy the last new element to
    // the old current element.

    // Copy all the new elements BUT the last new
    // element.
    //
    while ( -- n )
	lp.base[-- aux_offset] = * p ++;

#   if MIN_USE_OBJ_AUX_STUBS
    if ( lp.current_stub != NULL )
    {
	MIN_ASSERT ( previous );
	lp.base[-- aux_offset] = * p ++;
	min::uns64 c =
	    MUP::control_of ( lp.current_stub );
	if ( c & MUP::STUB_POINTER )
	    lp.base[-- aux_offset] =
		min::new_gen
		    ( MUP::stub_of_control ( c ) );
	else
	    lp.base[-- aux_offset] =
	        min::new_list_aux_gen
		    ( MUP::value_of_control ( c ) );
	MUP::set_control_of
	    ( lp.current_stub,
	      MUP::new_control_with_type
	         ( min::type_of ( lp.current_stub ),
		   total_size - first ) );
    }
    else
#   endif
    if ( previous )
    {
	lp.base[-- aux_offset] =
	    min::new_list_aux_gen
	        ( total_size - lp.current_index );
	lp.base[lp.current_index] = * p ++;
	lp.current_index = first;

#	if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.previous_stub != NULL )
	    {
		if ( lp.previous_is_sublist_head )
		{
		    MUP::set_gen_of
			( lp.previous_stub,
			  min::new_sublist_aux_gen
			      ( total_size - first ) );
		}
		else
		{
		    MUP::set_control_of
			( lp.previous_stub,
			  MUP::new_control_with_type
			      ( min::type_of
				  ( lp.previous_stub ),
				total_size - first ) );
		}
	    }
	    else
#	endif
	{
	    lp.base[lp.previous_index] =
		MUP::renew_gen
		    ( lp.base[lp.previous_index],
		      total_size - first );
	}
    }
    else
    {
	// With no previous we must use current aux
	// element as pointer to copied of current
	// element.
	//
	MIN_ASSERT ( lp.current_index != 0 );

	lp.base[-- aux_offset] = * p ++;
	min::unsptr next = lp.current_index;
	if ( next == lp.head_index )
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
	else
	    next = total_size - next;
	lp.base[-- aux_offset] =
	    min::new_list_aux_gen ( next );

	lp.base[lp.current_index] =
	    min::new_list_aux_gen
	        ( total_size - first );
	lp.previous_index = lp.current_index;
	lp.current_index = first;
    }

    unprotected::aux_offset_of ( lp.vecp ) = aux_offset;
}

min::unsptr min::remove
	( min::insertable_list_pointer & lp,
	  min::unsptr n )
{
    if ( n == 0 || lp.current == min::LIST_END )
        return 0;

    min::unsptr total_size = lp.total_size;
    MIN_ASSERT (    total_size
                 == min::total_size_of ( lp.vecp ) );

    if ( lp.current_index != 0
         &&
	 lp.current_index == lp.head_index )
    {
	// Special case: deleting list head of a list
	// with just 1 element.
	//
	lp.current = lp.base[lp.current_index]
	           = min::LIST_END;
	return 1;
    }

    // Save the current previous pointer and current
    // index.
    //
    min::unsptr previous_index = lp.previous_index;
    bool previous_is_sublist_head =
	lp.previous_is_sublist_head;
    min::unsptr current_index = lp.current_index;
#   if MIN_USE_OBJ_AUX_STUBS
	min::stub * previous_stub = lp.previous_stub;
#   endif

    // Count of elements removed; to be returned as
    // result.
    //
    min::unsptr count = 0;

    // Skip n elements (or until end of list).
    // Remove sublists and free aux stubs.
    // Set aux area elements to NONE.
    //
    while ( n -- )
    {
	if ( lp.current == min::LIST_END ) break;
	++ count;
	MINT::remove_sublist
	    ( lp.base, total_size, lp.current );

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
		min::stub * last_stub = lp.current_stub;
		next ( lp );
		MUP::free_aux_stub ( last_stub );
	    }
	    else
#       endif
	{
	    MIN_ASSERT ( lp.current_index != 0 );
	    lp.base[lp.current_index] = min::NONE;
	    lp.current =
		lp.base[-- lp.current_index];
	    if ( min::is_list_aux ( lp.current ) )
	    {
		if ( lp.current == min::LIST_END )
		    break;
		lp.base[lp.current_index] = min::NONE;
		lp.current_index =
		      total_size
		    - min::list_aux_of ( lp.current );
		lp.current = lp.base[lp.current_index];
	    }
	    else if ( min::is_stub ( lp.current ) )
		lp.forward ( lp.current_index );
	}
    }

    // Now lp.current_index/lp.current_stub are the new
    // current element and we must either set the old
    // previous element to point to this, or if there
    // was no old previous element, we must make the
    // old current element into a previous element.

#   if MIN_USE_OBJ_AUX_STUBS

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
			  MUP::new_control_with_type
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
			  MUP::new_control_with_type
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
			      (   total_size
			        - lp.current_index ) );
		else
		{
		    int type =
		        min::type_of ( previous_stub );
		    MUP::set_control_of
		        ( previous_stub,
			  MUP::new_control_with_type
			      ( type,
			          total_size
				- lp.current_index ) );
		}
	    }

	    lp.previous_stub = previous_stub;
	    lp.previous_is_sublist_head =
		previous_is_sublist_head;
	    lp.previous_index = 0;
	}
	else
#   endif
    if ( previous_index != 0 )
    {
#	if MIN_USE_OBJ_AUX_STUBS
	    lp.previous_stub = NULL;

	    if ( lp.current_stub != NULL )
	    {
		if ( previous_is_sublist_head )
		    MUP::set_type_of
			( lp.current_stub,
			  min::SUBLIST_AUX );
		lp.base[previous_index] =
		    min::new_gen ( lp.current_stub );

		lp.previous_index = previous_index;
		lp.previous_is_sublist_head =
		    previous_is_sublist_head;
	    }
	    else
#	endif
	if ( lp.current == min::LIST_END )
	{
	    if ( lp.current_index != 0 )
		lp.base[lp.current_index] = min::NONE;

	    if ( previous_is_sublist_head )
	    {
		lp.base[previous_index] =
		    min::EMPTY_SUBLIST;
		lp.current_index = 0;
		lp.previous_index = previous_index;
		lp.previous_is_sublist_head = true;
	    }
	    else
	    {
		lp.base[previous_index] =
		    min::LIST_END;
		lp.current_index = previous_index;
		lp.previous_index = 0;
		lp.previous_is_sublist_head = false;
	    }
	}
	else
	{
	    MIN_ASSERT ( lp.current_index != 0 );

	    if ( previous_is_sublist_head )
		lp.base[previous_index] =
		    min::new_sublist_aux_gen
			(   total_size
			  - lp.current_index );
	    else
		lp.base[previous_index] =
		    min::new_list_aux_gen
			(   total_size
			  - lp.current_index );

	    lp.previous_index = previous_index;
	    lp.previous_is_sublist_head =
		previous_is_sublist_head;
	}
    }
    else
    {
    	// No previous.  Then the first element
	// removed cannot be a stub, as stubs always
	// have a previous pointer.

	MIN_ASSERT ( current_index != 0 );

#       if MIN_USE_OBJ_AUX_STUBS
	    lp.previous_stub = NULL;
	    if ( lp.current_stub != NULL )
	    {
		lp.base[current_index] =
		    min::new_gen ( lp.current_stub );
		lp.previous_index = current_index;
	    }
	    else
#       endif
	if ( lp.current == min::LIST_END )
	{
	    if ( lp.current_index != 0 )
		lp.base[lp.current_index] = min::NONE;
	    lp.base[current_index] = min::LIST_END;
	    lp.current_index = current_index;
	    lp.previous_index = 0;
	}
	else
	{
	    MIN_ASSERT
	        ( current_index != lp.head_index );
	    lp.base[current_index] =
		min::new_list_aux_gen
		    ( total_size - lp.current_index );
	    lp.previous_index = current_index;
	}

	lp.previous_is_sublist_head = false;
    }

    return count;
}

bool MINT::insert_reserve
	( min::insertable_list_pointer & lp,
	  min::unsptr insertions,
	  min::unsptr elements,
	  bool use_obj_aux_stubs )
{
    bool result = false;

#   if MIN_USE_OBJ_AUX_STUBS
	if ( use_obj_aux_stubs )
	    MINT::acc_expand_stub_free_list
		( insertions + elements );
	else
#   endif
    {
	min::unsptr desired_size =
	    2 * insertions + elements;
	min::unsptr total_size =
	    min::total_size_of ( lp.vecp );
	if ( desired_size < 1000 )
	{
	    if ( desired_size < total_size / 2 )
		desired_size = total_size / 2;
	    if ( desired_size > 1000 )
	        desired_size = 1000;
	}
	min::resize ( lp.vecp, desired_size );
	min::insert_refresh ( lp );
	result = true;
    }

    lp.reserved_insertions = insertions;
    lp.reserved_elements = elements;
#   if MIN_USE_OBJ_AUX_STUBS
	lp.use_obj_aux_stubs = use_obj_aux_stubs;
#   endif

    return result;
}

// Object Attribute Level
// ------ --------- -----

template<>
const min::raw_vec_type_info
	min::attr_info_pointer::type_info
    = { "min::attr_info", "g",
        sizeof ( min::attr_info ),
	100, 1.0, 1000 };
template<>
const min::raw_vec_type_info
	min::reverse_attr_info_pointer::type_info
    = { "min::reverse_attr_info", "g",
        sizeof ( min::reverse_attr_info ),
	100, 1.0, 1000 };

# if MIN_ALLOW_PARTIAL_ATTR_LABELS

    template < class vecpt >
    void MINT::locate
	    ( MUP::attr_pointer_type<vecpt> & ap,
	      min::gen name,
	      bool allow_partial_labels )
    {
	typedef MUP::attr_pointer_type<vecpt> ap_type;

	ap.attr_name = name;
	ap.reverse_attr_name = min::NONE;

	// Set len to the number of elements in the
	// label and element[] to the vector of
	// label elements.
	//
	bool is_label = is_lab ( name );
	min::unsptr len;
	if ( is_label )
	{
	    len = min::lablen ( name );
	    MIN_ASSERT ( len > 0 );
	}
	else
	{
	    MIN_ASSERT
		( is_str ( name ) || is_num ( name ) );
	    len = 1;
	}
	min::gen element[len];
	if ( is_label )
	{
	    lab_of ( element, len, name );
	    if ( len == 1
	         &&
		 ( is_num ( element[0] )
		   ||
		   is_str ( element[0] ) ) )
	        ap.attr_name = element[0];
	}
	else element[0] = name;

	// Process element[0] and if found set
	// ap.length = 1.
	//
	// If element[0] is an integer in the right
	// range, locate attribute vector entry.
	// Otherwise locate hash table entry.
	// Set c to the node-descriptor if that is
	// found, and to LIST_END if not.
	//
	float64 f;
	int i;
	min::gen c;
	if ( is_num ( element[0] )
	     &&
	     0 <= ( f = float_of ( element[0] ) )
	     &&
	     ( i = (int) f ) == f
	     &&
	     i < attr_size_of
	             ( vec_pointer_of
		           ( ap.dlp ) ) )
	{
	    start_vector ( ap.dlp, i );
	    ap.flags = ap_type::IN_VECTOR;
	    ap.index = i;
	    c = current ( ap.dlp );
	}
	else
	{
	    ap.index = min::hash ( element[0] )
	             % hash_size_of
		         ( vec_pointer_of
			       ( ap.dlp ) );
	    ap.flags = 0;

	    start_hash ( ap.dlp, ap.index );

	    for ( c = current ( ap.dlp );
		  ! is_list_end ( c );
		  c = next ( ap.dlp ) )
	    {
	        next ( ap.dlp);
		if ( c == element[0] )
		{
		    c = current ( ap.dlp );
		    MIN_ASSERT ( ! is_list_end ( c ) );
		    break;
		}
	    }
	}

	if ( is_list_end ( c ) )
	{
	    ap.length = 0;
	    ap.state = ap_type::LOCATE_FAIL;
	    return;
	}
	else ap.length = 1;

	min::unsptr locate_length = 0;
	    // ap.length for the last node descriptor
	    // with a non-empty value set.
	    // locate_dlp is value of dlp for this
	    // length.

	while ( true )
	{
	    if ( ! is_sublist ( current ( ap.dlp ) ) )
	    {
	        locate_length = ap.length;
		start_copy ( ap.locate_dlp, ap.dlp );
	        break;
	    }
	    start_sublist ( ap.lp, ap.dlp );
	    c = current ( ap.lp );
	    if (    ! is_list_end ( c )
	         && ! is_sublist ( c )
		 && ! is_control_code ( c ) )
	    {
	        locate_length = ap.length;
		start_copy ( ap.locate_dlp, ap.dlp );
	    }

	    if ( ap.length >= len ) break;

	    for ( ;
	          ! is_sublist && ! is_list_end ( c );
		  c = next ( ap.lp ) );
	    if ( ! is_sublist ( c ) )
	        break;
	    start_sublist ( ap.lp );

	    for ( c = current ( ap.lp );
	          ! is_list_end ( c );
		  c = next ( ap.lp ) )
	    {
	        next ( ap.dlp);
		if ( c == element[ap.length] )
		{
		    c = next ( ap.lp );
		    MIN_ASSERT ( ! is_list_end ( c ) );
		    break;
		}
	    }
	    if ( is_list_end ( c ) ) break;

	    start_copy ( ap.dlp, ap.lp );
	    ++ ap.length;
	}

	if ( locate_length == len )
	{
	    MIN_ASSERT ( ap.length == locate_length );
	    ap.state = ap_type::LOCATE_NONE;
	}
	else if ( allow_partial_labels
	          &&
		  locate_length > 0 )
	{
	    ap.length = locate_length;
	    start_copy ( ap.dlp, ap.locate_dlp );
	    ap.state = ap_type::LOCATE_NONE;
	}
	else
	    ap.state = ap_type::LOCATE_FAIL;

	return;
    }

    // Continue relocation after ap.locate_dlp is
    // positioned at beginning of hash-list or
    // non-empty vector-list.  ap.length must be >= 1.
    // Result is a setting of ap.locate_dlp only.
    // 
    template < class vecpt >
    inline void MINT::relocate
	    ( MUP::attr_pointer_type<vecpt> & ap )
    {
	typedef MUP::attr_pointer_type<vecpt> ap_type;

	MIN_ASSERT ( ap.length > 0 );

	bool is_label = is_lab ( ap.attr_name );
	min::unsptr len;
	if ( is_label )
	    len = min::lablen ( ap.attr_name );
	else len = 1;
	min::gen element[len];
	if ( is_label )
	    lab_of ( element, len, ap.attr_name );
	else element[0] = ap.attr_name;

	min::gen c;
	if ( ! ( ap.flags & ap_type::IN_VECTOR ) )
	{
	    for ( c = current ( ap.locate_dlp );
		  ! is_list_end ( c );
		  next ( ap.locate_dlp),
		  c = next ( ap.locate_dlp ) )
	    {
		if ( c == element[0] )
		{
		    c = next ( ap.locate_dlp );
		    MIN_ASSERT ( ! is_list_end ( c ) );
		    break;
		}
	    }
	}

	MIN_ASSERT ( ap.length <= len );
	min::unsptr length = 1;
	start_copy ( ap.dlp, ap.locate_dlp );
	while ( length < ap.length )
	{
	    if ( ! is_sublist ( current ( ap.dlp ) ) )
	        break;
	    start_sublist ( ap.dlp );
	    for ( c = current ( ap.dlp );
	          ! is_sublist && ! is_list_end ( c );
		  c = next ( ap.dlp ) );
	    if ( ! is_sublist ( c ) )
	        break;
	    start_sublist ( ap.dlp );

	    for ( c = current ( ap.dlp );
	          ! is_list_end ( c );
		  next ( ap.dlp),
		  c = next ( ap.dlp ) )
	    {
		if ( c == element[ap.length] )
		{
		    c = next ( ap.dlp );
		    MIN_ASSERT ( ! is_list_end ( c ) );
		    break;
		}
	    }
	    if ( is_list_end ( c ) ) break;

	    start_copy ( ap.locate_dlp, ap.dlp );
	    ++ length;
	}

	MIN_ASSERT ( length == ap.length );
    }

    void MINT::attr_create
	    ( min::insertable_attr_pointer & ap,
	      min::gen v )
    {
	typedef min::insertable_attr_pointer ap_type;

	MIN_ASSERT
	    ( ap.state == ap_type::LOCATE_FAIL );

	bool is_label = is_lab ( ap.attr_name );
	min::unsptr len;
	if ( is_label )
	    len = min::lablen ( ap.attr_name );
	else
	    len = 1;
	min::gen element[len];
	if ( is_label )
	    lab_of ( element, len, ap.attr_name );
	else element[0] = ap.attr_name;

	MIN_ASSERT ( ap.length < len );

	update_refresh ( ap.locate_dlp );
	if ( insert_reserve
	           ( ap.locate_dlp,
	             2 * ( len - ap.length ),
	             4 * ( len - ap.length ) + 1 ) )
	{
	    insert_refresh ( ap.dlp );
	    insert_refresh ( ap.lp );
	}

	if ( ap.length == 0 )
	{
	    if ( ap.flags & ap_type::IN_VECTOR )
	    {
		start_vector
		    ( ap.locate_dlp, ap.index );
		min::gen c = current ( ap.locate_dlp );
		if ( is_list_end ( c ) )
		{
		    min::gen elements[1] =
			{ len == 1 ? v :
			  min::EMPTY_SUBLIST };
		    insert_before ( ap.locate_dlp,
				    elements, 1 );
		}
	    }
	    else
	    {
		start_hash ( ap.locate_dlp, ap.index );
		min::gen elements[2] =
		    { element[0], len == 1 ? v :
		                  min::EMPTY_SUBLIST };
		insert_before ( ap.locate_dlp,
		                elements, 2 );
		next ( ap.locate_dlp );
	    }

	    ap.length = 1;
	}

	while ( ap.length < len )
	{
	    min::gen c = current ( ap.locate_dlp );

	    if ( ! is_sublist ( c ) )
	    {
		update ( ap.locate_dlp,
		         min::EMPTY_SUBLIST );
		start_sublist ( ap.locate_dlp );
		min::gen elements[2] =
		    { c, min::EMPTY_SUBLIST };
		insert_before
		    ( ap.locate_dlp, elements, 2 );
		next ( ap.locate_dlp );
	    }
	    else
	    {
	        start_sublist ( ap.locate_dlp );
		for ( c = current ( ap.locate_dlp );
		         ! is_sublist ( c )
		      && ! is_control_code ( c )
		      && ! is_list_end ( c );
		      c = next ( ap.locate_dlp ) );
		if ( ! is_sublist ( c ) )
		{
		    min::gen elements[1] =
			{ min::EMPTY_SUBLIST };
		    insert_before
		        ( ap.locate_dlp, elements, 1 );
		}
	    }
	    start_sublist ( ap.locate_dlp );

	    min::gen elements[2] =
		{ element[ap.length],
		  len == ap.length + 1 ? v :
	          min::EMPTY_SUBLIST };
	    insert_before
	        ( ap.locate_dlp, elements, 2 );

	    next ( ap.locate_dlp );
	    ++ ap.length;
	}
	start_copy ( ap.dlp, ap.locate_dlp );

	if ( ap.reverse_attr_name == min::NONE )
	    ap.state = ap_type::LOCATE_NONE;
	else if (    ap.reverse_attr_name
	          == min::ANY )
	    ap.state = ap_type::LOCATE_ANY;
	else
	    ap.state = ap_type::REVERSE_LOCATE_FAIL;
    }

# else // ! MIN_ALLOW_PARTIAL_ATTR_LABELS

    template < class vecpt >
    void MINT::locate
	    ( MUP::attr_pointer_type<vecpt> & ap,
	      min::gen name )
    {
	typedef MUP::attr_pointer_type<vecpt> ap_type;

	ap.reverse_attr_name = min::NONE;

	// If name is label whose only element is an
	// atom, set name = the atom.
	//
	if ( is_lab ( name ) )
	{
	    if ( min::lablen ( name ) == 1 )
	    {
		min::gen atom;
		lab_of ( & atom, 1, name );
		if ( is_num ( atom )
		     ||
		     is_str ( atom ) )
		    name = atom;
	    }
	}
	else
	    MIN_ASSERT
		( is_str ( name ) || is_num ( name ) );

	ap.attr_name = name;

	// If name is an integer in the right range,
	// locate attribute vector entry and return.
	//
	if ( is_num ( name ) )
	{
	    float64 f = float_of ( name );
	    int i = (int) f;

	    if ( i == f
		 &&
		 0 <= i
		 &&
		 i < attr_size_of
		        ( vec_pointer_of ( ap.dlp ) ) )
	    {
		start_vector ( ap.locate_dlp, i );
		ap.index = i;
		ap.flags = ap_type::IN_VECTOR;

		min::gen c = current ( ap.locate_dlp );
		if ( is_list_end ( c ) )
		    ap.state = ap_type::LOCATE_FAIL;
		else
		{
		    ap.state = ap_type::LOCATE_NONE;
		    start_copy
		        ( ap.dlp, ap.locate_dlp );
		}
		return;
	    }
	}

	ap.index = min::hash ( name )
		 % hash_size_of
			  ( vec_pointer_of
			        ( ap.locate_dlp ) );
	ap.flags = 0;
	start_hash ( ap.locate_dlp, ap.index );

	for ( min::gen c = current ( ap.locate_dlp );
	      ! is_list_end ( c );
	      next ( ap.locate_dlp),
	      c = next ( ap.locate_dlp ) )
	{
	    if ( c == name )
	    {
		c = next ( ap.locate_dlp );
		MIN_ASSERT ( ! is_list_end ( c ) );
		start_copy ( ap.dlp, ap.locate_dlp );
		ap.flags = 0;
		ap.state = ap_type::LOCATE_NONE;
		return;
	    }
	}

        // Name not found.
	//
	ap.state = ap_type::LOCATE_FAIL;
	return;

    }

    // Continue relocation after ap.relocate_dlp is
    // positioned at beginning of hash-list.  State
    // must be >= LOCATE_NONE.  Is NOT called if IN_
    // VECTOR flag is set.  Result is a setting of
    // ap.locate_dlp only.
    // 
    template < class vecpt >
    inline void MINT::relocate
	    ( MUP::attr_pointer_type<vecpt> & ap )
    {
	typedef MUP::attr_pointer_type<vecpt> ap_type;

	for ( min::gen c = current ( ap.locate_dlp );
	      ! is_list_end ( c );
	      next ( ap.locate_dlp),
	      c = next ( ap.locate_dlp ) )
	{
	    if ( c == ap.attr_name )
	    {
		c = next ( ap.locate_dlp );
		MIN_ASSERT ( ! is_list_end ( c ) );
		return;
	    }
	}

	MIN_ABORT ( "relocate could not find"
	            " attribute" );
    }

    void MINT::attr_create
	    ( min::insertable_attr_pointer & ap,
	      min::gen v )
    {
	typedef min::insertable_attr_pointer ap_type;

	MIN_ASSERT
	    ( ap.state == ap_type::LOCATE_FAIL );

	if ( ap.flags & ap_type::IN_VECTOR )
	{
	    start_vector ( ap.locate_dlp, ap.index );
	    min::gen c = current ( ap.locate_dlp );
	    MIN_ASSERT ( is_list_end ( c ) );
	    if ( insert_reserve
	             ( ap.locate_dlp, 1, 1 ) )
	    {
	        insert_refresh ( ap.dlp );
	        insert_refresh ( ap.lp );
	    }
	    insert_before ( ap.locate_dlp, & v, 1 );
	    start_copy ( ap.dlp, ap.locate_dlp );
	    return;
	}

        start_hash ( ap.locate_dlp, ap.index );

	if ( insert_reserve ( ap.locate_dlp, 1, 2 ) )
	{
	    insert_refresh ( ap.dlp );
	    insert_refresh ( ap.lp );
	}

	min::gen elements[2] = { ap.attr_name, v };
	insert_before ( ap.locate_dlp, elements, 2 );

	next ( ap.locate_dlp );
	start_copy ( ap.dlp, ap.locate_dlp );

	if ( ap.reverse_attr_name == min::NONE )
	    ap.state = ap_type::LOCATE_NONE;
	else if (    ap.reverse_attr_name
	          == min::ANY )
	    ap.state = ap_type::LOCATE_ANY;
	else
	    ap.state = ap_type::REVERSE_LOCATE_FAIL;
    }

# endif

void MINT::reverse_attr_create
	( min::insertable_attr_pointer & ap,
	  min::gen v )
{
    typedef min::insertable_attr_pointer ap_type;

    MIN_ASSERT
	( ap.state == ap_type::REVERSE_LOCATE_FAIL );

    update_refresh ( ap.locate_dlp );
    start_copy ( ap.dlp, ap.locate_dlp );

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	if ( insert_reserve ( ap.dlp, 2, 5 ) )
#   else
	if ( insert_reserve ( ap.dlp, 2, 4 ) )
#   endif
	{
	    insert_refresh ( ap.locate_dlp );
	    insert_refresh ( ap.lp );
	}

    min::gen c = current ( ap.dlp );
    if ( ! is_sublist ( c ) )
    {
	update ( ap.dlp, min::EMPTY_SUBLIST );
	start_sublist ( ap.dlp );
#       if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    min::gen elements[3] =
		{ c, min::EMPTY_SUBLIST,
		     min::EMPTY_SUBLIST };
	    insert_before ( ap.dlp, elements, 3 );
	    next ( ap.dlp );
#       else
	    min::gen elements[2] =
		{ c, min::EMPTY_SUBLIST };
	    insert_before ( ap.dlp, elements, 2 );
#       endif
	next ( ap.dlp );
    }
    else
    {
	start_sublist ( ap.dlp );
	for ( c = current ( ap.dlp );
		 ! is_sublist ( c )
	      && ! is_control_code ( c )
	      && ! is_list_end ( c );
	      c = next ( ap.dlp ) );
#       if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    if ( ! is_sublist ( c ) )
	    {
		min::gen elements[2] =
		    { min::EMPTY_SUBLIST,
		      min::EMPTY_SUBLIST };
		insert_before ( ap.dlp, elements, 2 );
		next ( ap.dlp );
	    }
	    else if ( ! is_sublist ( next ( ap.dlp ) ) )
	    {
		min::gen elements[1] =
		    { min::EMPTY_SUBLIST };
		insert_before ( ap.dlp, elements, 1 );
	    }
#       else
	    if ( ! is_sublist ( c ) )
	    {
		min::gen elements[1] =
		    { min::EMPTY_SUBLIST };
		insert_before ( ap.dlp, elements, 1 );
	    }
#       endif
    }
    start_sublist ( ap.dlp );
    min::gen elements[2] =
	{ ap.reverse_attr_name, v };
    insert_before ( ap.dlp, elements, 2 );
    next ( ap.dlp );

    ap.state = ap_type::REVERSE_LOCATE_SUCCEED;
}

template < class vecpt >
void min::locate_reverse
	( MUP::attr_pointer_type<vecpt> & ap,
	  min::gen reverse_name )
{
    typedef MUP::attr_pointer_type<vecpt> ap_type;

    // If reverse_name is label whose only element is an
    // atom, set reverse_name = the atom.
    //
    if ( is_lab ( reverse_name )
	 &&
	 lablen ( reverse_name ) == 1 )
    {
	min::gen atom;
	lab_of ( & atom, 1, reverse_name );
	if ( is_str ( atom )
	     ||
	     is_num ( atom ) )
	    reverse_name = atom;
    }

    update_refresh ( ap.locate_dlp );

    ap.reverse_attr_name = reverse_name;

    switch ( ap.state )
    {
    case ap_type::INIT:
	    MIN_ABORT
	        ( "bad attribute reverse_locate call" );
    case ap_type::LOCATE_FAIL:
    	    return;
    case ap_type::LOCATE_NONE:
	    if ( reverse_name == min::NONE )
	        return;
	    else if ( reverse_name == min::ANY )
	    {
	        ap.state = ap_type::LOCATE_ANY;
		return;
	    }
	    break;
    case ap_type::LOCATE_ANY:
	    if ( reverse_name == min::ANY )
	        return;
	    else if ( reverse_name == min::NONE )
	    {
	        ap.state = ap_type::LOCATE_NONE;
		return;
	    }
	    break;
    case ap_type::REVERSE_LOCATE_FAIL:
    case ap_type::REVERSE_LOCATE_SUCCEED:
	    if ( reverse_name == min::NONE )
	    {
		start_copy ( ap.dlp, ap.locate_dlp );
	        ap.state = ap_type::LOCATE_NONE;
		return;
	    }
	    else if ( reverse_name == min::ANY )
	    {
		start_copy ( ap.dlp, ap.locate_dlp );
	        ap.state = ap_type::LOCATE_ANY;
		return;
	    }
	    break;
    }

    // ap.locate_dlp is as set by previous successful
    // locate and reverse_name is not NONE or ANY.

    start_copy ( ap.dlp, ap.locate_dlp );

    min::gen c = current ( ap.dlp );
    if ( ! is_sublist ( c ) )
    {
	ap.state = ap_type::REVERSE_LOCATE_FAIL;
	return;
    }
    start_sublist ( ap.dlp );
    for ( c = current ( ap.dlp );
	  ! is_sublist && ! is_list_end ( c );
	  c = next ( ap.dlp ) );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
    if ( ! is_sublist ( c ) )
    {
	ap.state = ap_type::REVERSE_LOCATE_FAIL;
	return;
    }
    c = next ( ap.dlp );
#   endif
    if ( ! is_sublist ( c ) )
    {
	ap.state = ap_type::REVERSE_LOCATE_FAIL;
	return;
    }
    start_sublist ( ap.dlp );

    for ( min::gen c = current ( ap.dlp );
	  ! is_list_end ( c );
	  next ( ap.dlp), c = next ( ap.dlp ) )
    {
	if ( c == reverse_name )
	{
	    c = next ( ap.dlp );
	    MIN_ASSERT ( ! is_list_end ( c ) );
	    ap.state =
		ap_type::REVERSE_LOCATE_SUCCEED;
	    return;
	}
    }

    ap.state = ap_type::REVERSE_LOCATE_FAIL;
}

template < class vecpt >
void min::relocate
	( MUP::attr_pointer_type<vecpt> & ap )
{
    typedef MUP::attr_pointer_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
        return;
    case ap_type::LOCATE_FAIL:
#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    if ( ap.length == 0 ) return;
#	else
	    return;
#	endif
    }

    if ( ap.flags & ap_type::IN_VECTOR )
    {
        start_vector ( ap.locate_dlp, ap.index );
#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    if ( ap.length != 1 ) MINT::relocate ( ap );
#	endif
    }
    else
    {
        start_hash ( ap.locate_dlp, ap.index );
	MINT::relocate ( ap );
    }

    start_copy ( ap.dlp, ap.locate_dlp );

    switch ( ap.state )
    {
    case ap_type::LOCATE_FAIL:
    case ap_type::LOCATE_NONE:
    case ap_type::LOCATE_ANY:
    case ap_type::REVERSE_LOCATE_FAIL:
	return;
    }

    // state == REVERSE_LOCATE_SUCCEED

    start_copy ( ap.dlp, ap.locate_dlp );

    min::gen c = current ( ap.dlp );
    if ( ! is_sublist ( c ) )
    {
	MIN_ABORT ( "relocate could not find"
	            " reverse attribute" );
    }
    start_sublist ( ap.dlp );
    for ( c = current ( ap.dlp );
	  ! is_sublist && ! is_list_end ( c );
	  c = next ( ap.dlp ) );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
    if ( ! is_sublist ( c ) )
    {
	MIN_ABORT ( "relocate could not find"
	            " reverse attribute" );
    }
    c = next ( ap.dlp );
#   endif
    if ( ! is_sublist ( c ) )
    {
	MIN_ABORT ( "relocate could not find"
	            " reverse attribute" );
    }
    start_sublist ( ap.dlp );

    for ( min::gen c = current ( ap.dlp );
	  ! is_list_end ( c );
	  next ( ap.dlp), c = next ( ap.dlp ) )
    {
	if ( c == ap.reverse_attr_name )
	{
	    c = next ( ap.dlp );
	    MIN_ASSERT ( ! is_list_end ( c ) );
	    return;
	}
    }

    MIN_ABORT ( "relocate could not find reverse"
                " attribute" );
}

template < class vecpt >
inline min::unsptr MINT::get
	( min::gen * out, min::unsptr n,
	  MUP::attr_pointer_type<vecpt> & ap )
{
    typedef MUP::attr_pointer_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT ( "min::get called before locate" );
    case ap_type::LOCATE_ANY:
        break;
    default:
	MIN_ABORT ( "abnormal call to min::get" );
    }

    // state == LOCATE_ANY:

    min::gen c;
    update_refresh ( ap.locate_dlp );
    start_copy ( ap.dlp, ap.locate_dlp );

    if ( ! is_sublist ( current ( ap.dlp ) ) )
        return 0;
    start_sublist ( ap.dlp );
    for ( c = current ( ap.dlp );
	  ! is_sublist && ! is_list_end ( c );
	  c = next ( ap.dlp ) );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	if ( ! is_sublist ( c ) ) return 0;
	c = next ( ap.dlp );
#   endif
    if ( ! is_sublist ( c ) ) return 0;
    start_sublist ( ap.dlp );

    min::unsptr result = 0;
    for ( min::gen c = current ( ap.dlp );
	  ! is_list_end ( c );
	  c = next ( ap.dlp ) )
    {
        c = next ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    if ( result < n ) * out ++ = c;
	    ++ result;
	}
	else
	{
	    start_sublist ( ap.lp, ap.dlp );
	    c = current ( ap.lp );
	    while ( ! is_list_end ( c ) )
	    {
		if ( result < n ) * out ++ = c;
		++ result;
		c = next ( ap.lp );
	    }
	}
    }

    return result;
}

template < class vecpt >
inline min::gen MINT::get
	( MUP::attr_pointer_type<vecpt> & ap )
{
    typedef MUP::attr_pointer_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT ( "min::get called before locate" );
    case ap_type::LOCATE_ANY:
        break;
    default:
	MIN_ABORT ( "abnormal call to min::get" );
    }

    // state == LOCATE_ANY:

    min::gen c;
    update_refresh ( ap.locate_dlp );
    start_copy ( ap.dlp, ap.locate_dlp );

    if ( ! is_sublist ( current ( ap.dlp ) ) )
        return min::NONE;
    start_sublist ( ap.dlp );
    for ( c = current ( ap.dlp );
	  ! is_sublist && ! is_list_end ( c );
	  c = next ( ap.dlp ) );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
    if ( ! is_sublist ( c ) ) return min::NONE;
    c = next ( ap.dlp );
#   endif
    if ( ! is_sublist ( c ) ) return min::NONE;
    start_sublist ( ap.dlp );

    min::gen result = min::NONE;
    for ( min::gen c = current ( ap.dlp );
	  ! is_list_end ( c );
	  c = next ( ap.dlp ) )
    {
        c = next ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    if ( result == min::NONE )
	        result = c;
	    else
	        return min::MULTI_VALUED;
	}
	else
	{
	    start_sublist ( ap.lp, ap.dlp );
	    c = current ( ap.lp );
	    while ( ! is_list_end ( c ) )
	    {
		if ( result == min::NONE )
		    result = c;
		else
		    return min::MULTI_VALUED;
		c = next ( ap.lp );
	    }
	}
    }

    return result;
}

template < class vecpt >
inline min::unsptr MINT::get_flags
	( MUP::attr_pointer_type<vecpt> & ap )
{
    typedef MUP::attr_pointer_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::get_flags called before locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::get_flags" );
    }
}

template < class vecpt >
inline min::unsptr MINT::test_flag
	( MUP::attr_pointer_type<vecpt> & ap,
	  unsigned n )
{
    typedef MUP::attr_pointer_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::test_flag called before locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::test_flag" );
    }
}

// Helper functions for min::get_attrs.
//
// Called with current(lp) being start of
// double-arrow-sublist.  Return number of reverse
// attribute names with non-empty value sets.
//
static min::unsptr count_reverse_attrs
	( min::list_pointer & lp )
{
    min::unsptr result = 0;
    min::list_pointer lpr
        ( min::vec_pointer_of ( lp ) );
    start_sublist ( lpr, lp );
    for ( min::gen c = min::current ( lpr );
          ! min::is_list_end ( c );
	  c = min::next ( lpr ) )
    {
        c = min::next ( lpr );
	if ( c != min::EMPTY_SUBLIST )
	    ++ result;
    }
    return result;
}

// Called with current(lp) equal to attribute-/node-
// descriptor for attribute.  Compute counts for
// attribute in info.  Does NOT set info.name.
// Return true if some count is non-zero and false if
// all counts are zero.
//
static bool compute_counts
	( min::list_pointer & lp,
	  min::attr_info & info )
{
    info.value_count = 0;
    info.flag_count = 0;
    info.reverse_attr_count = 0;

    min::gen c = min::current ( lp );
    if ( ! min::is_sublist ( c ) )
    {
        ++ info.value_count;
	return true;
    }
    else if ( c == min::EMPTY_SUBLIST )
        return false;
    else
    {
        min::list_pointer lpv
	    ( min::vec_pointer_of ( lp ) );
	min::start_sublist ( lpv, lp );
	for ( c = min::current ( lpv );
	      ! min::is_list_end ( c );
	      c = min::next ( lpv ) )
	{
#   	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		if ( min::is_sublist ( c ) )
		    c = min::next ( lpv );
#   	    endif
	    if ( min::is_sublist ( c ) )
	        info.reverse_attr_count =
		    count_reverse_attrs ( lpv );
	    else if ( min::is_control_code ( c ) )
	        ++ info.flag_count;
	    else
	        ++ info.value_count;
	}

	return (    info.value_count > 0
		 || info.flag_count > 0
		 || info.reverse_attr_count > 0 );
    }
}

# if MIN_ALLOW_PARTIAL_ATTR_LABELS
    // Compute labels associated with the children of
    // the node whose node-descriptor is pointed at
    // by lp.  The label of the node is
    // components[0 .. depth-1].  Output is to go into
    // `* out ++' if `result < n' and result is to be
    // incremented regardless.
    //
    static void compute_children
	( min::list_pointer & lp,
	  min::gen * components, min::unsptr depth,
	  min::attr_info * & out, min::unsptr & result,
	  min::unsptr n )
    {
	min::gen c = min::current ( lp );
	if ( ! min::is_sublist ( c ) ) return;
	min::list_pointer lpv
	    ( min::vec_pointer_of ( lp ) );
	min::start_sublist ( lpv, lp );
	for ( c = min::current ( lpv );
		 ! min::is_list_end ( c )
	      && ! min::is_sublist ( c );
	      c = min::next ( lpv ) );
	if ( ! min::is_sublist ( c ) ) return;
	if ( c == min::EMPTY_SUBLIST ) return;
	start_sublist ( lpv );

	min::gen labvec[depth+1];
	for ( min::unsptr i = 0; i < depth; ++ i )
	    labvec[i] = * components ++;

	for ( c = min::current ( lpv );
	      ! min::is_list_end ( c );
	      c = min::next ( lpv ) )
	{
	    labvec[depth] = c;
	    min::next ( lpv );
	    min::attr_info info;
	    if ( compute_counts ( lpv, info ) )
	    {
		if ( result < n )
		{
		    info.name = min::new_lab_gen
				  ( labvec, depth + 1 );
		    * out ++ = info;
		}
		++ result;
	    }
	    compute_children ( lpv, labvec, depth + 1,
			       out, result, n );
	}
    }
# endif

template < class vecpt >
min::unsptr min::get_attrs
	( min::attr_info * out, min::unsptr n,
	  unprotected::attr_pointer_type
	      < vecpt > & ap )
{
    attr_info info;

    vecpt & vp = vec_pointer_of ( ap );
    min::list_pointer lp ( vp );

    min::unsptr result = 0;
    for ( min::unsptr i = 0;
          i < hash_size_of ( vp );
	  ++ i )
    {
	start_hash ( lp, i );
	for ( min::gen c = current ( lp );
	      ! is_list_end ( c );
	      c = next ( lp ) )
	{
	    info.name = c;
#	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		// If info.name is a label then being a
		// the first component of a one
		// component name it must be embedded
		// in a label.
		if ( is_lab ( info.name ) )
		    info.name =
		        new_lab_gen ( & info.name, 1 );
#	    endif
	    next ( lp );
	    if ( compute_counts ( lp, info ) )
	    {
		if ( result < n )
		    * out ++ = info;
		++ result;
	    }
#	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		compute_children ( lp, & c, 1,
				   out, result, n );
#	    endif
	}
    }
    for ( min::unsptr i = 0;
          i < attr_size_of ( vp );
	  ++ i )
    {
	start_vector ( lp, i );
	if ( is_list_end ( current ( lp ) ) )
	    continue;

	info.name = new_num_gen ( i );
	if ( compute_counts ( lp, info ) )
	{
	    if ( result < n ) * out ++ = info;
	    ++ result;
	}
#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    compute_children ( lp, & info.name, 1,
			       out, result, n );
#	endif
    }
}

template < class vecpt >
min::unsptr min::get_reverse_attrs
	( min::reverse_attr_info * out, min::unsptr n,
	  unprotected::attr_pointer_type
	      < vecpt > & ap )
{
    typedef MUP::attr_pointer_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::get_reverse_attrs called before"
	      " locate" );
    case ap_type::LOCATE_FAIL:
        return 0;
    }

    min::gen c = update_refresh ( ap.locate_dlp );
    if ( ! is_sublist ( c ) ) return 0;
    start_sublist ( ap.lp, ap.locate_dlp );
    for ( c = current ( ap.lp );
             ! is_sublist ( c )
	  && ! is_list_end ( c );
	  c = next ( ap.lp ) );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	if ( ! is_sublist ( c ) ) return 0;
        next ( ap.lp );
#   endif
    if ( ! is_sublist ( c ) ) return 0;
    start_sublist ( ap.lp );

    list_pointer lpv ( vec_pointer_of ( ap.lp ) );
    reverse_attr_info info;
    min::unsptr result = 0;
    for ( c = current ( ap.lp );
          ! is_list_end ( c );
	  c = next ( ap.lp ) )
    {
        info.name = c;
	info.value_count = 0;
	c = next ( ap.lp );
	if ( ! is_sublist ( c ) )
	    ++ info.value_count;
	else if ( c == min::EMPTY_SUBLIST )
	    continue;
	else
	{
	    start_sublist ( lpv, ap.lp );
	    for ( c = current ( lpv );
	          ! is_list_end ( c );
		  c = next ( lpv ) )
	        ++ info.value_count;
	}
	if ( result < n ) * out ++ = info;
	++ result;
    }

    return result;
}

template < class vecpt >
inline min::gen MINT::update
	( MUP::attr_pointer_type<vecpt> & ap,
	  min::gen v )
{
    typedef MUP::attr_pointer_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::update called before locate" );
    case ap_type::LOCATE_FAIL:
	MIN_ABORT
	    ( "min::update called with no previous"
	      " value" );
    case ap_type::REVERSE_LOCATE_FAIL:
    case ap_type::REVERSE_LOCATE_SUCCEED:
    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::update called with reverse name"
	      " other than min::NONE" );
    default:
	MIN_ABORT
	    ( "min::update called with >= 2 previous"
	      " values" );
    }
}

// Helper functions for various set and remove
// functions.  See min.h for documentation.
//
void MINT::remove_reverse_attr_value
	( min::insertable_attr_pointer & ap,
	  min::insertable_vec_pointer & vp )
{
    typedef min::insertable_attr_pointer ap_type;

    insertable_attr_pointer rap ( vp );
    min::gen v = min::new_gen
        ( MUP::stub_of ( vec_pointer_of ( ap.dlp ) ) );

    min::locate ( rap, ap.reverse_attr_name );
    min::locate_reverse ( rap, ap.attr_name );
    MIN_ASSERT
        (    rap.state
	  == ap_type::REVERSE_LOCATE_SUCCEED );

    min::gen c = current ( rap.dlp );
    if ( ! is_sublist ( c ) )
    {
        MIN_ASSERT ( c == v );
	min::update ( rap, min::EMPTY_SUBLIST );
	return;
    }
    start_sublist ( rap.dlp );
    for ( c = current ( rap.dlp );
          c != v && ! is_list_end ( c );
	  c = next ( rap.dlp ) );
    MIN_ASSERT ( c == v );
    min::remove ( rap.dlp, 1 );
}
    
inline void MINT::remove_reverse_attr_value
	( min::insertable_attr_pointer & ap,
          min::gen v )
{
    insertable_vec_pointer & apvp =
        vec_pointer_of ( ap.dlp );
    min::stub * aps = MUP::stub_of ( apvp );
    const min::stub * s = min::stub_of ( v );
    if ( s == aps )
    {
        if ( ap.attr_name == ap.reverse_attr_name )
	    return;
	MINT::remove_reverse_attr_value ( ap, apvp );
    }
    else
    {
        insertable_vec_pointer vp ( v );
	MINT::remove_reverse_attr_value ( ap, vp );
    }
}

void MINT::add_reverse_attr_value
	( min::insertable_attr_pointer & ap,
	  min::insertable_vec_pointer & vp )
{
    typedef min::insertable_attr_pointer ap_type;

    insertable_attr_pointer rap ( vp );
    min::gen v = min::new_gen
        ( MUP::stub_of ( vec_pointer_of ( ap.dlp ) ) );

    min::locate ( rap, ap.reverse_attr_name );
    if ( rap.state != ap_type::LOCATE_NONE )
        attr_create ( rap, min::EMPTY_SUBLIST );
    min::locate_reverse ( rap, ap.attr_name );
    if ( rap.state != ap_type::REVERSE_LOCATE_SUCCEED )
    {
        reverse_attr_create ( rap, v );
	return;
    }
    MIN_ASSERT
        (    rap.state
	  == ap_type::REVERSE_LOCATE_SUCCEED );

    min::gen c = current ( rap.dlp );
    if ( ! is_sublist ( c ) )
    {
	min::update ( rap.dlp, min::EMPTY_SUBLIST );
	start_sublist ( rap.dlp );
	insert_reserve ( rap.dlp, 1, 2 );
	min::gen elements[2] = { c, v };
	insert_before ( rap.dlp, elements, 2 );
	return;
    }
    start_sublist ( rap.dlp );
    insert_reserve ( rap.dlp, 1, 1 );
    min::gen elements[1] = { v };
    insert_before ( rap.dlp, elements, 1 );
}
    
inline void MINT::add_reverse_attr_value
	( min::insertable_attr_pointer & ap,
          min::gen v )
{
    insertable_vec_pointer & apvp =
        vec_pointer_of ( ap.dlp );
    min::stub * aps = MUP::stub_of ( apvp );
    const min::stub * s = min::stub_of ( v );
    if ( s == aps )
    {
        if ( ap.attr_name == ap.reverse_attr_name )
	    return;
	MINT::add_reverse_attr_value ( ap, apvp );
	insert_refresh ( ap.locate_dlp );
	insert_refresh ( ap.dlp );
	insert_refresh ( ap.lp );
    }
    else
    {
        insertable_vec_pointer vp ( v );
	MINT::add_reverse_attr_value ( ap, vp );
    }
}

void MINT::set
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, min::unsptr n )
{
    typedef min::insertable_attr_pointer ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::set called before locate" );

    case ap_type::LOCATE_ANY:

	if ( n > 0 )
	    MIN_ABORT
		( "min::set called after reverse locate"
		  " of min::ANY" );
	else
	{
	    min::gen c =
	        update_refresh ( ap.locate_dlp );
	    if ( ! is_sublist ( c ) ) return;
	    start_sublist ( ap.dlp, ap.locate_dlp );
	    for ( c = current ( ap.dlp );
		     ! is_sublist ( c )
		  && ! is_list_end ( c );
		  c = next ( ap.dlp ) );
	    if ( ! is_sublist ( c ) ) return;
#   	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		c = next ( ap.dlp );
		if ( ! is_sublist ( c ) ) return;
#   	    endif
	    start_sublist ( ap.dlp );
	    for ( c = current ( ap.dlp );
		  ! is_list_end ( c );
		  c = next ( ap.lp ) )
	    {
	        c = next ( ap.dlp );
		if ( is_sublist ( c ) )
		{
		    start_sublist ( ap.lp, ap.dlp );
		    for ( c = current ( ap.lp );
		          ! is_list_end ( c );
			  c = next ( ap.lp ) )
			remove_reverse_attr_value
			    ( ap, c );
		}
		else
		    remove_reverse_attr_value ( ap, c );

		update ( ap.dlp, min::EMPTY_SUBLIST );
	    }
	}
	return;

    case ap_type::LOCATE_FAIL:
	if ( n == 0 )
	    return;
	else if ( ap.reverse_attr_name == min::ANY )
	    MIN_ABORT
		( "min::set called after reverse locate"
		  " of min::ANY" );
        else if ( n == 1
	          &&
		  ap.reverse_attr_name == min::NONE )
	{
	    MINT::attr_create ( ap, * in );
	    return;
	}
        else
	    MINT::attr_create
	        ( ap, min::EMPTY_SUBLIST );

	if ( ap.reverse_attr_name == min::NONE )
	    break;

    case ap_type::REVERSE_LOCATE_FAIL:
	if ( n == 0 )
	    return;
        else if ( n == 1 )
	{
	    MINT::reverse_attr_create ( ap, * in );
	    return;
	}
        else
	    MINT::reverse_attr_create
	        ( ap, min::EMPTY_SUBLIST );
	break;
    }

    min::gen c = update_refresh ( ap.dlp );

    bool is_reverse =
         (    ap.state
	   == ap_type::REVERSE_LOCATE_SUCCEED );
    if ( n == 0 )
    {
        if ( is_sublist ( c ) )
	{
	    if ( is_empty_sublist ( c ) ) return;

	    if ( is_reverse )
	    {
		start_sublist ( ap.lp, ap.dlp );
		for ( c = current ( ap.lp );
		      ! is_list_end ( c );
		      c = next ( ap.lp ) )
		    remove_reverse_attr_value
		        ( ap, c );
		update ( ap.dlp, min::EMPTY_SUBLIST );
	    }
	    else
	    {
		start_sublist ( ap.lp, ap.dlp );
		for ( c = current ( ap.lp );
		         ! is_list_end ( c )
		      && ! is_sublist ( c )
		      && ! is_control_code ( c );
		    )
		    remove ( ap.lp, 1 );
	    }
	}
	else
	{
	    if ( is_reverse )
		remove_reverse_attr_value
		    ( ap, current ( ap.dlp ) );
	    update ( ap.dlp, min::EMPTY_SUBLIST );
	}
	return;
    }

    if ( ! is_sublist ( c ) )
    {
	if ( is_reverse )
	    remove_reverse_attr_value ( ap, c );

        if ( n == 1 )
	    update ( ap.dlp, * in );
	else
	{
	    update ( ap.dlp, min::EMPTY_SUBLIST );
	    start_sublist ( ap.lp, ap.dlp );
	    if ( insert_reserve ( ap.lp, 1, n ) )
	    {
		insert_refresh ( ap.dlp );
		insert_refresh ( ap.locate_dlp );
	    }
	    insert_before ( ap.lp, in, n );
	}
    }
    else
    {
    	start_sublist ( ap.lp, ap.dlp );
	min::unsptr k = 0;
	for ( c == current ( ap.lp );
	         n > k
	      && ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c );
	      c = next ( ap.lp ) )
	{
	    if ( is_reverse )
		remove_reverse_attr_value ( ap, c );
	    update ( ap.lp, in[k++] );
	}
	if ( n > k )
	{
	    if ( insert_reserve ( ap.lp, 1, n - k ) )
	    {
		insert_refresh ( ap.dlp );
		insert_refresh ( ap.locate_dlp );
	    }
	    insert_before ( ap.lp, in + k, n - k );
	}
	else if (    ! is_list_end ( c )
	          && ! is_sublist ( c )
		  && ! is_control_code ( c ) )
	{
	    do
	    {
		if ( is_reverse )
		    remove_reverse_attr_value ( ap, c );
		remove ( ap.lp, 1 );
		c = current ( ap.lp );
	    } while (    ! is_list_end ( c )
	              && ! is_sublist ( c )
		      && ! is_control_code ( c ) );
	}
    }
    if ( is_reverse ) while ( n -- )
	add_reverse_attr_value ( ap, * in ++ );
}

void min::add_to_set
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, min::unsptr n )
{
    typedef min::insertable_attr_pointer ap_type;

    if ( n == 0 ) return;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::add_to_set called before"
	      " locate" );

    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::add_to_set called after"
		  " reverse locate of min::ANY" );

    case ap_type::LOCATE_FAIL:
	if ( ap.reverse_attr_name == min::ANY )
	    MIN_ABORT
		( "min::add_to_set called after"
		  " reverse locate of min::ANY" );
    case ap_type::REVERSE_LOCATE_FAIL:
	add_to_multiset ( ap, in, n );
	return;
    }

    min::gen c = update_refresh ( ap.dlp );

    if ( ! is_sublist ( c ) )
    {
        min::gen additions[n];
	min::unsptr m = 0;
	while ( n -- )
	{
	    min::gen v = * in ++;
	    if ( c != v ) additions[m++] = v;
	}
	if ( m > 0 ) add_to_multiset
	                 ( ap, additions, m );
    }
    else
    {
	// Copy input to additions, and then replace
	// every value already in multiset by
	// min::NONE.  Lastly compact additions and
	// call add_to_multiset.
	//
        min::gen additions[n];
	min::unsptr m = 0;
	while ( n -- ) additions[m++] = * in ++;
	start_sublist ( ap.lp, ap.dlp );
	for ( c == current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c );
	      c = next ( ap.lp ) )
	for ( min::unsptr i = 0; i < m; ++ i )
	{
	    if ( additions[i] == c )
	        additions[i] = min::NONE;
	}
	min::unsptr j = 0;
	for ( min::unsptr k = 0; k < m; ++ k )
	{
	    if ( additions[k] != min::NONE )
	        additions[j++] = additions[k];
	}
	add_to_multiset ( ap, additions, j );
    }
}

void min::add_to_multiset
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, min::unsptr n )
{
    typedef min::insertable_attr_pointer ap_type;

    if ( n == 0 ) return;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::add_to_multiset called before"
	      " locate" );

    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::add_to_multiset called after"
		  " reverse locate of min::ANY" );

    case ap_type::LOCATE_FAIL:
	if ( ap.reverse_attr_name == min::ANY )
	    MIN_ABORT
		( "min::add_to_multiset called after"
		  " reverse locate of min::ANY" );
        else if ( n == 1
	          &&
		  ap.reverse_attr_name == min::NONE )
	{
	    MINT::attr_create ( ap, * in );
	    return;
	}
        else
	    MINT::attr_create
	        ( ap, min::EMPTY_SUBLIST );

	if ( ap.reverse_attr_name == min::NONE )
	    break;

    case ap_type::REVERSE_LOCATE_FAIL:
        if ( n == 1 )
	{
	    MINT::reverse_attr_create ( ap, * in );
	    return;
	}
        else
	    MINT::reverse_attr_create
	        ( ap, min::EMPTY_SUBLIST );
	break;
    }

    min::gen c = update_refresh ( ap.dlp );

    if ( ! is_sublist ( c ) )
    {
	update ( ap.dlp, min::EMPTY_SUBLIST );
	start_sublist ( ap.lp, ap.dlp );
	if ( insert_reserve ( ap.lp, 2, n + 1 ) )
	{
	    insert_refresh ( ap.dlp );
	    insert_refresh ( ap.locate_dlp );
	}
	min::gen element[1] = { c };
	insert_before ( ap.lp, element, 1 );
	insert_after ( ap.lp, in, n );
    }
    else
    {
    	start_sublist ( ap.lp, ap.dlp );
	if ( insert_reserve ( ap.lp, 1, n ) )
	{
	    insert_refresh ( ap.dlp );
	    insert_refresh ( ap.locate_dlp );
	}
	insert_before ( ap.lp, in, n );
    }

    if (    ap.state
	 == ap_type::REVERSE_LOCATE_SUCCEED )
	while ( n -- )
	    MINT::add_reverse_attr_value
	        ( ap, * in ++ );
}

min::unsptr min::remove_one
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, min::unsptr n )
{
    typedef min::insertable_attr_pointer ap_type;

    if ( n == 0 ) return 0;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::remove_one called before"
	      " locate" );

    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::remove_one called after"
		  " reverse locate of min::ANY" );

    case ap_type::LOCATE_FAIL:
	if ( ap.reverse_attr_name == min::ANY )
	    MIN_ABORT
		( "min::remove_one called after"
		  " reverse locate of min::ANY" );
    case ap_type::REVERSE_LOCATE_FAIL:
	return 0;
    }

    min::gen c = update_refresh ( ap.dlp );

    bool is_reverse =
         (    ap.state
	   == ap_type::REVERSE_LOCATE_SUCCEED );
    if ( ! is_sublist ( c ) )
    {
	while ( n -- )
	{
	    if ( * in ++ == c )
	    {
		if ( is_reverse )
		    MINT::remove_reverse_attr_value
		        ( ap, c );
	        update ( ap.dlp, min::EMPTY_SUBLIST );
		return 1;
	    }
	}
	return 0;
    }
    else
    {
	// Copy input to removals, and then replace
	// every value already in removed by
	// min::NONE.  If we have removed each input
	// once, we can terminate early.
	//
        min::gen removals[n];
	for ( min::unsptr j = 0; j < n; )
	    removals[j++] = * in ++;

	min::unsptr result = 0;
	min::unsptr i;
	start_sublist ( ap.lp, ap.dlp );
	for ( c == current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c )
	      && result < n;
	    )
	{
	    for ( i = 0; i < n; ++ i )
	    {
		if ( removals[i] == c )
		    break;
	    }
	    if ( i < n )
	    {
		if ( is_reverse )
		    MINT::remove_reverse_attr_value
			( ap, c );
		remove ( ap.lp, 1 );
		removals[i] = min::NONE;
		++ result;
		c = current ( ap.lp );
	    }
	    else c = next ( ap.dlp );
	}
        return result;
    }
}

min::unsptr min::remove_all
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, min::unsptr n )
{
    typedef min::insertable_attr_pointer ap_type;

    if ( n == 0 ) return 0;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::remove_all called before"
	      " locate" );

    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::remove_all called after"
		  " reverse locate of min::ANY" );

    case ap_type::LOCATE_FAIL:
	if ( ap.reverse_attr_name == min::ANY )
	    MIN_ABORT
		( "min::remove_all called after"
		  " reverse locate of min::ANY" );
    case ap_type::REVERSE_LOCATE_FAIL:
	return 0;
    }

    min::gen c = update_refresh ( ap.dlp );

    bool is_reverse =
         (    ap.state
	   == ap_type::REVERSE_LOCATE_SUCCEED );
    if ( ! is_sublist ( c ) )
    {
	while ( n -- )
	{
	    if ( * in ++ == c )
	    {
		if ( is_reverse )
		    MINT::remove_reverse_attr_value
		        ( ap, c );
	        update ( ap.dlp, min::EMPTY_SUBLIST );
		return 1;
	    }
	}
	return 0;
    }
    else
    {
	min::unsptr result = 0;
	min::unsptr i;
	start_sublist ( ap.lp, ap.dlp );
	for ( c == current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c );
	    )
	{
	    for ( i = 0; i < n; ++ i )
	    {
		if ( in[i] == c )
		    break;
	    }
	    if ( i < n )
	    {
		if ( is_reverse )
		    MINT::remove_reverse_attr_value
			( ap, c );
		remove ( ap.lp, 1 );
		++ result;
		c = current ( ap.lp );
	    }
	    else c = next ( ap.dlp );
	}
        return result;
    }
}

void MINT::set_flags
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, unsigned n )
{
    typedef min::insertable_attr_pointer ap_type;

    for ( unsigned i = 0; i < n; ++ i )
        MIN_ASSERT ( is_control_code ( in[i] ) );

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::set_flags called before locate" );
    case ap_type::LOCATE_FAIL:
    	    MINT::attr_create
	              ( ap, min::EMPTY_SUBLIST );
    }

    min::gen c = update_refresh ( ap.locate_dlp );
    start_copy ( ap.lp, ap.locate_dlp );
    if ( ! is_sublist ( c ) )
    {
        if ( n == 0 ) return;

        if ( insert_reserve ( ap.lp, 2, n + 1 ) )
	{
	    insert_refresh ( ap.dlp );
	    insert_refresh ( ap.locate_dlp );
	}
	update ( ap.lp, min::EMPTY_SUBLIST );
	start_sublist ( ap.lp );
	min::gen element[1] = { c };
	insert_before ( ap.lp, element, 1 );
	insert_after ( ap.lp, in, n );
    }
    else
    {
        start_sublist ( ap.lp );
	for ( c = current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_control_code ( c );
	      c = next ( ap.lp ) );
	while ( is_control_code ( c ) )
	{
	    if ( n > 0 )
	    {
		update ( ap.lp, * in ++ );
		-- n;
		c = next ( ap.lp );
	    }
	    else
	    {
	        remove ( ap.lp );
		c = current ( ap.lp );
	    }
	}
	if ( n > 0 )
	{
	    if ( insert_reserve ( ap.lp, 1, n ) )
	    {
		insert_refresh ( ap.dlp );
		insert_refresh ( ap.locate_dlp );
	    }
	    insert_before ( ap.lp, in, n );
	}
    }
}

// Appends control codes to the current attribute's
// existing list of control codes.  ap.lp points at
// place to do insertion.
//
void MINT::set_more_flags
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, unsigned n )
{
    typedef min::insertable_attr_pointer ap_type;

    for ( unsigned i = 0; i < n; ++ i )
        MIN_ASSERT ( is_control_code ( in[i] ) );

    if ( insert_reserve ( ap.lp, 1, n ) )
    {
	insert_refresh ( ap.dlp );
	insert_refresh ( ap.locate_dlp );
    }
    insert_before ( ap.lp, in, n );
}

void MINT::set_some_flags
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, unsigned n )
{
    typedef insertable_attr_pointer ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::set_some_flags called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::set_some_flags" );
    }
}

void MINT::clear_some_flags
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, unsigned n )
{
    typedef insertable_attr_pointer ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::clear_some_flags called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to"
	      " min::clear_some_flags" );
    }
}

void MINT::flip_some_flags
	( min::insertable_attr_pointer & ap,
	  const min::gen * in, unsigned n )
{
    typedef insertable_attr_pointer ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::flip_some_flags called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::flip_some_flags" );
    }
}

// n is the flag number within otherwise 0 control codes
// that are to be added to the end of the attribute-/
// node-descriptor.
//
bool MINT::set_flag
	( min::insertable_attr_pointer & ap,
	  unsigned n )
{
    typedef insertable_attr_pointer ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::set_flag called before locate" );
    case ap_type::LOCATE_FAIL:
    	    MINT::attr_create
	              ( ap, min::EMPTY_SUBLIST );
    }

    min::gen elements[(n+VSIZE-1)/VSIZE + 1];
    min::unsptr j = 0;
    min::gen c = current ( ap.locate_dlp );
    if ( ! is_sublist ( c ) )
    {
        elements[j++] = c;
	update ( ap.locate_dlp, min::EMPTY_SUBLIST );
    }
    start_sublist ( ap.lp, ap.locate_dlp );
    for ( c = current ( ap.lp );
          ! is_list_end ( c );
	  c = next ( ap.lp ) );
    unsigned base = 0;
    while ( base < n )
    {
        unsigned next = base + VSIZE;
	if ( n < next )
	{
	    elements[j++] = new_control_code_gen
	        ( 1 << ( n - base ) );
	    break;
	}
	else
	    elements[j++] = new_control_code_gen ( 0 );

	base = next;
    }
    if ( insert_reserve ( ap.lp, 1, j ) )
    {
        insert_refresh ( ap.locate_dlp );
        insert_refresh ( ap.dlp );
    }
    insert_before ( ap.lp, elements, j );
    return false;
}

bool MINT::clear_flag
	( min::insertable_attr_pointer & ap,
	  unsigned n )
{
    typedef insertable_attr_pointer ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::clear_flag called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::clear_flag" );
    }
    return false;
}

bool MINT::flip_flag
	( min::insertable_attr_pointer & ap,
	  unsigned n )
{
    typedef insertable_attr_pointer ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::flip_flag called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::flip_flag" );
    }
    return false;
}
