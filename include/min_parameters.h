// MIN Language Implementation Parameters
//
// File:	min_parameters.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Mar 16 16:11:46 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Settable Software Parameters
//	Rarely Set Software Parameters
//	Hardware Functions
//	Hardware Parameters

// Usage and Setup
// ----- --- -----

// Note: GNU C++ requires that unsigned long long con-
// stants have the `ull' postfix.  The standard rule
// that a constant type is the first type in which the
// value fits does NOT seem to be followed.

// The following compiler options illustrate typical use
// in overriding defaults:
//
// -DMIN_PROTECT=0
// -DMIN_IS_COMPACT=1
// -DMIN_USE_OBJ_AUX_STUBS=1
// -DMIN_MAX_EPHEMERAL_LEVELS=1
// 
// See below for the definition of these parameters.
// Other parameters are computed from these.

# ifndef MIN_PARAMETERS_H
# define MIN_PARAMETERS_H

// Settable Software Parameters
// -------- -------- ----------

// 1 to include checks in protected functions; 0 not to.
//
# ifndef MIN_PROTECT
#   define MIN_PROTECT 1
# endif

// Define MIN_ASSERT so we can intercept assertions.
// Normally MIN_ASSERT == assert from <cassert>.
//
# ifndef MIN_ASSERT
#   if MIN_PROTECT
#	define MIN_ASSERT( expression ) \
		assert(expression)
#   else
#	define MIN_ASSERT( expression )
#   endif
# endif

// 1 if min::gen values are 32 bits; else 0.
//
# ifndef MIN_IS_COMPACT
#   define MIN_IS_COMPACT 0
# endif

// 1 if min::gen values are 64 bits; else 0.
//
# define MIN_IS_LOOSE ( ! MIN_IS_COMPACT )

// 1 to enable use of list auxiliary stubs; 0 not to.
//
# ifndef MIN_USE_OBJ_AUX_STUBS
#   define MIN_USE_OBJ_AUX_STUBS 0
# endif

// ACC Parameters

// Maximum number of ephemeral levels possible with the
// compiled code.  See below if you want to set this
// explicitly (there is no point in reducing it from
// the default value given here).
//
# ifndef MIN_MAX_EPHEMERAL_LEVELS
#   define MIN_MAX_EPHEMERAL_LEVELS \
	( MIN_PTR_BITS <= 32 ? 4 : 2 )
# endif

// Maximum number of stubs possible with the compiled
// code.  The defaults specified here permit stub
// addresses to be stored in min::gen values and in
// acc and other control values.  You can multiply
// these defaults by as much as 16 if stub indices
// are to be stored instead of addresses.
//
// The actual maximum number of stubs allowed is
// determined when the program starts, and must be
// smaller than the absolute maximum here if memory
// for stubs is to be successfully allocated.  See
// min_acc_parameters.h.
//
// NOTE: the g++ pre-processor has an operator
// grouping bug and requires that x ? y : z ? w : v
// be expressed as x ? y : ( z ? w : v ).
//
# ifndef MIN_MAX_NUMBER_OF_STUBS
#   define MIN_MAX_NUMBER_OF_STUBS \
	( MIN_IS_COMPACT ? 0x0DFFFFFF : \
	  ( MIN_PTR_BITS <= 32 ? 1 << 28 : \
	  ( MIN_MAX_EPHEMERAL_LEVELS <= 2 ? \
	          ( 1ull << 40 ) : \
	  ( 1ull << \
	    ( 48 - 4 * MIN_MAX_EPHEMERAL_LEVELS ) ) \
	    ) ) )
#   ifndef MIN_STUB_BASE
#	define MIN_STUB_BASE 0
#   endif
# endif

// Optional hard-coded numeric address of the stub vec-
// tor.  Defaults to zero when MIN_MAX_NUMBER_OF_STUBS
// is allowed to default above.  Otherwise not set by
// default, and when not set the base of the stub vector
// is determined at run time.  Must be an unsigned
// integer constant if set.
//
// # define MIN_STUB_BASE xxx

// Rarely Set Software Parameters
// ------ --- -------- ----------

// Normally you will not define the following, and so
// will get the defaults.

// All addresses are 64 bits except stub addresses,
// which are packed.  All stub address packing schemes
// represent stub addresses as unsigned integer values
// that need less than 64 bits of storage.  Stub
// addresses can be packed as absolute stub addresses,
// relative stub addresses, or stub indices.
//
// Absolute stub addresses are defined by the formula:
//
//	stub address = absolute stub address
//
// and are in the range:
//
//	0 .. MIN_MAX_ABSOLUTE_STUB_ADDRESS
//
// Relative stub addresses are defined by the formula:
//
//	stub address = (min::stub *)
//		       (   relative stub address
//		         + min::internal::stub_base )
//
// and are in the range:
//
//	0 .. MIN_MAX_RELATIVE_STUB_ADDRESS
//
// Stub indices are defined by the formula:
//
//	stub address =
//		  (min::stub *) min::internal::stub_base
//		+ stub index
//
// and are in the range:
//
//	0 .. MIN_MAX_RELATIVE_STUB_ADDRESS / 16
//
// where 16 is the number of bytes in a stub.
//
// The range of integer values available to store packed
// stub addresses is determined as follows:
//
//    stub address in:        integer value range
//
//    compact general value   0 .. 2**32 - 2**29 - 1
//
//    loose general value     0 .. 2**44 - 1
//
//    acc control value	      0 .. 2**G - 1 where
//			      G = 56 - MIN_ACC_FLAG_BITS
//
//    (non-acc) control       0 .. 2**48-1
//              value
//
// MIN_ACC_FLAG_BITS (which defaults to 12) controls how
// many ephemeral levels of garbage collector are
// implemented (see below).
//
// Note that addresses of stub bodies and addresses
// pointing into stub bodies can always be stored in
// 64 bit locations if pointers are as big as 64 bits.
// Thus only stub addresses are constrained to be
// storable in less than 64 bits, and only stub
// addresses are packed.

// The values you need to set to control all this
// follow.  All must be constant integer expressions
// unless otherwise noted.  Note that MIN_PTR_BITS
// is the number of bits needed to store a pointer, as
// determined by hardware: See Hardware Parameters
// below.

// Maximum relative stub address.
//
# ifndef MIN_MAX_RELATIVE_STUB_ADDRESS
#   define MIN_MAX_RELATIVE_STUB_ADDRESS \
	( 16 * MIN_MAX_NUMBER_OF_STUBS - 1 )
# endif

// Maximum absolute stub address.
//
# ifndef MIN_MAX_ABSOLUTE_STUB_ADDRESS
#   ifdef MIN_STUB_BASE
#	define MIN_MAX_ABSOLUTE_STUB_ADDRESS \
	    (   MIN_STUB_BASE \
	      + MIN_MAX_RELATIVE_STUB_ADDRESS )
#   else
#	define MIN_MAX_ABSOLUTE_STUB_ADDRESS \
	   (    MIN_IS_COMPACT \
	     && MIN_PTR_BITS <= 32 ? \
	       0xDFFFFFFFull : \
	     MIN_PTR_BITS <= 32 ? 0xFFFFFFFFull : \
	     ( ( 1ull << 48 ) - 1 ) )
#   endif
# endif

// Number of ACC flag bits.  The ACC needs 4N+4 flag
// bits to implement N ephemeral levels and 1 non-
// ephemeral level of garbage collection.
//
// NOTE: if MIN_MAX_ABSOLUTE_STUB_ADDRESS >=
// ( 1 << ( 56 - MIN_ACC_FLAG_BITS ) ) then an acc
// control value cannot hold an absolute stub address.
// It may still be able to hold a relative stub address
// or a stub index.
//
# ifndef MIN_ACC_FLAG_BITS
#   define MIN_ACC_FLAG_BITS \
        ( 4 + 4 * MIN_MAX_EPHEMERAL_LEVELS )
# endif

// Number of low order zero bits in a stub relative
// address.  Sizeof stub = 1 << MIN_STUB_SHIFT.
//
# ifndef MIN_STUB_SHIFT
#   define MIN_STUB_SHIFT 4
# endif

// Maximum stub index.
//
# ifndef MIN_MAX_STUB_INDEX
#   define MIN_MAX_STUB_INDEX \
           MIN_MAX_NUMBER_OF_STUBS
# endif

// Maximum size of a fixed block for the ACC fixed block
// allocator, and the log of that size.  The size here
// is very much an overestimate of the actual maximum
// size.  The logrithm base 2 of the size is used as the
// size of a table.
//
// May NOT be larger than 1 << 33 so that log2floor may
// be used on MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE/8.
// May NOT be larger than 1 << 30 if pointers are only
// 32 bits long.
//
# ifndef MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG
#   define MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG 30
# endif
# define MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE \
     ( 1ull << MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG )

// Initial maximum number of packed structure/vector
// subtypes, and also the increment to this maximum
// number if it about to be exceeded.
//
# ifndef MIN_PACKED_SUBTYPE_COUNT
#   define MIN_PACKED_SUBTYPE_COUNT 128
# endif

// Number of bits of the min::uns32 control word in a
// packed structure or packed vector header that is
// reserved for the packed structure/vector subtype.
//
# ifndef MIN_PACKED_CONTROL_SUBTYPE_BITS
#   define MIN_PACKED_CONTROL_SUBTYPE_BITS 16
# endif

// Hardware Functions
// -------- ---------

#ifndef MIN_USE_GNUC_BUILTINS
#   ifdef __GNUC__
#	define MIN_USE_GNUC_BUILTINS 1
#   else
#	define MIN_USE_GNUC_BUILTINS 0
#   endif
#endif

namespace min { namespace internal {

    // Return j such that (1<<j) <= u < (1<<(j+1)),
    // assuming 0 < u <= (1<<31)-1.
    //
    inline unsigned log2floor ( unsigned u )
    {
#   if MIN_USE_GNUC_BUILTINS
	return   8 * sizeof ( unsigned ) - 1
	       - __builtin_clz ( u );
#   else
        unsigned c = 0;
	if ( u >= (1 << 16) ) c += 16, u >>= 16;
	if ( u >= (1 << 8) ) c += 8, u >>= 8;
	if ( u >= (1 << 4) ) c += 4, u >>= 4;
	if ( u >= (1 << 2) ) c += 2, u >>= 2;
	if ( u >= (1 << 1) ) c += 1;
	return c;
#   endif
    }
} }

// Hardware Parameters
// -------- ----------

// Note: for GNU cpp, `cpp -dM </dev/null' lists all the
// predefined defined macros including those that define
// machine type.

//	MIN_INT32_TYPE
//	    Type of 32 bit signed integer.
//	MIN_INT64_TYPE
//	    Type of 64 bit signed integer.
//	MIN_PTR_BITS
//	    Number of bits needed to hold a pointer.
//	    Must not be greater than 64.
//	MIN_IS_BIG_ENDIAN
//	    1 if big endian; else 0.
//	MIN_IS_LITTLE_ENDIAN
//	    1 if little endian; else 0.
//	MIN_FLOAT64_SIGNALLING_NAN
//	    High order 24 bits of the smallest (as an
//	    unsigned binary integer) 64 bit floating
//	    point signalling NAN.
//
# if __i386__
#   define MIN_INT32_TYPE int
#   define MIN_INT64_TYPE long long
#   define MIN_IS_BIG_ENDIAN 0
#   define MIN_IS_LITTLE_ENDIAN 1
#   define MIN_PTR_BITS 32
#   define MIN_FLOAT64_SIGNALLING_NAN 0x7FF400
# endif
# if __x86_64__
#   define MIN_INT32_TYPE int
#   define MIN_INT64_TYPE long long
#   define MIN_IS_BIG_ENDIAN 0
#   define MIN_IS_LITTLE_ENDIAN 1
#   define MIN_PTR_BITS 64
#   define MIN_FLOAT64_SIGNALLING_NAN 0x7FF400
# endif

# endif // MIN_PARAMETERS_H
