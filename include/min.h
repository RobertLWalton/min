// MIN Language Interface
//
// File:	min.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Nov 12 09:30:22 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/12 16:21:49 $
//   $RCSfile: min.h,v $
//   $Revision: 1.28 $

// Table of Contents:
//
//	Setup
//	C++ Number Types
//	Internal Pointer Conversion Functions
//	General Value Types and Data
//	General Value Test Functions
//	General Value Read Functions
//	General Value Constructor Functions
//	Stub Types and Data
//	Stub Functions
//	Process Interface
//	Garbage Collector Interface
//	Numbers
//	Strings
//	Labels
//	Atom Functions
//	Objects

// Setup
// -----

# ifndef MIN_H

// Include parameters.
//
# include "min_parameters.h"
# include <climits>
# include <cstring>
# include <cassert>

# ifndef MIN_DEBUG
#   define MIN_DEBUG 0
# endif

namespace min {

    struct stub;	// See Stub Types and Data.

}

// C++ Number Types
// --- ------ -----

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

namespace min { namespace unprotected {

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
#   endif

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

#   if MIN_IS_LOOSE
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
		       (unsigned MIN_INT_POINTER_TYPE)
		       v;
	    }
	    inline min::uns64 stub_p_to_uns64
		    ( void * p )
	    {
		return (min::uns64)
		       (unsigned MIN_INT_POINTER_TYPE)
		       p;
	    }
#	endif
#   endif
} }

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
	// Unimplemented for COMPACT:
	//   unsigned GEN_DIRECT_FLOAT
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
	// Unimplemented for LOOSE:
	//   unsigned GEN_DIRECT_INT // illegal
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
	    return ( v < GEN_DIRECT_INT );
	}
	// Unimplemented for COMPACT:
	//  bool is_direct_float ( min::gen v )
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
	// Unimplemented for LOOSE:
	//   bool is_direct_int ( min::gen v )
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

namespace min { namespace unprotected {

    // MUP:: functions.

#   if MIN_IS_COMPACT
	inline min::stub * stub_of ( min::gen v )
	{
	    return unprotected::uns32_to_stub_p ( v );
	}
	// Unimplemented for COMPACT:
	//   float64 direct_float_of ( min::gen v )
	inline int direct_int_of ( min::gen v )
	{
	    return (int32)
	           ( v - ( GEN_DIRECT_INT << 24 )
		       - ( 1 << 27 ) );
	}
	inline uns64 direct_str_of ( min::gen v )
	{
#	    if MIN_BIG_ENDIAN
		return ( uns64 ( v ) << 40 );
#	    else
		return ( uns64 ( v & 0xFFFFFF ) );
#	    endif
	}
	inline unsigned list_aux_of ( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned sublist_aux_of ( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned indirect_pair_aux_of
		( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned indirect_indexed_aux_of
		( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned index_of ( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned control_code_of ( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	// Unimplemented for COMPACT:
	//    uns64 long_control_code_of ( min::gen v )
#   else // if MIN_IS_LOOSE
	inline min::stub * stub_of ( min::gen v )
	{
	    return unprotected::uns64_to_stub_p
	    		( v & 0xFFFFFFFFFF );
	}
	inline float64 direct_float_of ( min::gen v )
	{
	    return (float64) v;
	}
	// Unimplemented for LOOSE:
	//   int direct_int_of ( min::gen v )
	inline uns64 direct_str_of ( min::gen v )
	{
#	    if MIN_BIG_ENDIAN
		return ( v << 24 );
#	    else
		return ( v & 0xFFFFFFFFFF );
#	    endif
	}
	inline unsigned list_aux_of ( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned sublist_aux_of ( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned indirect_pair_aux_of
		( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned indirect_indexed_aux_of
		( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned index_of ( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline unsigned control_code_of ( min::gen v )
	{
	    return ( v & 0xFFFFFF );
	}
	inline uns64 long_control_code_of ( min::gen v )
	{
	    return ( v & 0xFFFFFFFFFF );
	}
#   endif
} }

namespace min {

    // min:: functions

    inline min::stub * stub_of ( min::gen v )
    {
	assert ( is_stub ( v ) );
	return unprotected::stub_of ( v );
    }
#   if MIN_IS_LOOSE
	inline float64 direct_float_of ( min::gen v )
	{
	    assert ( is_direct_float ( v ) );
	    return unprotected::direct_float_of ( v );
	}
#   endif
#   if MIN_IS_COMPACT
	inline int direct_int_of ( min::gen v )
	{
	    assert ( is_direct_int ( v ) );
	    return unprotected::direct_int_of ( v );
	}
#   endif
    inline uns64 direct_str_of ( min::gen v )
    {
	assert ( is_direct_str ( v ) );
	return unprotected::direct_str_of ( v );
    }
    inline unsigned list_aux_of ( min::gen v )
    {
	assert ( is_list_aux ( v ) );
	return unprotected::list_aux_of ( v );
    }
    inline unsigned sublist_aux_of ( min::gen v )
    {
	assert ( is_sublist_aux ( v ) );
	return unprotected::sublist_aux_of ( v );
    }
    inline unsigned indirect_pair_aux_of
	    ( min::gen v )
    {
	assert ( is_indirect_pair_aux ( v ) );
	return unprotected::indirect_pair_aux_of ( v );
    }
    inline unsigned indirect_indexed_aux_of
	    ( min::gen v )
    {
	assert ( is_indirect_indexed_aux ( v ) );
	return unprotected::indirect_indexed_aux_of
			( v );
    }
    inline unsigned index_of ( min::gen v )
    {
	assert ( is_index ( v ) );
	return unprotected::index_of ( v );
    }
    inline unsigned control_code_of ( min::gen v )
    {
	assert ( is_control_code ( v ) );
	return unprotected::control_code_of ( v );
    }
#   if MIN_IS_LOOSE
	inline uns64 long_control_code_of ( min::gen v )
	{
	    assert ( is_control_code ( v ) );
	    return unprotected::long_control_code_of
	    		( v );
	}
#   endif
}

// General Value Constructor Functions
// ------- ----- ----------- ---------

namespace min { namespace unprotected {

    // MUP:: constructors

#   if MIN_IS_COMPACT
	inline min::gen new_gen ( min::stub * s )
	{
	    return (min::gen)
	        unprotected::stub_p_to_uns32 ( s );
	}
	inline min::gen new_direct_int_gen ( int v )
	{
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
	    return (min::gen)
	           ( p + GEN_LIST_AUX << 24 );
	}
	inline min::gen new_sublist_aux_gen
		( unsigned p )
	{
	    return (min::gen)
	           ( p + GEN_SUBLIST_AUX << 24 );
	}
	inline min::gen new_indirect_pair_aux_gen
		( unsigned p )
	{
	    return (min::gen)
	           ( p + GEN_INDIRECT_PAIR_AUX << 24 );
	}
	inline min::gen new_indirect_indexed_aux_gen
		( unsigned p )
	{
	    return (min::gen)
	           (   p
		     + GEN_INDIRECT_INDEXED_AUX << 24 );
	}
	inline min::gen new_index_gen ( unsigned a )
	{
	    return (min::gen)
	           ( a + GEN_INDEX << 24 );
	}
	inline min::gen new_control_code_gen
		( unsigned c )
	{
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
		   ( unprotected::stub_p_to_uns64 ( s )
		     + ( (uns64) GEN_STUB << 40 )  );
	}
	// Unimplemented for LOOSE:
	//   min::gen new_direct_int_gen ( int v )
	inline min::gen new_direct_float_gen
		( float64 v )
	{
	    return * (min::gen *) & v;
	}
	inline min::gen new_direct_str_gen
		( const char * p )
	{
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
	    return (min::gen)
	           (   p
		     + ( (uns64) GEN_LIST_AUX << 40 ) );
	}
	inline min::gen new_sublist_aux_gen
		( unsigned p )
	{
	    return (min::gen)
	           (   p
		     + ( (uns64) GEN_SUBLIST_AUX
		         << 40 ) );
	}
	inline min::gen new_indirect_pair_aux_gen
		( unsigned p )
	{
	    return (min::gen)
	           (   p
		     + ( (uns64) GEN_INDIRECT_PAIR_AUX
		         << 40 ) );
	}
	inline min::gen new_indirect_indexed_aux_gen
		( unsigned p )
	{
	    return (min::gen)
	           (   p
		     + ( (uns64)
		         GEN_INDIRECT_INDEXED_AUX
		         << 40 ) );
	}
	inline min::gen new_index_gen ( unsigned a )
	{
	    return (min::gen)
	           ( a + ( (uns64) GEN_INDEX << 40 ) );
	}
	inline min::gen new_control_code_gen
		( unsigned c )
	{
	    return (min::gen)
	           (   c
		     + ( (uns64) GEN_CONTROL_CODE
		         << 40 ) );
	}
	inline min::gen new_long_control_code_gen
		( min::uns64 c )
	{
	    return (min::gen)
	           (   c
		     + ( (uns64) GEN_CONTROL_CODE
		         << 40 ) );
	}

#   endif
} }

namespace min {

    // min:: constructors

    inline min::gen new_gen ( min::stub * s )
    {
	return unprotected::new_gen ( s );
    }
#   if MIN_IS_COMPACT
	inline min::gen new_direct_int_gen ( int v )
	{
	    assert ( -1 << 27 <= v && v < 1 << 27 );
	    return unprotected::new_direct_int_gen
	    		( v );
	}
#   endif
#   if MIN_IS_LOOSE
	inline min::gen new_direct_float_gen
		( float64 v )
	{
	    return unprotected::new_direct_float_gen
	    		( v );
	}
#   endif
    inline min::gen new_direct_str_gen
	    ( const char * p )
    {
#       if MIN_IS_COMPACT
	    assert ( strlen ( p ) <= 3 );
#	else // MIN_IS_LOOSE
	    assert ( strlen ( p ) <= 5 );
#	endif
	return unprotected::new_direct_str_gen ( p );
    }
    inline min::gen new_list_aux_gen ( unsigned p )
    {
	assert ( p < 1 << 24 );
	return unprotected::new_list_aux_gen ( p );
    }
    inline min::gen new_sublist_aux_gen
	    ( unsigned p )
    {
	assert ( p < 1 << 24 );
	return unprotected::new_sublist_aux_gen ( p );
    }
    inline min::gen new_indirect_pair_aux_gen
	    ( unsigned p )
    {
	assert ( p < 1 << 24 );
	return unprotected::new_indirect_pair_aux_gen
			( p );
    }
    inline min::gen new_indirect_indexed_aux_gen
	    ( unsigned p )
    {
	assert ( p < 1 << 24 );
	return unprotected::new_indirect_indexed_aux_gen
			( p );
    }
    inline min::gen new_index_gen ( unsigned a )
    {
	assert ( a < 1 << 24 );
	return unprotected::new_index_gen ( a );
    }
    inline min::gen new_control_code_gen
	    ( unsigned c )
    {
	assert ( c < 1 << 24 );
	return unprotected::new_control_code_gen ( c );
    }
#   if MIN_IS_LOOSE
	inline min::gen new_long_control_code_gen
		( min::uns64 c )
	{
	    assert ( c < (uns64) 1 << 40 );
	    return
	      unprotected::new_long_control_code_gen
	      	( c );
	}
#   endif
}

// Stub Types and Data
// ---- ----- --- ----

namespace min {

    // Stub type codes.

    // Collectable.
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

    // Uncollectable.
    //
    const int LABEL_AUX			= -1;

    struct stub
    {
	union {
	    gen g;
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

// Stub Functions
// ---- ---------

namespace min {

    inline int type_of ( min::stub * s )
    {
        return s->c.i8[7*MIN_LITTLE_ENDIAN];
    }

    inline bool is_collectable ( int type )
    {
    	return type >= 0;
    }

    inline bool is_deallocated ( min::stub * s )
    {
        return type_of ( s ) == min::DEALLOCATED;
    }

    inline void assert_allocated
	    ( min::stub * s, unsigned size )
    {
        if ( MIN_DEALLOCATED_LIMIT < size || MIN_DEBUG )
	{
	    assert ( ! is_deallocated ( s ) );
	}
    }

    void deallocate ( min::stub * s );

    namespace unprotected {

        inline min::uns64 value_of ( min::stub * s )
	{
	    return s->v.u64;
	}

        inline min::float64 float_of ( min::stub * s )
	{
	    return s->v.f64;
	}

        inline min::gen gen_of ( min::stub * s )
	{
	    return s->v.g;
	}

        inline min::uns64 control_of ( min::stub * s )
	{
	    return s->c.u64;
	}

        inline void set_value_of
		( min::stub * s, min::uns64 v )
	{
	    s->v.u64 = v;
	}

        inline void set_float_of
		( min::stub * s, min::float64 v )
	{
	    s->v.f64 = v;
	}

        inline void set_gen_of
		( min::stub * s, min::gen v )
	{
	    s->v.g = v;
	}

        inline void set_control_of
		( min::stub * s, min::uns64 v )
	{
	    s->c.u64 = v;
	}

        inline min::uns64 stub_control
		( int type_code, unsigned flags,
		  unsigned value = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
	    	   |
		   ( min::uns64 ( flags ) << 44 )
		   |
		   value;
	}

        inline min::uns64 stub_control
		( int type_code, unsigned flags,
		  min::stub * s )
	{
	    return ( min::uns64 ( type_code ) << 56 )
	    	   |
		   ( min::uns64 ( flags ) << 44 )
		   |
#	    if MIN_IS_COMPACT
		   min::unprotected::
		        stub_p_to_uns32 ( s );
#	    else // if MIN_IS_LOOSE
		   min::unprotected::
		        stub_p_to_uns64 ( s );
#	    endif
	}

        inline int type_of_control ( min::uns64 c )
	{
	    return int ( min::int64 ( c ) >> 56 );
        }

        inline unsigned flags_of_control
		( min::uns64 c )
	{
	    return unsigned ( c >> 44 ) & 0xFFFF;
        }

        inline unsigned value_of_control
		( min::uns64 c )
	{
	    return unsigned ( c & 0xFFFFFFFFFFF );
        }

        inline min::stub * pointer_of_control
		( min::uns64 c )
	{
#	    if MIN_IS_COMPACT
	       return min::unprotected::uns32_to_stub_p
	       		( min::uns32 ( c ) );
#	    else // if MIN_IS_LOOSE
	       return min::unprotected::uns64_to_stub_p
	       		( c & 0xFFFFFFFFFFF );
#	    endif

        }
    }

}

// Process Interface
// ------- ---------

// This interface includes a process control block
// and functions to test for interrupts.  The process
// control block includes data for other subsystems.

namespace min { namespace unprotected {

    struct body_control; // See Garbage Collector
      			 // Interface.

    struct process_control {

	bool interrupt_flag;
	    // On if interrupt should occur.

	bool relocated_flag;
	    // On if bodies have been relocated.

        min::stub * last_allocated_stub;
	    // See Garbage Collector Interface.

	min::unprotected::
	     body_control * end_body_control;
	     // See Garbage Collector Interface.
    };

    process_control * current_process;

    // Out of line function to execute interrupt.
    // Returns true.
    //
    bool interrupt ( void );

} }

namespace min {

    inline bool interrupt ( void )
    {
        if ( unprotected::
	     current_process->interrupt_flag )
	    return unprotected::interrupt();
	else return false;
    }

    inline bool relocated_flag ( void )
    {
         return unprotected::
	        current_process->relocated_flag;
    }
    inline bool set_relocated_flag ( bool value )
    {
         bool old_value =
	     unprotected::
	     current_process->relocated_flag;
	 unprotected::current_process->relocated_flag =
	     value;
	 return old_value;
    }

    class relocated {
    public:
        bool relocated_flag;
	relocated ( void )
	{
	    relocated_flag =
	        min::set_relocated_flag ( false );
	}
	~ relocated ( void )
	{
	    min::set_relocated_flag ( relocated_flag );
	}
	operator bool ()
	{
	    if ( min::set_relocated_flag ( false ) )
	        return relocated_flag = true;
	    else
	        return false;
	}
    };
}

// Garbage Collector Interface
// ------- --------- ---------

// This interface includes high performance inline
// functions that allocate stubs and bodies and that
// write general values containing pointers into
// stubs or bodies.

namespace min { namespace unprotected {

    // For COMPACT implementations, the low order
    // 32 bits of the stub control hold the chain
    // pointer, and the next higher order 24 bits
    // are gc flags.
    //
    // For LOOSE implentations, the low order 44 bits
    // of the stub control hold the chain pointer, and
    // the next higher order 12 bits are the gc flags.

    // GC flags.
    //
    const unsigned MARKS = ( 0x3F << 6 );
    const unsigned AREAS = 0x3F;

    // Mutator (non-gc execution engine) action:
    //
    // If a pointer to stub S2 is stored in a datum
    // with stub S1, then the AREAS flags of S1 are
    // logically OR'ed into the MARKS flags of S2.
    //
    // In addition, if any MARKS flag turned on in S2
    // by this action is also on in MUP::gc_stack_marks,
    // a pointer to S2 is added to the MUP::gc_stack if
    // that stack is not full.
    //
    // When a new stub is allocated, it is given the
    // flags in MUP::gc_new_stub_flags, but is NOT put
    // on the MUP::gc_stack.

    unsigned gc_stack_marks;
    unsigned gc_new_stub_flags;

    // The GC Stack is a vector of min::stub * values
    // that is filled from low to high addresses.  MUP::
    // gc_stack points at the first empty location.
    // MUP::gc_stack_end points just after the vector.
    // MUP::gc_stack >= MUP::gc_stack_end iff the stack
    // is full.
    //
    min::stub * gc_stack;
    min::stub * gc_stack_end;


    // Stub allocation is from a single list of stubs
    // chained together by the chain part of the stub
    // control.
    //
    // A pointer to the last allocated stub is maintain-
    // ed.  To allocate a new stub, this is updated to
    // the next stub on the list, if any.  Otherwise, if
    // there is no next stub, an out-of-line function,
    // gc_stub_expand_free_list, is called to add to the
    // end of the list.
    //
    // Pointer to the last allocated stub, which must
    // exist (it can be a dummy).
    //
    // In process control:
    //		min::stub * last_allocated_stub;
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
    inline min::stub * new_stub ( void )
    {
#	if MIN_IS_COMPACT
	    uns32 v = current_process->
	              last_allocated_stub->
	    		c.u32[MIN_BIG_ENDIAN];
	    if ( v == 0 )
	        v = gc_stub_expand_free_list ();
	    return current_process->
	           last_allocated_stub =
	           unprotected::uns32_to_stub_p ( v );
#	else // if MIN_IS_LOOSE
	    uns64 v = current_process->
	              last_allocated_stub->c.u64;
	    v &= 0x00000FFFFFFFFFFF;
	    if ( v == 0 )
	        v = gc_stub_expand_free_list ();
	    return current_process->
	           last_allocated_stub =
	           unprotected::uns64_to_stub_p ( v );
#	endif
    }

    // Allocation of bodies is from a stack-like region
    // of memory.  Bodies are separated by body control
    // structures.
    //
    // The tail_body_control is just before the last
    // body of the region, and the end_body_control is
    // just after this body, and is at the very end of
    // the region.  The last body of the region is free,
    // and from its beginning are allocated new bodies.

    struct body_control {
        uns64 control;
	    // Pointer to stub associated with the
	    // following body, or 0 if body is free.
	    // High order 16 bits can be used in the
	    // future for other info.
	int64 size_difference;
	    // Size of next body - size of previous
	    // body, in bytes.  Each body size includes
	    // one body_control.  If there is no next
	    // body, that size is 0, and if there is
	    // no previous body, that size is 0.
    };
    //
    // In process_control:
    //	    body_control * end_body_control;
    //
    // Out of line function to return end_body_control
    // value for a situation in which the last free
    // body has at least n + sizeof ( body_control )
    // bytes.
    //
    body_control * gc_body_stack_expand ( unsigned n );
    //
    // Function to return the address of the body_con-
    // trol in front of a newly allocated body with n'
    // bytes, where n' is n rounded up to a multiple of
    // 8.
    //
    inline body_control * new_body ( unsigned n )
    {
        n = ( n + 7 ) & ~ 07;
	body_control * end = current_process->
			     end_body_control;
	if (   end->size_difference
	     + 2 * sizeof ( body_control )
	     + n > 0 )
	    end = gc_body_stack_expand ( n );
	uns8 * address = (uns8 *) end;
	address += end->size_difference;
	body_control * head = (body_control *) address;
	address += n + sizeof ( body_control );
	body_control * tail = (body_control *) address;
	head->size_difference +=
	    end->size_difference
	    + n + sizeof ( body_control );
	tail->size_difference =
	    - end->size_difference
	    - 2 * ( n + sizeof ( body_control ) );
	end->size_difference +=
	    n + sizeof ( body_control );
	tail->control = 0;
	return head;
    }
} }

// Numbers
// -------

namespace min {

#   if MIN_IS_COMPACT
	namespace unprotected {

	    // Function to create new number stub or
	    // return an existing stub.
	    //
	    min::gen new_num_stub_gen
		( min::float64 v );
	}

	inline min::float64 float_of ( min::stub * s )
	{
	    assert ( type_of ( s ) == min::NUMBER );
	    return unprotected::float_of ( s );
	}
        inline bool is_num ( min::gen v )
	{
	    if ( v >= ( min::GEN_DIRECT_STR << 24 ) )
	        return false;
	    else if ( v >=
	              ( min::GEN_DIRECT_INT << 24 ) )
	        return false;
	    else
	        return
		  ( type_of
		      ( unprotected::uns32_to_stub_p
		      		( v ) )
		        == min::NUMBER );
	}
	inline min::gen new_gen ( int v )
	{
	    if ( ( -1 << 27 ) <= v && v < ( 1 << 27 ) )
		return unprotected::new_direct_int_gen
				( v );
	    return unprotected::new_num_stub_gen ( v );
	}
	inline min::gen new_gen ( float64 v )
	{
	    if ( ( -1 << 27 ) <= v && v < ( 1 << 27 ) )
	    {
	        int i = (int) v;
		if ( i == v )
		    return unprotected::
		           new_direct_int_gen ( i );
	    }
		return unprotected::
		       new_num_stub_gen ( v );
	    }
	    inline int int_of ( min::gen v )
	    {
		if ( v < ( min::GEN_DIRECT_INT << 24 ) )
		{
		    min::stub * s =
			unprotected::
			uns32_to_stub_p ( v );
		    assert (    type_of ( s )
		             == min::NUMBER );
		    min::float64 f = s->v.f64;
		    assert (    INT_MIN <= f
		             && f <= INT_MAX );
		    int i = (int) f;
		    assert ( i == f );
		    return i;
		}
		else if
		    (   v
		      < ( min::GEN_DIRECT_STR << 24 ) )
		    return unprotected::
		           direct_int_of ( v );
		else
		{
		    assert ( is_num ( v ) );
		}
	    }
	    inline float64 float_of ( min::gen v )
	    {
		if ( v < ( min::GEN_DIRECT_INT << 24 ) )
		{
		    min::stub * s =
			unprotected::
			uns32_to_stub_p ( v );
		    return float_of ( s );
		}
		else if
		    (   v
		      < ( min::GEN_DIRECT_STR << 24 ) )
		    return unprotected::
		           direct_int_of ( v );
		else
		{
		    assert ( is_num ( v ) );
		}
	    }
    #   else // if MIN_IS_LOOSE
	    inline bool is_num ( min::gen v )
	    {
		return min::is_direct_float ( v );
	    }
	    inline min::gen new_gen ( int v )
	    {
		return new_direct_float_gen ( v );
	    }
	    inline min::gen new_gen ( float64 v )
	    {
		return new_direct_float_gen ( v );
	    }
	    inline int int_of ( min::gen v )
	    {
		assert ( is_num ( v ) );
		min::float64 f = (float64) ( v );
		assert ( INT_MIN <= f && f <= INT_MAX );
		int i = (int) f;
		assert ( i == f );
		return i;
	    }
	    inline float64 float_of ( min::gen v )
	    {
		assert ( is_num ( v ) );
		return (float64) ( v );
	    }
    #   endif

    min::uns32 floathash ( min::float64 f );

    inline min::uns32 numhash ( min::gen v )
    {
	return floathash ( min::float_of ( v ) );
    }
}

// Strings
// -------

namespace min { namespace unprotected {

    struct long_str {
	min::uns32 length;
	min::uns32 hash;
    };

    min::uns64 short_str_of ( min::stub * s )
    {
	return s->v.u64;
    }
    void set_short_str_of
	    ( min::stub * s, min::uns64 str )
    {
	s->v.u64 = str;
    }
    min::unprotected::long_str * long_str_of
	    ( min::stub * s )
    {
	return (min::unprotected::long_str *)
	       unprotected::
	       uns64_to_pointer ( s->v.u64 );
    }
    const char * str_of
    	    ( min::unprotected::long_str * str )
    {
	return (const char *) str
	       + sizeof ( min::unprotected::long_str );
    }
    char * writable_str_of
    	    ( min::unprotected::long_str * str )
    {
	return (char *) str
	       + sizeof ( min::unprotected::long_str );
    }
    inline unsigned hash_of
        ( min::unprotected::long_str * str )
    {
	return str->hash;
    }
    inline void set_length_of
	    ( min::unprotected::long_str * str,
	      unsigned length )
    {
	str->length = length;
    }
    inline void set_hash_of
	    ( min::unprotected::long_str * str,
	      unsigned hash )
    {
	str->hash = hash;
    }
} }

namespace min {

    inline unsigned length_of
    	    ( min::unprotected::long_str * str )
    {
	return str->length;
    }

    // Function to compute the hash of an arbitrary
    // char string.
    //
    min::uns32 strhash
	    ( const char * p, unsigned size );

    inline unsigned hash_of
	    ( min::unprotected::long_str * str )
    {
	if ( unprotected::hash_of ( str ) == 0 )
	    unprotected::set_hash_of
		( str,
		  strhash
		    ( unprotected::str_of ( str ),
		      length_of ( str ) ) );
	return unprotected::hash_of ( str );
    }

    inline unsigned strlen ( min::stub * s )
    {
	if ( type_of ( s ) == min::SHORT_STR )
	{
	    char * p = s->v.c8;
	    char * endp = p + 8;
	    while ( * p && p < endp ) ++ p;
	    return p - s->v.c8;
	}
	assert ( type_of ( s ) == min::LONG_STR );
	return length_of
	    ( unprotected::long_str_of ( s ) );
    }

    inline min::uns32 strhash ( min::stub * s )
    {
	if ( type_of ( s ) == min::SHORT_STR )
	{
	    int n;
	    if ( s->v.c8[7] != 0 ) n = 8;
	    else n = ::strlen ( s->v.c8 );
	    return min::strhash ( s->v.c8, n );
	}
	assert ( type_of ( s ) == min::LONG_STR );
	min::unprotected::long_str * ls =
	    unprotected::long_str_of ( s );
	return min::strhash
	    ( min::unprotected::str_of ( ls ),
	      min::length_of ( ls ) );
    }

    inline char * strcpy ( char * p, min::stub * s )
    {
	if ( type_of ( s ) == min::SHORT_STR )
	{
	    if ( s->v.c8[7] )
		p[8] = 0;
	    return strncpy ( p, s->v.c8, 8 );
	}
	assert ( type_of ( s ) == min::LONG_STR );
	return ::strcpy
	    ( p, min::unprotected::writable_str_of
		   ( unprotected::long_str_of ( s ) ) );
    }

    inline char * strncpy
	( char * p, min::stub * s, unsigned n )
    {
	if ( type_of ( s ) == min::SHORT_STR )
	{
	    if ( s->v.c8[7] && n >= 9 )
		p[8] = 0;
	    return ::strncpy
		     ( p, s->v.c8, n < 8 ? n : 8 );
	}
	assert ( type_of ( s ) == min::LONG_STR );
	return ::strncpy
	    ( p, min::unprotected::writable_str_of
		   ( unprotected::long_str_of ( s ) ),
		 n );
    }

    unsigned strlen ( min::gen v );
    min::uns32 strhash ( min::gen v );
    char * strcpy ( char * p, min::gen v );
    char * strncpy ( char * p, min::gen v, unsigned n );


    // Some forward reference stuff that must be
    // declared here before it is referenced by a
    // friend declaration.
    //
    namespace unprotected {
	class str_pointer;
    }
    const char * min::str_of
	( min::unprotected::str_pointer & sp );
    void min::relocate
	( min::unprotected::str_pointer & sp );

    namespace unprotected {

	class str_pointer
	{
	public:

	    str_pointer ( min::gen v )
	    {

		// TBD: push v into GC protection stack?

		if ( min::is_direct_str ( v ) )
		{
		    u.str = min::unprotected::
			    direct_str_of ( v );
		    beginp = u.buf;
		    s = NULL;
		    return;
		}
		s = min::stub_of ( v );
		if ( min::type_of ( s )
		     == min::SHORT_STR )
		{
		    u.str = s->v.u64;
		    u.buf[8] = 0;
		    beginp = u.buf;
		    s = NULL;
		    return;
		}
		assert ( min::type_of ( s )
			 == min::LONG_STR );
		beginp =
		    min::unprotected::str_of
			( min::unprotected::long_str_of
			      ( s ) );
	    }

	    friend const char * min::str_of
		( str_pointer & sp );
	    friend void min::relocate
		( str_pointer & sp );

	private:

	    min::stub * s;
		// Stub pointer if long string, or
		// NULL otherwise.

	    const char * beginp;
		// Pointer to start of string.

	    union { char buf[9]; min::uns64 str; } u;
		// Place to store direct string, and to
		// store short string so as to add a
		// NUL to end.
	};

    }

    inline const char * str_of
	    ( min::unprotected::str_pointer & sp )
    {
	return sp.beginp;
    }

    inline void relocate
	    ( min::unprotected::str_pointer & sp )
    {
	if ( sp.s != NULL )
	    sp.beginp =
		min::unprotected::str_of
		    ( min::unprotected::
		      long_str_of
			  ( sp.s ) );
    }

    inline bool is_str ( min::gen v )
    {
	if ( min::is_direct_str ( v ) )
	    return true;
	if ( ! min::is_stub ( v ) )
	    return false;
	min::stub * s = min::unprotected::stub_of ( v );
	return min::type_of ( s ) == min::SHORT_STR
	       ||
	       min::type_of ( s ) == min::LONG_STR;
    }

    namespace unprotected {
	min::gen new_str_stub_gen ( const char * p );
    }

    inline min::gen new_gen ( const char * p )
    {
	unsigned n = ::strlen ( p );
#	if MIN_IS_COMPACT
	    if ( n <= 3 )
		return min::unprotected::
		       new_direct_str_gen ( p );
#	else // if MIN_IS_LOOSE
	    if ( n <= 5 )
		return min::unprotected::
		       new_direct_str_gen ( p );
#	endif
	return min::unprotected::
	       new_str_stub_gen ( p );
    }
}

// Labels
// ------

// Labels are implemented by a chain beginning at the
// label stub and continuing with auxiliary stubs of
// type min::LABEL_AUX.  This is done on the presumption
// that most labels have only 2 or 3 components.
//
// The value of the min::LABEL stub points at the first
// of the chain of min::LABEL_AUX stubs.  Each of these
// has an element as value and a pointer to the next
// stub as chain.  min::LABEL_AUX stubs are uncollec-
// table.
//
// All of the pointers to min::LABEL_AUX stubs are
// stored as uns64 addresses and NOT as VSNs.

namespace min {

    inline unsigned lab_of
	    ( min::gen * p, unsigned n, min::stub * s )
    {
        assert ( min::type_of ( s ) == min::LABEL );
	min::stub * aux = (min::stub *)
			  min::unprotected::
	                       uns64_to_pointer
			           ( s->v.u64 );
	unsigned count = 0;
        while ( aux && count < n )
	{
	    * p ++ = min::gen ( aux->v.g );
	    ++ count;
	    aux = (min::stub *)
	          min::unprotected::uns64_to_pointer
	    	    ( aux->c.u64 & 0xFFFFFFFFFFFF );
	}
	return count;
    }

    inline unsigned lab_of
	    ( min::gen * p, unsigned n, min::gen v )
    {
	return min::lab_of ( p, n, min::stub_of ( v ) );
    }

    inline unsigned lablen ( min::stub * s )
    {
        assert ( min::type_of ( s ) == min::LABEL );
	min::stub * aux = (min::stub *)
			  min::unprotected::
	                       uns64_to_pointer
			           ( s->v.u64 );
	unsigned count = 0;
        while ( aux )
	{
	    ++ count;
	    aux = (min::stub *)
	          min::unprotected::uns64_to_pointer
	    	    ( aux->c.u64 & 0xFFFFFFFFFFFF );
	}
	return count;
    }

    inline unsigned lablen ( min::gen v )
    {
	return min::lablen ( min::stub_of ( v ) );
    }

    min::uns32 labhash ( min::stub * s );
    inline min::uns32 labhash ( min::gen v )
    {
	return min::labhash ( min::stub_of ( v ) );
    }
    min::uns32 labhash
	    ( const min::gen * p, unsigned n );

    min::gen new_gen ( const min::gen * p, unsigned n );

    inline bool is_lab ( min::gen v )
    {
	if ( ! min::is_stub ( v ) )
	    return false;
	min::stub * s = min::unprotected::stub_of ( v );
	return min::type_of ( s ) == min::LABEL;
    }
}

// Atom Functions
// ---- ---------

namespace min {
    inline bool is_atom ( min::gen v )
    {
        return min::is_num ( v )
	       ||
	       min::is_str ( v )
	       ||
	       min::is_lab ( v );
    }

    min::uns32 hash ( min::gen v );
}

// Objects
// -------

namespace min { namespace unprotected {

    // It is possible to encode the vector_offset in 8
    // bits by allowing only a small number of hash
    // table sizes that are referenced through a table.
    // By these means at least 8 bits of flags can be
    // added to these headers if desired.
    //
    struct short_obj
    {
        min::uns16	vector_offset;
        min::uns16	unused_offset;
        min::uns16	auxiliary_offset;
        min::uns16	end_offset;
    };

    struct long_obj
    {
        min::uns32	vector_offset;
        min::uns32	unused_offset;
        min::uns32	auxiliary_offset;
        min::uns32	end_offset;
    };

    inline min::unprotected::short_obj * short_obj_of
	    ( min::stub * s )
    {
        return (min::unprotected::short_obj *)
	       min::unprotected::
	       uns64_to_pointer
	           ( min::unprotected::value_of ( s ) );
    }

    inline min::unprotected::long_obj * long_obj_of
	    ( min::stub * s )
    {
        return (min::unprotected::long_obj *)
	       min::unprotected::
	       uns64_to_pointer
	           ( min::unprotected::value_of ( s ) );
    }
} }

namespace min {

    inline unsigned hash_table_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   so->vector_offset
	       - sizeof ( min::unprotected::short_obj )
	         / sizeof ( min::gen );
    }

    inline unsigned hash_table_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   lo->vector_offset
	       - sizeof ( min::unprotected::long_obj )
	         / sizeof ( min::gen );
    }

    inline unsigned attribute_vector_size_of
	    ( min::unprotected::short_obj * so )
    {
        return so->unused_offset - so->vector_offset;
    }

    inline unsigned attribute_vector_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return lo->unused_offset - lo->vector_offset;
    }

    inline unsigned auxiliary_area_size_of
	    ( min::unprotected::short_obj * so )
    {
        return so->end_offset - so->auxiliary_offset;
    }

    inline unsigned auxiliary_area_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return lo->end_offset - lo->auxiliary_offset;
    }

    inline unsigned unused_area_size_of
	    ( min::unprotected::short_obj * so )
    {
        return so->auxiliary_offset - so->unused_offset;
    }

    inline unsigned unused_area_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return lo->auxiliary_offset - lo->unused_offset;
    }

    inline unsigned total_size_of
	    ( min::unprotected::short_obj * so )
    {
        return so->end_offset;
    }

    inline unsigned total_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return lo->end_offset;
    }
}

// Numbers
// -------

namespace min {
}

// TBD
// ---

namespace min {
}

# endif // MIN_H
