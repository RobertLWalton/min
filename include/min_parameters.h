// MIN Language Implementation Parameters
//
// File:	min_parameters.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Nov  1 06:19:59 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/01 12:06:19 $
//   $RCSfile: min_parameters.h,v $
//   $Revision: 1.3 $

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

// 1 to use addresses and NOT VSNs in min::gen values
//
# define MIN_USES_ADDRESSES ( ! MIN_USES_VSNS )

// Machine Parameters
// ------- ----------

// Note: for GNU cpp, `cpp -dM </dev/null' lists all the
// predefined defined macros including those the define
// machine type.

//	MIN_INT32_TYPE
//	    Type of 32 bit signed integer.
//	MIN_INT64_TYPE
//	    Type of 64 bit signed integer.
//	MIN_BIG_ENDIAN
//	    1 if big endian; else 0.
//	MIN_POINTER_LENGTH
//	    Length in bits of xxx * pointer types.
//	MIN_FLOAT64_SIGNALLING_NAN
//	    High order 24 bits of the smallest (as an
//	    unsigned binary number) 64 bit floating
//	    point signalling NAN.
//
# ifdef __i386
#   define MIN_INT32_TYPE int
#   define MIN_INT64_TYPE long long
#   define MIN_BIG_ENDIAN 0
#   define MIN_POINTER_LENGTH 32
#   define MIN_FLOAT64_SIGNALLING_NAN 0x7FF800
# endif
