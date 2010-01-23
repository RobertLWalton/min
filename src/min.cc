// MIN Language Out-of-Line Code
//
// File:	min.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Jan 23 03:40:18 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/01/23 08:52:10 $
//   $RCSfile: min.cc,v $
//   $Revision: 1.118 $

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
//	Objects
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

    unsigned number_of_free_stubs;

    min::stub ** str_hash;
    unsigned str_hash_size;
    unsigned str_hash_mask;

    min::stub ** num_hash;
    unsigned num_hash_size;
    unsigned num_hash_mask;

    min::stub ** lab_hash;
    unsigned lab_hash_size;
    unsigned lab_hash_mask;

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

    unsigned max_fixed_block_size;

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
	unsigned hash = floathash ( v );
	unsigned h = hash & MINT::num_hash_mask;
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

char * min::strncpy ( char * p, min::gen v, unsigned n )
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
	( const char * p, min::gen v, unsigned n )
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
	( const char * p, unsigned n )
{
    unsigned hash = min::strnhash ( p, n );
    unsigned h = hash & MINT::str_hash_mask;
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
	( const min::gen * p, unsigned n )
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
	( const min::gen * p, unsigned n )
{
    uns32 hash = labhash ( p, n );
    unsigned h = hash & MINT::lab_hash_mask;

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

// Objects
// -------

namespace min { namespace internal {

    min::uns32 hash_size [] =
    {
	// [0 .. 15]
		  0,          1,          3,          5,
		  7,          9,         11,         13,
		 15,         19,         23,         27,
		 31,         35,         39,         43,
	// [16 .. 31]
		 47,         51,         55,         59,
		 63,         67,         71,         75,
		 79,         83,         87,         91,
		 95,         99,        103,        107,
	// [32 .. 47]
		111,        115,        119,        123,
		127,        131,        135,        139,
		143,        147,        151,        155,
		159,        163,        167,        171,
	// [48 .. 63]
		175,        179,        183,        187,
		191,        195,        199,        203,
		207,        211,        215,        219,
		223,        227,        231,        235,
	// [64 .. 79]
		239,        243,        247,        251,
		255,        259,        263,        267,
		271,        275,        279,        283,
		287,        291,        295,        299,
	// [80 .. 95]
		303,        307,        311,        315,
		319,        323,        327,        331,
		335,        339,        343,        347,
		351,        355,        359,        363,
	// [96 .. 111]
		367,        371,        375,        379,
		383,        387,        391,        395,
		399,        403,        407,        411,
		415,        419,        423,        427,
	// [112 .. 127]
		431,        435,        439,        443,
		447,        451,        455,        459,
		463,        467,        471,        475,
		479,        483,        487,        491,
	// [128 .. 143]
		495,        499,        503,        507,
		511,        515,        519,        523,
		527,        531,        535,        539,
		543,        547,        551,        555,
	// [144 .. 159]
		559,        563,        567,        571,
		575,        579,        583,        587,
		591,        595,        599,        603,
		609,        615,        621,        627,
	// [160 .. 175]
		633,        639,        645,        651,
		657,        663,        669,        675,
		681,        687,        693,        699,
		705,        711,        717,        723,
	// [176 .. 191]
		729,        735,        741,        747,
		753,        759,        765,        771,
		777,        783,        789,        795,
		801,        809,        817,        825,
	// [192 .. 207]
		833,        841,        849,        857,
		865,        873,        881,        889,
		897,        905,        913,        921,
		929,        937,        945,        953,
	// [208 .. 223]
		961,        969,        977,        985,
		993,       1001,       1011,       1021,
	       1031,       1041,       1051,       1061,
	       1071,       1081,       1091,       1101,
	// [224 .. 239]
	       1111,       1121,       1131,       1141,
	       1151,       1161,       1171,       1181,
	       1191,       1201,       1213,       1225,
	       1237,       1249,       1261,       1273,
	// [240 .. 255]
	       1285,       1297,       1309,       1321,
	       1333,       1345,       1357,       1369,
	       1381,       1393,       1405,       1419,
	       1433,       1447,       1461,       1475,
	// [256 .. 271]
	       1489,       1503,       1517,       1531,
	       1545,       1559,       1573,       1587,
	       1601,       1617,       1633,       1649,
	       1665,       1681,       1697,       1713,
	// [272 .. 287]
	       1729,       1745,       1761,       1777,
	       1793,       1809,       1827,       1845,
	       1863,       1881,       1899,       1917,
	       1935,       1953,       1971,       1989,
	// [288 .. 303]
	       2007,       2027,       2047,       2067,
	       2087,       2107,       2127,       2147,
	       2167,       2187,       2207,       2229,
	       2251,       2273,       2295,       2317,
	// [304 .. 319]
	       2339,       2361,       2383,       2405,
	       2429,       2453,       2477,       2501,
	       2525,       2549,       2573,       2597,
	       2621,       2647,       2673,       2699,
	// [320 .. 335]
	       2725,       2751,       2777,       2803,
	       2831,       2859,       2887,       2915,
	       2943,       2971,       2999,       3027,
	       3057,       3087,       3117,       3147,
	// [336 .. 351]
	       3177,       3207,       3239,       3271,
	       3303,       3335,       3367,       3399,
	       3431,       3465,       3499,       3533,
	       3567,       3601,       3637,       3673,
	// [352 .. 367]
	       3709,       3745,       3781,       3817,
	       3855,       3893,       3931,       3969,
	       4007,       4047,       4087,       4127,
	       4167,       4207,       4249,       4291,
	// [368 .. 383]
	       4333,       4375,       4417,       4461,
	       4505,       4549,       4593,       4637,
	       4683,       4729,       4775,       4821,
	       4869,       4917,       4965,       5013,
	// [384 .. 399]
	       5063,       5113,       5163,       5213,
	       5265,       5317,       5369,       5421,
	       5475,       5529,       5583,       5637,
	       5693,       5749,       5805,       5863,
	// [400 .. 415]
	       5921,       5979,       6037,       6097,
	       6157,       6217,       6279,       6341,
	       6403,       6467,       6531,       6595,
	       6659,       6725,       6791,       6857,
	// [416 .. 431]
	       6925,       6993,       7061,       7131,
	       7201,       7273,       7345,       7417,
	       7491,       7565,       7639,       7715,
	       7791,       7867,       7945,       8023,
	// [432 .. 447]
	       8103,       8183,       8263,       8345,
	       8427,       8511,       8595,       8679,
	       8765,       8851,       8939,       9027,
	       9117,       9207,       9299,       9391,
	// [448 .. 463]
	       9483,       9577,       9671,       9767,
	       9863,       9961,      10059,      10159,
	      10259,      10361,      10463,      10567,
	      10671,      10777,      10883,      10991,
	// [464 .. 479]
	      11099,      11209,      11321,      11433,
	      11547,      11661,      11777,      11893,
	      12011,      12131,      12251,      12373,
	      12495,      12619,      12745,      12871,
	// [480 .. 495]
	      12999,      13127,      13257,      13389,
	      13521,      13655,      13791,      13927,
	      14065,      14205,      14347,      14489,
	      14633,      14779,      14925,      15073,
	// [496 .. 511]
	      15223,      15375,      15527,      15681,
	      15837,      15995,      16153,      16313,
	      16475,      16639,      16805,      16973,
	      17141,      17311,      17483,      17657,
	// [512 .. 527]
	      17833,      18011,      18191,      18371,
	      18553,      18737,      18923,      19111,
	      19301,      19493,      19687,      19883,
	      20081,      20281,      20483,      20687,
	// [528 .. 543]
	      20893,      21101,      21311,      21523,
	      21737,      21953,      22171,      22391,
	      22613,      22839,      23067,      23297,
	      23529,      23763,      23999,      24237,
	// [544 .. 559]
	      24479,      24723,      24969,      25217,
	      25469,      25723,      25979,      26237,
	      26499,      26763,      27029,      27299,
	      27571,      27845,      28123,      28403,
	// [560 .. 575]
	      28687,      28973,      29261,      29553,
	      29847,      30145,      30445,      30749,
	      31055,      31365,      31677,      31993,
	      32311,      32633,      32959,      33287,
	// [576 .. 591]
	      33619,      33955,      34293,      34635,
	      34981,      35329,      35681,      36037,
	      36397,      36759,      37125,      37495,
	      37869,      38247,      38629,      39015,
	// [592 .. 607]
	      39405,      39799,      40195,      40595,
	      40999,      41407,      41821,      42239,
	      42661,      43087,      43517,      43951,
	      44389,      44831,      45279,      45731,
	// [608 .. 623]
	      46187,      46647,      47113,      47583,
	      48057,      48537,      49021,      49511,
	      50005,      50505,      51009,      51519,
	      52033,      52553,      53077,      53607,
	// [624 .. 639]
	      54143,      54683,      55229,      55781,
	      56337,      56899,      57467,      58041,
	      58621,      59207,      59799,      60395,
	      60997,      61605,      62221,      62843,
	// [640 .. 655]
	      63471,      64105,      64745,      65391,
	      66043,      66703,      67369,      68041,
	      68721,      69407,      70101,      70801,
	      71509,      72223,      72945,      73673,
	// [656 .. 671]
	      74409,      75153,      75903,      76661,
	      77427,      78201,      78983,      79771,
	      80567,      81371,      82183,      83003,
	      83833,      84671,      85517,      86371,
	// [672 .. 687]
	      87233,      88105,      88985,      89873,
	      90771,      91677,      92593,      93517,
	      94451,      95395,      96347,      97309,
	      98281,      99263,     100255,     101257,
	// [688 .. 703]
	     102269,     103291,     104323,     105365,
	     106417,     107481,     108555,     109639,
	     110735,     111841,     112959,     114087,
	     115227,     116379,     117541,     118715,
	// [704 .. 719]
	     119901,     121099,     122309,     123531,
	     124765,     126011,     127271,     128543,
	     129827,     131125,     132435,     133759,
	     135095,     136445,     137809,     139187,
	// [720 .. 735]
	     140577,     141981,     143399,     144831,
	     146279,     147741,     149217,     150709,
	     152215,     153737,     155273,     156825,
	     158393,     159975,     161573,     163187,
	// [736 .. 751]
	     164817,     166465,     168129,     169809,
	     171507,     173221,     174953,     176701,
	     178467,     180251,     182053,     183873,
	     185711,     187567,     189441,     191335,
	// [752 .. 767]
	     193247,     195179,     197129,     199099,
	     201089,     203099,     205129,     207179,
	     209249,     211341,     213453,     215587,
	     217741,     219917,     222115,     224335,
	// [768 .. 783]
	     226577,     228841,     231129,     233439,
	     235773,     238129,     240509,     242913,
	     245341,     247793,     250269,     252771,
	     255297,     257849,     260427,     263031,
	// [784 .. 799]
	     265661,     268317,     270999,     273707,
	     276443,     279207,     281999,     284817,
	     287665,     290541,     293445,     296379,
	     299341,     302333,     305355,     308407,
	// [800 .. 815]
	     311491,     314605,     317751,     320927,
	     324135,     327375,     330647,     333953,
	     337291,     340663,     344069,     347509,
	     350983,     354491,     358035,     361615,
	// [816 .. 831]
	     365231,     368883,     372571,     376295,
	     380057,     383857,     387695,     391571,
	     395485,     399439,     403433,     407467,
	     411541,     415655,     419811,     424009,
	// [832 .. 847]
	     428249,     432531,     436855,     441223,
	     445635,     450091,     454591,     459135,
	     463725,     468361,     473043,     477773,
	     482549,     487373,     492245,     497167,
	// [848 .. 863]
	     502137,     507157,     512227,     517349,
	     522521,     527745,     533021,     538351,
	     543733,     549169,     554659,     560205,
	     565807,     571465,     577179,     582949,
	// [864 .. 879]
	     588777,     594663,     600609,     606615,
	     612681,     618807,     624995,     631243,
	     637555,     643929,     650367,     656869,
	     663437,     670071,     676771,     683537,
	// [880 .. 895]
	     690371,     697273,     704245,     711287,
	     718399,     725581,     732835,     740163,
	     747563,     755037,     762587,     770211,
	     777913,     785691,     793547,     801481,
	// [896 .. 911]
	     809495,     817589,     825763,     834019,
	     842359,     850781,     859287,     867879,
	     876557,     885321,     894173,     903113,
	     912143,     921263,     930475,     939779,
	// [912 .. 927]
	     949175,     958665,     968251,     977933,
	     987711,     997587,    1007561,    1017635,
	    1027811,    1038089,    1048469,    1058953,
	    1069541,    1080235,    1091037,    1101947,
	// [928 .. 943]
	    1112965,    1124093,    1135333,    1146685,
	    1158151,    1169731,    1181427,    1193241,
	    1205173,    1217223,    1229395,    1241687,
	    1254103,    1266643,    1279309,    1292101,
	// [944 .. 959]
	    1305021,    1318071,    1331251,    1344563,
	    1358007,    1371587,    1385301,    1399153,
	    1413143,    1427273,    1441545,    1455959,
	    1470517,    1485221,    1500073,    1515073,
	// [960 .. 975]
	    1530223,    1545525,    1560979,    1576587,
	    1592351,    1608273,    1624355,    1640597,
	    1657001,    1673571,    1690305,    1707207,
	    1724279,    1741521,    1758935,    1776523,
	// [976 .. 991]
	    1794287,    1812229,    1830351,    1848653,
	    1867139,    1885809,    1904667,    1923713,
	    1942949,    1962377,    1981999,    2001817,
	    2021835,    2042053,    2062473,    2083097,
	// [992 .. 1007]
	    2103927,    2124965,    2146213,    2167675,
	    2189351,    2211243,    2233355,    2255687,
	    2278243,    2301025,    2324035,    2347275,
	    2370747,    2394453,    2418397,    2442579,
	// [1008 .. 1023]
	    2467003,    2491673,    2516589,    2541753,
	    2567169,    2592839,    2618767,    2644953,
	    2671401,    2698115,    2725095,    2752345,
	    2779867,    2807665,    2835741,    2864097,
	// [1024 .. 1039]
	    2892737,    2921663,    2950879,    2980387,
	    3010189,    3040289,    3070691,    3101397,
	    3132409,    3163733,    3195369,    3227321,
	    3259593,    3292187,    3325107,    3358357,
	// [1040 .. 1055]
	    3391939,    3425857,    3460115,    3494715,
	    3529661,    3564957,    3600605,    3636611,
	    3672977,    3709705,    3746801,    3784269,
	    3822111,    3860331,    3898933,    3937921,
	// [1056 .. 1071]
	    3977299,    4017071,    4057241,    4097813,
	    4138791,    4180177,    4221977,    4264195,
	    4306835,    4349903,    4393401,    4437335,
	    4481707,    4526523,    4571787,    4617503,
	// [1072 .. 1087]
	    4663677,    4710313,    4757415,    4804989,
	    4853037,    4901567,    4950581,    5000085,
	    5050085,    5100585,    5151589,    5203103,
	    5255133,    5307683,    5360759,    5414365,
	// [1088 .. 1103]
	    5468507,    5523191,    5578421,    5634205,
	    5690547,    5747451,    5804925,    5862973,
	    5921601,    5980817,    6040625,    6101031,
	    6162041,    6223661,    6285897,    6348755,
	// [1104 .. 1119]
	    6412241,    6476363,    6541125,    6606535,
	    6672599,    6739323,    6806715,    6874781,
	    6943527,    7012961,    7083089,    7153919,
	    7225457,    7297711,    7370687,    7444393,
	// [1120 .. 1135]
	    7518835,    7594023,    7669963,    7746661,
	    7824127,    7902367,    7981389,    8061201,
	    8141813,    8223231,    8305463,    8388517,
	    8472401,    8557125,    8642695,    8729121,
	// [1136 .. 1151]
	    8816411,    8904575,    8993619,    9083555,
	    9174389,    9266131,    9358791,    9452377,
	    9546899,    9642367,    9738789,    9836175,
	    9934535,   10033879,   10134217,   10235559,
	// [1152 .. 1167]
	   10337913,   10441291,   10545703,   10651159,
	   10757669,   10865245,   10973897,   11083635,
	   11194471,   11306415,   11419479,   11533673,
	   11649009,   11765499,   11883153,   12001983,
	// [1168 .. 1183]
	   12122001,   12243221,   12365653,   12489309,
	   12614201,   12740343,   12867745,   12996421,
	   13126385,   13257647,   13390223,   13524125,
	   13659365,   13795957,   13933915,   14073253,
	// [1184 .. 1199]
	   14213985,   14356123,   14499683,   14644679,
	   14791125,   14939035,   15088425,   15239309,
	   15391701,   15545617,   15701073,   15858083,
	   16016663,   16176829,   16338597,   16501981,
	// [1200 .. 1215]
	   16666999,   16833667,   17002003,   17172023,
	   17343743,   17517179,   17692349,   17869271,
	   18047963,   18228441,   18410725,   18594831,
	   18780779,   18968585,   19158269,   19349851,
	// [1216 .. 1231]
	   19543349,   19738781,   19936167,   20135527,
	   20336881,   20540249,   20745651,   20953107,
	   21162637,   21374263,   21588005,   21803885,
	   22021923,   22242141,   22464561,   22689205,
	// [1232 .. 1247]
	   22916097,   23145257,   23376709,   23610475,
	   23846579,   24085043,   24325893,   24569151,
	   24814841,   25062989,   25313617,   25566753,
	   25822419,   26080643,   26341449,   26604863,
	// [1248 .. 1263]
	   26870911,   27139619,   27411015,   27685125,
	   27961975,   28241593,   28524007,   28809247,
	   29097339,   29388311,   29682193,   29979013,
	   30278803,   30581591,   30887405,   31196279,
	// [1264 .. 1279]
	   31508241,   31823323,   32141555,   32462969,
	   32787597,   33115471,   33446625,   33781091,
	   34118901,   34460089,   34804689,   35152735,
	   35504261,   35859303,   36217895,   36580073,
	// [1280 .. 1295]
	   36945873,   37315331,   37688483,   38065367,
	   38446019,   38830479,   39218783,   39610969,
	   40007077,   40407147,   40811217,   41219329,
	   41631521,   42047835,   42468313,   42892995,
	// [1296 .. 1311]
	   43321923,   43755141,   44192691,   44634617,
	   45080963,   45531771,   45987087,   46446957,
	   46911425,   47380539,   47854343,   48332885,
	   48816213,   49304375,   49797417,   50295391,
	// [1312 .. 1327]
	   50798343,   51306325,   51819387,   52337579,
	   52860953,   53389561,   53923455,   54462689,
	   55007315,   55557387,   56112959,   56674087,
	   57240827,   57813235,   58391367,   58975279,
	// [1328 .. 1343]
	   59565031,   60160681,   60762287,   61369909,
	   61983607,   62603443,   63229477,   63861771,
	   64500387,   65145389,   65796841,   66454809,
	   67119357,   67790549,   68468453,   69153137,
	// [1344 .. 1359]
	   69844667,   70543113,   71248543,   71961027,
	   72680637,   73407443,   74141517,   74882931,
	   75631759,   76388075,   77151955,   77923473,
	   78702707,   79489733,   80284629,   81087475,
	// [1360 .. 1375]
	   81898349,   82717331,   83544503,   84379947,
	   85223745,   86075981,   86936739,   87806105,
	   88684165,   89571005,   90466715,   91371381,
	   92285093,   93207943,   94140021,   95081421,
	// [1376 .. 1391]
	   96032235,   96992557,   97962481,   98942105,
	   99931525,  100930839,  101940147,  102959547,
	  103989141,  105029031,  106079321,  107140113,
	  108211513,  109293627,  110386563,  111490427,
	// [1392 .. 1407]
	  112605331,  113731383,  114868695,  116017381,
	  117177553,  118349327,  119532819,  120728147,
	  121935427,  123154781,  124386327,  125630189,
	  126886489,  128155353,  129436905,  130731273,
	// [1408 .. 1423]
	  132038585,  133358969,  134692557,  136039481,
	  137399875,  138773873,  140161611,  141563227,
	  142978859,  144408647,  145852733,  147311259,
	  148784371,  150272213,  151774935,  153292683,
	// [1424 .. 1439]
	  154825609,  156373865,  157937603,  159516979,
	  161112147,  162723267,  164350499,  165994003,
	  167653943,  169330481,  171023785,  172734021,
	  174461361,  176205973,  177968031,  179747711,
	// [1440 .. 1455]
	  181545187,  183360637,  185194243,  187046185,
	  188916645,  190805811,  192713869,  194641007,
	  196587417,  198553291,  200538823,  202544211,
	  204569653,  206615349,  208681501,  210768315,
	// [1456 .. 1471]
	  212875997,  215004755,  217154801,  219326349,
	  221519611,  223734807,  225972155,  228231875,
	  230514193,  232819333,  235147525,  237498999,
	  239873987,  242272725,  244695451,  247142405,
	// [1472 .. 1487]
	  249613829,  252109967,  254631065,  257177375,
	  259749147,  262346637,  264970103,  267619803,
	  270296001,  272998961,  275728949,  278486237,
	  281271099,  284083809,  286924647,  289793893,
	// [1488 .. 1503]
	  292691831,  295618749,  298574935,  301560683,
	  304576289,  307622051,  310698271,  313805253,
	  316943305,  320112737,  323313863,  326547001,
	  329812471,  333110595,  336441699,  339806115,
	// [1504 .. 1519]
	  343204175,  346636215,  350102577,  353603601,
	  357139637,  360711033,  364318143,  367961323,
	  371640935,  375357343,  379110915,  382902023,
	  386731043,  390598353,  394504335,  398449377,
	// [1520 .. 1535]
	  402433869,  406458207,  410522789,  414628015,
	  418774295,  422962037,  427191657,  431463573,
	  435778207,  440135989,  444537347,  448982719,
	  453472545,  458007269,  462587341,  467213213,
	// [1536 .. 1551]
	  471885345,  476604197,  481370237,  486183939,
	  491045777,  495956233,  500915795,  505924951,
	  510984199,  516094039,  521254979,  526467527,
	  531732201,  537049523,  542420017,  547844217,
	// [1552 .. 1567]
	  553322659,  558855885,  564444443,  570088887,
	  575789775,  581547671,  587363147,  593236777,
	  599169143,  605160833,  611212441,  617324565,
	  623497809,  629732787,  636030113,  642390413,
	// [1568 .. 1583]
	  648814317,  655302459,  661855483,  668474037,
	  675158777,  681910363,  688729465,  695616759,
	  702572925,  709598653,  716694639,  723861585,
	  731100199,  738411199,  745795309,  753253261,
	// [1584 .. 1599]
	  760785793,  768393649,  776077585,  783838359,
	  791676741,  799593507,  807589441,  815665335,
	  823821987,  832060205,  840380807,  848784615,
	  857272461,  865845185,  874503635,  883248671,
	// [1600 .. 1615]
	  892081157,  901001967,  910011985,  919112103,
	  928303223,  937586255,  946962117,  956431737,
	  965996053,  975656013,  985412573,  995266697,
	 1005219363, 1015271555, 1025424269, 1035678511,
	// [1616 .. 1631]
	 1046035295, 1056495647, 1067060603, 1077731209,
	 1088508521, 1099393605, 1110387541, 1121491415,
	 1132706329, 1144033391, 1155473723, 1167028459,
	 1178698743, 1190485729, 1202390585, 1214414489,
	// [1632 .. 1647]
	 1226558633, 1238824219, 1251212461, 1263724585,
	 1276361829, 1289125447, 1302016701, 1315036867,
	 1328187235, 1341469107, 1354883797, 1368432633,
	 1382116959, 1395938127, 1409897507, 1423996481,
	// [1648 .. 1663]
	 1438236445, 1452618809, 1467144997, 1481816445,
	 1496634609, 1511600955, 1526716963, 1541984131,
	 1557403971, 1572978009, 1588707789, 1604594865,
	 1620640813, 1636847221, 1653215693, 1669747849,
	// [1664 .. 1679]
	 1686445327, 1703309779, 1720342875, 1737546303,
	 1754921765, 1772470981, 1790195689, 1808097645,
	 1826178621, 1844440407, 1862884811, 1881513659,
	 1900328795, 1919332081, 1938525401, 1957910655,
	// [1680 .. 1695]
	 1977489761, 1997264657, 2017237303, 2037409675,
	 2057783771, 2078361607, 2099145223, 2120136675,
	 2141338041, 2162751421, 2184378935, 2206222723,
	 2228284949, 2250567797, 2273073473, 2295804207,
	// [1696 .. 1711]
	 2318762249, 2341949871, 2365369369, 2389023061,
	 2412913291, 2437042423, 2461412847, 2486026975,
	 2510887243, 2535996115, 2561356075, 2586969635,
	 2612839331, 2638967723, 2665357399, 2692010971,
	// [1712 .. 1727]
	 2718931079, 2746120389, 2773581591, 2801317405,
	 2829330579, 2857623883, 2886200121, 2915062121,
	 2944212741, 2973654867, 3003391415, 3033425329,
	 3063759581, 3094397175, 3125341145, 3156594555,
	// [1728 .. 1743]
	 3188160499, 3220042103, 3252242523, 3284764947,
	 3317612595, 3350788719, 3384296605, 3418139571,
	 3452320965, 3486844173, 3521712613, 3556929739,
	 3592499035, 3628424025, 3664708265, 3701355347,
	// [1744 .. 1759]
	 3738368899, 3775752587, 3813510111, 3851645211,
	 3890161663, 3929063279, 3968353911, 4008037449,
	 4048117823, 4088599001, 4129484991, 4170779839,
	 4212487637, 4254612513, 4294967295
    };

} }

bool min::use_obj_aux_stubs = false;

static int short_obj_max_hi =
    ( 1 << MINT::SHORT_OBJ_HASH_SIZE_CODE_BITS ) - 1;
static unsigned short_obj_max_hash_size =
    MINT::hash_size[short_obj_max_hi];
static int long_obj_max_hi =
    (   sizeof ( MINT::hash_size )
      / sizeof ( min::uns32 ) )
    - 1;
static unsigned long_obj_max_hash_size =
    MINT::hash_size[long_obj_max_hi];

unsigned min::short_obj_hash_size ( unsigned u )
{
    int lo = 0, hi = short_obj_max_hi;
    if ( u <= 1 ) hi = u;
    else if ( u < short_obj_max_hash_size )
	while ( true )
	{
	    // Invariant:
	    //
	    //    MINT::hash_size[hi] >= u
	    //
	    int mid = ( lo + hi ) / 2;
	    if ( MINT::hash_size[mid] >= u ) hi = mid;
	    else if ( lo == mid ) break;
	    else lo = mid;
	}
    return MINT::hash_size[hi];
}

unsigned min::short_obj_total_size ( unsigned u )
{
    if ( u < ( 1 << 16 ) ) return u;
    else return ( 1 << 16 ) - 1;
}

unsigned min::long_obj_hash_size ( unsigned u )
{
    int lo = 0, hi = long_obj_max_hi;
    if ( u <= 1 ) hi = u;
    else if ( u < long_obj_max_hash_size )
	while ( true )
	{
	    // Invariant:
	    //
	    //    MINT::hash_size[hi] >= u
	    //
	    int mid = ( lo + hi ) / 2;
	    if ( MINT::hash_size[mid] >= u ) hi = mid;
	    else if ( lo == mid ) break;
	    else lo = mid;
	}
    return MINT::hash_size[hi];
}

unsigned min::long_obj_total_size ( unsigned u )
{
    if ( sizeof (min::uns32) < sizeof (unsigned) )
    {
        if ( u < min::uns32(-1) ) return u;
	else return min::uns32(-1);
    }
    else
        return u;
}

min::gen min::new_obj_gen
	    ( unsigned unused_size,
	      unsigned hash_size,
	      unsigned var_size )
{
    int lo = 0, hi = long_obj_max_hi;
    if ( hash_size <= 1 ) hi = hash_size;
    else if ( hash_size < long_obj_max_hash_size )
	while ( true )
	{
	    // Invariant:
	    //
	    //    MINT::hash_size[hi] >= u
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

    MIN_ASSERT (    unused_size
                 <=   min::uns32(-1)
		    - hash_size
		    - var_size
		    - MINT::long_obj_header_size );
    unsigned total_size =
        unused_size + hash_size + var_size;
    min::stub * s = MUP::new_acc_stub();
    min::gen * p;
    int type;
    if (   total_size + MINT::short_obj_header_size
         < ( 1 << 16 ) )
    {
        total_size += MINT::short_obj_header_size;

	type = min::SHORT_OBJ;
	MUP::new_body
	    ( s, sizeof (min::gen) * total_size );
	MINT::short_obj * so = MINT::short_obj_of ( s );
	so->flags = hi << MINT::SHORT_OBJ_FLAG_BITS;
	so->hash_offset = MINT::short_obj_header_size
	                + var_size;
	so->unused_offset =
	       so->hash_offset
	     + hash_size;
	so->aux_offset	= total_size;
	so->total_size  = total_size;
	p = (min::gen *) so
	  + MINT::short_obj_header_size;
    }
    else
    {
        total_size += MINT::long_obj_header_size;

	type = min::LONG_OBJ;
	MUP::new_body
	    ( s, sizeof (min::gen) * total_size );
	MINT::long_obj * lo = MINT::long_obj_of ( s );
	lo->flags = hi << MINT::LONG_OBJ_FLAG_BITS;
	lo->hash_offset = MINT::long_obj_header_size
	                + var_size;
	lo->unused_offset =
	       lo->hash_offset
	     + hash_size;
	lo->aux_offset	= total_size;
	lo->total_size  = total_size;
	p = (min::gen *) lo
	  + MINT::long_obj_header_size;
    }
    min::gen * endp = p + var_size;
    while ( p < endp ) * p ++ = min::UNDEFINED;
    endp += hash_size;
    while ( p < endp ) * p ++ = min::LIST_END;
    MUP::set_type_of ( s, type );
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
void MINT::allocate_stub_list
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
    MIN_ASSERT ( ! min::relocated_flag () );
}

void MINT::collect_aux_stub_helper ( min::stub * s )
{
    while ( true )
    {
	MINT::collect_aux_stub ( MUP::gen_of ( s ) );
	min::uns64 c = MUP::control_of ( s );

	MUP::free_aux_stub ( s );

	if ( ( c & MUP::STUB_POINTER ) == 0 ) break;
	s =  MUP::stub_of_control ( c );
	int type = min::type_of ( s );
	if ( type != min::LIST_AUX
	     &&
	     type != min::SUBLIST_AUX )
	    break;
    }
}

# endif // MIN_USES_OBJ_AUX_STUBS

void min::insert_before
	( min::insertable_list_pointer & lp,
	  const min::gen * p, unsigned n )
{
    if ( n == 0 ) return;

    unsigned unused_offset =
        unprotected::unused_offset_of ( lp.vecp );
    unsigned aux_offset =
        unprotected::aux_offset_of ( lp.vecp );

    MIN_ASSERT ( lp.reserved_insertions >= 1 );
    MIN_ASSERT ( lp.reserved_elements >= n );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    MUP::acc_write_update
            ( (min::stub *) stub_of ( lp.vecp ), p, n );

    if ( lp.current == min::LIST_END )
    {
	// Contiguous means the previous pointer does
	// not exists and current_index == aux_
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

#	if MIN_USES_OBJ_AUX_STUBS
	    else if ( lp.previous_stub == NULL )
		contiguous =
		    ( lp.current_index == aux_offset );

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
		MIN_ASSERT (      unused_offset
			        + n + ( ! contiguous )
		                + previous_is_list_head
			     <= aux_offset );

#	if MIN_USES_OBJ_AUX_STUBS
	    if ( lp.previous_stub != NULL )
	    {
	        if ( lp.previous_is_sublist_head )
		{
		    fgen = min::new_list_aux_gen
			       ( aux_offset - 1 );
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
			        aux_offset - 1 ) );
		}
		lp.previous_stub = NULL;
	    }
	    else
#	endif
	if ( lp.previous_index != 0 )
	{
	    if ( previous_is_list_head )
	    {
	        lp.base[-- aux_offset] =
		    lp.base[lp.previous_index];
		fgen = min::new_list_aux_gen
		    ( aux_offset );
	    }
	    else
		fgen = min::new_sublist_aux_gen
		    ( aux_offset - 1 );
	    lp.base[lp.previous_index] = fgen;
	}
	else if ( contiguous )
	    ++ aux_offset;
	else
	{
	    fgen = min::new_list_aux_gen
		       ( aux_offset - 1 );
	    lp.base[lp.current_index] = fgen;
	}

	while ( n -- )
	    lp.base[-- aux_offset] = * p ++;
	    
	lp.base[-- aux_offset] = min::LIST_END;
	lp.current_index = aux_offset;
	lp.previous_index = 0;
	lp.previous_is_sublist_head = false;
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

#   if MIN_USES_OBJ_AUX_STUBS
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
	    //
	    min::uns64 end;
	    min::stub * s;
	    int type = min::LIST_AUX;
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
		unsigned next = lp.current_index;
		if ( next < unused_offset )
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
		      MUP::new_control_with_type
		        ( min::LIST_AUX, next ) );
	    }
	    else
	    {
	        if ( lp.previous_is_sublist_head )
		    type == min::SUBLIST_AUX;
		end = MUP::new_control_with_type
		   ( 0, lp.current_index );
	    }

	    min::stub * first, * last;
	    MINT::allocate_stub_list
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
		lp.current_index = 0;
		lp.current_stub = s;
	    }
	    lp.previous_stub = last;
	    lp.previous_is_sublist_head = false;
	    return;
	}
	else
#   endif
	    MIN_ASSERT (      unused_offset
			    + n + 1 + ( ! previous )
			 <= aux_offset );

    unsigned first = aux_offset - 1;

    while ( n -- )
	lp.base[-- aux_offset] = * p ++;

#   if MIN_USES_OBJ_AUX_STUBS
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
	unsigned next = lp.current_index;
        if ( ! previous )
	{
	    lp.base[-- aux_offset] = lp.current;
	    if ( next < unused_offset )
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
        lp.base[-- aux_offset] =
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
		      MUP::new_control_with_type
			  ( type, first ) );
	    }
	    lp.previous_index = aux_offset;
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
	lp.previous_index = aux_offset;
	lp.previous_is_sublist_head = false;
    }
    else
    {
	MIN_ASSERT ( lp.current_index != 0 );
	lp.base[lp.current_index] =
	    min::new_list_aux_gen ( first );
	lp.current_index = aux_offset + 1;
    }

    unprotected::aux_offset_of ( lp.vecp ) = aux_offset;
}

void min::insert_after
	( min::insertable_list_pointer & lp,
	  const min::gen * p, unsigned n )
{
    if ( n == 0 ) return;

    unsigned unused_offset =
        unprotected::unused_offset_of ( lp.vecp );
    unsigned aux_offset =
        unprotected::aux_offset_of ( lp.vecp );

    MIN_ASSERT ( lp.reserved_insertions >= 1 );
    MIN_ASSERT ( lp.reserved_elements >= n );
    MIN_ASSERT ( lp.current != min::LIST_END );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    MUP::acc_write_update
	    ( (min::stub *) stub_of ( lp.vecp ), p, n );

    bool previous = ( lp.previous_index != 0 );
#   if MIN_USES_OBJ_AUX_STUBS
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
	    unsigned next =
	        lp.current_index - ! previous;
	    if ( lp.current_index < unused_offset )
	        next = 0;
	    min::uns64 end =
		MUP::new_control_with_type
		    ( type, next );

	    if ( n > previous )
		MINT::allocate_stub_list
		    ( first, last, min::LIST_AUX,
		      p, n - previous, end );

	    if ( previous )
	    {
		// Given previous, we can copy the last
		// new element to the old current
		// element.
		//
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
			unsigned type =
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
		lp.current_index = 0;
		lp.current_stub = s;
	    }
	    return;
	}
#   endif

    // Insertion will use aux area.

    unsigned first = aux_offset - 1;

    if ( lp.current_index != 0 )
	lp.base[-- aux_offset] = lp.current;

    // Copy all the new elements BUT the last new
    // element.
    //
    while ( -- n )
	lp.base[-- aux_offset] = * p ++;

#   if MIN_USES_OBJ_AUX_STUBS
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
		   first ) );
    }
    else
#   endif
    if ( previous )
    {
	// Given previous, we can copy the last new
	// element to the old current element.
	//

	lp.base[-- aux_offset] =
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
			  MUP::new_control_with_type
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

	lp.base[-- aux_offset] = * p ++;
	unsigned next = lp.current_index;
	if ( next < unused_offset )
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
	lp.base[-- aux_offset] =
	    min::new_list_aux_gen ( next );

	lp.base[lp.current_index] =
	    min::new_list_aux_gen ( first );
	lp.current_index = first;
    }

    unprotected::aux_offset_of ( lp.vecp ) = aux_offset;
}

unsigned min::remove
	( min::insertable_list_pointer & lp,
	  unsigned n )
{
    // Note: current code does NOT set orphaned elements
    // to NONE.  Note some of these may be pointers
    // to orphaned sublist aux stubs.

    if ( n == 0 || lp.current == min::LIST_END )
        return 0;
    else if ( lp.current_index != 0
              &&
	        lp.current_index
	      < unprotected::unused_offset_of
	            ( lp.vecp ) )
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
    unsigned previous_index = lp.previous_index;
    bool previous_is_sublist_head =
	lp.previous_is_sublist_head;
    unsigned current_index = lp.current_index;
#   if MIN_USES_OBJ_AUX_STUBS
	min::stub * previous_stub = lp.previous_stub;
#   endif

    // Count of elements removed; to be returned as
    // result.
    //
    unsigned count = 0;

    // Skip n elements (or until end of list).
    //
    while ( n -- )
    {
	if ( lp.current == min::LIST_END ) break;
	++ count;

#       if MIN_USES_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
		min::stub * last_stub = lp.current_stub;
		next ( lp );
		MINT::collect_aux_stub
		    ( MUP::gen_of ( last_stub ) );
		MUP::free_aux_stub ( last_stub );
	    }
	    else
#       endif
	{
	    MIN_ASSERT ( lp.current_index != 0 );
	    lp.current =
		lp.base[-- lp.current_index];
	    if ( lp.current == min::LIST_END )
	        break;
	    else if ( min::is_list_aux ( lp.current ) )
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
			      ( lp.current_index ) );
		else
		{
		    int type =
		        min::type_of ( previous_stub );
		    MUP::set_control_of
		        ( previous_stub,
			  MUP::new_control_with_type
			      ( type,
			        lp.current_index ) );
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
#	if MIN_USES_OBJ_AUX_STUBS
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
			( lp.current_index );
	    else
		lp.base[previous_index] =
		    min::new_list_aux_gen
			( lp.current_index );

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

#       if MIN_USES_OBJ_AUX_STUBS
	    lp.previous_stub = NULL;
	    MINT::collect_aux_stub
		( lp.base[current_index] );

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
	        (    current_index
		  >= unprotected::unused_offset_of
		         ( lp.vecp ) );
	    lp.base[current_index] =
		min::new_list_aux_gen
		    ( lp.current_index );
	    lp.previous_index = current_index;
	}

	lp.previous_is_sublist_head = false;
    }

    return count;
}

void MINT::insert_reserve
	( min::insertable_list_pointer & lp,
	  unsigned insertions,
	  unsigned elements,
	  bool use_obj_aux_stubs )
{
#   if MIN_USES_OBJ_AUX_STUBS
	if ( use_obj_aux_stubs )
	    MINT::acc_expand_stub_free_list
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

// Object Attribute Level
// ------ --------- -----

// Note: refresh ( ap.dlp) or refresh ( ap.locate_dlp )
// is never used instead of current() in MINT::
// functions as refresh() must be called by the min::
// functions before they call the MINT:: functions.

# if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS

    // Handle all cases except where the name is an
    // integer number atom in the range of an attribute
    // vector subscript.
    //
    template < class list_pointer_type >
    void MINT::locate
	    ( MINT::attribute_pointer_type
	          < list_pointer_type > & ap,
	      min::gen name,
	      bool allow_partial_labels )
    {
	typedef MINT::attribute_pointer_type
		    < list_pointer_type > ap_type;

	ap.attribute_name = name;
	ap.reverse_attribute_name = min::NONE;

	// Set len to the number of elements in the
	// label and element[] to the vector of
	// label elements.
	//
	bool is_label = is_lab ( name );
	unsigned len;
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
	if ( is_label ) lab_of ( element, len, name );
	else element[0] = name;

	// Process element[0] and if found set
	// ap.length = 1.
	//
	// If element[0] is an integer in the right
	// range, locate attribute vector entry.
	// Otherwise locate hash table entry.
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
	     i < attr_size_of ( ap.dlp ) )
	{
	    start_vector ( ap.dlp, i );
	    ap.flags = ap_type::IN_VECTOR;
	    ap.index = i;
	    ap.length = 1;
	    if ( len == 1 )
	    {
		start_copy ( ap.locate_dlp, ap.dlp );
	        ap.state = ap_type::LOCATE_NONE;
		return;
	    }
	}
	else
	{
	    ap.index = min::hash ( element[0] )
	             % hash_size_of ( ap.dlp );
	    ap.flags = 0;

	    start_hash ( ap.dlp, ap.index );

	    for ( c = current ( ap.dlp );
		  ! is_list_end ( c );
		  next ( ap.dlp), c = next ( ap.dlp ) )
	    {
		if ( c == element[0] )
		{
		    c = next ( ap.dlp );
		    MIN_ASSERT ( ! is_list_end ( c ) );
		    break;
		}
	    }

	    if ( is_list_end ( c ) )
	    {
		ap.length = 0;
		ap.state = ap_type::LOCATE_FAIL;
		return;
	    }
	    else ap.length = 1;
	}

	start_copy ( ap.locate_dlp, ap.dlp );
	while ( ap.length < len )
	{
	    if ( ! is_sublist ( current ( ap.dlp ) ) )
	        break;
	    start_sublist ( ap.dlp );
	    if ( ! is_sublist ( current ( ap.dlp ) ) )
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

	    ++ ap.length;
	    start_copy ( ap.locate_dlp, ap.dlp );
	}

	if ( ap.length == len )
	    ap.state = ap_type::LOCATE_NONE;
	else if ( allow_partial_labels )
	{
	    start_copy ( ap.dlp, ap.locate_dlp );
	    ap.state = ap_type::LOCATE_NONE;
	}
	else
	    ap.state = ap_type::LOCATE_FAIL;

	return;
    }

    // Continue relocation after ap.locate_dlp is
    // positioned at beginning of hash-list or
    // vector-list.  State must be >= LOCATE_NONE.
    // 
    template < class list_pointer_type >
    inline void MINT::relocate
	    ( MINT::attribute_pointer_type
	          < list_pointer_type > & ap )
    {
	typedef MINT::attribute_pointer_type
		    < list_pointer_type > ap_type;

	MIN_ASSERT ( ap.length > 0 );

	bool is_label = is_lab ( ap.attribute_name );
	unsigned len;
	if ( is_label )
	    len = min::lablen ( ap.attribute_name );
	else len = 1;
	min::gen element[len];
	if ( is_label )
	    lab_of ( element, len, ap.attribute_name );
	else element[0] = ap.attribute_name;

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
	unsigned length = 1;
	start_copy ( ap.dlp, ap.locate_dlp );
	while ( length < ap.length )
	{
	    if ( ! is_sublist ( current ( ap.dlp ) ) )
	        break;
	    start_sublist ( ap.dlp );
	    if ( ! is_sublist ( current ( ap.dlp ) ) )
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

	MIN_REQUIRE ( length == ap.length );
    }

# else // ! MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS

    template < class list_pointer_type >
    void MINT::locate
	    ( MINT::attribute_pointer_type
		  < list_pointer_type > & ap,
	      min::gen name )
    {
	typedef MINT::attribute_pointer_type
		    < list_pointer_type > ap_type;

	ap.reverse_attribute_name = min::NONE;

	// If name is label whose only element is an
	// atom, set name = the atom.
	//
	if ( is_lab ( name ) )
	{
	    if ( lablen ( name ) == 1 )
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

	ap.attribute_name = name;

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
		 i < attr_size_of ( ap.dlp.vecp ) )
	    {
		start_vector ( ap.dlp, i );
		start_copy ( ap.locate_dlp, ap.dlp );
		ap.index = i;
		ap.flags = ap_type::IN_VECTOR;
		ap.state = ap_type::LOCATE_NONE;
		return;
	    }
	}

	ap.index = min::hash ( name )
		 % hash_size_of
			  ( vec_pointer_of ( ap.dlp ) );
	start_hash ( ap.dlp, ap.index );

	for ( min::gen c = current ( ap.dlp );
	      ! is_list_end ( c );
	      next ( ap.dlp), c = next ( ap.dlp ) )
	{
	    if ( c == name )
	    {
		c = next ( ap.dlp );
		MIN_ASSERT ( ! is_list_end ( c ) );
		start_copy ( ap.locate_dlp, ap.dlp );
		ap.flags = 0;
		ap.state = ap_type::LOCATE_NONE;
		return;
	    }
	}

        // Name not found.
	//
	ap.flags = 0;
	ap.state = ap_type::LOCATE_FAIL;
	return;

    }

    // Continue relocation after ap.relocate_dlp is
    // positioned at beginning of hash-list or
    // vector-list.  State must be >= LOCATE_NONE.
    // Is NOT called if IN_VECTOR flag is set.
    // 
    template < class list_pointer_type >
    inline void MINT::relocate
	    ( MINT::attribute_pointer_type
		  < list_pointer_type > & ap )
    {
	typedef MINT::attribute_pointer_type
		    < list_pointer_type > ap_type;

	for ( min::gen c = current ( ap.locate_dlp );
	      ! is_list_end ( c );
	      next ( ap.locate_dlp),
	      c = next ( ap.locate_dlp ) )
	{
	    if ( c == ap.attribute_name )
	    {
		c = next ( ap.locate_dlp );
		MIN_ASSERT ( ! is_list_end ( c ) );
		return;
	    }
	}

	MIN_ABORT ( "relocate could not find"
	            " attribute" );
    }

# endif

template < class list_pointer_type >
void min::locate_reverse
	( MINT::attribute_pointer_type
	      < list_pointer_type > & ap,
	  min::gen reverse_name )
{
    typedef MINT::attribute_pointer_type
		< list_pointer_type > ap_type;

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

    if ( reverse_name == ap.reverse_attribute_name )
        return;

    ap.reverse_attribute_name = reverse_name;

    switch ( ap.state )
    {
    case ap_type::INIT:
	    MIN_ABORT
	        ( "bad attribute reverse_locate call" );
    case ap_type::LOCATE_FAIL:
    	    return;
    case ap_type::LOCATE_NONE:
	    if ( reverse_name == min::ANY )
	    {
	        ap.state = ap_type::LOCATE_ANY;
		return;
	    }
	    break;
    case ap_type::LOCATE_ANY:
	    if ( reverse_name == min::NONE )
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

    if ( ! is_sublist ( current ( ap.dlp ) )
	 ||
	 ! ( start_sublist ( ap.dlp ),
	     is_sublist ( current ( ap.dlp ) ) )
#	   if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	 ||
	 ! is_sublist ( next ( ap.dlp ) )
#	   endif
       )
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

template < class list_pointer_type >
void min::relocate
	( MINT::attribute_pointer_type
	      < list_pointer_type > & ap )
{
    typedef MINT::attribute_pointer_type
		< list_pointer_type > ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
        return;
    case ap_type::LOCATE_FAIL:
#	if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	    ap.length = 0;
#	endif
	return;
    }

    if ( ap.flags & ap_type::IN_VECTOR )
    {
        start_vector ( ap.locate_dlp, ap.index );
#	if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
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
    case ap_type::LOCATE_NONE:
    case ap_type::LOCATE_ANY:
    case ap_type::REVERSE_LOCATE_FAIL:
	return;
    }

    // state == REVERSE_LOCATE_SUCCEED

    if ( ! is_sublist ( current ( ap.dlp ) )
	 ||
	 ! ( start_sublist ( ap.dlp ),
	     is_sublist ( current ( ap.dlp ) ) )
#	   if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	 ||
	 ! is_sublist ( next ( ap.dlp ) )
#	   endif
       )
	MIN_ABORT ( "relocate could not find"
	            " reverse attribute" );

    start_sublist ( ap.dlp );

    for ( min::gen c = current ( ap.dlp );
	  ! is_list_end ( c );
	  next ( ap.dlp), c = next ( ap.dlp ) )
    {
	if ( c == ap.reverse_attribute_name )
	{
	    c = next ( ap.dlp );
	    MIN_REQUIRE ( ! is_list_end ( c ) );
	    return;
	}
    }
    MIN_ABORT ( "relocate could not find reverse"
                " attribute" );
}

template < class list_pointer_type >
inline unsigned MINT::count
	( MINT::attribute_pointer_type
	      < list_pointer_type > & ap )
{
    typedef MINT::attribute_pointer_type
		< list_pointer_type > ap_type;

    min:gen c;
    list_pointer lp ( min::vec_pointer_of ( ap.dlp ) );

    switch ( ap.state )
    {
    case ap_type::INIT:
    case ap_type::LOCATE_FAIL:
    case ap_type::REVERSE_LOCATE_FAIL:
	    MIN_ABORT ( "min::count failed" );
    case ap_type::LOCATE_NONE:
	    // We handled case of a non-sublist single
	    // value in the min::count function that
	    // called this function.
	    //
	    start_sublist ( lp, ap.dlp );
	    while ( is_sublist ( current ( lp ) ) )
		next ( lp );
	    while ( is_control_code ( current ( lp ) ) )
		next ( lp );
    	    break;
    case ap_type::REVERSE_LOCATE_SUCCEED:
	    // We handled case of a non-sublist single
	    // value in the min::count function that
	    // called this function.
	    //
	    start_sublist ( lp, ap.dlp );
    	    break;
    case ap_type::LOCATE_ANY:
        {
	    start_copy ( lp, ap.dlp );
	    if ( ! is_sublist ( current ( lp ) )
		 ||
		 ! ( start_sublist ( lp ),
		     is_sublist ( current ( lp ) ) )
#	    if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
		 ||
		 ! is_sublist ( next ( lp ) )
#	    endif
	       )
		return 0;

	    unsigned result = 0;
	    list_pointer lp2 ( vec_pointer_of ( lp ) );

	    start_sublist ( lp );
	    for ( c = next ( lp );
		  ! is_list_end ( c );
		  next ( ap.lp), c = next ( ap.lp ) )
	    {
		if ( is_sublist ( c  ) )
		{
		    start_sublist ( lp2, lp );
		    for ( c = current ( lp2 );
		          ! is_list_end ( c );
			  c = next ( lp2 ) )
			++ result;
		}
		else ++ result;
	    }
	    return result;
	}
    }

    unsigned result = 0;
    for ( c = current ( lp );
          ! is_list_end ( c );
	  c = next ( lp ) )
	++ result;

    return result;
}

template < class list_pointer_type >
inline unsigned MINT::get
	( min::gen * out, unsigned n,
	  MINT::attribute_pointer_type
	      < list_pointer_type > & ap )
{
    typedef MINT::attribute_pointer_type
		< list_pointer_type > ap_type;

    min:gen c;
    list_pointer lp ( vec_pointer_of ( ap.dlp ) );

    switch ( ap.state )
    {
    case ap_type::INIT:
    case ap_type::LOCATE_FAIL:
    case ap_type::REVERSE_LOCATE_FAIL:
	    MIN_ABORT ( "min::get failed" );
    case ap_type::LOCATE_NONE:
	    // We handled case of a non-sublist single
	    // value in the min::count function that
	    // called this function.
	    //
	    start_sublist ( lp, ap.dlp );
	    while ( is_sublist ( current ( lp ) ) )
		next ( lp );
	    while ( is_control_code ( current ( lp ) ) )
		next ( lp );
    	    break;
    case ap_type::REVERSE_LOCATE_SUCCEED:
	    // We handled case of a non-sublist single
	    // value in the min::count function that
	    // called this function.
	    //
	    start_sublist ( lp, ap.dlp );
    	    break;
    case ap_type::LOCATE_ANY:
        {
	    start_copy ( lp, ap.dlp );
	    if ( ! is_sublist ( current ( lp ) )
		 ||
		 ! ( start_sublist ( lp ),
		     is_sublist ( current ( lp ) ) )
#	    if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
		 ||
		 ! is_sublist ( next ( lp ) )
#	    endif
	       )
		return 0;

	    unsigned result = 0;
	    list_pointer lp2 ( vec_pointer_of ( lp ) );

	    start_sublist ( lp );
	    for ( c = next ( lp );
		  ! is_list_end ( c ) && result < n;
		  next ( ap.lp), c = next ( ap.lp ) )
	    {
		if ( is_sublist ( c  ) )
		{
		    start_sublist ( lp2, lp );
		    for ( c = current ( lp2 );
		             ! is_list_end ( c )
			  && result < n;
			  c = next ( lp2 ) )
			* out ++ = c, ++ result;
		}
		else ++ result, * out ++ = c;
	    }
	    return result;
	}
    }

    unsigned result = 0;
    for ( c = current ( lp );
          ! is_list_end ( c ) && result < n;
	  c = next ( lp ) )
	++ result, * out ++ = c;

    return result;
}

void MINT::set
	( MUP::writable_attribute_pointer & wap,
	  const min::gen * in, unsigned n )
{
    typedef MUP::writable_attribute_pointer ap_type;

    MIN_ASSERT
        ( wap.reverse_attribute_name != min::ANY );

    switch ( wap.state )
    {
    case ap_type::INIT:
	    MIN_ABORT ( "bad attribute set call" );
    case ap_type::LOCATE_FAIL:
    	    MINT::attribute_create ( wap );
	    if (    wap.reverse_attribute_name
	         == min::NONE )
	        break;
    case ap_type::REVERSE_LOCATE_FAIL:
	    MINT::reverse_attribute_create ( wap );
    }

    // WARNING: Attribute create may have relocated
    //	        object.

    insertable_list_pointer
        lp ( vec_pointer_of ( wap.dlp ) );
    min:gen c = refresh ( wap.dlp );
    start_copy ( lp, wap.dlp );

    if ( ! is_sublist ( c ) )
    {
        if ( n == 1 ) MIN_ABORT ( "min::set failed" );
    
	min::relocated relocated;
        min::insert_reserve ( lp, 1, n );
	if ( relocated )
	{
	    min::relocate ( wap );
	    MINT::set ( wap, in, n );
	    return;
	}
	update ( lp, min::EMPTY_SUBLIST );
	start_sublist ( lp );
	insert_before ( lp, in, n );
    }
    else if ( is_empty_sublist ( c ) && n == 1 )
    {
	update ( wap.dlp, * in );
	return;
    }
    else
    {
        start_sublist ( lp );
	while ( is_sublist ( current ( lp ) ) )
	    next ( lp );
	while ( is_control_code ( current ( lp ) ) )
	    next ( lp );
	for ( c = current ( lp );
	      ! is_list_end ( c ) && n > 0;
	      c = next ( lp ) )
	    update ( lp, ( -- n, * in ++ ) );
	if ( n > 0 )
	{
	    min::relocated relocated;
	    min::insert_reserve ( lp, 1, n );
	    if ( relocated )
	    {
		min::relocate ( wap );
		add_to_multiset ( wap, in, n );
		return;
	    }
	    insert_before ( lp, in, n );
	}
	else for ( ; ! is_list_end ( c );
		     c = current ( lp ) )
	    remove ( lp, 100 );
    }
}

void min::add_to_multiset
	( MUP::writable_attribute_pointer & wap,
	  const min::gen * in, unsigned n )
{
    typedef MUP::writable_attribute_pointer ap_type;

    MIN_ASSERT
        ( wap.reverse_attribute_name != min::ANY );

    if ( n == 0 ) return;

    switch ( wap.state )
    {
    case ap_type::INIT:
	    MIN_ABORT ( "bad attribute set call" );
    case ap_type::LOCATE_FAIL:
    case ap_type::REVERSE_LOCATE_FAIL:
	    min::set ( wap, in, n );
	    return;
    }

    insertable_list_pointer
        lp ( vec_pointer_of ( wap.dlp ) );
    min:gen c = refresh ( wap.dlp );
    start_copy ( lp, wap.dlp );

    if ( ! is_sublist ( c ) )
    {
	min::relocated relocated;
        min::insert_reserve ( lp, 2, n + 1 );
	if ( relocated )
	{
	    min::relocate ( wap );
	    min::add_to_multiset ( wap, in, n );
	    return;
	}
	update ( lp, min::EMPTY_SUBLIST );
	start_sublist ( lp );
	min::gen element[1] = { c };
	insert_before ( lp, element, 1 );
	insert_before ( lp, in, n );
    }
    else if ( is_empty_sublist ( c ) && n == 1 )
        update ( lp, * in );
    else
    {
        start_sublist ( lp );
	while ( is_sublist ( current ( lp ) ) )
	    next ( lp );
	while ( is_control_code ( current ( lp ) ) )
	    next ( lp );
	while ( ! is_list_end ( current ( lp ) ) )
	    next ( lp );

	min::relocated relocated;
	min::insert_reserve ( lp, 1, n );
	if ( relocated )
	{
	    min::relocate ( wap );
	    add_to_multiset ( wap, in, n );
	    return;
	}
	insert_before ( lp, in, n );
    }
}

void min::add_to_set
	( MUP::writable_attribute_pointer & wap,
	  const min::gen * in, unsigned n )
{
    typedef MUP::writable_attribute_pointer ap_type;

    MIN_ASSERT
        ( wap.reverse_attribute_name != min::ANY );

    if ( n == 0 ) return;

    switch ( wap.state )
    {
    case ap_type::INIT:
	    MIN_ABORT ( "bad attribute set call" );
    case ap_type::LOCATE_FAIL:
    case ap_type::REVERSE_LOCATE_FAIL:
	    min::set ( wap, in, n );
	    return;
    }

    insertable_list_pointer
        lp ( vec_pointer_of ( wap.dlp ) );
    min:gen c = refresh ( wap.dlp );
    start_copy ( lp, wap.dlp );

    if ( ! is_sublist ( c ) )
    {
	if ( n == 1 && * in == c ) return;

	min::relocated relocated;
	bool include_c = true;
	for ( int i = 0; include_c && i < n; ++ i )
	    include_c = ( c != in[i] );

        min::insert_reserve
	    ( lp, 1 + include_c, n + include_c );
	if ( relocated )
	{
	    min::relocate ( wap );
	    min::add_to_set ( wap, in, n );
	    return;
	}
	update ( lp, min::EMPTY_SUBLIST );
	start_sublist ( lp );
	if ( include_c )
	{
	    min::gen element[1] = { c };
	    insert_before ( lp, element, 1 );
	}
	insert_before ( lp, in, n );
    }
    else if ( is_empty_sublist ( c ) && n == 1 )
        update ( lp, * in );
    else
    {
        start_sublist ( lp );
	while ( is_sublist ( current ( lp ) ) )
	    next ( lp );
	while ( is_control_code ( current ( lp ) ) )
	    next ( lp );

	// Copy in vector to kept vector.  Remove from
	// kept vector all elements that are already
	// in the values, decrementing n for each
	// value removed.
	//
        min::gen kept[n];
	memcpy ( kept, in, n * sizeof ( min::gen ) );

	for ( c = current ( lp );
	      ! is_list_end ( c );
	      c = next ( lp ) )
	for ( int i = 0; i < n; )
	{
	    if ( c != kept[i] ) ++ i;
	    else
	    {
		for ( int j = i + 1; j < n; )
		    kept[i++] = kept[j++];
		-- n;
		break;
	    }
	}

	if ( n == 0 ) return;

	min::relocated relocated;
	min::insert_reserve ( lp, 1, n );
	if ( relocated )
	{
	    min::relocate ( wap );
	    add_to_multiset ( wap, kept, n );
	    return;
	}
	insert_before ( lp, kept, n );
    }
}

void MINT::set_flags
	( MUP::writable_attribute_pointer & wap,
	  const min::gen * in, unsigned n )
{
    typedef MUP::writable_attribute_pointer ap_type;

    for ( unsigned i = 0; i < n; ++ i )
        MIN_ASSERT ( is_control_code ( in[i] ) );

    switch ( wap.state )
    {
    case ap_type::INIT:
	    MIN_ABORT
	        ( "bad attribute set flags call" );
    case ap_type::LOCATE_FAIL:
    	    MINT::attribute_create ( wap );
    }

    // WARNING: Attribute create may have relocated
    //	        object.

    insertable_list_pointer
        lp ( vec_pointer_of ( wap.locate_dlp ) );
    min:gen c = refresh ( wap.locate_dlp );
    start_copy ( lp, wap.locate_dlp );

    if ( ! is_sublist ( c ) )
    {
        if ( n == 0 ) return;

	min::relocated relocated;
        min::insert_reserve ( lp, 2, n + 1 );
	if ( relocated )
	{
	    min::relocate ( wap );
	    MINT::set_flags ( wap, in, n );
	    return;
	}
	update ( lp, min::EMPTY_SUBLIST );
	start_sublist ( lp );
	insert_before ( lp, in, n );
	min::gen element[1] = { c };
	insert_before ( lp, element, 1 );
    }
    else
    {
        start_sublist ( lp );
	while ( is_sublist ( current ( lp ) ) )
	    next ( lp );
	for ( c = current ( lp );
	      is_control_code ( c ) && n > 0;
	      c = next ( lp ) )
	    -- n, update ( lp, * in ++ );
	if ( n > 0 )
	{
	    min::relocated relocated;
	    min::insert_reserve ( lp, 1, n );
	    if ( relocated )
	    {
		min::relocate ( wap );
		MINT::set_more_flags ( wap, in, n );
		return;
	    }
	    insert_before ( lp, in, n );
	}
	else for ( c = current ( lp );
	           is_control_code ( c );
		   c = next ( lp ) )
	    update ( lp, new_control_code_gen ( 0 ) );
    }
}

// Appends control codes to the current attribute's
// existing list of control codes.  Attribute must
// already have a descriptor that is a sublist.
//
void MINT::set_more_flags
	( MUP::writable_attribute_pointer & wap,
	  const min::gen * in, unsigned n )
{
    typedef MUP::writable_attribute_pointer ap_type;

    for ( unsigned i = 0; i < n; ++ i )
        MIN_ASSERT ( is_control_code ( in[i] ) );

    switch ( wap.state )
    {
    case ap_type::INIT:
    case ap_type::LOCATE_FAIL:
	    MIN_ABORT
	        ( "bad attribute set more flags call" );
    }

    insertable_list_pointer
        lp ( vec_pointer_of ( wap.locate_dlp ) );
    min:gen c = refresh ( wap.locate_dlp );
    start_copy ( lp, wap.locate_dlp );

    min::relocated relocated;
    min::insert_reserve ( lp, 1, n );
    if ( relocated )
    {
	min::relocate ( wap );
	set_more_flags ( wap, in, n );
	return;
    }

    MIN_ASSERT ( is_sublist ( c ) );
    start_sublist ( lp );
    while ( is_sublist ( current ( lp ) ) )
	next ( lp );
    while ( is_control_code ( current ( lp ) ) )
        next ( lp );
    insert_before ( lp, in, n );
}

#if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS

    void MINT::attribute_create
	    ( MUP::writable_attribute_pointer & wap )
    {
	typedef MUP::writable_attribute_pointer ap_type;

	MIN_ASSERT
	    ( wap.state == ap_type::LOCATE_FAIL );

	bool is_label = is_lab ( wap.attribute_name );
	unsigned len;
	if ( is_label )
	    len = min::lablen ( wap.attribute_name );
	else
	    len = 1;
	min::gen element[len];
	if ( is_label )
	    lab_of ( element, len, wap.attribute_name );
	else element[0] = wap.attribute_name;

	MIN_ASSERT ( wap.length < len );

	insertable_list_pointer lp
	    ( vec_pointer_of ( wap.locate_dlp ) );

	min::relocated relocated;
	while ( true )
	{
	    min::insert_reserve
	        ( lp, 2 * ( len - wap.length ),
		      3 * ( len - wap.length ) + 1 );
	    if ( ! relocated ) break;
	    min::relocate ( wap );
	}

	if ( wap.length == 0 )
	{
	    float64 f;
	    int i;
	    if ( is_num ( element[0] )
	         &&
		 0 <= ( f = float_of ( element[0] ) )
		 &&
		 ( i = (int) f ) == f
		 &&
		 i < attr_size_of
		         ( vec_pointer_of ( lp ) ) )
	    {
		wap.flags = ap_type::IN_VECTOR;
		wap.index = i;
		start_vector ( wap.locate_dlp, i );
	    }
	    else
	    {

		wap.flags = 0;
		wap.index = min::hash ( element[0] )
			  % hash_size_of
			        ( vec_pointer_of
				       ( lp ) );
		start_hash ( lp, wap.index );
		min::gen elements[2] =
		    { element[0], min::EMPTY_SUBLIST };
		insert_before ( lp, elements, 2 );
		min::start_hash
		    ( wap.locate_dlp, wap.index );
		next ( wap.locate_dlp );
	    }

	    wap.length = 1;
	}
	else refresh ( wap.locate_dlp );

	while ( wap.length < len )
	{
	    min::gen c = current ( wap.locate_dlp );
	    start_copy ( lp, wap.locate_dlp );

	    if ( ! is_sublist ( c ) )
	    {
		update ( lp, min::EMPTY_SUBLIST );
		start_sublist ( lp );
		min::gen elements[2] =
		    { min::EMPTY_SUBLIST, c };
		insert_before ( lp, elements, 2 );
		refresh ( wap.locate_dlp );
		start_sublist ( lp, wap.locate_dlp );
	    }
	    else if ( start_sublist ( lp ),
	         ! is_sublist ( current ( lp ) ) )
	    {
		min::gen elements[1] =
		    { min::EMPTY_SUBLIST };
		insert_before ( lp, elements, 1 );
		refresh ( wap.locate_dlp );
		start_sublist ( lp, wap.locate_dlp );
	    }
	    start_copy ( wap.locate_dlp, lp );

	    start_sublist ( lp );
	    min::gen elements[2] =
		{ element[wap.length],
		  min::EMPTY_SUBLIST };
	    insert_before ( lp, elements, 2 );

	    start_sublist ( wap.locate_dlp );
	    next ( wap.locate_dlp );
	    ++ wap.length;
	}

	start_copy ( wap.dlp, wap.locate_dlp );
	if ( wap.reverse_attribute_name == min::NONE )
	    wap.state = ap_type::LOCATE_NONE;
	else if (    wap.reverse_attribute_name
	          == min::ANY )
	    wap.state = ap_type::LOCATE_ANY;
	else
	    wap.state = ap_type::REVERSE_LOCATE_FAIL;
    }

#else // ! MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS

    void MINT::attribute_create
	    ( MUP::writable_attribute_pointer & wap )
    {
	typedef MUP::writable_attribute_pointer ap_type;

	MIN_ASSERT
	    ( wap.state == ap_type::LOCATE_FAIL );
	MIN_ASSERT
	    ( ! ( wap.flags & ap_type::IN_VECTOR ) );

	insertable_list_pointer lp
	    ( vec_pointer_of ( wap.dlp ) );
        min::start_hash ( lp, wap.index );

	min::relocated relocated;
	min::insert_reserve ( lp, 1, 2 );
	if ( relocated )
	{
	    min::relocate ( wap );
	    min::start_hash ( lp, wap.index );
	}

	min::gen elements[2] =
	    { wap.attribute_name, min::EMPTY_SUBLIST };
	insert_before ( lp, elements, 2 );

        start_hash ( wap.dlp, wap.index );
	next ( wap.dlp );
	start_copy ( wap.locate_dlp, wap.dlp );

	if ( wap.reverse_attribute_name == min::NONE )
	    wap.state = ap_type::LOCATE_NONE;
	else if (    wap.reverse_attribute_name
	          == min::ANY )
	    wap.state = ap_type::LOCATE_ANY;
	else
	    wap.state = ap_type::REVERSE_LOCATE_FAIL;
    }

#endif

void MINT::reverse_attribute_create
	( MUP::writable_attribute_pointer & wap )
{
    typedef MUP::writable_attribute_pointer ap_type;

    MIN_ASSERT
	( wap.state == ap_type::REVERSE_LOCATE_FAIL );

    min::relocated relocated;

    insertable_list_pointer lp
	( vec_pointer_of ( wap.locate_dlp ) );
    min::gen c = refresh ( wap.locate_dlp );
    start_copy ( lp, wap.locate_dlp );

    if ( ! is_sublist ( c ) )
    {
#	if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	    min::insert_reserve ( lp, 1, 3 );
	    if ( relocated ) min::relocate ( wap );
	    update ( wap.locate_dlp,
	             min::EMPTY_SUBLIST );
	    start_sublist ( lp, wap.locate_dlp );
	    min::gen elements[3] =
	        { min::EMPTY_SUBLIST,
		  min::EMPTY_SUBLIST,
		  c };
	    insert_before ( lp, elements, 3 );
	    start_sublist ( lp, wap.locate_dlp );
	    next ( lp );
#	else // ! MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	    min::insert_reserve ( lp, 1, 2 );
	    if ( relocated ) min::relocate ( wap );
	    update ( wap.locate_dlp,
	             min::EMPTY_SUBLIST );
	    start_sublist ( lp, wap.locate_dlp );
	    min::gen elements[2] =
	        { min::EMPTY_SUBLIST, c };
	    insert_before ( lp, elements, 2 );
	    start_sublist ( lp, wap.locate_dlp );
#	endif
    }
    else if ( start_sublist ( lp ),
              ! is_sublist ( current ( lp ) ) )
    {
#	if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	    min::insert_reserve ( lp, 1, 2 );
	    if ( relocated )
	    {
	        min::relocate ( wap );
		start_sublist ( lp, wap.locate_dlp );
	    }
	    min::gen elements[2] =
	        { min::EMPTY_SUBLIST,
		  min::EMPTY_SUBLIST };
	    insert_before ( lp, elements, 2 );
	    start_sublist ( lp, wap.locate_dlp );
	    next ( lp );
#	else // ! MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	    min::insert_reserve ( lp, 1, 1 );
	    if ( relocated )
	    {
	        min::relocate ( wap );
		start_sublist ( lp, wap.locate_dlp );
	    }
	    min::gen elements[1] =
	        { min::EMPTY_SUBLIST };
	    insert_before ( lp, elements, 1 );
	    start_sublist ( lp, wap.locate_dlp );
#	endif
    }

#   if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	else if ( ! is_sublist ( next ( lp ) ) )
	{
	    min::insert_reserve ( lp, 1, 1 );
	    if ( relocated )
	    {
	        min::relocate ( wap );
		start_sublist ( lp, wap.locate_dlp );
		next ( lp );
	    }
	    min::gen elements[1] =
	        { min::EMPTY_SUBLIST };
	    insert_before ( lp, elements, 1 );
	    start_sublist ( lp, wap.locate_dlp );
	    next ( lp );
	}
#   endif

    start_sublist ( lp );
    min::gen elements[2] =
	{ wap.reverse_attribute_name,
	  min::EMPTY_SUBLIST };
    insert_before ( lp, elements, 2 );

    start_sublist ( wap.dlp, wap.locate_dlp );
#   if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	next ( wap.dlp );
#   endif
    start_sublist ( wap.dlp );
    next ( wap.dlp );

    wap.state = ap_type::REVERSE_LOCATE_SUCCEED;
}
