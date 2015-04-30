// MIN Language Interface
//
// File:	min.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Apr 29 05:01:50 EDT 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Setup
//	C++ Number Types
//	General Value Types and Data
//	Stub Types and Data
//	Internal Pointer Conversion Functions
//	General Value Constructor Functions
//	General Value Test Functions
//	General Value Read Functions
//	Control Values
//	Stub Functions
//	Initializer Interface
//	Process Interface
//	Allocator/Collector/Compactor Interface
//	References
//	Pointers
//	Locatable Variables
//	UNICODE Characters
//	Numbers
//	Strings
//	Labels
//	Names
//	Packed Structures
//	Packed Vectors
//	Files
//	Identifier Maps
//	UNICODE Name Tables
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level
//	Printers
//	Printing General Values
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
# include "min_unicode.h"
# include <iostream>
# include <climits>
# include <cstring>
# include <new>

namespace min { namespace internal {

    // Return j such that (1<<(j-1)) < u <= (1<<j),
    // assuming 0 < u <= (1<<31).
    //
    inline unsigned log2ceil ( unsigned u )
    {
        MIN_ASSERT ( u != 0, "argument is zero" );
        return u == 1 ? 0
	              : 1 + log2floor ( u - 1 );
    }

} }

// C++ Number Types
// --- ------ -----

namespace min {

    typedef unsigned char uns8;
    typedef signed char int8;

    typedef unsigned short uns16;
    typedef signed short int16;

    typedef unsigned MIN_INT32_TYPE uns32;
    typedef unsigned MIN_INT32_TYPE Uchar;
    typedef signed MIN_INT32_TYPE int32;
    typedef float float32;

    typedef unsigned MIN_INT64_TYPE uns64;
    typedef signed MIN_INT64_TYPE int64;
    typedef double float64;

#   if MIN_PTR_BITS <= 32
	typedef uns32 unsptr;
	typedef int32 intptr;
	const unsptr unsptr_max = 0xFFFFFFFF;
	const intptr intptr_max = 0x7FFFFFFF;
	const intptr intptr_min = - 0x80000000;
#   else
	typedef uns64 unsptr;
	typedef int64 intptr;
	const unsptr unsptr_max = 0xFFFFFFFFFFFFFFFF;
	const intptr intptr_max = 0x7FFFFFFFFFFFFFFF;
	const intptr intptr_min = - 0x8000000000000000;
#   endif
}

// General Value Types and Data
// ------- ----- ----- --- ----

namespace min {

#   if MIN_IS_COMPACT

	// General Value Subtype (high order 8 bits)
	//   0x00-0x0F  direct integers >= 0
	//   0x10-0x18  illegal
	//   0x19-0x1F  auxiliary pointers, direct
	//              string, index, control code,
	//              special
	//   0x20-0xEF	stubs
	//   0xF0-0xFF  direct integers < 0

	// Unimplemented for COMPACT:
	//   unsigned GEN_DIRECT_FLOAT
	const unsigned GEN_DIRECT_INT
	    = 0x00;  // First non-negative integer
	             // subtype code.
	const unsigned GEN_DIRECT_INT_BOUND
	    = 0x10;  // Last non-negative integer
	             // subtype code + 1.
	const unsigned GEN_ILLEGAL
	    = 0x10;  // First illegal subtype code.
	const unsigned GEN_ILLEGAL_BOUND
	    = 0x19;  // Largest illegal subtype code
	             // + 1.
	const unsigned GEN_AUX
	    = 0x19;  // First aux subtype code.
	const unsigned GEN_INDIRECT_AUX
	    = 0x19;
	const unsigned GEN_SUBLIST_AUX
	    = 0x1A;
	const unsigned GEN_LIST_AUX
	    = 0x1B;
	const unsigned GEN_AUX_BOUND
	    = 0x1C;  // Largest aux subtype code + 1.
	const unsigned GEN_DIRECT_STR
	    = 0x1C;
	const unsigned GEN_INDEX
	    = 0x1D;
	const unsigned GEN_CONTROL_CODE
	    = 0x1E;
	const unsigned GEN_SPECIAL
	    = 0x1F;
	const unsigned GEN_STUB
	    = 0x20;   // First stub subtype code
	const unsigned GEN_STUB_BOUND
	    = 0xF0;   // Last stub subtype code + 1.
	const unsigned GEN_NEGATIVE_INT
	    = 0xF0;   // Integer < 0.
	const unsigned GEN_UPPER
	    = 0xFF;   // Largest subtype code.

	typedef uns32 unsgen;
	    // Unsigned type convertable to a min::gen
	    // value.
	const unsigned TSIZE = 8;
	const unsigned VSIZE = 24;
	    // Sized of type and value field in a
	    // min::gen.
#	define MIN_AMAX 0xCFFFFFFFu
	    // Maximum packed stub address value in
	    // general value.
	const int GEN_MIN_INT = - 0x10000000;
	const int GEN_MAX_INT = + 0x0FFFFFFF;
	    // Minimum and maximum integer storable
	    // in a direct integer min::gen value.

#   elif MIN_IS_LOOSE

	// General Value Subtype (high order 24 bits)
	// with base MIN_FLOAT_SIGNALLING_NAN:
	//
	//   0x00-0x0F	stub
	//   0x10-0x18	illegal
	//   0x19-0x1F  auxiliary pointers, direct
	//              string, index, control code,
	//              special
	//   other	floating point

	// Unimplemented for LOOSE:
	//   unsigned GEN_DIRECT_INT
	//   unsigned GEN_NEGATIVE_INT
	const unsigned GEN_DIRECT_FLOAT
	    = 0;
	const unsigned GEN_STUB
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x00;
	        // First stub subtype code.
	const unsigned GEN_STUB_BOUND
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x10;
	        // Last stub subtype code + 1.
	const unsigned GEN_ILLEGAL
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x10;
		// First illegal subtype code.
	const unsigned GEN_ILLEGAL_BOUND
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x19;
		// Largest illegal subtype code + 1.
	const unsigned GEN_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x19;
		// First aux subtype code.
	const unsigned GEN_INDIRECT_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x19;
	const unsigned GEN_SUBLIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1A;
	const unsigned GEN_LIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1B;
	const unsigned GEN_AUX_BOUND
		// Largest aux subtype code + 1.
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1C;
	const unsigned GEN_DIRECT_STR
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1C;
	const unsigned GEN_INDEX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1D;
	const unsigned GEN_CONTROL_CODE
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1E;
	const unsigned GEN_SPECIAL
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
#       if MIN_IS_COMPACT
	    typedef int32 intgen;
		// Signed type convertable to a min::gen
		// value (used to convert direct
		// integers).
#	endif
	const unsgen AOFFSET =
		 ( (unsgen) GEN_STUB << VSIZE );
	    // Offset of a stub address packed into
	    // a general value.
    }

    class gen;
    namespace unprotected {
        min::gen new_gen ( min::unsgen value );
	min::unsgen value_of ( min::gen g );
    }

    class gen
    {

    public:

        gen ( void ) : value
	    ( ( unsgen ( GEN_ILLEGAL ) << VSIZE ) ) {}
		

    private:

        min::unsgen value;

        friend min::gen min::unprotected::new_gen
	    ( min::unsgen value );
	friend min::unsgen min::unprotected::value_of
	    ( min::gen g );
    };

    namespace unprotected {

        inline min::gen new_gen ( min::unsgen value )
	{
	    min::gen result;
	    result.value = value;
	    return result;
	}

	inline min::unsgen value_of ( min::gen g )
	{
	    return g.value;
	}
    }

    namespace unprotected {

	inline min::gen new_special_gen
		( min::unsgen subcode )
	{
	    return unprotected::new_gen
		(   ( unsgen ( GEN_SPECIAL ) << VSIZE )
		  + subcode );
	}

    }

    // MIN special values must have indices in the
    // range 2**24 - 256 .. 2**24 - 1.
    //
    inline min::gen MISSING ( void )
	{ return min::unprotected::new_special_gen
	    ( 0xFFFFFF ); }
    inline min::gen NONE ( void )
	{ return min::unprotected::new_special_gen
	    ( 0xFFFFFE ); }
    inline min::gen ANY ( void )
	{ return min::unprotected::new_special_gen
	    ( 0xFFFFFD ); }
    inline min::gen MULTI_VALUED ( void )
	{ return min::unprotected::new_special_gen
	    ( 0xFFFFFC ); }
    inline min::gen UNDEFINED ( void )
	{ return min::unprotected::new_special_gen
	    ( 0xFFFFFB ); }
    inline min::gen SUCCESS ( void )
	{ return min::unprotected::new_special_gen
	    ( 0xFFFFFA ); }
    inline min::gen FAILURE ( void )
	{ return min::unprotected::new_special_gen
	    ( 0xFFFFF9 ); }
    inline min::gen ERROR ( void )
	{ return min::unprotected::new_special_gen
	    ( 0xFFFFF8 ); }
}

inline bool operator == ( min::gen g1, min::gen g2 )
{
    return min::unprotected::value_of ( g1 )
	   ==
	   min::unprotected::value_of ( g2 );
}

inline bool operator != ( min::gen g1, min::gen g2 )
{
    return min::unprotected::value_of ( g1 )
	   !=
	   min::unprotected::value_of ( g2 );
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

    const int OBJ			= 8;
    const int OBJ_MASK			= -8;

	const int TINY_OBJ		= OBJ + 0;
	const int SHORT_OBJ		= OBJ + 1;
	const int LONG_OBJ		= OBJ + 2;
	const int HUGE_OBJ		= OBJ + 3;

    const int PACKED			= 12;
    const int PACKED_MASK		= -4;

	const int PACKED_STRUCT		= PACKED + 0;
	const int PACKED_VEC		= PACKED + 1;

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
	const min::uns64 STUB_PTR = uns64(1) << 55;
	    // Indicates value part of control value is
	    // a packed stub address.

	// The flags
	//    ( 1ULL << ( 56 - MIN_ACC_FLAG_BITS ) )
	//    ( 1ULL << ( 56 - MIN_ACC_FLAG_BITS ) + 1 )
	// are shared between acc and non-acc control
	// values.
    }

    struct stub
    {
	union {
	    min::unsgen g;
	    min::float64 f64;
	    min::uns64 u64;
	    min::int64 i64;
	    min::uns32 u32[2];
	    min::uns16 u16[4];
	    min::uns8 u8[8];
	    char c8[8];
	} v; // value

	union {
	    min::uns64 u64;
	    min::int64 i64;
	    min::uns32 u32[2];
	    min::int8 i8[8];
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
	return (void *) (unsptr) v;
    }
    inline min::uns64 ptr_to_uns64 ( void * p )
    {
	return (uns64) (unsptr) p;
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
    // unsgen value that equals the min::gen value.

    inline min::stub * unsgen_to_stub
	    ( min::unsgen v )
    {
	unsptr p = (unsptr) ( v - AOFFSET );
#           if      MIN_MAX_ABSOLUTE_STUB_ADDRESS \
	         <= MIN_AMAX
	    return (min::stub *) p;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
	         <= MIN_AMAX
	    return (min::stub *) ( p + stub_base );
#           elif MIN_MAX_STUB_INDEX <= MIN_AMAX
	    return (min::stub *) stub_base + p;
#           else
#	        error MIN_MAX_STUB_INDEX > MIN_AMAX
#           endif
    }
    inline min::unsgen stub_to_unsgen
	    ( const min::stub * s )
    {
	unsgen v;
#           if      MIN_MAX_ABSOLUTE_STUB_ADDRESS \
	         <= MIN_AMAX
	    v = (unsptr) s;
#           elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
	         <= MIN_AMAX
	    v = (unsptr) s - stub_base;
#           elif MIN_MAX_STUB_INDEX <= MIN_AMAX
	    v = s - (min::stub *) stub_base;
#           else
#	        error MIN_MAX_STUB_INDEX > MIN_AMAX
#           endif
	return v + AOFFSET;
    }
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

    inline min::gen new_stub_gen
	    ( const min::stub * s )
    {
	return unprotected::new_gen
	    ( internal::stub_to_unsgen ( s ) );
    }

#   if MIN_IS_COMPACT
	inline min::gen new_direct_int_gen ( int v )
	{
	    return unprotected::new_gen
	           ( (unsgen) (internal::intgen) v );
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
	    return unprotected::new_gen ( v );
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
	    return unprotected::new_gen ( v );
	}

#   elif MIN_IS_LOOSE

	// Unimplemented for LOOSE:
	//   min::gen new_direct_int_gen ( int v )
	inline min::gen new_direct_float_gen
		( float64 v )
	{
	    return unprotected::new_gen
	        ( * (unsgen *) & v );
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
	    return unprotected::new_gen ( v );
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
	    return unprotected::new_gen ( v );
	}
#   endif

    inline min::gen new_list_aux_gen ( unsgen p )
    {
	return unprotected::new_gen
	       (   p
		 + ( (unsgen) GEN_LIST_AUX
		     << VSIZE ) );
    }
    inline min::gen new_sublist_aux_gen
	    ( unsgen p )
    {
	return unprotected::new_gen
	       (   p
		 + ( (unsgen) GEN_SUBLIST_AUX
		     << VSIZE ) );
    }
    inline min::gen new_indirect_aux_gen
	    ( unsgen p )
    {
	return unprotected::new_gen
	       (   p
		 + ( (unsgen) GEN_INDIRECT_AUX
		     << VSIZE ) );
    }
    inline min::gen new_index_gen ( unsgen i )
    {
	return unprotected::new_gen
	       ( i + ( (unsgen) GEN_INDEX << VSIZE ) );
    }
    inline min::gen new_control_code_gen
	    ( unsgen c )
    {
	return unprotected::new_gen
	       (   c
		 + ( (unsgen) GEN_CONTROL_CODE
		     << VSIZE ) );
    }
    // min::gen unprotected::new_special_gen is defined
    // above.
    inline min::gen renew_gen
	    ( min::gen v, min::unsgen p )
    {
	return unprotected::new_gen
	    (   (   unprotected::value_of ( v )
	          & ~ internal::VMASK )
	      + p );
    }
} }

namespace min {

    // min:: constructors

    inline min::gen new_stub_gen ( const min::stub * s )
    {
	return unprotected::new_stub_gen ( s );
    }
#   if MIN_IS_COMPACT
	inline min::gen new_direct_int_gen ( int v )
	{
	    MIN_ASSERT (    GEN_MIN_INT <= v
	                 && v <= GEN_MAX_INT,
			 "argument is not direct"
			 " integer" );
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
	    MIN_ASSERT ( strlen ( p ) <= 3,
	                 "string argument too long" );
#	elif MIN_IS_LOOSE
	    MIN_ASSERT ( strlen ( p ) <= 5,
	                 "string argument too long" );
#	endif
	return unprotected::new_direct_str_gen ( p );
    }
    inline min::gen new_direct_str_gen
	    ( const char * p, min::unsptr n )
    {
#       if MIN_IS_COMPACT
	    MIN_ASSERT
	        ( internal::strnlen ( p, n ) <= 3,
		  "string argument too long" );
#	elif MIN_IS_LOOSE
	    MIN_ASSERT
	        ( internal::strnlen ( p, n ) <= 5,
		  "string argument too long" );
#	endif
	return unprotected::new_direct_str_gen ( p, n );
    }
    inline min::gen new_list_aux_gen ( unsgen p )
    {
	MIN_ASSERT ( p < (unsgen) 1 << VSIZE,
	             "argument too large" );
	return unprotected::new_list_aux_gen ( p );
    }
    inline min::gen new_sublist_aux_gen
	    ( unsgen p )
    {
	MIN_ASSERT ( p < (unsgen) 1 << VSIZE,
	             "argument too large" );
	return unprotected::new_sublist_aux_gen ( p );
    }
    inline min::gen new_indirect_aux_gen
	    ( unsgen p )
    {
	MIN_ASSERT ( p < (unsgen) 1 << VSIZE,
	             "argument too large" );
	return unprotected::new_indirect_aux_gen
			( p );
    }
    inline min::gen new_index_gen ( unsgen i )
    {
	MIN_ASSERT ( i < (unsgen) 1 << VSIZE,
	             "argument too large" );
	return unprotected::new_index_gen ( i );
    }
    inline min::gen new_control_code_gen
	    ( unsgen c )
    {
	MIN_ASSERT ( c < (unsgen) 1 << VSIZE,
	             "argument too large" );
	return unprotected::new_control_code_gen ( c );
    }
    inline min::gen new_special_gen
	    ( unsgen i )
    {
	MIN_ASSERT ( i < (unsgen) 1 << VSIZE,
	             "argument too large" );
	return unprotected::new_special_gen ( i );
    }
}

// General Value Test Functions
// ------- ----- ---- ---------

namespace min {

    inline bool is_stub ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return
	    ( (unsgen) GEN_STUB << VSIZE ) <= v
	    &&
	    v < ( (unsgen) GEN_STUB_BOUND << VSIZE );
    }

    inline bool is_aux ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return
	    ( (unsgen) GEN_AUX << VSIZE ) <= v
	    &&
	    v < ( (unsgen) GEN_AUX_BOUND << VSIZE );
    }

#   if MIN_IS_COMPACT
	// Unimplemented for COMPACT:
	//  bool is_direct_float ( min::gen g )
	inline bool is_direct_int ( min::gen g )
	{
	    internal::intgen v =
	        (internal::intgen)
		unprotected::value_of ( g );
	    return GEN_MIN_INT <= v && v <= GEN_MAX_INT;
	}
	inline unsigned gen_subtype_of ( min::gen g )
	{
	    unsgen v = unprotected::value_of ( g );
	    v >>= VSIZE;
	    if ( v < GEN_ILLEGAL )
	        return v;
	    else if ( v < GEN_ILLEGAL_BOUND )
	        return GEN_ILLEGAL;
	    else if ( v < GEN_STUB )
	        return v;
	    else if ( v < GEN_STUB_BOUND )
	        return GEN_STUB;
	    else
	        return GEN_DIRECT_INT;
	}
#   elif MIN_IS_LOOSE
	inline bool is_direct_float ( min::gen g )
	{
	    unsgen v = unprotected::value_of ( g );

	    // Low order 45 bits and high order bit
	    // are masked for this test.
	    //
	    return
	        ( v & 0x7FFFE00000000000ull )
	        != ( (uns64) MIN_FLOAT64_SIGNALLING_NAN
		     << VSIZE );
	}
	// Unimplemented for LOOSE:
	//   bool is_direct_int ( min::gen g )
	inline unsigned gen_subtype_of ( min::gen g )
	{
	    unsgen v = unprotected::value_of ( g );
	    v >>= VSIZE;
	    if ( v < GEN_STUB )
	        return GEN_DIRECT_FLOAT;
	    else if ( v < GEN_STUB_BOUND )
	        return GEN_STUB;
	    else if ( v < GEN_ILLEGAL_BOUND )
	        return GEN_ILLEGAL;
	    else if ( v <= GEN_UPPER)
	        return v;
	    else
	        return GEN_DIRECT_FLOAT;
	}
#   endif

    inline bool is_direct_str ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v >> VSIZE == GEN_DIRECT_STR;
    }
    inline bool is_list_aux ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v >> VSIZE == GEN_LIST_AUX;
    }
    inline bool is_sublist_aux ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v >> VSIZE == GEN_SUBLIST_AUX;
    }
    inline bool is_indirect_aux ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v >> VSIZE == GEN_INDIRECT_AUX;
    }
    inline bool is_index ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v >> VSIZE == GEN_INDEX;
    }
    inline bool is_control_code ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v >> VSIZE == GEN_CONTROL_CODE;
    }
    inline bool is_special ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v >> VSIZE == GEN_SPECIAL;
    }
}

// General Value Read Functions
// ------- ----- ---- ---------

namespace min { namespace unprotected {

    // MUP:: functions.

    inline min::stub * stub_of ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return internal::unsgen_to_stub ( v );
    }

#   if MIN_IS_COMPACT

	// Unimplemented for COMPACT:
	//   float64 direct_float_of ( min::gen g )
	inline int direct_int_of ( min::gen g )
	{
	    return (internal::intgen)
	           unprotected::value_of ( g );
	}
	inline uns64 direct_str_of ( min::gen g )
	{
	    unsgen v = unprotected::value_of ( g );
#	    if MIN_IS_BIG_ENDIAN
		return ( uns64 ( v ) << TSIZE );
#	    elif MIN_IS_LITTLE_ENDIAN
		return
		    ( uns64 ( v & internal::VMASK ) );
#	    endif
	}
#   elif MIN_IS_LOOSE
	inline float64 direct_float_of ( min::gen g )
	{
	    unsgen v = unprotected::value_of ( g );
	    return * (float64 *) & v;
	}
	// Unimplemented for LOOSE:
	//   int direct_int_of ( min::gen g )
	inline uns64 direct_str_of ( min::gen g )
	{
	    unsgen v = unprotected::value_of ( g );
#	    if MIN_IS_BIG_ENDIAN
		return ( v << TSIZE );
#	    elif MIN_IS_LITTLE_ENDIAN
		return ( v & internal::VMASK );
#	    endif
	}
#   endif

    inline unsgen aux_of ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v & internal::VMASK;
    }
    inline unsgen list_aux_of ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v & internal::VMASK;
    }
    inline unsgen sublist_aux_of ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v & internal::VMASK;
    }
    inline unsgen indirect_aux_of ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v & internal::VMASK;
    }
    inline unsgen index_of ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v & internal::VMASK;
    }
    inline unsgen control_code_of ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v & internal::VMASK;
    }
    inline unsgen special_index_of ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	return v & internal::VMASK;
    }
} }

namespace min {

    // min:: functions

    inline const min::stub * stub_of ( min::gen g )
    {
	if ( is_stub ( g ) )
	    return unprotected::stub_of ( g );
	else
	    return NULL;
    }
#   if MIN_IS_COMPACT
	inline int direct_int_of ( min::gen g )
	{
	    MIN_ASSERT ( is_direct_int ( g ),
	                 "argument is not direct"
			 " integer" );
	    return unprotected::direct_int_of ( g );
	}
#   elif MIN_IS_LOOSE
	inline float64 direct_float_of ( min::gen g )
	{
	    MIN_ASSERT ( is_direct_float ( g ),
	                 "argument is not direct"
			 " float" );
	    return unprotected::direct_float_of ( g );
	}
#   endif
    inline uns64 direct_str_of ( min::gen g )
    {
	MIN_ASSERT ( is_direct_str ( g ),
	             "argument is not direct"
		     " string" );
	return unprotected::direct_str_of ( g );
    }
    inline unsgen list_aux_of ( min::gen g )
    {
	MIN_ASSERT ( is_list_aux ( g ),
	             "argument is not list aux" );
	return unprotected::list_aux_of ( g );
    }
    inline unsgen sublist_aux_of ( min::gen g )
    {
	MIN_ASSERT ( is_sublist_aux ( g ),
	             "argument is not sublist aux" );
	return unprotected::sublist_aux_of ( g );
    }
    inline unsgen indirect_aux_of ( min::gen g )
    {
	MIN_ASSERT ( is_indirect_aux ( g ),
	             "argument is not indirect aux" );
	return unprotected::indirect_aux_of ( g );
    }
    inline unsgen index_of ( min::gen g )
    {
	MIN_ASSERT ( is_index ( g ),
	             "argument is not index" );
	return unprotected::index_of ( g );
    }
    inline unsgen control_code_of ( min::gen g )
    {
	MIN_ASSERT ( is_control_code ( g ),
	             "argument is not control code" );
	return unprotected::control_code_of ( g );
    }
    inline unsgen special_index_of ( min::gen g )
    {
	MIN_ASSERT ( is_special ( g ),
	             "argument is not special" );
	return unprotected::special_index_of ( g );
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
	return int ( int64 ( c ) >> 56 );
    }

    inline int locator_of_control ( min::uns64 c )
    {
	return int ( int64 ( c ) >> 48 );
    }

    inline min::unsptr value_of_control ( min::uns64 c )
    {
	return unsptr ( c & MIN_CONTROL_VALUE_MASK );
    }

    inline min::uns64 new_control_with_type
	    ( int type_code, min::uns64 v,
	      min::uns64 flags = 0 )
    {
	return ( uns64 ( type_code ) << 56 )
	       |
	       v
	       |
	       flags;
    }

    inline min::uns64 new_control_with_locator
	    ( int locator, min::uns64 v )
    {
	return ( uns64 ( locator ) << 48 )
	       |
	       v;
    }

    inline min::uns64 renew_control_type
	    ( min::uns64 c, int type )
    {
	return ( c & ~ internal::TYPE_MASK )
	       | ( uns64 (type) << 56 );
    }

    inline min::uns64 renew_control_locator
	    ( min::uns64 c, int locator )
    {
	return ( c & ~ internal::LOCATOR_MASK )
	       | ( uns64 (locator) << 48 );
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
	           (unsptr)
	           (c & MIN_CONTROL_VALUE_MASK );
	}

	inline min::uns64 new_control_with_type
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( uns64 ( type_code ) << 56 )
		   |
		   (unsptr) s
		   |
		   flags;
	}

	inline min::uns64 new_control_with_locator
		( int locator, const min::stub * s )
	{
	    return ( uns64 ( locator ) << 48 )
		   |
		   (unsptr) s;
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return ( c & ~ MIN_CONTROL_VALUE_MASK )
		   | (unsptr) s;
	}

#   elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
         <= MIN_CONTROL_VALUE_MASK

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    unsptr p =
	        (unsptr) (c & MIN_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           ( p + internal::stub_base );
	}

	inline min::uns64 new_control_with_type
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( uns64 ( type_code ) << 56 )
		   |
	           ( (unsptr) s - internal::stub_base )
		   |
		   flags;
	}

	inline min::uns64 new_control_with_locator
		( int locator, const min::stub * s )
	{
	    return ( uns64 ( locator ) << 48 )
		   |
	           ( (unsptr) s - internal::stub_base );
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return ( c & ~ MIN_CONTROL_VALUE_MASK )
		   |
	           ( (unsptr) s - internal::stub_base );
	}

#   elif    MIN_MAX_STUB_INDEX \
         <= MIN_CONTROL_VALUE_MASK

	inline min::stub * stub_of_control
		( min::uns64 c )
	{
	    unsptr p =
	       (unsptr) (c & MIN_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           internal::stub_base + p;
	}

	inline min::uns64 new_control_with_type
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return   ( uns64 ( type_code ) << 56 )
	           | ( s - (min::stub *)
		           internal::stub_base )
		   | flags;
	}

	inline min::uns64 new_control_with_locator
		( int locator, const min::stub * s )
	{
	    return   ( uns64 ( locator ) << 48 )
	           | ( s - (min::stub *)
		           internal::stub_base );
	}

	inline min::uns64 renew_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return   ( c & ~ MIN_CONTROL_VALUE_MASK )
	           | ( s - (min::stub *)
		           internal::stub_base );
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
	           (unsptr)
	           (c & MIN_ACC_CONTROL_VALUE_MASK );
	}

	inline min::uns64 new_acc_control
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return ( uns64 ( type_code ) << 56 )
		   |
		   (unsptr) s
		   |
		   flags;
	}

	inline min::uns64 renew_acc_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return ( c & ~ MIN_ACC_CONTROL_VALUE_MASK )
		   | (unsptr) s;
	}

#   elif    MIN_MAX_RELATIVE_STUB_ADDRESS \
         <= MIN_ACC_CONTROL_VALUE_MASK

	inline min::stub * stub_of_acc_control
		( min::uns64 c )
	{
	    unsptr p =
	       (unsptr)
	       (c & MIN_ACC_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           ( p + internal::stub_base );
	}

	inline min::uns64 new_acc_control
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return   ( uns64 ( type_code ) << 56 )
		   | (   (unsptr) s
	               - internal::stub_base )
		   | flags;
	}

	inline min::uns64 renew_acc_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return ( c & ~ MIN_ACC_CONTROL_VALUE_MASK )
		 | (   (unsptr) s
		     - internal::stub_base );
	}

#   elif    MIN_MAX_STUB_INDEX \
         <= MIN_ACC_CONTROL_VALUE_MASK

	inline min::stub * stub_of_acc_control
		( min::uns64 c )
	{
	    unsptr p =
	       (unsptr)
	       (c & MIN_ACC_CONTROL_VALUE_MASK );
	    return (min::stub *)
	           internal::stub_base + p;
	}

	inline min::uns64 new_acc_control
		( int type_code, const min::stub * s,
		  min::uns64 flags = 0 )
	{
	    return   ( uns64 ( type_code ) << 56 )
	           | ( s - (min::stub *)
		           internal::stub_base )
		   | flags;
	}

	inline min::uns64 renew_acc_control_stub
		( min::uns64 c, const min::stub * s )
	{
	    return ( c & ~ MIN_ACC_CONTROL_VALUE_MASK )
	         | ( s - (min::stub *)
		         internal::stub_base );
	}
#   else
#	error   MIN_MAX_STUB_INDEX \
              > MIN_ACC_CONTROL_VALUE_MASK
#   endif

} }

// Stub Functions
// ---- ---------

namespace min {

    namespace unprotected {

	inline int type_of ( const min::stub * s )
	{
	    return s->c.i8[7*MIN_IS_LITTLE_ENDIAN];
	}

    }

    inline int type_of ( const min::stub * s )
    {
	if ( s == NULL )
	    return 0;
	else
	    return s->c.i8[7*MIN_IS_LITTLE_ENDIAN];
    }

    inline int type_of ( min::gen g )
    {
        if ( is_stub ( g ) )
	    return unprotected::type_of
	        ( unprotected::stub_of ( g ) );
	else
	    return 0;
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
	    return unprotected::new_gen ( s->v.g );
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
	    s->v.g = unprotected::value_of ( v );
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

// Initializer Interface
// ----------- ---------

namespace min {

    struct initializer;

    namespace internal {

        extern min::initializer * last_initializer;

	extern bool initialization_done;

	void initialize ( void );

    }

    struct initializer
    {
        void (*init) ( void );
	min::initializer * previous;

	initializer ( void (*init) ( void ) )
	    : init ( init )
	{
	    previous = internal::last_initializer;
	    internal::last_initializer = this;
	}
	~ initializer ( void )
	{
	    MIN_REQUIRE (    internal::last_initializer
	                  == this );
	    internal::last_initializer = previous;
	}
    };

    inline void initialize ( void )
    {
        if ( ! internal::initialization_done )
	    internal::initialize();
    }
}

// Process Interface
// ------- ---------

// This interface includes a process control block
// and functions to test for interrupts.  The process
// control block contains pointers to stacks.

namespace min {

    namespace internal {

	extern min::stub ** acc_stack;
	extern min::stub ** volatile acc_stack_limit;
	    // acc_stack points at the first unused
	    // location in the acc stack.  If
	    // acc_stack >= acc_stack_limit, an
	    // interrupt is invoked.
	    //
	    // On startup, acc_stack == acc_stack_limit
	    // == 0, so the first check for an interrupt
	    // will cause one.
	    //
	    // Most interrupts just perform acc actions
	    // such as emptying the acc_stack.
	    //
	    // Min::stub * values can be pushed into the
	    // acc stack, increasing the acc_stack
	    // value.  The acc_stack_limit is much less
	    // than the actual end of the acc stack.
	    // Interrupts may be scheduled by setting
	    // acc_stack_limit =< acc_stack, and work
	    // that leads to interrupts may be counted
	    // by decreasing acc_stack_limit.

	// Out of line function to execute interrupt.
	// Provided by ACC as most interrupts merely
	// process the ACC stack.  Begins by calling
	// min::initialize() and ends by calling
	// min::thread_interrupt().  Returns true.
	//
	bool acc_interrupt ( void );

	// On if a thread interrupt has been scheduled.
	//
	extern bool thread_interrupt_needed;

	// Call to execute a thread interrupt.
	//
	void thread_interrupt ( void );
    }

    inline void thread_interrupt ( void )
    {
        if ( internal::thread_interrupt_needed )
	    internal::thread_interrupt();
    }

    inline bool interrupt ( void )
    {
        if (    internal::acc_stack
	     >= internal::acc_stack_limit )
	    return internal::acc_interrupt();
	else return false;
    }
}

// Allocator/Collector/Compactor Interface
// ----------------------------- ---------

namespace min {

    const min::stub * const NULL_STUB =
        (const min::stub *) NULL;

    // The C offsetof macro does not work, so:
    //
    template < typename S, typename T >
    min::uns32 OFFSETOF ( T S::* d )
    {
        S * p = (S *) NULL;
	return (uns8 *) & (p->*d) - (uns8 *) p;
    }

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
    // for every kind of atom: the acc hash table for
    // non-ephemeral stubs and the aux hash table for
    // ephemeral stubs.  A stub starts out being ephem-
    // eral, and after surviving a sufficent number of
    // garbage collections, becomes non-ephemeral.
    //
    // The elements of the acc hash table are the stubs
    // hashed, chained together by their stub control
    // word stub pointers.  The stubs listed in this
    // hash table are not in the normal acc list.  So
    // for non-ephemeral stubs, the acc hash tables
    // supplement the acc list, with all non-ephemeral
    // stubs appearing in either the acc list or an acc
    // hash table (but not both).
    //
    // The elements of the aux hash table are HASHTABLE_
    // AUX aux stubs whose values point at the stubs of
    // ephemeral objects hashed.  The stubs hashed in
    // the aux table are also on the acc list.
    //
    // The acc and aux hash tables for a stub type xxx
    // have the same size, xxx_hash_size, which must be
    // a power of 2, and each xxx_hash_mask = xxx_hash_
    // size - 1.  Stubs are put in the list headed by
    // the table element whose index is the stub's hash
    // value masked by the xxx_hash_mask.  Lists are
    // ended by a pointer to MINT::null_stub.
    //
    // When a hashed stub is created it is ephemeral
    // and is put in the aux hash table.  The acc moves
    // a stub from the aux hash table to the correspond-
    // ing acc hash table when the stub ceases to be
    // ephemeral.

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
    //	0		    Fixed Body Flag; indicates
    //			    a stub contains a pointer
    //			    to a body allocated from
    //			    a fixed_block_list.
    //
    //			    This flag is shared between
    //			    acc and non-acc control
    //			    values.
    //
    //	1		    Reserved for future use.
    //
    //			    This flag is shared between
    //			    acc and non-acc control
    //			    values.
    //
    //	2, ..., 2+p-1	    Flag Pair Low Order Bits
    //
    //	2+p, ..., 2+2p-1    Flag Pair High Order Bits
    //
    // p = ACC_FLAG_PAIRS = number of flag pairs
    //			  = ( MIN_ACC_FLAG_BITS - 2 )
    //			    / 2
    //
    // Most the flags are in pairs.  The disposition of
    // these pairs is up to the ACC, but it is required
    // that the mutator put stub pointers s1 and s2 in
    // the acc stack if s2 is stored in the data of s1
    // and for some pair of flags, the low order flag
    // of s2, the high order flag of s1, and the low
    // order flag of the acc_stack_mask global variable
    // are all on.  For example, if the low order flag
    // of a pair means the stub is unmarked and the high
    // order flag means the stub has been scavenged, s1
    // and s2 will be put in the acc stack if a pointer
    // s2 to an unmarked stub is stored in the data of a
    // previously scavenged stub s1 and the acc_stack_
    // mask is enabled for this event.
    //
    const uns64 ACC_FLAG_MASK =
           ( (uns64(1) << MIN_ACC_FLAG_BITS) - 1 )
	<< ( 56 - MIN_ACC_FLAG_BITS );
    const uns64 ACC_FIXED_BODY_FLAG =
	( uns64(1) << ( 56 - MIN_ACC_FLAG_BITS ) );
    const unsigned ACC_FLAG_PAIRS =
        ( MIN_ACC_FLAG_BITS - 2 ) / 2;

    // unprotected::acc_write_update ( s1, s2 ) checks
    // whether
    //
    //		flags of *s2
    //		&
    //		( flags of *s1 >> ACC_FLAG_PAIRS )
    //		&
    //		acc_stack_mask
    //
    // is non-zero, and if so, pushes first s1 and then
    // s2 into the acc_stack.  This call is made when
    // a pointer to s2 is stored in s1 object data (the
    // object stub, body, and any associated auxiliary
    // stubs), and the acc_stack is used by the collec-
    // tor to adjust the marks it makes on objects.
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

    extern const min::stub * ZERO_STUB;

    // Function executed whenever a pointer to stub s2
    // is stored in a datum with stub s1.  s1 is the
    // source of the written pointer and s2 is the
    // target.  But does nothing if s2 == NULL.  Pre-
    // sumes/requires s1 != NULL.
    //
    inline void acc_write_update
	    ( const min::stub * s1,
	      const min::stub * s2 )
    {
        if ( s2 == NULL ) return;
	if ( s1 == ZERO_STUB ) return;
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
	    ( const min::stub * s1, min::gen g )
    {
	if ( is_stub ( g ) )
	    acc_write_update
		( s1, unprotected::stub_of ( g ) );
    }

    // Ditto but does nothing for loose implementation.
    //
    inline void acc_write_num_update
	    ( min::stub * s1, min::gen g )
    {
#	if MIN_IS_COMPACT
	    if ( is_stub ( g ) )
		acc_write_update
		    ( s1, unprotected::stub_of ( g ) );
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
	    min::gen g = * p ++;
	    if ( min::is_stub ( g ) )
	        acc_write_update
		    ( s1, min::unprotected
		             ::stub_of ( g ) );
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

    // Ditto but for a vector of stub pointers.
    //
    inline void acc_write_update
	    ( const min::stub * s1,
	      const min::stub * const * p,
	      min::unsptr n )
    {
        while ( n -- )
	    acc_write_update ( s1, * p ++ );
    }

} }

namespace min {

    // The following is a workaround for the fact that
    // some function specializations are not allowed but
    // the corresponding class specializations are
    // allowed.  So we define a class of no members
    // whose constructors are the functions we desire.
    //
    // For types T that cannot point to stubs, these
    // constructors are no-operations.  For types T that
    // can point to stubs, the constructors call acc_
    // write_update with the arguments given the
    // constructor.

    template < typename T >
    struct write_update
    {
        // For general types T these are nop's.
	//
	write_update ( const min::stub * s, T v ) {}
	write_update ( const min::stub * s,
	               T const * p, min::unsptr n ) {}
    };

    template <>
    struct write_update<const min::stub *>
    {
        write_update
	    ( const min::stub * s, const min::stub * v )
	{
	    unprotected::acc_write_update ( s, v );
	}
	write_update ( const min::stub * s,
	               const min::stub * const * p,
		       min::unsptr n )
	{
	    unprotected::acc_write_update ( s, p, n );
	}
    };

    template <>
    struct write_update<min::gen>
    {
        write_update ( const min::stub * s, min::gen v )
	{
	    unprotected::acc_write_update ( s, v );
	}
	write_update ( const min::stub * s,
	               min::gen const * p,
		       min::unsptr n )
	{
	    unprotected::acc_write_update ( s, p, n );
	}
    };
}

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
    // zero stub control flags, and min::NONE() value.
    //
    // Free stubs can be removed completely from the
    // acc list for use as aux stubs.  These are not
    // garbage collectible.  When freed, aux stubs are
    // put back on the acc list as free stubs.

    // Pointers to the first and last allocated stubs:
    //
    // The first acc list stub is the stub pointed at by
    // the control word of the head_stub (which is not
    // itself on the acc list).  The first free stub on
    // the acc list is pointed at by the control word of
    // the last_allocated_stub.  If there are allocated
    // stubs on the acc list, the last_allocated_stub is
    // the last of these; otherwise it equals head_stub.
    //
    // First_allocated_stub may equal MINT::null_stub
    // for some system configurations.
    //
    // The acc list is MINT::null_stub terminated.  So
    // if there are no free stubs, the control word of
    // last_allocate_stub points at MINT::null_stub.
    //
    extern min::stub * head_stub;
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
	if ( internal::number_of_free_stubs == 0 )
	    internal::acc_expand_stub_free_list ( 1 );

	-- internal::number_of_free_stubs;
	++ unprotected::acc_stubs_allocated;

	uns64 c = unprotected::control_of
		    ( internal::last_allocated_stub );
	min::stub * s =
	    unprotected::stub_of_acc_control ( c );
	unprotected::set_flags_of
	    ( s, internal::new_acc_stub_flags );
	return internal::last_allocated_stub = s;
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
	if ( internal::number_of_free_stubs == 0 )
	    internal::acc_expand_stub_free_list ( 1 );

	-- internal::number_of_free_stubs;
	++ unprotected::aux_stubs_allocated;

	uns64 c = unprotected::control_of
		    ( internal::last_allocated_stub );
	min::stub * s =
	    unprotected::stub_of_acc_control ( c );
	c = unprotected::renew_acc_control_stub
	        ( c, unprotected::stub_of_acc_control
			( unprotected
			     ::control_of ( s ) ) );
	unprotected::set_control_of
	    ( internal::last_allocated_stub, c );
	unprotected::set_type_of ( s, AUX_FREE );
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
        unprotected::set_gen_of ( s, NONE() );
	uns64 c = unprotected::control_of
		( internal::last_allocated_stub );
	min::stub * next =
	    unprotected::stub_of_acc_control ( c );
	unprotected::set_control_of
	    ( s, unprotected::new_acc_control
		  ( ACC_FREE, next ) );
	c = unprotected
	       ::renew_acc_control_stub ( c, s );
	unprotected::set_control_of
		( internal::last_allocated_stub, c );
	++ internal::number_of_free_stubs;
	++ unprotected::aux_stubs_freed;
    }

} }

namespace min { namespace internal {

    // Ditto but for use by acc to put stub it has
    // freed on acc list.  Same as above but does not
    // increment aux_subs_freed.
    //
    inline void free_acc_stub ( min::stub * s )
    {
        unprotected::set_gen_of ( s, NONE() );
	uns64 c = unprotected::control_of
		    ( internal::last_allocated_stub );
	min::stub * next =
	    unprotected::stub_of_acc_control ( c );
	unprotected::set_control_of
	    ( s, unprotected::new_acc_control
		  ( ACC_FREE, next ) );
	c = unprotected
	       ::renew_acc_control_stub ( c, s );
	unprotected::set_control_of
		( internal::last_allocated_stub, c );
	++ internal::number_of_free_stubs;
    }

    // fixed_block_lists[j] heads a free list of size
    // blocks of size 1 << ( j + 3 ), for 2 <= j,
    // (1<<j) <= MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE/8.
    // Each fixed block begins with a control word whose
    // locator is provided by the allocator and whose
    // stub address is set to point to the stub whose
    // body the block contains, if there is such a
    // stub, or equals the address of MINT::null_stub
    // otherwise.  When the block is on the free list,
    // the control word and the word following it are
    // specified by the free_fixed_size_block struct.
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
    const unsigned number_fixed_block_lists =
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
    } fixed_block_lists
          [number_fixed_block_lists];

    // Out of line allocators.  Only called by min::
    // new_body.
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
        // A power of 2 >= min_fixed block size and <=
	// 1 << MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG.

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

    // Return the optimal body size equal to or greater
    // than a given body size, from the point of view
    // of a resizing that increases the size of a body.
    // Thus if given 83 this returns 120, it means that
    // any newly allocated body with size larger than 83
    // bytes will in fact consume 120 bytes, so one
    // might as well ask for 120 bytes.  If this is
    // true, it is likely because the allocator uses
    // lists of fixed size places to put bodies (the
    // compactor may later squeeze the bodies into a
    // smaller area).
    //
    inline min::unsptr optimal_body_size
        ( min::unsptr size )
    {
	unsptr m = size + 2 * sizeof ( uns64 ) - 1;
	m &= ~ ( sizeof ( uns64 ) - 1 );

        if ( m < internal::min_fixed_block_size )
            m = internal::min_fixed_block_size;
	else if ( m <= internal::max_fixed_block_size )
	    m = (min::unsptr) 1
		<< 
		( internal::log2floor
		      ( (unsigned) m - 1 ) + 1 );
	return m - sizeof ( uns64 );
    }

    // Allocate a body to a stub.  n is the minimum size
    // of the body in bytes, not including the control
    // word that begins the body.  The ACC_FIXED_BODY_
    // flag of the stub control is set; otherwise the
    // stub control is left untouched.  The stub may be
    // either an acc or non-acc stub.
    //
    inline void new_body
            ( min::stub * s, min::unsptr n )
    {
	unsptr m = n + sizeof ( uns64);

        if ( m < internal::min_fixed_block_size )
            m = internal::min_fixed_block_size;

	if ( m > internal::max_fixed_block_size )
	{
	     internal::new_non_fixed_body ( s, n );
	     return;
	}

	// See min_parameters.h for log2floor.
	//
	m = m - 1;
	internal::fixed_block_list * fbl =
		  internal::fixed_block_lists
		+ internal::log2floor ( m )
		+ 1 - 3;

	if ( fbl->count == 0 )
	{
	     internal::new_fixed_body
	          ( s, n, fbl );
	     return;
	}

	internal::free_fixed_size_block * b =
	    fbl->last_free->next;

	fbl->last_free->next = b->next;
	if ( -- fbl->count == 0 )
	    fbl->last_free = NULL;

	b->block_control =
	    unprotected::renew_control_stub
	        ( b->block_control, s );
	unprotected::set_ptr_of
	    ( s, & b->block_control + 1 );
	unprotected::set_flags_of
	       ( s, internal::ACC_FIXED_BODY_FLAG );
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
    struct resize_body {

	// Construct resize_body for stub s with new
	// body size new_size and old body size
	// old_size.
	//
        resize_body ( min::stub * s,
	              min::unsptr new_size,
		      min::unsptr old_size )
	    : s ( s ),
	      old_size ( old_size ),
	      new_size ( new_size ),
	      new_type ( min::type_of ( s ) )
	{
	    // Allocate rstub and its body, which is
	    // the new body.

	    rstub = unprotected::new_aux_stub();

	    unprotected::set_type_of
		( rstub, RELOCATE_BODY );
	    unprotected::new_body
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
	    if (    unprotected::type_of ( rstub )
	         != DEALLOCATED )
	    {
		uns64 v = unprotected::value_of ( s );
		uns64 rv =
		    unprotected::value_of ( rstub );
		unprotected::set_value_of ( s, rv );
		unprotected::set_value_of ( rstub, v );

		uns64 c =
		    ( unprotected::control_of ( rstub )
		      ^
		      unprotected::control_of ( s ) )
		    &
		    internal::ACC_FIXED_BODY_FLAG;
		rstub->c.u64 ^= c;
		s->c.u64 ^= c;

		int type = unprotected::type_of ( s );
		unprotected::set_type_of
		    ( rstub, type );
		unprotected::set_type_of
		    ( s, new_type );

		uns64 * bp = (uns64 *)
		    unprotected::ptr_of ( s ) - 1;
		* bp = unprotected::renew_control_stub
			 ( * bp, s );
		bp = (uns64 *)
		    unprotected::ptr_of ( rstub )
		    - 1;
		* bp = unprotected::renew_control_stub
			 ( * bp, rstub );

		unprotected::deallocate_body
		    ( rstub, old_size );
	    }

	    unprotected::free_aux_stub ( rstub );
	}

	friend void * & new_body_ptr_ref
			( resize_body & r );
	friend void abort_resize_body
			( resize_body & r );
	friend void retype_resize_body
			( resize_body & r,
			  int new_type );

    private:

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
    };

    // Return a pointer to the new body.
    //
    inline void * & new_body_ptr_ref
	    ( resize_body & r )
    {
	return unprotected::ptr_ref_of ( r.rstub );
    }
    // Void the resize_body struct so it will not
    // change anything when deconstructed.  The
    // new body is deallocated.
    //
    inline void abort_resize_body ( resize_body & r )
    {
	unprotected::deallocate_body
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

namespace min {

    class stub_ptr;

    template < typename T >
    class locatable_var;

    typedef locatable_var<min::gen>
        locatable_gen;
    typedef locatable_var<min::stub_ptr>
        locatable_stub_ptr;
}

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
    // Lastly, if sc.state is set to sc.RESTART, the
    // scavenging of s1 will be restarted by recalling
    // the scavenger routine with sc.state == 0.
    // sc.state is set to sc.RESTART by the scavenge_
    // restart function if that function finds that a
    // stub is being scavenged.
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
	    // RESTART (see above).  This is adequate
	    // for small data.

	static const min::uns64 RESTART =
	    min::uns64 ( min::int64 ( -1 ) );

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

	min::uns64 thread_state;
	   // State of the thread_scavenge_routine.
	   // 0 to start from beginning, non-0 to
	   // continue.

	min::locatable_stub_ptr * locatable_var_last;
	min::locatable_gen * locatable_gen_last;
	   // Pointers to locatable variables from
	   // which to resume if thread_state != 0.
  
    };

    // Function to scavenge the static and thread loca-
    // table_gen, locatable_num_gen, and locatable_stub_
    // ptr structures, finding all pointers therein to
    // acc stubs s2.  For each s2 found, a particular
    // flag of s2, designated by sc.stub_flag, is
    // checked to see if it is on.  If it is, it is
    // turned off, and if the s2 stub is scavengable, a
    // pointer to s2 is pushed onto the to_be_scavenged
    // stack.
    //
    // This is known as thread scavenging.
    //
    // Note that s2 is scavengable if and only if
    // scavenger_rountines[type_of(s2)] != NULL.
    //
    // Thread scavenging can be interrupted by scaveng-
    // ing stubs from the to_be_scavenged stack (non-
    // thread scavenging).  To accomplish this, thread
    // scavenging uses the sc.thread_state instead of
    // sc.state as its state, and has separate variables
    // in sc to store supplementary state.  Set
    // sc.thread_state to 0 before calling this routine
    // to restart thread scavenging.  Upon return by
    // this routine, sc.thread_state will be 0 if thread
    // scavenging is finished, and non-zero if thread
    // scavenging needs to be continued or restarted.
    // It must be restarted by setting sc.thread_state
    // to 0 if the mutator runs before thread scavenging
    // can continue.
    //
    // This function updates sc.gen_count, sc.stub_
    // count, and sc.stub_flag_accumulator, and returns
    // with non-zero sc.thread_state if gen_count equals
    // or exceeds gen_limit.
    //
    void thread_scavenger_routine
        ( scavenge_control & sc );

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

    // If s is being scavenged, restart the scavenging
    // of s.  This should be done whenever s is being
    // reorganized.
    //
    inline void restart_scavenging ( min::stub * s1 )
    {
        for ( scavenge_control * sc = scavenge_controls;
	      sc <   scavenge_controls
	           + number_of_acc_levels;
	      ++ sc )
	{
	    if ( sc->state != 0 && sc->s1 == s1 )
		sc->state = sc->RESTART;
	}
    }

} }

// References
// ----------

namespace min {

    template < typename T >
    class ref;

    namespace unprotected {

	template < typename T >
	min::ref<T> new_ref
	    ( const min::stub * s, T const & location );

	template < typename T >
	min::ref<T> new_ref
	    ( const min::stub * s, min::unsptr offset );
    }

    template < typename T >
    class ref
    {

    public:

	const min::stub * const s;
	const min::unsptr offset;

	// We must define an explicit copy assignment
	// operator which copies the values referenced
	// and not the references.
	//
	ref<T> const & operator =
		( const ref<T> & r ) const
	{
	    T value = r;
	    * this->location() = value;
	    write_update<T> X ( this->s, value );
	    return * this;
	}

	ref<T> const & operator =
		( T const & value ) const
	{
	    T v = value;
	    * this->location() = v;
	    write_update<T> X ( this->s, v );
	    return * this;
	}

	operator T ( void ) const
	{
	    return * location();
	}

	T operator -> ( void ) const
	{
	    return * location();
	}

    private:

        ref ( const min::stub * s, min::unsptr offset )
	    : s ( s ), offset ( offset ) {}

	T * location ( void ) const
	{
	    return (T *)
		(   (uns8 *)
		    unprotected::ptr_of ( s )
		  + offset );
	}

	// Making this private prohibits uninitialized
	// references.
	//
        ref ( void )
	    : s ( NULL ), offset ( 0 ) {}

	friend min::ref<T> unprotected::new_ref<T>
	    ( const min::stub * s, T const & location );

	friend min::ref<T> unprotected::new_ref<T>
	    ( const min::stub * s, min::unsptr offset );
    };

    template < typename T >
    inline min::ref<T> unprotected::new_ref
        ( const min::stub * s, min::unsptr offset )
    {
        return min::ref<T> ( s, offset );
    }

    template < typename T >
    inline min::ref<T> unprotected::new_ref
        ( const min::stub * s, T const & location )
    {
        return min::ref<T>
	    ( s, (uns8 *) & location
		 -
		 (uns8 *) unprotected::ptr_of ( s ) );
    }

    template < typename T >
    inline min::ref<T> new_ref ( T & location )
    {
	return unprotected::new_ref<T>
	    ( unprotected::ZERO_STUB, location );
    }
}

// r == v will not automatically convert r to type v,
// so:
//
template < typename T >
inline bool operator == ( const min::ref<T> & r, T v )
{
    return (T) r == v;
}

template < typename T >
inline bool operator == ( T v, const min::ref<T> & r )
{
    return v == (T) r;
}
template < typename T >
inline bool operator != ( const min::ref<T> & r, T v )
{
    return (T) r != v;
}

template < typename T >
inline bool operator != ( T v, const min::ref<T> & r )
{
    return v != (T) r;
}

# define MIN_REF(type,name,ctype) \
    inline min::ref< type > name##_ref \
               ( ctype container ) \
    { \
        return min::unprotected::new_ref \
	    ( container, container->name ); \
    }

// Pointers
// --------

namespace min {

    template < typename T >
    class ptr;

    namespace unprotected {

	template < typename T >
	min::ptr<T> new_ptr
	    ( const min::stub * s, T * location );

	template < typename T >
	min::ptr<T> new_ptr
	    ( const min::stub * s, min::unsptr offset );
    }

    template < typename T >
    class ptr
    {

    public:

	const min::stub * const s;
	const min::unsptr offset;

        ptr ( void )
	    : s ( NULL ), offset ( 0 ) {}

        // Permit unprotected conversions of pointers.
	//
	template <typename S>
        ptr ( const ptr<S> & p )
	    : s ( p.s ), offset ( p.offset ) {}

	// Because s and offset are consts, we must
	// provide an explicit copy assignment operator.
	//
	ptr<T> & operator = ( const ptr<T> & p )
	{
	    new ( this ) ptr<T> ( p.s, p.offset );
	    return * this;
	}

	T * operator -> ( void ) const
	{
	    return location();
	}

	T * operator ! ( void ) const
	{
	    return location();
	}

	template <typename I> ref<T> operator []
		( I index ) const
	{
	    return unprotected::new_ref<T>
		( this->s,
		  this->offset +   sizeof ( T )
				 * index );
	}

	template <typename I> ptr<T> operator +
		( I index ) const
	{
	    return unprotected::new_ptr<T>
		( this->s,
		  this->offset +   sizeof ( T )
				 * index );
	}

    private:

        ptr ( const min::stub * s, min::unsptr offset )
	    : s ( s ), offset ( offset ) {}

	T * location ( void ) const
	{
	    return (T *)
		(   (uns8 *)
		    unprotected::ptr_of ( s )
		  + offset );
	}

	friend min::ptr<T> unprotected::new_ptr<T>
	    ( const min::stub * s, T * location );
	friend min::ptr<T> unprotected::new_ptr<T>
	    ( const min::stub * s, min::unsptr offset );
    };

}

template < typename T >
inline bool operator ==
    ( const min::ptr<T> & p1, const min::ptr<T> & p2 )
{
    return p1.s == p2.s
	   &&
	   p1.offset == p2.offset;
}

template < typename T >
inline bool operator !=
    ( const min::ptr<T> & p1, const min::ptr<T> & p2 )
{
    return p1.s != p2.s
	   ||
	   p1.offset != p2.offset;
}

template < typename T >
inline bool operator <
    ( const min::ptr<T> & p1, const min::ptr<T> & p2 )
{
    return p1.s == p2.s
	   &&
	   p1.offset < p2.offset;
}

template < typename T >
inline min::ptr<T> operator ++
	( min::ptr<T> & p, int )
{
    min::ptr<T> result = p;
    * (min::unsptr *) & p.offset += sizeof ( T );
	// Overrides p.offset being read-only.
    return result;
}

template < typename T >
inline min::ptr<T> operator -- ( min::ptr<T> & p )
{
    * (min::unsptr *) & p.offset -= sizeof ( T );
	// Overrides p.offset being read-only.
    return p;
}

namespace min {
 
    template < typename T >
    inline min::ptr<T> unprotected::new_ptr
        ( const min::stub * s, min::unsptr offset )
    {
        return min::ptr<T> ( s, offset );
    }
 
    template < typename T >
    inline min::ptr<T> unprotected::new_ptr
        ( const min::stub * s, T * location )
    {
        return min::ptr<T>
	    ( s, (uns8 *) location
		 -
		 (uns8 *) unprotected::ptr_of ( s ) );
    }

    template < typename T >
    inline min::ptr<T> new_ptr ( T * p )
    {
	return unprotected::new_ptr<T>
	    ( unprotected::ZERO_STUB, p );
    }

    template < typename T >
    inline min::ptr<T> operator &
	    ( const min::ref<T> & r )
    {
        return unprotected::new_ptr<T>
	    ( r.s, r.offset );
    }
    template < typename T >
    inline min::ref<T> operator *
	    ( const min::ptr<T> & p )
    {
	return unprotected::new_ref<T>
	    ( p.s, p.offset );
    }
}

namespace min {

    template < typename T >
    inline void min_stack_copy
        ( T * destination, T const * source,
	  min::unsptr length )
    {
        ::memcpy ( destination, source,
	           (sizeof (T)) * length );
    }

#   define MIN_STACK_COPY(T,name,length,source) \
	T name[length]; \
	min::min_stack_copy \
	    ( name, ! (source), (length) )
}

// Locatable Variables
// --------- ---------

namespace min {

    // See declarations of locatable_... above.

    class stub_ptr
    {

    private:

	const min::stub * value;

    public:

	stub_ptr ( const min::stub * const & s )
	    : value ( s ) {}

	stub_ptr ( void )
	    : value ( NULL ) {}

	stub_ptr & operator =
		( const min::stub * const & s )
	{
	    value = s;
	    return * this;
	}

	operator const min::stub * ( void )
	{
	    return value;
	}
    };

    namespace internal {

	extern min::locatable_stub_ptr *
	    locatable_stub_ptr_last;
	extern min::locatable_gen *
	    locatable_gen_last;

	template < typename T >
	inline min::locatable_var<T> *
	    push_locatable_var
		( min::locatable_var<T> * var )
	{
	    min::locatable_stub_ptr * last =
	        locatable_stub_ptr_last;
	    locatable_stub_ptr_last =
	        (min::locatable_stub_ptr *) var;
	    return (min::locatable_var<T> *) last;
	}
	template <>
	inline min::locatable_var<min::gen> *
	    push_locatable_var<min::gen>
		( min::locatable_var<min::gen> * var )
	{
	    min::locatable_gen * last =
		locatable_gen_last;
	    locatable_gen_last = var;
	    return last;
	}

	template < typename T >
	inline void pop_locatable_var
	    ( min::locatable_var<T> * var,
	      min::locatable_var<T> * previous )
	{
	    MIN_REQUIRE
		(    locatable_stub_ptr_last
		  == (min::locatable_stub_ptr *) var );

	    locatable_stub_ptr_last =
		(min::locatable_stub_ptr *) previous;
	}
	template <>
	inline void pop_locatable_var<min::gen>
		( min::locatable_gen * var,
		  min::locatable_gen * previous )
	{
	    MIN_REQUIRE
		(    locatable_gen_last
		  == (min::locatable_gen *) var );

	    locatable_gen_last = previous;
	}

	template < typename T >
	void locatable_var_check ( void );
	template < typename T >
	min::locatable_var<T> * locatable_var_previous
	    ( min::locatable_var<T> * var );

    }

    template < typename T >
    class locatable_var : public T
    {

    public:

        locatable_var ( void ) : T()
	{
	    previous = internal::push_locatable_var<T>
		( this );
	}

	// We must define an explicit copy constructor.
	//
        locatable_var ( locatable_var<T> const & var )
	    : T ( var )
	{
	    previous = internal::push_locatable_var<T>
		( this );
	}

	locatable_var ( T const & value )
	    : T ( value )
	{
	    previous = internal::push_locatable_var<T>
		( this );
	}

	~ locatable_var ( void )
	{
	    internal::pop_locatable_var<T>
		( this, previous );
	}

	locatable_var<T> & operator =
		( T const & value )
	{
	    new ( this ) T ( value );
	    return * this;
	}

	// We must define an explicit copy assignment
	// operator.
	//
        locatable_var<T> & operator =
		( const locatable_var<T> & var )
	{
	    new ( this ) T ( (T const &) var );
	    return * this;
	}

	operator min::ref<T> ( void )
	{
	    return unprotected::new_ref<T>
	        ( min::unprotected::ZERO_STUB,
		  * (T *) this );
	}

	operator min::ref<T const> ( void ) const
	{
	    return unprotected::new_ref<T>
	        ( min::unprotected::ZERO_STUB,
		  * (T *) this );
	}

	min::ptr<T> operator & ( void )
	{
	    return unprotected::new_ptr<T>
	        ( min::unprotected::ZERO_STUB,
		  (T *) this );
	}

	min::ptr<T const> operator & ( void ) const
	{
	    return unprotected::new_ptr<T>
	        ( min::unprotected::ZERO_STUB,
		  (T *) this );
	}

    private:

        friend void internal::locatable_var_check<T>
		( void );
        friend locatable_var<T> *
	    internal::locatable_var_previous<T>
		( locatable_var<T> * var );

        locatable_var<T> * previous;
    };

#   if MIN_IS_LOOSE
	typedef min::gen locatable_num_gen;
#   else
	typedef min::locatable_gen locatable_num_gen;
#   endif

    namespace internal {

	template < typename T >
	inline void locatable_var_check ( void )
	{
	    MIN_ASSERT
	      (    OFFSETOF
		     ( & locatable_var<T>::previous )
	        == OFFSETOF
		     ( & locatable_stub_ptr::previous ),
		"system programmer error"
	      );
	}
	template < typename T >
	inline locatable_var<T> * locatable_var_previous
	    ( locatable_var<T> * var )
	{
	    return var->previous;
	}
    }
}

// Use MIN_STUB_PTR_CLASS(TARGS,T) if the type
// template< TARGS > class T is convertable to a
// const min::stub * value.  This
//
//   * Redefines write_update<T> to call MUP::acc_write_
//     update.
//
//   * Defines == and != when the left argument is a
//     ref<T> value and the right argument is a const
//     min::stub * value.
//
// Notes:
//
//    * In TARGS and T use MIN_COMMA for comma.
//
//    * In the macro definition, avoid `T>' as T may end
//      with `>'.  Also avoid `TARGS>'.
//     
# define MIN_COMMA ,
# define MIN_STUB_PTR_CLASS(TARGS,T) \
namespace min { \
    \
    template < TARGS > \
    struct write_update<T> \
    { \
        write_update ( const min::stub * s, T v ) \
	{ \
	    unprotected::acc_write_update \
		( s, (const min::stub *) v ); \
	} \
	write_update ( const min::stub * s, \
	               T const * p, \
		       min::unsptr n ) \
	{ \
	    unprotected::acc_write_update \
	        ( s, (const min::stub **) p, n ); \
	} \
    }; \
} \
template < TARGS > \
bool operator == \
	( const min::ref<T> & r, const min::stub * s ) \
{ \
    return (const min::stub *) (T) r == s; \
} \
\
template < TARGS > \
bool operator != \
	( const min::ref<T> & r, const min::stub * s ) \
{ \
    return (const min::stub *) (T) r != s; \
}


// UNICODE Characters
// ------- ----------

namespace min {

    inline min::uns16 Uindex ( Uchar c )
    {
        return c < unicode::index_size ?
	           unicode::index[c] :
		   unicode::index
		       [unicode::index_size - 1];
    }

    struct support_control
    {
        min::uns32 support_mask;
        min::uns32 unsupported_char_flags;
    };

    extern const min::support_control
        ascii_support_control;
    extern const min::support_control
        latin1_support_control;
    extern const min::support_control
        support_all_support_control;

    inline min::uns32 char_flags
	    ( const min::uns32 * char_flags,
	      min::support_control sc,
	      min::Uchar c )
    {
	min::uns16 cindex = min::Uindex ( c );
	min::uns32 cflags = char_flags[cindex];
	if ( ( cflags & sc.support_mask ) == 0 )
	    cflags = sc.unsupported_char_flags;
	return cflags;
    }

    typedef min::uns32 ( * str_classifier )
	    ( const min::uns32 * char_flags,
	      min::support_control sc,
	      min::unsptr n,
	      min::ptr<const min::Uchar> p );

    extern const min::str_classifier
        standard_str_classifier;
    extern const min::str_classifier
        quote_all_str_classifier;
    extern const min::str_classifier
        null_str_classifier;

    void debug_str_class
	    ( const char * header,
	      min::uns32 str_class,
	      min::unsptr n,
	      min::ptr<const min::Uchar> p );

    // UTF-8 Conversion Functions

    unsptr utf8_to_unicode
    	( min::Uchar * & u, const min::Uchar * endu,
	  const char * & s, const char * ends );

    unsptr unicode_to_utf8
	( char * & s, const char * ends,
    	  const min::Uchar * & u,
	  const min::Uchar * endu );

    const min::uns32 IS_GRAPHIC		= ( 1 << 0 );
    const min::uns32 IS_NON_SPACING	= ( 1 << 1 );

    const min::uns32 IS_SP		= ( 1 << 2 );
    const min::uns32 IS_BHSPACE		= ( 1 << 3 );
    const min::uns32 IS_HSPACE		= ( 1 << 4 );

    const min::uns32 IS_VHSPACE		= ( 1 << 5 );
    const min::uns32 IS_CONTROL		= ( 1 << 6 );

    const min::uns32 IS_UNSUPPORTED	= ( 1 << 7 );

    // WARNING: IS_LEADING/TRAILING are the same bits
    // as AFTER_LEADING/TRAILING in printer->state.
    //
    const min::uns32 IS_LEADING		= ( 1 << 8 );
    const min::uns32 IS_TRAILING	= ( 1 << 9 );

    const min::uns32 CONDITIONAL_BREAK	= ( 1 << 10 );

    const min::uns32 NEEDS_QUOTES	= ( 1 << 11 );
    const min::uns32 IS_SEPARATOR	= ( 1 << 12 );
    const min::uns32 IS_REPEATER	= ( 1 << 13 );

    const min::uns32 IS_MARK		= ( 1 << 14 );

    const min::uns32 IS_ASCII		= ( 1 << 16 );
    const min::uns32 IS_LATIN1		= ( 1 << 17 );

    const min::uns32 IS_NON_GRAPHIC = IS_CONTROL
                                    + IS_UNSUPPORTED;

    extern const min::uns32 * standard_char_flags;

}

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
	    MIN_ASSERT ( type_of ( s ) == NUMBER,
	                 "stub argument type is not"
			 " NUMBER" );
	    return unprotected::float_of ( s );
	}
        inline bool is_num ( min::gen g )
	{
	    unsgen v = unprotected::value_of ( g );
	    if ( v < (    (unsgen) GEN_DIRECT_INT_BOUND
	               << VSIZE )
		 ||
	         v >= (   (unsgen) GEN_NEGATIVE_INT
		        << VSIZE ) )
	        return true;
	    else if (   v
	              < ( (unsgen) GEN_STUB << VSIZE ) )
	        return false;
	    else if (   v
	              < (    (unsgen) GEN_STUB_BOUND
		          << VSIZE ) )
	        return
		  ( type_of
		      ( internal::unsgen_to_stub ( v ) )
		    == NUMBER );
	    else
	        return false;
	}
	inline min::gen new_num_gen ( min::int32 v )
	{
	    if ( GEN_MIN_INT <= v && v <= GEN_MAX_INT )
		return unprotected::new_direct_int_gen
				( v );
	    return internal::new_num_stub_gen ( v );
	}
	inline min::gen new_num_gen ( min::uns32 v )
	{
	    if ( v <= (min::uns32) GEN_MAX_INT )
		return unprotected::new_direct_int_gen
				( (int) v );
	    return internal::new_num_stub_gen ( v );
	}
	inline min::gen new_num_gen ( min::int64 v )
	{
	    if ( GEN_MIN_INT <= v && v <= GEN_MAX_INT )
		return unprotected::new_direct_int_gen
				( (int) v );
	    return internal::new_num_stub_gen ( v );
	}
	inline min::gen new_num_gen ( min::uns64 v )
	{
	    if ( v <= (min::uns64) GEN_MAX_INT )
		return unprotected::new_direct_int_gen
				( (int) v );
	    return internal::new_num_stub_gen ( v );
	}
	inline min::gen new_num_gen ( min::float64 v )
	{
	    if ( GEN_MIN_INT <= v && v <= GEN_MAX_INT )
	    {
	        int i = (int) v;
		if ( i == v )
		    return unprotected::
		           new_direct_int_gen ( i );
	    }
	    return internal::
		   new_num_stub_gen ( v );
	}
	inline int int_of ( min::gen g )
	{
	    if ( is_stub ( g ) )
	    {
		const min::stub * s =
		    unprotected::stub_of ( g );
		MIN_ASSERT (    type_of ( s )
			     == NUMBER,
			     "argument has stub whose"
			     " type is not NUMBER" );
		float64 f =
		    unprotected::float_of ( s );
		MIN_ASSERT (    INT_MIN <= f
			     && f <= INT_MAX,
			     "argument out of range" );
		int i = (int) f;
		MIN_ASSERT ( i == f,
		             "argument has non-zero"
			     " fractional part");
		return i;
	    }
	    else if ( is_direct_int ( g ) )
		return unprotected::
		       direct_int_of ( g );
	    else
		MIN_ABORT ( "int_of non number" );
	}
	namespace unprotected {
	    inline float64 float_of ( min::gen g )
	    {
		if ( is_stub ( g ) )
		{
		    const min::stub * s =
			unprotected::stub_of ( g );
		    return unprotected
		              ::float_of ( s );
		}
		else
		    return unprotected::
			   direct_int_of ( g );
	    }
	}
	inline float64 float_of ( min::gen g )
	{
	    if ( is_stub ( g ) )
	    {
		const min::stub * s =
			unprotected::stub_of ( g );
		return float_of ( s );
	    }
	    else if ( is_direct_int ( g ) )
		return unprotected::
		       direct_int_of ( g );
	    else
		MIN_ABORT ( "float_of non number" );
	}
#   elif MIN_IS_LOOSE
	inline bool is_num ( min::gen g )
	{
	    return is_direct_float ( g );
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
	inline min::gen new_num_gen ( min::float64 v )
	{
	    return new_direct_float_gen ( v );
	}
	inline int int_of ( min::gen g )
	{
	    MIN_ASSERT ( is_num ( g ),
	                 "argument is not number" );
	    float64 f =
	        unprotected::direct_float_of ( g );
	    MIN_ASSERT
		( INT_MIN <= f && f <= INT_MAX,
		  "argument is out of range" );
	    int i = (int) f;
	    MIN_ASSERT ( i == f,
		         "argument has non-zero"
			 " fractional part");
	    return i;
	}
	namespace unprotected {
	    inline float64 float_of ( min::gen g )
	    {
		return unprotected
		       ::direct_float_of ( g );
	    }
	}
	inline float64 float_of ( min::gen g )
	{
	    MIN_ASSERT ( is_num ( g ),
	                 "argument is not number" );
	    return unprotected::direct_float_of ( g );
	}
#   endif

    min::uns32 floathash ( min::float64 f );

    inline min::uns32 numhash ( min::gen g )
    {
	return floathash ( float_of ( g ) );
    }
}

// Strings
// -------

namespace min { namespace unprotected {

    // A long string body consists a long_str structure
    // followed by the bytes of the NUL-terminated
    // string.  The latter is padded with zeros to a
    // multiple of 8 bytes.

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
	return (unprotected::long_str *)
	       unprotected::ptr_of ( s );
    }
    inline const char * str_of
    	    ( min::unprotected::long_str * str )
    {
	return (const char *) str
	       + sizeof ( unprotected::long_str );
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

    min::unsptr strlen ( min::gen g );
    min::uns32 strhash ( min::gen g );
    char * strcpy ( char * p, min::gen g );
    char * strncpy
        ( char * p, min::gen g, min::unsptr n );
    int strcmp ( const char * p, min::gen g );
    int strncmp
        ( const char * p, min::gen g, min::unsptr n );

    // For forward reference.
    //
    class str_ptr;

    // Functions that must be declared before they are
    // made friends.
    //
    namespace unprotected {
	const char * str_of ( const min::str_ptr & sp );
    }
    min::unsptr strlen ( const min::str_ptr & sp );
    min::uns32 strhash ( const min::str_ptr & sp );
    char * strcpy ( char * p, const min::str_ptr & sp );
    char * strncpy ( char * p,
                     const min::str_ptr & sp,
		     min::unsptr n );
    int strcmp
        ( const char * p,
	  const min::str_ptr & sp );
    int strncmp
        ( const char * p,
	  const min::str_ptr & sp,
	  min::unsptr n );
    ptr<const char> begin_ptr_of
	( const min::str_ptr & sp );

    inline min::uns64 strhead ( min::gen g )
    {
	unsgen v = unprotected::value_of ( g );
	if ( v >> VSIZE == GEN_DIRECT_STR )
	{
#	    if MIN_IS_BIG_ENDIAN
		return ( v << TSIZE );
#	    elif MIN_IS_LITTLE_ENDIAN
		return ( v & internal::VMASK );
#	    endif
	}

	if ( ! is_stub ( g ) ) return 0;

	const min::stub * s =
	    unprotected::stub_of ( g );
	if ( type_of ( s ) == min::SHORT_STR )
	    return s->v.u64;
	else if ( type_of ( s ) == min::LONG_STR )
	{
	    min::unprotected::long_str * lsp =
	        min::unprotected::long_str_of ( s );
	    // Note lsp->length > 0 and strings are
	    // NUL padded to a multiple of 8 bytes.
	    min::uns8 * cp = (min::uns8 *) lsp
			   + sizeof ( * lsp );
	    return * (min::uns64 *) cp;
	}
	else
	    return 0;
    }

    class str_ptr
    {
    public:

	str_ptr ( const min::stub * s )
	    : s ( s )
	{
	    int type = min::type_of ( s );
	    if ( type == LONG_STR ) return;
	    else if ( type != SHORT_STR )
	    {
		s = NULL;
		return;
	    }

	    pseudo_body.u.str = s->v.u64;
	    pseudo_body.u.buf[8] = 0;
	    this->s = & pseudo_stub;
	    unprotected::set_ptr_of
		   ( & pseudo_stub,
		     (void *) & pseudo_body );
	}

	str_ptr ( min::gen g )
	{

	    if ( is_stub ( g ) )
		new ( this )
		    str_ptr
			( unprotected::stub_of
			      ( g ) );
	    else if ( is_direct_str ( g ) )
	    {
		pseudo_body.u.str
		    = unprotected
			 ::direct_str_of ( g );
		s = & pseudo_stub;
		unprotected::set_ptr_of
		   ( & pseudo_stub,
		     (void *) & pseudo_body );
	    }
	    else
		s = NULL;
	}

	str_ptr ( void ) : s ( NULL ) {}

	operator bool ( void ) const
	{
	    return s != NULL;
	}

	// Operator[] MUST be a member and cannot
	// be a friend.
	//
	char operator [] ( int index ) const
	{
	    return ( (const char *)
	             unprotected::long_str_of ( s ) )
	           [   sizeof ( unprotected::long_str )
		     + index ];
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
		unprotected::set_ptr_of
		       ( & pseudo_stub,
			 (void *) & pseudo_body );
	    }
	    else
		s = sp.s;
	    return * this;
	}

	friend const char * min::unprotected::str_of
	    ( const str_ptr & sp );
	friend min::unsptr min::strlen
	    ( const str_ptr & sp );
	friend min::uns32 min::strhash
	    ( const str_ptr & sp );
	friend char * min::strcpy
	    ( char * p,
	      const str_ptr & sp );
	friend char * min::strncpy
	    ( char * p,
	      const str_ptr & sp,
	      min::unsptr n );
	friend int min::strcmp
	    ( const char * p,
	      const str_ptr & sp );
	friend int min::strncmp
	    ( const char * p,
	      const str_ptr & sp,
	      min::unsptr n );
	friend min::ptr<const char>
	    min::begin_ptr_of
		( const str_ptr & sp );

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

    // Function needed in min::str_ptr definition.
    //
    inline bool is_str ( min::gen g )
    {
	if ( is_direct_str ( g ) )
	    return true;
	if ( ! is_stub ( g ) )
	    return false;
	const min::stub * s =
	    unprotected::stub_of ( g );
	return type_of ( s ) == min::SHORT_STR
	       ||
	       type_of ( s ) == min::LONG_STR;
    }

    inline const char * unprotected::str_of
	    ( const min::str_ptr & sp )
    {
	return (const char * )
	       unprotected::long_str_of ( sp.s )
	       +
	       sizeof ( unprotected::long_str );
    }

    inline min::unsptr strlen
        ( const min::str_ptr & sp )
    {
        if ( sp.s == & sp.pseudo_stub )
	    return ::strlen ( sp.pseudo_body.u.buf );
	else
	    return unprotected::length_of
	              ( unprotected::long_str_of
		            ( sp.s ) );
    }

    inline min::uns32 strhash
	    ( const min::str_ptr & sp )
    {
        if ( sp.s == & sp.pseudo_stub )
	    return strhash ( sp.pseudo_body.u.buf );
	else
	    return unprotected::hash_of
	              ( unprotected::long_str_of
		            ( sp.s ) );
    }

    inline char * strcpy
    	( char * p,
	  const min::str_ptr & sp )
    {
        return ::strcpy
	    ( p, unprotected::str_of ( sp ) );
    }

    inline char * strncpy
    	( char * p,
	  const min::str_ptr & sp,
	  min::unsptr n )
    {
        return ::strncpy
	    ( p, unprotected::str_of ( sp ), n );
    }

    inline int strcmp
    	( const char * p,
	  const min::str_ptr & sp )
    {
        return ::strcmp
	    ( p, unprotected::str_of ( sp ) );
    }

    inline int strncmp
    	( const char * p,
	  const min::str_ptr & sp,
	  min::unsptr n )
    {
        return ::strncmp
	    ( p, unprotected::str_of ( sp ), n );
    }

    inline min::ptr<const char> begin_ptr_of
	( const str_ptr & sp )
    {
	return unprotected::new_ptr
	    ( sp.s != & sp.pseudo_stub ?
		  sp.s : unprotected::ZERO_STUB,
	      ( (const char * )
		unprotected::long_str_of ( sp.s ) )
	      + sizeof ( unprotected::long_str )
	    );
    }

    namespace internal {
	min::gen new_str_stub_gen
	    ( min::ptr<const char> p, min::unsptr n );

	inline min::gen new_str_gen
		( const char * p, min::unsptr n )
	{
#	if MIN_IS_COMPACT
		if ( n <= 3 )
		    return unprotected::
			   new_direct_str_gen ( p, n );
#	elif MIN_IS_LOOSE
		if ( n <= 5 )
		    return unprotected::
			   new_direct_str_gen ( p, n );
#	endif
	    return internal::new_str_stub_gen
	        ( new_ptr<const char> ( p ), n );
	}

	inline min::gen new_str_gen
		( min::ptr<const char> p,
		  min::unsptr n )
	{
#	if MIN_IS_COMPACT
		if ( n <= 3 )
		    return unprotected::
			   new_direct_str_gen
			       ( ! p, n );
#	elif MIN_IS_LOOSE
		if ( n <= 5 )
		    return unprotected::
			   new_direct_str_gen
			       ( ! p, n );
#	endif
	    return internal::new_str_stub_gen ( p, n );
	}
    }

    inline min::gen new_str_gen ( const char * p )
    {
	return internal::new_str_gen
	    ( p, ::strlen ( p ) );
    }

    inline min::gen new_str_gen
            ( const char * p, min::unsptr n )
    {
        return internal::new_str_gen
	    ( p, internal::strnlen ( p, n ) );
    }

    inline min::gen new_str_gen
	    ( min::ptr<const char> p )
    {
	return internal::new_str_gen
	    ( p, ::strlen ( ! p ) );
    }

    inline min::gen new_str_gen
            ( min::ptr<const char> p, min::unsptr n )
    {
        return internal::new_str_gen
	    ( p, internal::strnlen ( ! p, n ) );
    }

    // Compensate for the lack of implicit conversion
    // from ptr<char> to ptr<const char>.
    //
    inline min::gen new_str_gen
	    ( min::ptr<char> p )
    {
	return internal::new_str_gen
	    ( (min::ptr<const char>) p,
	      ::strlen ( ! p ) );
    }

    inline min::gen new_str_gen
            ( min::ptr<char> p, min::unsptr n )
    {
        return internal::new_str_gen
	    ( (min::ptr<const char>) p,
	      internal::strnlen ( ! p, n ) );
    }

    min::gen new_str_gen
            ( const min::Uchar * p, min::unsptr n );

    // new_str_gen with min::Uchar characters converts
    // to a UTF-8 char string in the stack and there-
    // fore handles relocatable min::ptr's.
    //
    inline min::gen new_str_gen
            ( min::ptr<const min::Uchar> p,
	      min::unsptr n )
    {
        return new_str_gen
	    ( ! p, n );
    }

    // Compensate for the lack of implicit conversion
    // from ptr<Uchar> to ptr<const Uchar>.
    //
    inline min::gen new_str_gen
            ( min::ptr<min::Uchar> p,
	      min::unsptr n )
    {
        return new_str_gen
	    ( ! p, n );
    }

    inline min::uns32 str_class
	    ( const min::uns32 * char_flags,
	      min::support_control sc,
	      min::unsptr n,
	      min::ptr<const min::Uchar> p,
	      const min::str_classifier strcl )
    {
	return ( * strcl ) ( char_flags, sc, n, p );
    }

    inline min::uns32 str_class
	    ( const min::uns32 * char_flags,
	      min::support_control sc,
	      min::gen v,
	      const min::str_classifier strcl )
    {
        if ( ! min::is_str ( v ) ) return false;
	min::str_ptr sp ( v );
	min::unsptr len = min::strlen ( sp );
	min::Uchar s[len];
	min::Uchar * p = s;
	const char * q = ! min::begin_ptr_of ( sp );
	min::utf8_to_unicode 
	    ( p, p + len, q, q + len );
	return ( * strcl )
	    ( char_flags, sc, p - s,
	      min::new_ptr<const min::Uchar> ( s ) );
    }
}

namespace min {

    // Functions to convert strings to numbers.
    //
    bool strto ( min::int32 & value,
                 const min::str_ptr sp, int & i,
		 int base = 0 );
    bool strto ( min::uns32 & value,
                 const min::str_ptr sp, int & i,
		 int base = 0 );
    bool strto ( min::int64 & value,
                 const min::str_ptr sp, int & i,
		 int base = 0 );
    bool strto ( min::uns64 & value,
                 const min::str_ptr sp, int & i,
		 int base = 0 );
    bool strto ( min::float32 & value,
                 const min::str_ptr sp, int & i );
    bool strto ( min::float64 & value,
                 const min::str_ptr sp, int & i );
    bool strto ( min::int32 & value, min::gen g,
                 int base = 0 );
    bool strto ( min::uns32 & value, min::gen g,
                 int base = 0 );
    bool strto ( min::int64 & value, min::gen g,
                 int base = 0 );
    bool strto ( min::uns64 & value, min::gen g,
                 int base = 0 );
    bool strto ( min::float32 & value, min::gen g );
    bool strto ( min::float64 & value, min::gen g );
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
	    return (internal::lab_header *)
		   unprotected::ptr_of ( s );
	}
    }

    namespace unprotected {

	// Declared for forward reference.
	//
	class lab_ptr;
    }

    // Declared for use as friend below.
    //
    min::ptr<const min::gen> begin_ptr_of
	    ( min::unprotected::lab_ptr & labp );
    min::ptr<const min::gen> end_ptr_of
	    ( min::unprotected::lab_ptr & labp );
    min::uns32 lablen
	    ( min::unprotected::lab_ptr & labp );
    min::uns32 labhash
	    ( min::unprotected::lab_ptr & labp );

    namespace unprotected {

	class lab_ptr
	{
	public:

	    lab_ptr ( const min::stub * s )
		: s ( s )
	    {
	        if ( min::type_of ( s ) != LABEL )
		    s = NULL;
	    }

	    lab_ptr ( min::gen g )
		: s ( min::stub_of ( g ) )
	    {
	        if ( min::type_of ( s ) != LABEL )
		    s = NULL;
	    }

	    lab_ptr ( void )
		: s ( NULL ) {}

	    operator bool ( void )
	    {
	        return s != NULL;
	    }

	    lab_ptr & operator = ( min::gen g )
	    {
	        new ( this ) lab_ptr ( g );
		return * this;
	    }

	    lab_ptr & operator = ( const min::stub * s )
	    {
	        new ( this ) lab_ptr ( s );
		return * this;
	    }

	    min::gen operator []
	        ( min::uns32 i ) const
	    {
		return base()[i];
	    }

	    operator const min::stub * ( void ) const
	    {
	        return s;
	    }

	    friend min::ptr<const min::gen>
	        min::begin_ptr_of
		    ( min::unprotected
		         ::lab_ptr & labp );
	    friend min::ptr<const min::gen>
	        min::end_ptr_of
		    ( min::unprotected
		         ::lab_ptr & labp );
	    friend min::uns32 min::lablen
		( min::unprotected::lab_ptr & labp );
	    friend min::uns32 min::labhash
		( min::unprotected::lab_ptr & labp );

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

        lab_ptr ( min::gen g )
	{
	    if ( is_stub ( g ) )
	    {
	        const min::stub * s = stub_of ( g );
		if ( type_of ( s ) == LABEL )
		    this->s = s;
	    }
	}

        lab_ptr ( const min::stub * s )
	{
	    if ( type_of ( s ) == LABEL ) this->s = s;
	}

        lab_ptr ( void ) {}

	lab_ptr & operator = ( min::gen g )
	{
	    new ( this ) lab_ptr ( g );
	    return * this;
	}

	lab_ptr & operator = ( const min::stub * s )
	{
	    new ( this ) lab_ptr ( s );
	    return * this;
	}

	min::gen operator []
	    ( min::uns32 i ) const
	{
	    MIN_ASSERT ( i < header()->length,
	                 "subscript is too large" );
	    return base()[i];
	}

    };

    inline min::ptr<const min::gen> begin_ptr_of
	    ( min::unprotected::lab_ptr & labp )
    {
        return unprotected::new_ptr
	    ( labp.s, labp.base() );
    }

    inline min::ptr<const min::gen> end_ptr_of
	    ( min::unprotected::lab_ptr & labp )
    {
        return unprotected::new_ptr
	    ( labp.s,
	      labp.base() + labp.header()->length );
    }

    inline min::uns32 lablen
	    ( min::unprotected::lab_ptr & labp )
    {
        return labp.header()->length;
    }

    inline min::uns32 labhash
	    ( min::unprotected::lab_ptr & labp )
    {
        return labp.header()->hash;
    }

    inline min::uns32 lablen ( const min::stub * s )
    {
        MIN_ASSERT ( type_of ( s ) == LABEL,
	             "stub argument type is not"
		     " LABEL" );
	return internal::lab_header_of(s)->length;
    }

    inline min::uns32 lablen ( min::gen g )
    {
	return lablen ( stub_of ( g ) );
    }

    inline min::uns32 labhash ( const min::stub * s )
    {
	MIN_ASSERT ( type_of ( s ) == LABEL,
		     "stub argument type is not"
		     " LABEL" );
	return internal::lab_header_of(s)->hash;
    }

    inline min::uns32 labhash ( min::gen g )
    {
	return labhash ( stub_of ( g ) );
    }

    min::uns32 labhash
	    ( const min::gen * p, min::uns32 n );

    const min::uns32 labhash_initial = 1009;

    const min::uns32 labhash_factor =
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
        hash = ( hash * labhash_factor ) + h;
	if ( hash == 0 ) hash = (uns32) -1;
	return hash;
    }

    inline min::uns32 labncpy
	    ( min::gen * p, const min::stub * s,
	      min::uns32 n )
    {
	lab_ptr labp ( s );
	if ( n > lablen ( labp ) )
	    n = lablen ( labp );
        for ( uns32 i = 0; i < n; ++ i )
	{
	    * p ++ = labp[i];
	}
	return n;
    }

    inline min::uns32 labncpy
	    ( min::gen * p, min::gen g, min::uns32 n )
    {
	return labncpy ( p, stub_of ( g ), n );
    }

    min::gen new_lab_gen
	    ( min::ptr<const min::gen> p,
	      min::uns32 n );

    inline min::gen new_lab_gen
	    ( min::ptr<min::gen> p,
	      min::uns32 n )
    {
        return new_lab_gen
	    ( (min::ptr<const min::gen>) p, n );
    }

    inline min::gen new_lab_gen
	    ( const min::gen * p, min::uns32 n )
    {
        return new_lab_gen ( new_ptr ( p ), n );
    }

    inline bool is_lab ( min::gen g )
    {
	if ( ! is_stub ( g ) )
	    return false;
	const min::stub * s =
	    unprotected::stub_of ( g );
	return type_of ( s ) == LABEL;
    }

    min::gen new_lab_gen
        ( const char * s1,
	  const char * s2 );
    min::gen new_lab_gen
        ( const char * s1,
	  const char * s2,
	  const char * s3 );

    min::int32 is_subsequence
        ( min::gen v1, min::gen v2 );
}

// Names
// -----

namespace min {

    inline bool is_name ( min::gen g )
    {
        return is_num ( g )
	       ||
	       is_str ( g )
	       ||
	       is_lab ( g );
    }

    namespace internal {

        inline bool is_name_type ( int type )
	{
	    switch ( type )
	    {
	    case NUMBER:
            case SHORT_STR:
            case LONG_STR:
            case LABEL:
	        return true;
	    }

	    return false;
	}

    }

    min::uns32 hash ( min::gen g );

    int compare ( min::gen g1, min::gen g2 );
}

// Packed Structures
// -----------------

namespace min {

    template < typename S >
    min::uns32 DISP ( const min::gen S::* d )
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

    struct packed_id
    {
	const packed_id * base;
    };

    namespace internal {

	// An packed struct/vec body begins with a
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
	// range 0 < t < packed_subtype_count.  If a
	// new packed subtype is allocated, packed_
	// subtype_count is incremented.  If it would
	// become > max_packed_subtype_count then
	// allocate_packed_subtypes is called.
	//
	extern unsigned packed_subtype_count;
	extern unsigned max_packed_subtype_count;

	// Allocate or reallocate the packed_subtypes
	// vector setting max_packed_subtype_count to
	// the given value.  WARNING: this is called
	// during static memory construction and BEFORE
	// min::initialize().
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

	    packed_struct_ptr_base ( min::gen g )
	    {
		new ( this )
		    internal
		    ::packed_struct_ptr_base<S>
			( stub_of ( g ) );
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

    namespace unprotected {

	inline min::uns32 packed_subtype_of
		( const min::stub * s )
	{
	    void * p = unprotected::ptr_of ( s );
	    uns32 subtype = * (uns32 *) p;
	    subtype &=
		internal::PACKED_CONTROL_SUBTYPE_MASK;
	    return subtype;
	}

    }

    inline min::uns32 packed_subtype_of
	    ( const min::stub * s )
    {
	int t = type_of ( s );
	if ( t == PACKED_STRUCT || t == PACKED_VEC )
	    return unprotected::packed_subtype_of ( s );
	else
	    return 0;
    }

    inline min::uns32 packed_subtype_of
	    ( min::gen g )
    {
	return packed_subtype_of ( stub_of ( g ) );
    }

    inline const char * name_of_packed_subtype
        ( min::uns32 subtype )
    {
	internal::packed_descriptor * pdescriptor =
	    (internal::packed_descriptor *)
	    (*internal::packed_subtypes)[subtype];
	return pdescriptor->name;
    }

    template < typename S >
    class packed_struct_ptr
	  : public internal::packed_struct_ptr_base<S>
    {

    public:

	packed_struct_ptr
	        ( const min::packed_struct_ptr<S>
		        & psp )
	{
	    this->s = psp.s;
	}
	packed_struct_ptr ( min::gen g )
	    : internal::packed_struct_ptr_base<S>
		( g ) {}
	packed_struct_ptr ( const min::stub * s )
	    : internal::packed_struct_ptr_base<S>
		( s ) {}
	packed_struct_ptr ( void )
	    : internal::packed_struct_ptr_base<S>
		() {}

	min::ptr<const S> operator -> ( void ) const
	{
	    return min::unprotected::new_ptr<const S>
	       ( this->s, (min::unsptr) 0 );
	}

	min::ref<const S> operator * ( void ) const
	{
	    return * min::unprotected::new_ptr<const S>
	       ( this->s, (min::unsptr) 0 );
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
		( min::gen g )
	{
	    new ( this )
		internal::packed_struct_ptr_base<S>
		    ( g );
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
	    ( const packed_struct_ptr<T> S::* d )
    {
	return   OFFSETOF ( d )
	       + packed_struct_ptr<T>::DISP();
    }

    template < typename S >
    class packed_struct_updptr
	  : public packed_struct_ptr<S>
    {

    public:

	packed_struct_updptr
	        ( const min::packed_struct_updptr<S>
		        & psup )
	  : packed_struct_ptr<S> ( psup.s ) {}
	packed_struct_updptr
		( min::gen g )
	    : packed_struct_ptr<S> ( g ) {}
	packed_struct_updptr
		( const min::stub * s )
	    : packed_struct_ptr<S> ( s ) {}
	packed_struct_updptr ( void )
	    : packed_struct_ptr<S>() {}

	min::ptr<S> operator -> ( void ) const
	{
	    return min::unprotected::new_ptr<S>
	       ( this->s, (min::unsptr) 0 );
	}

	min::ref<S> operator * ( void ) const
	{
	    return * min::unprotected::new_ptr<const S>
	       ( this->s, (min::unsptr) 0 );
	}

	packed_struct_updptr & operator =
		( const min::stub * s )
	{
	    new ( this ) packed_struct_ptr<S> ( s );
	    return * this;
	}

	packed_struct_updptr & operator =
		( min::gen g )
	{
	    new ( this ) packed_struct_ptr<S> ( g );
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
	    ( const packed_struct_updptr<T> S::* d )
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
	    return min::new_stub_gen
	        ( internal::packed_struct_new_stub
		      ( this ) );
	}

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

	int t = unprotected::type_of ( s );
	if ( t != PACKED_STRUCT )
	{
	    this->s = NULL;
	    return;
	}
	uns32 subtype =
	    unprotected::packed_subtype_of ( s );
	packed_struct_descriptor * psdescriptor =
	    (packed_struct_descriptor *)
	    (*packed_subtypes)[subtype];
	const packed_id * id = psdescriptor->id;
	while ( & packed_struct<S>::id != id )
	{
	    if ( id == NULL )
	    {
	        this->s = NULL;
		return;
	    }
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
          ( PACKED_STRUCT,
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
	      ( & S::control ) == 0 ),
	  "`control' member of structure is not first"
	  " member of the structure" );

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
        MIN_ASSERT ( id.base == NULL,
	             "packed_struct_with_base used"
		     " twice to declare same"
		     " packed_struct with two distinct"
		     " bases" );
	id.base = & base_class_id;
    }
}

// Packed Vectors
// --------------

// Much of packed vectors replicates packed structures;
// see comments under Packed Structures for documenta-
// tion.

namespace min {

    template < typename E, typename H, typename L >
	class packed_vec_ptr;
    template < typename E, typename H, typename L >
	class packed_vec_updptr;

    template < typename E, typename H, typename L >
    min::ptr< E const> begin_ptr_of
        ( min::packed_vec_ptr<E,H,L> p );
    template < typename E, typename H, typename L >
    min::ptr< E const> end_ptr_of
        ( min::packed_vec_ptr<E,H,L> p );

    template < typename E, typename H, typename L >
    min::ptr< E > begin_ptr_of
        ( min::packed_vec_updptr<E,H,L> p );
    template < typename E, typename H, typename L >
    min::ptr< E > end_ptr_of
        ( min::packed_vec_updptr<E,H,L> p );

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

	    min::unsptr initial_max_length;
	    min::float64 increment_ratio;
	    min::unsptr max_increment;

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

		  initial_max_length ( 128 ),
		  increment_ratio (0.5),
		  max_increment ( 4096 ) {}
	};

	template < typename E, typename H, typename L >
	class packed_vec_ptr_base
	{
	public:

	    packed_vec_ptr_base ( min::gen g )
	    {
	        new ( this )
		    packed_vec_ptr_base
			( stub_of ( g ) );
	    }
	    packed_vec_ptr_base
	        ( const min::stub * s );
	    packed_vec_ptr_base ( void )
	        : s ( NULL_STUB ) {}

	    operator const min::stub * ( void ) const
	    {
		return this->s;
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

	// Out-of-line new_stub function.
	//
	template < typename L >
	const min::stub * packed_vec_new_stub
	    ( packed_vec_descriptor * pvd,
	      L max_length,
	      L length,
	      const void * vp );

	// Out-of-line resize functions.
	//
	template < typename L >
	void packed_vec_resize
	    ( const min::stub * s,
	      L max_length );
	template < typename L >
	void packed_vec_resize
	    ( const min::stub * s,
	      packed_vec_descriptor * pvd,
	      L max_length );
    }

    template < typename L >
    struct packed_vec_header
    {
        const min::uns32 control;
        const L length;
        const L max_length;
    };

    template < typename E,
               typename H = packed_vec_header<uns32>,
	       typename L = uns32 >
    class packed_vec_ptr
	: public internal::packed_vec_ptr_base<E,H,L>
    {

    public:

	packed_vec_ptr
	        ( const min::packed_vec_ptr<E,H,L>
		  & pvp )
	    : internal::packed_vec_ptr_base<E,H,L>
	    ( pvp.s ) {}
	packed_vec_ptr ( min::gen g )
	    : internal::packed_vec_ptr_base<E,H,L>
	    ( g ) {}
	packed_vec_ptr ( const min::stub * s )
	    : internal::packed_vec_ptr_base<E,H,L>
	    ( s ) {}
	packed_vec_ptr ( void )
	    : internal::packed_vec_ptr_base<E,H,L>
	    () {}

	min::ptr<H const> operator -> ( void ) const
	{
	    return min::unprotected::new_ptr<const H>
	       ( this->s, (min::unsptr) 0 );
	}

	min::ref<H const> operator * ( void ) const
	{
	    return min::unprotected::new_ref<const H>
	       ( this->s, (min::unsptr) 0 );
	}

	min::ref<E const> operator [] ( L i ) const
	{
	    H * hp = (H *)
		unprotected::ptr_of ( this->s );
	    MIN_ASSERT ( i < hp->length,
	                 "subscript too large" );
	    return * min::unprotected::new_ptr<const E>
	       ( this->s,
		 internal::packed_vec_ptr_base<E,H,L>
		         ::computed_header_size
		 +
		 i * sizeof ( E ) );
	}

	min::ptr<E const> operator + ( L i )
	    const
	{
	    H * hp = (H *)
		unprotected::ptr_of ( this->s );
	    MIN_ASSERT ( i < hp->length,
	                 "subscript too large" );
	    return min::unprotected::new_ptr
		( this->s,
		  ( E const *)
		  ( (uns8 *) hp
		    +
		    internal::packed_vec_ptr_base<E,H,L>
		            ::computed_header_size
		    +
		    i * sizeof ( E ) ) );
	}

	friend min::ptr< E const> begin_ptr_of<>
	    ( min::packed_vec_ptr<E,H,L> p );
	friend min::ptr< E const> end_ptr_of<>
	    ( min::packed_vec_ptr<E,H,L> p );
	friend min::ptr< E > begin_ptr_of<>
	    ( min::packed_vec_updptr<E,H,L> p );
	friend min::ptr< E > end_ptr_of<>
	    ( min::packed_vec_updptr<E,H,L> p );

	packed_vec_ptr & operator =
		( const min::stub * s )
	{
	    new ( this )
		internal::packed_vec_ptr_base<E,H,L>
		    ( s );
	    return * this;
	}

	packed_vec_ptr & operator =
		( min::gen g )
	{
	    new ( this )
		internal::packed_vec_ptr_base<E,H,L>
		    ( g );
	    return * this;
	}

	static min::uns32 DISP ( void )
	{
	    return OFFSETOF ( & packed_vec_ptr::s );
	}
    };

    template < typename S,
               typename E, typename H, typename L >
    min::uns32 DISP
	    ( const packed_vec_ptr<E,H,L> S::* d )
    {
	return   OFFSETOF ( d )
	       + packed_vec_ptr<E,H,L>::DISP();
    }

    template < typename E, typename H, typename L >
    min::ptr<E const> begin_ptr_of
        ( min::packed_vec_ptr<E,H,L> p )
    {
	H * hp = (H *)
	    unprotected::ptr_of ( p.s );
	return min::unprotected::new_ptr
	    ( p.s,
	      ( E const *)
	      ( (uns8 *) hp
		+
		internal::packed_vec_ptr_base<E,H,L>
			::computed_header_size ) );
    }

    template < typename E, typename H, typename L >
    min::ptr<E const> end_ptr_of
        ( min::packed_vec_ptr<E,H,L> p )
    {
	H * hp = (H *) unprotected::ptr_of ( p.s );
	return min::unprotected::new_ptr
	    ( p.s,
	      ( E const *)
	      ( (uns8 *) hp
		+
		internal::packed_vec_ptr_base<E,H,L>
			::computed_header_size
		+
		hp->length * sizeof ( E ) ) );
    }

    template < typename E,
               typename H = packed_vec_header<uns32>,
	       typename L = uns32 >
    class packed_vec_updptr
	: public packed_vec_ptr<E,H,L>
    {

    public:

	packed_vec_updptr
	        ( const min::packed_vec_updptr<E,H,L>
		      & pvup )
	    : packed_vec_ptr<E,H,L> ( pvup.s ) {}
	packed_vec_updptr ( min::gen g )
	    : packed_vec_ptr<E,H,L> ( g ) {}
	packed_vec_updptr
		( const min::stub * s )
	    : packed_vec_ptr<E,H,L> ( s ) {}
	packed_vec_updptr ( void )
	    : packed_vec_ptr<E,H,L>() {}

	min::ptr<H> operator -> ( void ) const
	{
	    return min::unprotected::new_ptr<H>
	       ( this->s, (min::unsptr) 0 );
	}

	min::ref<H> operator * ( void ) const
	{
	    return min::unprotected::new_ref<H>
	       ( this->s, (min::unsptr) 0 );
	}

	min::ref<E> operator [] ( L i ) const
	{
	    H * hp = (H *)
		unprotected::ptr_of ( this->s );
	    MIN_ASSERT ( i < hp->length,
	                 "subscript too large" );
	    return * min::unprotected::new_ptr<E>
	       ( this->s,
		 internal::packed_vec_ptr_base<E,H,L>
		         ::computed_header_size
		 +
		 i * sizeof ( E ) );
	}

	min::ptr<E> operator + ( L i )
	    const
	{
	    H * hp = (H *)
		unprotected::ptr_of ( this->s );
	    MIN_ASSERT ( i < hp->length,
	                 "subscript too large" );
	    return min::unprotected::new_ptr<E>
		( this->s,
		  internal::packed_vec_ptr_base<E,H,L>
		          ::computed_header_size
		  +
		  i * sizeof ( E ) );
	}

	packed_vec_updptr & operator =
		( const min::stub * s )
	{
	    new ( this ) packed_vec_ptr<E,H,L> ( s );
	    return * this;
	}

	packed_vec_updptr & operator =
		( min::gen g )
	{
	    new ( this ) packed_vec_ptr<E,H,L> ( g );
	    return * this;
	}

	static min::uns32 DISP ( void )
	{
	    return OFFSETOF
	        ( & packed_vec_updptr::s );
	}
    };


    template < typename S,
               typename E, typename H, typename L >
    min::uns32 DISP
	    ( const packed_vec_updptr<E,H,L> S::* d )
    {
	return   OFFSETOF ( d )
	       + packed_vec_updptr<E,H,L>::DISP();
    }

    template < typename E, typename H, typename L >
    inline min::ptr<E> begin_ptr_of
        ( min::packed_vec_updptr<E,H,L> p )
    {
	H * hp = (H *)
	    unprotected::ptr_of ( p.s );
	return min::unprotected::new_ptr
	    ( p.s,
	      ( E *)
	      ( (uns8 *) hp
		+
		internal::packed_vec_ptr_base<E,H,L>
			::computed_header_size ) );
    }

    template < typename E, typename H, typename L >
    inline min::ptr<E> end_ptr_of
        ( min::packed_vec_updptr<E,H,L> p )
    {
	H * hp = (H *)
	    unprotected::ptr_of ( p.s );
	return min::unprotected::new_ptr
	    ( p.s,
	      ( E *)
	      ( (uns8 *) hp
		+
		internal::packed_vec_ptr_base<E,H,L>
			::computed_header_size
		+
		hp->length * sizeof ( E ) ) );
    }

    template < typename E,
               typename H = packed_vec_header<uns32>,
	       typename L = uns32 >
    class packed_vec_insptr
	: public packed_vec_updptr<E,H,L>
    {

    public:

	packed_vec_insptr
	        ( const min::packed_vec_insptr<E,H,L>
		      & pvip )
	    : packed_vec_updptr<E,H,L> ( pvip.s )
	{}
	packed_vec_insptr ( min::gen g )
	    : packed_vec_updptr<E,H,L> ( g ) {}
	packed_vec_insptr
		( const min::stub * s )
	    : packed_vec_updptr<E,H,L> ( s ) {}
	packed_vec_insptr ( void )
	    : packed_vec_updptr<E,H,L>() {}

	packed_vec_insptr & operator =
		( const min::stub * s )
	{
	    new ( this ) packed_vec_updptr<E,H,L> ( s );
	    return * this;
	}

	packed_vec_insptr & operator =
		( min::gen g )
	{
	    new ( this ) packed_vec_updptr<E,H,L> ( g );
	    return * this;
	}

	static min::uns32 DISP ( void )
	{
	    return OFFSETOF
	        ( & packed_vec_insptr::s );
	}

	void reserve ( L reserve_length );
	void resize  ( L max_length )
	{
	    internal::packed_vec_resize<L>
		( this->s, max_length );
	}
    };

    template < typename S,
               typename E, typename H, typename L >
    min::uns32 DISP
	    ( const packed_vec_insptr<E,H,L> S::* d )
    {
	return   OFFSETOF ( d )
	       + packed_vec_insptr<E,H,L>::DISP();
    }
}

namespace min {

    template < typename E,
               typename H = packed_vec_header<uns32>,
	       typename L = uns32 >
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
		( L max_length,
		  L length = 0,
		  E const * vp = NULL )
	{
	    return internal::packed_vec_new_stub<L>
		( this, max_length, length, vp );
	}
	const min::stub * new_stub ( void )
	{
	    return internal::packed_vec_new_stub<L>
		( this, initial_max_length, 0, NULL );
	}

	min::gen new_gen ( L max_length,
	                   L length = 0,
			   E const * vp = NULL )
	{
	    return new_stub_gen
	        ( internal::packed_vec_new_stub<L>
		    ( this, max_length, length, vp ) );
	}
	min::gen new_gen ( void )
	{
	    return min::new_stub_gen
	        ( internal::packed_vec_new_stub<L>
	              ( this, initial_max_length,
		        0, NULL ) );
	}

	static packed_id id;
    };

    template < typename E, typename H, typename B,
               typename L = min::uns32 >
    class packed_vec_with_base
	: public packed_vec<E,H,L>
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
	    : packed_vec<E,H,L>
	          ( name,
		    element_gen_disp,
		    element_stub_disp,
		    header_gen_disp,
		    header_stub_disp,
		    packed_struct<B>::id ) {}
    };

    template < typename E, typename H, typename L >
    internal::packed_vec_ptr_base<E,H,L>
            ::packed_vec_ptr_base
	    ( const min::stub * s ) : s ( s )
    {
	if ( s == NULL ) return;

        if ( type_of ( s ) != PACKED_VEC )
	{
	    this->s = NULL;
	    return;
	}

	uns32 subtype =
	    unprotected::packed_subtype_of ( s );
	packed_vec_descriptor * pvdescriptor =
	    (packed_vec_descriptor *)
	    (*packed_subtypes)[subtype];
	const packed_id * id = pvdescriptor->id;
	if ( & packed_vec<E,H,L>::id != id )
	{
	    this->s = NULL;
	    return;
	}
    }

    // Note: In the following index and length arguments
    // cannot be of type L as then if a lesser length
    // actual argument is provided type inference
    // fails.  This does not apply to member functions
    // or to min::internal functions (which always have
    // <L> as an actual template argument).

    template < typename E, typename H, typename L >
    inline min::ref<E> push
	( typename min::packed_vec_insptr<E,H,L> pvip )
    {
	if ( pvip->length >= pvip->max_length )
	    pvip.reserve ( 1 );
	E * p = ! end_ptr_of ( pvip );
	memset ( p, 0, sizeof ( E ) );
	++ * (L *) & pvip->length;
	return unprotected::new_ref ( pvip, * p );
    }
    template < typename E, typename H, typename L >
    inline void push
	( typename min::packed_vec_insptr<E,H,L> pvip,
	  min::unsptr n, E const * vp = NULL )
    {
	if ( n == 0 ) return;
	if ( pvip->length + n > pvip->max_length )
	    pvip.reserve ( n );
	E * p = ! end_ptr_of ( pvip );
	if ( vp )
	{
	    memcpy ( p, vp, n * sizeof ( E ) );
	    write_update<E> X ( pvip, p, n );
	}
	else
	    memset ( p, 0, n * sizeof ( E ) );
	* (L *) & pvip->length += n;
    }
    template < typename E, typename H, typename L >
    inline void push
	( typename min::packed_vec_insptr<E,H,L> pvip,
	  min::unsptr n, min::ptr<const E> vp )
    {
	if ( n == 0 ) return;
	if ( pvip->length + n > pvip->max_length )
	    pvip.reserve ( n );
	E * p = ! end_ptr_of ( pvip );
	memcpy ( p, ! vp, n * sizeof ( E ) );
	write_update<E> X ( pvip, p, n );
	* (L *) & pvip->length += n;
    }
    template < typename E, typename H, typename L >
    inline void push
	( typename min::packed_vec_insptr<E,H,L> pvip,
	  min::unsptr n, min::ptr<E> vp )
    {
        push ( pvip, n, (min::ptr<const E>) vp );
    }
    template < typename E, typename H, typename L >
    inline E pop
	( typename min::packed_vec_insptr<E,H,L> pvip )
    {
	MIN_ASSERT ( pvip->length > 0,
	             "vector is empty" );
	-- * (L *) & pvip->length;
	return * end_ptr_of ( pvip );
    }
    template < typename E, typename H, typename L >
    inline void pop
	( typename min::packed_vec_insptr<E,H,L> pvip,
	  min::unsptr n, E * vp = NULL )
    {
	MIN_ASSERT ( pvip->length >= n,
	             "second argument too large:"
		     " vector has too few elements" );
	* (L *) & pvip->length -= n;
	if ( vp )
	    memcpy ( vp,
		     ! end_ptr_of ( pvip ),
		     n * sizeof ( E ) );
    }
    template < typename E, typename H, typename L >
    inline void pop
	( typename min::packed_vec_insptr<E,H,L> pvip,
	  min::unsptr n, min::ptr<E> vp )
    {
        pop<E,H,L> ( pvip, ! vp );
    }

    template < typename E, typename H, typename L >
    inline void resize
	( typename min::packed_vec_insptr<E,H,L> pvip,
	  min::unsptr max_length )
    {
	pvip.resize ( max_length );
    }

    template < typename E, typename H, typename L >
    inline void reserve
	( typename min::packed_vec_insptr<E,H,L> pvip,
	  min::unsptr reserve_length )
    {
	if (   pvip->length + reserve_length
	     > pvip->max_length )
	    pvip.reserve ( reserve_length );
    }
}

// The following out-of-line functions should be in
// min.cc, BUT, since they are templates, they must be
// included in every compilation that might instantiate
// them.

template < typename E, typename H, typename L >
min::packed_id min::packed_vec<E,H,L>::id;

template < typename E, typename H, typename L >
min::packed_vec<E,H,L>::packed_vec
    ( const char * name,
      const min::uns32 * element_gen_disp,
      const min::uns32 * element_stub_disp,
      const min::uns32 * header_gen_disp,
      const min::uns32 * header_stub_disp,
      const packed_id & base_class_id )
    : internal::packed_vec_descriptor
          ( PACKED_VEC,
	    internal::packed_subtype_count ++,
            & id,
            name,

	    OFFSETOF<H,const L>
	        ( & H::length ),
	    OFFSETOF<H,const L>
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
	        ( & H::control ) == 0 ),
	  "`control' member of header is not first"
	  " member of the header" );

    min::internal::locatable_var_check
        < packed_vec_ptr<E,H,L> >();
    min::internal::locatable_var_check
        < packed_vec_updptr<E,H,L> >();
    min::internal::locatable_var_check
        < packed_vec_insptr<E,H,L> >();

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
        MIN_ASSERT ( id.base == NULL,
	             "packed_vec_with_base used twice"
		     " to declare same packed_vec"
		     " with two distinct bases" );
	id.base = & base_class_id;
    }
}

template < typename E, typename H, typename L >
void min::packed_vec_insptr<E,H,L>::reserve
	( L reserve_length )
{
    H * hp = (H *) unprotected::ptr_of ( this->s );
    uns32 subtype = hp->control;
    subtype &=
	internal::PACKED_CONTROL_SUBTYPE_MASK;
    internal::packed_vec_descriptor * pvdescriptor =
	(internal::packed_vec_descriptor *)
	(*internal::packed_subtypes)[subtype];
	
    L new_length =
          (L) pvdescriptor->increment_ratio
	* hp->max_length;
    if (   new_length
	 > pvdescriptor->max_increment )
	new_length =
	    pvdescriptor->max_increment;
    new_length += hp->max_length;
    L min_new_length =
	reserve_length + hp->length;
    if ( new_length < min_new_length )
	new_length = min_new_length;
    internal::packed_vec_resize<L>
	( this->s, pvdescriptor, new_length );
}

template < typename L >
const min::stub * min::internal::packed_vec_new_stub
	( min::internal::packed_vec_descriptor * pvd,
	  L max_length,
	  L length,
	  const void * vp )
{
    min::stub * s = min::unprotected::new_acc_stub();
    L size = pvd->header_size
           +   max_length
	     * pvd->element_size;
    min::unprotected::new_body ( s, size );
    uns8 * bodyp =
        (uns8 *) min::unprotected::ptr_of ( s );
    memset ( bodyp, 0, size );
    * (uns32 *) bodyp = pvd->subtype;
    * (L *) ( bodyp + pvd->length_disp ) = length;
    * (L *) ( bodyp + pvd->max_length_disp ) =
        max_length;
    if ( vp )
        memcpy ( bodyp + pvd->header_size,
	         vp, length * pvd->element_size);
    min::unprotected::set_type_of ( s, PACKED_VEC );
    return s;
}

template < typename L >
void min::internal::packed_vec_resize
        ( const min::stub * s,
	  L max_length )
{
    uns32 t = min::unprotected::packed_subtype_of ( s );
    packed_vec_descriptor * pvdescriptor =
	(packed_vec_descriptor *)
	(*packed_subtypes)[t];
    packed_vec_resize<L>
        ( s, pvdescriptor, max_length );
}

template < typename L >
void min::internal::packed_vec_resize
        ( const min::stub * s,
	  min::internal::packed_vec_descriptor * pvd,
	  L max_length )
{
    uns8 * & old_p = * (uns8 ** )
        & min::unprotected::ptr_ref_of
	      ( (min::stub *) s );
    L length =
        * (L *) ( old_p + pvd->length_disp );
    L old_max_length =
        * (L *) ( old_p + pvd->max_length_disp );
    unsptr copy_size = pvd->header_size
		     +   length
                       * pvd->element_size;
    unsptr old_size = pvd->header_size
		    +   old_max_length
                      * pvd->element_size;
    unsptr new_size = pvd->header_size
		    +   max_length
                      * pvd->element_size;
    if ( copy_size > new_size ) copy_size = new_size;
    min::unprotected::resize_body r
	( (min::stub *) s, new_size, old_size );
    uns8 * & new_p =
        * (uns8 **)
	& min::unprotected::new_body_ptr_ref ( r );
    memcpy ( new_p, old_p, copy_size );
    * (L *) ( new_p + pvd->max_length_disp ) =
        max_length;
    if ( length > max_length )
	* (L *) ( new_p + pvd->length_disp ) =
	    max_length;
}

MIN_STUB_PTR_CLASS ( typename S,
		     min::packed_struct_ptr<S> )
MIN_STUB_PTR_CLASS ( typename S,
		     min::packed_struct_updptr<S> )

MIN_STUB_PTR_CLASS ( typename E MIN_COMMA typename H
                                MIN_COMMA typename L,
		     min::packed_vec_ptr
			 < E MIN_COMMA H MIN_COMMA L > )
MIN_STUB_PTR_CLASS ( typename E MIN_COMMA typename H
                                MIN_COMMA typename L,
		     min::packed_vec_updptr
			 < E MIN_COMMA H MIN_COMMA L > )
MIN_STUB_PTR_CLASS ( typename E MIN_COMMA typename H
                                MIN_COMMA typename L,
		     min::packed_vec_insptr
			 < E MIN_COMMA H MIN_COMMA L > )

namespace min {

    extern packed_vec<min::gen> gen_packed_vec_type;
    extern packed_vec<char> char_packed_vec_type;
    extern packed_vec<min::uns32> uns32_packed_vec_type;
    extern packed_vec<const char *> 
        const_char_ptr_packed_vec_type;

}

// Files
// -----

namespace min {

    struct printer_struct;
    typedef packed_struct_updptr<printer_struct>
        printer;

    extern min::locatable_var<min::printer>
           error_message;
}

namespace min {

    struct file_struct;
    typedef min::packed_struct_updptr<file_struct> file;

    struct file_struct
    {
        const min::uns32 control;

	const min::packed_vec_insptr<char> buffer;
	min::uns32 end_offset;
	min::uns32 end_count;
	min::uns32 file_lines;
	min::uns32 next_line_number;
	min::uns32 next_offset;
	const min::packed_vec_insptr<min::uns32>
	    line_index;

	min::uns32 spool_lines;
	min::uns32 line_display;

	std::istream *		istream;
	const min::file		ifile;
	std::ostream * 		ostream;
	const min::printer	printer;
	const min::file		ofile;
	const min::gen		file_name;
    };

    MIN_REF ( min::packed_vec_insptr<char>,
              buffer, min::file )
    MIN_REF ( min::packed_vec_insptr<min::uns32>,
	      line_index,  min::file )
    MIN_REF ( min::file, ifile, min::file )
    MIN_REF ( min::printer, printer, min::file )
    MIN_REF ( min::file, ofile, min::file )
    MIN_REF ( min::gen, file_name, min::file )

    struct position
    {
	uns32	line;
	uns32	offset;
    };

    struct phrase_position
    {
        min::position begin;
        min::position end;
    };

    struct phrase_position_vec_header
    {
        const min::uns32 control;
        const min::uns32 length;
        const min::uns32 max_length;
	const min::file  file;
	min::phrase_position position;
    };
    typedef min::packed_vec_ptr
	    < phrase_position,
	      phrase_position_vec_header >
	phrase_position_vec;
    typedef min::packed_vec_insptr
	    < phrase_position,
	      phrase_position_vec_header >
	phrase_position_vec_insptr;

    MIN_REF ( min::file, file,
              min::phrase_position_vec_insptr )

    min::phrase_position_vec_insptr init
	    ( min::ref<min::phrase_position_vec_insptr>
		  vec,
	      min::file file,
	      const min::phrase_position & position,
	      min::uns32 max_length );

    class obj_vec_ptr;
    min::phrase_position_vec position_of
	    ( min::obj_vec_ptr & vp );

    const min::uns32 ALL_LINES = 0xFFFFFFFF;
    const min::uns32 NO_LINE   = 0xFFFFFFFF;

    void init ( min::ref<min::file> file );

    void init_line_display
	    ( min::ref<min::file> file,
	      min::uns32 line_display );

    void init_file_name
	    ( min::ref<min::file> file,
	      min::gen file_name );

    void init_ostream
	    ( min::ref<min::file> file,
	      std::ostream & ostream );

    void init_ofile
	    ( min::ref<min::file> file,
	      min::file ofile );

    void init_printer
	    ( min::ref<min::file> file,
	      min::printer printer );

    void init_input
            ( min::ref<min::file> file,
	      min::uns32 line_display = 0,
	      min::uns32 spool_lines = min::ALL_LINES );

    void init_input_stream
	    ( min::ref<min::file> file,
	      std::istream & istream,
	      min::uns32 line_display = 0,
	      min::uns32 spool_lines = min::ALL_LINES );

    void init_input_file
	    ( min::ref<min::file> file,
	      min::file ifile,
	      min::uns32 line_display = 0,
	      min::uns32 spool_lines = min::ALL_LINES );

    bool init_input_named_file
	    ( min::ref<min::file> file,
	      min::gen file_name,
	      min::uns32 line_display = 0,
	      min::uns32 spool_lines = min::ALL_LINES );

    void init_input_string
	    ( min::ref<min::file> file,
	      min::ptr<const char> string,
	      min::uns32 line_display = 0,
	      min::uns32 spool_lines = min::ALL_LINES );

    inline void init_input_string
	    ( min::ref<min::file> file,
	      min::ptr<char> string,
	      min::uns32 line_display = 0,
	      min::uns32 spool_lines = min::ALL_LINES )
    {
        init_input_string
	    ( file, (min::ptr<const char>) string,
	       line_display, spool_lines );
    }

    inline void init_input_string
	    ( min::ref<min::file> file,
	      const char * string,
	      min::uns32 line_display = 0,
	      min::uns32 spool_lines = min::ALL_LINES )
    {
        init_input_string
	    ( file, min::new_ptr ( string ),
	       line_display, spool_lines );
    }

    inline void end_line ( min::file file )
    {
        push(file->buffer) = 0;
	++ file->end_count;
	file->end_offset = file->buffer->length;
    }
    inline void end_line
	    ( min::file file, min::uns32 offset )
    {
	MIN_ASSERT ( offset < file->buffer->length,
	             "offset argument too large" );
	MIN_ASSERT ( offset >= file->end_offset,
	             "offset argument too small" );
        file->buffer[offset] = 0;
	file->end_offset = offset + 1;
	++ file->end_count;
    }

    inline void complete_file ( min::file file )
    {
        file->file_lines = file->end_count;
    }
    inline bool file_is_complete ( min::file file )
    {
        return file->file_lines != NO_LINE;
    }

    void load_string
	    ( min::file file,
	      min::ptr<const char> string );

    inline void load_string
	    ( min::file file,
	      min::ptr<char> string )
    {
        load_string
	    ( file, (min::ptr<const char>) string );
    }

    inline void load_string
	    ( min::file file,
	      const char * string )
    {
        load_string
	    ( file, min::new_ptr ( string ) );
    }

    bool load_named_file
	    ( min::file file,
	      min::gen file_name );

    min::uns32 next_line ( min::file file );
    min::uns32 line
	    ( min::file file,
	      min::uns32 line_number );

    inline min::uns32 remaining_length
	    ( min::file file )
    {
	return   file->buffer->length
	       - file->next_offset;
    }
    inline min::uns32 remaining_offset
	    ( min::file file )
    {
	return file->next_offset;
    }
    inline void skip_remaining ( min::file file )
    {
        file->next_offset = file->buffer->length;
    }

    inline min::uns32 partial_length
	    ( min::file file )
    {
	return   file->buffer->length
	       - file->end_offset;
    }
    inline min::uns32 partial_offset
	    ( min::file file )
    {
	return file->end_offset;
    }

    void flush_file
        ( min::file file,
	  bool copy_completion = true );
    void flush_line
	    ( min::file file, min::uns32 offset );
    void flush_remaining ( min::file file );
    void flush_spool
	    ( min::file file,
	      min::uns32 line_number = NO_LINE );

    void rewind ( min::file file,
                  min::uns32 line_number = 0 );

    min::uns32 print_line
    	    ( min::printer,
	      min::uns32 print_op_flags,
	      min::file file,
	      min::uns32 line_number,
	      const char * blank_line =
	          "<BLANK-LINE>",
	      const char * end_of_file =
	          "<END-OF-FILE>",
	      const char * unavailable_line =
	          "<UNAVALABLE-LINE>" );

    inline min::uns32 print_line
    	    ( min::printer printer,
	      min::file file,
	      min::uns32 line_number,
	      const char * blank_line =
	          "<BLANK-LINE>",
	      const char * end_of_file =
	          "<END-OF-FILE>",
	      const char * unavailable_line =
	          "<UNAVALABLE-LINE>" )
    {
        return print_line
	    ( printer, file->line_display, file,
	      line_number, blank_line, end_of_file,
	      unavailable_line );
    }

    struct print_format;
    min::uns32 print_line_column
	    ( min::file file,
	      const min::position & position,
	      min::uns32 line_display,
	      const min::print_format & print_format );

    void print_phrase_lines
	    ( min::printer printer,
	      min::uns32 line_display,
	      min::file file,
	      const min::phrase_position & position,
	      char mark = '^',
	      const char * blank_line =
	          "<BLANK-LINE>",
	      const char * end_of_file =
	          "<END-OF-FILE>",
	      const char * unavailable_line =
	          "<UNAVALABLE-LINE>" );

    inline void print_phrase_lines
	    ( min::printer printer,
	      min::file file,
	      const min::phrase_position & position,
	      char mark = '^',
	      const char * blank_line =
	          "<BLANK-LINE>",
	      const char * end_of_file =
	          "<END-OF-FILE>",
	      const char * unavailable_line =
	          "<UNAVALABLE-LINE>" )
    {
        return print_phrase_lines
	    ( printer, file->line_display, file,
	      position, mark, blank_line, end_of_file,
	      unavailable_line );
    }

    struct pline_numbers
    {
        min::file file;
	min::uns32 first, last;
	pline_numbers ( min::file file,
	                min::uns32 first,
			min::uns32 last )
	    : file ( file ),
	      first ( first ), last ( last ) {}
	pline_numbers ( min::file file,
	                const min::phrase_position
			    & position )
	    : file ( file ),
	      first ( position.begin.line ),
	      last  ( position.end.line )
	{
	    if ( last > first
	         &&
		 position.end.offset == 0 )
	        -- last;
	}
    };
}

min::printer operator <<
        ( min::printer printer,
	  const min::pline_numbers & pline_numbers );

std::ostream & operator <<
        ( std::ostream & out, min::file file );
min::file operator <<
        ( min::file ofile, min::file ifile );
min::printer operator <<
        ( min::printer printer, min::file file );

// Identifier Maps
// ---------- ----

namespace min {

    template < typename L >
    struct id_map_header
    {
        const min::uns32 control;

	const L length;
	const L max_length;

	L next;
	L occupied;

	min::uns32 hash_mask;
	min::uns32 hash_multiplier;
	L hash_max_offset;
	const min::packed_vec_insptr<L> hash_table;
	    // Given a stub,ID pair (s,id) then
	    //    stub_table[id] == s
	    //    hash_table[h] == id
	    // where h is obtained by entering (s,id)
	    // in the hash table using the algorithm:
	    //	  h = ::hash ( this_map, s );
	    //		// see min.cc for details
	    //    offset = 0;
	    //    while ( hash_table[h] != 0; )
	    //    {
	    //          h = ( h + 1 )
	    //            % hash_table->length;
	    //		++ offset;
	    //	  }
	    //    id = this->length;
	    //    push ( * this_map ) = s;
	    //    hash_table[h] = id;
	    //    if ( max_offset < offset )
	    //	      max_offset = offset;
	    //
	    // The hash_table is created by `find' if it
	    // does not exist.  If
	    //
	    //   hash_table->length < 2 * occupied
	    //
	    // the hash_table is reset to NULL_STUB so
	    // it will be recreated with a longer length
	    // by the next `find'.
    };
    
    typedef min::packed_vec_ptr
		< const min::stub *,
		  min::id_map_header<min::uns32>,
		  min::uns32 >
	    id_map;

    MIN_REF ( min::packed_vec_insptr<uns32>, hash_table,
              id_map );

    min::id_map init
            ( min::ref<min::id_map> map );
    uns32 find
            ( min::id_map map,
	      const min::stub * s );
    uns32 find_or_add
            ( min::id_map map,
	      const min::stub * s );
    void insert
            ( min::id_map map,
	      const min::stub * s,
	      min::uns32 id );
}

// UNICODE Name Tables
// ------- ---- ------

namespace min {

    namespace internal {
	struct unicode_name_entry
	{
	    min::Uchar c;
	    const min::ustring * name;
	};
    }
    
    typedef min::packed_vec_ptr
		< internal::unicode_name_entry >
	    unicode_name_table;

    const min::uns32 ALL_CHARS = 0xFFFFFFFF;

    min::unicode_name_table init
            ( min::ref<min::unicode_name_table> table,
	      const min::uns32 * char_flags =
	          min::standard_char_flags,
	      min::uns32 flags = min::ALL_CHARS,
	      min::uns32 extras = 10 );
    void add
            ( min::unicode_name_table table,
	      const min::ustring * name,
	      min::Uchar c );
    Uchar find
            ( min::unicode_name_table table,
	      const char * name );
}

// Objects
// -------

namespace min {

    // Object Flags: see below.
    //
    // OBJ_PUBLIC means vec_insptrs not allowed for
    // object.
    //
    // OBJ_PRIVATE means a vec_xxxptr exists for
    // the object and other vec_xxxptrs may not
    // be created for the object.
    //
    // OBJ_AUX means object may have aux stubs.  If this
    // flag is off, it means the object DEFINITELY does
    // NOT have aux stubs.
    //
    const unsigned OBJ_PRIVATE = ( 1 << 0 );
    const unsigned OBJ_PUBLIC  = ( 1 << 1 );
    const unsigned OBJ_AUX     = ( 1 << 3 );

    // OBJ_TYPE means object first variable (var[0])
    // points at the object's type.
    //
    const unsigned OBJ_TYPED   = ( 1 << 0 );

}

namespace min { namespace internal {

    // Bit 0 is low order.  N is the size of the flags,
    // codes, and offsets.
    //
    // Total Size Bits:
    //
    //    0		OBJ_PUBLIC
    //    1		OBJ_PRIVATE
    //    2		reserved
    //	  3		OBJ_AUX
    //    4..N-1	total size - 1
    //
    //  Unused Offset Bits:
    //
    //    0		OBJ_TYPED
    //	  1..3		reserved for more flags
    //	  4..N-1	unused offset - 1
    //
    //  Aux Offset Bits:
    //
    //	  0..3		reserved for more flags
    //	  4..N-1	unused offset - 1

    // Number of bits reserved for flags.
    //
    const unsigned OBJ_FLAG_BITS = 4;

    // Let object type be OBJ + T.  Then
    //
    //    Size of header in bytes is 1 << (T+2).
    //
    //    Size of total size and offset fields in bits
    //	  is ( 1 << (T+3) ) - OBJ_FLAG_BITS.
    //
    //    Size of hash and variable size fields in bits
    //    is 1 << (T+2).

    struct tiny_obj
    {
	min::uns8	total_size;
	min::uns8	var_size : 4;
	min::uns8	hash_size : 4;
	min::uns8	unused_offset;
	min::uns8	aux_offset;
    };

    struct short_obj
    {
        min::uns16	total_size;
        min::uns8	var_size;
        min::uns8	hash_size;
        min::uns16	unused_offset;
        min::uns16	aux_offset;
    };

    struct long_obj
    {
        min::uns32	total_size;
        min::uns16	var_size;
        min::uns16	hash_size;
        min::uns32	unused_offset;
        min::uns32	aux_offset;
    };

    struct huge_obj
    {
        min::uns64	total_size;
        min::uns32	var_size;
        min::uns32	hash_size;
        min::uns64	unused_offset;
        min::uns64	aux_offset;
    };

    // Object header size in bytes.
    //
    inline unsigned obj_header_size ( int type )
    {
        return 1 << ( type - min::OBJ + 2 );
    }

    // Object header size in min::gen values.
    //
    inline unsigned obj_header_gen_size ( int type )
    {
        return ( 1 << ( type - min::OBJ + 2 ) )
	       /
	       sizeof ( min::gen );
    }

    // Object header total size and offset bits.
    //
    inline unsigned obj_header_offset_bits ( int type )
    {
        return   ( 1 << ( type - min::OBJ + 3 ) )
	       - OBJ_FLAG_BITS;
    }

    // Object header hash and variable size bits.
    //
    inline unsigned obj_header_hash_bits ( int type )
    {
        return   ( 1 << ( type - min::OBJ + 2 ) );
    }

    // Offset of flags byte in object header.
    //
    inline unsigned obj_header_flags_offset ( int type )
    {
        return   MIN_IS_BIG_ENDIAN
	       * ( ( 1 << ( type - min::OBJ ) ) - 1 );
    }

} }

namespace min {

    min::gen new_obj_gen
	    ( min::unsptr unused_size,
	      min::unsptr hash_size = 0,
	      min::unsptr var_size = 0,
	      bool expand = true );

    inline bool is_obj ( min::gen g )
    {
	if ( ! is_stub ( g ) )
	    return false;
	const min::stub * s =
	    unprotected::stub_of ( g );
	int t = type_of ( s );
	return ( t & OBJ_MASK ) == OBJ;
    }
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
    const min::gen & var
	( min::obj_vec_ptr & vp, min::unsptr index );
    const min::gen & hash
	( min::obj_vec_ptr & vp, min::unsptr index );
    const min::gen & attr
	( min::obj_vec_ptr & vp, min::unsptr index );
    const min::gen & aux
	( min::obj_vec_ptr & vp, min::unsptr index );
    min::ptr<const min::gen> begin_ptr_of
	( min::obj_vec_ptr & vp );
    min::ptr<const min::gen> end_ptr_of
	( min::obj_vec_ptr & vp );

    namespace unprotected {
	min::gen * & base
	    ( min::obj_vec_updptr & vp );
    }
    min::ref<min::gen> var
	( min::obj_vec_updptr & vp, min::unsptr index );
    min::ref<min::gen> hash
	( min::obj_vec_updptr & vp, min::unsptr index );
    min::ref<min::gen> attr
	( min::obj_vec_updptr & vp, min::unsptr index );
    min::ref<min::gen> aux
	( min::obj_vec_updptr & vp, min::unsptr index );
    min::ptr<min::gen> begin_ptr_of
	( min::obj_vec_updptr & vp );
    min::ptr<min::gen> end_ptr_of
	( min::obj_vec_updptr & vp );

    namespace unprotected {
	min::unsptr & unused_offset_of
	    ( min::obj_vec_insptr & vp );
	min::unsptr & aux_offset_of
	    ( min::obj_vec_insptr & vp );
    }
    namespace internal {
	bool aux_flag_of
	    ( min::obj_vec_insptr & vp );
	void set_aux_flag_of
	    ( min::obj_vec_insptr & vp );
    }
    bool private_flag_of
	( min::obj_vec_ptr & vp );
    bool public_flag_of
	( min::obj_vec_ptr & vp );
    void set_public_flag_of
	( min::obj_vec_insptr & vp );

    min::unsptr hash_count_of
	( min::obj_vec_ptr & vp );
    void resize
	( min::obj_vec_insptr & vp,
	  min::unsptr unused_size,
	  min::unsptr var_size,
	  bool expand = true );
    void reorganize
	( min::obj_vec_insptr & vp,
	  min::unsptr hash_size,
	  min::unsptr var_size,
	  min::unsptr unused_size,
	  bool expand = true );
    void compact
	( min::obj_vec_insptr & vp,
	  min::unsptr var_size,
	  min::unsptr unused_size,
	  bool expand = true );

    ref<min::gen> attr_push
	( min::obj_vec_insptr & vp );
    void attr_push
	( min::obj_vec_insptr & vp,
	  min::unsptr n, const min::gen * p );
    ref<min::gen> aux_push
	( min::obj_vec_insptr & vp );
    void aux_push
	( min::obj_vec_insptr & vp,
	  min::unsptr n, const min::gen * p );

    min::gen attr_pop
	( min::obj_vec_insptr & vp );
    void attr_pop
	( min::obj_vec_insptr & vp,
	  min::unsptr n,
	  min::gen * p );
    min::gen aux_pop
	( min::obj_vec_insptr & vp );
    void aux_pop
	( min::obj_vec_insptr & vp,
	  min::unsptr n,
	  min::gen * p );

    class obj_vec_ptr
    {
    public:

	obj_vec_ptr ( const min::stub * s )
	    : type ( READONLY ), s ( (min::stub *) s )
	    { init(); }
	obj_vec_ptr ( min::gen g )
	    : type ( READONLY ),
	      s ( (min::stub *) min::stub_of ( g ) )
	    { init(); }
	obj_vec_ptr ( void )
	    : type ( READONLY), s ( NULL )
	    { init(); }

	~ obj_vec_ptr ( void )
	{
	    // If called by ~ obj_vec_updptr
	    // s == NULL but type == UPDATABLE.
	    //
	    if ( s == NULL ) return;
	    MIN_ASSERT ( type == READONLY,
	                 "system programmer error" );
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
		( min::gen g )
	{
	    this->~obj_vec_ptr();
	    new ( this ) obj_vec_ptr ( g );
	    return * this;
	}

	min::gen const & operator []
		( min::unsptr index ) const
	{
	    index += attr_offset;
	    MIN_ASSERT ( index < unused_offset,
	                 "subscript too large" );
	    return ( (min::gen const *)
		     unprotected::ptr_of (s) )
		   [index];
	}

	min::ptr<const min::gen> operator +
		( min::unsptr index ) const
	{
	    index += attr_offset;
	    MIN_ASSERT ( index < unused_offset,
	                 "subscript too large" );
	    return unprotected::new_ptr
	        ( s, ( (min::gen *)
		       unprotected::ptr_of (s) )
		     + index );
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

	friend const min::gen & var
	    ( min::obj_vec_ptr & vp,
	      min::unsptr index );
	friend const min::gen & hash
	    ( min::obj_vec_ptr & vp,
	      min::unsptr index );
	friend const min::gen & attr
	    ( min::obj_vec_ptr & vp,
	      min::unsptr index );
	friend const min::gen & aux
	    ( min::obj_vec_ptr & vp,
	      min::unsptr index );
	friend min::ptr<const min::gen>
	    min::begin_ptr_of
		( min::obj_vec_ptr & vp );
	friend min::ptr<const min::gen>
	    min::end_ptr_of
		( min::obj_vec_ptr & vp );

	friend min::gen * & unprotected::base
	    ( min::obj_vec_updptr & vp );
	friend min::ref<min::gen> var
	    ( min::obj_vec_updptr & vp,
	      min::unsptr index );
	friend min::ref<min::gen> hash
	    ( min::obj_vec_updptr & vp,
	      min::unsptr index );
	friend min::ref<min::gen> attr
	    ( min::obj_vec_updptr & vp,
	      min::unsptr index );
	friend min::ref<min::gen> aux
	    ( min::obj_vec_updptr & vp,
	      min::unsptr index );
	friend min::ptr<min::gen> min::begin_ptr_of
	    ( min::obj_vec_updptr & vp );
	friend min::ptr<min::gen> min::end_ptr_of
	    ( min::obj_vec_updptr & vp );

	friend min::unsptr &
	  unprotected::unused_offset_of
	    ( min::obj_vec_insptr & vp );
	friend min::unsptr & unprotected::aux_offset_of
	    ( min::obj_vec_insptr & vp );
	friend bool internal::aux_flag_of
	    ( min::obj_vec_insptr & vp );
	friend void internal::set_aux_flag_of
	    ( min::obj_vec_insptr & vp );
	friend bool private_flag_of
	    ( min::obj_vec_ptr & vp );
	friend bool public_flag_of
	    ( min::obj_vec_ptr & vp );
	friend void set_public_flag_of
	    ( min::obj_vec_insptr & vp );

	friend void resize
	    ( min::obj_vec_insptr & vp,
	      min::unsptr unused_size,
	      min::unsptr var_size,
	      bool expand );
	friend void reorganize
	    ( min::obj_vec_insptr & vp,
	      min::unsptr hash_size,
	      min::unsptr var_size,
	      min::unsptr unused_size,
	      bool expand );

	friend ref<min::gen> attr_push
	    ( min::obj_vec_insptr & vp );
	friend void attr_push
	    ( min::obj_vec_insptr & vp,
	      min::unsptr n, const min::gen * p );
	friend ref<min::gen> aux_push
	    ( min::obj_vec_insptr & vp );
	friend void aux_push
	    ( min::obj_vec_insptr & vp,
	      min::unsptr n, const min::gen * p );

	friend min::gen attr_pop
	    ( min::obj_vec_insptr & vp );
	friend void attr_pop
	    ( min::obj_vec_insptr & vp,
	      min::unsptr n,
	      min::gen * p );

	friend min::gen aux_pop
	    ( min::obj_vec_insptr & vp );
	friend void aux_pop
	    ( min::obj_vec_insptr & vp,
	      min::unsptr n,
	      min::gen * p );

    protected:

	obj_vec_ptr ( const min::stub * s, int type )
	    : type ( type ), s ( (min::stub *) s )
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

	min::uns8	total_size_flags;
	min::uns8	unused_offset_flags;
	min::uns8	aux_offset_flags;
	    // Flags word low order flag bits.

    private:

        void init ( void )
	{
	    if ( s == NULL )
	    {
		unused_offset = 0;
		aux_offset = 0;
		hash_size = 0;
		total_size = 0;
		var_offset = 0;
		hash_offset = 0;
		attr_offset = 0;
		total_size_flags = 0;
		unused_offset_flags = 0;
		aux_offset_flags = 0;
		return;
	    }

	    int type = type_of ( s );
	    uns8 * flags_p =
	        (uns8 *) unprotected::ptr_of ( s );
	    const unsigned SHIFT =
	        internal::OBJ_FLAG_BITS;
	    switch ( type )
	    {
	    case TINY_OBJ:
	    {
		internal::tiny_obj * hp =
		    (internal::tiny_obj *) flags_p;
		flags_p +=
		    internal::obj_header_flags_offset
		    	( TINY_OBJ );
		var_offset =
		    internal::obj_header_gen_size
		        ( TINY_OBJ );

		total_size_flags = hp->total_size;
		total_size =
		    ( hp->total_size >> SHIFT ) + 1;
		unused_offset_flags = hp->unused_offset;
		unused_offset =
		    ( hp->unused_offset >> SHIFT ) + 1;
		aux_offset_flags = hp->aux_offset;
		aux_offset =
		    ( hp->aux_offset >> SHIFT ) + 1;

		hash_offset = var_offset + hp->var_size;
		hash_size = hp->hash_size;
	    }
	        break;
	    case SHORT_OBJ:
	    {
		internal::short_obj * hp =
		    (internal::short_obj *) flags_p;
		flags_p +=
		    internal::obj_header_flags_offset
		    	( SHORT_OBJ );
		var_offset =
		    internal::obj_header_gen_size
		        ( SHORT_OBJ );

		total_size_flags = hp->total_size;
		total_size =
		    ( hp->total_size >> SHIFT ) + 1;
		unused_offset_flags = hp->unused_offset;
		unused_offset =
		    ( hp->unused_offset >> SHIFT ) + 1;
		aux_offset_flags = hp->aux_offset;
		aux_offset =
		    ( hp->aux_offset >> SHIFT ) + 1;

		hash_offset = var_offset + hp->var_size;
		hash_size = hp->hash_size;
	    }
	        break;
	    case LONG_OBJ:
	    {
		internal::long_obj * hp =
		    (internal::long_obj *) flags_p;
		flags_p +=
		    internal::obj_header_flags_offset
		    	( LONG_OBJ );
		var_offset =
		    internal::obj_header_gen_size
		        ( LONG_OBJ );

		total_size_flags = hp->total_size;
		total_size =
		    ( hp->total_size >> SHIFT ) + 1;
		unused_offset_flags = hp->unused_offset;
		unused_offset =
		    ( hp->unused_offset >> SHIFT ) + 1;
		aux_offset_flags = hp->aux_offset;
		aux_offset =
		    ( hp->aux_offset >> SHIFT ) + 1;

		hash_offset = var_offset + hp->var_size;
		hash_size = hp->hash_size;
	    }
	        break;
	    case HUGE_OBJ:
	    {
		internal::huge_obj * hp =
		    (internal::huge_obj *) flags_p;
		flags_p +=
		    internal::obj_header_flags_offset
		    	( HUGE_OBJ );
		var_offset =
		    internal::obj_header_gen_size
		        ( HUGE_OBJ );

		total_size_flags = hp->total_size;
		total_size =
		    ( hp->total_size >> SHIFT ) + 1;
		unused_offset_flags = hp->unused_offset;
		unused_offset =
		    ( hp->unused_offset >> SHIFT ) + 1;
		aux_offset_flags = hp->aux_offset;
		aux_offset =
		    ( hp->aux_offset >> SHIFT ) + 1;

		hash_offset = var_offset + hp->var_size;
		hash_size = hp->hash_size;
	    }
	        break;
	    default:
	    {
	        s = NULL;
		unused_offset = 0;
		aux_offset = 0;
		hash_size = 0;
		total_size = 0;
		var_offset = 0;
		hash_offset = 0;
		attr_offset = 0;
		total_size_flags = 0;
		unused_offset_flags = 0;
		aux_offset_flags = 0;
		return;
	    }
	    }

	    attr_offset = hash_offset + hash_size;

	    MIN_ASSERT
	        (    ( total_size_flags & OBJ_PRIVATE )
		  == 0,
		  "creating new object pointer to a"
		  " PRIVATE object" );

	    if ( total_size_flags & OBJ_PUBLIC )
	    {
	        MIN_ASSERT ( this->type != INSERTABLE,
		             "creating insptr pointer"
			     " to a PUBLIC object" );
	    }
	    else
	        * flags_p |= OBJ_PRIVATE;
	}

    protected:

	void deinit ( void )
	{
	    if ( s == NULL ) return;

	    int type = type_of ( s );
	    uns8 * body_p =
	        (uns8 *) unprotected::ptr_of ( s );
	    const unsigned SHIFT =
	        internal::OBJ_FLAG_BITS;
	    const unsigned MASK = ( 1 << SHIFT ) - 1;
	    switch ( type )
	    {
	    case TINY_OBJ:
	    {
		internal::tiny_obj * p =
		    (internal::tiny_obj *) body_p;

		p->total_size &= ~ OBJ_PRIVATE;
		if ( this->type == INSERTABLE )
		{
		    p->unused_offset =
		        (    ( unused_offset - 1 )
			  << SHIFT )
		      + ( unused_offset_flags & MASK );
		    p->aux_offset =
		        (    ( aux_offset - 1 )
			  << SHIFT )
		      + ( aux_offset_flags & MASK );
		}
	    }
	        break;
	    case SHORT_OBJ:
	    {
		internal::short_obj * p =
		    (internal::short_obj *) body_p;

		p->total_size &= ~ OBJ_PRIVATE;
		if ( this->type == INSERTABLE )
		{
		    p->unused_offset =
		        (    ( unused_offset - 1 )
			  << SHIFT )
		      + ( unused_offset_flags & MASK );
		    p->aux_offset =
		        (    ( aux_offset - 1 )
			  << SHIFT )
		      + ( aux_offset_flags & MASK );
		}
	    }
	        break;
	    case LONG_OBJ:
	    {
		internal::long_obj * p =
		    (internal::long_obj *) body_p;

		p->total_size &= ~ OBJ_PRIVATE;
		if ( this->type == INSERTABLE )
		{
		    p->unused_offset =
		        (    ( unused_offset - 1 )
			  << SHIFT )
		      + ( unused_offset_flags & MASK );
		    p->aux_offset =
		        (    ( aux_offset - 1 )
			  << SHIFT )
		      + ( aux_offset_flags & MASK );
		}
	    }
	        break;
	    case HUGE_OBJ:
	    {
		internal::huge_obj * p =
		    (internal::huge_obj *) body_p;

		p->total_size &= ~ OBJ_PRIVATE;
		if ( this->type == INSERTABLE )
		{
		    p->unused_offset =
		        (    ( unused_offset - 1 )
			  << SHIFT )
		      + ( unused_offset_flags & MASK );
		    p->aux_offset =
		        (    ( aux_offset - 1 )
			  << SHIFT )
		      + ( aux_offset_flags & MASK );
		}
	    }
	        break;
	    }

	    s = NULL;
	}
    };

    class obj_vec_updptr : public obj_vec_ptr {

    public:

	obj_vec_updptr ( const min::stub * s )
	    : obj_vec_ptr ( s, UPDATABLE )
	    {}
	obj_vec_updptr ( min::gen g )
	    : obj_vec_ptr
	          ( min::stub_of ( g ), UPDATABLE )
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
	    MIN_ASSERT ( type == UPDATABLE,
	                 "system programmer error" );
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
		( min::gen g )
	{
	    this->~obj_vec_updptr();
	    new ( this ) obj_vec_updptr ( g );
	    return * this;
	}

	min::ref<min::gen> operator []
		( min::unsptr index ) const
	{
	    index += attr_offset;
	    MIN_ASSERT ( index < unused_offset,
	                 "index argument too large" );
	    return unprotected::new_ref<min::gen>
	        ( s, ( (min::gen *)
		       unprotected::ptr_of (s) )
		     [index] );
	}

	min::ptr<min::gen> operator +
		( min::unsptr index ) const
	{
	    index += attr_offset;
	    MIN_ASSERT ( index < unused_offset,
	                 "index argument too large" );
	    return unprotected::new_ptr
	        ( s, ( (min::gen *)
		       unprotected::ptr_of (s) )
		     + index );
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
	obj_vec_insptr ( min::gen g )
	    : obj_vec_updptr
	          ( min::stub_of ( g ), INSERTABLE )
	    {}
	obj_vec_insptr ( void )
	    : obj_vec_updptr ( NULL, INSERTABLE )
	    {}

	~ obj_vec_insptr ( void )
	{
	    MIN_ASSERT ( type == INSERTABLE,
	                 "system programmer error" );
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
		( min::gen g )
	{
	    this->~obj_vec_insptr();
	    new ( this ) obj_vec_insptr ( g );
	    return * this;
	}
    };

    inline const min::gen * & unprotected::base
	( min::obj_vec_ptr & vp )
    {
	return * (const min::gen **) &
	       unprotected::ptr_ref_of ( vp.s );
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
    inline bool internal::aux_flag_of
	( min::obj_vec_insptr & vp )
    {
        return vp.total_size_flags & OBJ_AUX;
    }
    inline void internal::set_aux_flag_of
	( min::obj_vec_insptr & vp )
    {
        vp.total_size_flags |= OBJ_AUX;
    }
    inline bool private_flag_of
	( min::obj_vec_ptr & vp )
    {
        return vp.total_size_flags & OBJ_PRIVATE;
    }
    inline bool public_flag_of
	( min::obj_vec_ptr & vp )
    {
        return vp.total_size_flags & OBJ_PUBLIC;
    }
    inline void set_public_flag_of
	( min::obj_vec_insptr & vp )
    {
        vp.total_size_flags |= OBJ_PUBLIC;
	vp = NULL_STUB;
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
    inline min::unsptr size_of
	( min::obj_vec_ptr & vp )
    {
        return min::attr_size_of ( vp );
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

    inline const min::gen & var
	( min::obj_vec_ptr & vp, min::unsptr index )
    {
	index += vp.var_offset;
	MIN_ASSERT ( index < vp.hash_offset,
	             "index argument too large" );
	return unprotected::base(vp)[index];
    }

    inline const min::gen & hash
	( min::obj_vec_ptr & vp, min::unsptr index )
    {
	index += vp.hash_offset;
	MIN_ASSERT ( index < vp.attr_offset,
	             "index argument too large" );
	return unprotected::base(vp)[index];
    }

    inline const min::gen & attr
	( min::obj_vec_ptr & vp, min::unsptr index )
    {
	index += vp.attr_offset;
	MIN_ASSERT ( index < vp.unused_offset,
	             "index argument too large" );
	return unprotected::base(vp)[index];
    }

    inline min::gen const & aux
	( min::obj_vec_ptr & vp, min::unsptr index )
    {
	index = vp.total_size - index;
	MIN_ASSERT ( vp.aux_offset <= index,
	             "index argument too large" );
	MIN_ASSERT ( index < vp.total_size,
	             "index argument too large" );
	return unprotected::base(vp)[index];
    }

    inline min::ptr<const min::gen> begin_ptr_of
	( min::obj_vec_ptr & vp )
    {
	return unprotected::new_ptr
	    ( vp.s, ( (const min::gen *)
		   unprotected::ptr_of (vp.s) )
		 + vp.attr_offset );
    }

    inline min::ptr<const min::gen> end_ptr_of
	( min::obj_vec_ptr & vp )
    {
	return unprotected::new_ptr
	    ( vp.s, ( (const min::gen *)
		   unprotected::ptr_of (vp.s) )
		 + vp.unused_offset );
    }

    inline min::gen * & unprotected::base
	( min::obj_vec_updptr & vp )
    {
	return * (min::gen **) &
	       unprotected::ptr_ref_of ( vp.s );
    }

    inline min::ref<min::gen> var
	( min::obj_vec_updptr & vp, min::unsptr index )
    {
	index += vp.var_offset;
	MIN_ASSERT ( index < vp.hash_offset,
	             "index argument too large" );
	return unprotected::new_ref<min::gen>
	    ( vp.s, unprotected::base(vp)[index] );
    }

    inline min::ref<min::gen> hash
	( min::obj_vec_updptr & vp, min::unsptr index )
    {
	index += vp.hash_offset;
	MIN_ASSERT ( index < vp.attr_offset,
	             "index argument too large" );
	return unprotected::new_ref<min::gen>
	    ( vp.s, unprotected::base(vp)[index] );
    }

    inline min::ref<min::gen> attr
	( min::obj_vec_updptr & vp, min::unsptr index )
    {
	index += vp.attr_offset;
	MIN_ASSERT ( index < vp.unused_offset,
	             "index argument too large" );
	return unprotected::new_ref<min::gen>
	    ( vp.s, unprotected::base(vp)[index] );
    }

    inline min::ref<min::gen> aux
	( min::obj_vec_updptr & vp, min::unsptr index )
    {
	index = vp.total_size - index;
	MIN_ASSERT ( vp.aux_offset <= index,
	             "index argument too large" );
	MIN_ASSERT ( index < vp.total_size,
	             "index argument too large" );
	return unprotected::new_ref<min::gen>
	    ( vp.s, unprotected::base(vp)[index] );
    }

    inline min::ptr<min::gen> begin_ptr_of
	( min::obj_vec_updptr & vp )
    {
	return unprotected::new_ptr
	    ( vp.s, ( (min::gen *)
		   unprotected::ptr_of (vp.s) )
		 + vp.attr_offset );
    }

    inline min::ptr<min::gen> end_ptr_of
	( min::obj_vec_updptr & vp )
    {
	return unprotected::new_ptr
	    ( vp.s, ( (min::gen *)
		   unprotected::ptr_of (vp.s) )
		 + vp.unused_offset );
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

    inline void resize
	( min::obj_vec_insptr & vp,
	  min::unsptr unused_size )
    {
	resize ( vp, unused_size,
		     var_size_of ( vp ), false );
    }
    inline void expand
	( min::obj_vec_insptr & vp,
	  min::unsptr unused_size )
    {
	resize ( vp, unused_size,
		     var_size_of ( vp ), true );
    }
    inline void resize
	( min::gen obj,
	  min::unsptr unused_size,
	  min::unsptr var_size,
	  bool expand = true )
    {
        min::obj_vec_insptr vp ( obj );
	resize ( vp, unused_size, var_size, expand );
    }
    inline void resize
	( min::gen obj,
	  min::unsptr unused_size )
    {
        min::obj_vec_insptr vp ( obj );
	resize ( vp, unused_size,
	             var_size_of ( vp ), false );
    }
    inline void expand
	( min::gen obj,
	  min::unsptr unused_size )
    {
        min::obj_vec_insptr vp ( obj );
	resize ( vp, unused_size,
	         var_size_of ( vp ), true );
    }
    inline void reorganize
	( min::gen obj,
	  min::unsptr hash_size,
	  min::unsptr var_size,
	  min::unsptr unused_size,
	  bool expand = true )
    {
        min::obj_vec_insptr vp ( obj );
	reorganize
	    ( vp, hash_size, var_size, unused_size,
	          expand );
    }
    inline void compact
	( min::obj_vec_insptr & vp )
    {
	compact ( vp, var_size_of ( vp ), 0, false );
    }
    inline void compact
	( min::gen obj,
	  min::unsptr var_size,
	  min::unsptr unused_size,
	  bool expand = true )
    {
        min::obj_vec_insptr vp ( obj );
	compact ( vp, var_size, unused_size, expand );
    }
    inline void compact
	( min::gen obj )
    {
        min::obj_vec_insptr vp ( obj );
	compact ( vp, var_size_of ( vp ), 0, false );
    }
    inline void publish
	( min::obj_vec_insptr & vp )
    {
	compact ( vp, var_size_of ( vp ), 0, false );
	set_public_flag_of ( vp );
    }
    inline void publish
	( min::gen obj )
    {
        min::obj_vec_insptr vp ( obj );
	compact ( vp, var_size_of ( vp ), 0, false );
	set_public_flag_of ( vp );
    }

    inline bool private_flag_of ( min::gen obj )
    {
	if ( ! min::is_stub ( obj ) ) return false;
	const min::stub * s =
	    min::unprotected::stub_of ( obj );
	int type = min::unprotected::type_of ( s );
	if ( ( type & OBJ_MASK ) != OBJ ) return false;
	return     (   ( (const uns8 *)
	                 min::unprotected
			    ::ptr_of ( s ) )
	               [min::internal
		           ::obj_header_flags_offset
	                           ( type )]
		     & min::OBJ_PRIVATE )
		!= 0;
    }

    inline bool public_flag_of ( min::gen obj )
    {
	if ( ! min::is_stub ( obj ) ) return false;
	const min::stub * s =
	    min::unprotected::stub_of ( obj );
	int type = min::unprotected::type_of ( s );
	if ( ( type & OBJ_MASK ) != OBJ ) return false;
	return     (   ( (const uns8 *)
	                 min::unprotected
			    ::ptr_of ( s ) )
	               [min::internal
		           ::obj_header_flags_offset
	                           ( type )]
		     & min::OBJ_PUBLIC )
		!= 0;
    }

    inline ref<min::gen> attr_push
	( min::obj_vec_insptr & vp )
    {
	if ( vp.unused_offset >= vp.aux_offset )
	    expand ( vp, 1 );
	return unprotected::new_ref<min::gen>
	    ( vp.s, unprotected::base(vp)
	                [vp.unused_offset ++] );
    }

    inline void attr_push
	( min::obj_vec_insptr & vp,
	  min::unsptr n,
	  const min::gen * p = NULL )
    {
	min::unsptr m = unused_size_of ( vp );
	if ( m < n ) expand ( vp, n );

	if ( p == NULL )
	    memset (   unprotected::base(vp)
		     + vp.unused_offset,
		     0, sizeof ( min::gen ) * n );
	else
	{
	    memcpy (   unprotected::base(vp)
		     + vp.unused_offset,
		     p, sizeof ( min::gen ) * n );
	    unprotected::acc_write_update
	        ( vp.s, p, n );
        }
	vp.unused_offset += n;
    }

    inline ref<min::gen> aux_push
	( min::obj_vec_insptr & vp )
    {
	if ( vp.unused_offset >= vp.aux_offset )
	    expand ( vp, 1 );
	return unprotected::new_ref<min::gen>
	    ( vp.s, unprotected::base(vp)
	                [-- vp.aux_offset] );
    }

    inline void aux_push
	( min::obj_vec_insptr & vp,
	  min::unsptr n, const min::gen * p = NULL )
    {
	min::unsptr m = unused_size_of ( vp );
	if ( m < n ) expand ( vp, n );

	vp.aux_offset -= n;
	if ( p == NULL )
	    memset (   unprotected::base(vp)
	             + vp.aux_offset,
		     0, sizeof ( min::gen ) * n );
	else
	{
	    memcpy (   unprotected::base(vp)
	             + vp.aux_offset,
		     p, sizeof ( min::gen ) * n );
	    unprotected::acc_write_update
	        ( vp.s, p, n );
	}
    }

    inline min::gen attr_pop
	( min::obj_vec_insptr & vp )
    {
	MIN_ASSERT
	    ( vp.attr_offset < vp.unused_offset,
	      "no attribute left to pop" );
	return unprotected::base(vp)
		   [-- vp.unused_offset];
    }

    inline void attr_pop
	( min::obj_vec_insptr & vp,
	  min::unsptr n,
	  min::gen * p = NULL )
    {
	MIN_ASSERT
	    ( vp.attr_offset + n <= vp.unused_offset,
	      "less than n attributes left to pop" );
	vp.unused_offset -= n;
	if ( p != NULL )
	    memcpy
		( p,
		    unprotected::base(vp)
		  + vp.unused_offset,
		  sizeof ( min::gen ) * n );
    }

    inline min::gen aux_pop
	( min::obj_vec_insptr & vp )
    {
	MIN_ASSERT ( vp.aux_offset < vp.total_size,
	             "no aux element left to pop" );
	return unprotected::base(vp)[vp.aux_offset ++];
    }

    inline void aux_pop
	( min::obj_vec_insptr & vp,
	  min::unsptr n,
	  min::gen * p = NULL )
    {
	MIN_ASSERT
	    ( vp.aux_offset + n <= vp.total_size,
	      "less than n aux elements left to pop" );
	if ( p != NULL )
	    memcpy ( p,
		       unprotected::base(vp)
		     + vp.aux_offset,
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
// value NONE(), and can be garbage collected when the
// object is reorganized.  Because they are often
// isolated, no attempt is made to put them on a free
// list.

namespace min { namespace unprotected {

    // This is the generic list pointer type from which
    // specific list pointer types are made.

    template < class vecptr >
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

    inline min::gen LIST_END ( void )
    {
	return unprotected::new_gen
	    ( (unsgen) GEN_LIST_AUX << VSIZE );
    }
    inline min::gen EMPTY_SUBLIST ( void )
    {
	return unprotected::new_gen
	    ( (unsgen) GEN_SUBLIST_AUX << VSIZE );
    }

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
	    if ( is_stub ( v ) )
	    {
		min::stub * s =
		    unprotected::stub_of ( v );
		if ( type_of ( s ) == SUBLIST_AUX )
		    remove_list
		        ( base, total_size, 0, s );
	    }
	    else
#       endif
	if ( is_sublist_aux ( v )
	     &&
	     v != EMPTY_SUBLIST() )
	    remove_list
	        ( base, total_size,
		    total_size
		  - sublist_aux_of ( v ) );
    }
} }

namespace min {

    inline bool is_list_end ( min::gen v )
    {
        return v == LIST_END();
    }
    inline bool is_sublist ( min::gen v )
    {
        return is_sublist_aux ( v )
#              if MIN_USE_OBJ_AUX_STUBS
		   ||
		   ( is_stub ( v )
		     &&
		        type_of
		          ( unprotected::stub_of ( v ) )
		     == SUBLIST_AUX )
#	       endif
	       ;
    }
    inline bool is_empty_sublist ( min::gen v )
    {
        return v == EMPTY_SUBLIST();
    }

    // We must declare these before we make them
    // friends.

    template < class vecptr >
    vecptr & obj_vec_ptr_of
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp );

    template < class vecptr >
    min::unsptr hash_size_of
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp );

    template < class vecptr >
    min::unsptr attr_size_of
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp );

    template < class vecptr >
    min::gen start_hash
            ( min::unprotected
	         ::list_ptr_type<vecptr> & lp,
	      min::unsptr index );
    template < class vecptr >
    min::gen start_vector
            ( min::unprotected
	         ::list_ptr_type<vecptr> & lp,
	      min::unsptr index );
    template < class vecptr, class vecptr2 >
    min::gen start_copy
            ( min::unprotected
	         ::list_ptr_type<vecptr> & lp,
	      min::unprotected
	         ::list_ptr_type<vecptr2> & lp2 );
    template < class vecptr, class vecptr2 >
    min::gen start_sublist
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp,
    	      min::unprotected
	         ::list_ptr_type<vecptr2> & lp2 );
    template < class vecptr >
    min::gen start_sublist
    	    ( min::list_insptr & lp,
    	      min::unprotected
	         ::list_ptr_type<vecptr> & lp2 );

    template < class vecptr >
    min::gen next
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp );
    template < class vecptr >
    min::gen peek
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp );
    template < class vecptr >
    min::gen current
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp );
    template < class vecptr >
    min::gen update_refresh
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp );
    template < class vecptr >
    min::gen insert_refresh
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp );

    template < class vecptr >
    void update
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp,
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
	      hash_offset ( 0 ), aux_offset ( 0 ),
	      total_size ( 0 )
	{
	    // An unstarted list pointer behaves as if
	    // it were pointing at the end of a list
	    // for which insertions are illegal.
	    //
	    current = LIST_END();

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
	    // MIN_ASSERT violation).

	min::unsptr hash_offset;
	min::unsptr aux_offset;
	min::unsptr total_size;
	    // Save of hash_offset, aux_offset, and
	    // total_size from obj_vec pointer.  0 if
	    // list pointer has not yet been started.
	    //
	    // Hash_offset and total_size are unsed by
	    // refresh to adjust indices if resize has
	    // been called for another list pointer to
	    // the same object.
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
	    //
	    // Aux_offset is used by next(...) to detect
	    // badly formatted objects.

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
	// The previous pointer is only maintained and
	// used by min::list_insptr's, and NOT by min::
	// list_ptr's or min::list_updptr's.
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
	//	   and current == LIST_END()
	//
	// The previous pointer (used only by min::list_
	// insptr's) is similar, but also differs when
	// it points at a stub, since it could be point-
	// ing at either a sublist or list head:
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
	// Here `previous' is not stored in the list_
	// insptr, but is computed as needed.
	//
	// If a previous value is a stub pointer, then
	// it points at an auxiliary stub, and is
	// treated as a sublist pointer if the auxiliary
	// stub is of type SUBLIST_AUX, and is treated
	// as a list pointer if the stub is of type
	// LIST_AUX.
	//
	// If current == LIST_END() one of the following
	// special cases applies.
	//
	//   Current pointer exists:
	//	current_index != 0
	//	base[current_index] == LIST_END()
	//	previous pointer does NOT exist
	//
	//   Current pointer does NOT exist,
	//   previous pointer points at list head in the
	//	    attribute vector or hash table:
	//   previous_is_sublist_header == false:
	//      [current is the virtual LIST_END() after
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
	//      [current is the virtual LIST_END() at
	//	 the end of an EMPTY_SUBLIST()]
	//      previous == EMPTY_SUBLIST()
	//
	//   Current pointer does NOT exist,
	//   previous pointer exists and points at a
	//            stub,
	//   previous_is_sublist_header == false:
	//      [current is the LIST_END() in the stub
	//       control]
	//      control_of ( previous_stub ) ==
	//	    LIST_END()
	//
	//   Current pointer does NOT exist,
	//   previous pointer does NOT exist:
	//      [current is virtual LIST_END() of a list
	//       pointer that has never been started]
	//      The list pointer has never been started
	//      by a start_... function.   All
	//      operations should treat the pointer
	//      as pointing at an empty list into
	//      which insertions CANNOT be made.
	//
	// From the above the following can be deduced:
	//
	//   (1) If the current pointer points at a
	//       stub, the previous pointer must exist.
	//
	//   (2) If the previous pointer does not exist
	//       and current != LIST_END(), then
	//       current_index != 0 and current_stub ==
	//       NULL.
	//
	//   (3) If the previous pointer does not exist
	//       and current == LIST_END(), then either
	//       current_index != 0 or the list pointer
	//       was never started (e.g., by start_
	//       hash).
	//
	// If the current value is not LIST_END(), the
	// current pointer must exist.
	//
	// A current value can be LIST_END() or EMPTY_
	// SUBLIST, but cannot be a list or sublist
	// pointer (and in particular cannot point
	// at a stub of LIST_AUX or SUBLIST_AUX type).
	//
	// A list or sublist pointer cannot point at
	// a list pointer or LIST_END().

    // Friends:

	friend min::obj_vec_insptr & obj_vec_ptr_of<>
		( min::list_insptr & lp );

	friend min::unsptr hash_size_of<>
		( min::list_insptr & lp );

	friend min::unsptr attr_size_of<>
		( min::list_insptr & lp );

	friend min::gen min::start_hash<>
		( min::list_insptr & lp,
		  min::unsptr index );
	friend min::gen min::start_vector<>
		( min::list_insptr & lp,
		  min::unsptr index );

	friend min::gen start_copy<>
		( min::list_insptr & lp,
		  min::list_insptr & lp2 );

	friend min::gen start_sublist<>
		( min::list_ptr & lp,
		  min::list_insptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_updptr & lp,
		  min::list_insptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
		  min::list_ptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
		  min::list_updptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
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

	    if ( is_list_aux ( current ) )
	    {
		if ( current != LIST_END() )
		{
		    previous_index = current_index;
		    current_index =
			  total_size
			- min::list_aux_of ( current );
		    current = base[current_index];
		}
	    }
#           if MIN_USE_OBJ_AUX_STUBS
		else if ( is_stub ( current ) )
		{
		    min::stub * s =
		      unprotected::stub_of ( current );
		    int type =
		        unprotected::type_of ( s );

		    if ( type == LIST_AUX )
		    {
		        previous_index = current_index;
			current_index = 0;
			current_stub = s;
			current =
			    unprotected::gen_of ( s );
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
    template < class vecptr >
	class list_ptr_type {

    public:

        list_ptr_type ( vecptr & vecp )
	    : vecp ( vecp ),
	      base ( * (min::gen **) &
	             unprotected::base ( vecp ) ),
	      hash_offset ( 0 ), aux_offset ( 0 ),
	      total_size ( 0 )
	{
	    current = LIST_END();
	    current_index = 0;
	    head_index = 0;
#	    if MIN_USE_OBJ_AUX_STUBS
		current_stub = NULL;
#	    endif
	}

    private:

    // Private Data:

	vecptr & vecp;
	min::gen * & base;
	min::gen current;
	min::unsptr current_index;
	min::unsptr head_index;

#	if MIN_USE_OBJ_AUX_STUBS
	    min::stub * current_stub;
#	endif

	min::unsptr hash_offset;
	min::unsptr aux_offset;
	min::unsptr total_size;

    // Friends:

	friend vecptr & obj_vec_ptr_of<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp );

	friend min::unsptr hash_size_of<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp );

	friend min::unsptr attr_size_of<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp );

	friend min::gen min::start_hash<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp,
		  min::unsptr index );
	friend min::gen min::start_vector<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp,
		  min::unsptr index );

	friend min::gen start_copy<>
		( min::list_ptr & lp,
		  min::list_ptr & lp2 );
	friend min::gen start_copy<>
		( min::list_ptr & lp,
		  min::list_updptr & lp2 );
	friend min::gen start_copy<>
		( min::list_ptr & lp,
		  min::list_insptr & lp2 );
	friend min::gen start_copy<>
		( min::list_updptr & lp,
		  min::list_updptr & lp2 );
	friend min::gen start_copy<>
		( min::list_updptr & lp,
		  min::list_insptr & lp2 );

	friend min::gen start_sublist<>
		( min::list_ptr & lp,
		  min::list_ptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_ptr & lp,
		  min::list_updptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_ptr & lp,
		  min::list_insptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_updptr & lp,
		  min::list_ptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_updptr & lp,
		  min::list_updptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_updptr & lp,
		  min::list_insptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
		  min::list_ptr & lp2 );
	friend min::gen start_sublist<>
		( min::list_insptr & lp,
		  min::list_updptr & lp2 );

	friend min::gen min::next<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp );
	friend min::gen min::peek<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp );
	friend min::gen min::current<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp );
	friend min::gen min::update_refresh<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp );
	friend min::gen min::insert_refresh<>
		( min::unprotected
		     ::list_ptr_type<vecptr> & lp );

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

	    if ( is_list_aux ( current ) )
	    {
		if ( current != LIST_END() )
		{
		    current_index =
		          total_size
			- min::list_aux_of ( current );
		    current = base[current_index];
		}
	    }
#           if MIN_USE_OBJ_AUX_STUBS
		else if ( is_stub ( current ) )
		{
		    min::stub * s =
		      unprotected::stub_of ( current );
		    int type =
		        unprotected::type_of ( s );

		    if ( type == LIST_AUX )
		    {
			current_index = 0;
			current_stub = s;
			current =
			    unprotected::gen_of ( s );
		    }
		}
#           endif

	    return current;
	}
    };

    min::unsptr list_element_count
	    ( min::obj_vec_ptr & vp,
	      min::unsptr index
#	    if MIN_USE_OBJ_AUX_STUBS
	      , const min::stub * s = NULL
#	    endif
	);

} }

namespace min {

    // Inline functions.  See MIN design document.

    template < class vecptr >
    inline vecptr & obj_vec_ptr_of
	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp )
    {
    	return lp.vecp;
    }

    template < class vecptr >
    inline min::unsptr hash_size_of
	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp )
    {
    	return min::hash_size_of ( lp.vecp );
    }

    template < class vecptr >
    inline min::unsptr attr_size_of
	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp )
    {
    	return min::attr_size_of ( lp.vecp );
    }

    template < class vecptr >
    inline min::gen start_hash
            ( min::unprotected
	         ::list_ptr_type<vecptr> & lp,
	      min::unsptr index )
    {
	lp.hash_offset =
	    unprotected::hash_offset_of ( lp.vecp );
	lp.aux_offset =
	    unprotected::aux_offset_of ( lp.vecp );
	lp.total_size = total_size_of ( lp.vecp );

	index %= hash_size_of ( lp );

	lp.head_index = lp.hash_offset + index;
	return lp.forward ( lp.head_index );
    }

    template < class vecptr >
    inline min::gen start_vector
            ( min::unprotected
	         ::list_ptr_type<vecptr> & lp,
	      min::unsptr index )
    {
	lp.hash_offset =
	    unprotected::hash_offset_of ( lp.vecp );
	lp.aux_offset =
	    unprotected::aux_offset_of ( lp.vecp );
	lp.total_size = total_size_of ( lp.vecp );

	lp.head_index =
	      unprotected::attr_offset_of ( lp.vecp )
	    + index;
	MIN_ASSERT
	    (   lp.head_index
	      < unprotected::unused_offset_of
	            ( lp.vecp ),
	      "index argument too large" );
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
    template < class vecptr, class vecptr2 >
    inline min::gen start_copy
            ( min::unprotected
	         ::list_ptr_type<vecptr> & lp,
	      min::unprotected
	         ::list_ptr_type<vecptr2> & lp2 )
    {
	lp.hash_offset =
	    unprotected::hash_offset_of ( lp.vecp );
	lp.aux_offset =
	    unprotected::aux_offset_of ( lp.vecp );
	lp.total_size = total_size_of ( lp.vecp );

        MIN_ASSERT ( & lp.vecp == & lp2.vecp,
	             "list arguments do not have same"
		     " object vector pointer" );
	MIN_ASSERT ( lp.total_size == lp2.total_size,
	             "second list argument not up to"
		     " date (refreshed)" );

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
	      min::list_insptr & lp2 )
    {
	MIN_ASSERT ( lp2.hash_offset != 0,
	             "second list pointer argument"
		     " has not been started" );

	lp.hash_offset =
	    unprotected::hash_offset_of ( lp.vecp );
	lp.aux_offset =
	    unprotected::aux_offset_of ( lp.vecp );
	lp.total_size = total_size_of ( lp.vecp );

        MIN_ASSERT ( & lp.vecp == & lp2.vecp,
	             "list arguments do not have same"
		     " object vector pointer" );
	MIN_ASSERT ( lp.total_size == lp2.total_size,
	             "second list argument not up to"
		     " date (refreshed)" );

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
    template < class vecptr, class vecptr2 >
    inline min::gen start_sublist
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp,
    	      min::unprotected
	         ::list_ptr_type<vecptr2> & lp2 )
    {
	MIN_ASSERT ( lp2.hash_offset != 0,
	             "second list pointer argument"
		     " has not been started" );

	// We want the total size check to work even
	// if & lp == & lp2.
	//
	lp.hash_offset =
	    unprotected::hash_offset_of ( lp.vecp );
	lp.aux_offset =
	    unprotected::aux_offset_of ( lp.vecp );
	unsptr total_size = total_size_of ( lp.vecp );

        MIN_ASSERT ( & lp.vecp == & lp2.vecp,
	             "list arguments do not have same"
		     " object vector pointer" );
	MIN_ASSERT ( total_size == lp2.total_size,
	             "second list argument not up to"
		     " date (refreshed)" );
	lp.total_size = total_size;

	lp.head_index = lp2.head_index;

#	if MIN_USE_OBJ_AUX_STUBS
	    if ( is_stub ( lp2.current ) )
	    {
		lp.current_stub =
		  unprotected::stub_of ( lp2.current );
		lp.current_index = 0;
		MIN_ASSERT
		    (    type_of ( lp.current_stub )
		      == SUBLIST_AUX,
		      "system programming error" );
		lp.current =
		    unprotected::gen_of
		        ( lp.current_stub );
		return lp.current;
	    }
	    lp.current_stub = NULL;
#	endif

	lp.current_index =
	    sublist_aux_of ( lp2.current );
	if ( lp.current_index == 0 )
	    lp.current = LIST_END();
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
    template <class vecptr>
    inline min::gen start_sublist
    	    ( min::list_insptr & lp,
    	      min::unprotected
	         ::list_ptr_type<vecptr> & lp2 )
    {
	// We want the total size check to work even
	// if & lp == & lp2.
	//
	lp.hash_offset =
	    unprotected::hash_offset_of ( lp.vecp );
	lp.aux_offset =
	    unprotected::aux_offset_of ( lp.vecp );
	unsptr total_size = total_size_of ( lp.vecp );

        MIN_ASSERT ( & lp.vecp == & lp2.vecp,
	             "list arguments do not have same"
		     " object vector pointer" );
	MIN_ASSERT ( total_size == lp2.total_size,
	             "second list argument not up to" );
	lp.total_size = total_size;

	lp.previous_index = lp2.current_index;
	lp.head_index = lp2.head_index;
	lp.previous_is_sublist_head = true;

#	if MIN_USE_OBJ_AUX_STUBS
	    lp.previous_stub = lp2.current_stub;
	    if ( is_stub ( lp2.current ) )
	    {
		lp.current_stub =
		  unprotected::stub_of ( lp2.current );
		lp.current_index = 0;
		MIN_ASSERT
		    (    type_of ( lp.current_stub )
		      == SUBLIST_AUX,
		      "system programming error");
		lp.current =
		    unprotected::gen_of
		        ( lp.current_stub );
		return lp.current;
	    }
	    lp.current_stub = NULL;
#	endif

	lp.current_index =
	    sublist_aux_of ( lp2.current );
	if ( lp.current_index == 0 )
	    lp.current = LIST_END();
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

    template < class vecptr >
    inline min::gen next
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp )
    {
        if ( lp.current == LIST_END() )
	    return LIST_END();

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
	        uns64 c = unprotected::control_of
				( lp.current_stub );
		if ( c & unprotected::STUB_PTR )
		{
		    lp.current_stub =
		        unprotected::stub_of_control
			    ( c );
		    return
		        lp.current =
			    unprotected::gen_of
				   ( lp.current_stub );
		}
		else
		{
		    lp.current_index =
		        unprotected::value_of_control
			    ( c );
		    lp.current_stub = NULL;
		    if ( lp.current_index == 0 )
			lp.current = LIST_END();
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
	    // Current is list (not sublist) head and
	    // current pointer ceases to exist when
	    // we move to the next element.
	    //
	    lp.current_index = 0;
	    return lp.current = LIST_END();
	}
	else
	{
	    MIN_ASSERT
	        ( lp.current_index > lp.aux_offset,
		  "system programming error" );
	    return lp.forward ( lp.current_index - 1 );
	}
    }
    template <>
    inline min::gen next
    	    ( min::list_insptr & lp )
    {
        // The code of remove ( lp ) depends upon the
	// fact that this function does not READ
	// lp.previous_stub or lp.previous_index.

        if ( lp.current == LIST_END() )
	    return LIST_END();

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
		lp.previous_index = 0;
		lp.previous_is_sublist_head = false;
		lp.previous_stub = lp.current_stub;

	        uns64 c = unprotected::control_of
				( lp.current_stub );
		if ( c & unprotected::STUB_PTR )
		{
		    lp.current_stub =
		        unprotected::stub_of_control
			    ( c );
		    return
		        lp.current =
			    unprotected::gen_of
				   ( lp.current_stub );
		}
		else
		{
		    lp.current_stub = NULL;

		    lp.current_index =
		        unprotected::value_of_control
			    ( c );
		    if ( lp.current_index == 0 )
			lp.current = LIST_END();
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
	    // not LIST_END().
	    //
	    lp.previous_index = lp.current_index;
	    lp.current_index = 0;
	    return lp.current = LIST_END();
	}
	else
	{
	    MIN_ASSERT
	        ( lp.current_index > lp.aux_offset,
		  "system programming error" );
	    return lp.forward ( lp.current_index - 1 );
	}
    }

    template < class vecptr >
    inline min::gen peek
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp )
    {
        if ( lp.current == LIST_END() )
	    return LIST_END();

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
	        uns64 c =unprotected::control_of
		    	( lp.current_stub );
		if ( c & unprotected::STUB_PTR )
		{
		    min::stub * s =
		        unprotected::stub_of_control
			    ( c );
		    return unprotected::gen_of ( s );
		}
		else
		{
		    unsptr index =
		        unprotected::value_of_control
			    ( c );
		    if ( index == 0 )
			return LIST_END();
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
	    return LIST_END();
	}
	else
	{
	    unsptr index = lp.current_index;
	    MIN_ASSERT ( index > lp.aux_offset,
	                 "system programming error" );
	    -- index;
	    min::gen c = lp.base[index];

	    if ( is_list_aux ( c ) )
	    {
		index = list_aux_of ( c );
		if ( index == 0 )
		    return LIST_END();
		else
		{
		    index = lp.total_size - index;
		    return lp.base[index];
		}
	    }
#           if MIN_USE_OBJ_AUX_STUBS
		else if ( is_stub ( c ) )
		{
		    min::stub * s =
		        unprotected::stub_of ( c );
		    int type = type_of ( s );

		    if ( type == LIST_AUX )
		    {
			return
			    unprotected::gen_of ( s );
		    }
		}
#           endif

	    else
		return c;
        }
	MIN_ABORT ( "control should never reach here" );
    }

    template < class vecptr >
    inline min::gen current
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp )
    {
    	return lp.current;
    }

    template < class vecptr >
    inline min::gen update_refresh
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp )
    {
	if ( lp.hash_offset == 0 ) return lp.current;
	    // Unstarted list pointer needs nothing
	    // done.

	if ( lp.current_index != 0 )
	    return lp.current =
	    		lp.base[lp.current_index];
#       if MIN_USE_OBJ_AUX_STUBS
	    else if ( lp.current_stub != NULL )
		return lp.current =
			   unprotected::gen_of
			          ( lp.current_stub );
#	endif
	else if ( lp.current == LIST_END() )
	    return lp.current;

	MIN_ABORT ( "inconsistent list pointer" );
    }

    template < class vecptr >
    inline min::gen insert_refresh
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp )
    {
	if ( lp.hash_offset == 0 ) return lp.current;
	    // Unstarted list pointer needs nothing
	    // done.

	unsptr new_hash_offset =
	    unprotected::hash_offset_of ( lp.vecp );
	unsptr new_aux_offset =
	    unprotected::aux_offset_of ( lp.vecp );
	unsptr new_total_size =
	    total_size_of ( lp.vecp );

	unsptr adjust =
	    new_hash_offset - lp.hash_offset;
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
	lp.aux_offset = new_aux_offset;
	lp.total_size = new_total_size;

	if ( lp.current_index != 0 )
	    return lp.current =
	    		lp.base[lp.current_index];
#       if MIN_USE_OBJ_AUX_STUBS
	    else if ( lp.current_stub != NULL )
		return lp.current =
			   unprotected::gen_of
			          ( lp.current_stub );
#	endif
	else if ( lp.current == LIST_END() )
	    return lp.current;

	MIN_ABORT ( "inconsistent list pointer" );
    }

    template <>
    inline min::gen insert_refresh
    	    ( min::list_insptr & lp )
    {
	if ( lp.hash_offset == 0 ) return lp.current;
	    // Unstarted list pointer needs nothing
	    // done.

	unsptr new_hash_offset =
	    unprotected::hash_offset_of ( lp.vecp );
	unsptr new_aux_offset =
	    unprotected::aux_offset_of ( lp.vecp );
	unsptr new_total_size =
	    total_size_of ( lp.vecp );

	unsptr adjust =
	    new_hash_offset - lp.hash_offset;
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
	lp.aux_offset = new_aux_offset;
	lp.total_size = new_total_size;

	if ( lp.current_index != 0 )
	    return lp.current =
	    		lp.base[lp.current_index];
#       if MIN_USE_OBJ_AUX_STUBS
	    else if ( lp.current_stub != NULL )
		return lp.current =
			   unprotected::gen_of
			          ( lp.current_stub );
#	endif
	else if ( lp.current == LIST_END() )
	    return lp.current;

	MIN_ABORT ( "inconsistent list pointer" );
    }

    template < class vecptr >
    inline min::gen start_sublist
    	    ( min::unprotected
	         ::list_ptr_type<vecptr> & lp )
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
        MIN_ASSERT ( value != LIST_END(),
	             "value is LIST_END" );
        MIN_ASSERT ( lp.current != LIST_END(),
	             "list is currently at its end" );
        MIN_ASSERT ( ! is_sublist ( lp.current ),
	             "current list element is a"
		     " sublist" );
        MIN_ASSERT ( ! is_sublist ( value ),
	             "value is a sublist aux" );
	unprotected::acc_write_update
	    ( unprotected::stub_of ( lp.vecp ), value );

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
		unprotected::set_gen_of
		    ( lp.current_stub, value );
		lp.current = value;
	    }
	    else
#	endif
	if ( lp.current_index != 0 )
	    lp.current =
	        lp.base[lp.current_index] = value;
	else
	    MIN_ABORT ( "inconsistent list pointer" );
    }
    template <>
    inline void update
    	    ( min::list_insptr & lp,
	      min::gen value )
    {
        MIN_ASSERT ( value != LIST_END(),
	             "value is LIST_END" );
        MIN_ASSERT ( lp.current != LIST_END(),
	             "list is currently at its end" );
        MIN_ASSERT ( value == EMPTY_SUBLIST()
	             ||
		     ! is_sublist ( value ),
		     "value is a non-empty sublist" );
	internal::remove_sublist
	       ( lp.base, lp.total_size, lp.current );
	unprotected::acc_write_update
	    ( unprotected::stub_of ( lp.vecp ), value );

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
		unprotected::set_gen_of
		    ( lp.current_stub, value );
		lp.current = value;
	    }
	    else
#	endif
	if ( lp.current_index != 0 )
	    lp.current =
	        lp.base[lp.current_index] = value;
	else
	    MIN_ABORT ( "inconsistent list pointer" );
    }

    inline bool insert_reserve
    	    ( min::list_insptr & lp,
	      min::unsptr insertions,
	      min::unsptr elements,
	      bool use_obj_aux_stubs )
    {
        // Warning: it is OK if lp is not yet started.

        if ( elements == 0 ) elements = insertions;
	MIN_ASSERT ( insertions <= elements,
	             "more insertions reserved than"
		     " elements" );

	unsptr unused_size =
	      unprotected::aux_offset_of ( lp.vecp )
	    - unprotected::unused_offset_of ( lp.vecp );

	if (      unused_size
	        < 2 * insertions + elements
#	    if MIN_USE_OBJ_AUX_STUBS
	     && (    ! use_obj_aux_stubs
	          ||   internal::number_of_free_stubs
		     < insertions + elements )
#	    endif
	   )
	    return internal::insert_reserve
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

    min::unsptr list_element_count
	    ( min::obj_vec_ptr & vp );
}


// Object Attribute Level
// ------ --------- -----


namespace min { namespace unprotected {

    // This is the generic attribute pointer type from
    // which specific attribute pointer types are made.

    template < class vecptr >
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

    template < class vecptr >
    vecptr & obj_vec_ptr_of
    	    ( min::unprotected::attr_ptr_type
	          < vecptr > & ap );

    template < class vecptr >
    void locate
	    ( unprotected::attr_ptr_type
	          < vecptr > & ap,
	      min::gen name );
    template < class vecptr >
    void locatei
	    ( unprotected::attr_ptr_type
	          < vecptr > & ap,
	      int name );
    template < class vecptr >
    void locatei
	    ( unprotected::attr_ptr_type
	          < vecptr > & ap,
	      min::unsptr name );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	template < class vecptr >
	void locate
		( unprotected::attr_ptr_type
		      < vecptr > & ap,
		  min::unsptr & length, min::gen name );
#   endif
    template < class vecptr >
    void locate_reverse
	    ( unprotected::attr_ptr_type
	          < vecptr > & ap,
	      min::gen reverse_name );
    template < class vecptr >
    void relocate
	    ( unprotected::attr_ptr_type
	          < vecptr > & ap );
    template < class vecptr >
    min::unsptr get
	    ( min::gen * out, min::unsptr n,
	      unprotected::attr_ptr_type
	          < vecptr > & ap );
    template < class vecptr >
    min::gen get
	    ( unprotected::attr_ptr_type
	          < vecptr > & ap );
    template < class vecptr >
    unsigned get_flags
	    ( min::gen * out, unsigned n,
	      unprotected::attr_ptr_type
	          < vecptr > & ap );
    template < class vecptr >
    bool test_flag
	    ( unprotected::attr_ptr_type
	          < vecptr > & ap,
	      unsigned n );

    struct attr_info
    {
        min::gen    name;
	min::gen    value;
	min::uns64  flags;
	min::unsptr value_count;
	min::unsptr flag_count;
	min::unsptr reverse_attr_count;
    };
    struct reverse_attr_info
    {
        min::gen    name;
	min::gen    value;
	min::unsptr value_count;
    };
    void sort_attr_info
	    ( min::attr_info * out,
	      min::unsptr n );
    void sort_reverse_attr_info
	    ( min::reverse_attr_info * out,
	      min::unsptr n );

    template < class vecptr >
    min::unsptr get_attrs
	    ( min::attr_info * out,
	      min::unsptr n,
	      unprotected::attr_ptr_type
	          < vecptr > & ap,
	      bool include_attr_vec = false );
    template < class vecptr >
    min::unsptr get_reverse_attrs
	    ( min::reverse_attr_info * out,
	      min::unsptr n,
	      unprotected::attr_ptr_type
	          < vecptr > & ap );
    template < class vecptr >
    min::gen update
	    ( unprotected::attr_ptr_type
	          < vecptr > & ap,
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
	    template < class vecptr >
	    void locate
		    ( unprotected::attr_ptr_type
			  < vecptr > & ap,
		      min::gen name,
		      bool allow_partial_label = false
		    );
#	else // ! MIN_ALLOW_PARTIAL_ATTR_LABELS
	    template < class vecptr >
	    void locate
		    ( unprotected::attr_ptr_type
			  < vecptr > & ap,
		      min::gen name );
#	endif
	template < class vecptr >
	void relocate
		( unprotected::attr_ptr_type
		      < vecptr > & ap );
	template < class vecptr >
	min::unsptr get
		( min::gen * out, min::unsptr n,
		  unprotected::attr_ptr_type
		      < vecptr > & ap );
	template < class vecptr >
	min::gen get
		( unprotected::attr_ptr_type
		      < vecptr > & ap );
	template < class vecptr >
	min::unsptr get_flags
		( min::gen * out, min::unsptr n,
		  unprotected::attr_ptr_type
		      < vecptr > & ap );
	template < class vecptr >
	bool test_flag
		( unprotected::attr_ptr_type
		      < vecptr > & ap,
		  unsigned n );
	template < class vecptr >
	min::gen update
		( unprotected::attr_ptr_type
		      < vecptr > & ap,
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
	// tor to v, which may be EMPTY_SUBLIST().
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
	// may be EMPTY_SUBLIST().  The state is set to
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


    template < class vecptr >
    class attr_ptr_type {

    public:

        attr_ptr_type ( vecptr & vecp )
	    : attr_name ( NONE() ),
	      reverse_attr_name ( NONE() ),
	      state ( INIT ),
	      dlp ( vecp ),
	      locate_dlp ( vecp ),
	      lp ( vecp ) {}

    private:

    // Private Data:

        min::gen attr_name;
	    // The attribute name given to the last
	    // locate function call.
        min::gen reverse_attr_name;
	    // The reverse attribute name given to the
	    // last reverse_locate function call;
	    // also reset to NONE() by a locate function
	    // call.  Can be NONE() or ANY().

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
		    // to NONE().

		LOCATE_ANY		= 3,
		    // Last call to locate succeeded,
		    // and the last subsequent call to
		    // reverse_locate set the reverse
		    // attribute to ANY().

		REVERSE_LOCATE_FAIL	= 4,
		    // Last call to locate succeeded,
		    // the last subsequent call to
		    // reverse_locate set the reverse
		    // attribute to a value other than
		    // NONE() or ANY(), and this call
		    // failed.

		REVERSE_LOCATE_SUCCEED	= 5
		    // Last call to locate succeeded,
		    // the last subsequent call to
		    // reverse_locate set the reverse
		    // attribute to a value other than
		    // NONE() or ANY(), and this call
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
	    // Attribute vector index passed to the
	    // start_vector function, or the hash value
	    // passed to the start_hash function, by the
	    // last call to locate.  See the IN_VECTOR
	    // flag above.  Set even if locate failed.

    	list_ptr_type<vecptr> dlp;
	    // Descriptor list pointer.  Points at the
	    // list element containing the attribute- or
	    // node- descriptor found, if the state
	    // is LOCATE_NONE or REVERSE_LOCATE_SUCCEED.
	    // Otherwise is free for use as a temporary
	    // working pointer.
	    //
	    // For a descriptor in a hash-list or child-
	    // sublist, this points at the list element
	    // that contains the descriptor.
	    //
	    // For a descriptor in an attribute vector
	    // element, this points at first element of
	    // the list headed by the vector element,
	    // which is both the vector element and the
	    // first element of the list.

    	list_ptr_type<vecptr> locate_dlp;
	    // This is the value of dlp after the last
	    // successful locate, if state >= LOCATE_
	    // NONE.
	    //
	    // If partial attributes are allowed and the
	    // state is LOCATE_FAIL, this points at the
	    // node-descriptor associated with the
	    // longest initial segment of attr_name
	    // which has an associated node-descriptor.
	    // This permits create_attr to be optimized.

    	list_ptr_type<vecptr> lp;
	    // A working pointer for temporary use.

    // Friends:

	friend vecptr & obj_vec_ptr_of<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
	friend void locate<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap,
		  min::gen name );
	friend void locatei<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap,
		  int name );
	friend void locatei<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap,
		  min::unsptr name );
#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    friend void locate<>
		    ( min::unprotected
			 ::attr_ptr_type<vecptr>
			     & ap,
		      min::unsptr & length,
		      min::gen name );
#   endif
	friend void min::locate_reverse<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap,
		  min::gen reverse_name );
	friend void min::relocate<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
	friend min::unsptr min::get<>
		( min::gen * out, min::unsptr n,
		  min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
	friend min::gen min::get<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
	friend unsigned min::get_flags<>
		( min::gen * out, unsigned n,
		  min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
	friend bool min::test_flag<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap,
		  unsigned n );
	friend min::unsptr min::get_attrs<>
	        ( min::attr_info * out,
		  min::unsptr n,
		  min::unprotected
		     ::attr_ptr_type<vecptr> & ap,
	          bool include_attr_vec );
	friend min::unsptr min::get_reverse_attrs<>
	        ( min::reverse_attr_info * out,
		  min::unsptr n,
		  min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
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
			 ::attr_ptr_type<vecptr>
			     & ap,
		      min::gen name,
		      bool allow_partial_label
		    );
#	else // ! MIN_ALLOW_PARTIAL_ATTR_LABELS
	    friend void min::internal::locate<>
		    ( min::unprotected
			 ::attr_ptr_type<vecptr>
			     & ap,
		      min::gen name );
#	endif
	friend void min::internal::relocate<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
	friend min::unsptr min::internal::get<>
		( min::gen * out, min::unsptr n,
		  min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
	friend min::gen min::internal::get<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
	friend min::unsptr min::internal::get_flags<>
		( min::gen * out, min::unsptr n,
		  min::unprotected
		     ::attr_ptr_type<vecptr> & ap );
	friend bool min::internal::test_flag<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap,
		  unsigned n );
	friend min::gen min::internal::update<>
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap,
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

    template < class vecptr >
    inline vecptr & obj_vec_ptr_of
	    ( min::unprotected
	         ::attr_ptr_type<vecptr> & ap )
    {
        return obj_vec_ptr_of ( ap.locate_dlp );
    }

    template < class vecptr >
    inline void locatei
	    ( min::unprotected
	         ::attr_ptr_type<vecptr> & ap,
	      int name )
    {
	typedef unprotected::attr_ptr_type<vecptr>
	    ap_type;

	ap.attr_name = new_num_gen ( name );

	if ( 0 <= name
	     &&
	     (unsigned) name < attr_size_of
	                ( obj_vec_ptr_of
			    ( ap.locate_dlp ) ) )
	{
	    ap.index = name;
	    ap.flags = ap_type::IN_VECTOR;
	    ap.reverse_attr_name = NONE();

	    start_vector ( ap.locate_dlp, name );
	    start_copy ( ap.dlp, ap.locate_dlp );
	    ap.state = ap_type::LOCATE_NONE;
#	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		ap.length = 1;
#	    endif

	    return;
	}
	
	internal::locate ( ap, ap.attr_name );
    }

    template < class vecptr >
    inline void locatei
	    ( min::unprotected
	         ::attr_ptr_type<vecptr> & ap,
	      min::unsptr name )
    {
	typedef unprotected::attr_ptr_type<vecptr>
	    ap_type;

	ap.attr_name = new_num_gen ( name );

	if ( name < attr_size_of
	                ( obj_vec_ptr_of
			    ( ap.locate_dlp ) ) )
	{
	    ap.index = name;
	    ap.flags = ap_type::IN_VECTOR;
	    ap.reverse_attr_name = NONE();

	    start_vector ( ap.locate_dlp, name );
	    start_copy ( ap.dlp, ap.locate_dlp );
	    ap.state = ap_type::LOCATE_NONE;
#	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		ap.length = 1;
#	    endif

	    return;
	}
	
	internal::locate ( ap, ap.attr_name );
    }

    template < class vecptr >
    inline void locate
	    ( min::unprotected
	         ::attr_ptr_type<vecptr> & ap,
	      min::gen name )
    {
	typedef unprotected::attr_ptr_type<vecptr>
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
		 (unsigned) i < attr_size_of
			( obj_vec_ptr_of
			    ( ap.locate_dlp ) ) )
	    {
		ap.attr_name = name;
		ap.index = i;
		ap.flags = ap_type::IN_VECTOR;
		ap.reverse_attr_name = NONE();

		start_vector ( ap.locate_dlp, i );
		start_copy ( ap.dlp, ap.locate_dlp );
		ap.state = ap_type::LOCATE_NONE;
#	        if MIN_ALLOW_PARTIAL_ATTR_LABELS
		    ap.length = 1;
#	        endif

		return;
	    }
	}

	internal::locate ( ap, name );
    }

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	template < class vecptr >
	inline void locate
		( min::unprotected
		     ::attr_ptr_type<vecptr> & ap,
		  min::unsptr & length,
		  min::gen name )
	{
	    typedef min::unprotected
		       ::attr_ptr_type<vecptr>
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
		    ap.reverse_attr_name = NONE();

		    start_vector ( ap.locate_dlp, i );
		    start_copy
			( ap.dlp, ap.locate_dlp );
		    ap.state = ap_type::LOCATE_NONE;
		    ap.length = 1;

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

    template < class vecptr >
    inline min::unsptr get
	    ( min::gen * out, min::unsptr n,
	      min::unprotected
	         ::attr_ptr_type<vecptr> & ap )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecptr> ap_type;

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

    template < class vecptr >
    inline min::gen get
	    ( min::unprotected
	         ::attr_ptr_type<vecptr> & ap )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecptr> ap_type;

	switch ( ap.state )
	{
	case ap_type::INIT:
	case ap_type::LOCATE_ANY:
	    return internal::get ( ap );

	case ap_type::LOCATE_FAIL:
	case ap_type::REVERSE_LOCATE_FAIL:
	    return NONE();
	}

	min::gen c = update_refresh ( ap.dlp );
	if ( ! is_sublist ( c ) )
	    return c;
	start_sublist ( ap.lp, ap.dlp );
	c = current ( ap.lp );
	if ( is_list_end ( c )
	     ||
	     is_sublist ( c )
	     ||
	     is_control_code ( c ) )
	    return NONE();
	min::gen d = next ( ap.lp );
	if ( is_list_end ( d )
	     ||
	     is_sublist ( d )
	     ||
	     is_control_code ( d ) )
	    return c;
	else
	    return MULTI_VALUED();
    }

    template < class vecptr >
    inline unsigned get_flags
	    ( min::gen * out, unsigned n,
	      min::unprotected
	         ::attr_ptr_type<vecptr> & ap )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecptr> ap_type;

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
	    new_control_code_gen ( 0 );
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

    template < class vecptr >
    inline bool test_flag
	    ( min::unprotected
	         ::attr_ptr_type<vecptr> & ap,
	      unsigned n )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecptr> ap_type;

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
		    return ( (    (unsgen) 1
		               << ( n - base ) )
		             &
			     control_code_of ( c ) )
			   != 0;
		base = next;
	    }
	}
	return false;
    }

    template < class vecptr >
    inline min::gen update
	    ( min::unprotected
	         ::attr_ptr_type<vecptr> & ap,
	      min::gen v )
    {
	typedef min::unprotected
	           ::attr_ptr_type<vecptr> ap_type;

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
	if ( v == NONE() ) return 0;

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
	    update ( ap.dlp, EMPTY_SUBLIST() );
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
	if ( v == NONE() ) return 0;

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
	    update ( ap.dlp, EMPTY_SUBLIST() );
	    return 1;
	}
	start_sublist ( ap.lp, ap.dlp );
	unsptr result = 0;
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
				      ( * in ),
				  "vector argument"
				  " element is not"
				  " control code" );
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
				  ( * in ),
			      "vector argument"
			      " element is not"
			      " control code" );
			unsgen uc =
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
				  ( * in ),
			      "vector argument"
			      " element is not"
			      " control code" );
			unsgen uc =
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
				  ( * in ),
			      "vector argument"
			      " element is not"
			      " control code" );
			unsgen uc =
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
		    unsgen mask =
		        (unsgen) 1 << ( n - base );
		    unsgen cc = control_code_of ( c );
		    bool result = ( mask & cc ) != 0;
		    c = new_control_code_gen
			    ( cc | mask );
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
		    unsgen mask =
		        (unsgen) 1 << ( n - base );
		    unsgen cc = control_code_of ( c );
		    bool result = ( mask & cc ) != 0;
		    c = new_control_code_gen
			    ( cc & ~ mask );
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
		    unsgen mask =
		        (unsgen) 1 << ( n - base );
		    unsgen cc = control_code_of ( c );
		    bool result = ( mask & cc ) != 0;
		    c = new_control_code_gen
			    ( cc ^ mask );
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

    const min::uns32 EXPAND_HT		  = ( 1 << 0 );
    const min::uns32 DISPLAY_EOL	  = ( 1 << 1 );
    const min::uns32 DISPLAY_PICTURE	  = ( 1 << 2 );
    const min::uns32 DISPLAY_NON_GRAPHIC  = ( 1 << 3 );
    const min::uns32 FLUSH_ON_EOL	  = ( 1 << 4 );
    const min::uns32 FLUSH_ID_MAP_ON_EOM  = ( 1 << 5 );
    const min::uns32 FORCE_SPACE	  = ( 1 << 6 );

    extern const min::uns32 standard_op_flags;

    struct display_control
    {
        min::uns32 display_char;
	min::uns32 display_suppress;
    };

    extern const min::display_control
        graphic_and_hspace_display_control;
    extern const min::display_control
        graphic_only_display_control;
    extern const min::display_control
        graphic_and_vhspace_display_control;
    extern const min::display_control
	display_all_display_control;

    struct break_control
    {
        min::uns32 break_after;
	min::uns32 break_before;
	min::uns32 conditional_break;
	min::uns32 conditional_columns;
    };

    extern const min::break_control
        no_auto_break_break_control;
    extern const min::break_control
        break_after_space_break_control;
    extern const min::break_control
        break_before_all_break_control;
    extern const min::break_control
        break_after_hyphens_break_control;

    struct line_break
    {
	uns32 offset;
	uns32 column;
	uns32 line_length;
	uns32 indent;
    };

    extern const line_break default_line_break;

    typedef min::packed_vec_insptr<min::line_break>
        line_break_stack;

    struct char_name_format
    {
	const min::ustring *	      char_name_prefix;
	const min::ustring *	      char_name_postfix;
    };

    extern const char_name_format *
	standard_char_name_format;

    struct gen_format;
    struct print_format
    {
	min::uns32		      op_flags;
	const min::uns32 *	      char_flags;

	min::support_control 	      support_control;
	min::display_control 	      display_control;
	min::break_control 	      break_control;

	const min::char_name_format * char_name_format;
	const min::gen_format *	      gen_format;
    };

    extern const print_format default_print_format;

    typedef min::packed_vec_insptr<min::print_format>
        print_format_stack;

    // Flags for printer->state.
    //
    enum {

        BREAK_AFTER		= ( 1 << 0 ),
	    // Set a break before the NEXT character
	    // (i.e., a break after the last character)
	    // UNLESS the next character is also a
	    // break-after character.

	PARAGRAPH_POSSIBLE	= ( 1 << 1 ),
	    // Set when printing the last element of
	    // an object with more than one element.
	    // Such an element may be a paragraph, so
	    // paragraphs are only recognized if this
	    // is on and the object format has a non-
	    // NONE paragraph_type.
	    //
	    // This flag is turned off by printer init,
	    // ::end_line, and internal::print_unicode.

	AFTER_LINE_SEPARATOR	= ( 1 << 2 ),
	    // A line separator (obj_line_sep) has
	    // just been printed.  This can be used
	    // to suppress the return to indent that
	    // normally preceeds printing a following
	    // object of line_type or line_sep_type.
	    // It also signals that a restore_indent
	    // and bol need to be executed if there
	    // is no object of following line_type or
	    // line_sep_type.
	    //
	    // This flag is turned off by printer init,
	    // ::end_line, internal::print_unicode, and
	    // min::print_quoted_unicode. When turned
	    // off by the non-init functions, restore_
	    // indent and bol are exeucted.

	// WARNING: AFTER_LEADING/TRAILING are the same
	// bits as IS_LEADING/TRAILING in character
	// flags.
	//
        AFTER_LEADING		= ( 1 << 8 ),
        AFTER_TRAILING		= ( 1 << 9 ),
	    // We are immediately after a min::leading/
	    // trailing or min::leading/trailing_always.
	    // If min::leading/trailing (not _always)
	    // then FORCE_SPACE_OK is also set.
        FORCE_SPACE_OK		= ( 1 << 10 ),
	AFTER_SAVE_INDENT	= ( 1 << 11 )
	    // A min::save_indent was in a sequence
	    // of min::leading/trailing{,_always}.

    };

    struct printer_struct
    {
        const uns32 control;

	const min::file file;
	uns32 column;

	min::line_break line_break;
	min::print_format print_format;

	const min::line_break_stack
	    line_break_stack;
	const min::print_format_stack
	    print_format_stack;

	const min::id_map id_map;

	// The following are not printer parameters but
	// are set during printer operation.

        min::uns32 state;
	    // See state flags above.

	min::uns32 last_str_class;
	    // Result of str_class applied to last
	    // string in line, or 0 if after whitespace
	    // item.
    };

    MIN_REF ( min::file, file, min::printer )
    MIN_REF ( min::line_break_stack,
              line_break_stack, min::printer )
    MIN_REF ( min::print_format_stack,
              print_format_stack, min::printer )
    MIN_REF ( min::id_map, id_map, min::printer )

    struct op
    {
        enum OPCODE
	{
            PGEN = 1,
            PGEN_FORMAT,
	    MAP_PGEN,
	    PUNICODE1,
	    PUNICODE2,
	    PINT32,
	    PINT64,
	    PUNS32,
	    PUNS64,
	    PFLOAT64,

	    SET_LINE_LENGTH,
	    SET_INDENT,
	    PLACE_INDENT,
	    ADJUST_INDENT,
	    LEFT,
	    RIGHT,
	    RESERVE,

	    SET_CONTEXT_GEN_FLAGS,
	    SET_GEN_FORMAT,

	    SET_PRINT_OP_FLAGS,
	    CLEAR_PRINT_OP_FLAGS,

	    SET_LINE_DISPLAY,

	    SAVE_LINE_BREAK,
	    RESTORE_LINE_BREAK,
	    SAVE_INDENT,
	    RESTORE_INDENT,
	    SAVE_PRINT_FORMAT,
	    RESTORE_PRINT_FORMAT,

	    EOL,
	    BOL,
	    FLUSH,
	    BOM,
	    EOM,
	    SET_BREAK,
	    INDENT,
	    EOL_IF_AFTER_INDENT,
	    SPACES_IF_BEFORE_INDENT,
	    SPACE_IF_AFTER_INDENT,
	    SPACE_IF_NONE,

	    SET_SUPPORT_CONTROL,
	    SET_DISPLAY_CONTROL,
	    SET_BREAK_CONTROL,

	    VERBATIM,

	    LEADING,
	    TRAILING,
	    LEADING_ALWAYS,
	    TRAILING_ALWAYS,
	    SPACE,

	    FLUSH_ONE_ID,
	    FLUSH_ID_MAP,

	    PRINT_ASSERT
	} opcode;

	union {
	    const void * p;
	    uns32 u32;
	    int32 i32;
	    uns64 u64;
	    int64 i64;
	    unsptr uptr;
	    float64 f64;
	    min::unsgen g;
	} v1, v2, v3, v4;

	op ( op::OPCODE opcode )
	    : opcode ( opcode ) {}
	op ( op::OPCODE opcode,
	     min::gen v )
	    : opcode ( opcode )
	{
	    v1.g = unprotected::value_of ( v );
	}
	op ( op::OPCODE opcode,
	     min::gen v,
	     const void * p )
	    : opcode ( opcode )
	{
	    v1.g = unprotected::value_of ( v );
	    v2.p = p;
	}
	op ( op::OPCODE opcode,
	     min::uns32 u )
	    : opcode ( opcode ) { v1.u32 = u; }
	op ( op::OPCODE opcode,
	     min::int32 i )
	    : opcode ( opcode ) { v1.i32 = i; }
	op ( op::OPCODE opcode,
	     min::unsptr length,
	     min::ptr<const Uchar> buffer )
	    : opcode ( opcode )
	{
	    v1.uptr = length;
	    v2.p = (void *) buffer.s;
	    v3.uptr = buffer.offset;
	}
	op ( op::OPCODE opcode,
	     min::unsptr length,
	     min::ptr<Uchar> buffer )
	    : opcode ( opcode )
	{
	    v1.uptr = length;
	    v2.p = (void *) buffer.s;
	    v3.uptr = buffer.offset;
	}
	op ( op::OPCODE opcode,
	     const min::ustring * str )
	    : opcode ( opcode )
	{
	    v1.p = (void *) str;
	}
	op ( op::OPCODE opcode,
	     min::int32 i,
	     const char * printf_format )
	    : opcode ( opcode )
	{
	    v1.i32 = i;
	    v2.p = (void *) printf_format;
	}
	op ( op::OPCODE opcode,
	     min::int64 i,
	     const char * printf_format )
	    : opcode ( opcode )
	{
	    v1.i64 = i;
	    v2.p = (void *) printf_format;
	}
	op ( op::OPCODE opcode,
	     min::uns32 u,
	     const char * printf_format )
	    : opcode ( opcode )
	{
	    v1.u32 = u;
	    v2.p = (void *) printf_format;
	}
	op ( op::OPCODE opcode,
	     min::uns64 u,
	     const char * printf_format )
	    : opcode ( opcode )
	{
	    v1.u64 = u;
	    v2.p = (void *) printf_format;
	}
	op ( op::OPCODE opcode,
	     min::float64 f,
	     const char * printf_format )
	    : opcode ( opcode )
	{
	    v1.f64 = f;
	    v2.p = (void *) printf_format;
	}
	op ( op::OPCODE opcode,
	     const void * p )
	    : opcode ( opcode )
	{
	    v1.p = p;
	}

    };

    min::printer init
	    ( min::ref<min::printer> printer,
              min::file file = min::NULL_STUB );

    min::printer init_ostream
	    ( min::ref<min::printer> printer,
	      std::ostream & ostream );

    inline op punicode ( min::Uchar c )
    {
        return op ( op::PUNICODE1, c );
    }

    inline op punicode
	    ( min::unsptr length,
	      min::ptr<const min::Uchar> str )
    {
        return op ( op::PUNICODE2, length, str );
    }

    inline op punicode
	    ( min::unsptr length,
	      min::ptr<min::Uchar> str )
    {
        return op ( op::PUNICODE2, length, str );
    }

    inline op punicode
	    ( min::unsptr length,
	      const min::Uchar * str )
    {
        return op ( op::PUNICODE2, length,
	            min::new_ptr ( str ) );
    }

    inline op pint
	    ( min::int32 i,
              const char * printf_format )
    {
        return op ( op::PINT32, i, printf_format );
    }

    inline op pint
	    ( min::int64 i,
              const char * printf_format )
    {
        return op ( op::PINT64, i, printf_format );
    }

    inline op puns
	    ( min::uns32 u,
              const char * printf_format )
    {
        return op ( op::PUNS32, u, printf_format );
    }

    inline op puns
	    ( min::uns64 u,
              const char * printf_format )
    {
        return op ( op::PUNS64, u, printf_format );
    }

    inline op pfloat
	    ( min::float64 f,
              const char * printf_format )
    {
        return op ( op::PFLOAT64, f, printf_format );
    }

    inline op set_line_length ( uns32 line_length )
    {
        return op ( op::SET_LINE_LENGTH, line_length );
    }

    inline op set_indent ( uns32 indent )
    {
        return op ( op::SET_INDENT, indent );
    }

    inline op place_indent ( int32 offset )
    {
        return op ( op::PLACE_INDENT, offset );
    }

    inline op adjust_indent ( int32 offset )
    {
        return op ( op::ADJUST_INDENT, offset );
    }

    inline op set_print_op_flags
	    ( uns32 print_op_flags )
    {
        return op ( op::SET_PRINT_OP_FLAGS,
	            print_op_flags );
    }

    inline op clear_print_op_flags
	    ( uns32 print_op_flags )
    {
        return op ( op::CLEAR_PRINT_OP_FLAGS,
	            print_op_flags );
    }

    inline op set_line_display
	    ( uns32 line_display )
    {
        return op ( op::SET_LINE_DISPLAY,
	            line_display );
    }

    inline op set_support_control
    	( const min::support_control & sc )
    {
        return op ( op::SET_SUPPORT_CONTROL, & sc );
    }

    inline op set_display_control
    	( const min::display_control & dc )
    {
        return op ( op::SET_DISPLAY_CONTROL, & dc );
    }

    inline op set_break_control
    	( const min::break_control & bc )
    {
        return op ( op::SET_BREAK_CONTROL, & bc );
    }

    inline op left ( uns32 width )
    {
        return op ( op::LEFT, width );
    }

    inline op right ( uns32 width )
    {
        return op ( op::RIGHT, width );
    }

    inline op reserve ( uns32 width )
    {
        return op ( op::RESERVE, width );
    }

    extern const op save_line_break;
    extern const op restore_line_break;
    extern const op save_indent;
    extern const op restore_indent;
    extern const op save_print_format;
    extern const op restore_print_format;

    extern const op eol;
    extern const op bol;
    extern const op flush;
    extern const op bom;
    extern const op eom;
    extern const op set_break;
    extern const op indent;
    extern const op eol_if_after_indent;
    extern const op spaces_if_before_indent;
    extern const op space_if_after_indent;
    extern const op space_if_none;

    extern const op flush_one_id;
    extern const op flush_id_map;

    extern const op expand_ht;
    extern const op noexpand_ht;
    extern const op display_eol;
    extern const op nodisplay_eol;
    extern const op display_picture;
    extern const op nodisplay_picture;
    extern const op display_non_graphic;
    extern const op nodisplay_non_graphic;

    extern const op flush_on_eol;
    extern const op noflush_on_eol;
    extern const op flush_id_map_on_eom;
    extern const op noflush_id_map_on_eom;

    extern const op force_space;
    extern const op noforce_space;

    extern const op verbatim;

    extern const op leading;
    extern const op trailing;
    extern const op leading_always;
    extern const op trailing_always;
    extern const op space;

    extern const op ascii;
    extern const op latin1;
    extern const op support_all;

    extern const op graphic_and_hspace;
    extern const op graphic_only;
    extern const op graphic_and_vhspace;
    extern const op display_all;

    extern const op no_auto_break;
    extern const op break_after_space;
    extern const op break_before_all;
    extern const op break_after_hyphens;

    extern const op print_assert; // For debugging only.

    struct str_format;

    namespace internal {

	void print_item_preface
	    ( min::printer printer,
	      min::uns32 str_class );

	min::printer print_unicode
		( min::printer printer,
		  min::unsptr & n,
		  min::ptr<const min::Uchar> & p,
		  min::uns32 str_class,
		  min::uns32 & width,
		  const min::display_control *
		      display_control = NULL,
		  const min::Uchar * substring = NULL,
		  min::unsptr substring_length = 0,
		  const min::ustring * replacement =
		      NULL );

	bool insert_line_break
	    ( min::printer printer );
    }

    inline min::printer print_item_preface
    	( min::printer printer,
	  min::uns32 str_class )
    {
        if ( printer->state
	     &
	     (   AFTER_LEADING
	       | AFTER_TRAILING
               | FORCE_SPACE_OK
	       | AFTER_SAVE_INDENT
	       | PARAGRAPH_POSSIBLE
	       | AFTER_LINE_SEPARATOR ) )
	    internal::print_item_preface
	        ( printer, str_class );
	printer->last_str_class = str_class;
	return printer;
    }

    inline min::printer print_chars
        ( min::printer printer,
	  const char * p,
	  min::unsptr n,
	  min::uns32 columns )
    {
	if ( printer->column + columns
	     >
	     printer->line_break.line_length )
	    internal::insert_line_break ( printer );
	min::push ( printer->file->buffer, n, p );
	printer->column += columns;
        return printer;
    }

    inline min::printer print_chars
        ( min::printer printer,
	  const char * p,
	  min::unsptr n,
	  min::uns32 columns,
	  min::uns32 str_class )
    {
	if ( printer->column + columns
	     >
	     printer->line_break.line_length )
	    internal::insert_line_break ( printer );
	min::push ( printer->file->buffer, n, p );
	printer->column += columns;
	printer->last_str_class = str_class;
        return printer;
    }

    inline min::printer print_item
        ( min::printer printer,
	  const char * p,
	  min::unsptr n,
	  min::uns32 columns,
	  min::uns32 str_class = min::IS_GRAPHIC )
    {
        if ( n == 0 ) return printer;
	min::print_item_preface ( printer, str_class );
	return print_chars ( printer, p, n, columns );
    }

    inline min::printer print_spaces
    	    ( min::printer printer, min::unsptr n = 1 )
    {
        if ( n == 0 ) return printer;
	min::print_item_preface ( printer, 0 );
	while ( n -- )
	{
	    min::push ( printer->file->buffer ) = ' ';
	    ++ printer->column;
	}
	if ( printer->print_format.break_control
	                          .break_after
	     &
	     printer->print_format.char_flags[' '] )
	    printer->state |= min::BREAK_AFTER;
	else
	    printer->state &= ~ min::BREAK_AFTER;

	return printer;
    }

    inline min::printer print_prefix_space
	    ( min::printer printer )
    {
        if ( printer->last_str_class != 0 )
	{
	    min::print_item_preface ( printer, 0 );
	    min::push ( printer->file->buffer ) = ' ';
	    ++ printer->column;
	}
	return printer;
    }
    inline min::printer print_postfix_space
	    ( min::printer printer )
    {
	min::push ( printer->file->buffer ) = ' ';
	++ printer->column;
	printer->last_str_class = 0;

	if ( printer->print_format.break_control
	                          .break_after
	     &
	     printer->print_format.char_flags[' '] )
	    printer->state |= min::BREAK_AFTER;
	else
	    printer->state &= ~ min::BREAK_AFTER;
	return printer;
    }
    inline min::printer print_space_if_none
	    ( min::printer printer )
    {
        if ( printer->last_str_class != 0 )
	{
	    min::print_item_preface ( printer, 0 );
	    min::push ( printer->file->buffer ) = ' ';
	    ++ printer->column;
	    if ( printer->print_format.break_control
				      .break_after
		 &
		 printer->print_format.char_flags[' '] )
		printer->state |= min::BREAK_AFTER;
	    else
		printer->state &= ~ min::BREAK_AFTER;
	}
	return printer;
    }
    inline min::printer print_leading
	    ( min::printer printer )
    {
        if ( printer->last_str_class == 0 )
	    return printer;
        else if ( ! (   printer->last_str_class
	              & min::IS_LEADING ) )
	    return min::print_spaces ( printer, 1 );
	printer->state |= min::AFTER_LEADING
	                + min::FORCE_SPACE_OK;
	return printer;
    }
    inline min::printer print_trailing
	    ( min::printer printer )
    {
        if ( printer->last_str_class == 0 )
	    return printer;
        else if ( ! (   printer->last_str_class
	              & min::IS_GRAPHIC ) )
	    return min::print_spaces ( printer, 1 );
	printer->state |= min::AFTER_TRAILING
	                + min::FORCE_SPACE_OK;
	return printer;
    }
    inline min::printer print_leading_always
	    ( min::printer printer )
    {
        if ( printer->last_str_class == 0 )
	    return printer;
        else if ( ! (   printer->last_str_class
	              & min::IS_LEADING ) )
	    return min::print_spaces ( printer, 1 );
	printer->state |= min::AFTER_LEADING;
	return printer;
    }
    inline min::printer print_trailing_always
	    ( min::printer printer )
    {
        if ( printer->last_str_class == 0 )
	    return printer;
        else if ( ! (   printer->last_str_class
	              & min::IS_GRAPHIC ) )
	    return min::print_spaces ( printer, 1 );
	printer->state |= min::AFTER_TRAILING;
	return printer;
    }

    min::printer print_cstring
    	    ( min::printer printer, const char * s,
	      const min::str_format * sf = NULL );

    // Rather than have a default NULL str_format,
    // we default to an inline purely to optimize.
    // The effect is the same.
    //
    min::printer print_unicode
	    ( min::printer printer,
	      min::unsptr n,
	      min::ptr<const min::Uchar> p,
	      const min::str_format * );
    inline min::printer print_unicode
	    ( min::printer printer,
	      min::unsptr n,
	      min::ptr<const min::Uchar> p )
    {
        if ( n == 0 ) return printer;

	min::uns32 width = 0xFFFFFFFF;
	min::uns32 str_class =
	    min::str_class
	        ( printer->print_format.char_flags,
		  printer->print_format.support_control,
		  n, p, min::null_str_classifier );

	return internal::print_unicode
		( printer, n, p, str_class, width );
    }

    inline min::printer print_Uchar
	    ( min::printer printer,
	      min::Uchar c )
    {
	min::ptr<const Uchar> p =
	    min::new_ptr<const Uchar> ( & c );
	return min::print_unicode
	    ( printer, 1, p );
    }

    inline min::printer print_ustring
    	    ( min::printer printer,
	      const min::ustring * s )
    {
        if ( s != NULL )
	    return print_item
	        ( printer,
		  ustring_chars( s ),
		  ustring_length ( s ),
		  ustring_columns ( s ) );
	else
	    return printer;
    }

    min::uns32 pwidth
	( min::uns32 & column,
	  const char * s, min::unsptr n,
	  const min::print_format & print_format );

    typedef min::printer (* pstring )
	    ( min::printer printer );

    extern min::pstring
    	space_if_none_pstring;
    extern min::pstring
    	leading_always_pstring;
    extern min::pstring
    	trailing_always_pstring;
    extern min::pstring
    	left_square_colon_space_pstring;
    extern min::pstring
    	space_colon_right_square_pstring;
    extern min::pstring
    	left_square_dollar_space_pstring;
    extern min::pstring
    	space_dollar_right_square_pstring;
    extern min::pstring
    	left_curly_right_curly_pstring;
    extern min::pstring
    	left_curly_leading_pstring;
    extern min::pstring
    	trailing_right_curly_pstring;
    extern min::pstring
    	trailing_vbar_pstring;
    extern min::pstring
    	vbar_leading_pstring;
    extern min::pstring
    	trailing_always_colon_space_pstring;
    extern min::pstring
    	trailing_always_comma_space_pstring;
    extern min::pstring
    	trailing_always_semicolon_space_pstring;
    extern min::pstring
    	trailing_always_colon_pstring;
    extern min::pstring
    	no_space_pstring;
    extern min::pstring
    	space_equal_space_pstring;
    extern min::pstring
    	left_curly_star_space_pstring;
    extern min::pstring
    	space_star_right_curly_pstring;
    extern min::pstring
    	space_less_than_equal_space_pstring;
    extern min::pstring
    	left_curly_vbar_vbar_right_curly_pstring;
}

min::printer operator <<
	( min::printer printer,
	  const min::op & op );

inline min::printer operator <<
	( min::printer printer,
	  min::pstring pstring )
{
    if ( pstring == NULL ) return printer;
    else return ( * pstring ) ( printer );
}

inline min::printer operator <<
	( min::printer printer,
	  const char * s )
{
    return min::print_cstring ( printer, s );
}

inline min::printer operator <<
	( min::printer printer,
	  min::ptr<const char> s )
{
    return min::print_cstring ( printer, ! s );
}

inline min::printer operator <<
	( min::printer printer,
	  min::ptr<char> s )
{
    return min::print_cstring
        ( printer, ! (min::ptr<const char>) s );
}

inline min::printer operator <<
	( min::printer printer,
	  char c )
{
    return min::print_Uchar
        ( printer, (min::uns8) c );
}

min::printer operator <<
	( min::printer printer,
	  min::int64 i );

inline min::printer operator <<
	( min::printer printer,
	  min::int32 i )
{
    return printer << (min::int64) i;
}

min::printer operator <<
	( min::printer printer,
	  min::uns64 i );

inline min::printer operator <<
	( min::printer printer,
	  min::uns32 i )
{
    return printer << (min::uns64) i;
}

min::printer operator <<
	( min::printer printer,
	  min::float64 i );

inline std::ostream & operator <<
        ( std::ostream & out, min::printer printer )
{
    return out << printer->file;
}
inline min::file operator <<
        ( min::file file, min::printer printer )
{
    return file << printer->file;
}
inline min::printer operator <<
        ( min::printer oprinter, min::printer iprinter )
{
    return oprinter << iprinter->file;
}



// Printing General Values
// -------- ------- ------

std::ostream & operator <<
	( std::ostream & out, min::gen g );

namespace min {

    struct num_format
    {
        const char * 		int_printf_format;
	min::float64		non_float_bound;
        const char * 		float_printf_format;
	const min::uns32 *	fraction_divisors;
	min::float64		fraction_accuracy;
    };

    extern const min::uns32 * standard_divisors;

    extern const min::num_format *
        short_num_format;
    extern const min::num_format *
        long_num_format;
    extern const min::num_format *
        fraction_num_format;

    min::printer print_num
    	    ( min::printer printer,
	      min::float64 value,
	      const min::num_format * num_format =
	          NULL );

    struct bracket_format
    {
	const min::ustring *	str_prefix;
	const min::ustring *	str_postfix;
	const min::ustring *	str_postfix_replacement;
	const min::ustring *	str_concatenator;
    };

    extern const min::bracket_format
        quote_bracket_format;

    struct str_format
    {

        const min::str_classifier   str_classifier;
	min::bracket_format 	    bracket_format;
	min::display_control	    display_control;
	min::uns32		    id_map_if_longer;
    };

    extern const min::str_format *
        quote_all_str_format;
    extern const min::str_format *
        standard_str_format;

    struct lab_format
    {
	min::pstring	lab_prefix;
	min::pstring	lab_separator;
	min::pstring	lab_postfix;
    };

    extern const min::lab_format *
        name_lab_format;
    extern const min::lab_format *
        bracket_lab_format;
    extern const min::lab_format *
        leading_always_lab_format;
    extern const min::lab_format *
        trailing_always_lab_format;


    struct special_format
    {
	min::pstring		    special_prefix;
	min::pstring		    special_postfix;
	packed_vec_ptr<const char *>
				    special_names;
    };

    extern const min::special_format *
        name_special_format;
    extern const min::special_format *
        bracket_special_format;

    struct gen_format;
    struct obj_format
    {
        min::uns32		    obj_op_flags;

	const min::gen_format * element_format;
	const min::gen_format * compact_element_format;
	const min::gen_format * top_element_format;
	const min::gen_format * quote_element_format;
        const min::gen_format *	label_format;
	const min::gen_format * value_format;

	const min::gen_format * initiator_format;
        const min::gen_format *	separator_format;
	const min::gen_format * terminator_format;

	const min::str_classifier marking_type;
	min::gen		quote_type;
	min::gen		line_type;
	min::gen		line_sep_type;
	min::gen		paragraph_type;

	min::pstring		obj_empty;

	min::pstring		obj_bra;
	min::pstring		obj_braend;
	min::pstring		obj_ketbegin;
	min::pstring		obj_ket;

	min::pstring		obj_sep;

	min::pstring		obj_attrbegin;
	min::pstring		obj_attrsep;

	min::pstring		obj_attreol;

	min::pstring		obj_attrneg;
	min::pstring		obj_attreq;

	min::pstring		obj_valbegin;
	min::pstring		obj_valsep;
	min::pstring		obj_valend;
	min::pstring		obj_valreq;

	min::pstring		obj_line_sep;
	min::pstring		obj_paragraph_begin;

	packed_vec_ptr<const char *>
	                   	    attr_flag_names;
    };
    const min::uns32 ENABLE_COMPACT	= ( 1 << 0 );
    const min::uns32 PRINT_ID		= ( 1 << 1 );
    const min::uns32 ISOLATED_LINE	= ( 1 << 2 );
    const min::uns32 EMBEDDED_LINE	= ( 1 << 3 );
    const min::uns32 NO_TRAILING_TYPE	= ( 1 << 4 );

    extern const min::obj_format *
        compact_obj_format;
    extern const min::obj_format *
        isolated_line_obj_format;
    extern const min::obj_format *
        embedded_line_obj_format;
    extern const min::obj_format *
        id_obj_format;
    extern const min::obj_format *
        paragraph_element_obj_format;
    extern const min::obj_format *
        line_element_obj_format;
    extern const min::obj_format *
        line_obj_format;

    struct gen_format
    {
	min::printer     ( * pgen )
	    ( min::printer printer,
	      min::gen v,
	      const min::gen_format * gen_format );

        const min::num_format *	    num_format;
        const min::str_format *	    str_format;
        const min::lab_format *	    lab_format;
        const min::special_format * special_format;
        const min::obj_format *	    obj_format;

        const min::gen_format *	    id_map_format;
    };

    extern const min::gen_format *
        top_gen_format;
    extern const min::gen_format *
        id_map_gen_format;
    extern const gen_format *
        name_gen_format;
    extern const gen_format *
        leading_always_gen_format;
    extern const gen_format *
        trailing_always_gen_format;
    extern const min::gen_format *
        value_gen_format;
    extern const min::gen_format *
        element_gen_format;
    extern const min::gen_format *
        id_gen_format;
    extern const gen_format *
        always_quote_gen_format;
    extern const gen_format *
        never_quote_gen_format;
    extern const gen_format *
        line_element_gen_format;
    extern const gen_format *
        paragraph_element_gen_format;
    extern const gen_format *
        line_gen_format;

    extern packed_vec_ptr<const char *>
           standard_special_names;

    extern packed_vec_ptr<const char *>
           standard_attr_flag_names;

    min::printer standard_pgen
	    ( min::printer printer,
	      min::gen v,
	      const min::gen_format * gen_format );

    inline min::printer print_str
    	    ( min::printer printer, min::gen str,
	      const min::str_format * sf = NULL )
    {
        min::str_ptr sp ( str );
	return min::print_cstring
	    ( printer, ! min::begin_ptr_of ( sp ), sf );
    }

    min::printer print_obj
	    ( min::printer printer, min::gen obj,
	      const min::obj_format * objf,
	      min::uns32 obj_op_flags,
	      min::unsptr max_attrs = 1000 );
	// Max_attrs is hidden and undocumented.

    inline min::printer print_obj
	    ( min::printer printer, min::gen obj,
	      const min::obj_format * objf )
    {
        return min::print_obj
	    ( printer, obj, objf, objf->obj_op_flags );
    }

    inline min::printer print_obj
	    ( min::printer printer, min::gen obj,
	      min::uns32 obj_op_flags )
    {
	const min::obj_format * objf =
	    printer->print_format.gen_format
	           ->obj_format;
        return min::print_obj
	    ( printer, obj, objf, obj_op_flags );
    }

    inline min::printer print_obj
	    ( min::printer printer, min::gen obj )
    {
	const min::obj_format * objf =
	    printer->print_format.gen_format
	           ->obj_format;
        return min::print_obj
	    ( printer, obj, objf, objf->obj_op_flags );
    }

    min::printer print_id
    	    ( min::printer printer, min::gen v );

    inline min::printer print_gen
            ( min::printer printer, min::gen v,
	      const min::gen_format * f )
    {
        MIN_ASSERT ( f != NULL,
	             "second argument is NULL" );
        return ( * f->pgen ) ( printer, v, f );
    }

    inline min::printer print_gen
            ( min::printer printer, min::gen v )
    {
        const min::gen_format * f =
	    printer->print_format.gen_format;
        return ( * f->pgen ) ( printer, v, f );
    }

    inline op pgen ( min::gen v )
    {
        return op ( op::PGEN, v );
    }

    inline op pgen
            ( min::gen v, const min::gen_format * f )
    {
        return op ( op::PGEN_FORMAT, v, f );
    }

    inline op pgen_name ( min::gen v )
    {
        return op ( op::PGEN_FORMAT, v,
	            min::name_gen_format );
    }

    inline op pgen_quote ( min::gen v )
    {
        return op ( op::PGEN_FORMAT, v,
	            min::always_quote_gen_format );
    }

    inline op pgen_never_quote ( min::gen v )
    {
        return op ( op::PGEN_FORMAT, v,
	            min::never_quote_gen_format );
    }

    inline op map_pgen ( min::gen v )
    {
        return op ( op::MAP_PGEN, v );
    }

    inline op set_gen_format
	    ( const min::gen_format * gen_format )
    {
        return op ( op::SET_GEN_FORMAT, gen_format );
    }

    extern min::locatable_gen TRUE;
    extern min::locatable_gen FALSE;
    extern min::locatable_gen empty_string;
    extern min::locatable_gen doublequote;
    extern min::locatable_gen line_feed;
    extern min::locatable_gen colon;
    extern min::locatable_gen semicolon;
    extern min::locatable_gen dot_initiator;
    extern min::locatable_gen dot_separator;
    extern min::locatable_gen dot_terminator;
    extern min::locatable_gen dot_type;
    extern min::locatable_gen dot_position;

    // Deprecated:
    //
    extern min::locatable_gen new_line;
    extern min::locatable_gen number_sign;
    extern min::locatable_gen dot_middle;
    extern min::locatable_gen dot_name;
    extern min::locatable_gen dot_arguments;
    extern min::locatable_gen dot_keys;
    extern min::locatable_gen dot_operator;

    min::id_map set_id_map
            ( min::printer printer,
	      min::id_map map = min::NULL_STUB );
}

inline min::printer operator <<
	( min::printer printer,
	  const min::str_ptr & s )
{
    return min::print_cstring
        ( printer, ! min::begin_ptr_of ( s ) );
}

inline min::printer operator <<
	( min::printer printer,
	  min::gen g )
{
    return printer << min::pgen ( g );
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
    switch ( unprotected::type_of ( s ) )
    {
    case LONG_STR:
	return   unprotected::length_of
		     ( unprotected::long_str_of ( s ) )
	       + 1
	       + sizeof ( long_str );
    case LABEL:
	return   internal::lab_header_of ( s )->length
	       * sizeof ( min::gen )
	       + sizeof ( internal::lab_header );
    case TINY_OBJ:
	return   ( ( ( (internal::tiny_obj *)
	               unprotected::ptr_of ( s ) )
		     -> total_size
		     >> internal::OBJ_FLAG_BITS )
		   + 1 )
	       * sizeof ( min::gen );
    case SHORT_OBJ:
	return   ( ( ( (internal::short_obj *)
	               unprotected::ptr_of ( s ) )
		     -> total_size
		     >> internal::OBJ_FLAG_BITS )
		   + 1 )
	       * sizeof ( min::gen );
    case LONG_OBJ:
	return   ( ( ( (internal::long_obj *)
	               unprotected::ptr_of ( s ) )
		     -> total_size
		     >> internal::OBJ_FLAG_BITS )
		   + 1 )
	       * sizeof ( min::gen );
    case HUGE_OBJ:
	return   ( ( ( (internal::huge_obj *)
	               unprotected::ptr_of ( s ) )
		     -> total_size
		     >> internal::OBJ_FLAG_BITS )
		   + 1 )
	       * sizeof ( min::gen );
    case PACKED_STRUCT:
        {
	    uns32 control =
	        * ( uns32 *) unprotected::ptr_of ( s );
	    uns32 subtype =
		  control
		& internal::PACKED_CONTROL_SUBTYPE_MASK;
	    internal::packed_struct_descriptor * d =
	        (internal::packed_struct_descriptor *)
	        (*internal::packed_subtypes)[subtype];
	    return d->size;
	}
    case PACKED_VEC:
        {
	    uns8 * p =
	        (uns8 *) unprotected::ptr_of ( s );
	    uns32 control = * ( uns32 *) p;
	    uns32 subtype =
		  control
		& internal::PACKED_CONTROL_SUBTYPE_MASK;
	    internal::packed_vec_descriptor * d =
	        (internal::packed_vec_descriptor *)
	        (*internal::packed_subtypes)[subtype];
	    uns32 max_length = * (uns32 *)
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
    	unprotected::deallocate_body
	    ( (min::stub *) s,
	      unprotected::body_size_of ( s ) );
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
        min::stub * aux_s = * head;
	min::stub * last_aux_s = NULL;
	uns64 last_c;
	while ( aux_s != null_stub )
	{
	    uns64 c = unprotected::control_of ( aux_s );
	    if ( unprotected::ptr_of ( aux_s ) == s )
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

    // Remove a stub s with control c from any aux hash
    // table it might be in.  Return true if the stub
    // was removed from a table, and false if the stub
    // type indicated is was not in any aux hash table.
    // MIN_ABORT is called if the stub type indicates
    // it is in a table but it is not.
    //
    inline bool remove_from_aux_hash_table
	    ( min::uns64 c, min::stub * s )
    {

	int t = unprotected::type_of_control ( c );
	min::stub ** aux_head = NULL;
	min::uns32 h;
	switch ( t )
	{
#       if MIN_IS_COMPACT
	    case min::NUMBER:
		h = min::floathash
			( unprotected::float_of ( s ) );
		h &= num_hash_mask;
		aux_head = num_aux_hash + h;
		break;
#       endif
	case min::SHORT_STR:
	    h = min::strnhash ( s->v.c8, 8 );
	    h &= str_hash_mask;
	    aux_head = str_aux_hash + h;
	    break;
	case min::LONG_STR:
	    h = unprotected::hash_of
	            ( unprotected::long_str_of ( s ) );
	    h &= str_hash_mask;
	    aux_head = str_aux_hash + h;
	    break;
	case min::LABEL:
	    h = min::labhash ( s );
	    h &= lab_hash_mask;
	    aux_head = lab_aux_hash + h;
	    break;
	default:
	    return false;
	}

	remove_aux_hash ( aux_head, s );

	return true;
    }

    // Move a stub s with control c from any aux hash
    // table it is in to the corresponding acc hash
    // table, and remove it from the acc list.  Last_s
    // is the stub previous to s on the acc stub list,
    // and last_c is the control of last_s.  True is
    // returned if the stub is moved and false is
    // returned if the stub type indicates it is not
    // in the hash tables.
    //
    inline bool move_to_acc_hash_table
	    ( min::uns64 c, min::stub * s,
	      min::uns64 last_c, min::stub * last_s )
    {

	int t = unprotected::type_of_control ( c );
	min::stub ** aux_head = NULL;
	min::stub ** acc_head = NULL;
	min::uns32 h;
	switch ( t )
	{
#       if MIN_IS_COMPACT
	    case min::NUMBER:
		h = min::floathash
			( unprotected::float_of ( s ) );
		h &= num_hash_mask;
		aux_head = num_aux_hash + h;
		acc_head = num_acc_hash + h;
		break;
#       endif
	case min::SHORT_STR:
	    h = min::strnhash ( s->v.c8, 8 );
	    h &= str_hash_mask;
	    aux_head = str_aux_hash + h;
	    acc_head = str_acc_hash + h;
	    break;
	case min::LONG_STR:
	    h = unprotected::hash_of
	            ( unprotected::long_str_of ( s ) );
	    h &= str_hash_mask;
	    aux_head = str_aux_hash + h;
	    acc_head = str_acc_hash + h;
	    break;
	case min::LABEL:
	    h = min::labhash ( s );
	    h &= lab_hash_mask;
	    aux_head = lab_aux_hash + h;
	    acc_head = lab_acc_hash + h;
	    break;
	default:
	    return false;
	}

	remove_aux_hash ( aux_head, s );

	last_c = unprotected::renew_acc_control_stub
	    ( last_c,
	      unprotected::stub_of_acc_control ( c ) );
	unprotected::set_control_of ( last_s, last_c );
	c = unprotected::renew_acc_control_stub
	        ( c, * acc_head );
	unprotected::set_control_of ( s, c );
	* acc_head = s;

	return true;
    }

} }

# endif // MIN_H
