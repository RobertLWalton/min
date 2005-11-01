// MIN Language Implementation Parameters
//
// File:	min_parameters.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Nov  1 05:13:16 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/01 10:15:20 $
//   $RCSfile: min_parameters.h,v $
//   $Revision: 1.2 $

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

// Type of 32 bit integer.
//
# define MIN_32_BIT_INT int

// Type of 64 bit integer.
//
# define MIN_64_BIT_INT long long

// 1 if big endian; 0 if little endian.
//
# define MIN_BIG_ENDIAN 0

// Length in bits of xxx * pointer types.
//
# define MIN_POINTER_LENGTH 32
