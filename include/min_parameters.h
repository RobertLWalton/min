// MIN Language Implementation Parameters
//
// File:	min_parameters.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Nov  5 02:32:57 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/05 08:06:18 $
//   $RCSfile: min_parameters.h,v $
//   $Revision: 1.6 $

// Table of Contents
//
//	Software Parameters
//	Hardware Parameters

// Note: The following parameters may be overridden by
// giving -D options to the compiler:
//
//	MIN_IS_COMPACT
//	MIN_USES_VSNS
// 
// Other parameters are computed from these.

// Software Parameters.

// 1 if min::gen values are 32 bits; else 0
//
# ifndef MIN_IS_COMPACT
#   define MIN_IS_COMPACT 0
# endif

// 1 if min::gen values are 64 bits; else 0
//
# define MIN_IS_LOOSE ( ! MIN_IS_COMPACT )

// Size of min::gen values in bits; 32 or 64.
//
# define MIN_SIZEOF_GEN ( MIN_IS_COMPACT ? 32 : 64 )

// 1 to use VSNs in min::gen values to address stubs
//
# ifndef MIN_USES_VSNS
#   define MIN_USES_VSNS 0
# endif

// The conversion formula for VSNs is
//
//	address = VSN << MIN_VSN_SHIFT + MIN_VSN_BASE
//
# if MIN_USES_VSNS
#   ifndef MIN_VSN_BASE
#	define MIN_VSN_BASE 0
#   endif
#   ifndef MIN_VSN_SHIFT
#	define MIN_VSN_SHIFT 4
#   endif
# endif

// 1 to use addresses and NOT VSNs in min::gen values
//
# define MIN_USES_ADDRESSES ( ! MIN_USES_VSNS )

// Minimum size of unimplemented memory at which
// deallocated body pointers are pointed.
//
# ifndef MIN_DEALLOCATED_LIMIT
#   define MIN_DEALLOCATED_LIMIT (1 << 20)
# endif

// Machine Parameters
// ------- ----------

// Note: for GNU cpp, `cpp -dM </dev/null' lists all the
// predefined defined macros including those the define
// machine type.

//	MIN_INT32_TYPE
//	    Type of 32 bit signed integer.
//	MIN_INT64_TYPE
//	    Type of 64 bit signed integer.
//	MIN_INT_POINTER_TYPE
//	    Type of int (int32 or int64) that holds
//	    a pointer exactly.
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
# ifdef __i386
#   define MIN_INT32_TYPE int
#   define MIN_INT64_TYPE long long
#   define MIN_BIG_ENDIAN 0
#   define MIN_LITTLE_ENDIAN 1
#   define MIN_INT_POINTER_TYPE int
#   define MIN_FLOAT64_SIGNALLING_NAN 0x7FF800
# endif
