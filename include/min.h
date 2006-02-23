// MIN Language Interface
//
// File:	min.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Thu Feb 23 06:52:43 EST 2006
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2006/02/23 11:50:15 $
//   $RCSfile: min.h,v $
//   $Revision: 1.67 $

// Table of Contents:
//
//	Setup
//	C++ Number Types
//	Internal Pointer Conversion Functions
//	General Value Types and Data
//	General Value Constructor Functions
//	General Value Test Functions
//	General Value Read Functions
//	Control Values
//	Stub Types and Data
//	Stub Functions
//	Process Interface
//	Garbage Collector Interface
//	Numbers
//	Strings
//	Labels
//	Atom Functions
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level

// Setup
// -----

# ifndef MIN_H
# define MIN_H

// Include parameters.
//
# include "min_parameters.h"
# include <climits>
# include <cstring>
# include <cassert>
# include <new>

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

namespace min { namespace internal {

    // We need to be able to convert unsigned integers
    // to pointers and vice versa.

#   if MIN_POINTER_BITS <= 32
	typedef uns32 pointer_uns;
#   elif
	typedef uns64 pointer_uns;
#   endif

    inline void * uns64_to_pointer ( min::uns64 v )
    {
	return (void *)
	       (min::internal::pointer_uns) v;
    }
    inline min::uns64 pointer_to_uns64 ( void * p )
    {
	return (min::uns64)
	       (min::internal::pointer_uns) p;
    }

#   if MIN_STUB_NUMBER_BITS <= 32

	// If we can fit a stub address into 32 bits,
	// define routines to do so, even if we have
	// not implemented compact general values.

	inline min::stub * uns32_to_stub
		( min::uns32 v )
	{
	    return (min::stub *)
		   (min::internal::pointer_uns) v;
	    min::internal::pointer_uns p =
		(min::internal::pointer_uns) v;
#           if MIN_STUB_POINTER_BITS <= 32
		return ( min::stub * ) p;
#           elif MIN_STUB_RELATIVE_BITS <= 32
		return
		    (min::stub *)
		    ( p + (min::internal::pointer_uns)
			  MIN_STUB_BASE );
#           elif MIN_STUB_NUMBER_BITS <= 32
		return
		    (min::stub *)
		    (min::internal::pointer_uns)
		    MIN_STUB_BASE + p;
#           else
#	        error MIN_STUB_NUMBER_BITS > 32
#           endif
	}
	inline min::uns32 stub_to_uns32
		( min::stub * s )
	{
#           if MIN_STUB_POINTER_BITS <= 32
	        return (min::internal::pointer_uns) s;
#           elif MIN_STUB_RELATIVE_BITS <= 32
	        return   (min::internal::pointer_uns) s
		       - (min::internal::pointer_uns)
		         MIN_STUB_BASE;
#           elif MIN_STUB_NUMBER_BITS <= 32
	        return s - (min::stub *)
	                   (min::internal::pointer_uns)
		           MIN_STUB_BASE;
#           else
#	        error MIN_STUB_NUMBER_BITS > 32
#           endif
	}
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
#   elif MIN_IS_LOOSE
	typedef uns64 gen;
#   endif

#   if MIN_IS_COMPACT

	// Layout (high order 8 bits)
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
	    = 0xF7;  // First illegal subtype code.
	const unsigned GEN_UPPER
	    = 0xFF; // Largest subtype code.
#   elif MIN_IS_LOOSE

	// Layout (high order 24 bits) with base
	// MIN_FLOAT_SIGNALLING_NAN:
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
	    // First illegal subtype code.
	const unsigned GEN_UPPER
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1F;
	    // Largest subtype code.
#   endif
}

namespace min { namespace internal {

#   if MIN_IS_LOOSE
        inline min::stub * general_uns64_to_stub
	        ( min::uns64 v )
        {
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (v & ( ( min::uns64(1) << 44 ) - 1 ) );
#           if MIN_STUB_POINTER_BITS <= 44
	        return ( min::stub * ) p;
#           elif MIN_STUB_RELATIVE_BITS <= 44
	        return
	            (min::stub *)
		    ( p + (min::internal::pointer_uns)
		          MIN_STUB_BASE );
#           elif MIN_STUB_NUMBER_BITS <= 44
	        return
	            (min::stub *)
		    (min::internal::pointer_uns)
		    MIN_STUB_BASE + p;
#           else
#	        error MIN_STUB_NUMBER_BITS > 44
#           endif
        }

        inline min::uns64 general_stub_to_uns64
	        ( min::stub * s )
        {
#           if MIN_STUB_POINTER_BITS <= 44
	        return (min::internal::pointer_uns) s;
#           elif MIN_STUB_RELATIVE_BITS <= 44
	        return
	               (min::internal::pointer_uns) s
	             - (min::internal::pointer_uns)
		       MIN_STUB_BASE;
#           elif MIN_STUB_NUMBER_BITS <= 44
	        return   s
	               - (min::stub *)
		         (min::internal::pointer_uns)
		         MIN_STUB_BASE;
#           else
#	        error MIN_STUB_NUMBER_BITS > 44
#           endif
        }

        inline min::uns64 general_stub_into_uns64
	        ( min::uns64 v, min::stub * s )
        {
	    v &= ~ ( ( min::uns64(1) << 44 ) - 1 );
	    return v + general_stub_to_uns64 ( s );
        }
#   endif
} }

// General Value Constructor Functions
// ------- ----- ----------- ---------

namespace min { namespace unprotected {

    // MUP:: constructors

#   if MIN_IS_COMPACT
	inline min::gen new_gen ( min::stub * s )
	{
	    return (min::gen)
	        internal::stub_to_uns32 ( s );
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
	    uns32 v = (uns32) GEN_DIRECT_STR << 24;
	    char * s = ( (char *) & v )
		     + MIN_BIG_ENDIAN;
	       ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ );
	    return (min::gen) v;
	}
	inline min::gen new_list_aux_gen ( unsigned p )
	{
	    return (min::gen)
	           ( p + ( GEN_LIST_AUX << 24 ) );
	}
	inline min::gen new_sublist_aux_gen
		( unsigned p )
	{
	    return (min::gen)
	           ( p + ( GEN_SUBLIST_AUX << 24 ) );
	}
	inline min::gen new_indirect_pair_aux_gen
		( unsigned p )
	{
	    return (min::gen)
	        ( p + ( GEN_INDIRECT_PAIR_AUX << 24 ) );
	}
	inline min::gen new_indirect_indexed_aux_gen
		( unsigned p, unsigned i )
	{
	    return (min::gen)
	        (   ( p << 12 ) + i
		  + ( GEN_INDIRECT_INDEXED_AUX << 24 )
	        );
	}
	inline min::gen new_index_gen ( unsigned i )
	{
	    return (min::gen)
	           ( i + ( GEN_INDEX << 24 ) );
	}
	inline min::gen new_control_code_gen
		( unsigned c )
	{
	    return (min::gen)
	           ( c + ( GEN_CONTROL_CODE << 24 ) );
	}
	// Unimplemented for COMPACT:
	//  min::gen new_long_control_code_gen
	//	( unsigned c )
	inline min::gen renew_gen
		( min::gen v, min::uns32 p )
	{
	    return (min::gen)
	           ( ( v & 0xFF000000 ) + p );
	}

#   elif MIN_IS_LOOSE
	inline min::gen new_gen ( min::stub * s )
	{
	    return (min::gen)
		   (     internal
		       ::general_stub_to_uns64 ( s )
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
	    uns64 v = (uns64) GEN_DIRECT_STR << 40;
	    char * s = ( (char *) & v )
	             + 3 * MIN_BIG_ENDIAN;
	       ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ );
	    return (min::gen) v;
	}
	inline min::gen new_list_aux_gen ( unsigned p )
	{
	    return (min::gen)
	           (   p
		     + ( (uns64) GEN_LIST_AUX
		         << 40 ) );
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
		( unsigned p, unsigned i )
	{
	    return (min::gen)
	           (   ( (uns64) p << 20 ) + i
		     + ( (uns64)
		         GEN_INDIRECT_INDEXED_AUX
		         << 40 ) );
	}
	inline min::gen new_index_gen ( unsigned i )
	{
	    return (min::gen)
	           ( i + ( (uns64) GEN_INDEX << 40 ) );
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
	inline min::gen renew_gen
		( min::gen v, min::uns64 p )
	{
	    return (min::gen)
	           ( ( v & 0xFFFFFF0000000000 ) + p );
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
	    MIN_ASSERT ( -1 << 27 <= v && v < 1 << 27 );
	    return unprotected::new_direct_int_gen
	    		( v );
	}
#   elif MIN_IS_LOOSE
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
	    MIN_ASSERT ( strlen ( p ) <= 3 );
#	elif MIN_IS_LOOSE
	    MIN_ASSERT ( strlen ( p ) <= 5 );
#	endif
	return unprotected::new_direct_str_gen ( p );
    }
    inline min::gen new_list_aux_gen ( unsigned p )
    {
	MIN_ASSERT ( p < 1 << 24 );
	return unprotected::new_list_aux_gen ( p );
    }
    inline min::gen new_sublist_aux_gen
	    ( unsigned p )
    {
	MIN_ASSERT ( p < 1 << 24 );
	return unprotected::new_sublist_aux_gen ( p );
    }
    inline min::gen new_indirect_pair_aux_gen
	    ( unsigned p )
    {
	MIN_ASSERT ( p < 1 << 24 );
	return unprotected::new_indirect_pair_aux_gen
			( p );
    }
#   if MIN_IS_COMPACT
	inline min::gen new_indirect_indexed_aux_gen
		( unsigned p, unsigned i )
	{
	    MIN_ASSERT ( p < 1 << 12 );
	    MIN_ASSERT ( i < 1 << 12 );
	    return unprotected::
	           new_indirect_indexed_aux_gen
			    ( p, i );
	}
#   elif MIN_IS_LOOSE
	inline min::gen new_indirect_indexed_aux_gen
		( unsigned p, unsigned i )
	{
	    MIN_ASSERT ( p < 1 << 20 );
	    MIN_ASSERT ( i < 1 << 20 );
	    return unprotected::
	           new_indirect_indexed_aux_gen
			    ( p, i );
	}
#   endif
    inline min::gen new_index_gen ( unsigned i )
    {
	MIN_ASSERT ( i < 1 << 24 );
	return unprotected::new_index_gen ( i );
    }
    inline min::gen new_control_code_gen
	    ( unsigned c )
    {
	MIN_ASSERT ( c < 1 << 24 );
	return unprotected::new_control_code_gen ( c );
    }
#   if MIN_IS_LOOSE
	inline min::gen new_long_control_code_gen
		( min::uns64 c )
	{
	    MIN_ASSERT ( c < (uns64) 1 << 40 );
	    return
	      unprotected::new_long_control_code_gen
	      	( c );
	}
#   endif
}

// General Value Test Functions
// ------- ----- ---- ---------

namespace min {

#   if MIN_IS_COMPACT
	inline bool is_stub ( min::gen v )
	{
	    return ( v < ( GEN_DIRECT_INT << 24 ) );
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
	inline unsigned gen_subtype_of ( min::gen v )
	{
	    v = (min::uns32) v >> 24;
	    if ( v < GEN_DIRECT_INT )
	        return GEN_STUB;
	    else if ( v < GEN_DIRECT_STR)
	        return GEN_DIRECT_INT;
	    else if ( v < GEN_ILLEGAL)
	        return v;
	    else
	        return GEN_ILLEGAL;
	}
#   elif MIN_IS_LOOSE
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
	        ( (uns64) v & 0x7FFFE00000000000L )
	        != ( (uns64) MIN_FLOAT64_SIGNALLING_NAN
		     << 40 );
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
	inline unsigned gen_subtype_of ( min::gen v )
	{
	    v = (min::uns64) v >> 40;
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
	    return   internal
	           ::uns32_to_stub ( v );
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
	inline unsigned indirect_aux_of
		( min::gen v )
	{
	    return ( ( v >> 12 ) & 0xFFF );
	}
	inline unsigned indirect_index_of
		( min::gen v )
	{
	    return ( v & 0xFFF );
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
#   elif MIN_IS_LOOSE
	inline min::stub * stub_of ( min::gen v )
	{
	    return internal::general_uns64_to_stub
	    		( v & 0xFFFFFFFFFF );
	}
	inline float64 direct_float_of ( min::gen v )
	{
	    return * (float64 *) & v;
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
	inline unsigned indirect_aux_of
		( min::gen v )
	{
	    return ( ( v >> 20 ) & 0xFFFFF );
	}
	inline unsigned indirect_index_of
		( min::gen v )
	{
	    return ( v & 0xFFFFF );
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
	MIN_ASSERT ( is_stub ( v ) );
	return unprotected::stub_of ( v );
    }
#   if MIN_IS_LOOSE
	inline float64 direct_float_of ( min::gen v )
	{
	    MIN_ASSERT ( is_direct_float ( v ) );
	    return unprotected::direct_float_of ( v );
	}
#   endif
#   if MIN_IS_COMPACT
	inline int direct_int_of ( min::gen v )
	{
	    MIN_ASSERT ( is_direct_int ( v ) );
	    return unprotected::direct_int_of ( v );
	}
#   endif
    inline uns64 direct_str_of ( min::gen v )
    {
	MIN_ASSERT ( is_direct_str ( v ) );
	return unprotected::direct_str_of ( v );
    }
    inline unsigned list_aux_of ( min::gen v )
    {
	MIN_ASSERT ( is_list_aux ( v ) );
	return unprotected::list_aux_of ( v );
    }
    inline unsigned sublist_aux_of ( min::gen v )
    {
	MIN_ASSERT ( is_sublist_aux ( v ) );
	return unprotected::sublist_aux_of ( v );
    }
    inline unsigned indirect_pair_aux_of
	    ( min::gen v )
    {
	MIN_ASSERT ( is_indirect_pair_aux ( v ) );
	return unprotected::indirect_pair_aux_of ( v );
    }
    inline unsigned indirect_aux_of ( min::gen v )
    {
	MIN_ASSERT ( is_indirect_indexed_aux ( v ) );
	return unprotected::indirect_aux_of ( v );
    }
    inline unsigned indirect_index_of ( min::gen v )
    {
	MIN_ASSERT ( is_indirect_indexed_aux ( v ) );
	return unprotected::indirect_index_of ( v );
    }
    inline unsigned index_of ( min::gen v )
    {
	MIN_ASSERT ( is_index ( v ) );
	return unprotected::index_of ( v );
    }
    inline unsigned control_code_of ( min::gen v )
    {
	MIN_ASSERT ( is_control_code ( v ) );
	return unprotected::control_code_of ( v );
    }
#   if MIN_IS_LOOSE
	inline uns64 long_control_code_of ( min::gen v )
	{
	    MIN_ASSERT ( is_control_code ( v ) );
	    return unprotected::long_control_code_of
	    		( v );
	}
#   endif
}

// Control Values
// ------- ------

# define MIN_CONTROL_VALUE_BITS 48
# define MIN_GC_CONTROL_VALUE_BITS \
    ( 56 - MIN_GC_FLAG_BITS )

namespace min { namespace internal {

    const min::uns64 TYPE_MASK =
	~ ( ( uns64(1) << 56 ) - 1 );
    const min::uns64 CONTROL_VALUE_MASK =
	( ( uns64(1) << MIN_CONTROL_VALUE_BITS ) - 1 );
    const min::uns64 GC_CONTROL_VALUE_MASK =
	(   ( uns64(1) << MIN_GC_CONTROL_VALUE_BITS )
	  - 1 );
} }

namespace min { namespace unprotected {

    inline int type_of_control ( min::uns64 c )
    {
	return int ( min::int64 ( c ) >> 56 );
    }

    inline unsigned value_of_control
	    ( min::uns64 c )
    {
	return unsigned
	  ( c & min::internal::CONTROL_VALUE_MASK );
    }

    inline min::uns64 new_control
	    ( int type_code, min::uns64 v,
	      min::uns64 flags = 0 )
    {
	return ( min::uns64 ( type_code ) << 56 )
	       |
	       v
	       |
	       flags;
    }

    inline min::uns64 renew_control_type
	    ( min::uns64 c, int type )
    {
	return ( c & ~ min::internal::TYPE_MASK )
	       | ( min::uns64 (type) << 56 );
    }

    inline min::uns64 renew_control_value
	    ( min::uns64 c, min::uns64 v )
    {
	return ( c & ~ min::internal
	                  ::CONTROL_VALUE_MASK )
	       | v;
    }

#   if MIN_STUB_POINTER_BITS <= MIN_CONTROL_VALUE_BITS

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    return ( min::stub * )
	           (min::internal::pointer_uns)
	           (c & min::internal
		           ::CONTROL_VALUE_MASK );
	}

	inline min::uns64 new_control
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
		   (min::internal::pointer_uns) s
		   |
		   flags;
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ min::internal
	                      ::CONTROL_VALUE_MASK )
		   | (min::internal::pointer_uns) s;
	}

#   elif    MIN_STUB_RELATIVE_BITS \
         <= MIN_CONTROL_VALUE_BITS

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (c & min::internal::CONTROL_VALUE_MASK );
	    return
	        (min::stub *)
		( p + (min::internal::pointer_uns)
		      MIN_STUB_BASE );
	}

	inline min::uns64 new_control
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
	           (   (min::internal::pointer_uns) s
	             - (min::internal::pointer_uns)
		       MIN_STUB_BASE )
		   |
		   flags;
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ min::internal
	                      ::CONTROL_VALUE_MASK )
		   | (   (min::internal::pointer_uns) s
		       - (min::internal::pointer_uns)
		         MIN_STUB_BASE );
	}

#   elif    MIN_STUB_NUMBER_BITS \
         <= MIN_CONTROL_VALUE_BITS

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (c & min::internal::CONTROL_VALUE_MASK );
	    return
	        (min::stub *)
		(min::internal::pointer_uns)
		MIN_STUB_BASE + p;
	}

	inline min::uns64 new_control
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
		   (min::internal::pointer_uns) s
	           | (   s
	               - (min::stub *)
		         (min::internal::pointer_uns)
		         MIN_STUB_BASE )
		   |
		   flags;
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ min::internal
	                      ::CONTROL_VALUE_MASK )
	           | (   s
	               - (min::stub *)
		         (min::internal::pointer_uns)
		         MIN_STUB_BASE );
	}
#   else
#	error   MIN_STUB_NUMBER_BITS \
              > MIN_CONTROL_VALUE_BITS
#   endif

#   if    MIN_STUB_POINTER_BITS \
       <= MIN_GC_CONTROL_VALUE_BITS

	inline min::stub * stub_of_gc_control
		( min::uns64 c )
	{
	    return ( min::stub * )
	           (min::internal::pointer_uns)
	           (c & min::internal
		           ::GC_CONTROL_VALUE_MASK );
	}

	inline min::uns64 new_gc_control
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
		   (min::internal::pointer_uns) s
		   |
		   flags;
	}

	inline min::uns64 renew_gc_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ min::internal
	                      ::GC_CONTROL_VALUE_MASK )
		   | (min::internal::pointer_uns) s;
	}

#   elif    MIN_STUB_RELATIVE_BITS \
         <= MIN_GC_CONTROL_VALUE_BITS

	inline min::stub * stub_of_gc_control
		( min::uns64 c )
	{
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (c & min::internal
	               ::GC_CONTROL_VALUE_MASK );
	    return
	        (min::stub *)
		( p + (min::internal::pointer_uns)
		      MIN_STUB_BASE );
	}

	inline min::uns64 new_gc_control
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
	           (   (min::internal::pointer_uns) s
	             - (min::internal::pointer_uns)
		       MIN_STUB_BASE )
		   |
		   flags;
	}

	inline min::uns64 renew_gc_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ min::internal
	                      ::GC_CONTROL_VALUE_MASK )
		   | (   (min::internal::pointer_uns) s
		       - (min::internal::pointer_uns)
		         MIN_STUB_BASE );
	}

#   elif    MIN_STUB_NUMBER_BITS \
         <= MIN_GC_CONTROL_VALUE_BITS

	inline min::stub * stub_of_gc_control
		( min::uns64 c )
	{
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (c & min::internal
	               ::GC_CONTROL_VALUE_MASK );
	    return
	        (min::stub *)
		(min::internal::pointer_uns)
		MIN_STUB_BASE + p;
	}

	inline min::uns64 new_gc_control
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
		   (min::internal::pointer_uns) s
	           | (   s
	               - (min::stub *)
		         (min::internal::pointer_uns)
		         MIN_STUB_BASE )
		   |
		   flags;
	}

	inline min::uns64 renew_gc_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ min::internal
	                      ::GC_CONTROL_VALUE_MASK )
	           | (   s
	               - (min::stub *)
		         (min::internal::pointer_uns)
		         MIN_STUB_BASE );
	}
#   else
#	error   MIN_STUB_NUMBER_BITS \
              > MIN_GC_CONTROL_VALUE_BITS
#   endif

} }

// Stub Types and Data
// ---- ----- --- ----

namespace min {

    // Stub type codes.

    // Collectable.
    //
    const int FREE			= 1;
    const int DEALLOCATED		= 2;
    const int NUMBER			= 3;
    const int SHORT_STR			= 4;
    const int LONG_STR			= 5;
    const int LABEL			= 6;
    const int SHORT_OBJ			= 7;
    const int LONG_OBJ			= 8;
    const int VARIABLE_VECTOR		= 9;

    // Uncollectible.
    //
    const int LABEL_AUX			= -1;
    const int LIST_AUX			= -2;
    const int SUBLIST_AUX		= -3;

    namespace unprotected {
	// Non-gc flags for uncollectible controls.
	//
	const min::uns64 STUB_POINTER =
	    min::uns64(1) << 55;
    }

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

    inline bool is_collectible ( int type )
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
	    MIN_ASSERT ( ! is_deallocated ( s ) );
	}
    }

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

        inline void * pointer_of ( min::stub * s )
	{
	    return min::internal::
	           uns64_to_pointer ( s->v.u64 );
	}

        inline min::uns64 control_of ( min::stub * s )
	{
	    return s->c.u64;
	}

        inline bool test_flags_of
		( min::stub * s, min::uns64 flags )
	{
	    return s->c.u64 & flags;
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

        inline void set_pointer_of
		( min::stub * s, void * p )
	{
	    s->v.u64 = min::internal::
	                    pointer_to_uns64 ( p );
	}

        inline void set_control_of
		( min::stub * s, min::uns64 c )
	{
	    s->c.u64 = c;
	}

	inline void set_type_of
		( min::stub * s, int type )
	{
	    s->c.i8[7*MIN_LITTLE_ENDIAN] = type;
	}

        inline void set_flags_of
		( min::stub * s, min::uns64 flags )
	{
	    s->c.u64 |= flags;
	}

        inline void clear_flags_of
		( min::stub * s, min::uns64 flags )
	{
	    s->c.u64 &= ~ flags;
	}
    }
}

// Process Interface
// ------- ---------

// This interface includes a process control block
// and functions to test for interrupts.  The process
// control block contains pointers to stacks.

namespace min { namespace unprotected {

    bool interrupt_flag;
	// On if interrupt should occur.

    bool relocated_flag;
	// On if bodies have been relocated.

    struct process_control {

        // Pointers to stacks: TBD.

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
        if ( unprotected::interrupt_flag )
	    return unprotected::interrupt();
	else return false;
    }

    inline bool relocated_flag ( void )
    {
         return unprotected::relocated_flag;
    }
    inline bool set_relocated_flag ( bool value )
    {
         bool old_value =
	     unprotected::relocated_flag;
	 unprotected::relocated_flag =
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
	    bool current_relocated_flag =
	        min::set_relocated_flag
		    ( relocated_flag );
	    MIN_ASSERT ( ! current_relocated_flag );
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
// functions that allocate and deallocate stubs and
// bodies and that write general values containing
// pointers into stubs or bodies.

namespace min {

    void deallocate ( min::stub * s );

}

namespace min { namespace internal {

    // GC flags.
    //
    const uns64 GC_FLAG_MASK =
           ( (uns64(1) << MIN_GC_FLAG_BITS) - 1 )
	<< MIN_GC_CONTROL_VALUE_BITS;
    const uns64 GC_SCAVENGED_MASK =
    	  GC_FLAG_MASK
	& (    uns64(0xAAAAAA)
	    << MIN_GC_CONTROL_VALUE_BITS );
    const uns64 GC_MARKED_MASK =
        GC_SCAVENGED_MASK >> 1;

} }

namespace min { namespace unprotected {

    // Mutator (non-gc execution engine) action:
    //
    // If a pointer to stub S2 is stored in a datum
    // with stub S1, then for each pair of GC flags the
    // scavenged flag of the pair of S1 is logically
    // OR'ed into the marked flag of the pair of S2.
    //
    // In addition, if any marked flag turned on in S2
    // by this action is also on in MUP::gc_stack_marks,
    // a pointer to S2 is added to the MUP::gc_stack if
    // that stack is not full.
    //
    // When a new stub is allocated, it is given the
    // flags in MUP::gc_new_stub_flags, but is NOT put
    // on the MUP::gc_stack.
    //
    min::uns64 gc_stack_marks;
    min::uns64 gc_new_stub_flags;

    // The GC Stack is a vector of min::stub * values
    // that is filled from low to high addresses.  MUP::
    // gc_stack points at the first empty location.
    // MUP::gc_stack_end points just after the vector.
    // MUP::gc_stack >= MUP::gc_stack_end iff the stack
    // is full.
    //
    min::stub ** gc_stack;
    min::stub ** gc_stack_end;

    // Function executed whenever a pointer to stub S2
    // is stored in a datum with stub S1.  This function
    // updates the GC flags of S2.
    //
    // S1 is the source of the written pointer and S2
    // is the target.
    // 
    inline void gc_write_update
	    ( min::stub * s1, min::stub * s2 )
    {
        uns64 f = ( control_of ( s1 ) >> 1 )
	        & ( ~ control_of ( s2 ) )
		& internal::GC_MARKED_MASK;
	set_flags_of ( s2, f );
	if (    ( f & gc_stack_marks )
	     && gc_stack < gc_stack_end )
	    * gc_stack ++ = s2;
    }

    // Stub allocation is from a single list of stubs
    // chained together by the chain part of the stub
    // control.
    //
    // A pointer to the last allocated stub is maintain-
    // ed.  To allocate a new stub, this is updated to
    // the next stub on the list, if any.  Otherwise, if
    // there is no next stub, an out-of-line function,
    // gc_expand_stub_free_list, is called to add to the
    // end of the list.
    //
    // Unallocated stubs have stub type min::FREE and
    // zero stub control flags.

    // Pointer to the last allocated stub, which must
    // exist (it can be a dummy).
    //
    min::stub * last_allocated_stub;

    // Number of free stubs that can be allocated with-
    // out requiring a call to gc_expand_stub_free_list.
    //
    unsigned number_of_free_stubs;

    // Out of line function to increase the number of
    // free stubs to be at least n.
    //
    void gc_expand_stub_free_list ( unsigned n );

    // Function to return the next free stub as a
    // garbage collectible stub.  The type is set to
    // min::FREE and may be changed to any garbage
    // collectible type.  The value is NOT set.  The GC
    // flags are set to MUP::gc_new_stub_flags, and the
    // non-type part of the stub control is maintained
    // by the GC.
    //
    inline min::stub * new_stub ( void )
    {
	if ( number_of_free_stubs == 0 )
	    gc_expand_stub_free_list ( 1 );
	-- number_of_free_stubs;
	uns64 c = control_of ( last_allocated_stub );
	min::stub * s = stub_of_gc_control ( c );
	set_flags_of ( s, gc_new_stub_flags );
	return last_allocated_stub = s;
    }
    
    // Function to return the next free stub while
    // removing this stub from the GC list of stubs.
    //
    // This function does NOT set any part of the stub
    // returned.  The stub returned is ignored by the
    // GC.
    //
    inline min::stub * new_aux_stub ( void )
    {
	if ( number_of_free_stubs == 0 )
	    gc_expand_stub_free_list ( 1 );
	-- number_of_free_stubs;
	uns64 c = control_of ( last_allocated_stub );
	min::stub * s = stub_of_gc_control ( c );
	c = renew_gc_control_stub
	        ( c, stub_of_gc_control
			 ( control_of ( s ) ) );
	set_control_of ( last_allocated_stub, c );
	return s;
    }

    // Allocation of bodies is from a stack-like region
    // of memory.  Bodies are separated by body control
    // structures.
    //
    struct body_control {
        uns64 control;
	    // If not free, the tyep is any value but
	    // min::FREE, and the control contains a
	    // pointer to stub associated with the
	    // following body.  The stub and body itself
	    // must contain enough information to deter-
	    // mine the size of the body (in particular
	    // the location of the body_control follow-
	    // ing the body).
	    //
	    // If free, the type is min::FREE and the
	    // control value is the length of the free
	    // body following the body_control.
	int64 size_difference;
	    // Size of the body following the body_
	    // control - size of the body preceding the
	    // body_control, in bytes.  If there is no
	    // body after the body control, that size
	    // is 0, and if there is body before the
	    // body control, that size is 0.
    };

    // Free_body_control is the body_control before a
    // free body that is used as a stack to allocate
    // new bodies.  A new body is allocated to the
    // beginning of the free body, and free_body_control
    // is moved to the end of the newly allocated body.
    //
    body_control * free_body_control;

    // Out of line function to returns a value which
    // may be directly returned by new_body (see
    // below).  This function is called by new_body
    // when the body stack is too short to service an
    // allocation request.  This function may or may not
    // reset free_body_control.
    //
    // Here n must be the argument to new_body rounded
    // up to a multiple of 8.
    //
    body_control * gc_new_body ( unsigned n );

    // Function to return the address of the body_con-
    // trol in front of a newly allocated body with n'
    // bytes, where n' is n rounded up to a multiple of
    // 8.  The control member of the returned body
    // control is set to zero.
    //
    inline body_control * new_body ( unsigned n )
    {
        // We must be careful to convert unsigned and
	// sizeof values to int64 BEFORE negating them.

        n = ( n + 7 ) & ~ 07;
	body_control * head = free_body_control;
	internal::pointer_uns size =
	    value_of_control ( head->control );
	if ( size < n + sizeof ( body_control ) )
	    return gc_new_body ( n );

	// The pointers are:
	//
	//	head --------->	body_control
	//			n bytes		---+
	//	free ---------> body_control       |
	//			size - n           | ifb
	//			     - sizeof bc   |
	//			   bytes        ---+
	//	tail ---------> body_control
	//		
	// where
	//
	//	head = initial free_body_control
	//	     = body_control before returned body
	//	free = final free_body_control
	//	     = body_control after returned body
	//	     = body_control before new free body
	//	tail = body_control after original and
	//	       new free bodies
	//	sizeof bc = sizeof ( body_control )
	//	ifb = initial free body

	uns8 * address = (uns8 *) head
	               + sizeof ( body_control );
	body_control * free =
	    (body_control *) ( address + n );
	body_control * tail =
	    (body_control *) ( address + size );
	// Reset size to size of new free body.
	size -= internal::pointer_uns 
		    ( n + sizeof ( body_control ) );
	head->size_difference -=
	    int64 ( size + sizeof ( body_control ) );
	free->size_difference =
	    int64 ( size ) - int64 ( n );
	tail->size_difference +=
	    int64 ( n + sizeof ( body_control ) );
	head->control = 0;
	free->control = new_control ( min::FREE, size );
	free_body_control = free;
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
	    MIN_ASSERT ( type_of ( s ) == min::NUMBER );
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
		      ( internal::uns32_to_stub
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
			internal::
			uns32_to_stub ( v );
		    MIN_ASSERT (    type_of ( s )
		                 == min::NUMBER );
		    min::float64 f = s->v.f64;
		    MIN_ASSERT (    INT_MIN <= f
		                 && f <= INT_MAX );
		    int i = (int) f;
		    MIN_ASSERT ( i == f );
		    return i;
		}
		else if
		    (   v
		      < ( min::GEN_DIRECT_STR << 24 ) )
		    return unprotected::
		           direct_int_of ( v );
		else
		{
		    MIN_ASSERT ( is_num ( v ) );
		}
	    }
	    inline float64 float_of ( min::gen v )
	    {
		if ( v < ( min::GEN_DIRECT_INT << 24 ) )
		{
		    min::stub * s =
			internal::
			uns32_to_stub ( v );
		    return float_of ( s );
		}
		else if
		    (   v
		      < ( min::GEN_DIRECT_STR << 24 ) )
		    return unprotected::
		           direct_int_of ( v );
		else
		{
		    MIN_ASSERT ( is_num ( v ) );
		}
	    }
#   elif MIN_IS_LOOSE
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
		MIN_ASSERT ( is_num ( v ) );
		min::float64 f = (float64) ( v );
		MIN_ASSERT
		    ( INT_MIN <= f && f <= INT_MAX );
		int i = (int) f;
		MIN_ASSERT ( i == f );
		return i;
	    }
	    inline float64 float_of ( min::gen v )
	    {
		MIN_ASSERT ( is_num ( v ) );
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
	       internal::
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
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
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
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
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
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
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
	MIN_ASSERT ( type_of ( s ) == min::LONG_STR );
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
		MIN_ASSERT (    min::type_of ( s )
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
#	elif MIN_IS_LOOSE
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
        MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
	min::stub * aux = (min::stub *)
			  min::internal::
	                       uns64_to_pointer
			           ( s->v.u64 );
	unsigned count = 0;
        while ( aux && count < n )
	{
	    * p ++ = min::gen ( aux->v.g );
	    ++ count;
	    aux = (min::stub *)
	          min::internal::uns64_to_pointer
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
        MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
	min::stub * aux = (min::stub *)
			  min::internal::
	                       uns64_to_pointer
			           ( s->v.u64 );
	unsigned count = 0;
        while ( aux )
	{
	    ++ count;
	    aux = (min::stub *)
	          min::internal::uns64_to_pointer
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

    // attribute_vector_offset[I] is the offset of the
    // attribute vector in an object body, in min::gen
    // units.  I is the low order bits of the object
    // header flags word.
    // 
    const unsigned ATTRIBUTE_VECTOR_OFFSET_SIZE = 256;
    const unsigned ATTRIBUTE_VECTOR_OFFSET_MASK =
	    ATTRIBUTE_VECTOR_OFFSET_SIZE - 1;
    min::uns32 attribute_vector_offset
    		    [ATTRIBUTE_VECTOR_OFFSET_SIZE];

    struct short_obj
    {
        min::uns16	flags;
        min::uns16	unused_area_offset;
        min::uns16	aux_area_offset;
        min::uns16	total_size;
    };

    struct long_obj
    {
        min::uns32	flags;
        min::uns32	unused_area_offset;
        min::uns32	aux_area_offset;
        min::uns32	total_size;
    };

    inline min::unprotected::short_obj * short_obj_of
	    ( min::stub * s )
    {
        return (min::unprotected::short_obj *)
	       min::unprotected::pointer_of ( s );
    }

    inline min::unprotected::long_obj * long_obj_of
	    ( min::stub * s )
    {
        return (min::unprotected::long_obj *)
	       min::unprotected::pointer_of ( s );
    }

    inline unsigned flags_to_offset ( unsigned flags )
    {
        return min::unprotected::
	    attribute_vector_offset
		[   flags
		  & min::unprotected::
			 ATTRIBUTE_VECTOR_OFFSET_MASK ];
    }
} }

namespace min {

    inline unsigned hash_table_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   min::unprotected::
		      flags_to_offset (so->flags);
	       - sizeof ( min::unprotected::short_obj )
	         / sizeof ( min::gen );
    }

    inline unsigned hash_table_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   min::unprotected::
		      flags_to_offset (lo->flags);
	       - sizeof ( min::unprotected::long_obj )
	         / sizeof ( min::gen );
    }

    inline unsigned attribute_vector_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   so->unused_area_offset
               - min::unprotected::
		      flags_to_offset (so->flags);
    }

    inline unsigned attribute_vector_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   lo->unused_area_offset
               - min::unprotected::
		      flags_to_offset (lo->flags);
    }

    inline unsigned aux_area_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   so->total_size
	       - so->aux_area_offset;
    }

    inline unsigned aux_area_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   lo->total_size
	       - lo->aux_area_offset;
    }

    inline unsigned unused_area_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   so->aux_area_offset
	       - so->unused_area_offset;
    }

    inline unsigned unused_area_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   lo->aux_area_offset
	       - lo->unused_area_offset;
    }

    inline unsigned header_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   sizeof ( min::unprotected::short_obj )
	       / sizeof ( min::gen );
    }

    inline unsigned header_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   sizeof ( min::unprotected::long_obj )
	       / sizeof ( min::gen );
    }

    inline unsigned total_size_of
	    ( min::unprotected::short_obj * so )
    {
        return so->total_size;
    }

    inline unsigned total_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return lo->total_size;
    }
}

// Object Vector Level
// ------ ------ -----

namespace min { namespace unprotected {

    inline const min::gen * body_vector_of
    	    ( min::unprotected::short_obj * so )
    {
        return (const min::gen *) so;
    }

    inline const min::gen * body_vector_of
    	    ( min::unprotected::long_obj * lo )
    {
        return (const min::gen *) lo;
    }

    inline min::gen * writable_body_vector_of
    	    ( min::unprotected::short_obj * so )
    {
        return (min::gen *) so;
    }

    inline min::gen * writable_body_vector_of
    	    ( min::unprotected::long_obj * lo )
    {
        return (min::gen *) lo;
    }
} }

namespace min {

    inline unsigned hash_table_of
	    ( min::unprotected::short_obj * so )
    {
        return   sizeof ( min::unprotected::short_obj )
	       / sizeof ( min::gen );
    }

    inline unsigned hash_table_of
	    ( min::unprotected::long_obj * lo )
    {
        return   sizeof ( min::unprotected::long_obj )
	       / sizeof ( min::gen );
    }

    inline unsigned attribute_vector_of
	    ( min::unprotected::short_obj * so )
    {
        return min::unprotected::
		    flags_to_offset (so->flags);
    }

    inline unsigned attribute_vector_of
	    ( min::unprotected::long_obj * lo )
    {
        return min::unprotected::
		    flags_to_offset (lo->flags);
    }
}

// Object List Level
// ------ ---- -----

namespace min { namespace unprotected {
    class list_pointer;

    // Out of line versions of functions.
    //
    void insert_reserve
    	    ( min::unprotected::list_pointer & lp,
	      unsigned insertions,
	      unsigned elements,
	      bool use_object_aux_stubs );
} }

namespace min {

#   if MIN_IS_COMPACT
	const min::gen LIST_END = (min::gen)
	    ( (min::uns32) min::GEN_LIST_AUX << 24 );
	const min::gen EMPTY_SUBLIST = (min::gen)
	    ( (min::uns32) min::GEN_LIST_AUX << 24 );
#   elif MIN_IS_LOOSE
	const min::gen LIST_END = (min::gen)
	    ( (min::uns64) min::GEN_LIST_AUX << 40 );
	const min::gen EMPTY_SUBLIST = (min::gen)
	    ( (min::uns64) min::GEN_LIST_AUX << 40 );
#   endif

    bool use_object_aux_stubs;

    // We must declare these before we make them
    // friends.

    min::gen start_hash
            ( min::unprotected::list_pointer & lp,
	      unsigned index );
    min::gen start_vector
            ( min::unprotected::list_pointer & lp,
	      unsigned index );
    min::gen start_copy
            ( min::unprotected::list_pointer & lp,
	      min::unprotected::list_pointer & lp2 );
    min::gen next
    	    ( min::unprotected::list_pointer & lp );
    min::gen current
    	    ( min::unprotected::list_pointer & lp );
    min::gen start_sublist
    	    ( min::unprotected::list_pointer & lp );
    void insert_reserve
    	    ( min::unprotected::list_pointer & lp,
	      unsigned insertions,
	      unsigned elements = 0,
	      bool use_object_aux_stubs =
	          min::use_object_aux_stubs );
    void insert_before
    	    ( min::unprotected::list_pointer & lp,
	      const min::gen * p, unsigned n );
    void insert_after
    	    ( min::unprotected::list_pointer & lp,
	      const min::gen * p, unsigned n );
}

namespace min { namespace unprotected {

    // Internal functions: see min.cc.
    //
    unsigned allocate_aux_list
        ( min::unprotected::list_pointer & lp,
	  unsigned n );

    void allocate_stub_list
	( min::stub * & first,
	  min::stub * & last,
	  int type,
	  const min::gen * p, unsigned n,
	  min::uns64 end );

    class list_pointer {

    public:

        list_pointer ( min::stub * s )
	{
	    int t = min::type_of ( s );
	    MIN_ASSERT (    t == min::SHORT_OBJ
	    	         || t == min::LONG_OBJ );
	    this->s = s;

	    // An object may be converted from short to
	    // long by any relocation, so we cannot
	    // compute so/lo here.
	    so = NULL;
	    lo = NULL;

	    base = NULL;
	    current = 0;
	    current_index = previous_index = 0;
	    reserved_insertions = 0;
	    reserved_elements = 0;

#	    if MIN_USES_OBJECT_AUX_STUBS
		current_stub = previous_stub = NULL;
#	    endif
	}

        list_pointer ( min::gen v )
	{
	    new (this)
	        list_pointer ( min::stub_of ( v ) );
	}

    private:

    	min::stub * s;
	    // Stub of object.
	min::unprotected::short_obj * so;
	min::unprotected::long_obj * lo;
	    // After start, just one of these is
	    // non-NULL.

	min::gen * base;
	    // base of body vector.

	min::gen current;
	    // Value of current element, as returned by
	    // the min::current function.

	// Current element indices and stub pointers are
	// as follows:
	//
	//   if current_index != 0:
	//	base[current_index] is current element
	//   if current_stub != NULL:
	//	MUP::gen_of ( current_stub ) is current
	//	element
	//   if previous_index != 0:
	//	base[previous_index] is pointer to the
	//	current element or aux stub
	//   if previous_stub != NULL:
	//	MUP::control_of ( previous_stub ) is
	//	pointer to current element or aux stub
	//   if current_index == 0 and current_stub == 0
	//	and previous_index != 0:
	//	previous_index < unused_area_offset is
	//	list header of 1 element list and
	//	pointer is at end of the list after the
	//	1 element
	//   if current_index == 0 and current_stub == 0
	//	and previous_stub != 0:
	//	MUP::control_of ( previous_stub ) is
	//	min::LIST_END and pointer points at
	//	this.
	//
	unsigned current_index;
	unsigned previous_index;
#	if MIN_USES_OBJECT_AUX_STUBS
	    min::stub * current_stub;
	    min::stub * previous_stub;
#	endif

#	if MIN_USES_OBJECT_AUX_STUBS
	    bool use_object_aux_stubs;
		// True if list auxiliary stubs are to
		// be used for insertions if space in
		// the object runs out.
#	endif

	unsigned reserved_insertions;
	unsigned reserved_elements;
	    // Set by insert_resert and decremented by
	    // insert_{before,after}.

	friend unsigned allocate_aux_list
		( min::unprotected::list_pointer & lp,
		  unsigned n );

        friend void allocate_stub_list
		( min::stub * & first,
		  min::stub * & last,
		  int type,
		  const min::gen * p, unsigned n,
		  min::uns64 end );

	friend min::gen min::start_hash
		( min::unprotected::list_pointer & lp,
		  unsigned index );
	friend min::gen min::start_vector
		( min::unprotected::list_pointer & lp,
		  unsigned index );
	friend min::gen min::start_copy
		( min::unprotected::list_pointer & lp,
		  min::unprotected::list_pointer & lp2
		);

	friend min::gen min::next
		( min::unprotected::list_pointer & lp );
	friend min::gen min::current
		( min::unprotected::list_pointer & lp );
	friend min::gen min::start_sublist
		( min::unprotected::list_pointer & lp );

	friend void min::insert_reserve
		( min::unprotected::list_pointer & lp,
		  unsigned insertions,
		  unsigned elements,
		  bool use_object_aux_stubs );
	friend void min::insert_before
		( min::unprotected::list_pointer & lp,
		  const min::gen * p, unsigned n );
	friend void min::insert_after
		( min::unprotected::list_pointer & lp,
		  const min::gen * p, unsigned n );

        void relocate ( )
	{
	    int t = min::type_of ( s );
	    if ( t == min::SHORT_OBJ )
	    {
	        so = min::unprotected::
		     short_obj_of ( s );
		base = (min::gen *) so;
	    }
	    else if ( t == min::LONG_OBJ )
	    {
	        lo = min::unprotected::
		     long_obj_of ( s );
		base = (min::gen *) lo;
	    }
	    else
	    {
		MIN_ASSERT ( ! is_deallocated ( s ) );
	        MIN_ASSERT ( ! "s is not an object" );
	    }
	    current = 0;
	    current_index = previous_index = 0;
	    reserved_insertions = 0;
	    reserved_elements = 0;

#	    if MIN_USES_OBJECT_AUX_STUBS
		current_stub = previous_stub = NULL;
#	    endif
	}

	// Set current_index and current and then do
	// fowarding while is a list pointer.  Sets
	// current_stub = NULL.  Does not set previous_
	// index or previous_stub if there is no
	// forwarding.
	//
	min::gen forward ( unsigned index )
	{
	    current_index = index;
#           if MIN_USES_OBJECT_AUX_STUBS
		current_stub = NULL;
#	    endif
	    while ( true )
	    {
		current = base[current_index];
		if ( ! min::is_list_aux ( current ) )
		{
#	            if MIN_USES_OBJECT_AUX_STUBS
			if ( min::is_stub ( current ) )
			{
			    min::stub * s =
			        min::stub_of
					( current );
			    if (    min::type_of ( s )
			         == min::LIST_AUX )
			    {
				previous_index =
				    current_index;
				previous_stub = NULL;
				current_index = 0;
				current_stub = s;
				return
				  current =
				      min::unprotected::
				           gen_of ( s );
			    }
			}
#	            endif
		    return current;
		}
		else if ( current == min::LIST_END )
		    return current;
		else
		{
		    previous_index = current_index;
#	            if MIN_USES_OBJECT_AUX_STUBS
			previous_stub = NULL;
#	            endif
		    current_index =
		        min::list_aux_of ( current );
		}
	    }
	}
    };
} }

namespace min {

    inline min::gen start_hash
            ( min::unprotected::list_pointer & lp,
	      unsigned index )
    {
	lp.relocate();
        if ( lp.so )
	{
	    index +=   sizeof ( min::unprotected::
	                             short_obj )
	             / sizeof ( min::gen );
	    MIN_ASSERT
	        (   index
		  < min::unprotected::
		         flags_to_offset (lp.so->flags)
		);
	}
	else
	{
	    index +=   sizeof ( min::unprotected::
	                             long_obj )
	             / sizeof ( min::gen );
	    MIN_ASSERT
	        (   index
                  < min::unprotected::
		         flags_to_offset (lp.lo->flags)
		);
	}
	return lp.forward ( index );
    }
    inline min::gen start_vector
            ( min::unprotected::list_pointer & lp,
	      unsigned index )
    {
	lp.relocate();
        if ( lp.so )
	{
	    index += min::unprotected::
			flags_to_offset (lp.so->flags);
	    MIN_ASSERT
	        ( index < lp.so->unused_area_offset );
	}
	else
	{
	    index += min::unprotected::
			flags_to_offset (lp.lo->flags);
	    MIN_ASSERT
	        ( index < lp.lo->unused_area_offset );
	}
	return lp.forward ( index );
    }
    inline min::gen start_copy
            ( min::unprotected::list_pointer & lp,
	      min::unprotected::list_pointer & lp2 )
    {
        MIN_ASSERT ( lp.s == lp2.s );
	lp.lo = lp2.lo;
	lp.so = lp2.so;
	lp.current_index = lp2.current_index;
	lp.previous_index = lp2.previous_index;
#       if MIN_USES_OBJECT_AUX_STUBS
	    lp.current_stub = lp2.current_stub;
	    lp.previous_stub = lp2.previous_stub;
#       endif
	lp.reserved_insertions = 0;
	lp.reserved_elements = 0;
	return lp.current = lp2.current;
    }
    inline min::gen next
    	    ( min::unprotected::list_pointer & lp )
    {
        if ( lp.current == min::LIST_END )
	    return lp.current;

#       if MIN_USES_OBJECT_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
		lp.previous_stub = lp.current_stub;
		lp.previous_index = 0;

	        min::uns64 c =
		    min::unprotected::control_of
		    	( lp.current_stub );
		if ( c & min::unprotected::
			      STUB_POINTER )
		{
		    lp.current_stub =
		        min::unprotected::
			     stub_of_control ( c );
		    return
		        lp.current =
			    min::unprotected::
			         gen_of
				   ( lp.current_stub );
		}
		else
		{
		    unsigned index =
		        min::unprotected::
			     value_of_control ( c );
		    if ( index == 0 )
		    {
			lp.current_stub = NULL;
			return lp.current =
			    min::LIST_END;
		    }
		    else
		        return lp.forward ( index );
		}
	    }
	    lp.previous_stub = NULL;
#       endif

	lp.previous_index = lp.current_index;
	unsigned head_end;
	if ( lp.so )
	    head_end = lp.so->unused_area_offset;
	else
	    head_end = lp.lo->unused_area_offset;
	if ( lp.current_index < head_end )
	{
	    lp.current_index = 0;
	    return lp.current = min::LIST_END;
	}
	else
	    return lp.forward ( lp.current_index - 1 );
    }
    inline min::gen current
    	    ( min::unprotected::list_pointer & lp )
    {
    	return lp.current;
    }
    inline min::gen start_sublist
    	    ( min::unprotected::list_pointer & lp )
    {
#	if MIN_USES_OBJECT_AUX_STUBS
	    lp.previous_index = lp.current_index;
	    lp.previous_stub = lp.current_stub;
	    if ( min::is_sublist_aux ( lp.current ) )
		return lp.forward
		    ( min::unprotected::
			   sublist_aux_of
			       ( lp.current ) );
	    lp.current_stub =
	        min::stub_of ( lp.current );
	    lp.current_index = 0;
	    MIN_ASSERT
	        (    min::type_of ( lp.current_stub )
		  == min::SUBLIST_AUX );
	    return lp.current = min::unprotected::
			gen_of ( lp.current_stub );
#	else
	    lp.previous_index = lp.current_index;
	    return lp.forward
	        ( min:: sublist_aux_of ( lp.current ) );
#	endif
    }
    inline void insert_reserve
    	    ( min::unprotected::list_pointer & lp,
	      unsigned insertions,
	      unsigned elements,
	      bool use_object_aux_stubs )
    {
        if ( elements = 0 ) elements = insertions;
	MIN_ASSERT ( insertions <= elements );

	unsigned unused_area_size;
	if ( lp.so )
	    unused_area_size =
	        min::unused_area_size_of ( lp.so );
	else
	    unused_area_size =
	        min::unused_area_size_of ( lp.lo );
	if (      unused_area_size
	        < 2 * insertions + elements
#	    if MIN_USES_OBJECT_AUX_STUBS
	     && (    ! use_object_aux_stubs
	          ||   min::unprotected::
			    number_of_free_stubs
		     < elements )
#	    endif
	   )
	    min::unprotected::insert_reserve
	        ( lp, insertions, elements,
		  use_object_aux_stubs );
	else
	{
	    lp.reserved_insertions = insertions;
	    lp.reserved_elements = elements;
#	    if MIN_USES_OBJECT_AUX_STUBS
		lp.use_object_aux_stubs =
		   use_object_aux_stubs;
#	    endif
	}
    }
}

// Object Attribute Level
// ------ --------- -----

namespace min { namespace unprotected {
} }

namespace min {
}

// Numbers
// -------

namespace min { namespace unprotected {
} }

namespace min {
}

// TBD
// ---

namespace min {
}

# endif // MIN_H
