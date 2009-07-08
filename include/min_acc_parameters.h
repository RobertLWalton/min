// MIN Language Allocator/Collector/Compactor Parameters
//
// File:	min_acc_parameters.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Jul  8 05:02:19 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/07/08 17:36:53 $
//   $RCSfile: min_acc_parameters.h,v $
//   $Revision: 1.1 $

// Table of Contents
//
//	Usage and Setup
//	Allocator Parameters
//	Collector Parameters
//	Compactor Parameters

// This file defines the ACC (Allocator/Collector/
// Compactor) parameters and their DEFAULT values.
// Defaults can be overridden by the MIN_CONFIG
// environment variable at RUN time.  This variable has
// the form "name1=value1 name2=value2 ..." where all
// the values are numbers.  E.g.,
//
//   "MIN_CONFIG=ephemeral_levels=2 max_stubs=10000000"
//
// There are a few ACC parameters that must be compiled
// in and these are NOT in this file, but are instead
// in the min_parameters.h file.


// Usage and Setup
// ----- --- -----

# ifndef MIN_ACC_PARAMETERS_H
# define MIN_ACC_PARAMETERS_H

// Allocator Parameters
// --------- ----------

// max_stubs
//   The maximum number of stubs that can be allocate
//   (and not yet freed by the garbage collector) at
//   one time.  This is set when the program starts
//   and cannot be changed during execution.
//
//   16*max_stubs bytes of virtual memory are reserved
//   for the stub vector when the program starts, but
//   only pages at the beginning of the vector are
//   actually used (and therefore only these pages
//   exist).  max_stubs must be small enough so that
//   the allocation of the stub vector to virtual memory
//   succeeds; in particular, if virtual memory is only
//   32 bits, the stub vector may consume too much
//   virtual memory.  On a 64 bit address machine with
//   48 implemented virtual address bits, however, max_
//   stubs can be set very large.
//
//   max_stubs must be less than MIN_ABSOLUTE_MAX_
//   NUMBER_OF_STUBS defined in min_parameters.h which
//   is determined by the number of bits available for
//   packing stub addresses into stub control words.
//
# ifndef MIN_DEFAULT_MAX_STUBS
#    define MIN_DEFAULT_MAX_STUBS \
	( MIN_POINTER_BITS <= 32 ? 1 << 25 : \
          MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS \
	                 <= ( 1ull << 32 ) ? \
            MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS : \
	  ( 1ull << 32 ) )
# endif

// space_factor
//   Certain memory space inefficiencies are
//   approximately
//
//	M + allocated-memory/F
//
//   where F = the memory space factor and M is the
//   minimum amount of heap memory allocated to VIRTUAL
//   memory.
//
//	M = (F**2 * pagesize) * log2 ( F * pagesize ).
//
//   For a pagesize of 4096, if F = 16 then M = 16
//   Mbytes, if F = 32 then M = 68 Mbytes, and if
//   F = 64 then M = 288 MBytes.
//
//   More specifically, object bodies larger than F
//   pages are allocated as separate memory pools, and
//   regions are F**2 pages long.
//
//   F must be a power of two >= 4.
//
# ifndef MIN_DEFAULT_SPACE_FACTOR
#    define MIN_DEFAULT_SPACE_FACTOR \
	( MIN_POINTER_BITS <= 32 ? 16 : 64 )
# endif



// Collector Parameters
// --------- ----------

// ephemeral_levels
//   The number of ephemeral garbage collector levels.
//   In addition there is one non-ephemeral level.
//   The value must be less that MIN_MAX_EPHEMERAL_/
//   LEVELS defined in min_parameters.h (and determined
//   by the number of ACC flags in the stub control
//   word).
//
//   The number of ephemeral levels determines the
//   efficiency of the garbage collector in ways to
//   complex to model.
//
# ifndef MIN_DEFAULT_EPHEMERAL_LEVELS
#   define MIN_DEFAULT_EPHEMERAL_LEVELS \
	( MIN_MAX_EPHEMERAL_LEVELS >= 2 ? 2 : \
	  MIN_MAX_EPHEMERAL_LEVELS )
# endif



// Compactor Parameters
// --------- ----------


# endif // MIN_ACC_PARAMETERS_H
