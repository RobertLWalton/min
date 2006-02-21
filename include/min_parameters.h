// MIN Language Implementation Parameters
//
// File:	min_parameters.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Feb 21 08:29:26 EST 2006
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2006/02/21 13:41:45 $
//   $RCSfile: min_parameters.h,v $
//   $Revision: 1.13 $

// Table of Contents
//
//	Software Parameters
//	Hardware Parameters

// Note: The following parameters may be overridden by
// giving -D XXX=1 options to the compiler:
//
//	MIN_DEBUG
//	MIN_IS_COMPACT
//	MIN_USES_OBJECT_AUX_STUBS
//
// Other parameters can be overridden to have value YYY
// by giving -D XXX=YYY options to the compiler.
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

// Stub addresses can be packed into either relative
// stub addresses or stub numbers.  The packing scheme
// for relative stub addresses is:
//
//     stub address = relative stub address
//		    + MIN_STUB_BASE
//
// The packing scheme for stub numbers is:
//
//     stub address = ( stub number << MIN_STUB_SHIFT )
//		    + MIN_STUB_BASE
//
// where 1 << MIN_STUB_SHIFT is the size of a stub, and
// therefore MIN_STUB_SHIFT is 4.
//
// The number of bits required for each scheme of
// storing a stub address is:
//
//	stub address		MIN_POINTER_BITS
//	stub relative address	MIN_STUB_RELATIVE_BITS
//	stub number		MIN_STUB_NUMBER_BITS
//
// where MIN_POINTER_BITS is the number of bits in a
// pointer, as determined by the hardware (see below),
// and MIN_STUB_NUMBER_BITS = MIN_STUB_RELATIVE_BITS -
// MIN_STUB_SHIFT.
//
// When stub addresses are stored in a certain number of
// bits as much packing is used as is required to fit
// the stub address into the available bits.  This is
// determined as follows:
//
//	stub address in:	bits:
//
//	compact general value	32
//
//	loose general value	44
//
//	garbage collectible
//	stub control value	56 - MIN_GC_FLAG_BITS
//
//	auxiliary stub
//	control value		48
//
//	block control value	48
//
// MIN_GC_FLAG_BITS controls how many ephemeral levels
// of garbage collector are implemented.
//
// Note that addresses of stub bodies and addresses
// pointing into stub bodies can always be stored in
// 64 bit locations if MIN_POINTER_BITS is as big as
// 64 bits.  Thus only stub addresses are constrained
// to be less than 64 bits.

// The values you need to set to control all this
// follow.  All must be constant integer expressions
// unless otherwise noted.

// Base for relative stub addresses.  Can be constant
// or global variable (if global variable, should be
// of unsigned type large enough to hold a pointer).
//
# ifndef MIN_STUB_BASE
#   define MIN_STUB_BASE 0
# endif

// Number of bits in a relative stub address.
//
# ifndef MIN_STUB_RELATIVE_BITS
#   define MIN_STUB_RELATIVE_BITS MIN_POINTER_BITS
# endif

// Number of GC flag bits.  The GC needs 4N+2 flag bits
// to implement N ephemeral levels and 1 non-ephemeral
// level of garbage collection.
//
# ifndef MIN_GC_FLAG_BITS
#   define MIN_GC_FLAG_BITS ( 56 - MIN_POINTER_BITS )
# endif

// Normally you will not define the following, and so
// will get the defaults.

// Number of low order zero bits in a stub address.
//
# ifndef MIN_STUB_SHIFT
#   define MIN_STUB_SHIFT 4
# endif

// Number of bits in a stub number.
//
# ifndef MIN_STUB_NUMBER_BITS
#   define MIN_STUB_NUMBER_BITS \
           ( MIN_STUB_RELATIVE_BITS - MIN_STUB_SHIFT )
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
//	MIN_BIG_ENDIAN
//	    1 if big endian; else 0.
//	MIN_LITTLE_ENDIAN
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
#   define MIN_BIG_ENDIAN 0
#   define MIN_LITTLE_ENDIAN 1
#   define MIN_POINTER_BITS 32
#   define MIN_FLOAT64_SIGNALLING_NAN 0x7FF800
#   define MIN_DEALLOCATED_LIMIT (1 << 20)
# endif

# endif // MIN_PARAMETERS_H
