// MIN Allocator/Collector/Compactor Interface
//
// File:	min_acc.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Jul 21 04:09:44 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/07/21 08:28:09 $
//   $RCSfile: min_acc.h,v $
//   $Revision: 1.11 $

// The ACC interfaces described here are interfaces
// for use within and between the Allocator, Collector,
// and Compactor.  There interfaces are subject to
// change without notice.

// Table of Contents
//
//	Usage and Setup
//	Parameters
//	Stub Allocator Interface
//	Block Allocator Interface
//	Collector Interface
//	Compactor Interface

// Usage and Setup
// ----- --- -----

# ifndef MIN_ACC_H
# define MIN_ACC_H

# include <min.h>
# define MACC min::acc
# define MINT min::internal

// Stub Allocator Interface
// ---- --------- ---------

namespace min { namespace acc {

    // The stub allocator begins by allocating the
    // stub region, a contiguous sequence of pages that
    // can hold MACC::max_stubs stubs.  It then allo-
    // cates new stubs as needed from the beginning of
    // this region working up.  Newly added stubs are
    // appended to the list of free stubs headed by the
    // control word of MINT::last_allocated_stub.
    //
    // The Collector, and NOT the Allocator, frees
    // stubs and adds the freed stubs to the list of
    // free stubs.  The Allocator is only called to
    // allocate stubs when the list of free stubs is
    // exhausted.  The following, defined in min.h,
    // interface to the stub allocator:
    //
    //  min::stub * MINT::null_stub
    //    // Address of the first stub (stub with
    //	  // index 0).  This address is used instead
    //    // of the value NULL to end lists when a
    //    // real stub address is required.  The
    //	  // first stub should have DEALLOCATED type
    //    // and is always `allocated' and never freed.
    //  MINT::pointer_uns MINT::stub_base
    //    // Ditto but as an unsigned integer.
    //	void MINT::acc_expand_free_stub_list
    //		( unsigned n )
    //	  // Function to allocate n new stubs.
    //  unsigned MINT::number_of_free_stubs
    //    // Count of stubs on free stub list.
    //  min::stub * MINT::last_allocated_stub
    //    // The control word of this stub points at the
    //    // first free stub or is MINT::end_stub if
    //    // there are no free stubs.
    //  min::uns64 MINT::acc_new_stub_flags
    //    // Flags for newly allocated garbage
    //	  // collectable stubs.

    unsigned max_stubs = MIN_DEFAULT_MAX_STUBS;
        // The value of the max_stubs parameter.  The
	// number of stubs that can be allocated to the
	// stub region.  Value may be changed when the
	// program starts.

    unsigned stub_increment =
	    MIN_DEFAULT_STUB_INCREMENT;
        // The number of new stubs allocated by a call
        // to MINT::acc_expand_free_stub_list.

    min::stub * stub_begin;
    min::stub * stub_next;
    min::stub * stub_end;
        // Beginning and end of stub region, and next
	// location to be allocated in the region.
	// The end is the address just after the
	// region: stub_end = stub_begin + max_stubs.
} }


// Block Allocator Interface
// ----- --------- ---------

namespace min { namespace acc {

    // Bodies are stored in blocks.  Each block begins
    // with a block control word, and this is followed
    // by either an object body or, if the block does
    // not contain an object body, by a block subcontrol
    // word.
    //
    // The block control word is organized as follows:
    //
    //	Bits		Use
    //
    //	0 .. 47		Stub absolute or relative
    //			address or stub index.
    //
    //	48 .. 63	Region Locator.
    //
    // The region locator is a 16 bit signed integer L
    // that determines the address R of the MACC::region
    // control struct for the region that contains the
    // block.  L is interpreted as follows:
    //
    //	 if L < 0:	R = pageaddress + pagesize * L
    //
    //   if L >= 0:	R = & MACC::region_table[L]
    //
    // where `pageaddress' is the address of the page
    // containing the body control word containing L.
    //
    // If a block contains an object body, the stub
    // address points at the object's stub.  Otherwise
    // the stub address == MINT::end_stub and the block
    // control word is immediately followed in memory
    // by a block subcontrol word whose type field
    // gives the block_type and whose value field
    // specifies the size in bytes of the block.
    //
    enum block_type
        // Stored in block subcontrol word of blocks
	// that are NOT object bodies.
    {
        FREE			= 1,
	FIXED_SIZE_REGION	= 2,
	VARIABLE_SIZE_REGION	= 3,
	MULTI_BLOCK_REGION	= 4
    };

    // Bodies are organized into regions which are
    // contiguous sequences of pages.
    //
    // There are several kinds of regions:
    //
    //	 Fixed Size Block Regions
    //
    //	     The region consists of blocks all of the
    //	     same size B bytes, where B is a power of
    //	     two.  These blocks are known as the fixed
    //	     size blocks of the region.
    //
    //	     An object body of size S is allocated to
    //	     fixed size block of size B as follows:
    //
    //		8 bytes			control word
    //		S bytes			object body
    //		B - S - 8 bytes		unused padding
    //
    //		where B/2 < S + 8 <= B
    //
    //	     Note that unused padding can take up 50% of
    //	     a `full' fixed block region.  Fixed block
    //	     regions are intended for ephemeral object
    //	     bodies that will have short lifetimes.
    //
    //	     The MACC::region control struct for the
    //	     region is allocated at the very beginning
    //	     of the region, in the first fixed sized
    //	     block of the region.  The rest of this
    //	     block is called the `stunted block' of the
    //	     region and can be used to hold an object
    //	     body.  There is no limit to the number of
    //	     fixed block regions.
    //
    //	     Fixed size blocks are allocated from a free
    //	     list.  When an object body in a fixed size
    //	     block is freed, the block is put back on a
    //	     free list and becomes immediately available
    //	     for reuse.  See fixed_body_list_extension
    //	     struct below.
    //
    //	     The compactor may move all allocated blocks
    //	     in a fixed size block region to another
    //	     region (which may have fixed or variable
    //	     size blocks), thus permitting the emptied
    //	     region itself to be freed.
    //
    //	 Variable Size Block Regions
    //
    //	     The region consists of variable size blocks
    //	     each a multiple of 8 bytes in size.
    //
    //	     An object body of size S is allocated to
    //	     a variable size block of size B as follows:
    //
    //		8 bytes			control word
    //		S bytes			object body
    //		B - S - 8 bytes		unused padding
    //
    //	     		B - 8 < S + 8 <= B
    //
    //	     The MACC::region control struct for the
    //	     region is allocated at the beginning of the
    //	     region.  There is no limit to the number of
    //	     variable size block regions.
    //
    //	     A variable size block in a region is always
    //	     allocated after the last block previously
    //	     allocated to the region.  When an object
    //	     body in a variable size block is freed, the
    //	     block is marked as free, but cannot be re-
    //	     used until the variable size block region
    //	     is compacted.  Compaction moves all used
    //	     blocks in the region to the beginning of
    //	     the region, or to another region, thus
    //	     reclaiming the space occupied by variable
    //	     size free blocks.
    //
    //	     Variable size block regions are intended
    //	     for longer lived object bodies.
    //
    //	 Multi-Page Block Regions
    //
    //	     The region consists of variable size blocks
    //	     each of which is a sequence of pages.
    //  
    //	     If an object body of size S is allocated to
    //	     a variable size block of size B as follows:
    //
    //		8 bytes			control word
    //		S bytes			object body
    //		B - S - 8 bytes		unused padding
    //
    //	     		B - pagesize < S + 8 <= B
    //
    //	     As B is a multiple of the page size, there
    //	     may be up to just under one page of unused
    //	     bytes.  To avoid inefficiencies,
    //
    //			S >= F * page size
    //
    //	     is required, where F is the `memory space
    //	     factor' defined in min_parameters.h, so
    //	     that in a block
    //
    //	      unused-padding-size/block-size <= 1/(F+1)
    //
    //	     and the percentage of unused padding when a
    //	     region is `full' is limited to roughly 1/F.
    //
    //	     Multi-page block regions are intended for
    //	     larger object bodies.  When a multi-page
    //	     block is freed, the pages of a block are
    //	     typically freed, even though the surround-
    //	     ing pages may still be in use.  Also, the
    //	     pages of a multi-page block are not
    //	     generally allocated until they are first
    //	     used.
    //
    //	     Fixed and variable size block regions may
    //	     be allocated as multi-page block region
    //	     blocks.  The fixed and variable size
    //	     regions are `subregions' of the multi-page
    //	     block region.  These subregions begin with
    //	     a MACC::region control struct which in turn
    //	     begins with a body control word (see
    //	     below).
    //
    //	     A multi-page block region has a region
    //	     control block in the MACC::region_table
    //	     vector.  There is a limit of about 2**15
    //	     such regions.
    //
    //	     A block in a multi-page block region is
    //	     always allocated after the last block prev-
    //	     iously allocated to the region.  When a
    //	     multi-page block is is freed, the block is
    //	     marked as free, but cannot be reused until
    //	     the multi-page block region is compacted.
    //	     Compaction moves all used blocks in the
    //	     region to the beginning of the region, or
    //	     to another region, thus reclaiming the
    // 	     space occupied by multi-page free blocks.
    //	     Because all blocks are multi-page, moving
    //	     a block can be done by moving page table
    //	     entries, which is faster than moving bytes.

    struct region
    { 

        min::uns64 block_control;
	    // This is the block control word for the
	    // multi-page block containing the region,
	    // if the region is a fixed or variable
	    // block subregion of a multi-page block
	    // region.  In this case the locator field
	    // L of this word is such that
	    //
	    //	   MACC::region_table[L]
	    //
	    // is the  multi-page region containing
	    // this region, and the pointer field of
	    // the word equals MINT::end_stub.
	    //
	    // In other cases, where this region control
	    // block is in the MACC::region_table, this
	    // word is unused.

        min::uns64 block_subcontrol;
	    // The type code field of this word is the
	    // type of the region and the value field
	    // is the size of the region in bytes.

	min::uns64 offset;
	    // For variable size block and multi-page
	    // block regions, the offset of the first
	    // free byte in the region.  New blocks are
	    // always allocated at this offset.

	min::uns64 region_size;
	    // The size of the region in bytes.  Dupli-
	    // cates the value field of block_sub-
	    // control.

	min::uns32 block_size;
	    // For fixed size block regions, the size
	    // in bytes of the fixed size blocks.

	min::uns32 free_count;
	    // For fixed size block regions, the number
	    // of free blocks.

	MACC::region * subregion_previous,
		     * subregion_next;
	    // Previous and next region on a doubly
	    // linked list of subregions.  There is one
	    // such list for every multi-page region
	    // which includes that region and all of its
	    // subregions.

	MACC::region * free_previous,
		     * free_next;
	    // Previous and next region on a doubly
	    // linked list of fixed size block regions.
	    // There is one such list for every size
	    // of fixed block, so that the regions
	    // containing fixed blocks of a particular
	    // size can be located.  The order of this
	    // list also determines the order in which
	    // free blocks from regions will be used.

	void * free_first,
	     * free_last;
	    // The first and last free block for a fixed
	    // size block region.  Each free block is
	    // chained to the next by a `void *' pointer
	    // at the very beginning of the block (there
	    // is no block control word).  The list is
	    // NULL terminated and if the list is empty
	    // these members are NULL.

    };

    // Free List Management
    // ---- ---- ----------

    struct fixed_body_list_extension
        // Extension of fixed_body_list struct with data
	// specific to this allocator.
    {
        MINT::fixed_body_list * fbl;
	    // Address of associated fixed_body_list.
	    // fbl->size is size in bytes of fixed
	    // blocks controlled by this struct.

	MACC::region * first_region;
	MACC::region * current_region;
	    // First region from which free blocks are
	    // being allocated, and current region from
	    // which free blocks are being allocated,
	    // in the list of all fixed block regions
	    // for this size block.  The regions in the
	    // list are chained together with their
	    // free_previous/next fields.
	    //
	    // After a GC current_region is reset to
	    // first_region, then moved forward in
	    // list to first region with some free
	    // blocks.
	    //
	    // This strategy allows free blocks to
	    // accumulate in regions near end of list.
	    //
	    // fbl is set from info in current_region.
    };

} }


// Collector Interface
// --------- ---------

namespace min { namespace acc {

    // The `collector' segregates objects into N+1
    // levels, with level 0 being the `base level',
    // and levels 1, ..., N being `ephemeral levels'.
    // Objects in lower levels are older than objects
    // in higher levels.
    //
    // The collector can run one collector marking
    // algorithm execution for each level independently
    // and in parallel.  These are known as `markings'.
    // A level L marking marks all objects in levels
    // >= L that are pointed to by objects in the level
    // L root list or by executing threads, and recur-
    // sively marks any object pointed at by a marked
    // object.
    //
    // For each level L the collector maintains three
    // lists:
    //
    //	    Level List
    //		List of all objects in level L.
    //	
    //	    Root List
    //		List of all objects in levels < L
    //		that might point at objects in levels
    //		>= L.
    //
    //	    To Be Scavenged List
    //		List of all objects that remain to be
    //		scavenged during the currently executing
    //		level L marking.
    //
    // The `mutator' refers to all code outside the
    // collector algorithm execution.  The mutator may
    // write pointers to objects into other objects.
    //
    // The to-be-scavenged lists of all levels and the
    // root lists of all ephemeral levels are maintained
    // with the help of the following mutator action.
    //
    // Each stub has a set of ACC flags grouped into
    // pairs.  Each pair has a `scavenged flag' and an
    // `unmarked flag'.
    //
    // If a pointer to stub s2 is stored in a datum with
    // stub s1, then for each pair of ACC flags the sca-
    // venged flag of the pair of s1 and the unmarked
    // flag of the pair of s2 are logically ANDed, and
    // if the corresponding bit for any pair is on in
    // MUP::acc_stack_mask, a pointer to s1 and then a
    // pointer to s2 are pushed into the MUP::acc_stack.
    //
    // When a new stub is allocated, it is given the
    // flags in MUP::acc_new_stub_flags.
    //
    // Each level L has a pair of ACC flags associated
    // for use by level L markings (only one level L
    // marking executes at a time).  The result of the
    // marking is to set the unmarked flag on all level
    // >= L objects that CANNOT be reached from the
    // level L root list objects or from current threads
    // via pointers between objects, and clear the
    // unmarked flag on all other objects.  At the end
    // of the marking, the scavenged flag of each object
    // is on only if and only if the object is on the
    // level L root list or the object is >= level L and
    // its unmarked flag is cleared.
    //
    // A level L marking begins by setting the unmarked
    // flag of all level >= L objects (the level L
    // marking unmarked flag of a level < L object is
    // always clear), and clearing the scavenged flag of
    // all objects.  The level L to-be-scavenged list is
    // set to empty.  The level L marking then scavenges
    // all objects on the level L root list, all objects
    // on the to-be-scavenged list, and all current
    // threads.  To scavenge an object s1, each pointer
    // in s1 to another object s2 is examined, and if
    // the unmarked flag of s2 is on, it is cleared and
    // s2 is put on the to-be-scavenged list.  To
    // scavange a thread, the tread's list of objects
    // it points at is treated as a list object to be
    // scavenged.
    //
    // Note that at the end of a level L marking a
    // `marking termination' algorithm is run that
    // consists of scavenging the current threads and
    // then scavenging objects in the to-be-scavenged
    // list until that list is empty.  If this marking
    // termination takes too long, it is interrupted by
    // further execution of threads, and if this
    // happens, the marking termination algorithm must
    // be repeated.  Thrashing is theoretically
    // possible, but should rarely occur in practice as
    // each iteration should generate fewer objects in
    // the to-be-scavenged list.  Thrashing can be
    // avoided by refusing to interrupt the termination
    // algorithm if it has been run too many times at
    // the end of a marking.  Another option is to
    // switch to a new mode in which new objects are
    // given a clear unmarked flag and a set scavenged
    // flag.
    //
    // The following actions affect the level L marking
    // flags and the level L to-be-scavenged list:
    //
    // 	Object Creation: The unmarked flag is set and
    //  the scavenged flag is cleared.  Note that the
    //  object will be added to the list of objects the
    //  current thread is using.
    //
    //  Moving an Object from Level L1 to Level L1-1:
    //  This cannot happen during a level L1 marking,
    //  and so no marking flags are changed.  Note
    //  that below we will learn that the moved object
    //  will be put on the level L1 root list, so it
    //  can be found later to clear its scavenged flag
    //  before the next level L1 marking.
    //
    //  Level L Scavenging of an Object s1:  The
    //  scavenged flag of the object is set before the
    //  object is scavenged, and the object is removed
    //  from the to-be-scavenged list if it was taken
    //  from that list.
    //
    //  Removing an ACC::acc_stack Pointer Pair for an
    //  Object s1 Pointing at an Object s2:  If the
    //  level L marking list scavenged flag of s1 and
    //  the level L marking list unmarked flag of s2
    //  are both on, the unmarked flag of s2 is cleared.
    //
    //  Clearing the Unmarked Flag of an Object s2:
    //  s2 is put on the to-be-scavenged list.  Its
    //  scavenged flag should already be cleared.
    //
    // To maintain the level L root list a pair of ACC
    // flags is used, the level L root list flags.  The
    // unmarked flag is set for every level >= L object
    // and cleared for all other objects.  The
    // scavenged flag is set for all level < L objects
    // that are NOT on the level L root list, and
    // cleared for all other objects.  These flags
    // and the root list are maintained at ALL times.
    //
    // The following actions affect the level L root
    // list and its flags:
    //
    // 	Object Creation: The unmarked flag is set if L
    //  is the highest level and cleared otherwise (all
    //  newly created objects are put in the highest
    //  level).
    //
    //  Moving an Object from Level L1 to Level L1-1:
    //  If L == L1-1 the level L root unmarked flag is
    //  set.  If L == L1 the object is put on the level
    //  L root list (its scavenged flag should already
    //  be off).
    //
    //  Level L Scavenging of a Level L Root List
    //  Object:  If the object is found not to have any
    //  pointers to objects of level >= L, the object is
    //  removed from the root list and its scavenged
    //  flag is set.
    //
    //  Removing an ACC::acc_stack pointer Pair for an
    //  Object s1 Pointing at an Object s2:  If the
    //  level L root list scavenged flag of s1 and
    //  the level L root list unmarked flag of s1 are
    //  both on, s1 is put on the level L root list and
    //  its scavenged flag is cleared.

    // Object stubs are kept in a list, oldest stub
    // first, with the very first stub always being
    // MINT::null_stub.  Stubs are assigned generations,
    // which identified by a pair (L,S) of indices,
    // where L is the object level, and S, the senority,
    // is the number of garbage collections at level L
    // that the object must survive in order to be
    // premoted to level L-1.  For base level L = 0
    // there is no S (it is as if S were infinity).
    // Thus the indices of stubs on the list have the
    // order (0), (1,1), (1,2), (1,3), ..., (2,1),
    // (2,2), ...
    //
    struct generation
    {
        unsigned level;
	unsigned seniority;
	    // Level (L) and seniority (S) of this
	    // generation.  If L == 0 then S == 0.

	stub * last_before;
	    // Last stub on the stub list BEFORE the
	    // first stub on the list whose generation
	    // is the same as or later than (L,S).
	    //
	    // Note: MINT::null_stub is the last stub
	    // before generation (0).

	min::uns64 count;
	    // Number of stubs currently in this
	    // generation.

    } * generations;
        // Vector of all generations in ascending order
	// (0, (1,1), (1,2), ..., (2,1), (2,2), ...

    struct level
    {
        generation * g;
	    // First generation with this level in the
	    // generations vector.

	number_of_generations;
	    // Number of generations on this level.
	    // If N, then the seniorities of the
	    // generations of this level run from
	    // 1 through N, with N being the youngest.

	min::uns64 count;
	    // Number of stubs in this generation.

    } * levels;

} }


// Compactor Interface
// --------- ---------

# endif // MIN_ACC_H
