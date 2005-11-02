// MIN Language Protected Interface
//
// File:	min.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Nov  2 07:29:56 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/02 18:21:59 $
//   $RCSfile: min.h,v $
//   $Revision: 1.14 $

// Table of Contents:
//
//	Setup
//	Number Types
//	Internal Pointer Conversion Functions
//	General Value Types and Data
//	General Value Test Functions
//	General Value Read Functions
//	General Value Constructor Functions
//	Stub Types and Data

// Setup
// -----

# ifndef MIN_H

// Include parameters.
//
# include "min_parameters.h"
# include <cstring>
# include <cassert>

namespace min {

    struct stub;

}

// Number Types
// ------ -----

namespace min {

    typedef unsigned char uns8;
    typedef signed char int8;

    typedef unsigned short uns16;
    typedef signed short int16;

    typedef unsigned MIN_INT32_TYPE uns32;
    typedef signed MIN_INT32_TYPE int32;
    typedef float float32;

    typedef unsigned MIN_INT64_TYPE uns64;
    typedef signed MIN_INT64_TYPE int64;
    typedef double float64;
}

// Internal Pointer Conversion Functions
// -------- ------- ---------- ---------

namespace min {

    // We need to be able to convert unsigned integers
    // to pointers and vice versa.

#   if MIN_IS_COMPACT
	inline void * uns32_to_pointer ( min::uns32 v )
	{
	    return (void *)
		   (unsigned MIN_INT_POINTER_TYPE) v;
	}
	inline min::uns32 pointer_to_uns32 ( void * p )
	{
	    return (min::uns32)
		   (unsigned MIN_INT_POINTER_TYPE) p;
	}
#	if MIN_USES_VSNS
	    inline min::stub * uns32_to_stub_p
	    	( min::uns32 v )
	    {
		return (min::stub *)
		       (unsigned MIN_INT_POINTER_TYPE)
		       (   ( v << MIN_VSN_SHIFT )
			 + MIN_VSN_BASE );
	    }
	    inline min::uns32 stub_p_to_uns32
	    	( void * p )
	    {
		return (min::uns32)
		       ( ( (unsigned
		            MIN_INT_POINTER_TYPE) p
			   >> MIN_VSN_SHIFT )
			 - MIN_VSN_BASE );
	    }
#	else // MIN_USES_ADDRESSES
	    inline min::stub * uns32_to_stub_p
	    	( min::uns32 v )
	    {
		return (min::stub *)
		       (unsigned MIN_INT_POINTER_TYPE)
		       v;
	    }
	    inline min::uns32 stub_p_to_uns32
	    	( void * p )
	    {
		return (min::uns32)
		       (unsigned MIN_INT_POINTER_TYPE)
		       p;
	    }
#	endif
#   else // if MIN_IS_LOOSE
	inline void * uns64_to_pointer ( min::uns64 v )
	{
	    return (void *)
		   (unsigned MIN_INT_POINTER_TYPE) v;
	}
	inline min::uns64 pointer_to_uns64 ( void * p )
	{
	    return (min::uns64)
		   (unsigned MIN_INT_POINTER_TYPE) p;
	}
#	if MIN_USES_VSNS
	    inline min::stub * uns64_to_stub_p
	    	( min::uns64 v )
	    {
		return (min::stub *)
		       (unsigned MIN_INT_POINTER_TYPE)
		       (   ( v << MIN_VSN_SHIFT )
			 + MIN_VSN_BASE );
	    }
	    inline min::uns64 stub_p_to_uns64
	    	( void * p )
	    {
		return (min::uns64)
		       ( ( (unsigned
		            MIN_INT_POINTER_TYPE) p
			   >> MIN_VSN_SHIFT )
			 - MIN_VSN_BASE );
	    }
#	else // MIN_USES_ADDRESSES
	    inline min::stub * uns64_to_stub_p
	    	( min::uns64 v )
	    {
		return (min::stub *)
		       (unsigned MIN_INT_POINTER_TYPE) v;
	    }
	    inline min::uns64 stub_p_to_uns64 ( void * p )
	    {
		return (min::uns64)
		       (unsigned MIN_INT_POINTER_TYPE) p;
	    }
#	endif
#   endif
}

// General Value Types and Data
// ------- ----- ----- --- ----

namespace min {

    // We assume the machine has integer registers that
    // are the most efficient place for min::gen values.
    //
#   if MIN_IS_COMPACT
	typedef uns32 gen;
#   else // if MIN_IS_LOOSE
	typedef uns64 gen;
#   endif

#   if MIN_IS_COMPACT

	// Layout
	//   0x00-0xDF	stubs
	//   0xE0-0xEF  direct integers
	//   0xF0-0xF6  direct string and other
	//   0xF7-0xFF  illegal

	const unsigned GEN_STUB
	    = 0;
	const unsigned GEN_DIRECT_FLOAT
	    = 0xF7; // illegal
	const unsigned GEN_DIRECT_INT
	    = 0xE0;
	const unsigned GEN_DIRECT_STR
	    = 0xF0;
	const unsigned GEN_LIST_AUX
	    = 0xF1;
	const unsigned GEN_SUBLIST_AUX
	    = 0xF2;
	const unsigned GEN_INDIRECT_PAIR_AUX
	    = 0xF3;
	const unsigned GEN_INDIRECT_INDEXED_AUX
	    = 0xF4;
	const unsigned GEN_INDEX
	    = 0xF5;
	const unsigned GEN_CONTROL_CODE
	    = 0xF6;
	const unsigned GEN_ILLEGAL
	    = 0xF7;  // Upper limit for legal.
#   else // if MIN_IS_LOOSE

	// Layout with base MIN_FLOAT_SIGNALLING_NAN:
	//
	//   0x00-0x0F	stub
	//   0x10-0x16	direct string and others
	//   0x17-0x1F	illegal
	//   other	floating point

	const unsigned GEN_STUB
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x00;
	const unsigned GEN_DIRECT_FLOAT
	    = 0;
	const unsigned GEN_DIRECT_INT // illegal
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x17;
	const unsigned GEN_DIRECT_STR
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x10;
	const unsigned GEN_LIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x11;
	const unsigned GEN_SUBLIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x12;
	const unsigned GEN_INDIRECT_PAIR_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x13;
	const unsigned GEN_INDIRECT_INDEXED_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x14;
	const unsigned GEN_INDEX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x15;
	const unsigned GEN_CONTROL_CODE
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x16;
	const unsigned GEN_ILLEGAL
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x17;
	const unsigned GEN_UPPER
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1F;
#   endif
}

// General Value Test Functions
// ------- ----- ---- ---------

namespace min {

#   if MIN_IS_COMPACT
	inline bool is_stub ( min::gen v )
	{
	    return ( v < GEN_DIRECT_INT )
	}
	inline bool is_direct_float ( min::gen v )
	{
	    return false;
	}
	inline bool is_direct_int ( min::gen v )
	{
	    return ( v >> 28 == GEN_DIRECT_INT >> 4 );
	}
	inline bool is_direct_str ( min::gen v )
	{
	    return ( v >> 24 == GEN_DIRECT_STR );
	}
	inline bool is_list_aux ( min::gen v )
	{
	    return ( v >> 24 == GEN_LIST_AUX );
	}
	inline bool is_sublist_aux ( min::gen v )
	{
	    return ( v >> 24 == GEN_SUBLIST_AUX );
	}
	inline bool is_indirect_pair_aux ( min::gen v )
	{
	    return
	        ( v >> 24 == GEN_INDIRECT_PAIR_AUX );
	}
	inline bool is_indirect_indexed_aux
		( min::gen v )
	{
	    return
	    	( v >> 24 == GEN_INDIRECT_INDEXED_AUX );
	}
	inline bool is_index ( min::gen v )
	{
	    return ( v >> 24 == GEN_INDEX );
	}
	inline bool is_control_code ( min::gen v )
	{
	    return ( v >> 24 == GEN_CONTROL_CODE );
	}
	inline bool gen_subtype_of ( min::gen v )
	{
	    v >>= 24;
	    if ( v < GEN_DIRECT_INT )
	        return GEN_STUB;
	    else if ( v < GEN_DIRECT_STR)
	        return GEN_DIRECT_INT;
	    else if ( v < GEN_ILLEGAL)
	        return v;
	    else
	        return GEN_ILLEGAL;
	}
#   else // if MIN_IS_LOOSE
	inline bool is_stub ( min::gen v )
	{
	    return ( v >> 44 == GEN_STUB >> 4 );
	}
	inline bool is_direct_float ( min::gen v )
	{
	    // Low order 45 bits and high order bit
	    // are masked for this test.
	    //
	    return
	        (    uns32 ( v >> 45 )
	          != MIN_FLOAT64_SIGNALLING_NAN >> 5 );
	}
	inline bool is_direct_int ( min::gen v )
	{
	    return false;
	}
	inline bool is_direct_str ( min::gen v )
	{
	    return ( v >> 40 == GEN_DIRECT_STR );
	}
	inline bool is_list_aux ( min::gen v )
	{
	    return ( v >> 40 == GEN_LIST_AUX );
	}
	inline bool is_sublist_aux ( min::gen v )
	{
	    return ( v >> 40 == GEN_SUBLIST_AUX );
	}
	inline bool is_indirect_pair_aux ( min::gen v )
	{
	    return
	        ( v >> 40 == GEN_INDIRECT_PAIR_AUX );
	}
	inline bool is_indirect_indexed_aux
		( min::gen v )
	{
	    return
	    	( v >> 40 == GEN_INDIRECT_INDEXED_AUX );
	}
	inline bool is_index ( min::gen v )
	{
	    return ( v >> 40 == GEN_INDEX );
	}
	inline bool is_control_code ( min::gen v )
	{
	    return ( v >> 40 == GEN_CONTROL_CODE );
	}
	inline bool gen_subtype_of ( min::gen v )
	{
	    v >>= 40;
	    if ( v < GEN_STUB )
	        return GEN_DIRECT_FLOAT;
	    else if ( v < GEN_DIRECT_STR)
	        return GEN_STUB;
	    else if ( v < GEN_ILLEGAL)
	        return v;
	    else if ( v <= GEN_UPPER)
	        return GEN_ILLEGAL;
	    else
	        return GEN_DIRECT_FLOAT;
	}
#   endif
}

// General Value Read Functions
// ------- ----- ---- ---------

namespace min {

#   if MIN_IS_COMPACT
	inline min::stub * stub_of ( min::gen v )
	{
	    assert ( is_stub ( v ) );
	    return uns32_to_stub_p ( v );
	}
	// Unimplemented for COMPACT:
	//   float64 direct_float_of ( min::gen v )
	inline int direct_int_of ( min::gen v )
	{
	    assert ( is_direct_int ( v ) );
	    return (int32)
	           ( v - ( GEN_DIRECT_INT << 24 )
		       - ( 1 << 27 ) );
	}
	inline uns64 direct_str_of ( min::gen v )
	{
	    assert ( is_direct_str ( v ) );
#	    if MIN_BIG_ENDIAN
		return ( uns64 ( v ) << 40 );
#	    else
		return ( uns64 ( v & 0xFFFFFF ) );
#	    endif
	}
	inline unsigned list_aux_of ( min::gen v )
	{
	    assert ( is_list_aux ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned sublist_aux_of ( min::gen v )
	{
	    assert ( is_sublist_aux ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned indirect_pair_aux_of
		( min::gen v )
	{
	    assert ( is_indirect_pair_aux ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned indirect_indexed_aux_of
		( min::gen v )
	{
	    assert ( is_indirect_indexed_aux ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned index_of ( min::gen v )
	{
	    assert ( is_index ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned control_code_of ( min::gen v )
	{
	    assert ( is_control_code ( v ) );
	    return ( v & 0xFFFFFF );
	}
#   else // if MIN_IS_LOOSE
	inline min::stub * stub_of ( min::gen v )
	{
	    assert ( is_stub ( v ) );
	    return uns64_to_stub_p ( v & 0xFFFFFFFFFF );
	}
	inline float64 direct_float_of ( min::gen v )
	{
	    // Assertion always fails.
	    assert ( is_direct_float ( v ) );
	    return (float64) v;
	}
	inline int direct_int_of ( min::gen v )
	{
	    assert ( is_direct_int ( v ) );
	    // Assertion always fails.
	    return 0;
	}
	inline uns64 direct_str_of ( min::gen v )
	{
	    assert ( is_direct_str ( v ) );
#	    if MIN_BIG_ENDIAN
		return ( v << 24 );
#	    else
		return ( v & 0xFFFFFFFFFF );
#	    endif
	}
	inline unsigned list_aux_of ( min::gen v )
	{
	    assert ( is_list_aux ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned sublist_aux_of ( min::gen v )
	{
	    assert ( is_sublist_aux ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned indirect_pair_aux_of
		( min::gen v )
	{
	    assert ( is_indirect_pair_aux ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned indirect_indexed_aux_of
		( min::gen v )
	{
	    assert ( is_indirect_indexed_aux ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned index_of ( min::gen v )
	{
	    assert ( is_index ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline unsigned control_code_of ( min::gen v )
	{
	    assert ( is_control_code ( v ) );
	    return ( v & 0xFFFFFF );
	}
	inline uns64 long_control_code_of ( min::gen v )
	{
	    assert ( is_control_code ( v ) );
	    return ( v & 0xFFFFFFFFFF );
	}
#   endif
}

// General Value Constructor Functions
// ------- ----- ----------- ---------

namespace min {

#   if MIN_IS_COMPACT
	inline min::gen new_gen ( min::stub * s )
	{
	    return (min::gen) stub_p_to_uns32 ( s );
	}
	inline min::gen new_direct_int_gen ( int v )
	{
	    assert ( -1 << 27 <= v && v < 1 << 27 );
	    return (min::gen)
	           (  (uns32) v
		    + ( GEN_DIRECT_INT << 24 )
		    + ( 1 << 27 ) );
	}
	// Unimplemented for COMPACT:
	//  min::gen new_direct_float_gen ( float64 v )
	inline min::gen new_direct_str_gen
		( const char * p )
	{
	    assert ( strlen ( p ) <= 3 );
	    uns32 v = * (uns32 *) p;
#	    if MIN_BIG_ENDIAN
		return (min::gen)
		       (   ( v >> 8 )
		         + ( GEN_DIRECT_STR << 24 ) );
#	    else
		return (min::gen)
		       (   ( v & 0xFFFFFF )
		         + ( GEN_DIRECT_STR << 24 ) );
#	    endif
	}
	inline min::gen new_list_aux_gen ( unsigned p )
	{
	    assert ( p < 1 << 24 );
	    return (min::gen)
	           ( p + GEN_LIST_AUX << 24 );
	}
	inline min::gen new_sublist_aux_gen
		( unsigned p )
	{
	    assert ( p < 1 << 24 );
	    return (min::gen)
	           ( p + GEN_SUBLIST_AUX << 24 );
	}
	inline min::gen new_indirect_pair_aux_gen
		( unsigned p )
	{
	    assert ( p < 1 << 24 );
	    return (min::gen)
	           ( p + GEN_INDIRECT_PAIR_AUX << 24 );
	}
	inline min::gen new_indirect_indexed_aux_gen
		( unsigned p )
	{
	    assert ( p < 1 << 24 );
	    return (min::gen)
	           (   p
		     + GEN_INDIRECT_INDEXED_AUX << 24 );
	}
	inline min::gen new_index_gen ( unsigned a )
	{
	    assert ( a < 1 << 24 );
	    return (min::gen)
	           ( a + GEN_INDEX << 24 );
	}
	inline min::gen new_control_code_gen
		( unsigned c )
	{
	    assert ( c < 1 << 24 );
	    return (min::gen)
	           ( c + GEN_CONTROL_CODE << 24 );
	}
	// Unimplemented for COMPACT:
	//  min::gen new_long_control_code_gen
	//	( unsigned c )

#   else // if MIN_IS_LOOSE
	inline min::gen new_gen ( min::stub * s )
	{
	    return (min::gen)
		   ( stub_p_to_uns64 ( s )
		     + ( (uns64) GEN_STUB << 40 )  );
	}
	// Unimplemented for LOOSE:
	//   min::gen new_direct_int_gen ( int v )
	min::gen new_direct_float_gen ( float64 v )
	{
	    return * (min::gen *) & v;
	}
	inline min::gen new_direct_str_gen
		( const char * p )
	{
	    assert ( strlen ( p ) <= 5 );
	    uns64 v = * (uns64 *) p;
#	    if MIN_BIG_ENDIAN
		return (min::gen)
		       (   ( v >> 24 )
		         + ( (uns64) GEN_DIRECT_STR
			     << 40 ) );
#	    else
		return (min::gen)
		       (   ( v & 0xFFFFFFFFFF )
		         + ( (uns64) GEN_DIRECT_STR
			     << 40 ) );
#	    endif
	}
	inline min::gen new_list_aux_gen ( unsigned p )
	{
	    assert ( p < 1 << 24 );
	    return (min::gen)
	           (   p
		     + ( (uns64) GEN_LIST_AUX << 40 ) );
	}
	inline min::gen new_sublist_aux_gen
		( unsigned p )
	{
	    assert ( p < 1 << 24 );
	    return (min::gen)
	           (   p
		     + ( (uns64) GEN_SUBLIST_AUX
		         << 40 ) );
	}
	inline min::gen new_indirect_pair_aux_gen
		( unsigned p )
	{
	    assert ( p < 1 << 24 );
	    return (min::gen)
	           (   p
		     + ( (uns64) GEN_INDIRECT_PAIR_AUX
		         << 40 ) );
	}
	inline min::gen new_indirect_indexed_aux_gen
		( unsigned p )
	{
	    assert ( p < 1 << 24 );
	    return (min::gen)
	           (   p
		     + ( (uns64)
		         GEN_INDIRECT_INDEXED_AUX
		         << 40 ) );
	}
	inline min::gen new_index_gen ( unsigned a )
	{
	    assert ( a < 1 << 24 );
	    return (min::gen)
	           ( a + ( (uns64) GEN_INDEX << 40 ) );
	}
	inline min::gen new_control_code_gen
		( unsigned c )
	{
	    assert ( c < 1 << 24 );
	    return (min::gen)
	           (   c
		     + ( (uns64) GEN_CONTROL_CODE
		         << 40 ) );
	}
	inline min::gen new_control_code_gen
		( min::uns64 c )
	{
	    assert ( c < (uns64) 1 << 40 );
	    return (min::gen)
	           (   c
		     + ( (uns64) GEN_CONTROL_CODE
		         << 40 ) );
	}

#   endif
}

// Stub Types and Data
// ---- ----- --- ----

namespace min {

    // Stub type codes.
    //
    const int DEALLOCATED		= 1;
    const int NUMBER			= 2;
    const int SHORT_STR			= 3;
    const int LONG_STR			= 4;
    const int LABEL			= 5;
    const int SHORT_OBJ			= 6;
    const int LONG_OBJ			= 7;
    const int LIST_AUX			= 8;
    const int SUBLIST_AUX		= 9;
    const int VARIABLE_VECTOR		= 10;

    struct stub
    {
	union {
	    float64 f64;
	    uns64 u64;
	    int64 i64;
	    uns32 u32[2];
	    char c8[8];
	} v; // value

	union {
	    uns64 u64;
	    int64 i64;
	    uns32 u32[2];
	    int8 i8[8];
	} c; // control
    };
}

// Garbage Collector Interface
// ------- --------- ---------

// This interface includes high performance inline
// functions that allocate stubs and bodies and that
// write general values containing pointers into
// stubs or bodies.

namespace min {

    // For COMPACT implementations, the low order
    // 32 bits of the stub control hold the chain
    // pointer, and the next higher order 24 bits
    // are gc flags.
    //
    // For LOOSE implentations, the low order 44 bits
    // of the stub control hold the chain pointer, and
    // the next higher order 12 bits are the gc flags.

    // Stub allocation is from a single list of stubs
    // chained together by the chain part of the stub
    // control.
    //
    // A pointer to the last allocated stub is maintain-
    // ed.  To allocate a new stub, this is updated to
    // the next stub on the list, if any.  Otherwise, if
    // there is no next stub, an out-of-line function,
    // min::gc_stub_expand_free_list, is called to add
    // to the end of the list.
    //
    // Pointer to the last allocated stub, which must
    // exist (it can be a dummy).
    //
    min::stub * last_allocated_stub;
    //
    // Out of line function to return pointer to next
    // free stub as a uns32 or uns64 address or VSN.
    //
#   if MIN_IS_COMPACT
	uns32 gc_stub_expand_free_list ( void );
#   else // if MIN_IS_LOOSE
	uns64 gc_stub_expand_free_list ( void );
#   endif
    //
    // Function to return the next allocated stub.
    // This function does NOT set any part of the stub.
    min::stub * allocate_stub ( void )
    {
#	if MIN_IS_COMPACT
	    uns32 v = last_allocated_stub->
	    		c.u32[MIN_BIG_ENDIAN];
	    if ( v == 0 )
	        v = min::gc_stub_expand_free_list ();
	    return last_allocated_stub =
	           uns32_to_stub_p ( v );
#	else // if MIN_IS_LOOSE
	    uns64 v = last_allocated_stub->c.u64;
	    v &= 0x00000FFFFFFFFFFF;
	    if ( v == 0 )
	        v = min::gc_stub_expand_free_list ();
	    return last_allocated_stub =
	           uns64_to_stub_p ( v );
#	endif
    }
}

// TBD
// ---

namespace min {

    struct long_str
    {
	uns32 length;
	uns32 hash;
    };

    struct short_obj
    {
        uns16	maximum_list_length;
        uns16	list_length;
        uns16	maximum_hash_length;
        uns16	hash_length;
    };

    struct long_obj
    {
        uns32	maximum_list_length;
        uns32	list_length;
        uns32	maximum_hash_length;
        uns32	hash_length;
    };
}

# endif // MIN_H
