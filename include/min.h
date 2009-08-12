// MIN Language Interface
//
// File:	min.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Aug 12 07:48:07 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/08/12 16:24:59 $
//   $RCSfile: min.h,v $
//   $Revision: 1.181 $

// Table of Contents:
//
//	Setup
//	Parameter Computation and Checking
//	C++ Number Types
//	General Value Types and Data
//	Stub Types and Data
//	Internal Pointer Conversion Functions
//	General Value Constructor Functions
//	General Value Test Functions
//	General Value Read Functions
//	Control Values
//	Stub Functions
//	Process Interface
//	Allocator/Collector/Compactor Interface
//	Numbers
//	Strings
//	Labels
//	Name Functions
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level

// Namespaces:
//
//   			Abbreviation
//   Full Name		      Use and defining file.
//
//   min		min   Public protected min.h.
//   min::unprotected	MUP   Public unprotected min.h.
//   min::internal	MINT  Private min.h.
//   min::acc		MACC  Private min_acc.h.

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

#define MIN_ABORT(string) assert ( ! string )
#define MIN_REQUIRE(expr) assert ( expr )

// Parameter Computation and Checking
// --------- ----------- --- --------

namespace min { namespace internal {

    class initializer
    {
        public:

	initializer ( void );
	    // NOTE: This calls acc_initializer()
	    // defined below.
    };
    static initializer initializer_instance;

} }


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
	const unsigned GEN_INDEXED_AUX
	    = 0xF4;
	const unsigned GEN_INDEX
	    = 0xF5;
	const unsigned GEN_CONTROL_CODE
	    = 0xF6;
	const unsigned GEN_SPECIAL
	    = 0xF7;
	const unsigned GEN_ILLEGAL
	    = 0xF8;  // First illegal subtype code.
	const unsigned GEN_UPPER
	    = 0xFF; // Largest subtype code.

	typedef uns32 unsgen;
	    // Unsigned type convertable to a min::gen
	    // value.
	const unsigned TSIZE = 8;
	const unsigned VSIZE = 24;
	    // Sized of type and value field in a
	    // min:gen.

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
	const unsigned GEN_INDEXED_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x14;
	const unsigned GEN_INDEX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x15;
	const unsigned GEN_CONTROL_CODE
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x16;
	const unsigned GEN_SPECIAL
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x17;
	const unsigned GEN_ILLEGAL
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x18;
	    // First illegal subtype code.
	const unsigned GEN_UPPER
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1F;
	    // Largest subtype code.

	typedef uns64 unsgen;
	    // Unsigned type convertable to a min::gen
	    // value.
	const unsigned TSIZE = 24;
	const unsigned VSIZE = 40;
	    // Sized of type and value field in a
	    // min:gen.

#   endif

#   define MIN_NEW_SPECIAL_GEN(i) \
	( min::gen ( (   min::unsgen \
	                      ( min::GEN_SPECIAL ) \
	              << min::VSIZE ) \
		     + ( i ) ) )

    // MIN special values must have indices in the
    // range 2**24 - 256 .. 2**24 - 1.
    //
    const min::gen MISSING =
	MIN_NEW_SPECIAL_GEN ( 0xFFFFFF );
    const min::gen NONE =
	MIN_NEW_SPECIAL_GEN ( 0xFFFFFE );
    const min::gen ANY =
	MIN_NEW_SPECIAL_GEN ( 0xFFFFFD );
    const min::gen MULTI_VALUED =
	MIN_NEW_SPECIAL_GEN ( 0xFFFFFC );
    const min::gen UNDEFINED =
	MIN_NEW_SPECIAL_GEN ( 0xFFFFFB );
    const min::gen SUCCESS =
	MIN_NEW_SPECIAL_GEN ( 0xFFFFFA );
    const min::gen FAILURE =
	MIN_NEW_SPECIAL_GEN ( 0xFFFFF9 );
}

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
    const int HASHTABLE_AUX		= -4;

    namespace unprotected {
	// Non-acc flags for uncollectible controls.
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
	    char c8[8];
	} c; // control
    };
}

// Internal Pointer Conversion Functions
// -------- ------- ---------- ---------

namespace min { namespace internal {

    // We need to be able to convert unsigned integers
    // to pointers and vice versa.

#   if MIN_POINTER_BITS <= 32
	typedef uns32 pointer_uns;

	// For the 32 bit pointer size there is no point
	// in having a non-zero stub base.
#	ifndef MIN_STUB_BASE
#	    define MIN_STUB_BASE 0
#	endif

#   else
	typedef uns64 pointer_uns;
#   endif

    // Address of first stub (stub with index 0),
    // equals address of the `null_stub', whose address
    // is used in place of NULL to end lists of stubs
    // (because NULL cannot be represented by some
    // stub address representation schemes).
    //
    // WARNING: null_stub may not be an actual stub,
    // e.g., its address may be NULL, and it may not
    // be in accessible memory.
    //
#   ifdef MIN_STUB_BASE
	const min::internal::pointer_uns stub_base =
	    MIN_STUB_BASE;
	min::stub * const null_stub =
	    (min::stub *) stub_base;
#   else
	extern min::internal::pointer_uns stub_base;
	extern min::stub * null_stub;
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

    // Given a ref to an uns64 we need to convert it to
    // a ref to a pointer.

#   if MIN_POINTER_BITS <= 32
	inline void * & uns64_ref_to_pointer_ref
	    ( min::uns64 & v )
	{
	    return ( (void **) & v )[MIN_IS_BIG_ENDIAN];
	}
#   else
	inline void * & uns64_ref_to_pointer_ref
	    ( min::uns64 & v )
	{
	    return * (void **) & v;
	}
#   endif

#   if MIN_IS_COMPACT

	inline min::stub * general_uns32_to_stub
		( min::uns32 v )
	{
	    min::internal::pointer_uns p =
		(min::internal::pointer_uns) v;
#           if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
               <= 0xDFFFFFFFull
		return ( min::stub * ) p;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
                 <= 0xDFFFFFFFull
		return (min::stub *) ( p + stub_base );
#           elif MIN_MAX_STUB_INDEX <= 0xDFFFFFFFull
		return (min::stub *) stub_base + p;
#           else
#	        error   MIN_MAX_STUB_INDEX \
                      > 0xDFFFFFFF
#           endif
	}
	inline min::uns32 general_stub_to_uns32
		( min::stub * s )
	{
#           if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
               <= 0xDFFFFFFFull
	        return (min::internal::pointer_uns) s;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
                 <= 0xDFFFFFFFull
	        return   (min::internal::pointer_uns) s
		       - stub_base;
#           elif MIN_MAX_STUB_INDEX <= 0xDFFFFFFFull
	        return s - (min::stub *) stub_base;
#           else
#	        error   MIN_MAX_STUB_INDEX \
                      > 0xDFFFFFFF
#           endif
	}
#   elif MIN_IS_LOOSE
        inline min::stub * general_uns64_to_stub
	        ( min::uns64 v )
        {
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (v & ( ( min::uns64(1) << 44 ) - 1 ) );
#           if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
               <= 0xFFFFFFFFFFFull
	        return ( min::stub * ) p;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
                 <= 0xFFFFFFFFFFFull
	        return (min::stub *) ( p + stub_base );
#           elif MIN_MAX_STUB_INDEX <= 0xFFFFFFFFFFFull
	        return (min::stub *) stub_base + p;
#           else
#	        error   MIN_MAX_STUB_INDEX \
                      > 0xFFFFFFFFFFFull
#           endif
        }

        inline min::uns64 general_stub_to_uns64
	        ( min::stub * s )
        {
#           if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
               <= 0xFFFFFFFFFFFull
	        return (min::internal::pointer_uns) s;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
                 <= 0xFFFFFFFFFFFull
	        return   (min::internal::pointer_uns) s
	               - stub_base;
#           elif MIN_MAX_STUB_INDEX <= 0xFFFFFFFFFFFull
	        return s - (min::stub *) stub_base;
#           else
#	        error   MIN_MAX_STUB_INDEX \
                      > 0xFFFFFFFFFFFull
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

namespace min {

    // Helper function.

    inline unsigned strnlen
	    ( const char * p, unsigned n )
    {
        const char * q = p;
	while ( * q && n > 0 ) ++ q, -- n;
	return q - p;
    }
}

namespace min { namespace unprotected {

    // MUP:: constructors

#   if MIN_IS_COMPACT
	inline min::gen new_gen ( min::stub * s )
	{
	    return (min::gen)
	        internal::general_stub_to_uns32 ( s );
	}
	inline min::gen new_direct_int_gen ( int v )
	{
	    return (min::gen)
	           (  (uns32) v
		    + ( GEN_DIRECT_INT << VSIZE )
		    + ( 1 << 27 ) );
	}
	// Unimplemented for COMPACT:
	//  min::gen new_direct_float_gen ( float64 v )
	inline min::gen new_direct_str_gen
		( const char * p )
	{
	    uns32 v = (uns32) GEN_DIRECT_STR << VSIZE;
	    char * s = ( (char *) & v )
		     + MIN_IS_BIG_ENDIAN;
	       ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ );
	    return (min::gen) v;
	}
	inline min::gen new_direct_str_gen
		( const char * p, unsigned n )
	{
	    uns32 v = (uns32) GEN_DIRECT_STR << VSIZE;
	    char * s = ( (char *) & v )
		     + MIN_IS_BIG_ENDIAN;
	       ( n >= 1 && ( * s ++ = * p ++ ) )
	    && ( n >= 2 && ( * s ++ = * p ++ ) )
	    && ( n >= 3 && ( * s ++ = * p ++ ) );
	    return (min::gen) v;
	}

#   elif MIN_IS_LOOSE
	inline min::gen new_gen ( min::stub * s )
	{
	    return (min::gen)
		   (     internal
		       ::general_stub_to_uns64 ( s )
		     + ( (uns64) GEN_STUB << VSIZE )  );
	}
	// Unimplemented for LOOSE:
	//   min::gen new_direct_int_gen ( int v )
	inline min::gen new_direct_float_gen
		( float64 v )
	{
	    // Use of union helps the optimizer deal
	    // with aliasing.
	    //
	    union {
	        float64 f;
		min::gen g;
	    } u;
	    u.f = v;
	    return u.g;
	}
	inline min::gen new_direct_str_gen
		( const char * p )
	{
	    uns64 v = (uns64) GEN_DIRECT_STR << VSIZE;
	    char * s = ( (char *) & v )
	             + 3 * MIN_IS_BIG_ENDIAN;
	       ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ )
	    && ( * s ++ = * p ++ );
	    return (min::gen) v;
	}
	inline min::gen new_direct_str_gen
		( const char * p, unsigned n )
	{
	    uns64 v = (uns64) GEN_DIRECT_STR << VSIZE;
	    char * s = ( (char *) & v )
	             + 3 * MIN_IS_BIG_ENDIAN;
	       ( n >= 1 && ( * s ++ = * p ++ ) )
	    && ( n >= 2 && ( * s ++ = * p ++ ) )
	    && ( n >= 3 && ( * s ++ = * p ++ ) )
	    && ( n >= 4 && ( * s ++ = * p ++ ) )
	    && ( n >= 5 && ( * s ++ = * p ++ ) );
	    return (min::gen) v;
	}
#   endif

    inline min::gen new_list_aux_gen ( unsgen p )
    {
	return (min::gen)
	       (   p
		 + ( (unsgen) GEN_LIST_AUX
		     << VSIZE ) );
    }
    inline min::gen new_sublist_aux_gen
	    ( unsgen p )
    {
	return (min::gen)
	       (   p
		 + ( (unsgen) GEN_SUBLIST_AUX
		     << VSIZE ) );
    }
    inline min::gen new_indirect_pair_aux_gen
	    ( unsgen p )
    {
	return (min::gen)
	       (   p
		 + ( (unsgen) GEN_INDIRECT_PAIR_AUX
		     << VSIZE ) );
    }
    inline min::gen new_indexed_aux_gen
	    ( unsigned p, unsigned i )
    {
	return (min::gen)
	       (   ( (unsgen) p << (VSIZE/2) ) + i
		 + ( (unsgen)
		     GEN_INDEXED_AUX << VSIZE ) );
    }
    inline min::gen new_index_gen ( unsgen i )
    {
	return (min::gen)
	       ( i + ( (unsgen) GEN_INDEX << VSIZE ) );
    }
    inline min::gen new_control_code_gen
	    ( unsgen c )
    {
	return (min::gen)
	       (   c
		 + ( (unsgen) GEN_CONTROL_CODE
		     << VSIZE ) );
    }
    inline min::gen new_special_gen
	    ( unsgen i )
    {
	return (min::gen)
	       (   i
		 + ( (unsgen) GEN_SPECIAL
		     << VSIZE ) );
    }
    inline min::gen renew_gen
	    ( min::gen v, min::unsgen p )
    {
	return (min::gen)
	    ( ( (unsgen ) v & ( (unsgen) -1 << VSIZE ) )
	      + p );
    }
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
    inline min::gen new_direct_str_gen
	    ( const char * p, unsigned n )
    {
#       if MIN_IS_COMPACT
	    MIN_ASSERT ( strnlen ( p, n ) <= 3 );
#	elif MIN_IS_LOOSE
	    MIN_ASSERT ( strnlen ( p, n ) <= 5 );
#	endif
	return unprotected::new_direct_str_gen ( p );
    }
    inline min::gen new_list_aux_gen ( unsgen p )
    {
	MIN_ASSERT ( p < (unsgen) 1 << VSIZE );
	return unprotected::new_list_aux_gen ( p );
    }
    inline min::gen new_sublist_aux_gen
	    ( unsgen p )
    {
	MIN_ASSERT ( p < (unsgen) 1 << VSIZE );
	return unprotected::new_sublist_aux_gen ( p );
    }
    inline min::gen new_indirect_pair_aux_gen
	    ( unsgen p )
    {
	MIN_ASSERT ( p < (unsgen) 1 << VSIZE );
	return unprotected::new_indirect_pair_aux_gen
			( p );
    }
    inline min::gen new_indexed_aux_gen
	    ( unsigned p, unsigned i )
    {
	MIN_ASSERT ( p < 1 << ( VSIZE / 2 ) );
	MIN_ASSERT ( i < 1 << ( VSIZE / 2 ) );
	return unprotected::
	       new_indexed_aux_gen ( p, i );
    }
    inline min::gen new_index_gen ( unsgen i )
    {
	MIN_ASSERT ( i < (unsgen) 1 << VSIZE );
	return unprotected::new_index_gen ( i );
    }
    inline min::gen new_control_code_gen
	    ( unsgen c )
    {
	MIN_ASSERT ( c < (unsgen) 1 << VSIZE );
	return unprotected::new_control_code_gen ( c );
    }
    inline min::gen new_special_gen
	    ( unsgen i )
    {
	MIN_ASSERT ( i < (unsgen) 1 << VSIZE );
	return unprotected::new_special_gen ( i );
    }
}

// General Value Test Functions
// ------- ----- ---- ---------

namespace min {

#   if MIN_IS_COMPACT
	inline bool is_stub ( min::gen v )
	{
	    return ( v < ( GEN_DIRECT_INT << VSIZE ) );
	}
	// Unimplemented for COMPACT:
	//  bool is_direct_float ( min::gen v )
	inline bool is_direct_int ( min::gen v )
	{
	    return ( v >> 28 == GEN_DIRECT_INT >> 4 );
	}
	inline unsigned gen_subtype_of ( min::gen v )
	{
	    v = (min::uns32) v >> VSIZE;
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
	        ( (uns64) v & 0x7FFFE00000000000ull )
	        != ( (uns64) MIN_FLOAT64_SIGNALLING_NAN
		     << VSIZE );
	}
	// Unimplemented for LOOSE:
	//   bool is_direct_int ( min::gen v )
	inline unsigned gen_subtype_of ( min::gen v )
	{
	    v = (unsgen) v >> VSIZE;
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

    inline bool is_direct_str ( min::gen v )
    {
	return
	    ( (unsgen) v >> VSIZE == GEN_DIRECT_STR );
    }
    inline bool is_list_aux ( min::gen v )
    {
	return ( (unsgen) v >> VSIZE == GEN_LIST_AUX );
    }
    inline bool is_sublist_aux ( min::gen v )
    {
	return
	    ( (unsgen) v >> VSIZE == GEN_SUBLIST_AUX );
    }
    inline bool is_indirect_pair_aux ( min::gen v )
    {
	return
	    (    (unsgen) v >> VSIZE
	      == GEN_INDIRECT_PAIR_AUX );
    }
    inline bool is_indexed_aux ( min::gen v )
    {
	return
	    ( (unsgen) v >> VSIZE == GEN_INDEXED_AUX );
    }
    inline bool is_index ( min::gen v )
    {
	return ( (unsgen) v >> VSIZE == GEN_INDEX );
    }
    inline bool is_control_code ( min::gen v )
    {
	return
	    ( (unsgen) v >> VSIZE == GEN_CONTROL_CODE );
    }
    inline bool is_special ( min::gen v )
    {
	return ( (unsgen) v >> VSIZE == GEN_SPECIAL );
    }
}

// General Value Read Functions
// ------- ----- ---- ---------

namespace min { namespace unprotected {

    // MUP:: functions.

#   if MIN_IS_COMPACT
	inline min::stub * stub_of ( min::gen v )
	{
	    return   internal
	           ::general_uns32_to_stub ( v );
	}
	// Unimplemented for COMPACT:
	//   float64 direct_float_of ( min::gen v )
	inline int direct_int_of ( min::gen v )
	{
	    return (int32)
	           ( v - ( GEN_DIRECT_INT << VSIZE )
		       - ( 1 << 27 ) );
	}
	inline uns64 direct_str_of ( min::gen v )
	{
#	    if MIN_IS_BIG_ENDIAN
		return ( uns64 ( v ) << VSIZE );
#	    elif MIN_IS_LITTLE_ENDIAN
		return ( uns64 ( v & 0xFFFFFF ) );
#	    endif
	}
#   elif MIN_IS_LOOSE
	inline min::stub * stub_of ( min::gen v )
	{
	    return internal::general_uns64_to_stub
	    		( v & 0xFFFFFFFFFFull );
	}
	inline float64 direct_float_of ( min::gen v )
	{
	    // Use of union helps the optimizer deal
	    // with aliasing.
	    //
	    union {
	        float64 f;
		min::gen g;
	    } u;
	    u.g = v;
	    return u.f;
	}
	// Unimplemented for LOOSE:
	//   int direct_int_of ( min::gen v )
	inline uns64 direct_str_of ( min::gen v )
	{
#	    if MIN_IS_BIG_ENDIAN
		return ( v << VSIZE );
#	    elif MIN_IS_LITTLE_ENDIAN
		return ( v & 0xFFFFFFFFFFull );
#	    endif
	}
#   endif

    namespace internal {
	const unsgen VMASK =
	    ( (unsgen) 1 << VSIZE ) - 1;
	const unsgen VHALFMASK =
	    ( (unsgen) 1 << ( VSIZE / 2 ) ) - 1;
    }

    inline unsgen list_aux_of ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
    }
    inline unsgen sublist_aux_of ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
    }
    inline unsgen indirect_pair_aux_of
	    ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
    }
    inline unsigned indexed_aux_of ( min::gen v )
    {
	return   ( (unsgen) v >> ( VSIZE / 2 ) )
	       & internal::VHALFMASK;
    }
    inline unsigned indexed_index_of ( min::gen v )
    {
	return (unsgen) v & internal::VHALFMASK;
    }
    inline unsgen index_of ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
    }
    inline unsgen control_code_of ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
    }
    inline unsgen special_index_of ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
    }
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
    inline unsgen list_aux_of ( min::gen v )
    {
	MIN_ASSERT ( is_list_aux ( v ) );
	return unprotected::list_aux_of ( v );
    }
    inline unsgen sublist_aux_of ( min::gen v )
    {
	MIN_ASSERT ( is_sublist_aux ( v ) );
	return unprotected::sublist_aux_of ( v );
    }
    inline unsgen indirect_pair_aux_of
	    ( min::gen v )
    {
	MIN_ASSERT ( is_indirect_pair_aux ( v ) );
	return unprotected::indirect_pair_aux_of ( v );
    }
    inline unsigned indexed_aux_of ( min::gen v )
    {
	MIN_ASSERT ( is_indexed_aux ( v ) );
	return unprotected::indexed_aux_of ( v );
    }
    inline unsigned indexed_index_of ( min::gen v )
    {
	MIN_ASSERT ( is_indexed_aux ( v ) );
	return unprotected::indexed_index_of ( v );
    }
    inline unsgen index_of ( min::gen v )
    {
	MIN_ASSERT ( is_index ( v ) );
	return unprotected::index_of ( v );
    }
    inline unsgen control_code_of ( min::gen v )
    {
	MIN_ASSERT ( is_control_code ( v ) );
	return unprotected::control_code_of ( v );
    }
    inline unsgen special_index_of ( min::gen v )
    {
	MIN_ASSERT ( is_special ( v ) );
	return unprotected::special_index_of ( v );
    }
}

// Control Values
// ------- ------

// ACC control value layout:
//
//	Bits		Use
//
//	0 .. m-1		Stub absolute or relative
//			address or stub index.
//
//	m .. 55		ACC flag bits
//
//	56 .. 63	Type code.
//
// Non-ACC control value layout:
//
//	Bits		Use
//
//	0 .. 47		Stub absolute or relative
//			address or stub index.
//
//	48 .. 55	Flag bits
//
//	56 .. 63	Type code
//
//	48 .. 63	Locator (overlaps flag and type)

// ACC CONTROL MASK is 2**(56 - MIN_ACC_FLAG_BITS) - 1.
// CONTROL MASK is 2**48 - 1.
//
# define MIN_CONTROL_VALUE_MASK 0xFFFFFFFFFFFFull
# define MIN_ACC_CONTROL_VALUE_MASK \
    ( 0xFFFFFFFFFFFFFFull >> MIN_ACC_FLAG_BITS )

namespace min { namespace internal {

    const min::uns64 TYPE_MASK =
	~ ( ( uns64(1) << 56 ) - 1 );

    const min::uns64 LOCATOR_MASK =
	~ ( ( uns64(1) << 48 ) - 1 );
} }

namespace min { namespace unprotected {

    inline int type_of_control ( min::uns64 c )
    {
	return int ( min::int64 ( c ) >> 56 );
    }

    inline int locator_of_control ( min::uns64 c )
    {
	return int ( min::int64 ( c ) >> 48 );
    }

    inline unsigned value_of_control
	    ( min::uns64 c )
    {
	return unsigned
	  ( c & MIN_CONTROL_VALUE_MASK );
    }

    inline min::uns64 new_control_with_type
	    ( int type_code, min::uns64 v,
	      min::uns64 flags = 0 )
    {
	return ( min::uns64 ( type_code ) << 56 )
	       |
	       v
	       |
	       flags;
    }

    inline min::uns64 new_control_with_locator
	    ( int locator, min::uns64 v )
    {
	return ( min::uns64 ( locator ) << 48 )
	       |
	       v;
    }

    inline min::uns64 renew_control_type
	    ( min::uns64 c, int type )
    {
	return ( c & ~ min::internal::TYPE_MASK )
	       | ( min::uns64 (type) << 56 );
    }

    inline min::uns64 renew_control_locator
	    ( min::uns64 c, int locator )
    {
	return ( c & ~ min::internal::LOCATOR_MASK )
	       | ( min::uns64 (locator) << 48 );
    }

    inline min::uns64 renew_control_value
	    ( min::uns64 c, min::uns64 v )
    {
	return ( c & ~ MIN_CONTROL_VALUE_MASK )
	       | v;
    }

#   if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
       <= MIN_CONTROL_VALUE_MASK

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    return ( min::stub * )
	           (min::internal::pointer_uns)
	           (c & MIN_CONTROL_VALUE_MASK );
	}

	inline min::uns64 new_control_with_type
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
		   (min::internal::pointer_uns) s
		   |
		   flags;
	}

	inline min::uns64 new_control_with_locator
		( int locator, min::stub * s )
	{
	    return ( min::uns64 ( locator ) << 48 )
		   |
		   (min::internal::pointer_uns) s;
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ MIN_CONTROL_VALUE_MASK )
		   | (min::internal::pointer_uns) s;
	}

#   elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
         <= MIN_CONTROL_VALUE_MASK

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (c & MIN_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           ( p + min::internal::stub_base );
	}

	inline min::uns64 new_control_with_type
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
	           (   (min::internal::pointer_uns) s
	             - min::internal::stub_base )
		   |
		   flags;
	}

	inline min::uns64 new_control_with_locator
		( int locator, min::stub * s )
	{
	    return ( min::uns64 ( locator ) << 48 )
		   |
	           (   (min::internal::pointer_uns) s
	             - min::internal::stub_base );
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ MIN_CONTROL_VALUE_MASK )
		   | (   (min::internal::pointer_uns) s
		       - min::internal::stub_base );
	}

#   elif    MIN_MAX_STUB_INDEX \
         <= MIN_CONTROL_VALUE_MASK

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (c & MIN_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           min::internal::stub_base + p;
	}

	inline min::uns64 new_control_with_type
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return   ( min::uns64 ( type_code ) << 56 )
	           | ( s - (min::stub *)
		           min::internal::stub_base )
		   | flags;
	}

	inline min::uns64 new_control_with_locator
		( int locator, min::stub * s )
	{
	    return   ( min::uns64 ( locator ) << 48 )
	           | ( s - (min::stub *)
		           min::internal::stub_base );
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return   ( c & ~ MIN_CONTROL_VALUE_MASK )
	           | ( s - (min::stub *)
		           min::internal::stub_base );
	}
#   else
#	error   MIN_MAX_STUB_INDEX \
              > MIN_CONTROL_VALUE_MASK
#   endif

#   if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
       <= MIN_ACC_CONTROL_VALUE_MASK

	inline min::stub * stub_of_acc_control
		( min::uns64 c )
	{
	    return ( min::stub * )
	           (min::internal::pointer_uns)
	           (c & MIN_ACC_CONTROL_VALUE_MASK );
	}

	inline min::uns64 new_acc_control
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
		   (min::internal::pointer_uns) s
		   |
		   flags;
	}

	inline min::uns64 renew_acc_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ MIN_ACC_CONTROL_VALUE_MASK )
		   | (min::internal::pointer_uns) s;
	}

#   elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
         <= MIN_ACC_CONTROL_VALUE_MASK

	inline min::stub * stub_of_acc_control
		( min::uns64 c )
	{
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (c & MIN_ACC_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           ( p + min::internal::stub_base );
	}

	inline min::uns64 new_acc_control
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return   ( min::uns64 ( type_code ) << 56 )
		   | (   (min::internal::pointer_uns) s
	               - min::internal::stub_base )
		   | flags;
	}

	inline min::uns64 renew_acc_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ MIN_ACC_CONTROL_VALUE_MASK )
		 | (   (min::internal::pointer_uns) s
		     - min::internal::stub_base );
	}

#   elif    MIN_MAX_STUB_INDEX \
         <= MIN_ACC_CONTROL_VALUE_MASK

	inline min::stub * stub_of_acc_control
		( min::uns64 c )
	{
	    min::internal::pointer_uns p =
	       (min::internal::pointer_uns)
	       (c & MIN_ACC_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           min::internal::stub_base + p;
	}

	inline min::uns64 new_acc_control
		( int type_code, min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return   ( min::uns64 ( type_code ) << 56 )
	           | ( s - (min::stub *)
		           min::internal::stub_base )
		   | flags;
	}

	inline min::uns64 renew_acc_control_stub
		( min::uns64 c, min::stub * s )
	{
	    return ( c & ~ MIN_ACC_CONTROL_VALUE_MASK )
	         | ( s - (min::stub *)
		         min::internal::stub_base );
	}
#   else
#	error   MIN_MAX_STUB_INDEX \
              > MIN_ACC_CONTROL_VALUE_MASK
#   endif

} }

// Stub Functions
// ---- ---------

namespace min {

    inline int type_of ( min::stub * s )
    {
        return s->c.i8[7*MIN_IS_LITTLE_ENDIAN];
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

        inline void * & pointer_ref_of ( min::stub * s )
	{
	    return min::internal::
	           uns64_ref_to_pointer_ref
		       ( s->v.u64 );
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
	    s->c.i8[7*MIN_IS_LITTLE_ENDIAN] = type;
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

    extern bool interrupt_flag;
	// On if interrupt should occur.

    extern bool relocated_flag;
	// On if bodies have been relocated.

    struct process_control {

        // Pointers to stacks: TBD.

    };

    extern process_control * current_process;

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

// Allocator/Collector/Compactor Interface
// ----------------------------- ---------

namespace min {

    template < unsigned len > struct protect
    {
        protect * previous;
	unsigned length;
	min::gen values[len];

	static protect * last = NULL;

	protect ( void )
	{
	    this->length = len;
	    previous = last;
	    last = this;
	    memset ( values, 0, sizeof ( values ) );
	}

	~ protect ( void )
	{
	    last = previous;
	}

	min::gen & value ( unsigned i )
	{
	    assert ( i < len );
	    return values[i];
	}
    };
}

namespace min { namespace internal {

    // Function called by the the initializer (see
    // above) to initialize the acc.
    //
    void acc_initializer ( void );

    // Number of free stubs that can be allocated with-
    // out requiring a call to acc_expand_stub_free_
    // list.
    //
    extern unsigned number_of_free_stubs;

    // Out of line function to increase the number of
    // free stubs to be at least n.
    //
    void acc_expand_stub_free_list ( unsigned n );

    // Hash tables for atoms.  The elements of these
    // tables head a list of stubs chained together
    // by their stub control word stub pointers.  The
    // first stubs in each list are HASHTABLE_AUX
    // stubs whose value words point at the stubs of
    // ephemeral objects hashed to the list.  The last
    // stubs on each list are the stubs of non-ephemeral
    // garbage collectable objects hashed to the list.
    //
    // Xxx_hash_sizes are powers of 2, and each xxx_
    // hash_mask = xxx_hash_size - 1;
    //
    // When a hashed object is created it is ephemeral
    // and gets a HASHTABLE_AUX stub pointing at it
    // at the beginning of the hashtable list headed
    // by the hash table element indexed by the hashed
    // object's hash value masked by the xxx_hash_mask
    // value.

    extern min::stub ** str_hash;
    extern unsigned str_hash_size;
    extern unsigned str_hash_mask;

    extern min::stub ** num_hash;
    extern unsigned num_hash_size;
    extern unsigned num_hash_mask;

    extern min::stub ** lab_hash;
    extern unsigned lab_hash_size;
    extern unsigned lab_hash_mask;

    // The ACC Flag Bit layout is:
    //
    //	Bit		Use
    //
    //	0		Fixed Body Flag
    //
    //	1		Reserved
    //
    //	2, 4, 6, ...	Unmarked Flags
    //
    //	3, 5, 7, ...	Scavenged Flags

    // Collector flags.
    //
    const uns64 ACC_FLAG_MASK =
           ( (uns64(1) << MIN_ACC_FLAG_BITS) - 1 )
	<< ( 56 - MIN_ACC_FLAG_BITS );
    const uns64 ACC_SCAVENGED_MASK =
    	  ACC_FLAG_MASK
	& (    uns64(0xAAAAAAAAAA)
	    << ( 56 - MIN_ACC_FLAG_BITS ) + 1 );
    const uns64 ACC_UNMARKED_MASK =
        ACC_SCAVENGED_MASK >> 1;
    const uns64 ACC_FIXED_BODY_FLAG =
	( uns64(1) << ( 56 - MIN_ACC_FLAG_BITS ) );

    // acc_write_update ( s1, s2 ) checks whether
    //
    //		unmarked flags of *s2
    //		&
    //		scavenged flags of *s1
    //		&
    //		acc_stack_mask
    //
    // is non-zero, and if so, pushes first s1 and then
    // s2 into the acc_stack.  This call is made when
    // a pointer to s2 is stored in the s1 object, and
    // the acc_stackk is used by the collector to adjust
    // the marks it makes on objects.
    //
    // For efficiency, acc_stack_mask is an uns64 that
    // only has ON bits in the unmarked flag positions.
    // Then the unshifted control value of s2 and the
    // control value value of s1 left shifted by 1 can
    // be bitwise ANDed with the acc_stack_mask and the
    // result checked for zero.
    //
    // WARNING: only unmarked flag bits may be on in
    // MUP::acc_stack_mask.
    //
    extern min::uns64 acc_stack_mask;
    extern min::stub ** acc_stack;

    // Function executed whenever a pointer to stub s2
    // is stored in a datum with stub s1.  s1 is the
    // source of the written pointer and s2 is the
    // target.
    // 
    inline void acc_write_update
	    ( min::stub * s1, min::stub * s2 )
    {
        uns64 f = (    min::unprotected
	                  ::control_of ( s1 )
	            >> 1 )
	        & ( min::unprotected
		       ::control_of ( s2 ) )
		& min::internal::acc_stack_mask;

	if ( f != 0 )
	{
	    * acc_stack ++ = s1;
	    * acc_stack ++ = s2;
	}
    }

    // Function executed whenever the n general values
    // pointed at by p are stored in a datum with stub
    // s1, and the general values may contain stub
    // pointers.  This function calls acc_write_update
    // ( s1, s2 ) for every stub pointer s2 in one of
    // the general values.
    //
    inline void acc_write_update
	    ( min::stub * s1,
	      const min::gen * p, unsigned n )
    {
        while ( n -- )
	{
	    min::gen v = * p ++;
	    if ( min::is_stub ( v ) )
	        acc_write_update
		    ( s1, min::stub_of ( v ) );
	}
    }

    // Stub allocation is from a single list of stubs
    // chained together by the chain part of the stub
    // control.
    //
    // A pointer to the last allocated stub is maintain-
    // ed.  To allocate a new stub, this is updated to
    // the next stub on the list, if any.  Otherwise, if
    // there is no next stub, an out-of-line function,
    // acc_expand_stub_free_list, is called to add to
    // the end of the list.
    //
    // Unallocated stubs have stub type min::FREE, zero
    // stub control flags, and min:MISSING value.

    extern min::uns64 acc_new_stub_flags;

    // Pointer to the last allocated stub, which must
    // exist (it can be a dummy).
    //
    extern min::stub * last_allocated_stub;

    // Function to return the next free stub as a
    // garbage collectible stub.  The type is set to
    // min::FREE and may be changed to any garbage
    // collectible type.  The value is NOT set.  The ACC
    // flags are set to MUP::acc_new_stub_flags, and the
    // non-type part of the stub control is maintained
    // by the ACC.
    //
    inline min::stub * new_stub ( void )
    {
	if ( min::internal
	        ::number_of_free_stubs == 0 )
	    min::internal
	       ::acc_expand_stub_free_list ( 1 );
	-- min::internal::number_of_free_stubs;
	uns64 c = min::unprotected::control_of
			( last_allocated_stub );
	min::stub * s = min::unprotected
	                   ::stub_of_acc_control ( c );
	min::unprotected::set_flags_of
	    ( s, acc_new_stub_flags );
	return last_allocated_stub = s;
    }
    
    // Function to return the next free stub while
    // removing this stub from the ACC list of stubs.
    //
    // This function does NOT set any part of the stub
    // returned.  The stub returned is ignored by the
    // ACC.
    //
    inline min::stub * new_aux_stub ( void )
    {
	if ( min::internal
	        ::number_of_free_stubs == 0 )
	    min::internal
	       ::acc_expand_stub_free_list ( 1 );
	-- min::internal::number_of_free_stubs;
	uns64 c = min::unprotected::control_of
			( last_allocated_stub );
	min::stub * s = min::unprotected
	                   ::stub_of_acc_control ( c );
	c = min::unprotected::renew_acc_control_stub
	        ( c, min::unprotected
		        ::stub_of_acc_control
			    ( min::unprotected
			         ::control_of ( s ) ) );
	min::unprotected::set_control_of
		( last_allocated_stub, c );
	return s;
    }

    // Function to put a stub on the free list right
    // after the last allocated stub.  Note this means
    // that stubs that have been previously allocated
    // are preferred for new allocations over stubs
    // that have never been allocated.
    //
    inline void free_stub ( min::stub * s )
    {
        min::unprotected::set_gen_of
	    ( s, min::NONE );
	uns64 c = min::unprotected::control_of
			( last_allocated_stub );
	min::stub * next =
	    min::unprotected::stub_of_acc_control ( c );
	min::unprotected::set_control_of
	    ( s, min::unprotected::new_acc_control
		  ( min::FREE, next ) );
	c = min::unprotected
	       ::renew_acc_control_stub ( c, s );
	min::unprotected::set_control_of
		( last_allocated_stub, c );
	++ min::internal::number_of_free_stubs;
    }

    // fixed_bodies[j] is the head of a free list of
    // fixed bodies of size 1 << ( j + 3 ), for 0 <= j
    // and (1<<j) <= MIN_ABSOLUTE_MAX_FIXED_BODY_SIZE/8.
    // Each fixed body begins with a control word whose
    // locator is provided by the allocator and whose
    // stub address is set to the stub the body is
    // attached to.  When the body is on the free
    // list, the location after the control word is
    // a (void *) pointer to the next free body, or is
    // NULL.  When the body is in use, the object body
    // begins just after the control word, and the size
    // of the object body can be derived by using the
    // locator to find the region containing the body
    // (this is the domain of ACC code).
    //
    struct free_fixed_size_body
    {
        min::uns64	body_control;
	    // Body control word that is at the
	    // beginning of every fixed size_body.

	free_fixed_size_body * next;
	    // Next in NULL terminated list of free
	    // fixed size_bodies.
    };
    typedef struct fixed_body_list_extension;
        // Allocator specific extension of fixed_body_
	// list struct.
    extern struct fixed_body_list
    {
	unsigned size;	// Size of fixed body, including
			// control word.  For fixed_
			// bodies[j] this equals
			// 1 << (j+3).
        unsigned count;	// Number of fixed bodies on the
			// list.
	free_fixed_size_body * first;
			// First fixed body on the list,
			// or NULL if list empty.
	fixed_body_list_extension * extension;
			// Address of extension of this
			// structure.  Set during
			// allocator initialization.
    } fixed_bodies
          [MIN_ABSOLUTE_MAX_FIXED_BODY_SIZE_LOG-2];

    void new_non_fixed_body
	( min::stub * s, unsigned n );
    void new_fixed_body
	( min::stub * s, unsigned n,
	  fixed_body_list * fbl );

    extern unsigned max_fixed_body_size;

    // Allocate a body to a stub.  n is the minimum size
    // of the body in bytes, not including the control
    // word that begins the body.  The stub control is
    // set and its ACC flags may be changed.
    //
    inline void new_body ( min::stub * s, unsigned n )
    {
	unsigned m = n + sizeof ( min::uns64);

        assert ( m >=   sizeof ( min::uns64 )
	              + sizeof ( void * ) );

	if ( m > max_fixed_body_size )
	{
	     new_non_fixed_body ( s, n );
	     return;
	}

	// See min_parameters.h for fixed_bodies_log.
	//
	m = ( m - 1 ) >> 3;
	fixed_body_list * fbl =
		  fixed_bodies
		+ fixed_bodies_log ( m ) + 1;

	if ( fbl->count == 0 )
	{
	     new_fixed_body ( s, n, fbl );
	     return;
	}

	min::internal::free_fixed_size_body * b =
	    fbl->first;

	fbl->first = b->next;
	-- fbl->count;

	b->body_control =
	    min::unprotected::renew_control_stub
	        ( b->body_control, s );
	min::unprotected
	   ::set_pointer_of
	       ( s, & b->body_control + 1 );
	min::unprotected
	   ::set_flags_of ( s, ACC_FIXED_BODY_FLAG );
    }

    // Deallocate the body of a stub, and reset the stub
    // type to min::DEALLOCATED.  The stub body pointer
    // is pointed at inaccessible memory of at least the
    // same size as the old stub body.  The previous
    // stub type must specify that the stub has a body,
    // or it must equal min::DEALLOCATED (in which case
    // nothing is done).
    //
    void deallocate_body ( min::stub * s );

    // When constructed the relocate_body struct allo-
    // cates a new body for a stub s, and when descon-
    // structed the relocate_body struct installs the
    // new body in the stub s  while deallocating the
    // old body of s.  Stub s is not altered until the
    // relocate_body struct is deconstructed (the
    // void_relocate_body function can be used to
    // prevent stub s from ever being altered).
    //
    // After the new body is obtained, information
    // should be copied from the old body to the new
    // body, before the relocate_body struct is decon-
    // structed.
    // 
    struct relocate_body;
    void void_relocate_body ( relocate_body * r );
    struct relocate_body {

	// Construct relocate_body for stub s with new
	// body size n.
	//
        relocate_body ( min::stub * s, unsigned n )
	    : s ( s )
	{
	    min::unprotected
	       ::set_type_of ( & stub, min::FREE );
	    new_body ( & stub, n );
	}
	~relocate_body ( void )
	{
	    if ( s == NULL ) return;
	    s->v.u64 = stub.v.u64;
	    s->c.u64 ^= ( stub.c.u64 ^ s->c.u64 )
	                &
			ACC_FIXED_BODY_FLAG;
	}

	friend void * new_body_pointer
			( relocate_body & r );
	friend void void_relocate_body
			( relocate_body & r );

    private:

	min::stub * s;
        min::stub stub;

    };

    // Return a pointer to the new body.
    //
    inline void * new_body_pointer ( relocate_body & r )
    {
	return min::unprotected
		  ::pointer_of ( & r.stub );
    }
    // Void the relocate_body struct so it will not
    // change anything when deconstructed.  The
    // new body is deallocated.
    //
    inline void void_relocate_body ( relocate_body & r )
    {
	deallocate_body ( & r.stub );
	r.s = NULL;
    }

} }

namespace min {

    inline void deallocate ( min::stub * s )
    {
    	min::internal::deallocate_body ( s );
    }

}

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
	    if ( v >= ( min::GEN_DIRECT_STR << VSIZE ) )
	        return false;
	    else if ( v >= (    min::GEN_DIRECT_INT
	                     << VSIZE ) )
	        return true;
	    else
	        return
		  ( type_of
		      ( internal::general_uns32_to_stub
		      		( v ) )
		        == min::NUMBER );
	}
	inline min::gen new_num_gen ( int v )
	{
	    if ( ( -1 << 27 ) <= v && v < ( 1 << 27 ) )
		return unprotected::new_direct_int_gen
				( v );
	    return unprotected::new_num_stub_gen ( v );
	}
	inline min::gen new_num_gen ( float64 v )
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
	    if ( v < ( min::GEN_DIRECT_INT << VSIZE ) )
	    {
		min::stub * s =
		    internal::
		    general_uns32_to_stub ( v );
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
		  < ( min::GEN_DIRECT_STR << VSIZE ) )
		return unprotected::
		       direct_int_of ( v );
	    else
	    {
		MIN_ASSERT ( is_num ( v ) );
	    }
	}
	inline float64 float_of ( min::gen v )
	{
	    if ( v < ( min::GEN_DIRECT_INT << VSIZE ) )
	    {
		min::stub * s =
		    internal::
		    general_uns32_to_stub ( v );
		return float_of ( s );
	    }
	    else if
		(   v
		  < ( min::GEN_DIRECT_STR << VSIZE ) )
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
	inline min::gen new_num_gen ( int v )
	{
	    return new_direct_float_gen ( v );
	}
	inline min::gen new_num_gen ( float64 v )
	{
	    return new_direct_float_gen ( v );
	}
	inline int int_of ( min::gen v )
	{
	    MIN_ASSERT ( is_num ( v ) );
	    min::float64 f =
	        unprotected::direct_float_of ( v );
	    MIN_ASSERT
		( INT_MIN <= f && f <= INT_MAX );
	    int i = (int) f;
	    MIN_ASSERT ( i == f );
	    return i;
	}
	inline float64 float_of ( min::gen v )
	{
	    MIN_ASSERT ( is_num ( v ) );
	    return unprotected::direct_float_of ( v );
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

    inline min::uns64 short_str_of ( min::stub * s )
    {
	return s->v.u64;
    }
    inline min::unprotected::long_str * long_str_of
	    ( min::stub * s )
    {
	return (min::unprotected::long_str *)
	       min::internal
	          ::uns64_to_pointer ( s->v.u64 );
    }
    inline const char * str_of
    	    ( min::unprotected::long_str * str )
    {
	return (const char *) str
	       + sizeof ( min::unprotected::long_str );
    }

    inline unsigned length_of
    	    ( min::unprotected::long_str * str )
    {
	return str->length;
    }
    inline unsigned hash_of
        ( min::unprotected::long_str * str )
    {
	return str->hash;
    }
} }

namespace min {

    // Functions to compute the hash of an arbitrary
    // char string.
    //
    min::uns32 strnhash
	    ( const char * p, unsigned size );
    //
    min::uns32 strhash ( const char * p );

    unsigned strlen ( min::gen v );
    min::uns32 strhash ( min::gen v );
    char * strcpy ( char * p, min::gen v );
    char * strncpy ( char * p, min::gen v, unsigned n );
    int strcmp ( const char * p, min::gen v );
    int strncmp
        ( const char * p, min::gen v, unsigned n );

    // String Pointers:

    // Some forward reference stuff that must be
    // declared here before it is referenced by a
    // friend declaration.
    //
    class str_pointer;
    namespace unprotected {
	const char * str_of ( min::str_pointer & sp );
    }
    void initialize
	( min::str_pointer & sp, min::gen v );
    unsigned strlen ( min::str_pointer & sp );
    min::uns32 strhash ( min::str_pointer & sp );
    char * strcpy ( char * p, min::str_pointer & sp );
    char * strncpy ( char * p, min::str_pointer & sp,
    			       unsigned n );
    int strcmp
        ( const char * p, min::str_pointer & sp );
    int strncmp
        ( const char * p, min::str_pointer & sp );

    class str_pointer
    {
    public:

	str_pointer ( min::gen v )
	{

	    if ( min::is_stub ( v ) )
	    {
		s = min::unprotected::stub_of ( v );
		if (    min::type_of ( s )
		     == min::LONG_STR )
		    return;

		MIN_ASSERT (    min::type_of ( s )
			     == min::SHORT_STR );

		pseudo_body.u.str = s->v.u64;
		pseudo_body.u.buf[8] = 0;
	    }
	    else
	    {
	        MIN_ASSERT ( min::is_direct_str ( v ) );

		pseudo_body.u.str
		    = min::unprotected
			 ::direct_str_of ( v );
	    }

	    s = & pseudo_stub;
	    min::unprotected
	       ::set_pointer_of
		   ( & pseudo_stub,
		     (void *) & pseudo_body );
	}

	// Operator[] MUST be a member and cannot be a
	// friend.
	//
        const char operator[] ( int index )
	{
	    return
	        ( (const char * )
	          min::unprotected::long_str_of ( s ) )
	        [sizeof ( min::unprotected::long_str )
		 + index];
	}

	friend const char * min::unprotected::str_of
	    ( str_pointer & sp );
	friend void min::initialize
	    ( str_pointer & sp, min::gen v );
	friend unsigned min::strlen
	    ( min::str_pointer & sp );
	friend min::uns32 min::strhash
	    ( min::str_pointer & sp );
	friend char * min::strcpy
	    ( char * p, min::str_pointer & sp );
	friend char * min::strncpy
	    ( char * p, min::str_pointer & sp,
	                unsigned n );
	friend int min::strcmp
	    ( const char * p, min::str_pointer & sp );
	friend int min::strncmp
	    ( const char * p, min::str_pointer & sp,
	                      unsigned n );

    private:

	min::stub * s;
	    // Stub pointer if long string, or
	    // pointer to pseudo_stub for short
	    // or direct string.

	min::stub pseudo_stub;
	    // Pseudo-stub for short or direct
	    // string; only value is used to point
	    // at buf + sizeof ( MUP::long_str ).

	struct
	{
	    struct min::unprotected::long_str h;
	    union {
		char buf [9];
		    // NUL terminated copy of string.
		uns64 str;
		uns64 xx[2];
		    // Sized to maintain alignment
		    // of surrounding data.
	    } u;
	} pseudo_body;
    };

    inline const char * unprotected::str_of
	    ( min::str_pointer & sp )
    {
	return (const char * )
	       min::unprotected::long_str_of ( sp.s )
	       +
	       sizeof ( min::unprotected::long_str );
    }

    inline void initialize
	    ( min::str_pointer & sp, min::gen v )
    {
	new ( & sp ) min::str_pointer ( v );
    }

    inline unsigned strlen
        ( min::str_pointer & sp )
    {
        if ( sp.s == & sp.pseudo_stub )
	    return ::strlen ( sp.pseudo_body.u.buf );
	else
	    return min::unprotected
	              ::long_str_of ( sp.s )
		      -> length;
    }

    inline min::uns32 strhash ( min::str_pointer & sp )
    {
        if ( sp.s == & sp.pseudo_stub )
	    return min::strhash
	        ( sp.pseudo_body.u.buf );
	else
	    return min::unprotected
	              ::long_str_of ( sp.s )
		      -> hash;
    }

    inline char * strcpy
    	( char * p, min::str_pointer & sp )
    {
        return ::strcpy
	    ( p, min::unprotected::str_of ( sp ) );
    }

    inline char * strncpy
    	( char * p, min::str_pointer & sp, unsigned n )
    {
        return ::strncpy
	    ( p, min::unprotected::str_of ( sp ), n );
    }

    inline int strcmp
    	( const char * p, min::str_pointer & sp )
    {
        return ::strcmp
	    ( p, min::unprotected::str_of ( sp ) );
    }

    inline int strncmp
    	( const char * p, min::str_pointer & sp,
	                  unsigned n )
    {
        return ::strncmp
	    ( p, min::unprotected::str_of ( sp ), n );
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
	min::gen new_str_stub_gen_internal
	    ( const char * p, unsigned n );
	inline min::gen new_str_stub_gen
		( const char * p )
	{
	    return new_str_stub_gen_internal
	        ( p, ::strlen ( p ) );
	}
	inline min::gen new_str_stub_gen
	    ( const char * p, unsigned n )
	{
	    return new_str_stub_gen_internal
	        ( p, ::strnlen ( p, n ) );
	}
    }

    inline min::gen new_str_gen ( const char * p )
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

    inline min::gen new_str_gen
            ( const char * p, unsigned n )
    {
	n = strnlen ( p, n );
#	if MIN_IS_COMPACT
	    if ( n <= 3 )
		return min::unprotected::
		       new_direct_str_gen ( p, n );
#	elif MIN_IS_LOOSE
	    if ( n <= 5 )
		return min::unprotected::
		       new_direct_str_gen ( p, n );
#	endif
	return min::unprotected::
	       new_str_stub_gen ( p, n );
    }
}

// Labels
// ------

// Labels are implemented by a body consisting of a
// header followed by the label elements.  The header
// contains the length (number of elements) and the
// hash value.
//
// This implementation is hidden so an implementation
// using uncollectible stubs is also possible.

namespace min { namespace internal {

    struct lab_header
    {
        uns32		length;
        uns32		hash;
    };

    const unsigned lab_header_size =
        sizeof ( lab_header ) / sizeof ( min::gen );

    inline min::internal::lab_header * lab_header_of
	    ( min::stub * s )
    {
        return (min::internal::lab_header *)
	       min::unprotected::pointer_of ( s );
    }

    inline unsigned lablen
	    ( min::internal::lab_header * lh )
    {
        return lh->length;
    }

    inline unsigned labhash
	    ( min::internal::lab_header * lh )
    {
        return lh->hash;
    }

    inline const min::gen * vector_of
	    ( min::internal::lab_header * lh )
    {
        return (const min::gen *) lh
	     + min::internal::lab_header_size;
    }

} }

namespace min {

    inline unsigned lab_of
	    ( min::gen * p, unsigned n, min::stub * s )
    {
        MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
	min::internal::lab_header * lh =
	    min::internal::lab_header_of ( s );
	const min::gen * q =
	    min::internal::vector_of ( lh );
	unsigned len = min::internal::lablen ( lh );

	unsigned count = 0;
        while ( count < n && len -- )
	{
	    * p ++ = * q ++;
	    ++ count;
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
	return min::internal::lablen
	    ( min::internal::lab_header_of ( s ) );
    }

    inline unsigned lablen ( min::gen v )
    {
	return min::lablen ( min::stub_of ( v ) );
    }

    inline unsigned labhash ( min::stub * s )
    {
        MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
	return min::internal::labhash
	    ( min::internal::lab_header_of ( s ) );
    }

    inline min::uns32 labhash ( min::gen v )
    {
	return min::labhash ( min::stub_of ( v ) );
    }

    min::uns32 labhash
	    ( const min::gen * p, unsigned n );

    min::gen new_lab_gen
	    ( const min::gen * p, unsigned n );

    inline bool is_lab ( min::gen v )
    {
	if ( ! min::is_stub ( v ) )
	    return false;
	min::stub * s = min::unprotected::stub_of ( v );
	return min::type_of ( s ) == min::LABEL;
    }
}

// Name Functions
// ---- ---------

namespace min {

    inline bool is_name ( min::gen v )
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

    // Flags Bits:
    //
    //    0:		OBJ_OPEN
    //    1:    	OBJ_INSERT_MODE
    //    2:		reserved for future use
    //    3-4:		long objects only,
    //			reserved for future use
    //
    // The flag bits are the low order XXX_OBJ_FLAG_BITS
    // of the flags, and the higher order bits are the
    // hash size code I.
    //
    // hash_size[I] is the size of the hash table in an
    // object body, in min::gen units.
    //
    // OBJ_OPEN is on if a vec_pointer, updatable_vec_
    // pointer, or insertable_vec_pointer exists for the
    // object.
    //
    // OBJ_INSERT_MODE is on if an insertable_vec_
    // pointer exists for the object.
    // 
    const unsigned OBJ_OPEN = 1;
    const unsigned OBJ_INSERT_MODE = 2;

    const unsigned SHORT_OBJ_FLAG_BITS = 3;
    const unsigned LONG_OBJ_FLAG_BITS = 5;

    const unsigned SHORT_OBJ_HASH_SIZE_CODE_BITS =
        8 - SHORT_OBJ_FLAG_BITS;
    const unsigned LONG_OBJ_HASH_SIZE_CODE_BITS =
        16 - LONG_OBJ_FLAG_BITS;

    extern min::uns32 hash_size[];

    struct short_obj
    {
        min::uns8	flags;
        min::uns8	hash_offset;
        min::uns16	unused_offset;
        min::uns16	aux_offset;
        min::uns16	total_size;
    };

    const unsigned short_obj_header_size =
        sizeof ( short_obj ) / sizeof ( min::gen );

    struct long_obj
    {
        min::uns16	flags;
        min::uns16	hash_offset;
        min::uns32	unused_offset;
        min::uns32	aux_offset;
        min::uns32	total_size;
    };

    const unsigned long_obj_header_size =
        sizeof ( long_obj ) / sizeof ( min::gen );

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

    inline unsigned short_obj_hash_size_of_flags
	    ( unsigned flags )
    {
        return min::unprotected::hash_size
		[ flags >> min::unprotected
		              ::SHORT_OBJ_FLAG_BITS ];
    }

    inline unsigned long_obj_hash_size_of_flags
	    ( unsigned flags )
    {
        return min::unprotected::hash_size
		[ flags >> min::unprotected
		              ::LONG_OBJ_FLAG_BITS ];
    }
} }

namespace min {

    inline unsigned hash_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   min::unprotected::
		      short_obj_hash_size_of_flags
		          (so->flags);
    }

    inline unsigned hash_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   min::unprotected::
		      long_obj_hash_size_of_flags
		          (lo->flags);
    }

    inline unsigned attr_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   so->unused_offset
               - min::unprotected
		    ::short_obj_hash_size_of_flags
		        (so->flags)
	       - unprotected::short_obj_header_size;
    }

    inline unsigned attr_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   lo->unused_offset
               - min::unprotected
		    ::long_obj_hash_size_of_flags
		        (lo->flags)
	       - unprotected::long_obj_header_size;
    }

    inline unsigned aux_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   so->total_size
	       - so->aux_offset;
    }

    inline unsigned aux_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   lo->total_size
	       - lo->aux_offset;
    }

    inline unsigned unused_size_of
	    ( min::unprotected::short_obj * so )
    {
        return   so->aux_offset
	       - so->unused_offset;
    }

    inline unsigned unused_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return   lo->aux_offset
	       - lo->unused_offset;
    }

    inline unsigned header_size_of
	    ( min::unprotected::short_obj * so )
    {
        return unprotected::short_obj_header_size;
    }

    inline unsigned header_size_of
	    ( min::unprotected::long_obj * lo )
    {
        return unprotected::long_obj_header_size;
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

    unsigned short_obj_hash_size ( unsigned u );
    unsigned short_obj_total_size ( unsigned u );
    unsigned long_obj_hash_size ( unsigned u );
    unsigned long_obj_total_size ( unsigned u );

    min::gen new_obj_gen
	    ( unsigned hash_size,
	      unsigned unused_size );
}

// Object Vector Level
// ------ ------ -----

namespace min {

    // Vector Pointers:

    // Some forward reference stuff that must be
    // declared here before it is referenced by a
    // friend declaration.
    //
    class vec_pointer;
    class updatable_vec_pointer;
    class insertable_vec_pointer;

    void initialize
	( min::vec_pointer & vp, min::stub * s );
    void initialize
	( min::vec_pointer & vp, min::gen v );

    namespace unprotected {
	min::gen * & base
	    ( min::vec_pointer & vp );
	unsigned var_offset_of
	    ( min::vec_pointer & vp );
	unsigned hash_offset_of
	    ( min::vec_pointer & vp );
	unsigned attr_offset_of
	    ( min::vec_pointer & vp );
	unsigned unused_offset_of
	    ( min::vec_pointer & vp );
	unsigned aux_offset_of
	    ( min::vec_pointer & vp );
    }
    min::stub * stub_of
	( min::vec_pointer & vp );
    unsigned hash_size_of
	( min::vec_pointer & vp );
    unsigned attr_size_of
	( min::vec_pointer & vp );
    unsigned unused_size_of
	( min::vec_pointer & vp );
    const min::gen & var
	( min::vec_pointer & vp, unsigned index );
    const min::gen & hash
	( min::vec_pointer & vp, unsigned index );
    const min::gen & attr
	( min::vec_pointer & vp, unsigned index );
    const min::gen & aux
	( min::vec_pointer & vp, unsigned index );

    namespace unprotected {
	min::gen * & base
	    ( min::updatable_vec_pointer & vp );
    }
    min::gen & var
	( min::updatable_vec_pointer & vp,
	  unsigned index );
    min::gen & hash
	( min::updatable_vec_pointer & vp,
	  unsigned index );
    min::gen & attr
	( min::updatable_vec_pointer & vp,
	  unsigned index );
    min::gen & aux
	( min::updatable_vec_pointer & vp,
	  unsigned index );

    namespace unprotected {
	unsigned & unused_offset_of
	    ( min::insertable_vec_pointer & vp );
	unsigned & aux_offset_of
	    ( min::insertable_vec_pointer & vp );
    }

    void initialize
	( min::insertable_vec_pointer & vp,
	  min::stub * s );
    void initialize
	( min::insertable_vec_pointer & vp,
	  min::gen v );

    unsigned attr_push
	( min::insertable_vec_pointer & vp,
	  min::gen v );
    unsigned attr_push
	( min::insertable_vec_pointer & vp,
	  const min::gen * p, unsigned n );
    unsigned aux_push
	( min::insertable_vec_pointer & vp,
	  min::gen v );
    unsigned aux_push
	( min::insertable_vec_pointer & vp,
	  const min::gen * p, unsigned n );

    unsigned attr_pop
	( min::insertable_vec_pointer & vp,
	  min::gen & v );
    unsigned attr_pop
	( min::insertable_vec_pointer & vp,
	  min::gen * p, unsigned n );
    unsigned aux_pop
	( min::insertable_vec_pointer & vp,
	  min::gen & v );
    unsigned aux_pop
	( min::insertable_vec_pointer & vp,
	  min::gen * p, unsigned n );


    struct vec_pointer
    {
    public:

        enum {
	    PLAIN = 1,
	    UPDATABLE = 2,
	    INSERTABLE = 3 };

	const int type;

	vec_pointer ( min::stub * s )
	    : s ( s ), type ( PLAIN )
	    { init(); }
	vec_pointer ( min::gen v )
	    : s ( min::stub_of ( v ) ), type ( PLAIN )
	    { init(); }

        // Friends
	//
	friend void initialize
	    ( min::vec_pointer & vp, min::stub * s );
	friend void initialize
	    ( min::vec_pointer & vp, min::gen v );

	friend min::gen * & unprotected::base
	    ( min::vec_pointer & vp );
	friend unsigned unprotected::var_offset_of
	    ( min::vec_pointer & vp );
	friend unsigned unprotected::hash_offset_of
	    ( min::vec_pointer & vp );
	friend unsigned unprotected::attr_offset_of
	    ( min::vec_pointer & vp );
	friend unsigned unprotected::unused_offset_of
	    ( min::vec_pointer & vp );
	friend unsigned unprotected::aux_offset_of
	    ( min::vec_pointer & vp );

	friend min::stub * stub_of
	    ( min::vec_pointer & vp );
	friend unsigned hash_size_of
	    ( min::vec_pointer & vp );
	friend unsigned attr_size_of
	    ( min::vec_pointer & vp );
	friend unsigned unused_size_of
	    ( min::vec_pointer & vp );

	friend const min::gen & var
	    ( min::vec_pointer & vp, unsigned index );
	friend const min::gen & hash
	    ( min::vec_pointer & vp, unsigned index );
	friend const min::gen & attr
	    ( min::vec_pointer & vp, unsigned index );
	friend const min::gen & aux
	    ( min::vec_pointer & vp, unsigned index );

	friend min::gen * & unprotected::base
	    ( min::updatable_vec_pointer & vp );
	friend min::gen & var
	    ( min::updatable_vec_pointer & vp,
	      unsigned index );
	friend min::gen & hash
	    ( min::updatable_vec_pointer & vp,
	      unsigned index );
	friend min::gen & attr
	    ( min::updatable_vec_pointer & vp,
	      unsigned index );
	friend min::gen & aux
	    ( min::updatable_vec_pointer & vp,
	      unsigned index );

	friend unsigned & unprotected::unused_offset_of
	    ( min::insertable_vec_pointer & vp );
	friend unsigned & unprotected::aux_offset_of
	    ( min::insertable_vec_pointer & vp );

	friend void initialize
	    ( min::insertable_vec_pointer & vp,
	      min::stub * s );
	friend void initialize
	    ( min::insertable_vec_pointer & vp,
	      min::gen v );

	friend unsigned attr_push
	    ( min::insertable_vec_pointer & vp,
	      min::gen v );
	friend unsigned attr_push
	    ( min::insertable_vec_pointer & vp,
	      const min::gen * p, unsigned n );
	friend unsigned aux_push
	    ( min::insertable_vec_pointer & vp,
	      min::gen v );
	friend unsigned aux_push
	    ( min::insertable_vec_pointer & vp,
	      const min::gen * p, unsigned n );

	friend unsigned attr_pop
	    ( min::insertable_vec_pointer & vp,
	      min::gen & v );
	friend unsigned attr_pop
	    ( min::insertable_vec_pointer & vp,
	      min::gen * p, unsigned n );
	friend unsigned aux_pop
	    ( min::insertable_vec_pointer & vp,
	      min::gen & v );
	friend unsigned aux_pop
	    ( min::insertable_vec_pointer & vp,
	      min::gen * p, unsigned n );

    protected:

	vec_pointer ( min::stub * s, int type )
	    : s ( s ), type ( type )
	    { init(); }

	~ vec_pointer ( void )
	{
	    int t = min::type_of ( s );
	    if ( t == min::SHORT_OBJ )
	    {
		min::unprotected::short_obj * so =
		    min::unprotected
		       ::short_obj_of ( s );

		so->flags	  = flags;
		if ( type == INSERTABLE )
		{
		    so->unused_offset = unused_offset;
		    so->aux_offset    = aux_offset;
		}
	    }
	    else
	    {
	        MIN_ASSERT ( t == min::LONG_OBJ );
		min::unprotected::long_obj * lo =
		    min::unprotected
		       ::long_obj_of ( s );

		lo->flags	  = flags;
		if ( type == INSERTABLE )
		{
		    lo->unused_offset = unused_offset;
		    lo->aux_offset    = aux_offset;
		}
	    }
	}

    	min::stub * s;
	    // Stub of object; set by constructor or
	    // init function.

        min::uns32	flags;
        min::uns32	unused_offset;
        min::uns32	aux_offset;
        min::uns32	total_size;
	    // Cache of object header.

	min::uns32	hash_size;
	    // Hash table size.

	min::uns32	var_offset;
	min::uns32	hash_offset;
	min::uns32	attr_offset;
	    // Offsets of variable vector, hash table,
	    // and attribute vector.

    private:

        void init ( void )
	{
	    int t = min::type_of ( s );
	    unsigned forbidden =
	        type == INSERTABLE ?
		      unprotected::OBJ_OPEN
		    + unprotected::OBJ_INSERT_MODE :
		    unprotected::OBJ_INSERT_MODE;
	    unsigned new_flags =
	        type == INSERTABLE ?
		      unprotected::OBJ_OPEN
		    + unprotected::OBJ_INSERT_MODE :
		    unprotected::OBJ_OPEN;
	    if ( t == min::SHORT_OBJ )
	    {
		min::unprotected::short_obj * so =
		    min::unprotected
		       ::short_obj_of ( s );

		flags		= so->flags;
		MIN_ASSERT
		    ( ( flags & forbidden ) == 0 );
		so->flags 	= flags | new_flags;

		hash_offset	= so->hash_offset;
		unused_offset	= so->unused_offset;
		aux_offset	= so->aux_offset;
		total_size	= so->total_size;

		var_offset =
		    unprotected::short_obj_header_size;
		hash_size =
		    unprotected
		        ::short_obj_hash_size_of_flags
			    ( flags );
	    }
	    else
	    {
	        MIN_ASSERT ( t == min::LONG_OBJ );
		min::unprotected::long_obj * lo =
		    min::unprotected
		       ::long_obj_of ( s );

		flags		= lo->flags;
		MIN_ASSERT
		    ( ( flags & forbidden ) == 0 );
		lo->flags 	= flags | new_flags;

		hash_offset	= lo->hash_offset;
		unused_offset	= lo->unused_offset;
		aux_offset	= lo->aux_offset;
		total_size	= lo->total_size;

		var_offset =
		    unprotected::long_obj_header_size;
		hash_size =
		    unprotected
		        ::long_obj_hash_size_of_flags
			    ( flags );
	    }
	    attr_offset = hash_offset + hash_size;
	}
    };

    class updatable_vec_pointer : public vec_pointer {

    public:

	updatable_vec_pointer ( min::stub * s )
	    : vec_pointer ( s, UPDATABLE )
	    {}
	updatable_vec_pointer ( min::gen v )
	    : vec_pointer ( min::stub_of ( v ),
	                    UPDATABLE )
	    {}

    protected:

	updatable_vec_pointer
		( min::stub * s, int type )
	    : vec_pointer ( s, type )
	    {}

    };

    class insertable_vec_pointer
        : public updatable_vec_pointer {

    public:

	insertable_vec_pointer ( min::stub * s )
	    : updatable_vec_pointer ( s, INSERTABLE )
	    {}
	insertable_vec_pointer ( min::gen v )
	    : updatable_vec_pointer
	          ( min::stub_of ( v ), INSERTABLE )
	    {}
    };

    inline void initialize
	( min::vec_pointer & vp, min::stub * s )
    {
        new ( & vp ) min::vec_pointer ( s );
    }

    inline void initialize
	( min::vec_pointer & vp, min::gen v )
    {
        new ( & vp ) min::vec_pointer ( v );
    }

    inline min::gen * & unprotected::base
	( min::vec_pointer & vp )
    {
	return * (min::gen **) &
	       min::unprotected::pointer_ref_of
	           ( vp.s );
    }
    inline unsigned unprotected::var_offset_of
	( min::vec_pointer & vp )
    {
        return vp.var_offset;
    }
    inline unsigned unprotected::hash_offset_of
	( min::vec_pointer & vp )
    {
        return vp.hash_offset;
    }
    inline unsigned unprotected::attr_offset_of
	( min::vec_pointer & vp )
    {
        return vp.attr_offset;
    }
    inline unsigned unprotected::unused_offset_of
	( min::vec_pointer & vp )
    {
        return vp.unused_offset;
    }
    inline unsigned unprotected::aux_offset_of
	( min::vec_pointer & vp )
    {
        return vp.aux_offset;
    }

    inline min::stub * stub_of
	( min::vec_pointer & vp )
    {
        return vp.s;
    }
    inline unsigned hash_size_of
	( min::vec_pointer & vp )
    {
        return vp.hash_size;
    }
    inline unsigned attr_size_of
	( min::vec_pointer & vp )
    {
        return vp.unused_offset - vp.attr_offset;
    }
    inline unsigned unused_size_of
	( min::vec_pointer & vp )
    {
        return vp.aux_offset - vp.unused_offset;
    }

    inline const min::gen & var
	( min::vec_pointer & vp, unsigned index )
    {
	index += vp.var_offset;
	MIN_ASSERT ( index < vp.hash_offset );
	return unprotected::base(vp)[index];
    }

    inline const min::gen & hash
	( min::vec_pointer & vp, unsigned index )
    {
	index += vp.hash_offset;
	MIN_ASSERT ( index < vp.attr_offset );
	return unprotected::base(vp)[index];
    }

    inline const min::gen & attr
	( min::vec_pointer & vp, unsigned index )
    {
	index += vp.attr_offset;
	MIN_ASSERT ( index < vp.unused_offset );
	return unprotected::base(vp)[index];
    }

    inline const min::gen & aux
	( min::vec_pointer & vp, unsigned index )
    {
	MIN_ASSERT ( vp.aux_offset <= index );
	MIN_ASSERT ( index < vp.total_size );
	return unprotected::base(vp)[index];
    }

    inline min::gen * & unprotected::base
	( min::updatable_vec_pointer & vp )
    {
	return * (min::gen **) &
	       min::unprotected::pointer_ref_of
	           ( vp.s );
    }

    inline min::gen & var
	( min::updatable_vec_pointer & vp,
	  unsigned index )
    {
	index += vp.var_offset;
	MIN_ASSERT ( index < vp.hash_offset );
	return unprotected::base(vp)[index];
    }

    inline min::gen & hash
	( min::updatable_vec_pointer & vp,
	  unsigned index )
    {
	index += vp.hash_offset;
	MIN_ASSERT ( index < vp.attr_offset );
	return unprotected::base(vp)[index];
    }

    inline min::gen & attr
	( min::updatable_vec_pointer & vp,
	  unsigned index )
    {
	index += vp.attr_offset;
	MIN_ASSERT ( index < vp.unused_offset );
	return unprotected::base(vp)[index];
    }

    inline min::gen & aux
	( min::updatable_vec_pointer & vp,
	  unsigned index )
    {
	MIN_ASSERT ( vp.aux_offset <= index );
	MIN_ASSERT ( index < vp.total_size );
	return unprotected::base(vp)[index];
    }

    inline unsigned & unprotected::unused_offset_of
	( min::insertable_vec_pointer & vp )
    {
        return vp.unused_offset;
    }
    inline unsigned & unprotected::aux_offset_of
	( min::insertable_vec_pointer & vp )
    {
        return vp.aux_offset;
    }

    inline void initialize
	( min::insertable_vec_pointer & vp,
	  min::stub * s )
    {
	vp.~insertable_vec_pointer();
        new ( & vp ) min::vec_pointer ( s );
    }

    inline void initialize
	( min::insertable_vec_pointer & vp,
	  min::gen v )
    {
	vp.~insertable_vec_pointer();
        new ( & vp ) min::vec_pointer ( v );
    }

    inline unsigned attr_push
	( min::insertable_vec_pointer & vp,
	  min::gen v )
    {
	MIN_ASSERT ( vp.unused_offset < vp.aux_offset );
	unprotected::base(vp)[vp.unused_offset] = v;
	return vp.unused_offset ++;
    }

    inline unsigned attr_push
	( min::insertable_vec_pointer & vp,
	  const min::gen * p, unsigned n )
    {
	MIN_ASSERT
	    ( vp.unused_offset + n <= vp.aux_offset );
	unsigned result = vp.unused_offset;
	memcpy ( unprotected::base(vp) + result,
	         p, sizeof ( min::gen ) * n );
	vp.unused_offset += n;
	return result;
    }

    inline unsigned aux_push
	( min::insertable_vec_pointer & vp,
	  min::gen v )
    {
	MIN_ASSERT ( vp.unused_offset < vp.aux_offset );
	unprotected::base(vp)[-- vp.aux_offset] = v;
	return vp.aux_offset;
    }

    inline unsigned aux_push
	( min::insertable_vec_pointer & vp,
	  const min::gen * p, unsigned n )
    {
	MIN_ASSERT
	    ( vp.unused_offset + n <= vp.aux_offset );
	vp.aux_offset -= n;
	memcpy ( unprotected::base(vp) + vp.aux_offset,
	         p, sizeof ( min::gen ) * n );
	return vp.aux_offset;
    }

    inline unsigned attr_pop
	( min::insertable_vec_pointer & vp,
	  min::gen & v )
    {
	MIN_ASSERT
	    ( vp.attr_offset < vp.unused_offset );
	v = unprotected::base(vp)[-- vp.unused_offset];
	return vp.unused_offset;
    }

    inline unsigned attr_pop
	( min::insertable_vec_pointer & vp,
	  min::gen * p, unsigned n )
    {
	MIN_ASSERT
	    ( vp.attr_offset + n <= vp.unused_offset );
	vp.unused_offset -= n;
	memcpy
	    ( p,
	      unprotected::base(vp) + vp.unused_offset,
	      sizeof ( min::gen ) * n );
	return vp.unused_offset;
    }

    inline unsigned aux_pop
	( min::insertable_vec_pointer & vp,
	  min::gen & v )
    {
	MIN_ASSERT ( vp.aux_offset < vp.total_size );
	v = unprotected::base(vp)[vp.aux_offset ++];
	return vp.aux_offset;
    }

    inline unsigned aux_pop
	( min::insertable_vec_pointer & vp,
	  min::gen * p, unsigned n )
    {
	MIN_ASSERT
	    ( vp.aux_offset + n <= vp.total_size );
	memcpy ( p,
		 unprotected::base(vp) + vp.aux_offset,
	         sizeof ( min::gen ) * n );
	vp.aux_offset += n;
	return vp.aux_offset;
    }
}

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

    inline unsigned attr_push
    	    ( min::unprotected::short_obj * so,
	      min::gen value )
    {
        min::gen * q = (min::gen *) so;
	q[so->unused_offset] = value;
	return so->unused_offset ++;
    }

    inline unsigned attr_push
    	    ( min::unprotected::long_obj * lo,
	      min::gen value )
    {
        min::gen * q = (min::gen *) lo;
	q[lo->unused_offset] = value;
	return lo->unused_offset ++;
    }

    inline unsigned attr_push
    	    ( min::unprotected::short_obj * so,
	      min::gen * p, unsigned n )
    {
        min::gen * q = (min::gen *) so;
	unsigned result = so->unused_offset;
	while ( n -- )
	    q[so->unused_offset ++] = * p ++;
	return result;
    }

    inline unsigned attr_push
    	    ( min::unprotected::long_obj * lo,
	      min::gen * p, unsigned n )
    {
        min::gen * q = (min::gen *) lo;
	unsigned result = lo->unused_offset;
	while ( n -- )
	    q[lo->unused_offset ++] = * p ++;
	return result;
    }

    inline unsigned aux_push
    	    ( min::unprotected::short_obj * so,
	      min::gen value )
    {
        min::gen * q = (min::gen *) so;
	q[-- so->aux_offset] = value;
	return so->aux_offset;
    }

    inline unsigned aux_push
    	    ( min::unprotected::long_obj * lo,
	      min::gen value )
    {
        min::gen * q = (min::gen *) lo;
	q[-- lo->aux_offset] = value;
	return lo->aux_offset;
    }

    inline unsigned aux_push
    	    ( min::unprotected::short_obj * so,
	      min::gen * p, unsigned n )
    {
        min::gen * q = (min::gen *) so;
	p += n;
	while ( n -- )
	    q[-- so->aux_offset] = * -- p;
	return so->aux_offset;
    }

    inline unsigned aux_push
    	    ( min::unprotected::long_obj * lo,
	      min::gen * p, unsigned n )
    {
        min::gen * q = (min::gen *) lo;
	p += n;
	while ( n -- )
	    q[-- lo->aux_offset] = * -- p;
	return lo->aux_offset;
    }

    inline unsigned attr_pop
    	    ( min::unprotected::short_obj * so,
	      min::gen & value )
    {
        min::gen * q = (min::gen *) so;
	value = q[-- so->unused_offset];
	return so->unused_offset;
    }

    inline unsigned attr_pop
    	    ( min::unprotected::long_obj * lo,
	      min::gen & value )
    {
        min::gen * q = (min::gen *) lo;
	value = q[-- lo->unused_offset];
	return lo->unused_offset;
    }

    inline unsigned attr_pop
    	    ( min::unprotected::short_obj * so,
	      min::gen * p, unsigned n )
    {
        min::gen * q = (min::gen *) so;
	p += n;
	while ( n -- )
	    * -- p = q[-- so->unused_offset];
	return so->unused_offset;
    }

    inline unsigned attr_pop
    	    ( min::unprotected::long_obj * lo,
	      min::gen * p, unsigned n )
    {
        min::gen * q = (min::gen *) lo;
	p += n;
	while ( n -- )
	    * -- p = q[-- lo->unused_offset];
	return lo->unused_offset;
    }

    inline unsigned aux_pop
    	    ( min::unprotected::short_obj * so,
	      min::gen & value )
    {
        min::gen * q = (min::gen *) so;
	value = q[so->aux_offset ++];
	return so->aux_offset;
    }

    inline unsigned aux_pop
    	    ( min::unprotected::long_obj * lo,
	      min::gen & value )
    {
        min::gen * q = (min::gen *) lo;
	value = q[lo->aux_offset ++];
	return lo->aux_offset;
    }

    inline unsigned aux_pop
    	    ( min::unprotected::short_obj * so,
	      min::gen * p, unsigned n )
    {
        min::gen * q = (min::gen *) so;
	while ( n -- )
	    * p ++ = q[so->aux_offset ++];
	return so->aux_offset;
    }

    inline unsigned aux_pop
    	    ( min::unprotected::long_obj * lo,
	      min::gen * p, unsigned n )
    {
        min::gen * q = (min::gen *) lo;
	while ( n -- )
	    * p ++ = q[lo->aux_offset ++];
	return lo->aux_offset;
    }
} }

namespace min {

    inline unsigned hash_offset_of
	    ( min::unprotected::short_obj * so )
    {
        return  unprotected::short_obj_header_size;
    }

    inline unsigned hash_offset_of
	    ( min::unprotected::long_obj * lo )
    {
        return  unprotected::long_obj_header_size;
    }

    inline unsigned attr_offset_of
	    ( min::unprotected::short_obj * so )
    {
        return   unprotected::
		 short_obj_hash_size_of_flags
		     (so->flags)
	       + unprotected::short_obj_header_size;
    }

    inline unsigned attr_offset_of
	    ( min::unprotected::long_obj * lo )
    {
        return   unprotected::
		 long_obj_hash_size_of_flags (lo->flags)
	       + unprotected::long_obj_header_size;
    }

    inline unsigned aux_of
	    ( min::unprotected::short_obj * so )
    {
        return so->aux_offset;
    }

    inline unsigned aux_of
	    ( min::unprotected::long_obj * lo )
    {
        return lo->aux_offset;
    }

    inline unsigned attr_push
    	    ( min::unprotected::short_obj * so,
	      min::gen value )
    {
	MIN_ASSERT (   so->unused_offset
	             < so->aux_offset );
	return unprotected::attr_push
	    ( so, value );
    }

    inline unsigned attr_push
    	    ( min::unprotected::long_obj * lo,
	      min::gen value )
    {
	MIN_ASSERT (   lo->unused_offset
	             < lo->aux_offset );
	return unprotected::attr_push
	    ( lo, value );
    }

    inline unsigned attr_push
    	    ( min::unprotected::short_obj * so,
	      min::gen * p, unsigned n )
    {
	MIN_ASSERT (    so->unused_offset + n
	             <= so->aux_offset );
	return unprotected::attr_push
	    ( so, p, n );
    }

    inline unsigned attr_push
    	    ( min::unprotected::long_obj * lo,
	      min::gen * p, unsigned n )
    {
	MIN_ASSERT (    lo->unused_offset + n
	             <= lo->aux_offset );
	return unprotected::attr_push
	    ( lo, p, n );
    }

    inline unsigned aux_push
    	    ( min::unprotected::short_obj * so,
	      min::gen value )
    {
	MIN_ASSERT (   so->unused_offset
	             < so->aux_offset );
	return unprotected::aux_push ( so, value );
    }

    inline unsigned aux_push
    	    ( min::unprotected::long_obj * lo,
	      min::gen value )
    {
	MIN_ASSERT (   lo->unused_offset
	             < lo->aux_offset );
	return unprotected::aux_push ( lo, value );
    }

    inline unsigned aux_push
    	    ( min::unprotected::short_obj * so,
	      min::gen * p, unsigned n )
    {
	MIN_ASSERT (    so->unused_offset + n
	             <= so->aux_offset );
	return unprotected::aux_push ( so, p, n );
    }

    inline unsigned aux_push
    	    ( min::unprotected::long_obj * lo,
	      min::gen * p, unsigned n )
    {
	MIN_ASSERT (    lo->unused_offset + n
	             <= lo->aux_offset );
	return unprotected::aux_push ( lo, p, n );
    }

    inline unsigned attr_pop
    	    ( min::unprotected::short_obj * so,
	      min::gen & value )
    {
	MIN_ASSERT (   attr_offset_of ( so )
	             < so->unused_offset );
	return unprotected::attr_pop
	    ( so, value );
    }

    inline unsigned attr_pop
    	    ( min::unprotected::long_obj * lo,
	      min::gen & value )
    {
	MIN_ASSERT (   attr_offset_of ( lo )
	             < lo->unused_offset );
	return unprotected::attr_pop
	    ( lo, value );
    }

    inline unsigned attr_pop
    	    ( min::unprotected::short_obj * so,
	      min::gen * p, unsigned n )
    {
	MIN_ASSERT (    attr_offset_of ( so ) + n
	             <= so->unused_offset );
	return unprotected::attr_pop
	    ( so, p, n );
    }

    inline unsigned attr_pop
    	    ( min::unprotected::long_obj * lo,
	      min::gen * p, unsigned n )
    {
	MIN_ASSERT (    attr_offset_of ( lo ) + n
	             <= lo->unused_offset );
	return unprotected::attr_pop
	    ( lo, p, n );
    }

    inline unsigned aux_pop
    	    ( min::unprotected::short_obj * so,
	      min::gen & value )
    {
	MIN_ASSERT (   so->aux_offset
	             < so->total_size );
	return unprotected::aux_pop ( so, value );
    }

    inline unsigned aux_pop
    	    ( min::unprotected::long_obj * lo,
	      min::gen & value )
    {
	MIN_ASSERT (   lo->aux_offset
	             < lo->total_size );
	return unprotected::aux_pop ( lo, value );
    }

    inline unsigned aux_pop
    	    ( min::unprotected::short_obj * so,
	      min::gen * p, unsigned n )
    {
	MIN_ASSERT (    so->aux_offset + n
	             <= so->total_size );
	return unprotected::aux_pop ( so, p, n );
    }

    inline unsigned aux_pop
    	    ( min::unprotected::long_obj * lo,
	      min::gen * p, unsigned n )
    {
	MIN_ASSERT (    lo->aux_offset + n
	             <= lo->total_size );
	return unprotected::aux_pop ( lo, p, n );
    }
}

// Object List Level
// ------ ---- -----

// The control for LIST_AUX and SUBLIST_AUX object
// auxiliary stubs is either a list auxilary pointer
// with index in the value field of the stub control,
// or is a stub pointer to another LIST_AUX stub with
// the MUP::STUB_POINTER flag and the stub pointer in
// the control.

// Aux area elements that are not used are given the
// value NONE, and can be garbage collected when the
// object is reorganized.  Because they are often
// isolated, no attempt is made to put them on a free
// list.

namespace min { namespace internal {

    // This is the generic attribute pointer type from
    // which specific attribute pointer types are made.

    template < class vec_pointer_type >
        class list_pointer_type;

} }

namespace min {

    // Public list pointer types.

    typedef min::internal::list_pointer_type
	    < min::vec_pointer >
        list_pointer;
    typedef min::internal::list_pointer_type
	    < min::updatable_vec_pointer >
        updatable_list_pointer;
    typedef min::internal::list_pointer_type
	    < min::insertable_vec_pointer >
        insertable_list_pointer;
}

namespace min { namespace internal {

    // Internal functions: see min.cc.
    //
#   if MIN_USES_OBJ_AUX_STUBS
	void allocate_stub_list
	    ( min::stub * & first,
	      min::stub * & last,
	      int type,
	      const min::gen * p, unsigned n,
	      min::uns64 end );
#   endif

    // Out of line versions of functions: see min.cc.
    //
    void insert_reserve
    	    ( min::insertable_list_pointer & lp,
	      unsigned insertions,
	      unsigned elements,
	      bool use_obj_aux_stubs );

#   if MIN_USES_OBJ_AUX_STUBS

	// Any aux stubs used by an object MUST always
	// be reachable from min::gen elements in the
	// object vector body, although these elements
	// can be disconnected from the list structure
	// and therefore garbage.  If a min::gen value
	// is being completely removed from the object
	// vector body and it points at an aux stub,
	// then that aux stub and any aux stubs it
	// points at must be garbage collected.  This
	// function checks a min::gen value that is
	// being completely removed and performs the
	// appropriate aux stub collections.
	//
	void collect_aux_stub_helper ( min::stub * s );
	inline void collect_aux_stub ( min::gen v )
	{
	    if ( ! is_stub ( v ) ) return;
	    min::stub * s = min::stub_of ( v );
	    int type = min::type_of ( s );
	    if ( type == min::LIST_AUX
	         ||
		 type == min::SUBLIST_AUX )
	        collect_aux_stub_helper ( s );
	}
#   endif
} }

namespace min {

#   if MIN_IS_COMPACT
	const min::gen LIST_END = (min::gen)
	    ( (min::uns32) min::GEN_LIST_AUX << VSIZE );
	const min::gen EMPTY_SUBLIST = (min::gen)
	    (    (min::uns32) min::GEN_SUBLIST_AUX
	      << VSIZE );
#   elif MIN_IS_LOOSE
	const min::gen LIST_END = (min::gen)
	    ( (min::uns64) min::GEN_LIST_AUX << VSIZE );
	const min::gen EMPTY_SUBLIST = (min::gen)
	    (    (min::uns64) min::GEN_SUBLIST_AUX
	      << VSIZE );
#   endif

    extern bool use_obj_aux_stubs;

    inline bool is_list_end ( min::gen v )
    {
        return v == min::LIST_END;
    }
    inline bool is_sublist ( min::gen v )
    {
        return min::is_sublist_aux ( v )
#              if MIN_USES_OBJ_AUX_STUBS
		   ||
		   ( min::is_stub ( v )
		     &&
		     min::type_of ( min::stub_of ( v ) )
		     == min::SUBLIST_AUX )
#	       endif
	       ;
    }
    inline bool is_empty_sublist ( min::gen v )
    {
        return v == min::EMPTY_SUBLIST;
    }

    // We must declare these before we make them
    // friends.

    template < class vecpt >
    vecpt & vec_pointer_of
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp );

    template < class vecpt >
    min::gen start_hash
            ( min::internal
	         ::list_pointer_type<vecpt> & lp,
	      unsigned index );
    template < class vecpt >
    min::gen start_vector
            ( min::internal
	         ::list_pointer_type<vecpt> & lp,
	      unsigned index );
    template < class vecpt, class vecpt2 >
    min::gen start_copy
            ( min::internal
	         ::list_pointer_type<vecpt> & lp,
	      const
	      min::internal
	         ::list_pointer_type<vecpt2> & lp2 );
    template < class vecpt, class vecpt2 >
    min::gen start_sublist
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp,
    	      const min::internal
	         ::list_pointer_type<vecpt2> & lp2 );

    template < class vecpt >
    min::gen next
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp );
    template < class vecpt >
    min::gen current
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp );
    template < class vecpt >
    min::gen refresh
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp );

    template < class vecpt >
    void update
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp,
	      min::gen value );

    void insert_reserve
    	    ( min::insertable_list_pointer & lp,
	      unsigned insertions,
	      unsigned elements = 0,
	      bool use_obj_aux_stubs =
	          min::use_obj_aux_stubs );
    void insert_before
    	    ( min::insertable_list_pointer & lp,
	      const min::gen * p, unsigned n );
    void insert_after
    	    ( min::insertable_list_pointer & lp,
	      const min::gen * p, unsigned n );
    unsigned remove
    	    ( min::insertable_list_pointer & lp,
	      unsigned n = 1 );
}

namespace min { namespace internal {

    template < class vecpt >
	class list_pointer_type {

    public:

        list_pointer_type ( vecpt & vecp )
	    : vecp ( vecp ),
	      base ( min::unprotected::base ( vecp ) )
	{
	    // An unstarted list pointer behaves as if
	    // it were pointing at the end of a list
	    // for which insertions are illegal.
	    //
	    current = min::LIST_END;

	    // Reservations can be made anytime after a
	    // pointer is created, and can be made
	    // before start()ing a pointer.
	    //
	    reserved_insertions = 0;
	    reserved_elements = 0;
#	    if MIN_USES_OBJ_AUX_STUBS
		use_obj_aux_stubs = false;
#	    endif

	    // Other members are initialized by start()
	    // (see below), which sets so or lo.  This
	    // means that functions that assume a
	    // pointer has been started should first
	    // assert that so or lo is not NULL.
	}

    private:

    // Private Data:

	vecpt & vecp;
	min::gen * & base;
	    // Vector pointer to object and base(vecp).

	min::gen current;
	    // Value of current element, as returned by
	    // the min::current function.  Set to LIST_
	    // END by constructor.

	// The remaining members are NOT set by the
	// constructor.  After construction, a list_
	// pointer should behave as if it pointed at
	// the end of an empty list for which insertions
	// are programming errors.

	// Abstractly there is a current pointer and a
	// previous pointer.  The current pointer points
	// at the current value.  This current value may
	// be pointed at by a list or sublist pointer,
	// and in this case the previous pointer is set
	// to point at the list or sublist pointer, so
	// it can be updated if there is an insert
	// before the current position or a removal of
	// the current element.
	//
	// There are also some special cases:
	//
	//	The current value is the LIST_END that
	//	exists when a list_pointer has never
	//	been started.  There is no current or
	//	previous pointer.  lo == so == NULL.
	//
	//	The current value is the LIST_END that
	//	is after a list head in an attribute
	//	vector or hash table entry.  The current
	//	pointer does not exist, but the previous
	//	pointer points at the list header.
	//	previous_is_sublist_head == false.
	//
	//	The current value is the LIST_END that
	//	is after an element in a stub.  The
	//	current pointer does not exist, but the
	//	previous pointer points at the stub.
	//	previous_is_sublist_head == false.
	//
	//	The current value is the LIST_END that
	//	is the end of an empty sublist.  The
	//	current pointer does not exist, but the
	//	previous pointer points at the element
	//	containing EMPTY_SUBLIST.  previous_is_
	//	sublist_head == true.
	//
	// The current pointer is in one of the follow-
	// ing states (here `current' means `current
	// value'):
	//
	//	       current_index != 0
	//	   and current = base[current_index]
	//	   and current_stub == NULL
	//      or
	//	       current_stub != NULL
	//	   and current = gen_of ( current_stub )
	//	   and current_index == 0
	//      or
	//	       current_index == 0
	//	   and current_stub == NULL
	//	   and current pointer does not exist
	//	   and current == LIST_END
	//
	// The previous pointer is similar, but also
	// differs when it points at a stub, since it
	// could be pointing at either a sublist or
	// list head:
	//
	//	       previous_index != 0
	//	   and previous = base[previous_index]
	//	   and previous_stub == NULL
	//      or
	//	       previous_stub != NULL
	//	   and previous =
	//		   previous_is_sublist_head ?
	//		   gen_of ( previous_stub ) :
	//		   control_of ( previous_stub )
	//	   and previous_index == 0
	//      or
	//	       previous_index == 0
	//	   and previous_stub == NULL
	//	   and previous pointer does not exist
	//
	// If a previous value is a stub pointer, then
	// it points at an auxiliary stub, and is
	// treated as a sublist pointer if the auxiliary
	// stub is of type SUBLIST_AUX, and is treated
	// as a list pointer if the stub is of type
	// LIST_AUX.
	//
	// The gen_of value of the auxiliary stub is
	// equivalent to an auxiliary area element
	// pointed at by a sublist or list pointer.  The
	// control_of value of the stub is equivalent
	// to the next value after that in a list, but
	// that next value must be a list or sublist
	// pointer or LIST_END.
	//
	// If the current pointer points at a stub, the
	// previous pointer must exist.
	//
	// If the current value is not LIST_END, the
	// current pointer must exist.
	//
	// A current value can be LIST_END or EMPTY_
	// SUBLIST, but cannot be a list or sublist
	// pointer (and in particular cannot point
	// at a stub of LIST_AUX or SUBLIST_AUX type).
	//
	unsigned current_index;
	unsigned previous_index;
	bool previous_is_sublist_head;
	    // True if previous pointer exists and
	    // points at a sublist pointer.

#	if MIN_USES_OBJ_AUX_STUBS
	    min::stub * current_stub;
	    min::stub * previous_stub;
	    bool use_obj_aux_stubs;
		// True if list auxiliary stubs are to
		// be used for insertions if space in
		// the object auxiliary area runs out.
#	endif

	unsigned reserved_insertions;
	unsigned reserved_elements;
	    // Set by insert_reserve and decremented by
	    // insert_{before,after}.  The latter dec-
	    // rement reserved_insertions once and
	    // decrement reserved_elements once for
	    // each element inserted.  These counters
	    // must never become less than 0 (else
	    // assert violation).

    // Friends:

#	if MIN_USES_OBJ_AUX_STUBS
	    friend void min::internal
	                   ::allocate_stub_list
		    ( min::stub * & first,
		      min::stub * & last,
		      int type,
		      const min::gen * p, unsigned n,
		      min::uns64 end );
#	endif
	friend vecpt & vec_pointer_of<>
		( min::internal
		     ::list_pointer_type<vecpt> & lp );

	friend min::gen min::start_hash<>
		( min::internal
		     ::list_pointer_type<vecpt> & lp,
		  unsigned index );
	friend min::gen min::start_vector<>
		( min::internal
		     ::list_pointer_type<vecpt> & lp,
		  unsigned index );

	friend min::gen start_copy<>
		( min::list_pointer & lp,
		  const
		  min::list_pointer & lp2 );
	friend min::gen start_copy<>
		( min::list_pointer & lp,
		  const
		  min::updatable_list_pointer & lp2 );
	friend min::gen start_copy<>
		( min::list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );
	friend min::gen start_copy<>
		( min::updatable_list_pointer & lp,
		  const
		  min::updatable_list_pointer & lp2 );
	friend min::gen start_copy<>
		( min::updatable_list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );
	friend min::gen start_copy<>
		( min::insertable_list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );

	friend min::gen start_sublist<>
		( min::list_pointer & lp,
		  const
		  min::list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::list_pointer & lp,
		  const
		  min::updatable_list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::updatable_list_pointer & lp,
		  const
		  min::updatable_list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::updatable_list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::insertable_list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );

	friend min::gen min::next<>
		( min::internal
		     ::list_pointer_type<vecpt> & lp );
	friend min::gen min::current<>
		( min::internal
		     ::list_pointer_type<vecpt> & lp );
	friend min::gen min::refresh<>
		( min::internal
		     ::list_pointer_type<vecpt> & lp );

	friend void min::update<>
		( min::updatable_list_pointer & lp,
		  min::gen value );
	friend void min::update<>
		( min::insertable_list_pointer & lp,
		  min::gen value );
	friend void min::insert_reserve
		( min::insertable_list_pointer & lp,
		  unsigned insertions,
		  unsigned elements,
		  bool use_obj_aux_stubs );
	friend void min::internal::insert_reserve
		( min::insertable_list_pointer & lp,
		  unsigned insertions,
		  unsigned elements,
		  bool use_obj_aux_stubs );
	friend void min::insert_before
		( min::insertable_list_pointer & lp,
		  const min::gen * p, unsigned n );
	friend void min::insert_after
		( min::insertable_list_pointer & lp,
		  const min::gen * p, unsigned n );
	friend unsigned min::remove
		( min::insertable_list_pointer & lp,
		  unsigned n );

    // Private Helper Functions:

	// Set current pointer to the index argument,
	// and then set current.  Do fowarding if
	// current is a list pointer: i.e., a list aux
	// pointer or a pointer to a stub with type
	// LIST_AUX.  Set previous_index and previous_
	// stub; if there is no forwarding, set these
	// to indicate the previous pointer does not
	// exist.  Return current.  Index argument must
	// not be 0.
	//
	min::gen forward ( unsigned index )
	{
	    current_index = index;
	    current = base[current_index];

	    previous_index = 0;
	    previous_is_sublist_head = false;

#           if MIN_USES_OBJ_AUX_STUBS
		current_stub = NULL;
		previous_stub = NULL;
#	    endif

	    if ( min::is_list_aux ( current ) )
	    {
		if ( current != min::LIST_END )
		{
		    previous_index = current_index;
		    current_index =
			min::list_aux_of ( current );
		    current = base[current_index];
		}
	    }
#           if MIN_USES_OBJ_AUX_STUBS
		else if ( min::is_stub ( current ) )
		{
		    min::stub * s =
		        min::stub_of ( current );
		    int type = min::type_of ( s );

		    if ( type == min::LIST_AUX )
		    {
		        previous_index = current_index;
			current_index = 0;
			current_stub = s;
			current = min::unprotected::
			               gen_of ( s );
		    }
		}
#           endif

	    return current;
	}
    };

} }

namespace min {

    // Inline functions.  See MIN design document.

    template < class vecpt >
    inline vecpt & vec_pointer_of
	    ( min::internal
	         ::list_pointer_type<vecpt> & lp )
    {
    	return lp.vecp;
    }

    template < class vecpt >
    inline min::gen start_hash
            ( min::internal
	         ::list_pointer_type<vecpt> & lp,
	      unsigned index )
    {
	MIN_ASSERT ( index < hash_size_of ( lp.vecp ) );
	return lp.forward
	    (   unprotected::hash_offset_of ( lp.vecp )
	      + index );
    }

    template < class vecpt >
    inline min::gen start_vector
            ( min::internal
	         ::list_pointer_type<vecpt> & lp,
	      unsigned index )
    {
	index += unprotected::attr_offset_of
	             ( lp.vecp );
	MIN_ASSERT
	    ( index < unprotected::unused_offset_of
	                  ( lp.vecp ) );
	return lp.forward ( index );
    }

    // start_copy is declared as a friend only for
    // legal compinations of list_pointers, e.g.,
    //
    // start_copy ( updatable_list_pointer & lp,
    //		    insertable_list_pointer & lp2 )
    //
    // is allowed but
    //
    // start_copy ( insertable_list_pointer & lp,
    //		    updatable_list_pointer & lp2 )
    //
    // is not allowed (not a friend).
    //
    template < class vecpt, class vecpt2 >
    inline min::gen start_copy
            ( min::internal
	         ::list_pointer_type<vecpt> & lp,
	      const
	      min::internal
	         ::list_pointer_type<vecpt2> & lp2 )
    {
        MIN_ASSERT (    stub_of ( lp.vecp )
	             == stub_of ( lp2.vecp ) );
	lp.current_index = lp2.current_index;
	lp.previous_index = lp2.previous_index;
	lp.previous_is_sublist_head =
	    lp2.previous_is_sublist_head;
#       if MIN_USES_OBJ_AUX_STUBS
	    lp.current_stub = lp2.current_stub;
	    lp.previous_stub = lp2.previous_stub;
#       endif
	return lp.current = lp2.current;
    }

    // start_sublist is declared as a friend only for
    // legal compinations of list_pointers, just like
    // start_copy above.
    //
    template < class vecpt, class vecpt2 >
    inline min::gen start_sublist
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp,
    	      const min::internal
	         ::list_pointer_type<vecpt2> & lp2 )
    {
        MIN_ASSERT (    stub_of ( lp.vecp )
	             == stub_of ( lp2.vecp ) );

	lp.previous_index = lp2.current_index;
	lp.previous_is_sublist_head = true;

#	if MIN_USES_OBJ_AUX_STUBS
	    lp.previous_stub = lp2.current_stub;
	    if ( min::is_stub ( lp2.current ) )
	    {
		lp.current_stub =
		    min::unprotected::
		         stub_of ( lp2.current );
		lp.current_index = 0;
		MIN_ASSERT
		    (    min::type_of
		             ( lp.current_stub )
		      == min::SUBLIST_AUX );
		lp.current =
		    min::unprotected::
			 gen_of ( lp.current_stub );
		return lp.current;
	    }
	    lp.current_stub = NULL;
#	endif

	lp.current_index =
	    sublist_aux_of ( lp2.current );
	if ( lp.current_index == 0 )
	    lp.current = min::LIST_END;
	else
	    lp.current =
		lp.base[lp.current_index];
	return lp.current;
    }

    template < class vecpt >
    inline min::gen next
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp )
    {
        // The code of remove ( lp ) depends upon the
	// fact that this function does not READ
	// lp.previous_stub.


        if ( lp.current == min::LIST_END )
	    return min::LIST_END;

#       if MIN_USES_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
		lp.previous_index = 0;
		lp.previous_is_sublist_head = false;
		lp.previous_stub = lp.current_stub;

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
		    lp.current_index =
		        min::unprotected::
			     value_of_control ( c );
		    lp.current_stub = NULL;
		    if ( lp.current_index == 0 )
			lp.current = min::LIST_END;
		    else
			lp.current
			    = lp.base[lp.current_index];
		    return lp.current;
		}
	    }
#       endif

	if (   lp.current_index
	     < unprotected::unused_offset_of
	           ( lp.vecp ) )
	{
	    // Current is list (not sublist) head.
	    //
	    // Previous does not exist as current is
	    // not LIST_END.
	    //
	    lp.previous_index = lp.current_index;
	    lp.current_index = 0;
	    return lp.current = min::LIST_END;
	}
	else
	    return lp.forward ( lp.current_index - 1 );
    }

    template < class vecpt >
    inline min::gen current
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp )
    {
    	return lp.current;
    }

    template < class vecpt >
    inline min::gen refresh
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp )
    {
	if ( lp.current_index != 0 )
	    return lp.current =
	    		lp.base[lp.current_index];
#       if MIN_USES_OBJ_AUX_STUBS
	    else if ( lp.current_stub != NULL )
		return lp.current =
			   min::unprotected::
			        gen_of
			          ( lp.current_stub );
#	endif
	else if ( lp.current == min::LIST_END )
	    return lp.current;

	MIN_ABORT ( "inconsistent list_pointer" );
    }

    template < class vecpt >
    inline min::gen start_sublist
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp )
    {
	return start_sublist ( lp, lp );
    }

    // update is declared as a friend only for
    // updatable_ and insertable_ list_pointers.
    //
    template < class vecpt >
    inline void update
    	    ( min::internal
	         ::list_pointer_type<vecpt> & lp,
	      min::gen value )
    {
        MIN_ASSERT ( value != min::LIST_END );
        MIN_ASSERT ( lp.current != min::LIST_END );
        MIN_ASSERT ( value == min::EMPTY_SUBLIST
	             ||
		     ! is_sublist ( value ) );

#       if MIN_USES_OBJ_AUX_STUBS
	    min::internal
	       ::collect_aux_stub ( lp.current );
	    if ( lp.current_stub != NULL )
	    {
		min::unprotected::set_gen_of
		    ( lp.current_stub, value );
		lp.current = value;
	    }
	    else
#	endif
	if ( lp.current_index != 0 )
	    lp.current =
	        lp.base[lp.current_index] = value;
	else
	{
	    MIN_ABORT ( "inconsistent list_pointer" );
	}
    }

    inline void insert_reserve
    	    ( min::insertable_list_pointer & lp,
	      unsigned insertions,
	      unsigned elements,
	      bool use_obj_aux_stubs )
    {
        if ( elements == 0 ) elements = insertions;
	MIN_ASSERT ( insertions <= elements );

	unsigned unused_size =
	      unprotected::aux_offset_of ( lp.vecp )
	    - unprotected::unused_offset_of ( lp.vecp );

	if (      unused_size
	        < 2 * insertions + elements
#	    if MIN_USES_OBJ_AUX_STUBS
	     && (    ! use_obj_aux_stubs
	          ||   min::internal
			  ::number_of_free_stubs
		     < insertions + elements )
#	    endif
	   )
	    min::internal::insert_reserve
	        ( lp, insertions, elements,
		  use_obj_aux_stubs );
	else
	{
	    lp.reserved_insertions = insertions;
	    lp.reserved_elements = elements;
#	    if MIN_USES_OBJ_AUX_STUBS
		lp.use_obj_aux_stubs =
		   use_obj_aux_stubs;
#	    endif
	}
    }
}

// Object Attribute Level
// ------ --------- -----


namespace min { namespace internal {

    // This is the generic attribute pointer type from
    // which specific attribute pointer types are made.

    template < class list_pointer_type >
        class attribute_pointer_type;

} }

namespace min { namespace unprotected {

    // Public unprotected attribute pointer types.

    typedef min::internal::attribute_pointer_type
	    < min::list_pointer >
        attribute_pointer;
    typedef min::internal::attribute_pointer_type
	    < min::insertable_list_pointer >
        writable_attribute_pointer;

} }

namespace min { 

    // We must declare these before we make them
    // friends.

    template < class list_pointer_type >
    void locate
	    ( internal::attribute_pointer_type
	          < list_pointer_type > & ap,
	      min::gen name );
    template < class list_pointer_type >
    void locatei
	    ( internal::attribute_pointer_type
	          < list_pointer_type > & ap,
	      int name );
    template < class list_pointer_type >
    void locate_reverse
	    ( internal::attribute_pointer_type
	          < list_pointer_type > & ap,
	      min::gen reverse_name );
    template < class list_pointer_type >
    void relocate
	    ( internal::attribute_pointer_type
	          < list_pointer_type > & ap );

#   if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS

	template < class list_pointer_type >
	void locate
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap,
		  unsigned & length, min::gen name );

#   endif

    template < class list_pointer_type >
    unsigned count
	    ( internal::attribute_pointer_type
	          < list_pointer_type > & ap );

    template < class list_pointer_type >
    unsigned get
	    ( min::gen * out, unsigned n,
	      internal::attribute_pointer_type
	          < list_pointer_type > & ap );

    template < class list_pointer_type >
    unsigned count_flags
	    ( internal::attribute_pointer_type
	          < list_pointer_type > & ap );

    template < class list_pointer_type >
    unsigned get_flags
	    ( min::gen * out, unsigned n,
	      internal::attribute_pointer_type
	          < list_pointer_type > & ap );
    void set
	    ( min::unprotected
	         ::writable_attribute_pointer
		  & wap,
	      const min::gen * in, unsigned n );
    void add_to_set
	    ( min::unprotected
	         ::writable_attribute_pointer
		  & wap,
	      const min::gen * in, unsigned n );
    void add_to_multiset
	    ( min::unprotected
	         ::writable_attribute_pointer
		  & wap,
	      const min::gen * in, unsigned n );

    void set_flags
	    ( min::unprotected
	         ::writable_attribute_pointer
		  & wap,
	      const min::gen * in, unsigned n );

    namespace internal {

#	if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS

	    template < class list_pointer_type >
	    void locate
		    ( internal::attribute_pointer_type
			  < list_pointer_type > & ap,
		      min::gen name,
		      bool allow_partial_label = false
		    );

#	else // ! MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS

	    template < class list_pointer_type >
	    void locate
		    ( internal::attribute_pointer_type
			  < list_pointer_type > & ap,
		      min::gen name );
#	endif

	template < class list_pointer_type >
	void relocate
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	template < class list_pointer_type >
	unsigned count
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	template < class list_pointer_type >
	unsigned get
		( min::gen * out, unsigned n,
		  internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	void set
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	void set_flags
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	void set_more_flags
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	void attribute_create
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap );

	void reverse_attribute_create
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap );
    }

}

namespace min { namespace internal {


    template < class list_pointer_type >
    class attribute_pointer_type {

    public:

        attribute_pointer_type ( min::stub * s )
	    : dlp ( s ), locate_dlp ( s ),
	      attribute_name ( min::NONE ),
	      reverse_attribute_name ( min::NONE ),
	      state ( INIT )
	{
	}

        attribute_pointer_type ( min::gen v )
	    : dlp ( v ), locate_dlp ( v ),
	      attribute_name ( min::NONE ),
	      reverse_attribute_name ( min::NONE ),
	      state ( INIT )
	{
	}

    private:

    // Private Data:

        min::gen attribute_name;
	    // The attribute name given to the last
	    // locate function call.
        min::gen reverse_attribute_name;
	    // The reverse attribute name given to the
	    // last reverse_locate function call, and
	    // also reset to NONE by a locate function
	    // call.

#	if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
	    unsigned length;
	        // Length that would be returned by the
		// last locate if that locate had a
		// length argument.
#	endif

	unsigned state;  // One of:
	    enum {
	        // Note, larger state values imply more
		// progress in setting things up.

	        INIT			= 0,
		    // No locate function called.

		 // A call to locate succeeds if it
		 // finds an attribute- or node-
		 // descriptor, and fails otherwise.
		 //
		 // A locate with a length argument
		 // succeeds if it returns length > 0
		 // and fails otherwise.

		LOCATE_FAIL		= 1,
		    // Last call to locate failed.

		// Note: states >= LOCATE_NONE imply
		// the last call to locate succeeded.

		LOCATE_NONE		= 2,
		    // Last call to locate succeeded,
		    // and no call to reverse_locate
		    // has been made or the last call
		    // to reverse_locate set the
		    // reverse_attribute to NONE.

		LOCATE_ANY		= 3,
		    // Last call to reverse_locate set
		    // the reverse attribute to ANY.

		REVERSE_LOCATE_FAIL	= 4,
		    // Last call to reverse_locate when
		    // the state was >= LOCATE_NONE set
		    // the reverse attribute to a value
		    // other than NONE or ANY and
		    // failed.

		REVERSE_LOCATE_SUCCEED	= 5
		    // Last call to reverse_locate when
		    // the state was >= LOCATE_NONE set
		    // the reverse attribute to a value
		    // other than NONE or ANY and
		    // succeeded.

	    };

	unsigned flags;
	    // All flags are zeroed when the attribute
	    // pointer is created, and remain zeroed
	    // until set by the first locate.

	    enum {

	        IN_VECTOR	= ( 1 << 0 ),
		    // On if index is attribute vector
		    // index; off it its hash table
		    // index or no locate yet.
	    };

    	list_pointer_type dlp;
	    // Descriptor list pointer.  Points at the
	    // list element containing the attribute- or
	    // node- descriptor found, unless the state
	    // is INIT, LOCATE_FAIL, or REVERSE_LOCATE_
	    // FAIL, in which case this is not set.

    	list_pointer_type locate_dlp;
	    // This is the value of dlp after the last
	    // successful locate, or the value dlp
	    // would have had if the last unsuccessful
	    // locate had a length argument, in the case
	    // where partial labels are allowed.  This
	    // last permits the `set' function to be
	    // optimized.
	    //
	    // This is not set if the state is INIT, or
	    // if the state is LOCATE_FAIL and length
	    // member does not exist or is == 0.

	unsigned index;
	    // Hash or attribute vector index passed to
	    // the start_hash or start_vector functions
	    // by the last call to locate.  See the
	    // IN_VECTOR flag above.
	        

    // Friends:

	friend void min::locate<>
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap,
		  min::gen name );
	friend void min::locatei<>
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap,
		  int name );
	friend void min::locate_reverse<>
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap,
		  min::gen reverse_name );
	friend void min::relocate<>
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap );

#	if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS

	    friend void min::locate<>
		    ( internal::attribute_pointer_type
			  < list_pointer_type > & ap,
		      unsigned & length,
		      min::gen name );

	    friend void min::internal::locate<>
		    ( internal::attribute_pointer_type
			  < list_pointer_type > & ap,
		      min::gen name,
		      bool allow_partial_labels );

#	else

	    friend void min::internal::locate<>
		    ( internal::attribute_pointer_type
			  < list_pointer_type > & ap,
		      min::gen name );

#	endif

	friend void min::internal::relocate<>
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	friend unsigned min::count<>
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	friend unsigned min::get<>
		( min::gen * out, unsigned n,
		  internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	friend unsigned min::count_flags<>
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	friend unsigned min::get_flags<>
		( min::gen * out, unsigned n,
		  internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	friend void min::set
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	friend void min::add_to_set
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	friend void min::add_to_multiset
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	friend void min::set_flags
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	friend unsigned min::internal::count<>
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	friend unsigned min::internal::get<>
		( min::gen * out, unsigned n,
		  internal::attribute_pointer_type
		      < list_pointer_type > & ap );

	friend void min::internal::set
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	friend void min::internal::set_flags
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	friend void min::internal::set_more_flags
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap,
		  const min::gen * in, unsigned n );

	friend void min::internal::attribute_create
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap );

	friend void min::internal
	               ::reverse_attribute_create
		( min::unprotected
		     ::writable_attribute_pointer
		      & wap );

    };

} }

namespace min {

    // Inline functions.  See MIN design document.

    template < class list_pointer_type >
    inline void locatei
	    ( internal::attribute_pointer_type
		  < list_pointer_type > & ap,
	      int name )
    {
	typedef internal::attribute_pointer_type
		    < list_pointer_type > ap_type;

	ap.attribute_name = min::new_num_gen ( name );

	if ( 0 <= name
	     &&
	     name < min::attr_size_of
	                ( min::vec_pointer_of
			      ( ap.dlp ) ) )
	{
	    start_vector ( ap.dlp, name );
	    start_copy ( ap.locate_dlp, ap.dlp );

	    ap.index = name;
	    ap.flags = ap_type::IN_VECTOR;
	    ap.state = ap_type::LOCATE_NONE;
	    ap.reverse_attribute_name = min::NONE;

#	    if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS
		ap.length = 1;
#	    endif

	    return;
	}
	
	internal::locate ( ap, ap.attribute_name );
    }

    template < class list_pointer_type >
    inline void locate
	    ( internal::attribute_pointer_type
		  < list_pointer_type > & ap,
	      min::gen name )
    {

	// We only handle the case of vector elements
	// inline.
	//
	if ( is_num ( name ) )
	{
	    float64 f = float_of ( name );
	    int i = (int) f;
	    if ( i == f )
	    {
		locatei ( ap, i );
		return;
	    }
	}
	internal::locate ( ap, name );
    }

#   if MIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS

	template < class list_pointer_type >
	inline void locate
		( internal::attribute_pointer_type
		      < list_pointer_type > & ap,
		  unsigned & length,
		  min::gen name )
	{

	    // We only handle the case of vector
	    // elements inline.
	    //
	    if ( is_num ( name ) )
	    {
		float64 f = float_of ( name );
		int i = (int) f;
		if ( i == f )
		{
		    // Because name has only one
		    // component, there is no difference
		    // between
		    // internal::locate
		    //	   ( ap, name, true )
		    // and
		    // internal::locate
		    //	   ( ap, name, false ).
		    //
		    locatei ( ap, i );
		    return;
		}
	    }
	    internal::locate ( ap, name, true );
	    length = ap.length;
	}

#   endif

    // In most the following functions either
    // refresh ( {w}ap.dlp ) or
    // refresh ( {w}ap.locate_dlp )
    // must be called before calling any internal
    // function.

    template < class list_pointer_type >
    inline unsigned count
	    ( internal::attribute_pointer_type
	          < list_pointer_type > & ap )
    {
	typedef internal::attribute_pointer_type
		    < list_pointer_type > ap_type;

	min::gen c = refresh ( ap.dlp );
	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
		return 0;
	case ap_type::LOCATE_NONE:
	case ap_type::REVERSE_LOCATE_SUCCEED:
		if ( ! is_sublist ( c ) ) return 1;
	}
	return internal::count ( ap );
    }

    template < class list_pointer_type >
    inline unsigned get
	    ( min::gen * out, unsigned n,
	      internal::attribute_pointer_type
	          < list_pointer_type > & ap )
    {
	typedef internal::attribute_pointer_type
		    < list_pointer_type > ap_type;

	if ( n == 0 ) return 0;
	min::gen c =  refresh ( ap.dlp );
	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
		return 0;
	case ap_type::LOCATE_NONE:
	case ap_type::REVERSE_LOCATE_SUCCEED:
		if ( ! is_sublist ( c ) )
		{
		    out[0] = c;
		    return 1;
		}
	}
	internal::get ( out, n, ap );
    }

    template < class list_pointer_type >
    inline unsigned count_flags
	    ( internal::attribute_pointer_type
	          < list_pointer_type > & ap )
    {
	typedef internal::attribute_pointer_type
		    < list_pointer_type > ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_FAIL:
		return 0;
	}

	min::gen c = refresh ( ap.locate_dlp );
	if ( ! is_sublist ( c ) ) return 0;
	list_pointer lp
	    ( vec_pointer_of ( ap.locate_dlp ) );
	start_sublist ( lp, ap.locate_dlp );
	for ( c = current ( lp );
	      is_sublist ( c );
	      c = next ( lp ) );
	unsigned result = 0;
	for ( ; is_control_code ( c ); c = next ( lp ) )
	    ++ result;
	return result;
    }

    template < class list_pointer_type >
    inline unsigned get_flags
	    ( min::gen * out, unsigned n,
	      internal::attribute_pointer_type
	          < list_pointer_type > & ap )
    {
	typedef internal::attribute_pointer_type
		    < list_pointer_type > ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_FAIL:
		return 0;
	}

	min::gen c =  refresh ( ap.locate_dlp );
	if ( ! is_sublist ( c ) ) return 0;
	list_pointer lp
	    ( vec_pointer_of ( ap.locate_dlp ) );
	start_sublist ( lp, ap.locate_dlp );
	for ( c = current ( lp );
	      is_sublist ( c );
	      c = next ( lp ) );
	unsigned result = 0;
	for ( ; result < n && is_control_code ( c );
	        c = next ( lp ) )
	    ++ result, * out ++ = c;
	return result;
    }

    inline void set
	    ( min::unprotected
		 ::writable_attribute_pointer & wap,
	      const min::gen * in, unsigned n )
    {
	typedef unprotected::writable_attribute_pointer
		    ap_type;

	min::gen c =  refresh ( wap.dlp );
	if ( n == 1 ) switch ( wap.state )
	{
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::LOCATE_NONE:
		if ( ! is_sublist ( c ) )
		{
		    update ( wap.dlp, * in );
		    return;
		}
	}

	internal::set ( wap, in, n  ); 
    }

    inline void set_flags
	    ( min::unprotected
		 ::writable_attribute_pointer & wap,
	      const min::gen * in, unsigned n )
    {
	typedef unprotected::writable_attribute_pointer
		    ap_type;

	min::gen c = refresh ( wap.locate_dlp );
	switch ( wap.state )
	{
	case ap_type::LOCATE_NONE:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::REVERSE_LOCATE_FAIL:
		if ( is_sublist ( c ) )
		{
		    updatable_list_pointer
		        lp ( vec_pointer_of
				( wap.locate_dlp ) );
		    start_sublist
		        ( lp, wap.locate_dlp );
		    for ( c = current ( lp );
			  is_sublist ( c );
			  c = next ( lp ) );
		    for ( ;    n > 0
		            && is_control_code ( c );
			   -- n, c = next ( lp ) )
		    {
			MIN_ASSERT
			    ( is_control_code
			          ( * in ) );
			update ( lp, * in ++ );
		    }
		    if ( n > 0 )
		        internal::set_more_flags
			    ( wap, in, n );
		    else for ( ; is_control_code ( c );
		                 c = next ( lp ) )
		        update ( lp,
			         new_control_code_gen
				     ( 0 ) );
		    return;
		}
	}

	internal::set_flags ( wap, in, n );
    }

}

// TBD
// ---

namespace min {
}

# endif // MIN_H
