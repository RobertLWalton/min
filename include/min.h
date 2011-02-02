// MIN Language Interface
//
// File:	min.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Feb  2 11:18:27 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/08/06 21:25:53 $
//   $RCSfile: min.h,v $
//   $Revision: 1.371 $

// Table of Contents:
//
//	Setup
//	Initialization
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
//	Names
//	Packed Structures
//	Packed Vectors
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level
//	Printers
//	Printing
//	More Allocator/Collector/Compactor Interface

// Namespaces:
//
//   			Abbreviation
//   Full Name		      Use and defining file.
//
//   min		min   Public protected min.h.
//   min::unprotected	MUP   Public unprotected min.h.
//   min::internal	MINT  Private min.h.
//   min::os		MOS   Private min_os.h.
//   min::acc		MACC  Private min_acc.h.

// Setup
// -----

# ifndef MIN_H
# define MIN_H

// Include parameters.
//
# include "min_parameters.h"
# include <iostream>
# include <climits>
# include <cstring>
# include <cassert>
# include <new>

#define MIN_ABORT(string) assert ( ! string )
#define MIN_REQUIRE(expr) assert ( expr )

namespace min { namespace internal {

    // Return j such that (1<<(j-1)) < u <= (1<<j),
    // assuming 0 < u <= (1<<31).
    //
    inline unsigned log2ceil ( unsigned u )
    {
        assert ( u != 0 );
        return u == 1 ? 0
	              : 1 + log2floor ( u - 1 );
    }

} }



// Initialization
// --------------

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

#   if MIN_PTR_BITS <= 32
	typedef uns32 unsptr;
	typedef int32 intptr;
#   else
	typedef uns64 unsptr;
	typedef int64 intptr;
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
#   elif MIN_IS_LOOSE
	typedef uns64 gen;
#   endif


#   if MIN_IS_COMPACT

	// General Value Subtype (high order 8 bits)
	//   0x00-0xDF	stubs
	//   0xE0-0xEF  direct integers
	//   0xF0-0xF7  direct string and other
	//   0xF8-0xFC  illegal
	//   0xFD-0xFF  auxiliary pointers

	const unsigned GEN_STUB
	    = 0;
	// Unimplemented for COMPACT:
	//   unsigned GEN_DIRECT_FLOAT
	const unsigned GEN_DIRECT_INT
	    = 0xE0;
	const unsigned GEN_DIRECT_STR
	    = 0xF0;
	const unsigned GEN_INDEX
	    = 0xF1;
	const unsigned GEN_CONTROL_CODE
	    = 0xF2;
	const unsigned GEN_SPECIAL
	    = 0xF3;
	const unsigned GEN_ILLEGAL
	    = 0xF4;  // First illegal subtype code
	             // below GEN_AUX_LOWER.

	// Auxiliary pointer subtype codes are at the
	// end.
	//
	const unsigned GEN_AUX_LOWER
	    = 0xFD;  // Lowest AUX subtype code.
	const unsigned GEN_INDIRECT_AUX
	    = 0xFD;
	const unsigned GEN_SUBLIST_AUX
	    = 0xFE;
	const unsigned GEN_LIST_AUX
	    = 0xFF;

	const unsigned GEN_UPPER
	    = 0xFF; // Largest subtype code.

	typedef uns32 unsgen;
	    // Unsigned type convertable to a min::gen
	    // value.
	const unsigned TSIZE = 8;
	const unsigned VSIZE = 24;
	    // Sized of type and value field in a
	    // min::gen.
#	define MIN_AMAX 0xDFFFFFFFu
	    // Maximum packed stub address value in
	    // general value.

#   elif MIN_IS_LOOSE

	// General Value Subtype (high order 24 bits)
	// with base MIN_FLOAT_SIGNALLING_NAN:
	//
	//   0x00-0x0F	stub
	//   0x10-0x13	direct string and others
	//   0x14-0x1C	illegal
	//   0x1D-0x1F	auxiliary pointers
	//   other	floating point

	const unsigned GEN_STUB
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x00;
	const unsigned GEN_DIRECT_FLOAT
	    = 0;
	// Unimplemented for LOOSE:
	//   unsigned GEN_DIRECT_INT // illegal
	const unsigned GEN_DIRECT_STR
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x10;
	const unsigned GEN_INDEX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x11;
	const unsigned GEN_CONTROL_CODE
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x12;
	const unsigned GEN_SPECIAL
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x13;
	const unsigned GEN_ILLEGAL
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x14;
	    // First illegal subtype code below
	    // aux subtype codes.

	// Auxiliary pointer subtype codes are at the
	// end.
	//
	const unsigned GEN_AUX_LOWER
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1D;
	const unsigned GEN_INDIRECT_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1D;
	const unsigned GEN_SUBLIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1E;
	const unsigned GEN_LIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1F;

	const unsigned GEN_UPPER
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1F;
	    // Largest subtype code.

	typedef uns64 unsgen;
	    // Unsigned type convertable to a min::gen
	    // value.
	const unsigned TSIZE = 24;
	const unsigned VSIZE = 40;
	    // Sized of type and value field in a
	    // min::gen.

#	define MIN_AMAX 0xFFFFFFFFFFFull
	    // Maximum packed stub address value in
	    // general value.

#   endif

    namespace internal {
	const unsgen VMASK =
	    ( (unsgen) 1 << VSIZE ) - 1;
	    // Value mask.
	const unsgen VHALFMASK =
	    ( (unsgen) 1 << ( VSIZE / 2 ) ) - 1;
	    // Half value mask.
	const unsgen VHIHALFMASK =
	    VHALFMASK << ( VSIZE / 2 );
	    // Ditto for upper half of VSIZE datum.
#       if MIN_IS_LOOSE
	    const unsgen AMASK =
		( (unsgen) 1 << ( VSIZE + 4 ) ) - 1;
		// Mask for packed stub address in
		// general value.
#	endif
    }

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

    const unsigned SPECIAL_NAME_LENGTH = 7;
    extern const char * special_name
                            [SPECIAL_NAME_LENGTH];
        // special_name[0xFFFFFF-i] is the name of
	// MIN_NEW_SPECIAL_GEN(i).  E.g.,
	// special_name[0] == "MISSING".
}

// Stub Types and Data
// ---- ----- --- ----

namespace min {

    // Stub type codes.

    // Collectable.
    //
    const int ACC_FREE			= 1;
    const int ACC_MARK			= 2;
    const int DEALLOCATED		= 3;
    const int NUMBER			= 4;
    const int SHORT_STR			= 5;
    const int LONG_STR			= 6;
    const int LABEL			= 7;
    const int TINY_OBJ			= 8;
        // Unimplemented.
    const int SHORT_OBJ			= 9;
    const int LONG_OBJ			= 10;
    const int HUGE_OBJ			= 11;
        // Unimplemented.
    const int PACKED_STRUCT		= 12;
    const int PACKED_VEC		= 13;

    // Uncollectible.
    //
    const int AUX_FREE			= -1;
    const int LABEL_AUX			= -2;
    const int LIST_AUX			= -3;
    const int SUBLIST_AUX		= -4;
    const int HASHTABLE_AUX		= -5;
    const int RELOCATE_BODY		= -6;

    extern const char ** type_name;
        // type_name[i] is the name of type i.  E.g.,
	// type_name[1] == "ACC_FREE" and
	// type_name[-1] == "AUX_FREE".

    namespace unprotected {
	// Flags for non-acc control values.
	//
	const min::uns64 STUB_PTR = min::uns64(1) << 55;
	    // Indicates value part of control value is
	    // a packed stub address.
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

#   if MIN_PTR_BITS <= 32
	// For the 32 bit pointer size there is no point
	// in having a non-zero stub base.
#	ifndef MIN_STUB_BASE
#	    define MIN_STUB_BASE 0
#	endif
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
	const min::unsptr stub_base =
	    MIN_STUB_BASE;
	min::stub * const null_stub =
	    (min::stub *) MIN_STUB_BASE;
#   else
	extern min::unsptr stub_base;
	extern min::stub * null_stub;
#   endif

    inline void * uns64_to_ptr ( min::uns64 v )
    {
	return (void *) (min::unsptr) v;
    }
    inline min::uns64 ptr_to_uns64 ( void * p )
    {
	return (min::uns64) (min::unsptr) p;
    }

    // Given a ref to an uns64 we need to convert it to
    // a ref to a pointer.

#   if MIN_PTR_BITS <= 32
	inline void * & uns64_ref_to_ptr_ref
	    ( min::uns64 & v )
	{
	    return ( (void **) & v )[MIN_IS_BIG_ENDIAN];
	}
#   else
	inline void * & uns64_ref_to_ptr_ref
	    ( min::uns64 & v )
	{
	    return * (void **) & v;
	}
#   endif

    // We need to be able to pack/unpack a stub address
    // into/from the appropriate part of a general
    // value.  The packed stub address is stored in an
    // unsgen value that equals the min::gen value
    // stripped of its subtype.

#   if MIN_IS_COMPACT

	inline min::stub * unsgen_to_stub
		( min::unsgen v )
	{
	    min::unsptr p = (min::unsptr) v;
#           if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
               <= MIN_AMAX
		return (min::stub *) p;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
                 <= MIN_AMAX
		return (min::stub *) ( p + stub_base );
#           elif MIN_MAX_STUB_INDEX <= MIN_AMAX
		return (min::stub *) stub_base + p;
#           else
#	        error   MIN_MAX_STUB_INDEX > MIN_AMAX
#           endif
	}
	inline min::unsgen stub_to_unsgen
		( const min::stub * s )
	{
#           if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
               <= MIN_AMAX
	        return (min::unsptr) s;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
                 <= MIN_AMAX
	        return   (min::unsptr) s - stub_base;
#           elif MIN_MAX_STUB_INDEX <= MIN_AMAX
	        return s - (min::stub *) stub_base;
#           else
#	        error   MIN_MAX_STUB_INDEX > MIN_AMAX
#           endif
	}
#   elif MIN_IS_LOOSE
        inline min::stub * unsgen_to_stub
	        ( min::unsgen v )
        {
#           if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
               <= MIN_AMAX
	        return (min::stub *) v;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
                 <= MIN_AMAX
	        return (min::stub *) ( v + stub_base );
#           elif MIN_MAX_STUB_INDEX <= MIN_AMAX
	        return (min::stub *) stub_base + v;
#           else
#	        error   MIN_MAX_STUB_INDEX > MIN_AMAX
#           endif
        }

        inline min::unsgen stub_to_unsgen
	        ( const min::stub * s )
        {
#           if    MIN_MAX_ABSOLUTE_STUB_ADDRESS \
               <= MIN_AMAX
	        return (min::unsptr) s;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
                 <= MIN_AMAX
	        return   (min::unsptr) s - stub_base;
#           elif MIN_MAX_STUB_INDEX <= MIN_AMAX
	        return s - (min::stub *) stub_base;
#           else
#	        error   MIN_MAX_STUB_INDEX > MIN_AMAX
#           endif
        }

	// Insert packed stub address into general
	// value.
	//
        inline min::gen stub_into_gen
	        ( min::gen v, const min::stub * s )
        {
	    v &= ~ ( ( min::uns64(1) << 44 ) - 1 );
	    return v + stub_to_unsgen ( s );
        }
#   endif
} }

// General Value Constructor Functions
// ------- ----- ----------- ---------

namespace min { namespace internal {

    // Helper function.

    inline min::unsptr strnlen
	    ( const char * p, min::unsptr n )
    {
        const char * q = p;
	while ( * q && n > 0 ) ++ q, -- n;
	return q - p;
    }
} }

namespace min { namespace unprotected {

    // MUP:: constructors

#   if MIN_IS_COMPACT
	inline min::gen new_gen ( const min::stub * s )
	{
	    return (min::gen)
	        internal::stub_to_unsgen ( s );
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
		( const char * p, min::unsptr n )
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
	inline min::gen new_gen ( const min::stub * s )
	{
	    return (min::gen)
		   (   internal::stub_to_unsgen ( s )
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
		( const char * p, min::unsptr n )
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
    inline min::gen new_indirect_aux_gen
	    ( unsgen p )
    {
	return (min::gen)
	       (   p
		 + ( (unsgen) GEN_INDIRECT_AUX
		     << VSIZE ) );
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
	    ( ( (unsgen ) v & ~ internal::VMASK )
	      + p );
    }
} }

namespace min {

    // min:: constructors

    inline min::gen new_gen ( const min::stub * s )
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
	    ( const char * p, min::unsptr n )
    {
#       if MIN_IS_COMPACT
	    MIN_ASSERT
	        ( internal::strnlen ( p, n ) <= 3 );
#	elif MIN_IS_LOOSE
	    MIN_ASSERT
	        ( internal::strnlen ( p, n ) <= 5 );
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
    inline min::gen new_indirect_aux_gen
	    ( unsgen p )
    {
	MIN_ASSERT ( p < (unsgen) 1 << VSIZE );
	return unprotected::new_indirect_aux_gen
			( p );
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
	    return   (min::unsgen) v
	           < ( GEN_DIRECT_INT << VSIZE );
	}
	// Unimplemented for COMPACT:
	//  bool is_direct_float ( min::gen v )
	inline bool is_direct_int ( min::gen v )
	{
	    return ( v >> 28 == GEN_DIRECT_INT >> 4 );
	}
	inline unsigned gen_subtype_of ( min::gen v )
	{
	    v = (min::unsgen) v >> VSIZE;
	    if ( v < GEN_DIRECT_INT )
	        return GEN_STUB;
	    else if ( v < GEN_DIRECT_STR)
	        return GEN_DIRECT_INT;
	    else if ( v < GEN_ILLEGAL)
	        return v;
	    else if ( v < GEN_AUX_LOWER)
	        return GEN_ILLEGAL;
	    else
	        return v;
	}
	inline bool is_aux ( min::gen v )
	{
	    return (min::unsgen) v
	           >= (GEN_AUX_LOWER << VSIZE);
	}
#   elif MIN_IS_LOOSE
	inline bool is_stub ( min::gen v )
	{
	    return    (    (min::unsgen) v
	                >> ( VSIZE + 4 ) )
	           == GEN_STUB >> 4;
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
	    else if ( v < GEN_AUX_LOWER)
	        return GEN_ILLEGAL;
	    else if ( v <= GEN_UPPER)
	        return v;
	    else
	        return GEN_DIRECT_FLOAT;
	}
	inline bool is_aux ( min::gen v )
	{
	    min::unsgen subtype =
	        (min::unsgen) v >> VSIZE;
	    return GEN_AUX_LOWER <= subtype
	           &&
		   subtype <= GEN_UPPER;
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
    inline bool is_indirect_aux ( min::gen v )
    {
	return
	    (    (unsgen) v >> VSIZE
	      == GEN_INDIRECT_AUX );
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
	    return internal::unsgen_to_stub ( v );
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
		return ( uns64 ( v ) << TSIZE );
#	    elif MIN_IS_LITTLE_ENDIAN
		return ( uns64 ( v & 0xFFFFFF ) );
#	    endif
	}
#   elif MIN_IS_LOOSE
	inline min::stub * stub_of ( min::gen v )
	{
	    return internal::unsgen_to_stub
	    		( v & min::internal::AMASK );
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
		return ( v << TSIZE );
#	    elif MIN_IS_LITTLE_ENDIAN
		return ( v & min::internal::VMASK );
#	    endif
	}
#   endif

    inline unsgen aux_of ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
    }
    inline unsgen list_aux_of ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
    }
    inline unsgen sublist_aux_of ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
    }
    inline unsgen indirect_aux_of
	    ( min::gen v )
    {
	return (unsgen) v & internal::VMASK;
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

    inline const min::stub * stub_of ( min::gen v )
    {
	MIN_ASSERT ( is_stub ( v ) );
	return unprotected::stub_of ( v );
    }
#   if MIN_IS_COMPACT
	inline int direct_int_of ( min::gen v )
	{
	    MIN_ASSERT ( is_direct_int ( v ) );
	    return unprotected::direct_int_of ( v );
	}
#   elif MIN_IS_LOOSE
	inline float64 direct_float_of ( min::gen v )
	{
	    MIN_ASSERT ( is_direct_float ( v ) );
	    return unprotected::direct_float_of ( v );
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
    inline unsgen indirect_aux_of
	    ( min::gen v )
    {
	MIN_ASSERT ( is_indirect_aux ( v ) );
	return unprotected::indirect_aux_of ( v );
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

// Acc control values are used as the second word of
// garbage collectible stubs.
//
// Non-acc control values are used as the second word
// of non-garbage-collectible stubs, and as the first
// word of object bodies (actually, the body proper
// begins right after this word).  The acc may use them
// elsewhere (see min_acc.h).

// Acc control value layout:
//
//	Bits		Use
//
//	0 .. m-1	Stub absolute or relative
//			address or stub index.
//
//	m .. 55		acc flag bits
//
//	56 .. 63	Type code.
//
// Non-acc control value layout:
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
// CONTROL MASK is 2**48 - 1.  These are used in ifdefs
// and must be macro constants.
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

    inline min::unsptr value_of_control
	    ( min::uns64 c )
    {
	return min::unsptr
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
	    return (min::stub *)
	           (min::unsptr)
	           (c & MIN_CONTROL_VALUE_MASK );
	}

	inline min::uns64 new_control_with_type
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
		   (min::unsptr) s
		   |
		   flags;
	}

	inline min::uns64 new_control_with_locator
		( int locator, const min::stub * s )
	{
	    return ( min::uns64 ( locator ) << 48 )
		   |
		   (min::unsptr) s;
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return ( c & ~ MIN_CONTROL_VALUE_MASK )
		   | (min::unsptr) s;
	}

#   elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
         <= MIN_CONTROL_VALUE_MASK

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    min::unsptr p =
	       (min::unsptr)
	       (c & MIN_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           ( p + min::internal::stub_base );
	}

	inline min::uns64 new_control_with_type
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
	           (   (min::unsptr) s
	             - min::internal::stub_base )
		   |
		   flags;
	}

	inline min::uns64 new_control_with_locator
		( int locator, const min::stub * s )
	{
	    return ( min::uns64 ( locator ) << 48 )
		   |
	           (   (min::unsptr) s
	             - min::internal::stub_base );
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return ( c & ~ MIN_CONTROL_VALUE_MASK )
		   | (   (min::unsptr) s
		       - min::internal::stub_base );
	}

#   elif    MIN_MAX_STUB_INDEX \
         <= MIN_CONTROL_VALUE_MASK

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    min::unsptr p =
	       (min::unsptr)
	       (c & MIN_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           min::internal::stub_base + p;
	}

	inline min::uns64 new_control_with_type
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return   ( min::uns64 ( type_code ) << 56 )
	           | ( s - (min::stub *)
		           min::internal::stub_base )
		   | flags;
	}

	inline min::uns64 new_control_with_locator
		( int locator, const min::stub * s )
	{
	    return   ( min::uns64 ( locator ) << 48 )
	           | ( s - (min::stub *)
		           min::internal::stub_base );
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, const min::stub * s )
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
	    return (min::stub *)
	           (min::unsptr)
	           (c & MIN_ACC_CONTROL_VALUE_MASK );
	}

	inline min::uns64 new_acc_control
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( min::uns64 ( type_code ) << 56 )
		   |
		   (min::unsptr) s
		   |
		   flags;
	}

	inline min::uns64 renew_acc_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return ( c & ~ MIN_ACC_CONTROL_VALUE_MASK )
		   | (min::unsptr) s;
	}

#   elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
         <= MIN_ACC_CONTROL_VALUE_MASK

	inline min::stub * stub_of_acc_control
		( min::uns64 c )
	{
	    min::unsptr p =
	       (min::unsptr)
	       (c & MIN_ACC_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           ( p + min::internal::stub_base );
	}

	inline min::uns64 new_acc_control
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return   ( min::uns64 ( type_code ) << 56 )
		   | (   (min::unsptr) s
	               - min::internal::stub_base )
		   | flags;
	}

	inline min::uns64 renew_acc_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return ( c & ~ MIN_ACC_CONTROL_VALUE_MASK )
		 | (   (min::unsptr) s
		     - min::internal::stub_base );
	}

#   elif    MIN_MAX_STUB_INDEX \
         <= MIN_ACC_CONTROL_VALUE_MASK

	inline min::stub * stub_of_acc_control
		( min::uns64 c )
	{
	    min::unsptr p =
	       (min::unsptr)
	       (c & MIN_ACC_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           min::internal::stub_base + p;
	}

	inline min::uns64 new_acc_control
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return   ( min::uns64 ( type_code ) << 56 )
	           | ( s - (min::stub *)
		           min::internal::stub_base )
		   | flags;
	}

	inline min::uns64 renew_acc_control_stub
		( min::uns64 c, const min::stub * s )
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

    inline int type_of ( const min::stub * s )
    {
        return s->c.i8[7*MIN_IS_LITTLE_ENDIAN];
    }

    inline bool is_collectible ( int type )
    {
    	return type >= 0;
    }

    inline bool is_deallocated ( const min::stub * s )
    {
        return type_of ( s ) == min::DEALLOCATED;
    }

    namespace unprotected {

        inline min::uns64 value_of
		( const min::stub * s )
	{
	    return s->v.u64;
	}

        inline min::float64 float_of
		( const min::stub * s )
	{
	    return s->v.f64;
	}

        inline min::gen gen_of ( const min::stub * s )
	{
	    return s->v.g;
	}

        inline void * ptr_of ( const min::stub * s )
	{
	    return min::internal::
	           uns64_to_ptr ( s->v.u64 );
	}

        inline void * & ptr_ref_of ( min::stub * s )
	{
	    return min::internal::
	           uns64_ref_to_ptr_ref
		       ( s->v.u64 );
	}

        inline min::uns64 control_of
	    ( const min::stub * s )
	{
	    return s->c.u64;
	}

        inline bool test_flags_of
		( const min::stub * s,
		  min::uns64 flags )
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

        inline void set_ptr_of
		( min::stub * s, void * p )
	{
	    s->v.u64 = min::internal::
	                    ptr_to_uns64 ( p );
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

namespace min { namespace internal {


    extern bool relocated_flag;
	// On if bodies have been relocated.

    extern min::stub ** acc_stack;
    extern min::stub ** acc_stack_limit;
        // acc_stack points at the first unused location
	// in the acc stack.
	//
        // If acc_stack >= acc_stack_limit, an interrupt
	// is invoked.  Min::stub * values can be pushed
	// into the acc stack, increasing the acc_stack
	// value.  The acc_stack_limit is much less than
	// the actual end of the acc stack.  Interrupts
	// may be scheduled by setting acc_stack_limit
	// =< acc_stack, and work that leads to inter-
	// rupts may be counted by decreasing acc_stack_
	// limit.
	//
	// Interrupts perform acc actions such as
	// emptying the acc_stack (and resetting acc_
	// stack_limit), and may perform thread
	// switches.

    // Out of line function to execute interrupt.
    // Returns true.
    //
    bool interrupt ( void );

} }

namespace min {

    inline bool interrupt ( void )
    {
        if (    internal::acc_stack
	     >= internal::acc_stack_limit )
	    return internal::interrupt();
	else return false;
    }

    inline bool relocated_flag ( void )
    {
         return internal::relocated_flag;
    }
    inline bool set_relocated_flag ( bool value )
    {
         bool old_value =
	     internal::relocated_flag;
	 internal::relocated_flag =
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

    namespace internal {

        struct gen_locator
	{
	    gen_locator * previous;
	    min::unsptr length;
	    min::gen * values;
	};

        struct stub_locator
	{
	    stub_locator * previous;
	    min::unsptr length;
	    const min::stub ** values;
	};

        extern gen_locator * static_gen_last;
        extern gen_locator * stack_gen_last;
        extern stub_locator * static_stub_last;
        extern stub_locator * stack_stub_last;

	template < min::unsptr len,
	           gen_locator * & last >
	struct non_num_gen_vec
	{
	    min::internal::gen_locator locator;
	    min::gen values[len];

	    non_num_gen_vec ( void )
	    {
		this->locator.length = len;
		this->locator.values = values;
		this->locator.previous = last;
		last = & this->locator;
		for ( min::unsptr i = 0; i < len; ++ i )
		    values[i] = min::MISSING;
	    }

	    ~ non_num_gen_vec ( void )
	    {
		last = this->locator.previous;
	    }

	    min::gen & operator[] ( min::unsptr i )
	    {
		MIN_ASSERT ( i < len );
		return values[i];
	    }
	};

#       if MIN_IS_LOOSE
	    template < min::unsptr len,
		       gen_locator * & last >
	    struct num_gen_vec
	    {
		min::gen values[len];

		num_gen_vec ( void )
		{
		    for ( min::unsptr i = 0;
		          i < len; ++ i )
			values[i] = min::MISSING;
		}

		min::gen & operator[] ( min::unsptr i )
		{
		    MIN_ASSERT ( i < len );
		    return values[i];
		}
	    };
#       endif

	template < min::unsptr len,
	           stub_locator * & last >
	struct stub_vec
	{
	    min::internal::stub_locator locator;
	    const min::stub * values[len];

	    stub_vec ( void )
	    {
		this->locator.length = len;
		this->locator.values = values;
		this->locator.previous = last;
		last = & this->locator;
		for ( min::unsptr i = 0; i < len; ++ i )
		    values[i] = NULL;
	    }

	    ~ stub_vec ( void )
	    {
		last = this->locator.previous;
	    }

	    const min::stub * & operator[]
		    ( min::unsptr i )
	    {
		MIN_ASSERT ( i < len );
		return values[i];
	    }
	};
    }

    template < min::unsptr len >
	struct static_gen : internal::non_num_gen_vec
		<len,internal::static_gen_last> {};
    template < min::unsptr len >
	struct stack_gen : internal::non_num_gen_vec
		<len,internal::stack_gen_last> {};

#   if MIN_IS_LOOSE
	template < min::unsptr len >
	  struct static_num_gen : internal::num_gen_vec
		<len,internal::static_gen_last> {};
	template < min::unsptr len >
	  struct stack_num_gen : internal::num_gen_vec
		<len,internal::stack_gen_last> {};
#   else // if MIN_IS_COMPACT
	template < min::unsptr len >
	  struct static_num_gen
	      : internal::non_num_gen_vec
		    <len,internal::static_gen_last> {};
	template < min::unsptr len >
	  struct stack_num_gen
	      : internal::non_num_gen_vec
		    <len,internal::stack_gen_last> {};
#   endif

    template < min::unsptr len >
	struct static_stub : internal::stub_vec
		<len,internal::static_stub_last> {};
    template < min::unsptr len >
	struct stack_stub : internal::stub_vec
		<len,internal::stack_stub_last> {};

}

namespace min { namespace internal {

    // Function called by the initializer (see `Initial-
    // ization' above) to initialize the acc.
    //
    void acc_initializer ( void );

    // Number of free stubs that can be allocated with-
    // out requiring a call to acc_expand_stub_free_
    // list.
    //
    extern min::unsptr number_of_free_stubs;

    // Out of line function to increase the number of
    // free stubs to be at least n.
    //
    void acc_expand_stub_free_list ( min::unsptr n );

    // Hash tables for atoms.  There are two hash tables
    // for every kind of atom: the acc hash table and
    // the aux hash table.
    //
    // The elements of the acc hash table are the stubs
    // hashed, chained together by their stub control
    // word stub pointers.  The stubs listed in this
    // hash table are not in the normal acc list.
    //
    // The elements of the aux hash table are HASHTABLE_
    // AUX aux stubs whose values point at the stubs of
    // ephemeral objects hashed.  The stubs hashed in
    // the aux table are also on the acc list.
    //
    // Xxx_hash_sizes are powers of 2, and each xxx_
    // hash_mask = xxx_hash_size - 1;  The acc and aux
    // hash tables for a stub type have the same size.
    // Stubs are put in the list headed by the table
    // element whose index is the stub's hash value
    // masked by the xxx_hash_mask.  Lists are ended
    // by a pointer to MINT::null_stub.
    //
    // When a hashed stub is created it is ephemeral
    // and is put in the aux hash table.  The acc moves
    // stubs from the aux hash table to the correspond-
    // ing acc hash table.

    extern min::stub ** str_acc_hash;
    extern min::stub ** str_aux_hash;
    extern min::unsptr str_hash_size;
    extern min::unsptr str_hash_mask;

#   if MIN_IS_COMPACT
	extern min::stub ** num_acc_hash;
	extern min::stub ** num_aux_hash;
	extern min::unsptr num_hash_size;
	extern min::unsptr num_hash_mask;
#   endif

    extern min::stub ** lab_acc_hash;
    extern min::stub ** lab_aux_hash;
    extern min::unsptr lab_hash_size;
    extern min::unsptr lab_hash_mask;

    // ACC flags to be set and cleared when a stub is
    // found in the hash table and returned as a
    // `newly allocated stub'.
    //
    extern min::uns64 hash_acc_set_flags;
    extern min::uns64 hash_acc_clear_flags;

    // Acc flags are bits 55 .. m of an acc control
    // value, where 56 - m == MIN_ACC_FLAG_BITS.
    //
    // The acc flag bit layout (bit 0 is lowest order)
    // is:
    //
    //	Bit		Use
    //
    //	0		    Fixed Body Flag
    //
    //	1		    Reserved
    //
    //	2, ..., 2+p-1	    Flag Pair Low Order Bits
    //
    //	2+p, ..., 2+2p-1    Flag Pair High Order Bits
    //
    // p = ACC_FLAG_PAIRS = number of flag pairs
    //			  = ( MIN_ACC_FLAG_BITS - 2 )
    //			    / 2
    //
    const uns64 ACC_FLAG_MASK =
           ( (uns64(1) << MIN_ACC_FLAG_BITS) - 1 )
	<< ( 56 - MIN_ACC_FLAG_BITS );
    const uns64 ACC_FIXED_BODY_FLAG =
	( uns64(1) << ( 56 - MIN_ACC_FLAG_BITS ) );
    const unsigned ACC_FLAG_PAIRS =
        ( MIN_ACC_FLAG_BITS - 2 ) / 2;

} }

namespace min { namespace internal {

    // acc_write_update ( s1, s2 ) checks whether
    //
    //		flags of *s2
    //		&
    //		( flags of *s1 >> ACC_FLAG_PAIRS )
    //		&
    //		acc_stack_mask
    //
    // is non-zero, and if so, pushes first s1 and then
    // s2 into the acc_stack.  This call is made when
    // a pointer to s2 is stored in the s1 object, and
    // the acc_stack is used by the collector to adjust
    // the marks it makes on objects.
    //
    // For efficiency, acc_stack_mask is an uns64 that
    // only has ON bits in the appropriate flag pair
    // positions.  Then the unshifted control value of
    // s2 and the control value value of s1 right
    // shifted by the number of flag pairs can be
    // bitwise ANDed with the acc_stack_mask and the
    // result checked for zero.
    //
    // WARNING: only low order flag pair bits may be on
    // in MUP::acc_stack_mask.
    //
    extern min::uns64 acc_stack_mask;
    extern min::stub ** acc_stack;

} }

namespace min { namespace unprotected {

    // Function executed whenever a pointer to stub s2
    // is stored in a datum with stub s1.  s1 is the
    // source of the written pointer and s2 is the
    // target.
    //
    inline void acc_write_update
	    ( const min::stub * s1,
	      const min::stub * s2 )
    {
        uns64 f = (    min::unprotected
	                  ::control_of ( s1 )
	            >> min::internal::ACC_FLAG_PAIRS )
	        & ( min::unprotected
		       ::control_of ( s2 ) )
		& min::internal::acc_stack_mask;

	if ( f != 0 )
	{
	    * min::internal::acc_stack ++ =
	        (min::stub *) s1;
	    * min::internal::acc_stack ++ =
	        (min::stub *) s2;
	}
    }

    // Function executed whenever a general value v is
    // stored in a datum with stub s1, and the general
    // value may contain a stub pointer.  This function
    // calls acc_write_update ( s1, s2 ) if the general
    // value contains the stub pointer s2.
    //
    inline void acc_write_update
	    ( const min::stub * s1, min::gen v )
    {
	if ( min::is_stub ( v ) )
	    acc_write_update
		( s1, min::unprotected
			 ::stub_of ( v ) );
    }

    // Ditto but does nothing for loose implementation.
    //
    inline void acc_write_num_update
	    ( min::stub * s1, min::gen v )
    {
#	if MIN_IS_COMPACT
	    if ( min::is_stub ( v ) )
		acc_write_update
		    ( s1, min::unprotected
			     ::stub_of ( v ) );
#	endif
    }

    // Function executed whenever the n general values
    // pointed at by p are stored in a datum with stub
    // s1, and the general values may contain stub
    // pointers.  This function calls acc_write_update
    // ( s1, s2 ) for every stub pointer s2 in one of
    // the general values.
    //
    inline void acc_write_update
	    ( const min::stub * s1,
	      const min::gen * p, min::unsptr n )
    {
        while ( n -- )
	{
	    min::gen v = * p ++;
	    if ( min::is_stub ( v ) )
	        acc_write_update
		    ( s1, min::unprotected
		             ::stub_of ( v ) );
	}
    }

    // Ditto but does nothing for loose implementation.
    //
    inline void acc_write_num_update
	    ( const min::stub * s1,
	      const min::gen * p, min::unsptr n )
    {
#       if MIN_IS_COMPACT
	    acc_write_update ( s1, p, n );
#	endif
    }

} }

namespace min { namespace internal {

    // Stub allocation is from a single list of stubs
    // chained together by the pointers in the stub
    // controls.  This is referred to as the `acc list'.
    // It is null_stub terminated.
    //
    // The acc list is divided into two segments.  The
    // first is the allocated stubs, and the second is
    // the free stubs.
    //
    // A pointer to the last allocated stub is maintain-
    // ed.  To allocate a new stub, this is updated to
    // the next stub on the acc list, if any.  Other-
    // wise, if there is no next stub (i.e., no free
    // stubs on the acc list), an out-of-line function,
    // acc_expand_stub_free_list, is called to add more
    // free stubs to the end of the acc list.
    //
    // Free acc list stubs have stub type min::ACC_FREE,
    // zero stub control flags, and min::NONE value.
    //
    // Free stubs can be removed completely from the
    // acc list for use as aux stubs.  These are not
    // garbage collectible.  When freed, aux stubs are
    // put back on the acc list as free stubs.

    // Pointers to the first and last allocated stub.
    //
    // The first acc list stub is the stub pointed at by
    // the control word of the first_allocated_stub
    // (which is not itself on the acc list, and there-
    // fore not actually `allocated').  The first free
    // stub on the acc list is pointed at by the control
    // word of the last_allocated_stub.  If there are
    // allocated stubs on the acc list, the last_alloca-
    // ted_stub is the last of these; otherwise it
    // equals first_allocated_stub.
    //
    // First_allocated_stub may equal MINT::null_stub
    // for some system configurations.
    //
    // The acc list is MINT::null_stub terminated.  So
    // if there are no free stubs, the control word of
    // last_allocate_stub points at MINT::null_stub.
    //
    extern min::stub * first_allocated_stub;
    extern min::stub * last_allocated_stub;

    // ACC flags of stub returned by new_acc_stub.
    //
    extern min::uns64 new_acc_stub_flags;

} }

namespace min { namespace unprotected {

    // Counters for statistics only.  Incremented when-
    // ever an acc or aux stub is allocated or freed.
    // The number of current acc or aux stubs is the
    // number allocated minus the number freed.
    //
    extern min::uns64 acc_stubs_allocated;
    extern min::uns64 acc_stubs_freed;
    extern min::uns64 aux_stubs_allocated;
    extern min::uns64 aux_stubs_freed;

    // Function to return the next free stub as an acc
    // (garbage collectible) stub.  The type is set to
    // min::ACC_FREE and may be changed to any garbage
    // collectible type.  The value is NOT set.  The acc
    // flags are set to MINT::new_acc_stub_flags, and
    // the non-type part of the stub control is main-
    // tained by the acc.
    //
    inline min::stub * new_acc_stub ( void )
    {
	if ( min::internal
	        ::number_of_free_stubs == 0 )
	    min::internal
	       ::acc_expand_stub_free_list ( 1 );

	-- min::internal::number_of_free_stubs;
	++ min::unprotected::acc_stubs_allocated;

	uns64 c = min::unprotected::control_of
			( min::internal
			     ::last_allocated_stub );
	min::stub * s = min::unprotected
	                   ::stub_of_acc_control ( c );
	min::unprotected::set_flags_of
	    ( s, min::internal::new_acc_stub_flags );
	return min::internal::last_allocated_stub = s;
    }

    // Function to return the next free stub while
    // removing this stub from the acc list.
    //
    // The type is set to min::AUX_FREE.  This function
    // does NOT set any other part of the stub returned.
    // The stub returned is a non-acc stub ignored by
    // the garbage collector.
    //
    inline min::stub * new_aux_stub ( void )
    {
	if ( min::internal
	        ::number_of_free_stubs == 0 )
	    min::internal
	       ::acc_expand_stub_free_list ( 1 );

	-- min::internal::number_of_free_stubs;
	++ min::unprotected::aux_stubs_allocated;

	uns64 c = min::unprotected::control_of
			( min::internal
			     ::last_allocated_stub );
	min::stub * s = min::unprotected
	                   ::stub_of_acc_control ( c );
	c = min::unprotected::renew_acc_control_stub
	        ( c, min::unprotected
		        ::stub_of_acc_control
			    ( min::unprotected
			         ::control_of ( s ) ) );
	min::unprotected::set_control_of
	    ( min::internal::last_allocated_stub, c );
	min::unprotected::set_type_of
	    ( s, min::AUX_FREE );
	return s;
    }

    // Function to put a stub on the acc free stub list
    // right after the last allocated stub.  Stub must
    // NOT be on the acc list (it should be an auxiliary
    // stub).
    //
    // Note that stubs that have been previously
    // allocated are preferred for new stub allocations
    // over stubs that have never been allocated.
    //
    inline void free_aux_stub ( min::stub * s )
    {
        min::unprotected::set_gen_of
	    ( s, min::NONE );
	uns64 c = min::unprotected::control_of
			( min::internal
			     ::last_allocated_stub );
	min::stub * next =
	    min::unprotected::stub_of_acc_control ( c );
	min::unprotected::set_control_of
	    ( s, min::unprotected::new_acc_control
		  ( min::ACC_FREE, next ) );
	c = min::unprotected
	       ::renew_acc_control_stub ( c, s );
	min::unprotected::set_control_of
		( min::internal
		     ::last_allocated_stub, c );
	++ min::internal::number_of_free_stubs;
	++ min::unprotected::aux_stubs_freed;
    }

} }

namespace min { namespace internal {

    // Ditto but for use by acc to put stub it has
    // freed on acc list.  Same as above but does not
    // increment aux_subs_freed.
    //
    inline void free_acc_stub ( min::stub * s )
    {
        min::unprotected::set_gen_of
	    ( s, min::NONE );
	uns64 c = min::unprotected::control_of
			( min::internal
			     ::last_allocated_stub );
	min::stub * next =
	    min::unprotected::stub_of_acc_control ( c );
	min::unprotected::set_control_of
	    ( s, min::unprotected::new_acc_control
		  ( min::ACC_FREE, next ) );
	c = min::unprotected
	       ::renew_acc_control_stub ( c, s );
	min::unprotected::set_control_of
		( min::internal
		     ::last_allocated_stub, c );
	++ min::internal::number_of_free_stubs;
    }

    // fixed_blocks[j] is the head of a free list of
    // fixed blocks of size 1 << ( j + 3 ), for 2 <= j,
    // (1<<j) <= MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE/8.
    // Each fixed block begins with a control word whose
    // locator is provided by the allocator and whose
    // stub address is set to the stub whose body the
    // block contains, if there is such a stub, or
    // equals the address of MINT::null_stub otherwise.
    // When the block is on the free list, the control
    // word and the word following it are specified by
    // the free_fixed_size_block struct.
    //
    struct free_fixed_size_block
    {
        min::uns64	block_control;
	    // Block control word that is at the
	    // beginning of every fixed size block.

	free_fixed_size_block * next;
	    // Next in circular list of free fixed size
	    // blocks.
    };
    struct fixed_block_list_extension;
        // Allocator specific extension of fixed_block_
	// list struct.
    const unsigned number_fixed_blocks =
          MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG+1-3;
    extern struct fixed_block_list
    {
	min::unsptr size;
			// Size of fixed block, includ-
	                // ing control word.  For fixed_
			// blocks[j] this equals
			// 1 << (j+3).
        min::unsptr count;
			// Number of fixed blocks on the
			// free list.
	free_fixed_size_block * last_free;
			// Last fixed block on the
			// circular list of free blocks,
			// or NULL if list empty.
	fixed_block_list_extension * extension;
			// Address of extension of this
			// structure.  Set during
			// allocator initialization.
    } fixed_blocks
          [number_fixed_blocks];

    // Out of line allocators:
    //
    void new_non_fixed_body
	( min::stub * s, min::unsptr n );
    void new_fixed_body
	( min::stub * s, min::unsptr n,
	  fixed_block_list * fbl );

    extern min::unsptr min_fixed_block_size;
        // The smallest power of 2 not smaller than the
	// size of min::internal::free_fixed_size_block.
    extern min::unsptr max_fixed_block_size;
        // 1 << MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG;

} }

namespace min { namespace unprotected {

    // Return the size of the body associated with the
    // stub.  0 is returned for deallocated stubs (of
    // type min::DEALLOCATED) and stubs with no body.
    // For stubs with a body, the size must be the same
    // as the size passed to new_body to allocated
    // the body.  This function is NOT provided by the
    // acc; it is used by the acc.  The acc is NOT
    // responsible for keeping track of body sizes.
    //
    min::unsptr body_size_of ( const min::stub * s );

    // Allocate a body to a stub.  n is the minimum size
    // of the body in bytes, not including the control
    // word that begins the body.  The stub control is
    // set and its acc flags may be changed.
    //
    inline void new_body
            ( min::stub * s, min::unsptr n )
    {
	min::unsptr m = n + sizeof ( min::uns64);

        if ( m < min::internal::min_fixed_block_size )
            m = min::internal::min_fixed_block_size;

	if ( m > min::internal::max_fixed_block_size )
	{
	     min::internal::new_non_fixed_body ( s, n );
	     return;
	}

	// See min_parameters.h for log2floor.
	//
	m = m - 1;
	min::internal::fixed_block_list * fbl =
		  min::internal::fixed_blocks
		+ min::internal::log2floor ( m )
		+ 1 - 3;

	if ( fbl->count == 0 )
	{
	     min::internal::new_fixed_body
	          ( s, n, fbl );
	     return;
	}

	min::internal::free_fixed_size_block * b =
	    fbl->last_free->next;

	fbl->last_free->next = b->next;
	if ( -- fbl->count == 0 )
	    fbl->last_free = NULL;

	b->block_control =
	    min::unprotected::renew_control_stub
	        ( b->block_control, s );
	min::unprotected
	   ::set_ptr_of ( s, & b->block_control + 1 );
	min::unprotected
	   ::set_flags_of
	       ( s, min::internal
	               ::ACC_FIXED_BODY_FLAG );
    }

    // Deallocate the body of a stub, and reset the stub
    // type to min::DEALLOCATED.  The size n of the body
    // must be given.  The stub body pointer is pointed
    // at inaccessible memory of a size that is usually
    // as big or bigger than the old stub body.  If
    // n == 0 it is assumed that the stub has no body,
    // and nothing is done.
    //
    void deallocate_body
	( min::stub * s, min::unsptr n );

} }


namespace min { namespace internal {

    // Initialize resize_body stub list.
    //
    void acc_initialize_resize_body ( void );

} }

namespace min { namespace unprotected {

    // When constructed the resize_body struct allo-
    // cates a new body for a stub s, and when descon-
    // structed the resize_body struct installs the
    // new body in the stub s  while deallocating the
    // old body of s.  Stub s is not altered until the
    // resize_body struct is deconstructed (the
    // abort_resize_body function can be used to
    // prevent stub s from ever being altered).
    //
    // After the new body is obtained, information
    // should be copied from the old body to the new
    // body, before the resize_body struct is decon-
    // structed.  The new body will not be touched by
    // the garbage collector while the resize_body
    // struct exists.  However, it may be relocated.
    // The existing stub s MUST be protected by the
    // resize_body user from garbage collection and
    // its body protected from reorganization while the
    // resize_body struct exists, but that body may
    // also be relocated.  s must NOT be deallocated
    // while the resize_body struct exists, unless
    // abort_resize_body has been called.
    //
    struct resize_body;
    void abort_resize_body ( resize_body & r );
    struct resize_body {

	// Construct resize_body for stub s with new
	// body size new_size and old body size
	// old_size.
	//
        resize_body ( min::stub * s,
	              min::unsptr new_size,
		      min::unsptr old_size )
	    : s ( s ), new_size ( new_size ),
	      old_size ( old_size ),
	      new_type ( min::type_of ( s ) )
	{
	    // Allocate rstub and its body, which is
	    // the new body.

	    uns64 c = min::unprotected::control_of
		( last_allocated );
	    rstub = min::unprotected
		       ::stub_of_control ( c );
	    if ( rstub == min::internal::null_stub )
	        rstub = rstub_allocate();

	    last_allocated_save = last_allocated;
	    last_allocated = rstub;

	    min::unprotected
	       ::set_type_of ( rstub,
	                       min::RELOCATE_BODY );
	    min::unprotected::new_body
	        ( rstub, new_size );
	}

	// Deconstruct relocated_body, switching the
	// body pointers in s and rstub, and then
	// deallocating rstub.  But do nothing if
	// rstub deallocated.
	//
	// When switching, switch ACC_FIXED_BODY_FLAG.
	//
	// When switching, assumes s is NOT deallocated.
	//
	~resize_body ( void )
	{
	    if ( type_of ( rstub ) != min::DEALLOCATED )
	    {
		min::uns64 v =
		    unprotected::value_of ( s );
		min::uns64 rv =
		    unprotected::value_of ( rstub );
		unprotected::set_value_of ( s, rv );
		unprotected::set_value_of ( rstub, v );

		min::uns64 c =
		    ( unprotected::control_of ( rstub )
		      ^
		      unprotected::control_of ( s ) )
		    &
		    min::internal
		       ::ACC_FIXED_BODY_FLAG;
		rstub->c.u64 ^= c;
		s->c.u64 ^= c;

		int type = min::type_of ( s );
		unprotected::set_type_of
		    ( rstub, type );
		unprotected::set_type_of
		    ( s, new_type );

		min::uns64 * bp = (min::uns64 *)
		    unprotected::ptr_of ( s ) - 1;
		* bp = unprotected::renew_control_stub
			 ( * bp, s );
		bp = (min::uns64 *)
		    unprotected::ptr_of ( rstub )
		    - 1;
		* bp = unprotected::renew_control_stub
			 ( * bp, rstub );

		unprotected::deallocate_body
		    ( rstub, old_size );
	    }

	    last_allocated = last_allocated_save;
	}

	friend void * & new_body_ptr_ref
			( resize_body & r );
	friend void abort_resize_body
			( resize_body & r );
	friend void retype_resize_body
			( resize_body & r,
			  int new_type );
	friend void min::internal
	               ::acc_initialize_resize_body
		             ( void );

    private:

        // Out of line rstub allocator.  Expands
	// resize_body stub list.
	//
	min::stub * rstub_allocate ( void );

	min::stub * s;
	    // Stub whose body is being relocated.
        min::stub * rstub;
	    // Temporary stub for new body.  Type is
	    // min::DEALLOCATED if resize_body is
	    // voided.
	min::unsptr old_size;
	    // Size of old body (body being
	    // reallocated).
	min::unsptr new_size;
	    // Size of new body.
	int new_type;
	    // Type to be installed in stub s when new
	    // body is installed in stub.  Initialized
	    // to type of stub s and reset by retype_
	    // resize_body.
	min::stub * last_allocated_save;
	    // Save of last_allocated.

	static min::stub * last_allocated;
	    // Last allocated stub on the relocated
	    // body stub list.
    };

    // Return a pointer to the new body.
    //
    inline void * & new_body_ptr_ref
	    ( resize_body & r )
    {
	return min::unprotected::ptr_ref_of ( r.rstub );
    }
    // Void the resize_body struct so it will not
    // change anything when deconstructed.  The
    // new body is deallocated.
    //
    inline void abort_resize_body ( resize_body & r )
    {
	min::unprotected::deallocate_body
	    ( r.rstub, r.new_size );
    }
    // Set type to be installed in stub when new body
    // body is installed in stub.
    //
    inline void retype_resize_body
	    ( resize_body & r, int new_type )
    {
	r.new_type = new_type;
    }

} }

namespace min { namespace internal {

    // Scavenging a stub is done by calling a stub type
    // specific scavenger routine with a scavenge
    // control struct as argument.
    //
    struct scavenge_control;
    typedef void (*scavenger_routine)
        ( scavenge_control & sc );

    // scavenger_routine[t] is the scavenger routine
    // for stubs of type t, with t >= 0 (i.e., t is
    // for an acc stub and not an aux stub).
    //
    extern scavenger_routine scavenger_routines[128];

    inline bool is_scavengable ( int type )
    {
        return scavenger_routines[type] != NULL;
    }

    // Function to scavenge the thread stack_gen and
    // stack_num_gen structures and the static_gen
    // and static_num_gen structures, finding all
    // pointers therein to acc stubs s2.  For each s2
    // found, a particular flag of s2, designated by
    // sc.stub_flag, is checked to see if it is on.
    // If it is, it is turned off and if the s2 stub
    // is scavengable, a pointer to s2 is pushed onto
    // the to_be_scavenged stack.
    //
    // Note that s2 is scavengable if and only if
    // scavenger_rountines[type_of(s2)] != NULL;
    //
    // Returns with sc.state != 0 only if it runs out
    // of to_be_scavenged stack, in which case it should
    // be recalled with 0 sc.state after to_be_scavenged
    // stack is emptied.
    //
    // This function ignores sc.gen_limit and sc.stub_
    // flag_accumulator, but increments gen_count and
    // stub_count.
    //
    void thread_scavenger_routine
        ( scavenge_control & sc );
    
    // The job of a scavenger routine is to scavenge
    // a stub s1 which is pointed at by a scavenge
    // control struct sc.  This means going through the
    // stub, any body pointed at by the stub, and any
    // auxiliary stubs pointed at by the stub, by the
    // body, or by other auxiliary stubs so pointed
    // at, and finding all pointers therein to acc
    // stubs s2.  For each s2 found, two things are
    // done.
    //
    // First, a particular flag of s2, designated by
    // sc.stub_flag, is checked to see if it is on.
    // If it is, it is turned off and if s2 is scaven-
    // gable a pointer to s2 is put in a to-be-scavenged
    // stack.  s2 is scavengable if and only if
    // scavenger_rountines[type_of(s2)] != NULL;
    //
    // The second thing is to logically OR the control
    // word of s2 into an accumulator designed as
    // sc.stub_flag_accumulator.  This may be set to 0
    // by the caller of the scavenger routine, when
    // that routine returns after finishing scavenging
    // s1, certain bits of this accumulator can be used
    // to tell if s1 contained any pointer to an acc
    // stub of level >= L, for each possible level L.
    //
    // It is also possible for the scavenger routine to
    // return before it has finished scavenging s1.
    // A work limit is placed on the scavenger routine
    // so it does not execute for too long.  And the
    // routine can also return because it runs out of
    // space in the currently available portion of the
    // to-be-scavenged stack.  When the routine returns
    // early, it may be recalled to resume where it
    // left off.  To allow this, there is a scavenge
    // control datum sc.state which is initialized to
    // 0 by the caller of the scavenger routine before
    // starting the scavenging of s1, is left non-zero
    // if the routine returns before finishing scaven-
    // ging s1, and is set to 0 if the routine returns
    // after finishing scavenging s1.
    // 
    struct scavenge_control
    {
        // For definitions of s1 and s2 see above.

	min::uns64 state;
	    // State of the scavenge of the stub being
	    // scavenged.  Set to 0 by caller on first
	    // call to scavenge s1.  Returned as 0 by
	    // the scavenger routine when scavenging of
	    // s1 is done.  Returned as non-zero by
	    // scavenger routine if scavenging of s1
	    // must be continued.
	    //
	    // Note that s1 can be scavenged multiple
	    // times without harm, so if a scavenger
	    // routine must return before it is done
	    // with s1 it can simply set the state to
	    // non-zero to indicate the routine should
	    // be rerun from the beginning.  This is
	    // adequate for small data.

	min::uns64 stub_flag;
	    // A word with a single bit set.  If this
	    // bit is set in the stub control of s2 then
	    // the scavenger routine must clear the bit
	    // in s2's control and, if scavenger_
	    // routines[type_of(s2)] is not NULL, put s2
	    // on the to-be-scavenged stack.

	min::uns64 stub_flag_accumulator;
	    // Logical OR of all the stub controls of
	    // all the stubs s2 pointed at by s1.  This
	    // is not initially set by the scavenger
	    // routine; to initialize it to zero it must
	    // be zeroed by the routine's caller before
	    // beginning the scavenging of s1.

	min::stub * s1;
	   // Stub being scavenged.

	min::stub ** to_be_scavenged;
	min::stub ** to_be_scavenged_limit;
	   // Pointer to the first unused location in
	   // the to-be-scavenged stack and to the first
	   // location after the stack.  The stack grows
	   // in the direction of increasing addresses.
	   // When there is no more room in the stack
	   // the scavenger routine must return (with a
	   // non-zero state if it is not done).

	min::uns32 stub_count;
	   // Counter incremented by the scavenger
	   // routine when it finds a pointer to a
	   // stub s2 in s1.  Just used for statistics.

	min::uns32 gen_count;
	   // Counter incremented by the scavenger
	   // routine when it inspects a min::gen value
	   // or other place that an s2 stub pointer
	   // may be stored.  Used both for statistics
	   // and as a measure of scavenger routine
	   // execution time (see gen_limit).

	min::uns32 gen_limit;
	   // Upper limit to gen_count.  If exceeded
	   // the scavenger routine must return.  Used
	   // to bound the amount of time spent in a
	   // single call to a scavenger routine.
    };

    // scavenge_control[L] is the scavenge control
    // struct for acc level L scavenge routine
    // executions.
    //
    extern scavenge_control scavenge_controls
    		[ 1 + MIN_MAX_EPHEMERAL_LEVELS ];

    // Actual number of acc levels.
    //
    extern unsigned number_of_acc_levels;

    // Searches scavenge_controls to see if a stub is
    // being scavenged and returns true if yes and false
    // it no.  A stub is being scavenged only if a
    // scavenge of the stub was interrupted.
    //
    inline bool is_being_scavenged ( min::stub * s1 )
    {
        for ( scavenge_control * sc = scavenge_controls;
	      sc <   scavenge_controls
	           + number_of_acc_levels;
	      ++ sc )
	{
	    if ( sc->state != 0 && sc->s1 == s1 )
	        return true;
	}
	return false;
    }

} }

// Numbers
// -------

namespace min {

#   if MIN_IS_COMPACT
	namespace internal {

	    // Function to create new number stub or
	    // return an existing stub.
	    //
	    min::gen new_num_stub_gen
		( min::float64 v );
	}

	inline min::float64 float_of
	    ( const min::stub * s )
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
		      ( internal::unsgen_to_stub ( v ) )
		    == min::NUMBER );
	}
	inline min::gen new_num_gen ( min::int32 v )
	{
	    if ( ( -1 << 27 ) <= v && v < ( 1 << 27 ) )
		return unprotected::new_direct_int_gen
				( v );
	    return internal::new_num_stub_gen ( v );
	}
	inline min::gen new_num_gen ( min::uns32 v )
	{
	    if ( v < ( 1 << 27 ) )
		return unprotected::new_direct_int_gen
				( (int) v );
	    return internal::new_num_stub_gen ( v );
	}
	inline min::gen new_num_gen ( min::int64 v )
	{
	    if ( v < ( 1 << 27 ) )
		return unprotected::new_direct_int_gen
				( (int) v );
	    return internal::new_num_stub_gen ( v );
	}
	inline min::gen new_num_gen ( min::uns64 v )
	{
	    if ( v < ( 1 << 27 ) )
		return unprotected::new_direct_int_gen
				( (int) v );
	    return internal::new_num_stub_gen ( v );
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
	    return internal::
		   new_num_stub_gen ( v );
	}
	inline int int_of ( min::gen v )
	{
	    if ( is_stub ( v ) )
	    {
		const min::stub * s =
		    internal::unsgen_to_stub ( v );
		MIN_ASSERT (    type_of ( s )
			     == min::NUMBER );
		min::float64 f =
		    unprotected::float_of ( s );
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
		MIN_ABORT ( "int_of non number" );
	    }
	}
	namespace unprotected {
	    inline float64 float_of ( min::gen v )
	    {
		if ( is_stub ( v ) )
		{
		    const min::stub * s =
			internal::unsgen_to_stub ( v );
		    return min::unprotected
		              ::float_of ( s );
		}
		else
		    return unprotected::
			   direct_int_of ( v );
	    }
	}
	inline float64 float_of ( min::gen v )
	{
	    if ( is_stub ( v ) )
	    {
		const min::stub * s =
		    internal::unsgen_to_stub ( v );
		return float_of ( s );
	    }
	    else if
		(   v
		  < ( min::GEN_DIRECT_STR << VSIZE ) )
		return unprotected::
		       direct_int_of ( v );
	    else
	    {
		MIN_ABORT ( "float_of non number" );
	    }
	}
#   elif MIN_IS_LOOSE
	inline bool is_num ( min::gen v )
	{
	    return min::is_direct_float ( v );
	}
	inline min::gen new_num_gen ( min::int32 v )
	{
	    return new_direct_float_gen ( v );
	}
	inline min::gen new_num_gen ( min::uns32 v )
	{
	    return new_direct_float_gen ( v );
	}
	inline min::gen new_num_gen ( min::uns64 v )
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
	namespace unprotected {
	    inline float64 float_of ( min::gen v )
	    {
		return unprotected
		       ::direct_float_of ( v );
	    }
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

    inline min::uns64 short_str_of
            ( const min::stub * s )
    {
	return s->v.u64;
    }
    inline min::unprotected::long_str * long_str_of
	    ( const min::stub * s )
    {
	return (min::unprotected::long_str *)
	       min::internal::uns64_to_ptr ( s->v.u64 );
    }
    inline const char * str_of
    	    ( min::unprotected::long_str * str )
    {
	return (const char *) str
	       + sizeof ( min::unprotected::long_str );
    }

    inline min::unsptr length_of
    	    ( min::unprotected::long_str * str )
    {
	return str->length;
    }
    inline min::uns32 hash_of
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
	    ( const char * p, min::unsptr size );
    //
    min::uns32 strhash ( const char * p );

    min::unsptr strlen ( min::gen v );
    min::uns32 strhash ( min::gen v );
    char * strcpy ( char * p, min::gen v );
    char * strncpy
        ( char * p, min::gen v, min::unsptr n );
    int strcmp ( const char * p, min::gen v );
    int strncmp
        ( const char * p, min::gen v, min::unsptr n );

    namespace unprotected {

	// For forward reference.
	//
	class str_ptr;
    }

    // Functions that must be declared before they are
    // made friends.
    //
    namespace unprotected {
	const char * str_of
	    ( const min::unprotected::str_ptr & sp );
    }
    min::unsptr strlen
        ( const min::unprotected::str_ptr & sp );
    min::uns32 strhash
        ( const min::unprotected::str_ptr & sp );
    char * strcpy ( char * p,
                    const min::unprotected
		             ::str_ptr & sp );
    char * strncpy ( char * p,
                     const min::unprotected
		              ::str_ptr & sp,
		     min::unsptr n );
    int strcmp
        ( const char * p,
	  const min::unprotected::str_ptr & sp );
    int strncmp
        ( const char * p,
	  const min::unprotected::str_ptr & sp,
	  min::unsptr n );

    namespace unprotected {

	class str_ptr
	{
	public:

	    str_ptr ( const min::stub * s )
	        : s ( s )
	    {
		if ( s == NULL ) return;
		else if (    min::type_of ( s )
		          == min::LONG_STR )
		    return;

		pseudo_body.u.str = s->v.u64;
		pseudo_body.u.buf[8] = 0;
		this->s = & pseudo_stub;
		min::unprotected::set_ptr_of
		       ( & pseudo_stub,
			 (void *) & pseudo_body );
	    }

	    str_ptr ( min::gen v )
	    {

		if ( min::is_stub ( v ) )
		    new ( this )
		        str_ptr
			    ( unprotected::stub_of
			          ( v ) );
		else
		{
		    pseudo_body.u.str
			= min::unprotected
			     ::direct_str_of ( v );
		    s = & pseudo_stub;
		    min::unprotected
		       ::set_ptr_of
			   ( & pseudo_stub,
			     (void *) & pseudo_body );
		}
	    }

	    str_ptr ( void ) : s ( NULL ) {}

	    // Operator[] MUST be a member and cannot
	    // be a friend.
	    //
	    const char & operator[] ( int index ) const
	    {
		return
		    ( (const char * )
		      unprotected::long_str_of ( s ) )
		    [sizeof ( unprotected::long_str )
		     + index];
	    }

	    str_ptr & operator = ( const min::stub * s )
	    {
		new ( this ) str_ptr ( s );
		return * this;
	    }

	    str_ptr & operator = ( min::gen v )
	    {
		new ( this ) str_ptr ( v );
		return * this;
	    }

	    str_ptr & operator = ( const str_ptr & sp )
	    {
		if ( sp.s == & sp.pseudo_stub )
		{
		    pseudo_body = sp.pseudo_body;
		    s = & pseudo_stub;
		    min::unprotected
		       ::set_ptr_of
			   ( & pseudo_stub,
			     (void *) & pseudo_body );
		}
		else
		    s = sp.s;
		return * this;
	    }

	    friend const char * min::unprotected::str_of
		( const unprotected::str_ptr & sp );
	    friend min::unsptr min::strlen
		( const unprotected::str_ptr & sp );
	    friend min::uns32 min::strhash
		( const unprotected::str_ptr & sp );
	    friend char * min::strcpy
		( char * p,
		  const unprotected::str_ptr & sp );
	    friend char * min::strncpy
		( char * p,
		  const unprotected::str_ptr & sp,
		  min::unsptr n );
	    friend int min::strcmp
		( const char * p,
		  const unprotected::str_ptr & sp );
	    friend int min::strncmp
		( const char * p,
		  const unprotected::str_ptr & sp,
		  min::unsptr n );

	private:

	    const min::stub * s;
		// Stub pointer if long string, or
		// pointer to pseudo_stub for short
		// or direct string.

	    min::stub pseudo_stub;
		// Pseudo-stub for short or direct
		// string; only value is used to point
		// at pseudo_body.buf.

	    struct
	    {
		struct min::unprotected::long_str h;
		union {
		    char buf [9];
			// NUL terminated copy of
			// string.
		    uns64 str;
		    uns64 xx[2];
			// Sized to maintain alignment
			// of surrounding data.
		} u;
	    } pseudo_body;
	};
    }

    // Function needed in min::str_ptr definition.
    //
    inline bool is_str ( min::gen v )
    {
	if ( min::is_direct_str ( v ) )
	    return true;
	if ( ! min::is_stub ( v ) )
	    return false;
	const min::stub * s =
	    min::unprotected::stub_of ( v );
	return min::type_of ( s ) == min::SHORT_STR
	       ||
	       min::type_of ( s ) == min::LONG_STR;
    }

    class str_ptr : public unprotected::str_ptr
    {
    public:
	str_ptr ( min::gen v )
	{
	    MIN_ASSERT ( min::is_str ( v ) );
	    new ( this ) unprotected::str_ptr ( v );
	}
	str_ptr ( const min::stub * s )
	{
	    if ( s != NULL )
	    {
	        int t = min::type_of ( s );
		MIN_ASSERT ( t == min::SHORT_STR
		             ||
		             t == min::LONG_STR );
	    }
	    new ( this ) unprotected::str_ptr ( s );
	}
	str_ptr ( void )
	{
	    new ( this ) unprotected::str_ptr();
	}

	str_ptr & operator = ( const min::stub * s )
	{
	    new ( this ) str_ptr ( s );
	    return * this;
	}

	str_ptr & operator = ( min::gen v )
	{
	    new ( this ) str_ptr ( v );
	    return * this;
	}

	str_ptr & operator = ( const str_ptr & sp )
	{
	    * (unprotected::str_ptr *) this = sp;
	    return * this;
	}
    };

    inline const char * unprotected::str_of
	    ( const min::unprotected::str_ptr & sp )
    {
	return (const char * )
	       min::unprotected::long_str_of ( sp.s )
	       +
	       sizeof ( min::unprotected::long_str );
    }

    inline min::unsptr strlen
        ( const min::unprotected::str_ptr & sp )
    {
        if ( sp.s == & sp.pseudo_stub )
	    return ::strlen ( sp.pseudo_body.u.buf );
	else
	    return unprotected::length_of
	              ( unprotected::long_str_of
		            ( sp.s ) );
    }

    inline min::uns32 strhash
	    ( const min::unprotected::str_ptr & sp )
    {
        if ( sp.s == & sp.pseudo_stub )
	    return min::strhash
	        ( sp.pseudo_body.u.buf );
	else
	    return unprotected::hash_of
	              ( unprotected::long_str_of
		            ( sp.s ) );
    }

    inline char * strcpy
    	( char * p,
	  const min::unprotected::str_ptr & sp )
    {
        return ::strcpy
	    ( p, min::unprotected::str_of ( sp ) );
    }

    inline char * strncpy
    	( char * p,
	  const min::unprotected::str_ptr & sp,
	  min::unsptr n )
    {
        return ::strncpy
	    ( p, min::unprotected::str_of ( sp ), n );
    }

    inline int strcmp
    	( const char * p,
	  const min::unprotected::str_ptr & sp )
    {
        return ::strcmp
	    ( p, min::unprotected::str_of ( sp ) );
    }

    inline int strncmp
    	( const char * p,
	  const min::unprotected::str_ptr & sp,
	  min::unsptr n )
    {
        return ::strncmp
	    ( p, min::unprotected::str_of ( sp ), n );
    }

    namespace internal {
	min::gen new_str_stub_gen
	    ( const char * p, min::unsptr n );
    }

    inline min::gen new_str_gen ( const char * p )
    {
	min::unsptr n = ::strlen ( p );
#	if MIN_IS_COMPACT
	    if ( n <= 3 )
		return min::unprotected::
		       new_direct_str_gen ( p );
#	elif MIN_IS_LOOSE
	    if ( n <= 5 )
		return min::unprotected::
		       new_direct_str_gen ( p );
#	endif
	return internal::new_str_stub_gen ( p, n );
    }

    inline min::gen new_str_gen
            ( const char * p, min::unsptr n )
    {
	n = internal::strnlen ( p, n );
#	if MIN_IS_COMPACT
	    if ( n <= 3 )
		return min::unprotected::
		       new_direct_str_gen ( p, n );
#	elif MIN_IS_LOOSE
	    if ( n <= 5 )
		return min::unprotected::
		       new_direct_str_gen ( p, n );
#	endif
	return internal::new_str_stub_gen ( p, n );
    }
}

// Labels
// ------

// Labels are implemented by a body consisting of a
// header followed by the label elements.  The header
// contains the length (number of elements) and the
// hash value.

namespace min {

    namespace internal {

	struct lab_header
	    // This must be an integral number of
	    /// min::gen units in length.
	{
	    uns32		length;
	    uns32		hash;
	};

	inline min::internal::lab_header * lab_header_of
		( const min::stub * s )
	{
	    return (min::internal::lab_header *)
		   min::unprotected::ptr_of ( s );
	}
    }

    namespace unprotected {

	// Declared for forward reference.
	//
	class lab_ptr;
    }

    // Declared for use as friend below.
    //
    min::uns32 length_of
	    ( min::unprotected
		 ::lab_ptr & labp );
    min::uns32 hash_of
	    ( min::unprotected
		 ::lab_ptr & labp );

    namespace unprotected {

	class lab_ptr
	{
	public:

	    lab_ptr ( min::gen v )
		: s ( min::stub_of ( v ) ) {}

	    lab_ptr ( const min::stub * s )
		: s ( s ) {}

	    lab_ptr ( void )
		: s ( NULL ) {}

	    min::gen operator [] ( min::uns32 i )
	        const
	    {
		MIN_ASSERT ( i < header()->length );
		return base()[i];
	    }

	    operator const min::stub * ( void ) const
	    {
	        return s;
	    }

	    lab_ptr & operator = ( min::gen v )
	    {
	        new ( this ) lab_ptr ( v );
		return * this;
	    }

	    lab_ptr & operator = ( const min::stub * s )
	    {
	        new ( this ) lab_ptr ( s );
		return * this;
	    }

	    friend min::uns32 min::length_of
		    ( min::unprotected
			 ::lab_ptr & labp );
	    friend min::uns32 min::hash_of
		    ( min::unprotected
			 ::lab_ptr & labp );

	protected:

	    const min::stub * s;

	    internal::lab_header * header ( void ) const
	    {
		return
		    * (internal::lab_header **) &
		    unprotected::ptr_ref_of
			( (min::stub *) s );
	    }

	    const min::gen * base ( void ) const
	    {
		return (const min::gen *)
		       ( header() + 1 );
	    }
	};
    }

    class lab_ptr : public unprotected::lab_ptr
    {
    public:

        lab_ptr ( min::gen v )
	    : unprotected::lab_ptr ( v )
	{
	    MIN_ASSERT ( type_of ( s ) == min::LABEL );
	}

        lab_ptr ( const min::stub * s )
	    : unprotected::lab_ptr ( s )
	{
	    MIN_ASSERT ( type_of ( s ) == min::LABEL );
	}

        lab_ptr ( void ) {}

	lab_ptr & operator = ( min::gen v )
	{
	    new ( this ) lab_ptr ( v );
	    return * this;
	}

	lab_ptr & operator = ( const min::stub * s )
	{
	    new ( this ) lab_ptr ( s );
	    return * this;
	}

    };

    inline min::uns32 length_of
	    ( min::unprotected::lab_ptr & labp )
    {
        return labp.header()->length;
    }

    inline min::uns32 hash_of
	    ( min::unprotected::lab_ptr & labp )
    {
        return labp.header()->hash;
    }

    inline min::uns32 lablen ( const min::stub * s )
    {
        MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
	return min::internal::lab_header_of(s)->length;
    }

    inline min::uns32 lablen ( min::gen v )
    {
	return min::lablen ( min::stub_of ( v ) );
    }

    inline min::uns32 labhash ( const min::stub * s )
    {
        MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
	return min::internal::lab_header_of(s)->hash;
    }

    inline min::uns32 labhash ( min::gen v )
    {
	return min::labhash ( min::stub_of ( v ) );
    }

    min::uns32 labhash
	    ( const min::gen * p, min::uns32 n );

    const min::uns32 lab_hash_factor =
        // 65599 ** 10
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
    //
    inline min::uns32 labhash
	    ( min::uns32 hash, min::uns32 h )
    {
        hash = ( hash * lab_hash_factor ) + h;
	if ( hash == 0 ) hash = (min::uns32) -1;
	return hash;
    }

    inline min::uns32 lab_of
	    ( min::gen * p, min::uns32 n,
	      const min::stub * s )
    {
	lab_ptr labp ( s );
	if ( n > length_of ( labp ) )
	    n = length_of ( labp );
        for ( uns32 i = 0; i < n; ++ i )
	{
	    * p ++ = labp[i];
	}
	return n;
    }

    inline min::uns32 lab_of
	    ( min::gen * p, min::uns32 n, min::gen v )
    {
	return min::lab_of ( p, n, min::stub_of ( v ) );
    }

    min::gen new_lab_gen
	    ( const min::gen * p, min::uns32 n );

    inline bool is_lab ( min::gen v )
    {
	if ( ! min::is_stub ( v ) )
	    return false;
	const min::stub * s =
	    min::unprotected::stub_of ( v );
	return min::type_of ( s ) == min::LABEL;
    }
}

// Names
// -----

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

    int compare ( min::gen v1, min::gen v2 );
}

// Packed Structures
// -----------------

namespace min {

    // The C offsetof macro does not work, so:
    //
    template < typename S, typename T >
    min::uns32 OFFSETOF ( T S::* d )
    {
        S * p = (S *) NULL;
	return (uns8 *) & (p->*d) - (uns8 *) p;
    }

    template < typename S >
    min::uns32 DISP ( min::gen S::* d )
    {
	return OFFSETOF ( d );
    }
    template < typename S >
    min::uns32 DISP ( const min::stub * S::* d )
    {
	return OFFSETOF ( d );
    }
    template < typename S >
    min::uns32 DISP ( min::stub * S::* d )
    {
	return OFFSETOF ( d );
    }
    const min::uns32 DISP_END = min::uns32 ( -1 );

    const min::stub * const NULL_STUB =
        (const min::stub *) NULL;

    struct packed_id
    {
	const packed_id * base;
    };

    namespace internal {

	// An packed struct/vect body begins with a
	// min::uns32 control word whose low order MIN_
	// PACKED_CONTROL_SUBTYPE_BITS bits are a
	// subtype t such that (*packed_subtypes)[t] is
	// the address of the body's packed_struct/vec_
	// descriptor.
	//
	extern void *** packed_subtypes;

	const min::uns32 PACKED_CONTROL_SUBTYPE_MASK =
	  ( 1 << MIN_PACKED_CONTROL_SUBTYPE_BITS ) - 1;

	// Types t for packed_subtypes must be in the
	// range 0 <= t < packed_subtype_count.  If a
	// new packed subtype is allocated, packed_
	// subtype_count is incremented.  If it would
	// become > max_packed_subtype_count then
	// allocate_packed_subtypes is called.
	//
	extern unsigned packed_subtype_count;
	extern unsigned max_packed_subtype_count;

	// Use the following after checking
	// type_of ( s ).
	//
	inline min::uns32 packed_subtype_of
		( const min::stub * s )
	{
	    void * p = unprotected::ptr_of ( s );
	    min::uns32 subtype = * (min::uns32 *) p;
	    subtype &=
		internal::PACKED_CONTROL_SUBTYPE_MASK;
	    MIN_ASSERT
		(   subtype
		  < internal::packed_subtype_count);
	    return subtype;
	}

	// Allocate or reallocate the packed_subtypes
	// vector setting max_packed_subtype_count to
	// the given value.
	//
	void allocate_packed_subtypes
	    ( min::uns32 max_packed_subtype_count );

	// The packed_descriptor is the common part of
	// of packed_struct_descriptor and packed_vec_
	// descriptor.
	//
	struct packed_descriptor
	{
	    const min::int32 type;
	        // Either min::PACKED_STRUCT
		// or min::PACKED_VEC.
	    const min::uns32 subtype;
		// Index of descriptor address in MINT::
		// packed_subtypes.
	    const packed_id * const id;
	    const char * const name;

	    // Since many members of the descriptor are
	    // const's, we need to set them with a
	    // constructor.
	    //
	    packed_descriptor
	        ( min::int32 type,
		  min::uns32 subtype,
		  const packed_id * id,
		  const char * name )
	        : type ( type ),
		  subtype ( subtype ),
		  id ( id ),
		  name ( name ) {}
	};

	// The packed_struct_descriptor is the part
	// of a packed_struct<S> that does not dependent
	// on S in the template sense.  It can be used
	// by functions that need not have detailed
	// knowledge of S, such as acc functions.
	//
	struct packed_struct_descriptor
	    : public packed_descriptor
	{
	    const min::uns32 size; // sizeof ( S )
	    const min::uns32 * const gen_disp;
	    const min::uns32 * const stub_disp;
	    const min::uns32 * io_disp;
	    const min::gen   * io_names;

	    // Since many members of the descriptor are
	    // const's, we need to set them with a
	    // constructor.
	    //
	    packed_struct_descriptor
	        ( min::int32 type,
		  min::uns32 subtype,
		  const packed_id * id,
		  const char * name,
		  min::uns32 size,
		  const min::uns32 * gen_disp,
		  const min::uns32 * stub_disp )
	        : packed_descriptor
		      ( type, subtype, id, name ),
		  size ( size ),
		  gen_disp ( gen_disp ),
		  stub_disp ( stub_disp ),
		  io_disp ( NULL ),
		  io_names ( NULL ) {}
	};

	// Pointer base inherited by public pointer
	// classes.
	//
	template < typename S >
	class packed_struct_ptr_base
	{
	public:

	    packed_struct_ptr_base ( min::gen v )
	    {
		new ( this )
		    internal
		    ::packed_struct_ptr_base<S>
			( stub_of ( v ) );
	    }
	    packed_struct_ptr_base
		( const min::stub * s );
	    packed_struct_ptr_base ( void )
		: s ( NULL_STUB ) {}

	    operator const min::stub * ( void ) const
	    {
		return this->s;
	    }

	protected:
		
	    // A packed structure pointer is just a
	    // const min::stub * value with garnish.
	    //
	    const min::stub * s;
	};

	// Out-of-line new_stub function (non-template).
	//
	const min::stub * packed_struct_new_stub
	    ( packed_struct_descriptor * psd );
    }

    inline min::uns32 packed_subtype_of
	    ( const min::stub * s )
    {
	int t = type_of ( s );
	MIN_ASSERT
	    ( t == PACKED_STRUCT || t == PACKED_VEC );
	return internal::packed_subtype_of ( s );
    }

    inline min::uns32 packed_subtype_of
	    ( min::gen v )
    {
        return packed_subtype_of ( stub_of ( v ) );
    }

    template < typename S >
    class packed_struct_ptr
	  : public internal::packed_struct_ptr_base<S>
    {

    public:

	packed_struct_ptr ( min::gen v )
	    : internal::packed_struct_ptr_base<S>
		( v ) {}
	packed_struct_ptr ( const min::stub * s )
	    : internal::packed_struct_ptr_base<S>
		( s ) {}
	packed_struct_ptr ( void )
	    : internal::packed_struct_ptr_base<S>
		() {}

	const S * operator -> ( void ) const
	{
	    return (const S *)
		   unprotected::ptr_of ( this->s );
	}

	const S & operator * ( void ) const
	{
	    return * (const S *)
		   unprotected::ptr_of ( this->s );
	}

	packed_struct_ptr & operator =
		( const min::stub * s )
	{
	    new ( this )
		internal::packed_struct_ptr_base<S>
		    ( s );
	    return * this;
	}

	packed_struct_ptr & operator =
		( min::gen v )
	{
	    new ( this )
		internal::packed_struct_ptr_base<S>
		    ( v );
	    return * this;
	}

	static min::uns32 DISP ( void )
	{
	    return OFFSETOF
	        ( & packed_struct_ptr::s );
	}
    };

    template < typename S, typename T >
    min::uns32 DISP
	    ( packed_struct_ptr<T> S::* d )
    {
	return   OFFSETOF ( d )
	       + packed_struct_ptr<T>::DISP();
    }

    template < typename S >
    class packed_struct_updptr
	  : public
	    internal::packed_struct_ptr_base<S>
    {

    public:

	packed_struct_updptr
		( min::gen v )
	    : internal::packed_struct_ptr_base<S>
		( v ) {}
	packed_struct_updptr
		( const min::stub * s )
	    : internal::packed_struct_ptr_base<S>
		( s ) {}
	packed_struct_updptr ( void )
	    : internal::packed_struct_ptr_base<S>
		() {}

	S * operator -> ( void ) const
	{
	    return (S *)
		   unprotected::ptr_of ( this->s );
	}

	S & operator * ( void ) const
	{
	    return * (S *)
		   unprotected::ptr_of ( this->s );
	}

	packed_struct_updptr & operator =
		( const min::stub * s )
	{
	    new ( this )
		internal::packed_struct_ptr_base<S>
		    ( s );
	    return * this;
	}

	packed_struct_updptr & operator =
		( min::gen v )
	{
	    new ( this )
		internal::packed_struct_ptr_base<S>
		    ( v );
	    return * this;
	}

	static min::uns32 DISP ( void )
	{
	    return OFFSETOF
	        ( & packed_struct_updptr::s );
	}
    };

    template < typename S, typename T >
    min::uns32 DISP
	    ( packed_struct_updptr<T> S::* d )
    {
	return   OFFSETOF ( d )
	       + packed_struct_updptr<T>::DISP();
    }

    template < typename S >
    class packed_struct
	: public internal::packed_struct_descriptor
    {
    public:

        packed_struct
	    ( const char * name,
	      const min::uns32 * gen_disp = NULL,
	      const min::uns32 * stub_disp = NULL,
	      const packed_id & base_class_id =
		  * (packed_id *) NULL );

	const min::stub * new_stub ( void )
	{
	    return internal::packed_struct_new_stub
		( this );
	}

	min::gen new_gen ( void )
	{
	    return min::new_gen
	        ( internal::packed_struct_new_stub
		      ( this ) );
	}

	typedef typename
		min::packed_struct_ptr<S> ptr;
	typedef typename
		min::packed_struct_updptr<S> updptr;

	static packed_id id;
    };

    template < typename S, typename B >
    class packed_struct_with_base
	: public packed_struct<S>
    {
    public:

        packed_struct_with_base
	    ( const char * name,
	      const min::uns32 * gen_disp = NULL,
	      const min::uns32 * stub_disp = NULL )
	    : packed_struct<S>
	          ( name, gen_disp, stub_disp,
		    packed_struct<B>::id ) {}
    };

    template < typename S >
    inline internal
           ::packed_struct_ptr_base<S>
	   ::packed_struct_ptr_base
	    ( const min::stub * s ) : s ( s )
    {
	if ( s == NULL ) return;

	int t = type_of ( s );
	MIN_ASSERT
	    ( t == PACKED_STRUCT || t == PACKED_VEC );
	min::uns32 subtype =
	    internal::packed_subtype_of ( s );
	packed_struct_descriptor * psdescriptor =
	    (packed_struct_descriptor *)
	    (*packed_subtypes)[subtype];
	const packed_id * id = psdescriptor->id;
	while ( & packed_struct<S>::id != id )
	{
	    MIN_ASSERT ( id != NULL );
	    id = id->base;
	}
    }
}

// The following should be in min.cc BUT since they are
// templates they must be included in every compilation
// that might instantiate them.

template < typename S >
min::packed_id min::packed_struct<S>::id;

template < typename S >
min::packed_struct<S>::packed_struct
    ( const char * name,
      const min::uns32 * gen_disp,
      const min::uns32 * stub_disp,
      const packed_id & base_class_id )
    : internal::packed_struct_descriptor
          ( min::PACKED_STRUCT,
	    internal::packed_subtype_count ++,
	    & id,
            name,
            sizeof ( S ),
            gen_disp,
            stub_disp )
{
    // Check that the control member is the first
    // thing in the S structure.
    //
    MIN_ASSERT
        ( ( OFFSETOF<S,const min::uns32>
	      ( & S::control ) == 0 ) );

    if (   internal::packed_subtype_count
	 > internal::max_packed_subtype_count )
	internal::allocate_packed_subtypes
	    ( internal::max_packed_subtype_count
	      + MIN_PACKED_SUBTYPE_COUNT );
    (*internal::packed_subtypes) [ subtype ] =
        (internal::packed_struct_descriptor *)
	this;
    if ( & base_class_id != id.base )
    {
        MIN_ASSERT ( id.base == NULL );
	id.base = & base_class_id;
    }
}

// Packed Vectors
// --------------

// Much of packed vectors replicates packed structures;
// see comments under Packed Structures for documenta-
// tion.

namespace min {

    namespace internal {

	struct packed_vec_descriptor
	    : public packed_descriptor
	{
	    const min::uns32 length_disp;
	    const min::uns32 max_length_disp;
	        // Displacements of length and
		// max_length elements in H.

	    const min::uns32 element_size;
	        // sizeof ( E ).
	    const min::uns32 * const element_gen_disp;
	    const min::uns32 * const
	                       element_stub_disp;
	    const min::uns32 * element_io_disp;
	    const min::gen   * element_io_names;

	    const min::uns32 header_size;
	        // sizeof ( H ) rounded up as per
		// computed_header_size.
	    const min::uns32 * const header_gen_disp;
	    const min::uns32 * const
	                       header_stub_disp;
	    const min::uns32 * header_io_disp;
	    const min::gen   * header_io_names;

	    min::uns32 initial_max_length;
	    min::float64 increment_ratio;
	    min::uns32 max_increment;

	    packed_vec_descriptor
	        ( min::int32 type,
		  min::uns32 subtype,
		  const packed_id * id,
		  const char * name,

		  min::uns32 length_disp,
		  min::uns32 max_length_disp,

		  min::uns32 element_size,
		  const min::uns32 * element_gen_disp,
		  const min::uns32 *
		      element_stub_disp,

		  min::uns32 header_size,
		  const min::uns32 * header_gen_disp,
		  const min::uns32 *
		      header_stub_disp )
	        : packed_descriptor
		      ( type, subtype, id, name ),

		  length_disp ( length_disp ),
		  max_length_disp ( max_length_disp ),

		  element_size ( element_size ),
		  element_gen_disp ( element_gen_disp ),
		  element_stub_disp
		      ( element_stub_disp ),
		  element_io_disp ( NULL ),
		  element_io_names ( NULL ),

		  header_size ( header_size ),
		  header_gen_disp ( header_gen_disp ),
		  header_stub_disp
		      ( header_stub_disp ),
		  header_io_disp ( NULL ),
		  header_io_names ( NULL ),

		  initial_max_length ( 0 ),
		  increment_ratio (0.5),
		  max_increment ( 1000 ) {}
	};

	template < typename E, typename H >
	class packed_vec_ptr_base
	{
	public:

	    packed_vec_ptr_base ( min::gen v )
	    {
	        new ( this )
		    packed_vec_ptr_base
			( stub_of ( v ) );
	    }
	    packed_vec_ptr_base
	        ( const min::stub * s );
	    packed_vec_ptr_base ( void )
	        : s ( NULL_STUB ) {}

	    operator const min::stub * ( void ) const
	    {
		return this->s;
	    }

	    // Returns & pvip[pvip->length] even though
	    // the subscript is not < length.
	    //
	    const E * end_ptr ( void ) const
	    {
		H * hp = (H *)
		    unprotected::ptr_of ( this->s );
		return (E *)
		    ( (min::uns8 *) hp
		      +
		      computed_header_size
		      +
		      hp->length * sizeof ( E ) );
	    }

	protected:
	        
	    const min::stub * s;

	    // Computed actual header size, which is
	    // sizeof ( H ) rounded up to sizeof ( E )
	    // boundary if sizeof ( E ) == 1, 2, or 4,
	    // else rounded up to 8 byte boundary.
	    //
	    static const min::uns32 header_mask =
		sizeof ( E ) == 1 ? 0 :
		sizeof ( E ) == 2 ? 1 :
		sizeof ( E ) == 4 ? 3 : 7;

	public:

	    static const min::uns32
	        computed_header_size =
		    ( sizeof ( H ) + header_mask )
		& ~ header_mask;

	};

	// Out-of-line new_stub function (non-template).
	//
	const min::stub * packed_vec_new_stub
	    ( packed_vec_descriptor * pvd,
	      min::uns32 max_length,
	      min::uns32 length,
	      const void * vp );

	// Out-of-line resize functions (non-template).
	//
	void packed_vec_resize
	    ( const min::stub * s,
	      min::uns32 max_length );
	void packed_vec_resize
	    ( const min::stub * s,
	      packed_vec_descriptor * pvd,
	      min::uns32 max_length );
    }

    struct packed_vec_header
    {
        const min::uns32 control;
        const min::uns32 length;
        const min::uns32 max_length;
    };

    template < typename E,
               typename H = packed_vec_header >
    class packed_vec_ptr
	: public internal::packed_vec_ptr_base<E,H>
    {

    public:

	packed_vec_ptr ( min::gen v )
	    : internal::packed_vec_ptr_base<E,H>
	    ( v ) {}
	packed_vec_ptr ( const min::stub * s )
	    : internal::packed_vec_ptr_base<E,H>
	    ( s ) {}
	packed_vec_ptr ( void )
	    : internal::packed_vec_ptr_base<E,H>
	    () {}

	const H * operator -> ( void ) const
	{
	    return (const H *)
		   unprotected::ptr_of ( this->s );
	}

	const E & operator [] ( min::uns32 i ) const
	{
	    H * hp = (H *)
		unprotected::ptr_of ( this->s );
	    MIN_ASSERT ( i < hp->length );
	    return * (const E *)
		( (min::uns8 *) hp
		  +
		  internal::packed_vec_ptr_base<E,H>
		  ::computed_header_size
		  +
		  i * sizeof ( E ) );
	}

	packed_vec_ptr & operator =
		( const min::stub * s )
	{
	    new ( this )
		internal::packed_vec_ptr_base<E,H>
		    ( s );
	    return * this;
	}

	packed_vec_ptr & operator =
		( min::gen v )
	{
	    new ( this )
		internal::packed_vec_ptr_base<E,H>
		    ( v );
	    return * this;
	}

	static min::uns32 DISP ( void )
	{
	    OFFSETOF ( & packed_vec_ptr::s );
	}
    };

    template < typename S, typename E, typename H >
    min::uns32 DISP
	    ( packed_vec_ptr<E,H> S::* d )
    {
	return   OFFSETOF ( d )
	       + packed_vec_ptr<E,H>::DISP();
    }

    template < typename E,
               typename H = packed_vec_header >
    class packed_vec_updptr
	: public internal::packed_vec_ptr_base<E,H>
    {

    public:

	packed_vec_updptr ( min::gen v )
	    : internal::packed_vec_ptr_base<E,H>
	    ( v ) {}
	packed_vec_updptr
		( const min::stub * s )
	    : internal::packed_vec_ptr_base<E,H>
	    ( s ) {}
	packed_vec_updptr ( void )
	    : internal::packed_vec_ptr_base<E,H>
	    () {}

	H * operator -> ( void ) const
	{
	    return (H *)
		   unprotected::ptr_of ( this->s );
	}

	E & operator [] ( min::uns32 i ) const
	{
	    H * hp = (H *)
		unprotected::ptr_of ( this->s );
	    MIN_ASSERT ( i < hp->length );
	    return * (E *)
		( (min::uns8 *) hp
		  +
		  internal::packed_vec_ptr_base<E,H>
		  ::computed_header_size
		  +
		  i * sizeof ( E ) );
	}

	packed_vec_updptr & operator =
		( const min::stub * s )
	{
	    new ( this )
		internal::packed_vec_ptr_base<E,H>
		    ( s );
	    return * this;
	}

	packed_vec_updptr & operator =
		( min::gen v )
	{
	    new ( this )
		internal::packed_vec_ptr_base<E,H>
		    ( v );
	    return * this;
	}

	static min::uns32 DISP ( void )
	{
	    return OFFSETOF
	        ( & packed_vec_updptr::s );
	}
    };

    template < typename S, typename E, typename H >
    min::uns32 DISP
	    ( packed_vec_updptr<E,H> S::* d )
    {
	return   OFFSETOF ( d )
	       + packed_vec_updptr<E,H>::DISP();
    }

    template < typename E,
               typename H = packed_vec_header >
    class packed_vec_insptr
	: public internal::packed_vec_ptr_base<E,H>
    {

    public:

	packed_vec_insptr ( min::gen v )
	    : internal::packed_vec_ptr_base<E,H>
	    ( v ) {}
	packed_vec_insptr
		( const min::stub * s )
	    : internal::packed_vec_ptr_base<E,H>
	    ( s ) {}
	packed_vec_insptr ( void )
	    : internal::packed_vec_ptr_base<E,H>
	    () {}

	H * operator -> ( void ) const
	{
	    return (H *)
		   unprotected::ptr_of ( this->s );
	}

	E & operator [] ( min::uns32 i ) const
	{
	    H * hp = (H *)
		unprotected::ptr_of ( this->s );
	    MIN_ASSERT ( i < hp->length );
	    return * (E *)
		( (min::uns8 *) hp
		  +
		  internal::packed_vec_ptr_base<E,H>
		  ::computed_header_size
		  +
		  i * sizeof ( E ) );
	}

	packed_vec_insptr & operator =
		( const min::stub * s )
	{
	    new ( this )
		internal::packed_vec_ptr_base<E,H>
		    ( s );
	    return * this;
	}

	packed_vec_insptr & operator =
		( min::gen v )
	{
	    new ( this )
		internal::packed_vec_ptr_base<E,H>
		    ( v );
	    return * this;
	}

	static min::uns32 DISP ( void )
	{
	    return OFFSETOF
	        ( & packed_vec_insptr::s );
	}

	void reserve ( min::uns32 reserve_length );
	void resize  ( min::uns32 max_length )
	{
	    internal::packed_vec_resize
		( this->s, max_length );
	}
    };

    template < typename S, typename E, typename H >
    min::uns32 DISP
	    ( packed_vec_insptr<E,H> S::* d )
    {
	return   OFFSETOF ( d )
	       + packed_vec_insptr<E,H>::DISP();
    }

    template < typename E,
               typename H = packed_vec_header >
    class packed_vec
	: public internal::packed_vec_descriptor
    {

    private:

	// Computed actual header size, which is
	// sizeof ( H ) rounded up to sizeof ( E )
	// boundary if sizeof ( E ) == 1, 2, or 4,
	// else rounded up to 8 byte boundary.
	//
	static const min::uns32 header_mask =
	    sizeof ( E ) == 1 ? 0 :
	    sizeof ( E ) == 2 ? 1 :
	    sizeof ( E ) == 4 ? 3 : 7;

    public:
        static const min::uns32 computed_header_size =
	        ( sizeof ( H ) + header_mask )
	    & ~ header_mask;

    public:

        packed_vec
	    ( const char * name,
	      const min::uns32 * element_gen_disp
	          = NULL,
	      const min::uns32 * element_stub_disp
	          = NULL,
	      const min::uns32 * header_gen_disp
		  = NULL,
	      const min::uns32 * header_stub_disp
	          = NULL,
	      const packed_id & base_class_id =
		  * (packed_id *) NULL );

	const min::stub * new_stub
		( min::uns32 max_length,
		  min::uns32 length = 0,
		  const E * vp = NULL )
	{
	    return internal::packed_vec_new_stub
		( this, max_length, length, vp );
	}
	const min::stub * new_stub ( void )
	{
	    return internal::packed_vec_new_stub
		( this, initial_max_length, 0, NULL );
	}

	min::gen new_gen ( min::uns32 max_length,
	                   min::uns32 length = 0,
			   const E * vp = NULL )
	{
	    return min::new_gen
	        ( internal::packed_vec_new_stub
		    ( this, max_length, length, vp ) );
	}
	min::gen new_gen ( void )
	{
	    return min::new_gen
	        ( internal::packed_vec_new_stub
	              ( this, initial_max_length,
		        0, NULL ) );
	}

	// Notice: Member subclasses of a template
	// class are incompatible with the use of
	// template functions, so using typdefs to
	// get subclasses is a workaround.
	//
	// Specifically, according to the C++
	// standard as of 2010, in
	//
	// template < typename T >
	// void foo ( typename A<T>::B & x ) {...}
	//
	// the T in A<T>::B is in `non-deduced context'
	// and cannot be deduced by parameter matching.
	// So the template function definition of foo
	// will not be visible to a function call such
	// as
	//
	//	A<int>::B x;
	//	foo ( x );
	//
	// which will get a `function not found'
	// compiler error.
	//
	typedef typename
	        min::packed_vec_ptr<E,H> ptr;
	typedef typename
	        min::packed_vec_updptr<E,H> updptr;
	typedef typename
	        min::packed_vec_insptr<E,H> insptr;

	static packed_id id;
    };

    template < typename E, typename H, typename B >
    class packed_vec_with_base
	: public packed_vec<E,H>
    {
    public:

        packed_vec_with_base
	    ( const char * name,
	      const min::uns32 * element_gen_disp
	          = NULL,
	      const min::uns32 * element_stub_disp
	          = NULL,
	      const min::uns32 * header_gen_disp
		  = NULL,
	      const min::uns32 * header_stub_disp
	          = NULL )
	    : packed_vec<E,H>
	          ( name,
		    element_gen_disp,
		    element_stub_disp,
		    header_gen_disp,
		    header_stub_disp,
		    packed_struct<B>::id ) {}
    };

    template < typename E, typename H >
    internal::packed_vec_ptr_base<E,H>
            ::packed_vec_ptr_base
	    ( const min::stub * s ) : s ( s )
    {
	if ( s == NULL ) return;

        MIN_ASSERT ( type_of ( s ) == PACKED_VEC );
	min::uns32 subtype =
	    internal::packed_subtype_of ( s );
	packed_vec_descriptor * pvdescriptor =
	    (packed_vec_descriptor *)
	    (*packed_subtypes)[subtype];
	const packed_id * id = pvdescriptor->id;
	MIN_ASSERT ( ( & packed_vec<E,H>::id == id ) );
    }

    template < typename E, typename H >
    inline E & push
	( typename min::packed_vec_insptr<E,H> & pvip )
    {
	if ( pvip->length >= pvip->max_length )
	    pvip.reserve ( 1 );
	E * p = (E *) pvip.end_ptr();
	memset ( p, 0, sizeof ( E ) );
	++ * (min::uns32 *) & pvip->length;
	return * p;
    }
    template < typename E, typename H >
    inline E & push
	( typename min::packed_vec_insptr<E,H> & pvip,
	  min::uns32 n, const E * vp = NULL )
    {
	if ( n == 0 ) return * (E *) NULL;
	else if ( pvip->length + n > pvip->max_length )
	    pvip.reserve ( n );
	E * p = (E *) pvip.end_ptr();
	if ( vp )
	    memcpy ( p, vp, n * sizeof ( E ) );
	else
	    memset ( p, 0, n * sizeof ( E ) );
	* (min::uns32 *) & pvip->length += n;
	return * p;
    }
    template < typename E, typename H >
    inline E pop
	( typename min::packed_vec_insptr<E,H> & pvip )
    {
	assert ( pvip->length > 0 );
	-- * (min::uns32 *) & pvip->length;
	return * pvip.end_ptr();
    }
    template < typename E, typename H >
    inline void pop
	( typename min::packed_vec_insptr<E,H> & pvip,
	  min::uns32 n, E * vp = NULL )
    {
	assert ( pvip->length >= n );
	* (min::uns32 *) & pvip->length -= n;
	if ( vp )
	    memcpy ( vp,
		     pvip.end_ptr(),
		     n * sizeof ( E ) );
    }

    template < typename E, typename H >
    inline void resize
	( typename min::packed_vec_insptr<E,H> & pvip,
	  min::uns32 max_length )
    {
	pvip.resize ( max_length );
    }

    template < typename E, typename H >
    inline void reserve
	( typename
	  min::packed_vec_insptr<E,H> & pvip,
	  min::uns32 reserve_length )
    {
	if (   pvip->length + reserve_length
	     > pvip->max_length )
	    pvip.reserve ( reserve_length );
    }

    template < typename E, typename H >
    void packed_vec_insptr<E,H>::reserve
	    ( min::uns32 reserve_length )
    {
        H * hp = (H *) unprotected::ptr_of ( this->s );
	min::uns32 subtype = hp->control;
	subtype &=
	    internal::PACKED_CONTROL_SUBTYPE_MASK;
	internal::packed_vec_descriptor * pvdescriptor =
	    (internal::packed_vec_descriptor *)
	    (*internal::packed_subtypes)[subtype];
	    
        min::uns32 new_length = (min::uns32)
	      pvdescriptor->increment_ratio
	    * hp->max_length;
	if (   new_length
	     > pvdescriptor->max_increment )
	    new_length =
	        pvdescriptor->max_increment;
	new_length += hp->max_length;
    	min::uns32 min_new_length =
	    reserve_length + hp->length;
	if ( new_length < min_new_length )
	    new_length = min_new_length;
	internal::packed_vec_resize
	    ( this->s, pvdescriptor, new_length );
    }
}

// The following out-of-line functions should be in
// min.cc, BUT, since they are templates, they must be
// included in every compilation that might instantiate
// them.

template < typename E, typename H >
min::packed_id min::packed_vec<E,H>::id;

template < typename E, typename H >
min::packed_vec<E,H>::packed_vec
    ( const char * name,
      const min::uns32 * element_gen_disp,
      const min::uns32 * element_stub_disp,
      const min::uns32 * header_gen_disp,
      const min::uns32 * header_stub_disp,
      const packed_id & base_class_id )
    : internal::packed_vec_descriptor
          ( min::PACKED_VEC,
	    internal::packed_subtype_count ++,
            & id,
            name,

	    OFFSETOF<H,const min::uns32>
	        ( & H::length ),
	    OFFSETOF<H,const min::uns32>
	        ( & H::max_length ),

	    sizeof ( E ),
            element_gen_disp,
            element_stub_disp,

            computed_header_size,
            header_gen_disp,
            header_stub_disp )
{
    // Check that the control member is the first
    // thing in the H structure.
    //
    MIN_ASSERT
        ( ( OFFSETOF<H,const min::uns32>
	        ( & H::control ) == 0 ) );

    if (   internal::packed_subtype_count
	 > internal::max_packed_subtype_count )
	internal::allocate_packed_subtypes
	    ( internal::max_packed_subtype_count
	      + MIN_PACKED_SUBTYPE_COUNT );
    (*internal::packed_subtypes) [ subtype ] =
        (internal::packed_vec_descriptor *)
	this;
    if ( & base_class_id != id.base )
    {
        MIN_ASSERT ( id.base == NULL );
	id.base = & base_class_id;
    }
}

// Objects
// -------

namespace min {

    const unsigned OBJ_TYPED   = 1;
    const unsigned OBJ_PUBLIC  = 2;
    const unsigned OBJ_PRIVATE = 4;

}

namespace min { namespace internal {

    // Flags Bits:
    //
    // Bit 0 is low order, 15 or 31 is high order.
    //
    //    Short	    Long
    //	  Objects   Objects
    //
    //    0	    0		OBJ_TYPED
    //    1    	    1		OBJ_PUBLIC
    //    2         2		OBJ_PRIVATE
    //	  3	    3-11	reserved for future use
    //    4-15      12-27	total size mantissa (M)
    //    ---       28-31	total size exponent (E)
    //
    //	    total_size = (M+1) << E
    //
    // For short objects more flags could be added to
    // the offsets as only 12 bits of offset is needed.
    //
    // OBJ_TYPE means object first variable (var[0])
    // points at the object's type.
    //
    // OBJ_PUBLIC means vec_insptrs not allowed for
    // object.
    //
    // OBJ_PRIVATE means a vec_xxxptr exists for
    // the object and other vec_xxxptrs may not
    // be created for the object.

    // Codes Bits:
    //
    // Bit 0 is low order, 15 or 31 is high order.
    //
    //    Short	    Long
    //	  Objects   Objects
    //
    //    0-6       0-11	hash size code
    //    7-15      12-31	var size code
    //
    //	    hash size = hash_size[hash size code]
    //	    var size = var size code

    const unsigned SHORT_OBJ_HEADER_SIZE =
        1 + MIN_IS_COMPACT;
    const unsigned LONG_OBJ_HEADER_SIZE =
        2 + 2*MIN_IS_COMPACT;

    const unsigned SHORT_OBJ_FLAG_BITS = 4;
    const unsigned LONG_OBJ_FLAG_BITS  = 12;
    const unsigned SHORT_OBJ_MANTISSA_BITS = 12;
    const unsigned LONG_OBJ_MANTISSA_BITS  = 15;
    const unsigned SHORT_OBJ_EXPONENT_BITS = 0;
    const unsigned LONG_OBJ_EXPONENT_BITS  = 5;

    const unsigned SHORT_OBJ_HASH_CODE_BITS = 7;
    const unsigned LONG_OBJ_HASH_CODE_BITS  = 12;
    const unsigned SHORT_OBJ_VAR_CODE_BITS = 9;
    const unsigned LONG_OBJ_VAR_CODE_BITS  = 20;

    const min::unsptr SHORT_OBJ_MANTISSA_MASK =
        ( ( 1 << SHORT_OBJ_MANTISSA_BITS ) - 1 );
    const min::unsptr LONG_OBJ_MANTISSA_MASK  =
        ( ( 1 << LONG_OBJ_MANTISSA_BITS ) - 1 );
    const min::unsptr SHORT_OBJ_FLAG_MASK =
        ( ( 1 << SHORT_OBJ_FLAG_BITS ) - 1 );
    const min::unsptr LONG_OBJ_FLAG_MASK  =
        ( ( 1 << LONG_OBJ_FLAG_BITS ) - 1 );
    const min::unsptr SHORT_OBJ_HASH_CODE_MASK =
        ( ( 1 << SHORT_OBJ_HASH_CODE_BITS ) - 1 );
    const min::unsptr LONG_OBJ_HASH_CODE_MASK  =
        ( ( 1 << LONG_OBJ_HASH_CODE_BITS ) - 1 );

    const min::unsptr SHORT_OBJ_MAX_VAR_SIZE =
        ( 1 << SHORT_OBJ_VAR_CODE_BITS ) - 1;
    const min::unsptr LONG_OBJ_MAX_VAR_SIZE =
        ( 1 << LONG_OBJ_VAR_CODE_BITS ) - 1;
    const min::unsptr SHORT_OBJ_MAX_HASH_SIZE_CODE =
        ( 1 << SHORT_OBJ_HASH_CODE_BITS ) - 1;
    const min::unsptr LONG_OBJ_MAX_HASH_SIZE_CODE =
        ( 1 << LONG_OBJ_HASH_CODE_BITS ) - 1;
    const min::unsptr SHORT_OBJ_MAX_TOTAL_SIZE =
    	( 1 << SHORT_OBJ_MANTISSA_BITS )
	<<
	( ( 1 << SHORT_OBJ_EXPONENT_BITS ) - 1 );
	// This must be <= (1 << 16) in this implemen-
	// tation because of offsets are stored in
	// min::uns16's.
    const min::unsptr LONG_OBJ_MAX_TOTAL_SIZE =
    	( MIN_IS_COMPACT ? ( 1 << 24 ) :
	  MIN_PTR_BITS <= 32 ?
	  (    (min::unsptr) (-1)
	    << ( 32 - LONG_OBJ_MANTISSA_BITS ) ) :
	  ( 1ull << 32 ) );
	// This must be <= (1 << 32) in this implemen-
	// tation because of offsets are stored in
	// min::uns32's.  It must also be representable
	// in a min::unsptr value and values less than
	// it must be representable in auxiliary
	// pointers.

    //    hash table size = hash_size[hash size code]
    //
    extern min::uns32 hash_size[];

    struct short_obj
    {
        min::uns16	flags;
        min::uns16	codes;
        min::uns16	unused_offset;
        min::uns16	aux_offset;
    };

    struct long_obj
    {
        min::uns32	flags;
        min::uns32	codes;
        min::uns32	unused_offset;
        min::uns32	aux_offset;
    };

    inline min::internal::short_obj * short_obj_of
	    ( const min::stub * s )
    {
        return (min::internal::short_obj *)
	       unprotected::ptr_of ( s );
    }

    inline min::internal::long_obj * long_obj_of
	    ( const min::stub * s )
    {
        return (min::internal::long_obj *)
	       unprotected::ptr_of ( s );
    }

    inline min::unsptr short_obj_total_size_of_flags
	    ( min::unsptr flags )
    {
	flags >>= SHORT_OBJ_FLAG_BITS;
	return (   ( flags & SHORT_OBJ_MANTISSA_MASK )
	         + 1 )
	       <<
	       ( flags >> SHORT_OBJ_MANTISSA_BITS );
    }

    inline min::unsptr long_obj_total_size_of_flags
	    ( min::unsptr flags )
    {
	flags >>= LONG_OBJ_FLAG_BITS;
	return (   ( flags & LONG_OBJ_MANTISSA_MASK )
	         + 1 )
	       <<
	       ( flags >> LONG_OBJ_MANTISSA_BITS );
    }

    inline min::unsptr short_obj_hash_size_of_codes
	    ( min::unsptr codes )
    {
	return hash_size
	    [codes & SHORT_OBJ_HASH_CODE_MASK];
    }

    inline min::unsptr long_obj_hash_size_of_codes
	    ( min::unsptr codes )
    {
	return hash_size
	    [codes & LONG_OBJ_HASH_CODE_MASK];
    }

    inline min::unsptr short_obj_var_size_of_codes
	    ( min::unsptr codes )
    {
	return codes >> SHORT_OBJ_HASH_CODE_BITS;
    }

    inline min::unsptr long_obj_var_size_of_codes
	    ( min::unsptr codes )
    {
	return codes >> LONG_OBJ_HASH_CODE_BITS;
    }

} }

namespace min {

    extern const min::unsptr SHORT_OBJ_MAX_VAR_SIZE;
    extern const min::unsptr SHORT_OBJ_MAX_HASH_SIZE;
    extern const min::unsptr SHORT_OBJ_MAX_TOTAL_SIZE;
    extern const min::unsptr LONG_OBJ_MAX_VAR_SIZE;
    extern const min::unsptr LONG_OBJ_MAX_HASH_SIZE;
    extern const min::unsptr LONG_OBJ_MAX_TOTAL_SIZE;

    min::unsptr obj_var_size ( min::unsptr u );
    min::unsptr obj_hash_size ( min::unsptr u );
    min::unsptr obj_total_size ( min::unsptr u );

    min::gen new_obj_gen
	    ( min::unsptr unused_size,
	      min::unsptr hash_size = 0,
	      min::unsptr var_size = 0 );
}

// Object Vector Level
// ------ ------ -----

namespace min {

    // Vector Pointers:

    // Some forward reference stuff that must be
    // declared here before it is referenced by a
    // friend declaration.
    //
    class obj_vec_ptr;
    class obj_vec_updptr;
    class obj_vec_insptr;

    namespace unprotected {
	const min::gen * & base
	    ( min::obj_vec_ptr & vp );
	min::stub * stub_of
	    ( min::obj_vec_ptr & vp );
	min::unsptr var_offset_of
	    ( min::obj_vec_ptr & vp );
	min::unsptr hash_offset_of
	    ( min::obj_vec_ptr & vp );
	min::unsptr attr_offset_of
	    ( min::obj_vec_ptr & vp );
	min::unsptr unused_offset_of
	    ( min::obj_vec_ptr & vp );
	min::unsptr aux_offset_of
	    ( min::obj_vec_ptr & vp );
    }
    min::unsptr var_size_of
	( min::obj_vec_ptr & vp );
    min::unsptr hash_size_of
	( min::obj_vec_ptr & vp );
    min::unsptr attr_size_of
	( min::obj_vec_ptr & vp );
    min::unsptr unused_size_of
	( min::obj_vec_ptr & vp );
    min::unsptr aux_size_of
	( min::obj_vec_ptr & vp );
    min::unsptr total_size_of
	( min::obj_vec_ptr & vp );
    min::gen var
	( min::obj_vec_ptr & vp, min::unsptr index );
    min::gen hash
	( min::obj_vec_ptr & vp, min::unsptr index );
    min::gen attr
	( min::obj_vec_ptr & vp, min::unsptr index );
    min::gen aux
	( min::obj_vec_ptr & vp, min::unsptr index );

    namespace unprotected {
	min::gen * & base
	    ( min::obj_vec_updptr & vp );
    }
    void set_var
	( min::obj_vec_updptr & vp,
	  min::unsptr index, min::gen value );
    void set_hash
	( min::obj_vec_updptr & vp,
	  min::unsptr index, min::gen value );
    void set_attr
	( min::obj_vec_updptr & vp,
	  min::unsptr index, min::gen value );
    void set_aux
	( min::obj_vec_updptr & vp,
	  min::unsptr index, min::gen value );

    namespace unprotected {
	min::unsptr & unused_offset_of
	    ( min::obj_vec_insptr & vp );
	min::unsptr & aux_offset_of
	    ( min::obj_vec_insptr & vp );
    }

    void attr_push
	( min::obj_vec_insptr & vp,
	  min::gen v );
    void attr_push
	( min::obj_vec_insptr & vp,
	  const min::gen * p, min::unsptr n );
    void aux_push
	( min::obj_vec_insptr & vp,
	  min::gen v );
    void aux_push
	( min::obj_vec_insptr & vp,
	  const min::gen * p, min::unsptr n );

    void attr_pop
	( min::obj_vec_insptr & vp,
	  min::gen & v );
    void attr_pop
	( min::obj_vec_insptr & vp,
	  min::gen * p, min::unsptr n );
    void aux_pop
	( min::obj_vec_insptr & vp,
	  min::gen & v );
    void aux_pop
	( min::obj_vec_insptr & vp,
	  min::gen * p, min::unsptr n );

    bool resize
	( min::obj_vec_insptr & vp,
	  min::unsptr unused_size );
    bool resize
	( min::obj_vec_insptr & vp,
	  min::unsptr unused_size,
	  min::unsptr var_size );

    class obj_vec_ptr
    {
    public:

	obj_vec_ptr ( const min::stub * s )
	    : s ( (min::stub *) s ), type ( READONLY )
	    { init(); }
	obj_vec_ptr ( min::gen v )
	    : s ( (min::stub *) min::stub_of ( v ) ),
	      type ( READONLY )
	    { init(); }
	obj_vec_ptr ( void )
	    : s ( NULL ), type ( READONLY )
	    { init(); }

	~ obj_vec_ptr ( void )
	{
	    // If called by ~ obj_vec_updptr
	    // s == NULL but type == UPDATABLE.
	    //
	    if ( s == NULL ) return;
	    MIN_ASSERT ( type == READONLY );
	    deinit();
	}

	operator const min::stub * ( void ) const
	{
	    return s;
	}

	obj_vec_ptr & operator =
		( const min::stub * s )
	{
	    this->~obj_vec_ptr();
	    new ( this ) obj_vec_ptr ( s );
	    return * this;
	}

	obj_vec_ptr & operator =
		( min::gen v )
	{
	    this->~obj_vec_ptr();
	    new ( this ) obj_vec_ptr ( v );
	    return * this;
	}

        // Friends
	//
	friend const min::gen * & unprotected::base
	    ( min::obj_vec_ptr & vp );
	friend min::stub * unprotected::stub_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr unprotected::var_offset_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr unprotected::hash_offset_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr unprotected::attr_offset_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr unprotected::unused_offset_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr unprotected::aux_offset_of
	    ( min::obj_vec_ptr & vp );

	friend min::unsptr var_size_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr hash_size_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr attr_size_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr unused_size_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr aux_size_of
	    ( min::obj_vec_ptr & vp );
	friend min::unsptr total_size_of
	    ( min::obj_vec_ptr & vp );

	friend min::gen var
	    ( min::obj_vec_ptr & vp,
	      min::unsptr index );
	friend min::gen hash
	    ( min::obj_vec_ptr & vp,
	      min::unsptr index );
	friend min::gen attr
	    ( min::obj_vec_ptr & vp,
	      min::unsptr index );
	friend min::gen aux
	    ( min::obj_vec_ptr & vp,
	      min::unsptr index );

	friend min::gen * & unprotected::base
	    ( min::obj_vec_updptr & vp );
	friend void set_var
	    ( min::obj_vec_updptr & vp,
	      min::unsptr index, min::gen value );
	friend void set_hash
	    ( min::obj_vec_updptr & vp,
	      min::unsptr index, min::gen value );
	friend void set_attr
	    ( min::obj_vec_updptr & vp,
	      min::unsptr index, min::gen value );
	friend void set_aux
	    ( min::obj_vec_updptr & vp,
	      min::unsptr index, min::gen value );

	friend min::unsptr &
	  unprotected::unused_offset_of
	    ( min::obj_vec_insptr & vp );
	friend min::unsptr & unprotected::aux_offset_of
	    ( min::obj_vec_insptr & vp );

	friend void attr_push
	    ( min::obj_vec_insptr & vp,
	      min::gen v );
	friend void attr_push
	    ( min::obj_vec_insptr & vp,
	      const min::gen * p, min::unsptr n );
	friend void aux_push
	    ( min::obj_vec_insptr & vp,
	      min::gen v );
	friend void aux_push
	    ( min::obj_vec_insptr & vp,
	      const min::gen * p, min::unsptr n );

	friend void attr_pop
	    ( min::obj_vec_insptr & vp,
	      min::gen & v );
	friend void attr_pop
	    ( min::obj_vec_insptr & vp,
	      min::gen * p, min::unsptr n );
	friend void aux_pop
	    ( min::obj_vec_insptr & vp,
	      min::gen & v );
	friend void aux_pop
	    ( min::obj_vec_insptr & vp,
	      min::gen * p, min::unsptr n );

	friend bool resize
	    ( min::obj_vec_insptr & vp,
	      min::unsptr unused_size );
	friend bool resize
	    ( min::obj_vec_insptr & vp,
	      min::unsptr unused_size,
	      min::unsptr var_size );

    protected:

	obj_vec_ptr ( const min::stub * s, int type )
	    : s ( (min::stub *) s ), type ( type )
	    { init(); }

        enum {
	    READONLY = 1,
	    UPDATABLE = 2,
	    INSERTABLE = 3 };
	int type;
	    // Set by constructor.  Checked by
	    // destructor.

    	min::stub * s;
	    // Stub of object; set by constructor.  May
	    // be NULL if pointer is not pointing at any
	    // object.

        min::unsptr	unused_offset;
        min::unsptr	aux_offset;
	    // Cached from object header.

	min::unsptr	hash_size;
        min::unsptr	total_size;
	    // Hash table and total size.

	min::unsptr	var_offset;
	min::unsptr	hash_offset;
	min::unsptr	attr_offset;
	    // Offsets of variable vector, hash table,
	    // and attribute vector.

    private:

        void init ( void )
	{
	    if ( s == NULL ) return;

	    int type = min::type_of ( s );
	    min::uns8 * flags_p;
	    if ( type == min::SHORT_OBJ )
	    {
		internal::short_obj * so =
		    internal::short_obj_of ( s );

	        flags_p = (min::uns8 *) & so->flags
		        + MIN_IS_BIG_ENDIAN;

		unused_offset	= so->unused_offset;
		aux_offset	= so->aux_offset;

		var_offset = internal::
		             SHORT_OBJ_HEADER_SIZE;
		hash_offset = var_offset +
		    internal::
		    short_obj_var_size_of_codes
		        ( so->codes );
		hash_size =
		    internal::
		    short_obj_hash_size_of_codes
		        ( so->codes );
		total_size =
		    internal::
		    short_obj_total_size_of_flags
		        ( so->flags );
	    }
	    else
	    {
	        MIN_ASSERT ( type == min::LONG_OBJ );
		internal::long_obj * lo =
		    internal::long_obj_of ( s );

	        flags_p = (min::uns8 *) & lo->flags
		        + 3 * MIN_IS_BIG_ENDIAN;

		unused_offset	= lo->unused_offset;
		aux_offset	= lo->aux_offset;

		var_offset =
		    internal::LONG_OBJ_HEADER_SIZE;
		hash_offset = var_offset +
		    internal::
		    long_obj_var_size_of_codes
		        ( lo->codes );
		hash_size =
		    internal::
		    long_obj_hash_size_of_codes
		        ( lo->codes );
		total_size =
		    internal::
		    long_obj_total_size_of_flags
		        ( lo->flags );
	    }
	    attr_offset = hash_offset + hash_size;

	    MIN_ASSERT
	        ( ( * flags_p & OBJ_PRIVATE ) == 0 );
	    if ( * flags_p & OBJ_PUBLIC )
	    {
	        MIN_ASSERT ( type != INSERTABLE );
	    }
	    else
	        * flags_p |= OBJ_PRIVATE;
	}

    protected:

	void deinit ( void  )
	{
	    if ( s == NULL ) return;

	    int t = min::type_of ( s );
	    if ( t == min::SHORT_OBJ )
	    {
		internal::short_obj * so =
		    internal::short_obj_of ( s );

		so->flags &= ~ OBJ_PRIVATE;
		if ( type == INSERTABLE )
		{
		    so->unused_offset = unused_offset;
		    so->aux_offset    = aux_offset;
		}
	    }
	    else
	    {
	        MIN_ASSERT ( t == min::LONG_OBJ );
		internal::long_obj * lo =
		    internal::long_obj_of ( s );

		lo->flags &= ~ OBJ_PRIVATE;
		if ( type == INSERTABLE )
		{
		    lo->unused_offset = unused_offset;
		    lo->aux_offset    = aux_offset;
		}
	    }

	    s = NULL;
	}
    };

    class obj_vec_updptr : public obj_vec_ptr {

    public:

	obj_vec_updptr ( const min::stub * s )
	    : obj_vec_ptr ( s, UPDATABLE )
	    {}
	obj_vec_updptr ( min::gen v )
	    : obj_vec_ptr
	          ( min::stub_of ( v ), UPDATABLE )
	    {}
	obj_vec_updptr ( void )
	    : obj_vec_ptr ( NULL, UPDATABLE )
	    {}

	~ obj_vec_updptr ( void )
	{
	    // If called by ~ obj_vec_insptr
	    // s == NULL but type == INSERTABLE.
	    //
	    if ( s == NULL ) return;
	    MIN_ASSERT ( type == UPDATABLE );
	    deinit();
	}

	obj_vec_updptr & operator =
		( const min::stub * s )
	{
	    this->~obj_vec_updptr();
	    new ( this ) obj_vec_updptr ( s );
	    return * this;
	}

	obj_vec_updptr & operator =
		( min::gen v )
	{
	    this->~obj_vec_updptr();
	    new ( this ) obj_vec_updptr ( v );
	    return * this;
	}

    protected:

	obj_vec_updptr
		( const min::stub * s, int type )
	    : obj_vec_ptr ( s, type )
	    {}

    };

    class obj_vec_insptr
        : public obj_vec_updptr {

    public:

	obj_vec_insptr ( const min::stub * s )
	    : obj_vec_updptr ( s, INSERTABLE )
	    {}
	obj_vec_insptr ( min::gen v )
	    : obj_vec_updptr
	          ( min::stub_of ( v ), INSERTABLE )
	    {}
	obj_vec_insptr ( void )
	    : obj_vec_updptr ( NULL, INSERTABLE )
	    {}

	~ obj_vec_insptr ( void )
	{
	    MIN_ASSERT ( type == INSERTABLE );
	    deinit();
	}

	obj_vec_insptr & operator =
		( const min::stub * s )
	{
	    this->~obj_vec_insptr();
	    new ( this ) obj_vec_insptr ( s );
	    return * this;
	}

	obj_vec_insptr & operator =
		( min::gen v )
	{
	    this->~obj_vec_insptr();
	    new ( this ) obj_vec_insptr ( v );
	    return * this;
	}
    };

    inline const min::gen * & unprotected::base
	( min::obj_vec_ptr & vp )
    {
	return * (const min::gen **) &
	       min::unprotected::ptr_ref_of
	           ( vp.s );
    }
    inline min::stub * unprotected::stub_of
	( min::obj_vec_ptr & vp )
    {
        return vp.s;
    }
    inline min::unsptr unprotected::var_offset_of
	( min::obj_vec_ptr & vp )
    {
        return vp.var_offset;
    }
    inline min::unsptr unprotected::hash_offset_of
	( min::obj_vec_ptr & vp )
    {
        return vp.hash_offset;
    }
    inline min::unsptr unprotected::attr_offset_of
	( min::obj_vec_ptr & vp )
    {
        return vp.attr_offset;
    }
    inline min::unsptr unprotected::unused_offset_of
	( min::obj_vec_ptr & vp )
    {
        return vp.unused_offset;
    }
    inline min::unsptr unprotected::aux_offset_of
	( min::obj_vec_ptr & vp )
    {
        return vp.aux_offset;
    }

    inline min::unsptr var_size_of
	( min::obj_vec_ptr & vp )
    {
        return vp.hash_offset - vp.var_offset;
    }
    inline min::unsptr hash_size_of
	( min::obj_vec_ptr & vp )
    {
        return vp.hash_size;
    }
    inline min::unsptr attr_size_of
	( min::obj_vec_ptr & vp )
    {
        return vp.unused_offset - vp.attr_offset;
    }
    inline min::unsptr unused_size_of
	( min::obj_vec_ptr & vp )
    {
        return vp.aux_offset - vp.unused_offset;
    }
    inline min::unsptr aux_size_of
	( min::obj_vec_ptr & vp )
    {
        return vp.total_size - vp.aux_offset;
    }
    inline min::unsptr total_size_of
	( min::obj_vec_ptr & vp )
    {
        return vp.total_size;
    }

    inline min::gen var
	( min::obj_vec_ptr & vp, min::unsptr index )
    {
	index += vp.var_offset;
	MIN_ASSERT ( index < vp.hash_offset );
	return unprotected::base(vp)[index];
    }

    inline min::gen hash
	( min::obj_vec_ptr & vp, min::unsptr index )
    {
	index += vp.hash_offset;
	MIN_ASSERT ( index < vp.attr_offset );
	return unprotected::base(vp)[index];
    }

    inline min::gen attr
	( min::obj_vec_ptr & vp, min::unsptr index )
    {
	index += vp.attr_offset;
	MIN_ASSERT ( index < vp.unused_offset );
	return unprotected::base(vp)[index];
    }

    inline min::gen aux
	( min::obj_vec_ptr & vp, min::unsptr index )
    {
	index = vp.total_size - index;
	MIN_ASSERT ( vp.aux_offset <= index );
	MIN_ASSERT ( index < vp.total_size );
	return unprotected::base(vp)[index];
    }

    inline min::gen * & unprotected::base
	( min::obj_vec_updptr & vp )
    {
	return * (min::gen **) &
	       min::unprotected::ptr_ref_of
	           ( vp.s );
    }

    inline void set_var
	( min::obj_vec_updptr & vp,
	  min::unsptr index, min::gen value )
    {
	index += vp.var_offset;
	MIN_ASSERT ( index < vp.hash_offset );
	unprotected::base(vp)[index] = value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void set_hash
	( min::obj_vec_updptr & vp,
	  min::unsptr index, min::gen value )
    {
	index += vp.hash_offset;
	MIN_ASSERT ( index < vp.attr_offset );
	unprotected::base(vp)[index] = value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void set_attr
	( min::obj_vec_updptr & vp,
	  min::unsptr index, min::gen value )
    {
	index += vp.attr_offset;
	MIN_ASSERT ( index < vp.unused_offset );
	unprotected::base(vp)[index] = value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void set_aux
	( min::obj_vec_updptr & vp,
	  min::unsptr index, min::gen value )
    {
	index = vp.total_size - index;
	MIN_ASSERT ( vp.aux_offset <= index );
	MIN_ASSERT ( index < vp.total_size );
	unprotected::base(vp)[index] = value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline min::unsptr & unprotected::unused_offset_of
	( min::obj_vec_insptr & vp )
    {
        return vp.unused_offset;
    }
    inline min::unsptr & unprotected::aux_offset_of
	( min::obj_vec_insptr & vp )
    {
        return vp.aux_offset;
    }

    inline void attr_push
	( min::obj_vec_insptr & vp,
	  min::gen value )
    {
	MIN_ASSERT ( vp.unused_offset < vp.aux_offset );
	unprotected::base(vp)[vp.unused_offset ++] =
	    value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void attr_push
	( min::obj_vec_insptr & vp,
	  const min::gen * p, min::unsptr n )
    {
	MIN_ASSERT
	    ( vp.unused_offset + n <= vp.aux_offset );
	memcpy (   unprotected::base(vp)
	         + vp.unused_offset,
	         p, sizeof ( min::gen ) * n );
	vp.unused_offset += n;
	unprotected::acc_write_update ( vp.s, p, n );
    }

    inline void aux_push
	( min::obj_vec_insptr & vp,
	  min::gen value )
    {
	MIN_ASSERT ( vp.unused_offset < vp.aux_offset );
	unprotected::base(vp)[-- vp.aux_offset] = value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void aux_push
	( min::obj_vec_insptr & vp,
	  const min::gen * p, min::unsptr n )
    {
	MIN_ASSERT
	    ( vp.unused_offset + n <= vp.aux_offset );
	vp.aux_offset -= n;
	memcpy ( unprotected::base(vp) + vp.aux_offset,
	         p, sizeof ( min::gen ) * n );
	unprotected::acc_write_update ( vp.s, p, n );
    }

    inline void attr_pop
	( min::obj_vec_insptr & vp,
	  min::gen & v )
    {
	MIN_ASSERT
	    ( vp.attr_offset < vp.unused_offset );
	v = unprotected::base(vp)[-- vp.unused_offset];
    }

    inline void attr_pop
	( min::obj_vec_insptr & vp,
	  min::gen * p, min::unsptr n )
    {
	MIN_ASSERT
	    ( vp.attr_offset + n <= vp.unused_offset );
	vp.unused_offset -= n;
	memcpy
	    ( p,
	      unprotected::base(vp) + vp.unused_offset,
	      sizeof ( min::gen ) * n );
    }

    inline void aux_pop
	( min::obj_vec_insptr & vp,
	  min::gen & v )
    {
	MIN_ASSERT ( vp.aux_offset < vp.total_size );
	v = unprotected::base(vp)[vp.aux_offset ++];
    }

    inline void aux_pop
	( min::obj_vec_insptr & vp,
	  min::gen * p, min::unsptr n )
    {
	MIN_ASSERT
	    ( vp.aux_offset + n <= vp.total_size );
	memcpy ( p,
		 unprotected::base(vp) + vp.aux_offset,
	         sizeof ( min::gen ) * n );
	vp.aux_offset += n;
    }
}


// Object List Level
// ------ ---- -----

// The control for LIST_AUX and SUBLIST_AUX object
// auxiliary stubs is either a list auxilary pointer
// with index in the value field of the stub control,
// or is a stub pointer to another LIST_AUX stub with
// the MUP::STUB_PTR flag and the stub pointer in the
// control.

// Aux area elements that are not used are given the
// value NONE, and can be garbage collected when the
// object is reorganized.  Because they are often
// isolated, no attempt is made to put them on a free
// list.

namespace min { namespace unprotected {

    // This is the generic list pointer type from which
    // specific list pointer types are made.

    template < class vecpt >
        class list_ptr_type;

} }

namespace min {

    // Public list pointer types.

    typedef min::unprotected::list_ptr_type
	    < min::obj_vec_ptr >
        list_ptr;
    typedef min::unprotected::list_ptr_type
	    < min::obj_vec_updptr >
        list_updptr;
    typedef min::unprotected::list_ptr_type
	    < min::obj_vec_insptr >
        list_insptr;

    // List constants.

    const min::gen LIST_END = (min::gen)
	( (min::unsgen) min::GEN_LIST_AUX << VSIZE );
    const min::gen EMPTY_SUBLIST = (min::gen)
	(    (min::unsgen) min::GEN_SUBLIST_AUX
	  << VSIZE );

    // Global data.

    extern bool use_obj_aux_stubs;
}

namespace min { namespace internal {

    // Internal functions: see min.cc.
    //
#   if MIN_USE_OBJ_AUX_STUBS
	void allocate_stub_list
	    ( min::stub * & first,
	      min::stub * & last,
	      int type,
	      const min::gen * p, min::unsptr n,
	      min::uns64 end );
#   endif

    // Out of line versions of functions: see min.cc.
    //
    bool insert_reserve
    	    ( min::list_insptr & lp,
	      min::unsptr insertions,
	      min::unsptr elements,
	      bool use_obj_aux_stubs );

    void remove_list
	    ( min::gen * & base,
	      min::unsptr total_size,
	      min::unsptr index
#	      if MIN_USE_OBJ_AUX_STUBS
	          , min::stub * s = NULL
#	      endif
	      );

    // If v is a non-empty sublist pointer, remove
    // the sublist.  Base is a list pointer base,
    // and total_size is the object total size.
    //
    inline void remove_sublist
        ( min::gen * & base,
	  min::unsptr total_size,
	  min::gen v )
    {
#       if MIN_USE_OBJ_AUX_STUBS
	    if ( min::is_stub ( v ) )
	    {
		min::stub * s =
		    min::unprotected::stub_of ( v );
		if (    min::type_of ( s )
		     == min::SUBLIST_AUX )
		    remove_list
		        ( base, total_size, 0, s );
	    }
	    else
#       endif
	if ( min::is_sublist_aux ( v )
	     &&
	     v != min::EMPTY_SUBLIST )
	    remove_list
	        ( base, total_size,
		    total_size
		  - min::sublist_aux_of ( v  ) );
    }
} }

namespace min {

    inline bool is_list_end ( min::gen v )
    {
        return v == min::LIST_END;
    }
    inline bool is_sublist ( min::gen v )
    {
        return min::is_sublist_aux ( v )
#              if MIN_USE_OBJ_AUX_STUBS
		   ||
		   ( min::is_stub ( v )
		     &&
		     min::type_of ( min::unprotected
		                       ::stub_of ( v ) )
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
    vecpt & obj_vec_ptr_of
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp );

    template < class vecpt >
    min::gen start_hash
            ( min::unprotected
	         ::list_ptr_type<vecpt> & lp,
	      min::unsptr index );
    template < class vecpt >
    min::gen start_vector
            ( min::unprotected
	         ::list_ptr_type<vecpt> & lp,
	      min::unsptr index );
    template < class vecpt, class vecpt2 >
    min::gen start_copy
            ( min::unprotected
	         ::list_ptr_type<vecpt> & lp,
	      const
	      min::unprotected
	         ::list_ptr_type<vecpt2> & lp2 );
    template < class vecpt, class vecpt2 >
    min::gen start_sublist
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp,
    	      const min::unprotected
	         ::list_ptr_type<vecpt2> & lp2 );
    template < class vecpt >
    min::gen start_sublist
    	    ( min::list_insptr & lp,
    	      const min::unprotected
	         ::list_ptr_type<vecpt> & lp2 );

    template < class vecpt >
    min::gen next
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp );
    template < class vecpt >
    min::gen peek
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp );
    template < class vecpt >
    min::gen current
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp );
    template < class vecpt >
    min::gen update_refresh
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp );
    template < class vecpt >
    min::gen insert_refresh
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp );

    template < class vecpt >
    void update
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp,
	      min::gen value );

    bool insert_reserve
    	    ( min::list_insptr & lp,
	      min::unsptr insertions,
	      min::unsptr elements = 0,
	      bool use_obj_aux_stubs =
	          min::use_obj_aux_stubs );
    void insert_before
    	    ( min::list_insptr & lp,
	      const min::gen * p, min::unsptr n );
    void insert_after
    	    ( min::list_insptr & lp,
	      const min::gen * p, min::unsptr n );
    min::unsptr remove
    	    ( min::list_insptr & lp,
	      min::unsptr n = 1 );
}

namespace min { namespace unprotected {

    template <>
	class list_ptr_type
	          <min::obj_vec_insptr> {

    public:

        list_ptr_type
		( min::obj_vec_insptr & vecp )
	    : vecp ( vecp ),
	      base ( min::unprotected::base ( vecp ) ),
	      hash_offset ( 0 ), total_size ( 0 )
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
#	    if MIN_USE_OBJ_AUX_STUBS
		use_obj_aux_stubs = false;
#	    endif

	    current_index = 0;
	    previous_index = 0;
	    head_index = 0;
#	    if MIN_USE_OBJ_AUX_STUBS
		current_stub = NULL;
		previous_stub = NULL;
#	    endif
	    previous_is_sublist_head = false;
	}

    private:

    // Private Data:

	min::obj_vec_insptr & vecp;
	min::gen * & base;
	    // Vector pointer to object and base(vecp).

	min::gen current;
	    // Value of current element, as returned by
	    // the min::current function.  Set to LIST_
	    // END by constructor.

	min::unsptr current_index;
	min::unsptr previous_index;
	    // See below.  IMPORTANT NOTE: Unlike
	    // aux pointers, these indices are relative
	    // to the base even if they point into the
	    // aux area.
	bool previous_is_sublist_head;
	    // True if previous pointer, if it exists,
	    // points at a sublist pointer.  See below.

	min::unsptr head_index;
	    // The value of current_index when the list
	    // is started by start_hash or start_vector.
	    // This is an index relative to the object
	    // body vector base of the element in the
	    // hash table or attribute vector where the
	    // list pointer was started.  Current_index
	    // or previous_index point into the hash
	    // table or attribute vector if and only if
	    // they equal head_index.  0 if list pointer
	    // has not yet been started.

#	if MIN_USE_OBJ_AUX_STUBS
	    min::stub * current_stub;
	    min::stub * previous_stub;
	        // See below.
	    bool use_obj_aux_stubs;
		// True if list auxiliary stubs are to
		// be used for insertions if space in
		// the object auxiliary area runs out.
#	endif

	min::unsptr reserved_insertions;
	min::unsptr reserved_elements;
	    // Set by insert_reserve and decremented by
	    // insert_{before,after}.  The latter dec-
	    // rement reserved_insertions once and
	    // decrement reserved_elements once for
	    // each element inserted.  These counters
	    // must never become less than 0 (else
	    // assert violation).

	min::unsptr hash_offset;
	min::unsptr total_size;
	    // Save of hash_offset and total_size from
	    // obj_vec pointer.  0 if list pointer has
	    // not yet been started.
	    //
	    // Used by refresh to adjust indices if
	    // resize has been called for another list
	    // pointer to the same object.
	    //
	    // Adjusting an index is done by as follows:
	    //
	    //   Let offset_adjust =
	    //         new_hash_offset - old_hash_offset
	    //       size_adjust =
	    //         new_total_size - old_total_size
	    //   If old_index == 0:
	    //	   new_index = 0
	    //	 Else if old_index == head_index
	    //      new_index =
	    //        old_index + offset_adjust
	    //   Else
	    //      new_index =
	    //        old_index + size_adjust

	// Abstractly there is a current pointer and a
	// previous pointer.  The current pointer points
	// at the current value.  This current value
	// might also be pointed at by a list or sublist
	// pointer in the object, and in this case the
	// previous pointer in the list pointer is set
	// to point at this list or sublist pointer,
	// so the latter can be updated if there is an
	// insert before the current position or a
	// removal of the current element.
	//
	// The gen_of value of the auxiliary stub is
	// equivalent to an auxiliary area element
	// pointed at by a sublist or list pointer.  The
	// control_of value of the stub is equivalent
	// to the next value after that in a list, but
	// that next value must be a list pointer or
	// LIST_END (and cannot be a sublist pointer).
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
	// If current == LIST_END one of the following
	// special cases applies.
	//
	//   Current pointer exists:
	//	current_index != 0
	//	base[current_index] == LIST_END
	//	previous pointer does NOT exist
	//
	//   Current pointer does NOT exist,
	//   previous pointer points at list head in the
	//	    attribute vector or hash table:
	//      [current is the virtual LIST_END after
	//       a 1-element list whose first element
	//       is the list head]
	//      previous_index != 0
	//      previous_index == head_index
	//      previous_is_sublist_header == false
	//	base[previous_index] is the sole element
	//	    of a 1-element list
	//
	//   Current pointer does NOT exist,
	//   previous pointer exists,
	//   previous_is_sublist_header == true:
	//      [current is the virtual LIST_END at the
	//	 end of an EMPTY_SUBLIST]
	//      previous == EMPTY_SUBLIST
	//
	//   Current pointer does NOT exist,
	//   previous pointer exists and points at a
	//            stub,
	//   previous_is_sublist_header == false:
	//      [current is the LIST_END in the stub
	//       control]
	//      control_of ( previous_stub ) == LIST_END
	//
	//   Current pointer does NOT exist,
	//   previous pointer does NOT exist:
	//      [current is virtual LIST_END of a list
	//       pointer that has never been started]
	//      The list pointer has never been started
	//      by a start_... function.   All
	//      operations should treat the pointer
	//      as pointing at an empty list into
	//      which insertions CANNOT be made.
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
	// A list or sublist pointer cannot point at
	// a list pointer or LIST_END.

    // Friends:

	friend min::obj_vec_insptr &
	    obj_vec_ptr_of<>
		( min::list_insptr & lp );

	friend min::gen min::start_hash<>
		( min::list_insptr & lp,
		  min::unsptr index );
	friend min::gen min::start_vector<>
		( min::list_insptr & lp,
		  min::unsptr index );

	friend min::gen start_copy<>
		( min::list_insptr & lp,
		  const
		  min::list_insptr & lp2 );

	friend min::gen start_sublist<>
		( min::list_ptr & lp,
		  const
		  min::list_insptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_updptr & lp,
		  const
		  min::list_insptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
		  const
		  min::list_ptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
		  const
		  min::list_updptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
		  const
		  min::list_insptr & lp2 );

	friend min::gen min::next<>
		( min::list_insptr & lp );
	friend min::gen min::peek<>
		( min::list_insptr & lp );
	friend min::gen min::current<>
		( min::list_insptr & lp );
	friend min::gen min::update_refresh<>
		( min::list_insptr & lp );
	friend min::gen min::insert_refresh<>
		( min::list_insptr & lp );

	friend void min::update<>
		( min::list_insptr & lp,
		  min::gen value );
	friend bool min::insert_reserve
		( min::list_insptr & lp,
		  min::unsptr insertions,
		  min::unsptr elements,
		  bool use_obj_aux_stubs );
	friend bool min::internal::insert_reserve
		( min::list_insptr & lp,
		  min::unsptr insertions,
		  min::unsptr elements,
		  bool use_obj_aux_stubs );
	friend void min::insert_before
		( min::list_insptr & lp,
		  const min::gen * p, min::unsptr n );
	friend void min::insert_after
		( min::list_insptr & lp,
		  const min::gen * p, min::unsptr n );
	friend min::unsptr min::remove
		( min::list_insptr & lp,
		  min::unsptr n );

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
	min::gen forward ( min::unsptr index )
	{
	    // The code of remove depends upon the fact
	    // that this function does not READ
	    // lp.previous_stub or lp.previous_index
	    // (forward is called from next is called
	    // from remove).

	    current_index = index;
	    current = base[current_index];

	    previous_index = 0;
	    previous_is_sublist_head = false;

#           if MIN_USE_OBJ_AUX_STUBS
		current_stub = NULL;
		previous_stub = NULL;
#	    endif

	    if ( min::is_list_aux ( current ) )
	    {
		if ( current != min::LIST_END )
		{
		    previous_index = current_index;
		    current_index =
			  total_size
			- min::list_aux_of ( current );
		    current = base[current_index];
		}
	    }
#           if MIN_USE_OBJ_AUX_STUBS
		else if ( min::is_stub ( current ) )
		{
		    min::stub * s =
		        min::unprotected
			   ::stub_of ( current );
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

    // The following is for min::list_ptrs and min::
    // list_updptrs and is just like the class
    // for min::list_insptrs except that the
    // previous pointer and the reservations are
    // omitted.
    //
    template < class vecpt >
	class list_ptr_type {

    public:

        list_ptr_type ( vecpt & vecp )
	    : vecp ( vecp ),
	      base ( * (min::gen **) &
	             min::unprotected::base ( vecp ) ),
	      hash_offset ( 0 ), total_size ( 0 )
	{
	    current = min::LIST_END;
	    current_index = 0;
	    head_index = 0;
#	    if MIN_USE_OBJ_AUX_STUBS
		current_stub = NULL;
#	    endif
	}

    private:

    // Private Data:

	vecpt & vecp;
	min::gen * & base;
	min::gen current;
	min::unsptr current_index;
	min::unsptr head_index;

#	if MIN_USE_OBJ_AUX_STUBS
	    min::stub * current_stub;
#	endif

	min::unsptr hash_offset;
	min::unsptr total_size;

    // Friends:

	friend vecpt & obj_vec_ptr_of<>
		( min::unprotected
		     ::list_ptr_type<vecpt> & lp );

	friend min::gen min::start_hash<>
		( min::unprotected
		     ::list_ptr_type<vecpt> & lp,
		  min::unsptr index );
	friend min::gen min::start_vector<>
		( min::unprotected
		     ::list_ptr_type<vecpt> & lp,
		  min::unsptr index );

	friend min::gen start_copy<>
		( min::list_ptr & lp,
		  const
		  min::list_ptr & lp2 );
	friend min::gen start_copy<>
		( min::list_ptr & lp,
		  const
		  min::list_updptr & lp2 );
	friend min::gen start_copy<>
		( min::list_ptr & lp,
		  const
		  min::list_insptr & lp2 );
	friend min::gen start_copy<>
		( min::list_updptr & lp,
		  const
		  min::list_updptr & lp2 );
	friend min::gen start_copy<>
		( min::list_updptr & lp,
		  const
		  min::list_insptr & lp2 );

	friend min::gen start_sublist<>
		( min::list_ptr & lp,
		  const
		  min::list_ptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_ptr & lp,
		  const
		  min::list_updptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_ptr & lp,
		  const
		  min::list_insptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_updptr & lp,
		  const
		  min::list_ptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_updptr & lp,
		  const
		  min::list_updptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_updptr & lp,
		  const
		  min::list_insptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
		  const
		  min::list_ptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
		  const
		  min::list_updptr & lp2 );

	friend min::gen min::next<>
		( min::unprotected
		     ::list_ptr_type<vecpt> & lp );
	friend min::gen min::peek<>
		( min::unprotected
		     ::list_ptr_type<vecpt> & lp );
	friend min::gen min::current<>
		( min::unprotected
		     ::list_ptr_type<vecpt> & lp );
	friend min::gen min::update_refresh<>
		( min::unprotected
		     ::list_ptr_type<vecpt> & lp );
	friend min::gen min::insert_refresh<>
		( min::unprotected
		     ::list_ptr_type<vecpt> & lp );

	friend void min::update<>
		( min::list_updptr & lp,
		  min::gen value );

	min::gen forward ( min::unsptr index )
	{
	    current_index = index;
	    current = base[current_index];

#           if MIN_USE_OBJ_AUX_STUBS
		current_stub = NULL;
#	    endif

	    if ( min::is_list_aux ( current ) )
	    {
		if ( current != min::LIST_END )
		{
		    current_index =
		          total_size
			- min::list_aux_of ( current );
		    current = base[current_index];
		}
	    }
#           if MIN_USE_OBJ_AUX_STUBS
		else if ( min::is_stub ( current ) )
		{
		    min::stub * s =
		        min::unprotected
			   ::stub_of ( current );
		    int type = min::type_of ( s );

		    if ( type == min::LIST_AUX )
		    {
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
    inline vecpt & obj_vec_ptr_of
	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp )
    {
    	return lp.vecp;
    }

    template < class vecpt >
    inline min::gen start_hash
            ( min::unprotected
	         ::list_ptr_type<vecpt> & lp,
	      min::unsptr index )
    {
	lp.hash_offset =
	    min::unprotected
	       ::hash_offset_of ( lp.vecp );
	lp.total_size = min::total_size_of ( lp.vecp );

	MIN_ASSERT ( index < hash_size_of ( lp.vecp ) );

	lp.head_index = lp.hash_offset + index;
	return lp.forward ( lp.head_index );
    }

    template < class vecpt >
    inline min::gen start_vector
            ( min::unprotected
	         ::list_ptr_type<vecpt> & lp,
	      min::unsptr index )
    {
	lp.hash_offset =
	    min::unprotected
	       ::hash_offset_of ( lp.vecp );
	lp.total_size = min::total_size_of ( lp.vecp );

	lp.head_index =
	      unprotected::attr_offset_of ( lp.vecp )
	    + index;
	MIN_ASSERT
	    (   lp.head_index
	      < unprotected::unused_offset_of
	            ( lp.vecp ) );
	return lp.forward ( lp.head_index );
    }

    // start_copy is declared as a friend only for
    // legal combinations of list pointers, e.g.,
    //
    // start_copy ( list_updptr & lp,
    //		    list_insptr & lp2 )
    //
    // is allowed but
    //
    // start_copy ( list_insptr & lp,
    //		    list_updptr & lp2 )
    //
    // is not allowed (not a friend).
    //
    template < class vecpt, class vecpt2 >
    inline min::gen start_copy
            ( min::unprotected
	         ::list_ptr_type<vecpt> & lp,
	      const
	      min::unprotected
	         ::list_ptr_type<vecpt2> & lp2 )
    {
	lp.hash_offset =
	    min::unprotected
	       ::hash_offset_of ( lp.vecp );
	lp.total_size = min::total_size_of ( lp.vecp );

        MIN_ASSERT ( & lp.vecp == & lp2.vecp );
	MIN_ASSERT ( lp.total_size == lp2.total_size );

	lp.current_index = lp2.current_index;
	lp.head_index = lp2.head_index;
#       if MIN_USE_OBJ_AUX_STUBS
	    lp.current_stub = lp2.current_stub;
#       endif
	return lp.current = lp2.current;
    }

    template <>
    inline min::gen start_copy
            ( min::list_insptr & lp,
	      const
	      min::list_insptr & lp2 )
    {
	lp.hash_offset =
	    min::unprotected
	       ::hash_offset_of ( lp.vecp );
	lp.total_size = min::total_size_of ( lp.vecp );

        MIN_ASSERT ( & lp.vecp == & lp2.vecp );
	MIN_ASSERT ( lp.total_size == lp2.total_size );

	lp.current_index = lp2.current_index;
	lp.previous_index = lp2.previous_index;
	lp.head_index = lp2.head_index;
	lp.previous_is_sublist_head =
	    lp2.previous_is_sublist_head;
#       if MIN_USE_OBJ_AUX_STUBS
	    lp.current_stub = lp2.current_stub;
	    lp.previous_stub = lp2.previous_stub;
#       endif
	return lp.current = lp2.current;
    }

    // start_sublist is declared as a friend only for
    // legal combinations of list pointers, just like
    // start_copy above.
    //
    template < class vecpt, class vecpt2 >
    inline min::gen start_sublist
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp,
    	      const min::unprotected
	         ::list_ptr_type<vecpt2> & lp2 )
    {
	// We want the total size check to work even
	// if & lp == & lp2.
	//
	lp.hash_offset =
	    min::unprotected
	       ::hash_offset_of ( lp.vecp );
	min::unsptr total_size =
	    min::total_size_of ( lp.vecp );

        MIN_ASSERT ( & lp.vecp == & lp2.vecp );
	MIN_ASSERT ( total_size == lp2.total_size );
	lp.total_size = total_size;

	lp.head_index = lp2.head_index;

#	if MIN_USE_OBJ_AUX_STUBS
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
	{
	    lp.current_index =
		  lp.total_size
		- lp.current_index;
	    lp.current =
		lp.base[lp.current_index];
	}

	return lp.current;
    }
    template <class vecpt>
    inline min::gen start_sublist
    	    ( min::list_insptr & lp,
    	      const min::unprotected
	         ::list_ptr_type<vecpt> & lp2 )
    {
	// We want the total size check to work even
	// if & lp == & lp2.
	//
	lp.hash_offset =
	    min::unprotected
	       ::hash_offset_of ( lp.vecp );
	min::unsptr total_size =
	    min::total_size_of ( lp.vecp );

        MIN_ASSERT ( & lp.vecp == & lp2.vecp );
	MIN_ASSERT ( total_size == lp2.total_size );
	lp.total_size = total_size;

	lp.previous_index = lp2.current_index;
	lp.head_index = lp2.head_index;
	lp.previous_is_sublist_head = true;

#	if MIN_USE_OBJ_AUX_STUBS
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
	{
	    lp.current_index =
		  lp.total_size
		- lp.current_index;
	    lp.current =
		lp.base[lp.current_index];
	}

	return lp.current;
    }

    template < class vecpt >
    inline min::gen next
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp )
    {
        if ( lp.current == min::LIST_END )
	    return min::LIST_END;

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
	        min::uns64 c =
		    min::unprotected::control_of
		    	( lp.current_stub );
		if ( c & min::unprotected::STUB_PTR )
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
		    {
		        lp.current_index =
			      lp.total_size
			    - lp.current_index;
			lp.current
			    = lp.base[lp.current_index];
		    }

		    return lp.current;
		}
	    }
#       endif

	if ( lp.current_index == lp.head_index )
	{
	    // Current is list (not sublist) head.
	    //
	    lp.current_index = 0;
	    return lp.current = min::LIST_END;
	}
	else
	    return lp.forward ( lp.current_index - 1 );
    }
    template <>
    inline min::gen next
    	    ( min::list_insptr & lp )
    {
        // The code of remove ( lp ) depends upon the
	// fact that this function does not READ
	// lp.previous_stub or lp.previous_index.

        if ( lp.current == min::LIST_END )
	    return min::LIST_END;

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
		lp.previous_index = 0;
		lp.previous_is_sublist_head = false;
		lp.previous_stub = lp.current_stub;

	        min::uns64 c =
		    min::unprotected::control_of
		    	( lp.current_stub );
		if ( c & min::unprotected::STUB_PTR )
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
		    lp.current_stub = NULL;

		    lp.current_index =
		        min::unprotected::
			     value_of_control ( c );
		    if ( lp.current_index == 0 )
			lp.current = min::LIST_END;
		    else
		    {
			lp.current_index =
			      lp.total_size
			    - lp.current_index;
			lp.current
			    = lp.base[lp.current_index];
		    }

		    return lp.current;
		}
	    }
#       endif

	if ( lp.current_index == lp.head_index )
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
    inline min::gen peek
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp )
    {
        if ( lp.current == min::LIST_END )
	    return min::LIST_END;

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
	        min::uns64 c =
		    min::unprotected::control_of
		    	( lp.current_stub );
		if ( c & min::unprotected::STUB_PTR )
		{
		    min::stub * s =
		        min::unprotected
			   ::stub_of_control ( c );
		    return min::unprotected
		              ::gen_of ( s );
		}
		else
		{
		    min::unsptr index =
		        min::unprotected
			   ::value_of_control ( c );
		    if ( index == 0 )
			return min::LIST_END;
		    else
		    {
		        index =
			      lp.total_size
			    - index;
			return lp.base[index];
		    }
		}
	    }
#       endif

	if ( lp.current_index == lp.head_index )
	{
	    // Current is list (not sublist) head.
	    //
	    return min::LIST_END;
	}
	else
	{
	    min::unsptr index = lp.current_index;
	    MIN_ASSERT ( index != 0 );
	    -- index;
	    min::gen c = lp.base[index];

	    if ( min::is_list_aux ( c ) )
	    {
		index = min::list_aux_of ( c );
		if ( index == 0 )
		    return min::LIST_END;
		else
		{
		    index = lp.total_size - index;
		    return lp.base[index];
		}
	    }
#           if MIN_USE_OBJ_AUX_STUBS
		else if ( min::is_stub ( c ) )
		{
		    min::stub * s =
		        min::unprotected
			   ::stub_of ( c );
		    int type = min::type_of ( s );

		    if ( type == min::LIST_AUX )
		    {
			return min::unprotected::
			            gen_of ( s );
		    }
		}
#           endif

	    else
		return c;
        }
    }

    template < class vecpt >
    inline min::gen current
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp )
    {
    	return lp.current;
    }

    template < class vecpt >
    inline min::gen update_refresh
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp )
    {
	if ( lp.current_index != 0 )
	    return lp.current =
	    		lp.base[lp.current_index];
#       if MIN_USE_OBJ_AUX_STUBS
	    else if ( lp.current_stub != NULL )
		return lp.current =
			   min::unprotected::
			        gen_of
			          ( lp.current_stub );
#	endif
	else if ( lp.current == min::LIST_END )
	    return lp.current;

	MIN_ABORT ( "inconsistent list pointer" );
    }

    template < class vecpt >
    inline min::gen insert_refresh
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp )
    {
	min::unsptr new_hash_offset =
	    min::unprotected
	       ::hash_offset_of ( lp.vecp );
	min::unsptr new_total_size =
	    min::total_size_of ( lp.vecp );

	min::unsptr adjust = new_hash_offset
	                   - lp.hash_offset;
	if ( lp.current_index != 0 )
	{
	    if ( lp.current_index == lp.head_index )
	        lp.current_index += adjust;
	    else
	        lp.current_index += new_total_size
		                  - lp.total_size;
	}
	if ( lp.head_index != 0 )
	        lp.head_index += adjust;

	lp.hash_offset = new_hash_offset;
	lp.total_size = new_total_size;

	if ( lp.current_index != 0 )
	    return lp.current =
	    		lp.base[lp.current_index];
#       if MIN_USE_OBJ_AUX_STUBS
	    else if ( lp.current_stub != NULL )
		return lp.current =
			   min::unprotected::
			        gen_of
			          ( lp.current_stub );
#	endif
	else if ( lp.current == min::LIST_END )
	    return lp.current;

	MIN_ABORT ( "inconsistent list pointer" );
    }

    template <>
    inline min::gen insert_refresh
    	    ( min::list_insptr & lp )
    {
	min::unsptr new_hash_offset =
	    min::unprotected
	       ::hash_offset_of ( lp.vecp );
	min::unsptr new_total_size =
	    min::total_size_of ( lp.vecp );

	min::unsptr adjust = new_hash_offset
	                   - lp.hash_offset;
	if ( lp.current_index != 0 )
	{
	    if ( lp.current_index == lp.head_index )
	        lp.current_index += adjust;
	    else
	        lp.current_index += new_total_size
		                  - lp.total_size;
	}
	if ( lp.previous_index != 0 )
	{
	    if ( lp.previous_index == lp.head_index )
	        lp.previous_index += adjust;
	    else
	        lp.previous_index += new_total_size
		                   - lp.total_size;
	}
	if ( lp.head_index != 0 )
	        lp.head_index += adjust;

	lp.hash_offset = new_hash_offset;
	lp.total_size = new_total_size;

	if ( lp.current_index != 0 )
	    return lp.current =
	    		lp.base[lp.current_index];
#       if MIN_USE_OBJ_AUX_STUBS
	    else if ( lp.current_stub != NULL )
		return lp.current =
			   min::unprotected::
			        gen_of
			          ( lp.current_stub );
#	endif
	else if ( lp.current == min::LIST_END )
	    return lp.current;

	MIN_ABORT ( "inconsistent list pointer" );
    }

    template < class vecpt >
    inline min::gen start_sublist
    	    ( min::unprotected
	         ::list_ptr_type<vecpt> & lp )
    {
	return start_sublist ( lp, lp );
    }

    // Set is declared as a friend only for
    // list_{upd,ins}ptrs.
    //
    template <>
    inline void update
    	    ( min::list_updptr & lp,
	      min::gen value )
    {
        MIN_ASSERT ( value != min::LIST_END );
        MIN_ASSERT ( lp.current != min::LIST_END );
        MIN_ASSERT ( ! is_sublist ( lp.current ) );
        MIN_ASSERT ( ! is_sublist ( value ) );
	unprotected::acc_write_update
	    ( unprotected::stub_of ( lp.vecp ), value );

#       if MIN_USE_OBJ_AUX_STUBS
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
	    MIN_ABORT ( "inconsistent list pointer" );
	}
    }
    template <>
    inline void update
    	    ( min::list_insptr & lp,
	      min::gen value )
    {
        MIN_ASSERT ( value != min::LIST_END );
        MIN_ASSERT ( lp.current != min::LIST_END );
        MIN_ASSERT ( value == min::EMPTY_SUBLIST
	             ||
		     ! is_sublist ( value ) );
	min::internal
	   ::remove_sublist
	       ( lp.base, lp.total_size, lp.current );
	unprotected::acc_write_update
	    ( unprotected::stub_of ( lp.vecp ), value );

#       if MIN_USE_OBJ_AUX_STUBS
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
	    MIN_ABORT ( "inconsistent list pointer" );
	}
    }

    inline bool insert_reserve
    	    ( min::list_insptr & lp,
	      min::unsptr insertions,
	      min::unsptr elements,
	      bool use_obj_aux_stubs )
    {
        if ( elements == 0 ) elements = insertions;
	MIN_ASSERT ( insertions <= elements );

	min::unsptr unused_size =
	      unprotected::aux_offset_of ( lp.vecp )
	    - unprotected::unused_offset_of ( lp.vecp );

	if (      unused_size
	        < 2 * insertions + elements
#	    if MIN_USE_OBJ_AUX_STUBS
	     && (    ! use_obj_aux_stubs
	          ||   min::internal
			  ::number_of_free_stubs
		     < insertions + elements )
#	    endif
	   )
	    return min::internal::insert_reserve
	        ( lp, insertions, elements,
		  use_obj_aux_stubs );
	else
	{
	    lp.reserved_insertions = insertions;
	    lp.reserved_elements = elements;
#	    if MIN_USE_OBJ_AUX_STUBS
		lp.use_obj_aux_stubs =
		   use_obj_aux_stubs;
#	    endif
	    return false;
	}
    }
}


// Object Attribute Level
// ------ --------- -----


namespace min { namespace unprotected {

    // This is the generic attribute pointer type from
    // which specific attribute pointer types are made.

    template < class vecpt >
        class attr_ptr_type;

} }

namespace min {

    // Public protected attribute pointer types.

    typedef min::unprotected::attr_ptr_type
	    < min::obj_vec_ptr >
        attr_ptr;
    typedef min::unprotected::attr_ptr_type
	    < min::obj_vec_updptr >
        attr_updptr;
    typedef min::unprotected::attr_ptr_type
	    < min::obj_vec_insptr >
        attr_insptr;


    // We must declare these before we make them
    // friends.

    template < class vecpt >
    void locate
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap,
	      min::gen name );
    template < class vecpt >
    void locatei
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap,
	      int name );
    template < class vecpt >
    void locatei
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap,
	      min::unsptr name );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	template < class vecpt >
	void locate
		( unprotected::attr_ptr_type
		      < vecpt > & ap,
		  min::unsptr & length, min::gen name );
#   endif
    template < class vecpt >
    void locate_reverse
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap,
	      min::gen reverse_name );
    template < class vecpt >
    void relocate
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap );
    template < class vecpt >
    min::unsptr get
	    ( min::gen * out, min::unsptr n,
	      unprotected::attr_ptr_type
	          < vecpt > & ap );
    template < class vecpt >
    min::gen get
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap );
    template < class vecpt >
    unsigned get_flags
	    ( min::gen * out, unsigned n,
	      unprotected::attr_ptr_type
	          < vecpt > & ap );
    template < class vecpt >
    bool test_flag
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap,
	      unsigned n );

    struct attr_info
    {
        min::gen    name;
	min::unsptr value_count;
	min::unsptr flag_count;
	min::unsptr reverse_attr_count;
    };
    struct reverse_attr_info
    {
        min::gen    name;
	min::unsptr value_count;
    };
    typedef min::packed_vec<attr_info>
	    attr_info_vec;
    typedef min::packed_vec<reverse_attr_info>
	    reverse_attr_info_vec;
    extern attr_info_vec attr_info_vec_type;
    extern reverse_attr_info_vec
           reverse_attr_info_vec_type;
    typedef attr_info_vec::ptr
        attr_info_ptr;
    typedef reverse_attr_info_vec::ptr
        reverse_attr_info_ptr;
    void sort_attr_info ( min::gen v );
    void sort_reverse_attr_info ( min::gen v );

    template < class vecpt >
    min::gen get_attrs
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap );
    template < class vecpt >
    min::gen get_reverse_attrs
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap );
    template < class vecpt >
    min::gen update
	    ( unprotected::attr_ptr_type
	          < vecpt > & ap,
	      min::gen v );
    void set
	    ( min::attr_insptr & ap,
	      const min::gen * in, min::unsptr n );
    void set
	    ( min::attr_insptr & ap,
	      min::gen v );
    void add_to_set
	    ( min::attr_insptr & ap,
	      const min::gen * in, min::unsptr n );
    void add_to_set
	    ( min::attr_insptr & ap,
	      min::gen v );
    void add_to_multiset
	    ( min::attr_insptr & ap,
	      const min::gen * in, min::unsptr n );
    void add_to_multiset
	    ( min::attr_insptr & ap,
	      min::gen v );
    min::unsptr remove_one
	    ( min::attr_insptr & ap,
	      const min::gen * in, min::unsptr n );
    min::unsptr remove_one
	    ( min::attr_insptr & ap,
	      min::gen v );
    min::unsptr remove_all
	    ( min::attr_insptr & ap,
	      const min::gen * in, min::unsptr n );
    min::unsptr remove_all
	    ( min::attr_insptr & ap,
	      min::gen v );
    void set_flags
	    ( min::attr_insptr & ap,
	      const min::gen * in, unsigned n );
    void set_some_flags
	    ( min::attr_insptr & ap,
	      const min::gen * in, unsigned n );
    void clear_some_flags
	    ( min::attr_insptr & ap,
	      const min::gen * in, unsigned n );
    void flip_some_flags
	    ( min::attr_insptr & ap,
	      const min::gen * in, unsigned n );
    bool set_flag
	    ( min::attr_insptr & ap,
	      unsigned n );
    bool clear_flag
	    ( min::attr_insptr & ap,
	      unsigned n );
    bool flip_flag
	    ( min::attr_insptr & ap,
	      unsigned n );

    namespace internal {

#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    template < class vecpt >
	    void locate
		    ( unprotected::attr_ptr_type
			  < vecpt > & ap,
		      min::gen name,
		      bool allow_partial_label = false
		    );
#	else // ! MIN_ALLOW_PARTIAL_ATTR_LABELS
	    template < class vecpt >
	    void locate
		    ( unprotected::attr_ptr_type
			  < vecpt > & ap,
		      min::gen name );
#	endif
	template < class vecpt >
	void relocate
		( unprotected::attr_ptr_type
		      < vecpt > & ap );
	template < class vecpt >
	min::unsptr get
		( min::gen * out, min::unsptr n,
		  unprotected::attr_ptr_type
		      < vecpt > & ap );
	template < class vecpt >
	min::gen get
		( unprotected::attr_ptr_type
		      < vecpt > & ap );
	template < class vecpt >
	min::unsptr get_flags
		( min::gen * out, min::unsptr n,
		  unprotected::attr_ptr_type
		      < vecpt > & ap );
	template < class vecpt >
	bool test_flag
		( unprotected::attr_ptr_type
		      < vecpt > & ap,
		  unsigned n );
	template < class vecpt >
	min::gen update
		( unprotected::attr_ptr_type
		      < vecpt > & ap,
		  min::gen v );
	void set
		( min::attr_insptr & ap,
		  const min::gen * in, min::unsptr n );
	void set_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	void set_more_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	void set_some_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	void clear_some_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	void flip_some_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	bool set_flag
		( min::attr_insptr & ap,
		  unsigned n );
	bool clear_flag
		( min::attr_insptr & ap,
		  unsigned n );
	bool flip_flag
		( min::attr_insptr & ap,
		  unsigned n );

	// Create an attribute that does not exist and
	// set its attribute-descriptor or node-descrip-
	// tor to v, which may be EMPTY_SUBLIST.
	// locate_dlp and dlp are set to point at the
	// descriptor, and the state is set to LOCATE_
	// NONE, LOCATE_ANY, or REVERSE_LOCATE_FAIL
	// according to the setting of ap.reverse_attr_
	// name.
	//
	void attr_create
		( min::attr_insptr & ap,
		  min::gen v );

	// Create a reverse attribute that does not
	// exist and set its value-multiset to v, which
	// may be EMPTY_SUBLIST.  The state is set to
	// REVERSE_LOCATE_SUCCEED and dlp is set to
	// point at the value-multiset.
	//
	void reverse_attr_create
		( min::attr_insptr & ap,
		  min::gen v );

	// Given a reverse attribute value v, remove it
	// from the object at the other end of the
	// double arrow.  Specifically, if O1 is the
	// object pointed at by v, and O2 is the object
	// pointed at by ap, then from O1 remove the
	// reverse attribute value pointing at O2 that
	// has the attribute name ap.reverse_attr_name
	// and the reverse attribute name ap.attr_name.
	// The removal is done by editing the value-
	// multiset of the designated reverse attribute
	// of O1, possibly making it empty.  In order to
	// do this, and insertable vector pointer to O1
	// is temporarily created, if O1 != O2.
	//
	// Handles corner cases where O1 == O2 and
	// ap.reverse_attr_name == ap.attr_name.
	// Does nothing if BOTH of these are true,
	// because in this case there is only one copy
	// of the min::gen value stored in the reverse
	// attribute set and no need to remove another
	// copy.
	//
	void remove_reverse_attr_value
		( min::attr_insptr & ap,
		  min::gen v );

	// Ditto but specify O1 by a vector pointer vp
	// instead of by a min::gen value v.  Does NOT
	// handle case where BOTH O1 == O2 AND
	// ap.reverse_attr_name == ap.attr_name.
	//
	void remove_reverse_attr_value
		( min::attr_insptr & ap,
		  min::obj_vec_insptr & vp );

	// Given a reverse attribute value v, add it
	// to the object at the other end of the
	// double arrow.  Specifically, if O1 is the
	// object pointed at by v, and O2 is the object
	// pointed at by ap, then to O1 add the
	// reverse attribute value pointing at O2 that
	// has the attribute name ap.reverse_attr_name
	// and the reverse attribute name ap.attr_name.
	// The addition is done by editing the value-
	// multiset of the designated reverse attribute
	// of O1, adding to it a min::gen value pointing
	// at O2.  In order to do this an an insertable
	// vector pointer to O1 is temporarily created,
	// if O1 != O2.
	//
	// Handles corner cases where O1 == O2 and
	// ap.reverse_attr_name == ap.attr_name.
	// Does nothing if BOTH of these are true, as
	// in this case only one copy of the min::gen
	// pointer is stored in the value-multiset, and
	// no second copy needs to be added.
	//
	void add_reverse_attr_value
		( min::attr_insptr & ap,
		  min::gen v );

	// Ditto but specify O1 by an object vector
	// pointer vp instead of by a min::gen value v.
	// Does NOT handle case where BOTH O1 == O2 AND
	// ap.reverse_attr_name == ap.attr_name.
	//
	void add_reverse_attr_value
		( min::attr_insptr & ap,
		  min::obj_vec_insptr & vp );
    }

}

namespace min { namespace unprotected {


    template < class vecpt >
    class attr_ptr_type {

    public:

        attr_ptr_type ( vecpt & vecp )
	    : dlp ( vecp ),
	      locate_dlp ( vecp ),
	      lp ( vecp ),
	      attr_name ( min::NONE ),
	      reverse_attr_name ( min::NONE ),
	      state ( INIT )
	{
	}

    private:

    // Private Data:

        min::gen attr_name;
	    // The attribute name given to the last
	    // locate function call.
        min::gen reverse_attr_name;
	    // The reverse attribute name given to the
	    // last reverse_locate function call;
	    // also reset to NONE by a locate function
	    // call.  Can be NONE or ANY.

#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    min::unsptr length;
		// If the state is not INIT or LOCATE_
		// FAIL, then if the last call to locate
		// had a length argument, this is the
		// value returned in that argument, and
		// otherwise this is the length of the
		// attr_name.  If the the state is
		// LOCATE_FAIL, this is the length of
		// the longest initial segment of attr_
		// name for which there is any node-
		// descriptor, with or without an empty
		// value-multiset.
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
		    // has been made since or the last
		    // subsequent call to reverse_
		    // locate set the reverse_attribute
		    // to NONE.

		LOCATE_ANY		= 3,
		    // Last call to locate succeeded,
		    // and the last subsequent call to
		    // reverse_locate set the reverse
		    // attribute to ANY.

		REVERSE_LOCATE_FAIL	= 4,
		    // Last call to locate succeeded,
		    // the last subsequent call to
		    // reverse_locate set the reverse
		    // attribute to a value other than
		    // NONE or ANY, and this call
		    // failed.

		REVERSE_LOCATE_SUCCEED	= 5
		    // Last call to locate succeeded,
		    // the last subsequent call to
		    // reverse_locate set the reverse
		    // attribute to a value other than
		    // NONE or ANY, and this call
		    // succeeded.

	    };

	unsigned flags;
	    // All flags are zeroed when the attribute
	    // pointer is created, and remain zeroed
	    // until set by a locate.  Set even if
	    // locate failed.

	    enum {

	        IN_VECTOR	= ( 1 << 0 ),
		    // On if index is attribute vector
		    // index; off it its hash table
		    // index or no locate yet.
	    };

	min::unsptr index;
	    // Hash or attribute vector index passed to
	    // the start_hash or start_vector functions
	    // by the last call to locate.  See the
	    // IN_VECTOR flag above.  Set even if
	    // locate failed.

    	list_ptr_type<vecpt> dlp;
	    // Descriptor list pointer.  Points at the
	    // list element containing the attribute- or
	    // node- descriptor found, if the state
	    // is LOCATE_NONE or REVERSE_LOCATE_SUCCEED.
	    // Otherwise is free for use as a temporary
	    // working pointer.

    	list_ptr_type<vecpt> locate_dlp;
	    // This is the value of dlp after the last
	    // successful locate, if state >= LOCATE_
	    // NONE.
	    //
	    // If partial attributes are allowed and the
	    // state is LOCATE_FAIL, this points at the
	    // node-descriptor associated with the
	    // longest initial segment of attr_name
	    // which has an associted node-descriptor.
	    // This permits create_attr to be optimized.

    	list_ptr_type<vecpt> lp;
	    // A working pointer for temporary use.

    // Friends:

	friend void locate<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap,
		  min::gen name );
	friend void locatei<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap,
		  int name );
	friend void locatei<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap,
		  min::unsptr name );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    friend void locate<>
		    ( min::unprotected
			 ::attr_ptr_type<vecpt>
			     & ap,
		      min::unsptr & length,
		      min::gen name );
#   endif
	friend void min::locate_reverse<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap,
		  min::gen reverse_name );
	friend void min::relocate<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend min::unsptr min::get<>
		( min::gen * out, min::unsptr n,
		  min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend min::gen min::get<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend unsigned min::get_flags<>
		( min::gen * out, unsigned n,
		  min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend bool min::test_flag<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap,
		  unsigned n );
	friend min::gen min::get_attrs<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend min::gen min::get_reverse_attrs<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend min::gen min::update<>
		( min::attr_updptr & ap,
		  min::gen v );
	friend min::gen min::update<>
		( min::attr_insptr & ap,
		  min::gen v );
	friend void min::set
		( min::attr_insptr & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::set
		( min::attr_insptr & ap,
		  min::gen v );
	friend void min::add_to_set
		( min::attr_insptr & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::add_to_set
		( min::attr_insptr & ap,
		  min::gen v );
	friend void min::add_to_multiset
		( min::attr_insptr & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::add_to_multiset
		( min::attr_insptr & ap,
		  min::gen v );
	friend min::unsptr min::remove_one
		( min::attr_insptr & ap,
		  const min::gen * in, min::unsptr n );
	friend min::unsptr min::remove_one
		( min::attr_insptr & ap,
		  min::gen v );
	friend min::unsptr min::remove_all
		( min::attr_insptr & ap,
		  const min::gen * in, min::unsptr n );
	friend min::unsptr min::remove_all
		( min::attr_insptr & ap,
		  min::gen v );
	friend void min::set_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	friend void min::set_some_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	friend void min::clear_some_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	friend void min::flip_some_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	friend bool min::set_flag
		( min::attr_insptr & ap,
		  unsigned n );
	friend bool min::clear_flag
		( min::attr_insptr & ap,
		  unsigned n );
	friend bool min::flip_flag
		( min::attr_insptr & ap,
		  unsigned n );

    #	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    friend void min::internal::locate<>
		    ( min::unprotected
			 ::attr_ptr_type<vecpt>
			     & ap,
		      min::gen name,
		      bool allow_partial_label
		    );
#	else // ! MIN_ALLOW_PARTIAL_ATTR_LABELS
	    friend void min::internal::locate<>
		    ( min::unprotected
			 ::attr_ptr_type<vecpt>
			     & ap,
		      min::gen name );
#	endif
	friend void min::internal::relocate<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend min::unsptr min::internal::get<>
		( min::gen * out, min::unsptr n,
		  min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend min::gen min::internal::get<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend min::unsptr min::internal::get_flags<>
		( min::gen * out, min::unsptr n,
		  min::unprotected
		     ::attr_ptr_type<vecpt> & ap );
	friend bool min::internal::test_flag<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap,
		  unsigned n );
	friend min::gen min::internal::update<>
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap,
		  min::gen v );
	friend void min::internal::set
		( min::attr_insptr & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::internal::set_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	friend void min::internal::set_more_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	friend void min::internal::set_some_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	friend void min::internal::clear_some_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	friend void min::internal::flip_some_flags
		( min::attr_insptr & ap,
		  const min::gen * in, unsigned n );
	friend bool min::internal::set_flag
		( min::attr_insptr & ap,
		  unsigned n );
	friend bool min::internal::clear_flag
		( min::attr_insptr & ap,
		  unsigned n );
	friend bool min::internal::flip_flag
		( min::attr_insptr & ap,
		  unsigned n );
	friend void min::internal::attr_create
		( min::attr_insptr & ap,
		  min::gen v );
	friend void min::internal::reverse_attr_create
		( min::attr_insptr & ap,
		  min::gen v );
	friend void min::internal
	               ::remove_reverse_attr_value
		( min::attr_insptr & ap,
		  min::gen v );
	friend void min::internal
	               ::remove_reverse_attr_value
		( min::attr_insptr & ap,
		  min::obj_vec_insptr & vp );
	friend void min::internal
	               ::add_reverse_attr_value
		( min::attr_insptr & ap,
		  min::gen v );
	friend void min::internal
	               ::add_reverse_attr_value
		( min::attr_insptr & ap,
		  min::obj_vec_insptr & vp );
    };

} }

namespace min {

    // Inline functions.  See MIN design document.

    // Locate Functions:

    template < class vecpt >
    inline void locatei
	    ( min::unprotected
	         ::attr_ptr_type<vecpt> & ap,
	      int name )
    {
	typedef unprotected::attr_ptr_type<vecpt>
	    ap_type;

	ap.attr_name = new_num_gen ( name );

	if ( 0 <= name
	     &&
	     name < attr_size_of
	                ( obj_vec_ptr_of
			    ( ap.locate_dlp ) ) )
	{
	    ap.index = name;
	    ap.flags = ap_type::IN_VECTOR;
	    ap.reverse_attr_name = min::NONE;

	    start_vector ( ap.locate_dlp, name );
	    min::gen c = current ( ap.locate_dlp );
	    if ( is_list_end ( c ) )
	    {
		ap.state = ap_type::LOCATE_FAIL;
#	        if MIN_ALLOW_PARTIAL_ATTR_LABELS
		    ap.length = 0;
#	        endif
	    }
	    else
	    {
		start_copy ( ap.dlp, ap.locate_dlp );
		ap.state = ap_type::LOCATE_NONE;
#	        if MIN_ALLOW_PARTIAL_ATTR_LABELS
		    ap.length = 1;
#	        endif
	    }

	    return;
	}
	
	internal::locate ( ap, ap.attr_name );
    }

    template < class vecpt >
    inline void locatei
	    ( min::unprotected
	         ::attr_ptr_type<vecpt> & ap,
	      min::unsptr name )
    {
	typedef unprotected::attr_ptr_type<vecpt>
	    ap_type;

	ap.attr_name = new_num_gen ( name );

	if ( name < attr_size_of
	                ( obj_vec_ptr_of
			    ( ap.locate_dlp ) ) )
	{
	    ap.index = name;
	    ap.flags = ap_type::IN_VECTOR;
	    ap.reverse_attr_name = min::NONE;

	    start_vector ( ap.locate_dlp, name );
	    min::gen c = current ( ap.locate_dlp );
	    if ( is_list_end ( c ) )
	    {
		ap.state = ap_type::LOCATE_FAIL;
#	        if MIN_ALLOW_PARTIAL_ATTR_LABELS
		    ap.length = 0;
#	        endif
	    }
	    else
	    {
		start_copy ( ap.dlp, ap.locate_dlp );
		ap.state = ap_type::LOCATE_NONE;
#	        if MIN_ALLOW_PARTIAL_ATTR_LABELS
		    ap.length = 1;
#	        endif
	    }

	    return;
	}
	
	internal::locate ( ap, ap.attr_name );
    }

    template < class vecpt >
    inline void locate
	    ( min::unprotected
	         ::attr_ptr_type<vecpt> & ap,
	      min::gen name )
    {
	typedef unprotected::attr_ptr_type<vecpt>
	    ap_type;

	// We only handle the case of vector elements
	// inline.
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
			( obj_vec_ptr_of
			    ( ap.locate_dlp ) ) )
	    {
		ap.attr_name = name;
		ap.index = i;
		ap.flags = ap_type::IN_VECTOR;
		ap.reverse_attr_name = min::NONE;

		start_vector ( ap.locate_dlp, i );
		min::gen c = current ( ap.locate_dlp );
		if ( is_list_end ( c ) )
		{
		    ap.state = ap_type::LOCATE_FAIL;
    #	        if MIN_ALLOW_PARTIAL_ATTR_LABELS
			ap.length = 0;
    #	        endif
		}
		else
		{
		    start_copy
		        ( ap.dlp, ap.locate_dlp );
		    ap.state = ap_type::LOCATE_NONE;
    #	        if MIN_ALLOW_PARTIAL_ATTR_LABELS
			ap.length = 1;
    #	        endif
		}

		return;
	    }
	}

	internal::locate ( ap, name );
    }

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	template < class vecpt >
	inline void locate
		( min::unprotected
		     ::attr_ptr_type<vecpt> & ap,
		  min::unsptr & length,
		  min::gen name )
	{
	    typedef min::unprotected
		       ::attr_ptr_type<vecpt>
		         ap_type;

	    // We only handle the case of vector
	    // elements inline.
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
			    ( obj_vec_ptr_of
				  ( ap.locate_dlp ) ) )
		{
		    ap.attr_name = name;
		    ap.index = i;
		    ap.flags = ap_type::IN_VECTOR;
		    ap.reverse_attr_name = min::NONE;

		    start_vector ( ap.locate_dlp, i );
		    min::gen c =
		        current ( ap.locate_dlp );
		    if ( is_list_end ( c ) )
		    {
			ap.state = ap_type::LOCATE_FAIL;
			ap.length = 0;
		    }
		    else
		    {
			start_copy
			    ( ap.dlp, ap.locate_dlp );
			ap.state = ap_type::LOCATE_NONE;
			ap.length = 1;
		    }

		    return;
		}
	    }

	    internal::locate ( ap, name, true );
	    if ( ap.state == ap_type::LOCATE_NONE )
		length = ap.length;
	    else
	        length = 0;
	}
#   endif

    // Access Functions:

    // We begin each function with an update_refresh as
    // this permits us to avoid calls to relocate when
    // only updates are performed, and calls to update_
    // refresh are very cheap.

    template < class vecpt >
    inline min::unsptr get
	    ( min::gen * out, min::unsptr n,
	      min::unprotected
	         ::attr_ptr_type<vecpt> & ap )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecpt> ap_type;

	if ( n == 0 ) return 0;

	switch ( ap.state )
	{
	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
	    return 0;
	case ap_type::INIT:
	case ap_type::LOCATE_ANY:
	    return internal::get ( out, n, ap );
	}

	min::gen c = update_refresh ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    out[0] = c;
	    return 1;
	}
	start_sublist ( ap.lp, ap.dlp );
	unsigned result = 0;
	for ( c = current ( ap.lp );
	      ! is_list_end ( c )
	      &&
	      ! is_sublist ( c )
	      &&
	      ! is_control_code ( c );
	      c = next ( ap.lp ) )
	{
	    if ( result < n ) * out ++ = c;
	    ++ result;
	}
	return result;
    }

    template < class vecpt >
    inline min::gen get
	    ( min::unprotected
	         ::attr_ptr_type<vecpt> & ap )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecpt> ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_ANY:
	    return internal::get ( ap );

	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
	    return min::NONE;
	}

	min::gen c = update_refresh ( ap.dlp );
	if ( ! is_sublist ( c ) )
	    return c;
	start_sublist ( ap.lp, ap.dlp );
	c = current ( ap.lp );
	if ( is_sublist ( c )
	     ||
	     is_control_code ( c ) )
	    return min::NONE;
	min::gen d = next ( ap.lp );
	if ( is_list_end ( d )
	     ||
	     is_sublist ( d )
	     ||
	     is_control_code ( d ) )
	    return c;
	else
	    return min::MULTI_VALUED;
    }

    template < class vecpt >
    inline unsigned get_flags
	    ( min::gen * out, unsigned n,
	      min::unprotected
	         ::attr_ptr_type<vecpt> & ap )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecpt> ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	    return internal::get_flags ( out, n, ap );

	case ap_type::LOCATE_FAIL:
	    return 0;
	}

	min::gen c =  update_refresh ( ap.locate_dlp );
	if ( ! is_sublist ( c ) ) return 0;
	start_sublist ( ap.lp, ap.locate_dlp );
	unsigned result = 0;
	unsigned count = 0;
	const min::gen zero_cc =
	    min::new_control_code_gen ( 0 );
	for ( c = current ( ap.lp );
	      ! is_list_end ( c );
	      c = next ( ap.lp ) )
	{
	    if ( is_control_code ( c ) )
	    {
	    	++ count;
		if ( c != zero_cc )
		{
		    while ( ++ result != count
		            &&
			    result <= n )
			* out ++ = zero_cc;
		    result = count;
		    if ( result <= n )
			* out ++ = c;
		}
	    }
	}
	return result;
    }

    template < class vecpt >
    inline bool test_flag
	    ( min::unprotected
	         ::attr_ptr_type<vecpt> & ap,
	      unsigned n )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecpt> ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	    return internal::test_flag ( ap, n );

	case ap_type::LOCATE_FAIL:
	    return false;
	}

	min::gen c =  update_refresh ( ap.locate_dlp );
	if ( ! is_sublist ( c ) ) return false;
	start_sublist ( ap.lp, ap.locate_dlp );
	unsigned base = 0;
	for ( c = current ( ap.lp );
	      ! is_list_end ( c );
	      c = next ( ap.lp ) )
	{
	    if ( is_control_code ( c ) )
	    {
	        unsigned next = base + VSIZE;
		if ( n < next )
		    return ( ( 1 << ( n - base ) )
		             &
			     control_code_of ( c ) )
			   != 0;
		base = next;
	    }
	}
	return false;
    }

    template < class vecpt >
    inline min::gen update
	    ( min::unprotected
	         ::attr_ptr_type<vecpt> & ap,
	      min::gen v )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecpt> ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::LOCATE_ANY:
	    return internal::update ( ap, v );
	}

	min::gen c = update_refresh ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    update ( ap.dlp, v );
	    return c;
	}
	start_sublist ( ap.lp, ap.dlp );
	c = current ( ap.lp );
	if ( ! is_list_end ( c )
	     &&
	     ! is_sublist ( c )
	     &&
	     ! is_control_code ( c ) )
	{
	    min::gen d = peek ( ap.lp );
	    if ( is_list_end ( d )
		 ||
		 is_sublist ( d )
		 ||
		 is_control_code ( d ) )
	    {
		update ( ap.lp, v );
		return c;
	    }
	}
	return internal::update ( ap, v );
    }

    inline void set
	    ( min::attr_insptr & ap,
	      const min::gen * in, min::unsptr n )
    {
	typedef min::attr_insptr ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_FAIL:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	    internal::set ( ap, in, n );
	    return;
	}
	if ( n != 1 )
	{
	    internal::set ( ap, in, n );
	    return;
	}

	min::gen c = update_refresh ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    update ( ap.dlp, * in );
	    return;
	}
	start_sublist ( ap.lp, ap.dlp );
	c = current ( ap.lp );
	if ( ! is_list_end ( c )
	     &&
	     ! is_sublist ( c )
	     &&
	     ! is_control_code ( c ) )
	{
	    min::gen d = peek ( ap.lp );
	    if ( is_list_end ( d )
		 ||
		 is_sublist ( d )
		 ||
		 is_control_code ( d ) )
	    {
		update ( ap.lp, * in );
		return;
	    }
	}
	internal::set ( ap, in, n );
    }

    inline void set
	    ( min::attr_insptr & ap,
	      min::gen v )
    {
	typedef min::attr_insptr ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_FAIL:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	    internal::set ( ap, &v, 1 );
	    return;
	}

	min::gen c = update_refresh ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    update ( ap.dlp, v );
	    return;
	}
	start_sublist ( ap.lp, ap.dlp );
	c = current ( ap.lp );
	if ( ! is_list_end ( c )
	     &&
	     ! is_sublist ( c )
	     &&
	     ! is_control_code ( c ) )
	{
	    min::gen d = peek ( ap.lp );
	    if ( is_list_end ( d )
		 ||
		 is_sublist ( d )
		 ||
		 is_control_code ( d ) )
	    {
		update ( ap.lp, v );
		return;
	    }
	}
	internal::set ( ap, & v, 1 );
    }

    inline void add_to_set
	    ( min::attr_insptr & ap,
	      min::gen v )
    {
	typedef min::attr_insptr ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_ANY:
	    add_to_set ( ap, & v, 1 );

	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
	    add_to_multiset ( ap, & v, 1 );
	    return;
	}

	min::gen c = update_refresh ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    if ( c == v ) return;
	}
	else
	{
	    start_sublist ( ap.lp, ap.dlp );
	    c = current ( ap.lp );
	    for ( c = current ( ap.lp );
		     ! is_list_end ( c )
		  && ! is_sublist ( c )
		  && ! is_control_code ( c );
		  c = next ( ap.lp ) )
	    {
		if ( c == v ) return;
	    }
	}
	add_to_multiset ( ap, & v, 1 );
    }

    inline void add_to_multiset
	    ( min::attr_insptr & ap,
	      min::gen v )
    {
	add_to_multiset ( ap, & v, 1 );
    }

    inline min::unsptr remove_one
	    ( min::attr_insptr & ap,
	      min::gen v )
    {
	if ( v == min::NONE ) return 0;

	typedef min::attr_insptr ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	    return remove_one ( ap, & v, 1 );

	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
	    return 0;
	}

	// We do NOT do double arrow removals here.

	min::gen c = update_refresh ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    if ( c != v ) return 0;
	    update ( ap.dlp, min::EMPTY_SUBLIST );
	    return 1;
	}
	start_sublist ( ap.lp, ap.dlp );
	for ( c = current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c );
	      c = next ( ap.lp ) )
	{
	    if ( c == v )
	    {
	        remove ( ap.lp, 1 );
		return 1;
	    }
	}
	return 0;
    }

    inline min::unsptr remove_all
	    ( min::attr_insptr & ap,
	      min::gen v )
    {
	if ( v == min::NONE ) return 0;

	typedef min::attr_insptr ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	    return remove_all ( ap, & v, 1 );

	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
	    return 0;
	}

	// We do NOT do double arrow removals here.

	min::gen c = update_refresh ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    if ( c != v ) return 0;
	    update ( ap.dlp, min::EMPTY_SUBLIST );
	    return 1;
	}
	start_sublist ( ap.lp, ap.dlp );
	min::unsptr result = 0;
	for ( c = current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c );
	    )
	{
	    if ( c == v )
	    {
	        remove ( ap.lp, 1 );
		++ result;
	        c = current ( ap.lp );
	    }
	    else
	        c = next ( ap.lp );
	}
	return result;
    }

    inline void set_flags
	    ( min::attr_insptr & ap,
	      const min::gen * in, unsigned n )
    {
	typedef attr_insptr ap_type;

	min::gen c = update_refresh ( ap.locate_dlp );
	switch ( ap.state )
	{
	case ap_type::LOCATE_NONE:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::REVERSE_LOCATE_FAIL:
	    if ( is_sublist ( c ) )
	    {
		start_sublist
		    ( ap.lp, ap.locate_dlp );
		const min::gen zero_cc =
		     new_control_code_gen ( 0 );
		for ( c = current ( ap.lp );
		      ! is_list_end ( c );
		      c = next ( ap.lp ) )
		{
		    if ( is_control_code ( c ) )
		    {
		        if ( n > 0 ) 
			{
			    MIN_ASSERT
				( is_control_code
				      ( * in ) );
			    update ( ap.lp, * in ++ );
			    -- n;
			}
			else
			    update ( ap.lp, zero_cc );
		    }
		}
		if ( n > 0 )
		    internal::set_more_flags
			( ap, in, n );
		return;
	    }
	}

	internal::set_flags ( ap, in, n );
    }

    inline void set_some_flags
	    ( min::attr_insptr & ap,
	      const min::gen * in, unsigned n )
    {
	typedef attr_insptr ap_type;

	min::gen c = update_refresh ( ap.locate_dlp );
	switch ( ap.state )
	{
	case ap_type::LOCATE_NONE:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::REVERSE_LOCATE_FAIL:
	    if ( is_sublist ( c ) )
	    {
		start_sublist
		    ( ap.lp, ap.locate_dlp );
		for ( c = current ( ap.lp );
		      n > 0 && ! is_list_end ( c );
		      c = next ( ap.lp ) )
		{
		    if ( is_control_code ( c ) )
		    {
			MIN_ASSERT
			    ( is_control_code
				  ( * in ) );
			min::unsgen uc =
			    control_code_of ( c )
			    |
			    control_code_of ( * in ++ );
			update ( ap.lp,
			         new_control_code_gen
				     ( uc ) );
			-- n;
		    }
		}
		if ( n > 0 )
		    internal::set_more_flags
			( ap, in, n );
	    }
	    else
		internal::set_flags ( ap, in, n );

	    return;
	}

	internal::set_some_flags ( ap, in, n );
    }

    inline void clear_some_flags
	    ( min::attr_insptr & ap,
	      const min::gen * in, unsigned n )
    {
	typedef attr_insptr ap_type;

	min::gen c = update_refresh ( ap.locate_dlp );
	switch ( ap.state )
	{
	case ap_type::LOCATE_NONE:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::REVERSE_LOCATE_FAIL:
	    if ( is_sublist ( c ) )
	    {
		start_sublist
		    ( ap.lp, ap.locate_dlp );
		for ( c = current ( ap.lp );
		      n > 0 && ! is_list_end ( c );
		      c = next ( ap.lp ) )
		{
		    if ( is_control_code ( c ) )
		    {
			MIN_ASSERT
			    ( is_control_code
				  ( * in ) );
			min::unsgen uc =
			    control_code_of ( c )
			    & ~
			    control_code_of
				( * in  ++ );
			update
			    ( ap.lp,
			      new_control_code_gen
				  ( uc ) );
			-- n;
		    }
		}
	    }

	    return;
	}

	internal::clear_some_flags ( ap, in, n );
    }

    inline void flip_some_flags
	    ( min::attr_insptr & ap,
	      const min::gen * in, unsigned n )
    {
	typedef attr_insptr ap_type;

	min::gen c = update_refresh ( ap.locate_dlp );
	switch ( ap.state )
	{
	case ap_type::LOCATE_NONE:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::REVERSE_LOCATE_FAIL:
	    if ( is_sublist ( c ) )
	    {
		start_sublist
		    ( ap.lp, ap.locate_dlp );
		for ( c = current ( ap.lp );
		      n > 0 && ! is_list_end ( c );
		      c = next ( ap.lp ) )
		{
		    if ( is_control_code ( c ) )
		    {
			MIN_ASSERT
			    ( is_control_code
				  ( * in ) );
			min::unsgen uc =
			    control_code_of ( c )
			    ^
			    control_code_of
				( * in  ++ );
			update
			    ( ap.lp,
			      new_control_code_gen
				  ( uc ) );
			-- n;
		    }
		}
		if ( n > 0 )
		    internal::set_more_flags
			( ap, in, n );
	    }
	    else
		internal::set_flags ( ap, in, n );

	    return;
	}
	internal::flip_some_flags ( ap, in, n );
    }

    inline bool set_flag
	    ( min::attr_insptr & ap,
	      unsigned n )
    {
	typedef min::attr_insptr ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_FAIL:
	    return internal::set_flag ( ap, n );
	}

	min::gen c =  update_refresh ( ap.locate_dlp );
	if ( ! is_sublist ( c ) )
	    return internal::set_flag ( ap, n );
	start_sublist ( ap.lp, ap.locate_dlp );
	unsigned base = 0;
	for ( c = current ( ap.lp );
	      ! is_list_end ( c );
	      c = next ( ap.lp ) )
	{
	    if ( is_control_code ( c ) )
	    {
	        unsigned next = base + VSIZE;
		if ( n < next )
		{
		    min::unsgen mask =
		        1 << ( n - base );
		    bool result =
		        ( mask & control_code_of ( c ) )
			!= 0;
		    c = (min::gen)
		        ( (min::unsgen) c | mask );
		    update ( ap.lp, c );
		    return result;
		}
		base = next;
	    }
	}
	return internal::set_flag ( ap, n - base );
    }

    inline bool clear_flag
	    ( min::attr_insptr & ap,
	      unsigned n )
    {
	typedef min::attr_insptr ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	    return internal::clear_flag ( ap, n );
	case ap_type::LOCATE_FAIL:
	    return false;
	}

	min::gen c =  update_refresh ( ap.locate_dlp );
	if ( ! is_sublist ( c ) )
	    return false;
	start_sublist ( ap.lp, ap.locate_dlp );
	unsigned base = 0;
	for ( c = current ( ap.lp );
	      ! is_list_end ( c );
	      c = next ( ap.lp ) )
	{
	    if ( is_control_code ( c ) )
	    {
	        unsigned next = base + VSIZE;
		if ( n < next )
		{
		    min::unsgen mask =
		        1 << ( n - base );
		    bool result =
		        ( mask & control_code_of ( c ) )
			!= 0;
		    c = (min::gen)
		        ( (min::unsgen) c & ~ mask );
		    update ( ap.lp, c );
		    return result;
		}
		base = next;
	    }
	}
	return false;
    }

    inline bool flip_flag
	    ( min::attr_insptr & ap,
	      unsigned n )
    {
	typedef min::attr_insptr ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	    return internal::flip_flag ( ap, n );
	case ap_type::LOCATE_FAIL:
	    return internal::set_flag ( ap, n );
	}

	min::gen c =  update_refresh ( ap.locate_dlp );
	if ( ! is_sublist ( c ) )
	    return internal::set_flag ( ap, n );
	start_sublist ( ap.lp, ap.locate_dlp );
	unsigned base = 0;
	for ( c = current ( ap.lp );
	      ! is_list_end ( c );
	      c = next ( ap.lp ) )
	{
	    if ( is_control_code ( c ) )
	    {
	        unsigned next = base + VSIZE;
		if ( n < next )
		{
		    min::unsgen mask =
		        1 << ( n - base );
		    bool result =
		        ( mask & control_code_of ( c ) )
			!= 0;
		    c = (min::gen)
		        ( (min::unsgen) c ^ mask );
		    update ( ap.lp, c );
		    return result;
		}
		base = next;
	    }
	}
	return internal::set_flag ( ap, n - base );
    }

}

// Printers
// --------

namespace min {

    struct printer_header;
    typedef packed_vec_insptr<char,printer_header>
        printer;

    struct printer_format
    {
        const char * number_format;
	const char * str_prefix;
	const char * str_postfix;
	const char * lab_prefix;
	const char * lab_separator;
	const char * lab_postfix;
	const char * special_prefix;
	const char * special_postfix;
	min::printer & ( * pr_stub )
	    ( min::printer & out, const min::stub * s );
    };

    extern printer_format default_printer_format;

    struct printer_parameters
    {
        printer_format * format;
	uns32 line_length;
	uns32 indent;
	uns32 flags;
    };

    extern printer_parameters default_printer_parameters;

    struct printer_header
    {
        uns32 control;

	printer_parameters parameters;
	printer_parameters saved_parameters;
	std::ostream * ostream;

	uns32 length;
	uns32 max_length;

	uns32 line;
	uns32 column;
	uns32 line_offset;
	uns32 break_offset;
    };

    enum {
        ASCII_FLAG		= ( 1 << 0 ),
        GRAPHIC_FLAG		= ( 1 << 1 ),
        DISPLAY_EOL_FLAG	= ( 1 << 2 ),
        HEX_FLAG		= ( 1 << 3 ),
        NOBREAKS_FLAG		= ( 1 << 4 ),
        EOL_FLUSH_FLAG		= ( 1 << 5 ),
        EOM_FLUSH_FLAG		= ( 1 << 6 ),
        KEEP_FLAG		= ( 1 << 7 ),
    };

    struct printer_item
    {
        printer & ( * op )
	    ( printer & prtr,
	      const printer_item & item );
	union {
	    void * p;
	    uns32 u;
	    min::gen g;
	} v1, v2;
    };

    namespace printer_internal
    {
        printer & pgen
	    ( printer & prtr,
	      const printer_item & item );
        printer & punicode1
	    ( printer & prtr,
	      const printer_item & item );
        printer & punicode2
	    ( printer & prtr,
	      const printer_item & item );
        printer & flush0
	    ( printer & prtr,
	      const printer_item & item );
        printer & flush1
	    ( printer & prtr,
	      const printer_item & item );
        printer & format
	    ( printer & prtr,
	      const printer_item & item );
        printer & line_length
	    ( printer & prtr,
	      const printer_item & item );
        printer & indent
	    ( printer & prtr,
	      const printer_item & item );
        printer & set_printer_flags
	    ( printer & prtr,
	      const printer_item & item );
        printer & clear_printer_flags
	    ( printer & prtr,
	      const printer_item & item );
    }

    inline printer_item pgen
	    ( min::gen v,
              printer_format * format = NULL )
    {
        printer_item item;
	item.op = printer_internal::pgen;
	item.v1.g = v;
	item.v2.p = format;
	return item;
    }

    inline printer_item punicode ( min::uns32 c )
    {
        printer_item item;
	item.op = printer_internal::punicode1;
	item.v1.u = c;
	return item;
    }

    inline printer_item punicode
	    ( min::unsptr length,
	      const min::uns32 * buffer )
    {
        printer_item item;
	item.op = printer_internal::punicode2;
	item.v1.u = length;
	item.v1.p = (void *) buffer;
	return item;
    }

    inline printer_item flush ( void )
    {
        printer_item item;
	item.op = printer_internal::flush0;
	return item;
    }

    inline printer_item flush
	    ( std::ostream & ostream )
    {
        printer_item item;
	item.op = printer_internal::flush1;
	item.v1.p = & ostream;
	return item;
    }

    inline printer_item format
	    ( printer_format * format )
    {
        printer_item item;
	item.op = printer_internal::format;
	item.v1.p = format;
	return item;
    }

    inline printer_item line_length
	    ( uns32 line_length )
    {
        printer_item item;
	item.op = printer_internal::line_length;
	item.v1.u = line_length;
	return item;
    }

    inline printer_item indent
	    ( uns32 indent )
    {
        printer_item item;
	item.op = printer_internal::indent;
	item.v1.u = indent;
	return item;
    }

    inline printer_item set_printer_flags
	    ( uns32 flags )
    {
        printer_item item;
	item.op = printer_internal::set_printer_flags;
	item.v1.u = flags;
	return item;
    }

    inline printer_item clear_printer_flags
	    ( uns32 flags )
    {
        printer_item item;
	item.op = printer_internal::clear_printer_flags;
	item.v1.u = flags;
	return item;
    }

    extern printer_item save_printer_parameters;
    extern printer_item restore_printer_parameters;

    extern printer_item eol;
    extern printer_item eom;

    extern printer_item hexadecimal;
    extern printer_item nohexadecimal;
    extern printer_item ascii;
    extern printer_item utf8;
    extern printer_item graphic;
    extern printer_item nographic;
    extern printer_item display_eol;
    extern printer_item nodisplay_eol;
    extern printer_item nobreaks;
    extern printer_item breaks;
    extern printer_item eol_flush;
    extern printer_item noeol_flush;
    extern printer_item eom_flush;
    extern printer_item noeom_flush;
    extern printer_item keep;
    extern printer_item nokeep;
}

inline min::printer & operator <<
	( min::printer & prtr,
	  const min::printer_item & item )
{
    return (* item.op ) ( prtr, item );
}

min::printer & operator <<
	( min::printer & prtr,
	  const char * s );

inline min::printer & operator <<
	( min::printer & prtr,
	  char c )
{
    char buffer[2] = { c, 0 };
    return prtr << buffer;
}

min::printer & operator <<
	( min::printer & prtr,
	  min::int64 i );

inline min::printer & operator <<
	( min::printer & prtr,
	  min::int32 i )
{
    return prtr << (min::int64) i;
}

min::printer & operator <<
	( min::printer & prtr,
	  min::uns64 i );

inline min::printer & operator <<
	( min::printer & prtr,
	  min::uns32 i )
{
    return prtr << (min::uns64) i;
}

min::printer & operator <<
	( min::printer & prtr1,
	  min::printer & prtr2 );

std::ostream & operator <<
	( std::ostream & out,
	  min::printer & prtr );


// Printing
// --------

namespace min {

    typedef packed_vec_insptr<char> charbuf;

    struct pr_format
    {
        const char * number_format;
	const char * str_prefix;
	const char * str_postfix;
	const char * lab_prefix;
	const char * lab_separator;
	const char * lab_postfix;
	const char * special_prefix;
	const char * special_postfix;
	std::ostream & ( * ostream_pr_stub )
	    ( std::ostream & out, const min::stub * s );
	min::charbuf & ( * charbuf_pr_stub )
	    ( min::charbuf & out, const min::stub * s );
    };

    extern pr_format default_pr_format;

    struct pr
    {
	pr ( min::gen value,
	     min::pr_format & format =
	         min::default_pr_format ) :
	    value ( value ), format ( format ) {}

        min::gen value;
	min::pr_format & format;
    };

    void init ( charbuf & buf,
                const char * initial_value = NULL );
}

std::ostream & operator <<
	( std::ostream & out, const min::pr & prv );

min::charbuf & operator <<
	( min::charbuf & buf, const min::pr & prv );

inline min::charbuf & operator <<
	( min::charbuf & buf, const char * s )
{
    int length = strlen ( s );
    min::pop ( buf );
    min::push ( buf, length + 1, s );
    return buf;
}

inline min::charbuf & operator <<
	( min::charbuf & buf, char c )
{
    buf[buf->length-1] = c;
    min::push(buf) = 0;
    return buf;
}

inline min::charbuf & operator <<
	( min::charbuf & buf, min::int64 i )
{
    char buffer[64];
    sprintf ( buffer, "%lld", i );
    return buf << buffer;
}

inline min::charbuf & operator <<
	( min::charbuf & buf, min::uns64 u )
{
    char buffer[64];
    sprintf ( buffer, "%llu", u );
    return buf << buffer;
}

inline min::charbuf & operator <<
	( min::charbuf & buf, min::int32 i )
{
    return buf << (min::int64) i;
}

inline min::charbuf & operator <<
	( min::charbuf & buf, min::uns32 u )
{
    return buf << (min::uns64) u;
}

inline std::ostream & operator <<
	( std::ostream & out, const min::charbuf & buf )
{
    return out << & buf[0];
}

// More Allocator/Collector/Compactor Interface
// ---- ----------------------------- ---------

#ifndef MIN_NON_STANDARD_UNPROTECTED_BODY_SIZE_OF

// Min::unprotected::body_size_of function for standard
// min types.  There are macros that permit adding
// additional stub type cases or completely redefining
// the function.
//
inline min::unsptr min::unprotected::body_size_of
	( const min::stub * s )
{
    switch ( min::type_of ( s ) )
    {
    case min::LONG_STR:
	return   unprotected::length_of
		     ( unprotected::long_str_of ( s ) )
	       + 1
	       + sizeof ( long_str );
    case min::LABEL:
	return   internal::lab_header_of ( s )->length
	       * sizeof ( min::gen )
	       + sizeof ( internal::lab_header );
    case min::SHORT_OBJ:
	return   internal::short_obj_total_size_of_flags
	             ( internal::short_obj_of ( s )
			-> flags )
	       * sizeof ( min::gen );
    case min::LONG_OBJ:
	return   internal::long_obj_total_size_of_flags
	             ( internal::long_obj_of ( s )
			-> flags )
	       * sizeof ( min::gen );
    case min::PACKED_STRUCT:
        {
	    min::uns32 control =
	        * ( min::uns32 *)
		unprotected::ptr_of ( s );
	    min::uns32 subtype =
		  control
		& internal::PACKED_CONTROL_SUBTYPE_MASK;
	    internal::packed_struct_descriptor * d =
	        (internal::packed_struct_descriptor *)
	        (*internal::packed_subtypes)[subtype];
	    return d->size;
	}
    case min::PACKED_VEC:
        {
	    min::uns8 * p = (min::uns8 *)
		unprotected::ptr_of ( s );
	    min::uns32 control = * ( min::uns32 *) p;
	    min::uns32 subtype =
		  control
		& internal::PACKED_CONTROL_SUBTYPE_MASK;
	    internal::packed_vec_descriptor * d =
	        (internal::packed_vec_descriptor *)
	        (*internal::packed_subtypes)[subtype];
	    min::uns32 max_length = * (min::uns32 *)
	        ( p + d->max_length_disp );
	    return   d->header_size
	           + d->element_size * max_length;
	}
#   ifdef MIN_UNPROTECTED_BODY_SIZE_OF_EXTRA_CASES
    MIN_UNPROTECTED_BODY_SIZE_OF_EXTRA_CASES
#   endif
    default:
        return 0;
    }
}
#else // def MIN_NON_STANDARD_UNPROTECTED_BODY_SIZE_OF
    // Define the following as empty to put function
    // out of line.
    MIN_NON_STANDARD_UNPROTECTED_BODY_SIZE_OF
#endif // ndef MIN_NON_STANDARD_UNPROTECTED_BODY_SIZE_OF

namespace min {

    inline void deallocate ( const min::stub * s )
    {
    	min::unprotected::deallocate_body
	    ( (min::stub *) s,
	      min::unprotected::body_size_of ( s ) );
    }
}

namespace min { namespace internal {

    // Remove a stub s from an aux hash table list.
    // Here head = xxx_aux_hash + index, where index =
    // s's hash value & xxx_hash_mask, is the head of
    // the list.  If stub is not in the list, MIN_ABORT.
    //
    inline void remove_aux_hash
	    ( min::stub ** head, min::stub * s )
    {
	min::gen g = min::new_gen ( s );
        min::stub * aux_s = * head;
	min::stub * last_aux_s = NULL;
	min::uns64 last_c;
	while ( aux_s != null_stub )
	{
	    min::uns64 c =
	        unprotected::control_of ( aux_s );
	    if ( unprotected::gen_of ( aux_s ) == g )
	    {
	        unprotected::free_aux_stub ( aux_s );
		if ( last_aux_s != NULL )
		{
		    last_c = unprotected
				 ::renew_control_stub
			( last_c,
			  unprotected
			    ::stub_of_control ( c ) );
		    unprotected::set_control_of
		        ( last_aux_s, last_c );
		}
		else
		    * head = unprotected
		               ::stub_of_control ( c );
		return;
	    }
	    last_c = c;
	    last_aux_s = aux_s;
	    aux_s = unprotected::stub_of_control ( c );
	}
        MIN_ABORT ( "remove_aux_hash failed" );
    }

} }


# endif // MIN_H
