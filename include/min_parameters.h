// MIN Language Implementation Parameters
//
// File:	min_parameters.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Sep  6 05:24:34 EDT 2004
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2004/09/06 11:10:22 $
//   $RCSfile: min_parameters.h,v $
//   $Revision: 1.1 $

// 1 to compact object bodies by using 32 bit object
// numbers to reference stubs; 0 to use gen64 values
// instead to minimize creation of atoms.
//
# define MIN_COMPACT_DATA 1

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
