// MIN Language Allocator/Collector/Compactor Parameters
//
// File:	min_acc_parameters.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Nov 22 06:14:53 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

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
// the form "name1=value1 name2=value2 ...".  E.g.,
//
//   "MIN_CONFIG=ephemeral_levels=2 max_stubs=10000000"
//
// There are a few ACC parameters that must be compiled
// in and these are NOT in this file, but are instead
// in the min_parameters.h file.
//
// With the following exceptions all the parameters are
// integers as described below.
//
//   debug	A list of characters each of which turns
//		on a debugging print switch, as follows:
//
//	p   Trace parameter value changes.
//	m   Trace memory pool allocations.
//	M   Ditto with memory map changes.
//	D   Ditto with memory map dumps before
//	    operations.
//	f   Trace fixed size block free list
//	    augmentations.
//	v   Trace variable size block allocations.
//	c   Trace collector phase execution.
//
//              E.g., debug=pv

// Usage and Setup
// ----- --- -----

# ifndef MIN_ACC_PARAMETERS_H
# define MIN_ACC_PARAMETERS_H

# include <min.h>

// Allocator Parameters
// --------- ----------

// max_stubs
//
//   The maximum number of stubs that can be allocated
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
//   max_stubs must be less than MIN_MAX_NUMBER_OF_STUBS
//   defined in min_parameters.h which by default is
//   determined by the number of bits available for
//   packing stub addresses into stub control words.
//
# ifndef MIN_DEFAULT_MAX_STUBS
#    define MIN_DEFAULT_MAX_STUBS \
	( MIN_PTR_BITS <= 32 ? 1 << 25 : \
          MIN_MAX_NUMBER_OF_STUBS <= ( 1ull << 32 ) ? \
            MIN_MAX_NUMBER_OF_STUBS : ( 1ull << 32 ) )
# endif

// stub_increment
//
//   The number of stubs added at one time when stubs
//   run out.  The number of new pages allocated is
//   16 * stub_increment / pagesize.
//
# ifndef MIN_DEFAULT_STUB_INCREMENT
#    define MIN_DEFAULT_STUB_INCREMENT 1024
# endif

// space_factor
//
//   Certain memory space inefficiencies are
//   approximately
//
//	M + allocated-memory/F
//
//   where F = the memory space factor and M is the
//   MINIMUM amount of heap memory allocated to VIRTUAL
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
//   fixed block and variable block regions are F**2
//   pages long.
//
//   F must be a power of two >= 4 and <= 128.
//
# ifndef MIN_DEFAULT_SPACE_FACTOR
#    define MIN_DEFAULT_SPACE_FACTOR \
	( MIN_PTR_BITS <= 32 ? 16 : 64 )
# endif

// cache_line_size
//
//   This is a power of two that is the assumed size
//   of a cache line in bytes.  Object bodies of the
//   same or smaller size are aligned if reasonable so
//   they are inside a cache line.
//
# ifndef MIN_DEFAULT_CACHE_LINE_SIZE
#    define MIN_DEFAULT_CACHE_LINE_SIZE 256
# endif

// page_size
//
//   This is the page size used by the acc.  It must be
//   a multiple of the hardware page size returned by
//   MOS::pagesize(), and defaults to this hardware page
//   size.

// deallocated_body_size
//
//   The size of the inaccessible memory block to which
//   a min::DEALLOCATED stub is pointed.  Default:
//
//	MIN_DEALLOCATED_BODY_SIZE
//
//   if defined, or max_paged_body_size otherwise.
//   Must a multiple of the page size.  This size
//   includes both the body and the 8 byte control
//   word that is just before the body.
//
//   A block of inaccessible memory of this size is
//   allocated.  If an access is made to a deallocated
//   body of this or smaller size, a memory fault will
//   occur.  If an access is made to a larger dealloca-
//   ted body the result is undefined.

// Parameters not usually changed.  F = space_factor.
//
// subregion_size
//
//   Size of variable size block and fixed size block
//   regions.  Default: F**2 * page_size.
//
// superregion_size
//
//   Size of superregion; should be multiple of size of
//   subregion.  Default: 64 * subregion_size.
//
// max_paged_body_size
//
//   Maximum size of body allocated to a paged body
//   region.  Default: F**2 * page_size.
//
// paged_body_region_size
//
//   Normal size of a paged body region.  Default:
//   F * max_paged_body_size.
//
// stub_stack_segment_size
//
//   Size of stub stack segments.  A stub stack holds
//   stub pointers for the Collector and consists of
//   a list of stub stack segments each holding
//   about stub_stack_segment_size/8 pointers.
//   Default:: 4 * page_size.
//
// stub_stack_region_size
//
//   Size of stub stack region.  Default:
//   16 * stub_stack_segment_size.
//
// str_hash_size
//
//   Size of string hash table in entries, each entry
//   being one pointer.  Must be a power of 2.
//
# ifndef MIN_DEFAULT_STR_HASH_SIZE
#   define MIN_DEFAULT_STR_HASH_SIZE 4096
# endif
//
// lab_hash_size
//
//   Ditto for the label hash table.
//
# ifndef MIN_DEFAULT_LAB_HASH_SIZE
#   define MIN_DEFAULT_LAB_HASH_SIZE 4096
# endif
//
// num_hash_size
//
//   Ditto for the number hash table, which only exists
//   in compact implementations.
//
# ifndef MIN_DEFAULT_NUM_HASH_SIZE
#   define MIN_DEFAULT_NUM_HASH_SIZE 65536
# endif
//
// Note: The current implementation does NOT dynamically
// resize hash tables.



// Collector Parameters
// --------- ----------

// ephemeral_levels
//
//   The number of ephemeral garbage collector levels.
//   In addition there is one non-ephemeral level.
//   The value must be less than or equal to MIN_MAX_
//   EPHEMERAL_LEVELS defined in min_parameters.h (and
//   determined by the number of ACC flags in the stub
//   control word).
//
//   The number of ephemeral levels determines the
//   efficiency of the garbage collector in ways too
//   complex to model.
//
# ifndef MIN_DEFAULT_EPHEMERAL_LEVELS
#   define MIN_DEFAULT_EPHEMERAL_LEVELS \
	( MIN_MAX_EPHEMERAL_LEVELS >= 2 ? 2 : \
	  MIN_MAX_EPHEMERAL_LEVELS )
# endif

// ephemeral_sublevels[n]
//
//   The number of sublevels for ephemeral level n.
//   This is the number of garbage collections a datum
//   must survive in level n before it is promoted to
//   level n - 1.
//
//   Data is sorted into a list by age.  This list is
//   divided into sublevels which are consecutive
//   segments.  When a datum survives one garbage
//   collection, is it promoted to the next lower
//   (i.e., older) sublevel.  Sublevels are grouped
//   into levels.  Level 0, the non-ephemeral level,
//   has 1 sublevel, and data in that sublevel that
//   survives a garbage collection stays in that sub-
//   level.  Levels n > 0 are ephemeral and have the
//   number of sublevels given by this parameter.
//   When a datum in the lowest sublevel of level
//   n > 0 survives a garbage collection, it is promoted
//   to the highest sublevel of level n - 1, and this is
//   how data is promoted from an ephemeral level to the
//   next lower level.
//
//   Must be >= 1.  Ephemeral levels are numbered n =
//   1 .. ephemeral_levels where the highest ephemeral
//   level holds the most recently allocated objects.
//
//   The maximum value is compiled in and the actual
//   value for each ephemeral level is a settable
//   parameter subject to the compiled in maximum
//   and given the below default value.  The maximum
//   and default values are the same for each ephemeral
//   level, but the set values need not be.
//
# ifndef MIN_MAX_EPHEMERAL_SUBLEVELS
#   define MIN_MAX_EPHEMERAL_SUBLEVELS 10
# endif
# ifndef MIN_DEFAULT_EPHEMERAL_SUBLEVELS
#   define MIN_DEFAULT_EPHEMERAL_SUBLEVELS 5
# endif

// Values that control the timing of collections.
//
// Collection algorithm execution are performed in
// `collector increments', each of which does an amount
// of work bounded by the following parameters.  A
// complete understanding of these parameters requires
// an understanding of the collector algorithm: see
// min_acc.h.

// scan_limit
//     The maximum number of values that can be examined
//     in a collection increment.  Examination of a
//     value requires execution of a few tens of
//     instructions.  E.g., examination of a min::gen
//     value to see if it points at a stub, and if so
//     to see if the stub it points at needs to be
//     marked and put on a to-be-scavenged list.
//
# ifndef MIN_DEFAULT_ACC_SCAN_LIMIT
#   define MIN_DEFAULT_ACC_SCAN_LIMIT 1000
# endif

// scavenge_limit
//     The maximum number of stubs that can be scavenged
//     in a collection increment.  Must be > 1.
//
# ifndef MIN_DEFAULT_ACC_SCAVENGE_LIMIT
#   define MIN_DEFAULT_ACC_SCAVENGE_LIMIT 30
# endif

// collection_limit
//     The maximum number of stubs that can be collected
//     (i.e., deallocated) in a collection increment.
//     Must be >= 10.
//
# ifndef MIN_DEFAULT_ACC_COLLECTION_LIMIT
#   define MIN_DEFAULT_ACC_COLLECTION_LIMIT 10
# endif

// acc_stack_max_size
//     Max size of the stack into which pairs of
//     pointers to stubs are pushed by the mutator when
//     a pointer to an unmarked stub is stored in a
//     pointer to a scavenged stub or a pointer to a
//     level >= L stub is pushed into a level < L
//     non-L-root stub.  Must be a multiple of page
//     size.
//
# ifndef MIN_DEFAULT_ACC_STACK_MAX_SIZE
#   define MIN_DEFAULT_ACC_STACK_MAX_SIZE (1<<20)
# endif

// acc_stack_trigger
//     Number of pointer pairs normally allowed in the
//     acc stack before an interrupt is triggered that
//     will empty the acc stack.
//
# ifndef MIN_DEFAULT_ACC_STACK_TRIGGER
#   define MIN_DEFAULT_ACC_STACK_TRIGGER 100
# endif

// collector_period
//     Length in milliseconds of the collector time
//     period.  There is an interrupt at the end of each
//     such period.  0 if there is no collector time
//     period.
# ifndef MIN_DEFAULT_COLLECTOR_PERIOD
#   define MIN_DEFAULT_COLLECTOR_PERIOD 100
# endif

// collector_period_increments
//     The number of collector increments that are to be
//     executed each period.  If fewer have been execu-
//     ted by the end of the period, the remainder are
//     executed when the period ends.
# ifndef MIN_DEFAULT_COLLECTOR_PERIOD_INCREMENTS
#   define MIN_DEFAULT_COLLECTOR_PERIOD_INCREMENTS 1
# endif


// Compactor Parameters
// --------- ----------


# endif // MIN_ACC_PARAMETERS_H
