// MIN Language Implementation Parameters
//
// File:	min_parameters.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon May 25 02:56:52 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/05/25 08:59:25 $
//   $RCSfile: min_parameters.h,v $
//   $Revision: 1.36 $

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
// -DMIN_DEBUG=1
// -DMIN_PROTECT=0
// -DMIN_IS_COMPACT=1
// -DMIN_USES_OBJ_AUX_STUBS=1
// -DMIN_MAX_ABSOLUTE_STUB_ADDRESS=0xFFFFFFFFFFFFull
//	// 2**48 - 1; forces use of stub relative
//	// addresses or indices in 64 bit general
//	// values.
// -DMIN_MAX_RELATIVE_STUB_ADDRESS=0xFFFFFFFFFFFFull
//	// 2**48 - 1; forces use of stub indices in 64
//	// bit general values.
// -DMIN_ACC_FLAG_BITS=16
//	// Adds an ephemeral ACC level.
// 
// See below for the definition of these parameters.
// Other parameters are computed from these.

# ifndef MIN_PARAMETERS_H
# define MIN_PARAMETERS_H

// Settable Software Parameters
// -------- -------- ----------

// 1 to add extra debugging code; 0 not to.
//
# ifndef MIN_DEBUG
#   define MIN_DEBUG 0
# endif

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
# ifndef MIN_USES_OBJ_AUX_STUBS
#   define MIN_USES_OBJ_AUX_STUBS 0
# endif

// ACC Parameters

// Maximum number of ephemeral levels possible with the
// compiled code.  See below if you want to set this
// explicitly (there is no point in reducing it from
// the default value given here).
//
# ifndef MIN_MAX_EPHEMERAL_LEVELS
#   define MIN_MAX_EPHEMERAL_LEVELS \
	( MIN_POINTER_BITS <= 32 ? 4 : 2 )
# endif

// Maximum number of stubs possible with the compiled
// code.  The defaults specified here permit stub
// addresses to be stored in min::gen values and in
// stub and other control values.  You can multiply
// these defaults by as much as 16 if stub indices
// are to be stored instead of addresses.
//
// The actual maximum number of stubs allowed is
// determined when the program starts, and must be
// smaller than the absolute maximum here if memory
// for stubs is to be successfully allocated.  See
// MIN_DEFAULT_MAX_NUMBER_OF_STUBS below.
//
// NOTE: the g++ pre-processor has an operator
// grouping bug and requires that x ? y : z ? w : v
// be expressed as x ? y : ( z ? w : v ).
//
# ifndef MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS
#   define MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS \
	( MIN_IS_COMPACT ? 0x0DFFFFFF : \
	  ( MIN_POINTER_BITS <= 32 ? 1 << 28 : \
	  ( MIN_MAX_EPHEMERAL_LEVELS <= 2 ? \
	          0xFFFFFFFFFFull : \
	  ( ( 1ull << \
	      ( 52 - 4 * MIN_MAX_EPHEMERAL_LEVELS ) ) \
	    - 1 ) ) ) )
#   ifndef MIN_STUB_BASE
#	define MIN_STUB_BASE 0
#   endif
# endif

// The maximum number of stubs M is set when the program
// starts.  It must be less than or equal the absolute
// maximum above.  16*M bytes of virtual memory are
// reserved for the stub vector when the program starts,
// but only pages at the beginning of the vector are
// used for stubs.  M must be small enough so that 
// the allocation to virtual memory succeeds.  The
// following is the default for M:
//
// See NOTE above.
//
# ifndef MIN_DEFAULT_MAX_NUMBER_OF_STUBS
#    define MIN_DEFAULT_MAX_NUMBER_OF_STUBS \
        ( MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS \
	                 <= ( 1 << 27 ) ? \
              MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS : \
          ( MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS \
	                 <= ( 1 << 28 ) ? \
              1 << 27 : \
          ( MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS \
	                 <= ( 1ull << 36 ) ? \
              MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS / 2 : \
	  ( 1ull << 35 ) ) ) )
# endif

// Optional hard-coded address of the stub vector.  De-
// faults to zero when MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS
// is allowed to default above.  Otherwise not set by
// default, and when not set the base of the stub vector
// is determined at run time.
//
// # define MIN_STUB_BASE xxx

// Certain memory space inefficiencies are approximately
//
//	M + allocated-memory/F
//
// where F = the memory space factor and M is the
// minimum amount of MIN heap memory allocated
//
//	M = (F**2 * pagesize) * log2 ( F * pagesize ).
//
// For a pagesize of 4096 and F = 16, M = 16 Mbytes.
//
// More specifically, object bodies larger than F pages
// are allocated as separate memory pools, and regions
// are F**2 pages long.
//
# define MIN_DEFAULT_MEMORY_SPACE_FACTOR 16

// Rarely Set Software Parameters
// ------ --- -------- ----------

// Normally you will not define the following, and so
// will get the defaults.

// All addresses are 64 bits except stub addresses,
// which are packed.  All stub address packing schemes
// represent stub addresses as unsigned integers that
// need less than 64 bits of storage.  Stub addresses
// can be packed as absolute stub addresses, relative
// stub addresses, or stub indices.
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
// When stub addresses are stored in a certain number of
// bits as much packing is used as is required to fit
// the stub address into the available bits.  This is
// determined as follows:
//
//    stub address in:        range:
//
//    compact general value   0 .. 2**32 - 2**29 - 1
//
//    loose general value     0 .. 2**44 - 1
//
//    acc control value	      0 .. 2**G - 1 where
//			      G = 56 - MIN_ACC_FLAG_BITS
//
//    (other) control value   0 .. 2**48-1
//
// MIN_ACC_FLAG_BITS (which defaults to 12) controls how
// many ephemeral levels of garbage collector are
// implemented (see below).
//
// Note that addresses of stub bodies and addresses
// pointing into stub bodies can always be stored in
// 64 bit locations if pointers are as big as 64 bits.
// Thus only stub addresses are constrained to be
// storable in less than 64 bits (they must in fact be
// storable in at most 44 bits).

// The values you need to set to control all this
// follow.  All must be constant integer expressions
// unless otherwise noted.  Note that MIN_POINTER_BITS
// is the number of bits needed to store a pointer, as
// determined by hardware: See Hardware Parameters
// below.

// Maximum relative stub address.
//
# ifndef MIN_MAX_RELATIVE_STUB_ADDRESS
#   define MIN_MAX_RELATIVE_STUB_ADDRESS \
	( 16 * MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS )
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
	   ( MIN_POINTER_BITS <= 32 ? 0xDFFFFFFFull : \
	     ( ( 1ull << 44 ) - 1 ) )
#   endif
# endif

// Number of ACC flag bits.  The ACC needs 4N+4 flag
// bits to implement N ephemeral levels and 1 non-
// ephemeral level of garbage collection.
//
// NOTE: if MIN_MAX_ABSOLUTE_STUB_ADDRESS >=
// ( 1 << ( 56 - MIN_ACC_FLAG_BITS ) ) then the
// control value of a stub cannot hold an absolute
// stub address.  It may still be able to hold a
// relative stub address or a stub index.
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
           MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS
# endif

// Maximum size of a fixed block for the ACC fixed block
// allocator, and the log of that size.
//
# define MIN_MAX_FIXED_BODY_SIZE_LOG 17
# define MIN_MAX_FIXED_BODY_SIZE \
         ( 1 << MIN_MAX_FIXED_BODY_SIZE_LOG )

// Hardware Functions
// -------- ---------

namespace min { namespace internal {

    // Return j such that (1<<j) <= u <= (1<<(j+1)),
    // assuming 0 < u <= MIN_MAX_FIXED_BODY_SIZE/8.
    //
    inline unsigned fixed_bodies_log ( unsigned u )
    {
#   ifdef __GNUC__
	return   8 * sizeof ( unsigned ) - 1
	       - __builtin_clz ( u );
#   else
	return
	  ( u < (1<<8) ?
	      ( u < (1<<4) ?
	          ( u < (1<<2) ?
		      ( u < (1<<1) ? 0 : 1 ) :
		      ( u < (1<<3) ? 2 : 3 ) ) :
	          ( u < (1<<6) ?
		      ( u < (1<<5) ? 4 : 5 ) :
		      ( u < (1<<7) ? 6 : 7 ) ) ) :
	      ( u < (1<<12) ?
	          ( u < (1<<10) ?
		      ( u < (1<<9) ? 8 : 9 ) :
		      ( u < (1<<11) ? 10 : 11 ) ) :
	          ( u < (1<<14) ?
		      ( u < (1<<13) ? 12 : 13 ) :
		      ( u < (1<<15) ? 14 : 15 ) ) ) );
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
//	MIN_POINTER_BITS
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
//	MIN_DEALLOCATED_LIMIT
//	    Minimum size of unimplemented memory at
//	    which deallocated body pointers are pointed.
//
# if __i386__
#   define MIN_INT32_TYPE int
#   define MIN_INT64_TYPE long long
#   define MIN_IS_BIG_ENDIAN 0
#   define MIN_IS_LITTLE_ENDIAN 1
#   define MIN_POINTER_BITS 32
#   define MIN_FLOAT64_SIGNALLING_NAN 0x7FF400
#   define MIN_DEALLOCATED_LIMIT (1 << 20)
# endif
# if __x86_64__
#   define MIN_INT32_TYPE int
#   define MIN_INT64_TYPE long long
#   define MIN_IS_BIG_ENDIAN 0
#   define MIN_IS_LITTLE_ENDIAN 1
#   define MIN_POINTER_BITS 64
#   define MIN_FLOAT64_SIGNALLING_NAN 0x7FF400
#   define MIN_DEALLOCATED_LIMIT (1 << 20)
# endif

# endif // MIN_PARAMETERS_H
