// MIN Language Interface
//
// File:	min.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Feb 16 09:33:03 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/02/16 14:34:07 $
//   $RCSfile: min.h,v $
//   $Revision: 1.266 $

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
//	Name Functions
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level
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
# include <climits>
# include <cstring>
# include <cassert>
# include <new>

#define MIN_ABORT(string) assert ( ! string )
#define MIN_REQUIRE(expr) assert ( expr )

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

#   if MIN_POINTER_BITS <= 32
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
}

// Stub Types and Data
// ---- ----- --- ----

namespace min {

    // Stub type codes.

    // Collectable.
    //
    const int ACC_FREE			= 1;
    const int DEALLOCATED		= 2;
    const int NUMBER			= 3;
    const int SHORT_STR			= 4;
    const int LONG_STR			= 5;
    const int LABEL			= 6;
    const int TINY_OBJ			= 7;
        // Unimplemented.
    const int SHORT_OBJ			= 8;
    const int LONG_OBJ			= 9;
    const int HUGE_OBJ			= 10;
        // Unimplemented.

    // Uncollectable.
    //
    const int AUX_FREE			= -1;
    const int LABEL_AUX			= -2;
    const int LIST_AUX			= -3;
    const int SUBLIST_AUX		= -4;
    const int HASHTABLE_AUX		= -5;
    const int RELOCATE_BODY		= -6;

    namespace unprotected {
	// Flags for non-acc control values.
	//
	const min::uns64 STUB_POINTER =
	    min::uns64(1) << 55;
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

#   if MIN_POINTER_BITS <= 32
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

    inline void * uns64_to_pointer ( min::uns64 v )
    {
	return (void *) (min::unsptr) v;
    }
    inline min::uns64 pointer_to_uns64 ( void * p )
    {
	return (min::uns64) (min::unsptr) p;
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
// garbage collectable stubs.
//
// Non-acc control values are used as the second word
// of non-garbage-collectable stubs, and as the first
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

    inline bool is_collectable ( int type )
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

        inline void * pointer_of ( const min::stub * s )
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

namespace min { namespace internal {


    extern bool relocated_flag;
	// On if bodies have been relocated.

    extern min::stub ** acc_stack;
    extern min::stub ** acc_stack_limit;
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

        extern gen_locator * static_gen_last;
        extern gen_locator * stack_gen_last;
    }

    template < min::unsptr len > struct static_gen
    {
        min::internal::gen_locator locator;
	min::gen values[len];

	static_gen ( void )
	{
	    this->locator.length = len;
	    this->locator.values = values;
	    this->locator.previous =
	        min::internal::static_gen_last;
	    min::internal::static_gen_last =
	        & this->locator;
	    memset ( values, 0, sizeof ( values ) );
	}

	~ static_gen ( void )
	{
	    min::internal::static_gen_last =
	        this->locator.previous;
	}

	min::gen & operator[] ( min::unsptr i )
	{
	    MIN_ASSERT ( i < len );
	    return values[i];
	}
    };

    template < min::unsptr len > struct stack_gen
    {
        min::internal::gen_locator locator;
	min::gen values[len];

	stack_gen ( void )
	{
	    this->locator.length = len;
	    this->locator.values = values;
	    this->locator.previous =
	        min::internal::stack_gen_last;
	    min::internal::stack_gen_last =
	        & this->locator;
	    memset ( values, 0, sizeof ( values ) );
	}

	~ stack_gen ( void )
	{
	    min::internal::stack_gen_last =
	        this->locator.previous;
	}

	min::gen & operator[] ( min::unsptr i )
	{
	    MIN_ASSERT ( i < len );
	    return values[i];
	}
    };

    template < min::unsptr len > struct static_num_gen
    {
#       if MIN_IS_COMPACT
	    min::internal::gen_locator locator;
#	endif
	min::gen values[len];

	static_num_gen ( void )
	{
#           if MIN_IS_COMPACT
		this->locator.length = len;
		this->locator.values = values;
		this->locator.previous =
		    min::internal::static_gen_last;
		min::internal::static_gen_last =
		    & this->locator;
#	    endif
	    memset ( values, 0, sizeof ( values ) );
	}

#       if MIN_IS_COMPACT
	~ static_num_gen ( void )
	{
	    min::internal::static_gen_last =
	        this->locator.previous;
	}
#	endif

	min::gen & operator[] ( min::unsptr i )
	{
	    MIN_ASSERT ( i < len );
	    return values[i];
	}
    };

    template < min::unsptr len > struct stack_num_gen
    {
#       if MIN_IS_COMPACT
	    min::internal::gen_locator locator;
#	endif
	min::gen values[len];

	stack_num_gen ( void )
	{
#           if MIN_IS_COMPACT
		this->locator.length = len;
		this->locator.values = values;
		this->locator.previous =
		    min::internal::stack_gen_last;
		min::internal::stack_gen_last =
		    & this->locator;
#	    endif
	    memset ( values, 0, sizeof ( values ) );
	}

#       if MIN_IS_COMPACT
	~ stack_num_gen ( void )
	{
	    min::internal::stack_gen_last =
	        this->locator.previous;
	}
#	endif

	min::gen & operator[] ( min::unsptr i )
	{
	    MIN_ASSERT ( i < len );
	    return values[i];
	}
    };
}

namespace min { namespace internal {

    // Function called by the the initializer (see
    // `Initialization' above) to initialize the acc.
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

    // Hash tables for atoms.  The elements of these
    // tables head a list of stubs chained together
    // by their stub control word stub pointers.  The
    // first stubs in each list are HASHTABLE_AUX
    // stubs whose values point at the stubs of
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
    extern min::unsptr str_hash_size;
    extern min::unsptr str_hash_mask;

    extern min::stub ** num_hash;
    extern min::unsptr num_hash_size;
    extern min::unsptr num_hash_mask;

    extern min::stub ** lab_hash;
    extern min::unsptr lab_hash_size;
    extern min::unsptr lab_hash_mask;

    // Acc flags are bits 55 .. m of an acc control
    // value, where 56 - m == MIN_ACC_FLAG_BITS.
    //
    // The acc flag bit layout (bit 0 is lowest order)
    // is:
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
	    << ( 56 - MIN_ACC_FLAG_BITS ) + 2 );
    const uns64 ACC_UNMARKED_MASK =
        ACC_SCAVENGED_MASK >> 1;
    const uns64 ACC_FIXED_BODY_FLAG =
	( uns64(1) << ( 56 - MIN_ACC_FLAG_BITS ) );

} }

namespace min { namespace internal {

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
    // the acc_stack is used by the collector to adjust
    // the marks it makes on objects.
    //
    // For efficiency, acc_stack_mask is an uns64 that
    // only has ON bits in the unmarked flag positions.
    // Then the unshifted control value of s2 and the
    // control value value of s1 right shifted by 1 can
    // be bitwise ANDed with the acc_stack_mask and the
    // result checked for zero.
    //
    // WARNING: only unmarked flag bits may be on in
    // MUP::acc_stack_mask.
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
	    * min::internal::acc_stack ++ = s1;
	    * min::internal::acc_stack ++ = s2;
	}
    }

    // Function executed whenever a general value v is
    // stored in a datum with stub s1, and the general
    // value may contain a stub pointer.  This function
    // calls acc_write_update ( s1, s2 ) if the general
    // value contains the stub pointer s2.
    //
    inline void acc_write_update
	    ( min::stub * s1, min::gen v )
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
	    ( min::stub * s1,
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
	    ( min::stub * s1,
	      const min::gen * p, min::unsptr n )
    {
#       if MIN_IS_COMPACT
	    acc_write_update ( s1, p, n );
#	endif
    }

} }

namespace min { namespace internal {

    // Stub allocation is from a single list of stubs
    // chained together by the chain part of the stub
    // control.  This is referred to as the `acc stub
    // list'.
    //
    // A pointer to the last allocated stub is maintain-
    // ed.  To allocate a new stub, this is updated to
    // the next stub on the list, if any.  Otherwise, if
    // there is no next stub, an out-of-line function,
    // acc_expand_stub_free_list, is called to add to
    // the end of the list.
    //
    // Unallocated acc stubs have stub type min::
    // ACC_FREE, zero stub control flags, and min::NONE
    // value.

    // Pointer to the last allocated stub, which must
    // exist (it can be a dummy).
    //
    extern min::stub * last_allocated_stub;

    // Flags of stub returned by new_acc_stub.
    //
    extern min::uns64 new_acc_stub_flags;

} }

namespace min { namespace unprotected {

    // Function to return the next free stub as a acc
    // (garbage collectable) stub.  The type is set to
    // min::ACC_FREE and may be changed to any garbage
    // collectable type.  The value is NOT set.  The acc
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
    // removing this stub from the acc stub list.
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

    // Function to put a stub on the acc stub list right
    // after the last allocated stub.  Stub must NOT be
    // on the acc list (it should be an auxiliary stub).
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
    }

} }

namespace min { namespace internal {

    // fixed_blocks[j] is the head of a free list of
    // fixed blocks of size 1 << ( j + 3 ), for 2 <= j,
    // (1<<j) <= MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE/8.
    // Each fixed block begins with a control word whose
    // locator is provided by the allocator and whose
    // stub address is set to the stub whose body the
    // block contains, if there is such a stub, or
    // equals the address of MINT::null_stub otherwise.
    // When the block is on the free list, the control
    // word and the words following it are specified by
    // the free_fixed_size_block struct.
    //
    struct free_fixed_size_block
    {
        min::uns64	block_control;
	    // Block control word that is at the
	    // beginning of every fixed size block.

        min::uns64	block_subcontrol;
	    // Block subcontrol word that tells the type
	    // and size of a block that does NOT contain
	    // an object body.

	free_fixed_size_block * next;
	    // Next in NULL terminated list of free
	    // fixed size blocks.
    };
    typedef struct fixed_block_list_extension;
        // Allocator specific extension of fixed_block_
	// list struct.
    extern struct fixed_block_list
    {
	min::unsptr size;
			// Size of fixed block, includ-
	                // ing control word.  For fixed_
			// blocks[j] this equals
			// 1 << (j+3).
        min::unsptr count;
			// Number of fixed blocks on the
			// list.
	free_fixed_size_block * first;
			// First fixed block on the
			// list, or NULL if list empty.
	fixed_block_list_extension * extension;
			// Address of extension of this
			// structure.  Set during
			// allocator initialization.
    } fixed_blocks
          [MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG+1-3];

    // Out of line allocators:
    //
    void new_non_fixed_body
	( min::stub * s, min::unsptr n );
    void new_fixed_body
	( min::stub * s, min::unsptr n,
	  fixed_block_list * fbl );

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

        MIN_ASSERT
	  ( m >= sizeof
	             ( min::internal
	                  ::free_fixed_size_block ) );

	if ( m > min::internal::max_fixed_block_size )
	{
	     min::internal::new_non_fixed_body ( s, n );
	     return;
	}

	// See min_parameters.h for fixed_bodies_log.
	//
	m = m - 1;
	min::internal::fixed_block_list * fbl =
		  min::internal::fixed_blocks
		+ min::internal::fixed_bodies_log ( m )
		+ 1 - 3;

	if ( fbl->count == 0 )
	{
	     min::internal::new_fixed_body
	          ( s, n, fbl );
	     return;
	}

	min::internal::free_fixed_size_block * b =
	    fbl->first;

	fbl->first = b->next;
	-- fbl->count;

	b->block_control =
	    min::unprotected::renew_control_stub
	        ( b->block_control, s );
	min::unprotected
	   ::set_pointer_of
	       ( s, & b->block_control + 1 );
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
		    unprotected::pointer_of ( s ) - 1;
		* bp = unprotected::renew_control_stub
			 ( * bp, s );
		bp = (min::uns64 *)
		    unprotected::pointer_of ( rstub )
		    - 1;
		* bp = unprotected::renew_control_stub
			 ( * bp, rstub );

		unprotected::deallocate_body
		    ( rstub, old_size );
	    }

	    last_allocated = last_allocated_save;
	}

	friend void * & new_body_pointer_ref
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
    inline void * & new_body_pointer_ref
	    ( resize_body & r )
    {
	return min::unprotected
		  ::pointer_ref_of ( r.rstub );
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
	inline min::gen new_num_gen ( int v )
	{
	    if ( ( -1 << 27 ) <= v && v < ( 1 << 27 ) )
		return unprotected::new_direct_int_gen
				( v );
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
	       min::internal
	          ::uns64_to_pointer ( s->v.u64 );
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

    // String Pointers:

    // Some forward reference stuff that must be
    // declared here before it is referenced by a
    // friend declaration.
    //
    class str_pointer;
    namespace unprotected {
	const char * str_of
	    ( const min::str_pointer & sp );
    }
    void initialize
	( min::str_pointer & sp, min::gen v );
    min::unsptr strlen ( const min::str_pointer & sp );
    min::uns32 strhash ( const min::str_pointer & sp );
    char * strcpy ( char * p,
                    const min::str_pointer & sp );
    char * strncpy ( char * p,
                     const min::str_pointer & sp,
		     min::unsptr n );
    int strcmp
        ( const char * p, const min::str_pointer & sp );
    int strncmp
        ( const char * p, const min::str_pointer & sp );

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
	    ( const str_pointer & sp );
	friend void min::initialize
	    ( str_pointer & sp, min::gen v );
	friend min::unsptr min::strlen
	    ( const min::str_pointer & sp );
	friend min::uns32 min::strhash
	    ( const min::str_pointer & sp );
	friend char * min::strcpy
	    ( char * p, const min::str_pointer & sp );
	friend char * min::strncpy
	    ( char * p, const min::str_pointer & sp,
	                min::unsptr n );
	friend int min::strcmp
	    ( const char * p,
	      const min::str_pointer & sp );
	friend int min::strncmp
	    ( const char * p,
	      const min::str_pointer & sp,
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
		    // NUL terminated copy of string.
		uns64 str;
		uns64 xx[2];
		    // Sized to maintain alignment
		    // of surrounding data.
	    } u;
	} pseudo_body;
    };

    inline const char * unprotected::str_of
	    ( const min::str_pointer & sp )
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

    inline min::unsptr strlen
        ( const min::str_pointer & sp )
    {
        if ( sp.s == & sp.pseudo_stub )
	    return ::strlen ( sp.pseudo_body.u.buf );
	else
	    return unprotected::length_of
	              ( unprotected::long_str_of
		            ( sp.s ) );
    }

    inline min::uns32 strhash
	    ( const min::str_pointer & sp )
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
    	( char * p, const min::str_pointer & sp )
    {
        return ::strcpy
	    ( p, min::unprotected::str_of ( sp ) );
    }

    inline char * strncpy
    	( char * p, const min::str_pointer & sp,
	            min::unsptr n )
    {
        return ::strncpy
	    ( p, min::unprotected::str_of ( sp ), n );
    }

    inline int strcmp
    	( const char * p, const min::str_pointer & sp )
    {
        return ::strcmp
	    ( p, min::unprotected::str_of ( sp ) );
    }

    inline int strncmp
    	( const char * p, const min::str_pointer & sp,
	                  min::unsptr n )
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
	const min::stub * s =
	    min::unprotected::stub_of ( v );
	return min::type_of ( s ) == min::SHORT_STR
	       ||
	       min::type_of ( s ) == min::LONG_STR;
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
//
// This implementation is hidden so an implementation
// using uncollectable stubs is also possible.

namespace min { namespace internal {

    struct lab_header
    {
        uns32		length;
        uns32		hash;
    };

    const min::unsptr lab_header_size =
        sizeof ( lab_header ) / sizeof ( min::gen );

    inline min::internal::lab_header * lab_header_of
	    ( const min::stub * s )
    {
        return (min::internal::lab_header *)
	       min::unprotected::pointer_of ( s );
    }

    inline min::unsptr lablen
	    ( min::internal::lab_header * lh )
    {
        return lh->length;
    }

    inline min::uns32 labhash
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

    inline min::unsptr lab_of
	    ( min::gen * p, min::unsptr n,
	      const min::stub * s )
    {
        MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
	min::internal::lab_header * lh =
	    min::internal::lab_header_of ( s );
	const min::gen * q =
	    min::internal::vector_of ( lh );
	min::unsptr len = min::internal::lablen ( lh );

	min::unsptr count = 0;
        while ( count < n && len -- )
	{
	    * p ++ = * q ++;
	    ++ count;
	}
	return count;
    }

    inline min::unsptr lab_of
	    ( min::gen * p, min::unsptr n, min::gen v )
    {
	return min::lab_of ( p, n, min::stub_of ( v ) );
    }

    inline min::unsptr lablen ( const min::stub * s )
    {
        MIN_ASSERT ( min::type_of ( s ) == min::LABEL );
	return min::internal::lablen
	    ( min::internal::lab_header_of ( s ) );
    }

    inline min::unsptr lablen ( min::gen v )
    {
	return min::lablen ( min::stub_of ( v ) );
    }

    inline min::uns32 labhash ( const min::stub * s )
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
	    ( const min::gen * p, min::unsptr n );

    min::gen new_lab_gen
	    ( const min::gen * p, min::unsptr n );

    inline bool is_lab ( min::gen v )
    {
	if ( ! min::is_stub ( v ) )
	    return false;
	const min::stub * s =
	    min::unprotected::stub_of ( v );
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
    // OBJ_PUBLIC means insertable_vec_pointers not
    // allowed for object.
    //
    // OBJ_PRIVATE means a xxx_vec_pointer exists for
    // the object and other xxx_vec_pointers may not
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
	  MIN_POINTER_BITS <= 32 ?
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
	       unprotected::pointer_of ( s );
    }

    inline min::internal::long_obj * long_obj_of
	    ( const min::stub * s )
    {
        return (min::internal::long_obj *)
	       unprotected::pointer_of ( s );
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
    class vec_pointer;
    class updatable_vec_pointer;
    class insertable_vec_pointer;

    void initialize
	( min::vec_pointer & vp, const min::stub * s );
    void initialize
	( min::vec_pointer & vp, min::gen v );

    namespace unprotected {
	min::gen * & base
	    ( min::vec_pointer & vp );
	min::stub * stub_of
	    ( min::vec_pointer & vp );
	min::unsptr var_offset_of
	    ( min::vec_pointer & vp );
	min::unsptr hash_offset_of
	    ( min::vec_pointer & vp );
	min::unsptr attr_offset_of
	    ( min::vec_pointer & vp );
	min::unsptr unused_offset_of
	    ( min::vec_pointer & vp );
	min::unsptr aux_offset_of
	    ( min::vec_pointer & vp );
    }
    const min::stub * stub_of
	( min::vec_pointer & vp );
    min::unsptr var_size_of
	( min::vec_pointer & vp );
    min::unsptr hash_size_of
	( min::vec_pointer & vp );
    min::unsptr attr_size_of
	( min::vec_pointer & vp );
    min::unsptr unused_size_of
	( min::vec_pointer & vp );
    min::unsptr aux_size_of
	( min::vec_pointer & vp );
    min::unsptr total_size_of
	( min::vec_pointer & vp );
    min::gen var
	( min::vec_pointer & vp, min::unsptr index );
    min::gen hash
	( min::vec_pointer & vp, min::unsptr index );
    min::gen attr
	( min::vec_pointer & vp, min::unsptr index );
    min::gen aux
	( min::vec_pointer & vp, min::unsptr index );

    void initialize
	( min::updatable_vec_pointer & vp,
	  min::stub * s );
    void initialize
	( min::updatable_vec_pointer & vp,
	  min::gen v );

    namespace unprotected {
	min::gen * & base
	    ( min::updatable_vec_pointer & vp );
    }
    void set_var
	( min::updatable_vec_pointer & vp,
	  min::unsptr index, min::gen value );
    void set_hash
	( min::updatable_vec_pointer & vp,
	  min::unsptr index, min::gen value );
    void set_attr
	( min::updatable_vec_pointer & vp,
	  min::unsptr index, min::gen value );
    void set_aux
	( min::updatable_vec_pointer & vp,
	  min::unsptr index, min::gen value );

    void initialize
	( min::insertable_vec_pointer & vp,
	  min::stub * s );
    void initialize
	( min::insertable_vec_pointer & vp,
	  min::gen v );

    namespace unprotected {
	min::unsptr & unused_offset_of
	    ( min::insertable_vec_pointer & vp );
	min::unsptr & aux_offset_of
	    ( min::insertable_vec_pointer & vp );
    }

    void attr_push
	( min::insertable_vec_pointer & vp,
	  min::gen v );
    void attr_push
	( min::insertable_vec_pointer & vp,
	  const min::gen * p, min::unsptr n );
    void aux_push
	( min::insertable_vec_pointer & vp,
	  min::gen v );
    void aux_push
	( min::insertable_vec_pointer & vp,
	  const min::gen * p, min::unsptr n );

    void attr_pop
	( min::insertable_vec_pointer & vp,
	  min::gen & v );
    void attr_pop
	( min::insertable_vec_pointer & vp,
	  min::gen * p, min::unsptr n );
    void aux_pop
	( min::insertable_vec_pointer & vp,
	  min::gen & v );
    void aux_pop
	( min::insertable_vec_pointer & vp,
	  min::gen * p, min::unsptr n );

    bool resize
	( min::insertable_vec_pointer & vp,
	  min::unsptr unused_size );
    bool resize
	( min::insertable_vec_pointer & vp,
	  min::unsptr unused_size,
	  min::unsptr var_size );

    class vec_pointer
    {
    public:

	vec_pointer ( const min::stub * s )
	    : s ( (min::stub *) s ), type ( READONLY )
	    { init(); }
	vec_pointer ( min::gen v )
	    : s ( (min::stub *) min::stub_of ( v ) ),
	      type ( READONLY )
	    { init(); }

        // Friends
	//
	friend void initialize
	    ( min::vec_pointer & vp,
	      const min::stub * s );
	friend void initialize
	    ( min::vec_pointer & vp, min::gen v );

	friend min::gen * & unprotected::base
	    ( min::vec_pointer & vp );
	friend min::stub * unprotected::stub_of
	    ( min::vec_pointer & vp );
	friend min::unsptr unprotected::var_offset_of
	    ( min::vec_pointer & vp );
	friend min::unsptr unprotected::hash_offset_of
	    ( min::vec_pointer & vp );
	friend min::unsptr unprotected::attr_offset_of
	    ( min::vec_pointer & vp );
	friend min::unsptr unprotected::unused_offset_of
	    ( min::vec_pointer & vp );
	friend min::unsptr unprotected::aux_offset_of
	    ( min::vec_pointer & vp );

	friend const min::stub * stub_of
	    ( min::vec_pointer & vp );
	friend min::unsptr var_size_of
	    ( min::vec_pointer & vp );
	friend min::unsptr hash_size_of
	    ( min::vec_pointer & vp );
	friend min::unsptr attr_size_of
	    ( min::vec_pointer & vp );
	friend min::unsptr unused_size_of
	    ( min::vec_pointer & vp );
	friend min::unsptr aux_size_of
	    ( min::vec_pointer & vp );
	friend min::unsptr total_size_of
	    ( min::vec_pointer & vp );

	friend min::gen var
	    ( min::vec_pointer & vp,
	      min::unsptr index );
	friend min::gen hash
	    ( min::vec_pointer & vp,
	      min::unsptr index );
	friend min::gen attr
	    ( min::vec_pointer & vp,
	      min::unsptr index );
	friend min::gen aux
	    ( min::vec_pointer & vp,
	      min::unsptr index );

	friend void initialize
	    ( min::updatable_vec_pointer & vp,
	      min::stub * s );
	friend void initialize
	    ( min::updatable_vec_pointer & vp,
	      min::gen v );

	friend min::gen * & unprotected::base
	    ( min::updatable_vec_pointer & vp );
	friend void set_var
	    ( min::updatable_vec_pointer & vp,
	      min::unsptr index, min::gen value );
	friend void set_hash
	    ( min::updatable_vec_pointer & vp,
	      min::unsptr index, min::gen value );
	friend void set_attr
	    ( min::updatable_vec_pointer & vp,
	      min::unsptr index, min::gen value );
	friend void set_aux
	    ( min::updatable_vec_pointer & vp,
	      min::unsptr index, min::gen value );

	friend void initialize
	    ( min::insertable_vec_pointer & vp,
	      min::stub * s );
	friend void initialize
	    ( min::insertable_vec_pointer & vp,
	      min::gen v );

	friend min::unsptr &
	  unprotected::unused_offset_of
	    ( min::insertable_vec_pointer & vp );
	friend min::unsptr & unprotected::aux_offset_of
	    ( min::insertable_vec_pointer & vp );

	friend void attr_push
	    ( min::insertable_vec_pointer & vp,
	      min::gen v );
	friend void attr_push
	    ( min::insertable_vec_pointer & vp,
	      const min::gen * p, min::unsptr n );
	friend void aux_push
	    ( min::insertable_vec_pointer & vp,
	      min::gen v );
	friend void aux_push
	    ( min::insertable_vec_pointer & vp,
	      const min::gen * p, min::unsptr n );

	friend void attr_pop
	    ( min::insertable_vec_pointer & vp,
	      min::gen & v );
	friend void attr_pop
	    ( min::insertable_vec_pointer & vp,
	      min::gen * p, min::unsptr n );
	friend void aux_pop
	    ( min::insertable_vec_pointer & vp,
	      min::gen & v );
	friend void aux_pop
	    ( min::insertable_vec_pointer & vp,
	      min::gen * p, min::unsptr n );

	friend bool resize
	    ( min::insertable_vec_pointer & vp,
	      min::unsptr unused_size );
	friend bool resize
	    ( min::insertable_vec_pointer & vp,
	      min::unsptr unused_size,
	      min::unsptr var_size );

	~ vec_pointer ( void )
	{
	    MIN_ASSERT ( type == READONLY );
	    int t = min::type_of ( s );
	    if ( t == min::SHORT_OBJ )
	    {
		internal::short_obj * so =
		    internal::short_obj_of ( s );

		so->flags &= ~ OBJ_PRIVATE;
	    }
	    else
	    {
	        MIN_ASSERT ( t == min::LONG_OBJ );
		internal::long_obj * lo =
		    internal::long_obj_of ( s );

		lo->flags &= ~ OBJ_PRIVATE;
	    }
	}

    protected:

	vec_pointer ( const min::stub * s, int type )
	    : s ( (min::stub *) s ), type ( type )
	    { init(); }

        enum {
	    READONLY = 1,
	    UPDATABLE = 2,
	    INSERTABLE = 3 };
	int type;
	    // Higher level destructors change this so
	    // the lower level destructors they
	    // implicitly call will not have errors.

    	min::stub * s;
	    // Stub of object; set by constructor or
	    // init function.

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

	~ updatable_vec_pointer ( void )
	{
	    MIN_ASSERT ( type == UPDATABLE );
	    type = READONLY;
	}

    protected:

	updatable_vec_pointer
		( const min::stub * s, int type )
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
	          ( min::stub_of ( v ),
		    INSERTABLE )
	    {}

	~ insertable_vec_pointer ( void )
	{
	    int t = min::type_of ( s );
	    if ( t == min::SHORT_OBJ )
	    {
		internal::short_obj * so =
		    internal::short_obj_of ( s );
		so->unused_offset = unused_offset;
		so->aux_offset    = aux_offset;
	    }
	    else
	    {
	        MIN_ASSERT ( t == min::LONG_OBJ );
		internal::long_obj * lo =
		    internal::long_obj_of ( s );

		lo->unused_offset = unused_offset;
		lo->aux_offset    = aux_offset;
	    }
	    type = UPDATABLE;
	}
    };

    inline void initialize
	( min::vec_pointer & vp, const min::stub * s )
    {
	vp.~vec_pointer();
        new ( & vp ) min::vec_pointer ( s );
    }

    inline void initialize
	( min::vec_pointer & vp, min::gen v )
    {
	vp.~vec_pointer();
        new ( & vp ) min::vec_pointer ( v );
    }

    inline min::gen * & unprotected::base
	( min::vec_pointer & vp )
    {
	return * (min::gen **) &
	       min::unprotected::pointer_ref_of
	           ( vp.s );
    }
    inline min::stub * unprotected::stub_of
	( min::vec_pointer & vp )
    {
        return vp.s;
    }
    inline min::unsptr unprotected::var_offset_of
	( min::vec_pointer & vp )
    {
        return vp.var_offset;
    }
    inline min::unsptr unprotected::hash_offset_of
	( min::vec_pointer & vp )
    {
        return vp.hash_offset;
    }
    inline min::unsptr unprotected::attr_offset_of
	( min::vec_pointer & vp )
    {
        return vp.attr_offset;
    }
    inline min::unsptr unprotected::unused_offset_of
	( min::vec_pointer & vp )
    {
        return vp.unused_offset;
    }
    inline min::unsptr unprotected::aux_offset_of
	( min::vec_pointer & vp )
    {
        return vp.aux_offset;
    }

    inline const min::stub * stub_of
	( min::vec_pointer & vp )
    {
        return vp.s;
    }
    inline min::unsptr var_size_of
	( min::vec_pointer & vp )
    {
        return vp.hash_offset - vp.var_offset;
    }
    inline min::unsptr hash_size_of
	( min::vec_pointer & vp )
    {
        return vp.hash_size;
    }
    inline min::unsptr attr_size_of
	( min::vec_pointer & vp )
    {
        return vp.unused_offset - vp.attr_offset;
    }
    inline min::unsptr unused_size_of
	( min::vec_pointer & vp )
    {
        return vp.aux_offset - vp.unused_offset;
    }
    inline min::unsptr aux_size_of
	( min::vec_pointer & vp )
    {
        return vp.total_size - vp.aux_offset;
    }
    inline min::unsptr total_size_of
	( min::vec_pointer & vp )
    {
        return vp.total_size;
    }

    inline min::gen var
	( min::vec_pointer & vp, min::unsptr index )
    {
	index += vp.var_offset;
	MIN_ASSERT ( index < vp.hash_offset );
	return unprotected::base(vp)[index];
    }

    inline min::gen hash
	( min::vec_pointer & vp, min::unsptr index )
    {
	index += vp.hash_offset;
	MIN_ASSERT ( index < vp.attr_offset );
	return unprotected::base(vp)[index];
    }

    inline min::gen attr
	( min::vec_pointer & vp, min::unsptr index )
    {
	index += vp.attr_offset;
	MIN_ASSERT ( index < vp.unused_offset );
	return unprotected::base(vp)[index];
    }

    inline min::gen aux
	( min::vec_pointer & vp, min::unsptr index )
    {
	index = vp.total_size - index;
	MIN_ASSERT ( vp.aux_offset <= index );
	MIN_ASSERT ( index < vp.total_size );
	return unprotected::base(vp)[index];
    }

    inline void initialize
	( min::updatable_vec_pointer & vp,
	  min::stub * s )
    {
	vp.~updatable_vec_pointer();
        new ( & vp ) min::updatable_vec_pointer ( s );
    }

    inline void initialize
	( min::updatable_vec_pointer & vp, min::gen v )
    {
	vp.~updatable_vec_pointer();
        new ( & vp ) min::updatable_vec_pointer ( v );
    }

    inline min::gen * & unprotected::base
	( min::updatable_vec_pointer & vp )
    {
	return * (min::gen **) &
	       min::unprotected::pointer_ref_of
	           ( vp.s );
    }

    inline void set_var
	( min::updatable_vec_pointer & vp,
	  min::unsptr index, min::gen value )
    {
	index += vp.var_offset;
	MIN_ASSERT ( index < vp.hash_offset );
	unprotected::base(vp)[index] = value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void set_hash
	( min::updatable_vec_pointer & vp,
	  min::unsptr index, min::gen value )
    {
	index += vp.hash_offset;
	MIN_ASSERT ( index < vp.attr_offset );
	unprotected::base(vp)[index] = value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void set_attr
	( min::updatable_vec_pointer & vp,
	  min::unsptr index, min::gen value )
    {
	index += vp.attr_offset;
	MIN_ASSERT ( index < vp.unused_offset );
	unprotected::base(vp)[index] = value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void set_aux
	( min::updatable_vec_pointer & vp,
	  min::unsptr index, min::gen value )
    {
	index = vp.total_size - index;
	MIN_ASSERT ( vp.aux_offset <= index );
	MIN_ASSERT ( index < vp.total_size );
	unprotected::base(vp)[index] = value;
	unprotected::acc_write_update ( vp.s, value );
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

    inline min::unsptr & unprotected::unused_offset_of
	( min::insertable_vec_pointer & vp )
    {
        return vp.unused_offset;
    }
    inline min::unsptr & unprotected::aux_offset_of
	( min::insertable_vec_pointer & vp )
    {
        return vp.aux_offset;
    }

    inline void attr_push
	( min::insertable_vec_pointer & vp,
	  min::gen value )
    {
	MIN_ASSERT ( vp.unused_offset < vp.aux_offset );
	unprotected::base(vp)[vp.unused_offset ++] =
	    value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void attr_push
	( min::insertable_vec_pointer & vp,
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
	( min::insertable_vec_pointer & vp,
	  min::gen value )
    {
	MIN_ASSERT ( vp.unused_offset < vp.aux_offset );
	unprotected::base(vp)[-- vp.aux_offset] = value;
	unprotected::acc_write_update ( vp.s, value );
    }

    inline void aux_push
	( min::insertable_vec_pointer & vp,
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
	( min::insertable_vec_pointer & vp,
	  min::gen & v )
    {
	MIN_ASSERT
	    ( vp.attr_offset < vp.unused_offset );
	v = unprotected::base(vp)[-- vp.unused_offset];
    }

    inline void attr_pop
	( min::insertable_vec_pointer & vp,
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
	( min::insertable_vec_pointer & vp,
	  min::gen & v )
    {
	MIN_ASSERT ( vp.aux_offset < vp.total_size );
	v = unprotected::base(vp)[vp.aux_offset ++];
    }

    inline void aux_pop
	( min::insertable_vec_pointer & vp,
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
// the MUP::STUB_POINTER flag and the stub pointer in
// the control.

// Aux area elements that are not used are given the
// value NONE, and can be garbage collected when the
// object is reorganized.  Because they are often
// isolated, no attempt is made to put them on a free
// list.

namespace min { namespace unprotected {

    // This is the generic list pointer type from which
    // specific list pointer types are made.

    template < class vecpt >
        class list_pointer_type;

} }

namespace min {

    // Public list pointer types.

    typedef min::unprotected::list_pointer_type
	    < min::vec_pointer >
        list_pointer;
    typedef min::unprotected::list_pointer_type
	    < min::updatable_vec_pointer >
        updatable_list_pointer;
    typedef min::unprotected::list_pointer_type
	    < min::insertable_vec_pointer >
        insertable_list_pointer;

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
    	    ( min::insertable_list_pointer & lp,
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
    vecpt & vec_pointer_of
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp );

    template < class vecpt >
    min::gen start_hash
            ( min::unprotected
	         ::list_pointer_type<vecpt> & lp,
	      min::unsptr index );
    template < class vecpt >
    min::gen start_vector
            ( min::unprotected
	         ::list_pointer_type<vecpt> & lp,
	      min::unsptr index );
    template < class vecpt, class vecpt2 >
    min::gen start_copy
            ( min::unprotected
	         ::list_pointer_type<vecpt> & lp,
	      const
	      min::unprotected
	         ::list_pointer_type<vecpt2> & lp2 );
    template < class vecpt, class vecpt2 >
    min::gen start_sublist
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp,
    	      const min::unprotected
	         ::list_pointer_type<vecpt2> & lp2 );
    template < class vecpt >
    min::gen start_sublist
    	    ( min::insertable_list_pointer & lp,
    	      const min::unprotected
	         ::list_pointer_type<vecpt> & lp2 );

    template < class vecpt >
    min::gen next
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp );
    template < class vecpt >
    min::gen peek
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp );
    template < class vecpt >
    min::gen current
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp );
    template < class vecpt >
    min::gen update_refresh
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp );
    template < class vecpt >
    min::gen insert_refresh
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp );

    template < class vecpt >
    void update
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp,
	      min::gen value );

    bool insert_reserve
    	    ( min::insertable_list_pointer & lp,
	      min::unsptr insertions,
	      min::unsptr elements = 0,
	      bool use_obj_aux_stubs =
	          min::use_obj_aux_stubs );
    void insert_before
    	    ( min::insertable_list_pointer & lp,
	      const min::gen * p, min::unsptr n );
    void insert_after
    	    ( min::insertable_list_pointer & lp,
	      const min::gen * p, min::unsptr n );
    min::unsptr remove
    	    ( min::insertable_list_pointer & lp,
	      min::unsptr n = 1 );
}

namespace min { namespace unprotected {

    template <>
	class list_pointer_type
	          <min::insertable_vec_pointer> {

    public:

        list_pointer_type
		( min::insertable_vec_pointer & vecp )
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

	min::insertable_vec_pointer & vecp;
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
	    // vec pointer.  0 if list pointer has not
	    // yet been started.
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

	friend min::insertable_vec_pointer &
	    vec_pointer_of<>
		( min::insertable_list_pointer & lp );

	friend min::gen min::start_hash<>
		( min::insertable_list_pointer & lp,
		  min::unsptr index );
	friend min::gen min::start_vector<>
		( min::insertable_list_pointer & lp,
		  min::unsptr index );

	friend min::gen start_copy<>
		( min::insertable_list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );

	friend min::gen start_sublist<>
		( min::list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::updatable_list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::insertable_list_pointer & lp,
		  const
		  min::list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::insertable_list_pointer & lp,
		  const
		  min::updatable_list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::insertable_list_pointer & lp,
		  const
		  min::insertable_list_pointer & lp2 );

	friend min::gen min::next<>
		( min::insertable_list_pointer & lp );
	friend min::gen min::peek<>
		( min::insertable_list_pointer & lp );
	friend min::gen min::current<>
		( min::insertable_list_pointer & lp );
	friend min::gen min::update_refresh<>
		( min::insertable_list_pointer & lp );
	friend min::gen min::insert_refresh<>
		( min::insertable_list_pointer & lp );

	friend void min::update<>
		( min::insertable_list_pointer & lp,
		  min::gen value );
	friend bool min::insert_reserve
		( min::insertable_list_pointer & lp,
		  min::unsptr insertions,
		  min::unsptr elements,
		  bool use_obj_aux_stubs );
	friend bool min::internal::insert_reserve
		( min::insertable_list_pointer & lp,
		  min::unsptr insertions,
		  min::unsptr elements,
		  bool use_obj_aux_stubs );
	friend void min::insert_before
		( min::insertable_list_pointer & lp,
		  const min::gen * p, min::unsptr n );
	friend void min::insert_after
		( min::insertable_list_pointer & lp,
		  const min::gen * p, min::unsptr n );
	friend min::unsptr min::remove
		( min::insertable_list_pointer & lp,
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

    // The following is for min::list_pointers and
    // min::updatable_list_pointers and is just like
    // the class for min::insertable_list_pointers
    // except that the previous pointer and the
    // reservations are omitted.
    //
    template < class vecpt >
	class list_pointer_type {

    public:

        list_pointer_type ( vecpt & vecp )
	    : vecp ( vecp ),
	      base ( min::unprotected::base ( vecp ) ),
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

	friend vecpt & vec_pointer_of<>
		( min::unprotected
		     ::list_pointer_type<vecpt> & lp );

	friend min::gen min::start_hash<>
		( min::unprotected
		     ::list_pointer_type<vecpt> & lp,
		  min::unsptr index );
	friend min::gen min::start_vector<>
		( min::unprotected
		     ::list_pointer_type<vecpt> & lp,
		  min::unsptr index );

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
		  min::list_pointer & lp2 );
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
		  min::list_pointer & lp2 );
	friend min::gen start_sublist<>
		( min::insertable_list_pointer & lp,
		  const
		  min::updatable_list_pointer & lp2 );

	friend min::gen min::next<>
		( min::unprotected
		     ::list_pointer_type<vecpt> & lp );
	friend min::gen min::peek<>
		( min::unprotected
		     ::list_pointer_type<vecpt> & lp );
	friend min::gen min::current<>
		( min::unprotected
		     ::list_pointer_type<vecpt> & lp );
	friend min::gen min::update_refresh<>
		( min::unprotected
		     ::list_pointer_type<vecpt> & lp );
	friend min::gen min::insert_refresh<>
		( min::unprotected
		     ::list_pointer_type<vecpt> & lp );

	friend void min::update<>
		( min::updatable_list_pointer & lp,
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
    inline vecpt & vec_pointer_of
	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp )
    {
    	return lp.vecp;
    }

    template < class vecpt >
    inline min::gen start_hash
            ( min::unprotected
	         ::list_pointer_type<vecpt> & lp,
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
	         ::list_pointer_type<vecpt> & lp,
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
    // legal combinations of list_pointers, e.g.,
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
            ( min::unprotected
	         ::list_pointer_type<vecpt> & lp,
	      const
	      min::unprotected
	         ::list_pointer_type<vecpt2> & lp2 )
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
            ( min::insertable_list_pointer & lp,
	      const
	      min::insertable_list_pointer & lp2 )
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
    // legal combinations of list_pointers, just like
    // start_copy above.
    //
    template < class vecpt, class vecpt2 >
    inline min::gen start_sublist
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp,
    	      const min::unprotected
	         ::list_pointer_type<vecpt2> & lp2 )
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
    	    ( min::insertable_list_pointer & lp,
    	      const min::unprotected
	         ::list_pointer_type<vecpt> & lp2 )
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
	         ::list_pointer_type<vecpt> & lp )
    {
        if ( lp.current == min::LIST_END )
	    return min::LIST_END;

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
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
    	    ( min::insertable_list_pointer & lp )
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
	         ::list_pointer_type<vecpt> & lp )
    {
        if ( lp.current == min::LIST_END )
	    return min::LIST_END;

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
	        min::uns64 c =
		    min::unprotected::control_of
		    	( lp.current_stub );
		if ( c & min::unprotected
			    ::STUB_POINTER )
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
	         ::list_pointer_type<vecpt> & lp )
    {
    	return lp.current;
    }

    template < class vecpt >
    inline min::gen update_refresh
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp )
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

	MIN_ABORT ( "inconsistent list_pointer" );
    }

    template < class vecpt >
    inline min::gen insert_refresh
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp )
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

	MIN_ABORT ( "inconsistent list_pointer" );
    }

    template <>
    inline min::gen insert_refresh
    	    ( min::insertable_list_pointer & lp )
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

	MIN_ABORT ( "inconsistent list_pointer" );
    }

    template < class vecpt >
    inline min::gen start_sublist
    	    ( min::unprotected
	         ::list_pointer_type<vecpt> & lp )
    {
	return start_sublist ( lp, lp );
    }

    // Set is declared as a friend only for
    // updatable_ and insertable_ list_pointers.
    //
    template <>
    inline void update
    	    ( min::updatable_list_pointer & lp,
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
	    MIN_ABORT ( "inconsistent list_pointer" );
	}
    }
    template <>
    inline void update
    	    ( min::insertable_list_pointer & lp,
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
	    MIN_ABORT ( "inconsistent list_pointer" );
	}
    }

    inline bool insert_reserve
    	    ( min::insertable_list_pointer & lp,
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
        class attr_pointer_type;

} }

namespace min {

    // Public protected attribute pointer types.

    typedef min::unprotected::attr_pointer_type
	    < min::vec_pointer >
        attr_pointer;
    typedef min::unprotected::attr_pointer_type
	    < min::updatable_vec_pointer >
        updatable_attr_pointer;
    typedef min::unprotected::attr_pointer_type
	    < min::insertable_vec_pointer >
        insertable_attr_pointer;


    // We must declare these before we make them
    // friends.

    template < class vecpt >
    void locate
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap,
	      min::gen name );
    template < class vecpt >
    void locatei
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap,
	      int name );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	template < class vecpt >
	void locate
		( unprotected::attr_pointer_type
		      < vecpt > & ap,
		  min::unsptr & length, min::gen name );
#   endif
    template < class vecpt >
    void locate_reverse
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap,
	      min::gen reverse_name );
    template < class vecpt >
    void relocate
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap );
    template < class vecpt >
    min::unsptr count
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap );
    template < class vecpt >
    min::unsptr get
	    ( min::gen * out, min::unsptr n,
	      unprotected::attr_pointer_type
	          < vecpt > & ap );
    template < class vecpt >
    min::unsgen get
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap );
    template < class vecpt >
    unsigned count_flags
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap );
    template < class vecpt >
    unsigned get_flags
	    ( min::gen * out, unsigned n,
	      unprotected::attr_pointer_type
	          < vecpt > & ap );
    template < class vecpt >
    bool test_flag
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap,
	      unsigned n );
    template < class vecpt >
    unsigned get_attrs
	    ( min::gen * out, unsigned n,
	      unprotected::attr_pointer_type
	          < vecpt > & ap );
    template < class vecpt >
    bool rewind_attrs
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap );
    template < class vecpt >
    unsigned get_reverse_attrs
	    ( min::gen * out, unsigned n,
	      unprotected::attr_pointer_type
	          < vecpt > & ap );
    template < class vecpt >
    min::gen update
	    ( unprotected::attr_pointer_type
	          < vecpt > & ap,
	      min::gen v );
    void set
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, min::unsptr n );
    void set
	    ( min::insertable_attr_pointer & ap,
	      min::gen v );
    void add_to_set
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, min::unsptr n );
    void add_to_set
	    ( min::insertable_attr_pointer & ap,
	      min::gen v );
    void add_to_multiset
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, min::unsptr n );
    void add_to_multiset
	    ( min::insertable_attr_pointer & ap,
	      min::gen v );
    void remove_one
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, min::unsptr n );
    void remove_one
	    ( min::insertable_attr_pointer & ap,
	      min::gen v );
    void remove_all
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, min::unsptr n );
    void remove_all
	    ( min::insertable_attr_pointer & ap,
	      min::gen v );
    void set_flags
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, unsigned n );
    void set_some_flags
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, unsigned n );
    void clear_some_flags
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, unsigned n );
    void flip_some_flags
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, unsigned n );
    bool set_flag
	    ( min::insertable_attr_pointer & ap,
	      unsigned n );
    bool clear_flag
	    ( min::insertable_attr_pointer & ap,
	      unsigned n );
    bool flip_flag
	    ( min::insertable_attr_pointer & ap,
	      unsigned n );

    namespace internal {

#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    template < class vecpt >
	    void locate
		    ( unprotected::attr_pointer_type
			  < vecpt > & ap,
		      min::gen name,
		      bool allow_partial_label = false
		    );
#	else // ! MIN_ALLOW_PARTIAL_ATTR_LABELS
	    template < class vecpt >
	    void locate
		    ( unprotected::attr_pointer_type
			  < vecpt > & ap,
		      min::gen name );
#	endif
	template < class vecpt >
	void relocate
		( unprotected::attr_pointer_type
		      < vecpt > & ap );
	template < class vecpt >
	min::unsptr count
		( unprotected::attr_pointer_type
		      < vecpt > & ap );
	template < class vecpt >
	min::unsptr get
		( min::gen * out, min::unsptr n,
		  unprotected::attr_pointer_type
		      < vecpt > & ap );
	template < class vecpt >
	min::gen update
		( unprotected::attr_pointer_type
		      < vecpt > & ap,
		  min::gen v );
	void set
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, min::unsptr n );
	void set_flags
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, unsigned n );
	void set_more_flags
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, unsigned n );
	void attr_create
		( min::insertable_attr_pointer & ap );
	void reverse_attr_create
		( min::insertable_attr_pointer & ap );
    }

}

namespace min { namespace unprotected {


    template < class vecpt >
    class attr_pointer_type {

    public:

        attr_pointer_type ( vecpt & vecp )
	    : dlp ( vecp ), locate_dlp ( vecp ),
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
	    // last reverse_locate function call, and
	    // also reset to NONE by a locate function
	    // call.  Can be NONE or ANY.

#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    min::unsptr length;
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
		    //
		    // A call to locate succeeds if it
		    // finds an attribute- or node-
		    // descriptor, and fails otherwise.
		    //
		    // A locate with a length argument
		    // succeeds if it returns length > 0
		    // and fails otherwise.
		    //
		    // A call to get_attrs moves to the
		    // first label.  A call go rewind_
		    // attrs sets this state.

		END_INIT		= 1,
		    // Like INIT but a call to get_attrs
		    // does not deliver any labels.
		    // This state is only set by a call
		    // to get_attrs that delivers the
		    // last label.

		LOCATE_FAIL		= 2,
		    // Last call to locate failed.

		// Note: states >= LOCATE_NONE imply
		// the last call to locate succeeded.

		LOCATE_NONE		= 3,
		    // Last call to locate succeeded,
		    // and no call to reverse_locate
		    // has been made or the last call
		    // to reverse_locate set the
		    // reverse_attribute to NONE.

		LOCATE_ANY		= 4,
		    // Last call to reverse_locate set
		    // the reverse attribute to ANY.

		REVERSE_LOCATE_FAIL	= 5,
		    // Last call to reverse_locate when
		    // the state was >= LOCATE_NONE set
		    // the reverse attribute to a value
		    // other than NONE or ANY and
		    // failed.

		REVERSE_LOCATE_SUCCEED	= 6
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

    	list_pointer_type<vecpt> dlp;
	    // Descriptor list pointer.  Points at the
	    // list element containing the attribute- or
	    // node- descriptor found, unless the state
	    // is INIT, END_INIT, LOCATE_FAIL, or
	    // REVERSE_LOCATE_FAIL, in which case this
	    // is not set.

    	list_pointer_type<vecpt> locate_dlp;
	    // This is the value of dlp after the last
	    // successful locate, or the value dlp
	    // would have had if the last unsuccessful
	    // locate had a length argument, in the case
	    // where partial labels are allowed.  This
	    // last permits the `set' function to be
	    // optimized.
	    //
	    // This is not set if the state is INIT or
	    // END_INIT or if the state is LOCATE_FAIL
	    // and length member does not exist or is
	    // == 0.

	min::unsptr index;
	    // Hash or attribute vector index passed to
	    // the start_hash or start_vector functions
	    // by the last call to locate.  See the
	    // IN_VECTOR flag above.
	        

    // Friends:

	friend void locate<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap,
		  min::gen name );
	friend void locatei<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap,
		  int name );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    friend void locate<>
		    ( min::unprotected
			 ::attr_pointer_type<vecpt>
			     & ap,
		      min::unsptr & length,
		      min::gen name );
#   endif
	friend void min::locate_reverse<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap,
		  min::gen reverse_name );
	friend void min::relocate<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend min::unsptr min::count<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend min::unsptr min::get<>
		( min::gen * out, min::unsptr n,
		  min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend min::unsgen min::get<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend unsigned min::count_flags<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend unsigned min::get_flags<>
		( min::gen * out, unsigned n,
		  min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend bool min::test_flag<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap,
		  unsigned n );
	friend unsigned min::get_attrs<>
		( min::gen * out, unsigned n,
		  min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend bool min::rewind_attrs<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend unsigned min::get_reverse_attrs<>
		( min::gen * out, unsigned n,
		  min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend min::gen min::update<>
		( min::updatable_attr_pointer & ap,
		  min::gen v );
	friend min::gen min::update<>
		( min::insertable_attr_pointer & ap,
		  min::gen v );
	friend void min::set
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::set
		( min::insertable_attr_pointer & ap,
		  min::gen v );
	friend void min::add_to_set
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::add_to_set
		( min::insertable_attr_pointer & ap,
		  min::gen v );
	friend void min::add_to_multiset
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::add_to_multiset
		( min::insertable_attr_pointer & ap,
		  min::gen v );
	friend void min::remove_one
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::remove_one
		( min::insertable_attr_pointer & ap,
		  min::gen v );
	friend void min::remove_all
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::remove_all
		( min::insertable_attr_pointer & ap,
		  min::gen v );
	friend void min::set_flags
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, unsigned n );
	friend void min::set_some_flags
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, unsigned n );
	friend void min::clear_some_flags
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, unsigned n );
	friend void min::flip_some_flags
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, unsigned n );
	friend bool min::set_flag
		( min::insertable_attr_pointer & ap,
		  unsigned n );
	friend bool min::clear_flag
		( min::insertable_attr_pointer & ap,
		  unsigned n );
	friend bool min::flip_flag
		( min::insertable_attr_pointer & ap,
		  unsigned n );

    #	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    friend void min::internal::locate<>
		    ( min::unprotected
			 ::attr_pointer_type<vecpt>
			     & ap,
		      min::gen name,
		      bool allow_partial_label
		    );
#	else // ! MIN_ALLOW_PARTIAL_ATTR_LABELS
	    friend void min::internal::locate<>
		    ( min::unprotected
			 ::attr_pointer_type<vecpt>
			     & ap,
		      min::gen name );
#	endif
	friend void min::internal::relocate<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend min::unsptr min::internal::count<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend min::unsptr min::internal::get<>
		( min::gen * out, min::unsptr n,
		  min::unprotected
		     ::attr_pointer_type<vecpt> & ap );
	friend min::gen min::internal::update<>
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap,
		  min::gen v );
	friend void min::internal::set
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, min::unsptr n );
	friend void min::internal::set_flags
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, unsigned n );
	friend void min::internal::set_more_flags
		( min::insertable_attr_pointer & ap,
		  const min::gen * in, unsigned n );
	friend void min::internal::attr_create
		( min::insertable_attr_pointer & ap );
	friend void min::internal::reverse_attr_create
		( min::insertable_attr_pointer & ap );

    };

} }

namespace min {

    // Inline functions.  See MIN design document.

    template < class vecpt >
    inline void locatei
	    ( min::unprotected
	         ::attr_pointer_type<vecpt> & ap,
	      int name )
    {
	typedef min::unprotected
	           ::attr_pointer_type<vecpt> ap_type;

	ap.attr_name = min::new_num_gen ( name );

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
	    ap.reverse_attr_name = min::NONE;

#	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		ap.length = 1;
#	    endif

	    return;
	}
	
	internal::locate ( ap, ap.attr_name );
    }

    template < class vecpt >
    inline void locate
	    ( min::unprotected
	         ::attr_pointer_type<vecpt> & ap,
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

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	template < class vecpt >
	inline void locate
		( min::unprotected
		     ::attr_pointer_type<vecpt> & ap,
		  min::unsptr & length,
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

    template < class vecpt >
    inline min::unsptr count
	    ( min::unprotected
	         ::attr_pointer_type<vecpt> & ap )
    {
	typedef min::unprotected
	           ::attr_pointer_type<vecpt> ap_type;

	min::gen c = current ( ap.dlp );
	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::END_INIT:
	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
		return 0;
	case ap_type::LOCATE_NONE:
	case ap_type::REVERSE_LOCATE_SUCCEED:
		if ( ! is_sublist ( c ) ) return 1;
	}
	return internal::count ( ap );
    }

    template < class vecpt >
    inline min::unsptr get
	    ( min::gen * out, min::unsptr n,
	      min::unprotected
	         ::attr_pointer_type<vecpt> & ap )
    {
	typedef unprotected::attr_pointer_type
		    < vecpt > ap_type;

	if ( n == 0 ) return 0;
	min::gen c =  current ( ap.dlp );
	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::END_INIT:
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

    template < class vecpt >
    inline unsigned count_flags
	    ( min::unprotected
	         ::attr_pointer_type<vecpt> & ap )
    {
	typedef min::unprotected
	           ::attr_pointer_type<vecpt> ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::END_INIT:
	case ap_type::LOCATE_FAIL:
		return 0;
	}

	min::gen c = current ( ap.locate_dlp );
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

    template < class vecpt >
    inline unsigned get_flags
	    ( min::gen * out, unsigned n,
	      min::unprotected
	         ::attr_pointer_type<vecpt> & ap )
    {
	typedef min::unprotected
	           ::attr_pointer_type<vecpt> ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::END_INIT:
	case ap_type::LOCATE_FAIL:
		return 0;
	}

	min::gen c =  current ( ap.locate_dlp );
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

// TBD

    template < class vecpt >
    inline min::gen update
	    ( min::unprotected
	         ::attr_pointer_type<vecpt> & ap,
	      min::gen v )
    {
	typedef min::unprotected
	           ::attr_pointer_type<vecpt> ap_type;

	min::gen c =  current ( ap.dlp );
	if ( ! is_sublist ( c ) ) switch ( ap.state )
	{
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::LOCATE_NONE:
		update ( ap.dlp, v );
		update_refresh ( ap.locate_dlp );
		return c;
	}

	return internal::update ( ap, v ); 
    }

    inline void set
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, min::unsptr n )
    {
	typedef min::insertable_attr_pointer ap_type;

	min::gen c =  current ( ap.dlp );
	if ( n == 1 ) switch ( ap.state )
	{
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::LOCATE_NONE:
		if ( ! is_sublist ( c ) )
		{
		    update ( ap.dlp, * in );
		    update_refresh ( ap.locate_dlp );
		    return;
		}
	}

	internal::set ( ap, in, n  ); 
    }

    inline void set_flags
	    ( min::insertable_attr_pointer & ap,
	      const min::gen * in, unsigned n )
    {
	typedef insertable_attr_pointer ap_type;

	min::gen c = current ( ap.locate_dlp );
	switch ( ap.state )
	{
	case ap_type::LOCATE_NONE:
	case ap_type::LOCATE_ANY:
	case ap_type::REVERSE_LOCATE_SUCCEED:
	case ap_type::REVERSE_LOCATE_FAIL:
		if ( is_sublist ( c ) )
		{
		    updatable_list_pointer
		        lp ( vec_pointer_of
				( ap.locate_dlp ) );
		    start_sublist
		        ( lp, ap.locate_dlp );
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
			    ( ap, in, n );
		    else for ( ; is_control_code ( c );
		                 c = next ( lp ) )
		        update ( lp,
			         new_control_code_gen
				     ( 0 ) );
		    return;
		}
	}

	internal::set_flags ( ap, in, n );
    }

}

// TBD
// ---

namespace min {
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
	return   internal::lablen
		     ( internal::lab_header_of ( s ) )
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

    inline void deallocate ( min::stub * s )
    {
    	min::unprotected::deallocate_body
	    ( s, min::unprotected::body_size_of ( s ) );
    }
}


# endif // MIN_H
