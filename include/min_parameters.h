// MIN Language Implementation Parameters
//
// File:	min_parameters.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Feb 27 22:12:27 EST 2006
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2006/02/28 03:09:25 $
//   $RCSfile: min_parameters.h,v $
//   $Revision: 1.20 $

// Table of Contents
//
//	Software Parameters
//	Hardware Parameters

// Note: The following parameters may be overridden by
// giving -DXXX=1 options to the compiler:
//
//	MIN_DEBUG
//	MIN_IS_COMPACT
//	MIN_USES_OBJECT_AUX_STUBS
//
// Other parameters can be overridden to have value YYY
// by giving -DXXX=YYY options to the compiler.  Some
// examples are:
//
//    -DMIN_MAXIMUM_ABSOLUTE_STUB_ADDRESS=0xFFFFFFFF
//      Value is 2**32-1.  Allows high 2**29 bytes to be
// 	used for stubs on machines with 32 bit addres-
//	ses.
//
//    -DMIN_MAXIMUM_RELATIVE_STUB_ADDRESS=0xFFFFFFFFFFFF
//	Value is 2**48-1.  Allows up to 2**44 stubs to
//	exist on machines with 64 bit addresses.
// 
// See below for the definition of these paramters.
// Other parameters are computed from these.

# ifndef MIN_PARAMETERS_H
# define MIN_PARAMETERS_H

// Software Parameters.

// 1 to add extra debugging code; 0 not to.
//
# ifndef MIN_DEBUG
#   define MIN_DEBUG 0
# endif

// Define MIN_ASSERT so we can intercept assertions.
// Normally MIN_ASSERT == assert from <cassert>.
//
# ifndef MIN_ASSERT
#    define MIN_ASSERT( expression ) assert(expression)
# endif

// 1 if min::gen values are 32 bits; else 0.
//
# ifndef MIN_IS_COMPACT
#   define MIN_IS_COMPACT 0
# endif

// 1 if min::gen values are 64 bits; else 0.
//
# define MIN_IS_LOOSE ( ! MIN_IS_COMPACT )

// Size of min::gen values in bits; 32 or 64.
//
# define MIN_SIZEOF_GEN ( MIN_IS_COMPACT ? 32 : 64 )

// 1 to enable use of list auxiliary stubs; 0 not to.
//
# ifndef MIN_USES_OBJECT_AUX_STUBS
#   define MIN_USES_OBJECT_AUX_STUBS 0
# endif

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
//	0 .. MIN_MAXIMUM_ABSOLUTE_STUB_ADDRESS
//
// Relative stub addresses are defined by the formula:
//
//	stub address = (min::stub *)
//		       (   relative stub address
//		         + MIN_STUB_BASE )
//
// and are in the range:
//
//	0 .. MIN_MAXIMUM_RELATIVE_STUB_ADDRESS
//
// Stub indices are defined by the formula:
//
//	stub address = (min::stub *) MIN_STUB_BASE
//		     + stub index
//
// and are in the range:
//
//	0 .. MIN_MAXIMUM_RELATIVE_STUB_ADDRESS / 16
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
//    gc control value	      0 .. 2**G - 1 where
//			      G = 56 - MIN_GC_FLAG_BITS
//
//    (other) control value   0 .. 2**48-1
//
// MIN_GC_FLAG_BITS controls how many ephemeral levels
// of garbage collector are implemented (see below).
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

// Maximum absolute stub address.  Defaults to:
//
//	2**32 - 2**29 - 1    if MIN_POINTER_BITS == 32
//
//	2**44 - 1	     if MIN_POINTER_BITS != 32
//
# ifndef MIN_MAXIMUM_ABSOLUTE_STUB_ADDRESS
#   define MIN_MAXIMUM_ABSOLUTE_STUB_ADDRESS \
	   ( MIN_POINTER_BITS == 32 ? 0xDFFFFFFF : \
	     0xFFFFFFFFFFF )
# endif

// Maximum relative stub address.  Defaults to:
//
//	2**32 - 2**29 - 1    if MIN_POINTER_BITS == 32
//
//	2**48 - 1	     if MIN_POINTER_BITS != 32
//
# ifndef MIN_MAXIMUM_RELATIVE_STUB_ADDRESS
#   define MIN_MAXIMUM_RELATIVE_STUB_ADDRESS \
	   ( MIN_POINTER_BITS == 32 ? 0xDFFFFFFF : \
	     0xFFFFFFFFFFFF )
# endif

// Base for relative stub addresses.  Can be a non-
// negative constant or global variable.  If a global
// variable, should be of unsigned type large enough
// to hold a pointer.
//
# ifndef MIN_STUB_BASE
#   define MIN_STUB_BASE 0
# endif

// Number of GC flag bits.  The GC needs 4N+2 flag bits
// to implement N ephemeral levels and 1 non-ephemeral
// level of garbage collection.  Defaults to 12.
//
# ifndef MIN_GC_FLAG_BITS
#   define MIN_GC_FLAG_BITS 12
# endif

// Normally you will not define the following, and so
// will get the defaults.

// Number of low order zero bits in a stub relative
// address.  Sizeof stub = 1 << MIN_STUB_SHIFT.
//
# ifndef MIN_STUB_SHIFT
#   define MIN_STUB_SHIFT 4
# endif

// Maximum stub index.
//
# ifndef MIN_MAXIMUM_STUB_INDEX
#   define MIN_MAXIMUM_STUB_INDEX \
           ( MIN_MAXIMUM_RELATIVE_STUB_ADDRESS / 16 )
# endif


// Machine Parameters
// ------- ----------

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
//	    
//	MIN_FLOAT64_SIGNALLING_NAN
//	    High order 24 bits of the smallest (as an
//	    unsigned binary number) 64 bit floating
//	    point signalling NAN.
//
//	MIN_DEALLOCATED_LIMIT
//	    Minimum size of unimplemented memory at
//	    which deallocated body pointers are pointed.
//
# ifdef __i386
#   define MIN_INT32_TYPE int
#   define MIN_INT64_TYPE long long
#   define MIN_IS_BIG_ENDIAN 0
#   define MIN_IS_LITTLE_ENDIAN 1
#   define MIN_POINTER_BITS 32
#   define MIN_FLOAT64_SIGNALLING_NAN 0x7FF800
#   define MIN_DEALLOCATED_LIMIT (1 << 20)
# endif

# endif // MIN_PARAMETERS_H
