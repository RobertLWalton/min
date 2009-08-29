// MIN Allocator/Collector/Compactor Interface
//
// File:	min_acc.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Aug 29 08:08:00 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/08/29 12:08:31 $
//   $RCSfile: min_acc.h,v $
//   $Revision: 1.33 $

// The ACC interfaces described here are interfaces
// for use within and between the Allocator, Collector,
// and Compactor.  These interfaces are private and
// subject to change without notice.

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

# include <min_acc_parameters.h>
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
    //    // Address of stub with index 0.  This stub
    //    // may not exist in real memory.
    //    //
    //    // This address is used instead of the value
    //    // NULL when a real stub address is required.
    //	  // If MIN_BASE is defined this is defined to
    //    // be a constant equal to MIN_BASE; otherwise
    //    // it is the first real stub (== stub_begin
    //	  // below) and is given the DEALLOCATED type.
    //    // It is possible that MIN_BASE == stub_begin
    //    // and null_stub is both a constant and the
    //    // first real stub.
    //  MINT::pointer_uns MINT::stub_base
    //    // Ditto but as an unsigned integer.
    //	void MINT::acc_expand_free_stub_list
    //		( unsigned n )
    //	  // Function to allocate n new stubs.
    //  unsigned MINT::number_of_free_stubs
    //    // Count of stubs on free stub list.
    //  min::stub * MINT::last_allocated_stub
    //    // The control word of this stub points at the
    //    // first free stub or is MINT::null_stub if
    //    // there are no free stubs.
    //  min::uns64 MINT::acc_new_stub_flags
    //    // Flags for newly allocated garbage
    //	  // collectable stubs.

    extern min::uns64 max_stubs;
        // The value of the max_stubs parameter.  The
	// number of stubs that can be allocated to the
	// stub region.  Value may be changed when the
	// program starts.

    extern unsigned stub_increment;
        // The number of new stubs allocated by a call
        // to MINT::acc_expand_free_stub_list.

    extern min::stub * stub_begin;
    extern min::stub * stub_next;
    extern min::stub * stub_end;
        // Beginning and end of stub region, and next
	// location to be allocated in the region.
	// The end is the address just after the
	// region: stub_end = stub_begin + max_stubs.

    // Deficiency: If MIN_POINTER_SIZE <= 32 stub
    // memory should be allocated as needed and not
    // all at once.
} }


// Block Allocator Interface
// ----- --------- ---------

namespace min { namespace acc {

    // Parameters:

    // The space factor F controls some aspects of the
    // block allocator:
    //
    //	  The maximum sized block in a fixed or variable
    //	  size block region is F pages.
    //
    //    The size of a fixed or variable size block
    //	  region is F**2 pages.
    //
    // See min_acc_parameters.h for more info.
    //
    extern unsigned space_factor;

    // Cache Line Size.  Fixed blocks of equal or
    // smaller size are aligned so they are inside a
    // cache line.
    //
    extern unsigned cache_line_size;

    // Blocks:

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
    //	 if L <= 0:	R = page_address + page_size * L
    //
    //   if L > 0:	R = & MACC::region_table[L]
    //
    // where `page_address' is the address of the page
    // containing the block control word containing L.
    //
    // If a block contains an object body, the stub
    // address points at the object's stub.  Otherwise
    // the stub address == MINT::null_stub and the block
    // control word is immediately followed in memory
    // by a block subcontrol word whose type field
    // gives the block_type and whose value field
    // specifies the size in bytes of the block.
    //
    // Note: free fixed size blocks differ in NOT having
    // a subcontrol word.  Their control word has
    // MINT::null_stub as its stub address, and that
    // is sufficient to indicate that the block is
    // free, while looking up the block's region gives
    // the blocks size.  See MINT::free_fixed_size_block
    // in min.h.
    //
    enum block_type
        // Stored in block subcontrol word of blocks
	// that are NOT object bodies.
    {
        FREE				= 1,
	FIXED_SIZE_BLOCK_REGION		= 2,
	VARIABLE_SIZE_BLOCK_REGION	= 3,
	SUPERREGION			= 4,
	PAGED_BODY_REGION		= 5,
	MONO_BODY_REGION		= 6,
	STACK_SEGMENT			= 7,
	STACK_REGION			= 8
    };

    // Regions:

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
    //	     of the region.  This MACC::region struct
    //	     is padded at its end until its size is a
    //       multiple of the block size of the region
    //       or of the cache_line_size, whichever is
    //	     smaller, and then the fixed size blocks
    //	     are allocated after this padding.
    //
    //	     There is no limit to the number of fixed
    //       size block regions.
    //
    //	     Fixed size blocks are allocated from a free
    //	     list.  When an object body in a fixed size
    //	     block is freed, the block is put back on a
    //	     free list and becomes immediately available
    //	     for reuse.  See fixed_block_list_extension
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
    //	     		B - page_size < S + 8 <= B
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
    //	     typically freed, except for the first page
    //       which holds the block header, even though
    //       the surrounding pages may still be in use.
    //       Also, the pages of a multi-page block are
    //       not generally allocated until they are
    //       first used.
    //
    //	     Fixed and variable size block regions may
    //	     be allocated as multi-page block region
    //	     blocks.  The fixed and variable size
    //	     regions are `subregions' of the multi-page
    //	     block region, which is the `superregion'.
    //       Subregions begin with a MACC::region
    //       struct which in turn begins with a block
    //       control word followed by a block subcontrol
    //       word (see below).
    //
    //	     A multi-page block region has a region
    //	     control block in the MACC::region_table
    //	     vector.  There is a limit of about 2**15
    //	     such regions.
    //
    //	     A block in a multi-page block region is
    //	     always allocated after the last block prev-
    //	     iously allocated to the region.  When a
    //	     multi-page block is freed, the block is
    //	     marked as free, but cannot be reused until
    //	     the multi-page block region is compacted.
    //	     Compaction moves all used blocks in the
    //	     region to the beginning of the region, or
    //	     to another region, thus reclaiming the
    // 	     space occupied by multi-page free blocks.
    //	     Because all blocks are multi-page, moving
    //	     a block can be done by moving page table
    //	     entries, which is faster than moving bytes.

    // The following are variants of the above kinds
    // of regions:
    //
    //    Subregions
    //
    //	     A subregion is a fixed size block region
    //       or a variable size block region that is
    //       allocated to a superregion.  All subregions
    //       have the same size, so if one is freed, it
    //       can be put on a free list and reused.
    //       subregions are used to hold smaller object
    //       bodies, typically bodies up to F pages in
    //       size, where F is the space factor.  The
    //       size of a subregion is typically F**2
    //	     pages.
    //
    //    Superregions
    //
    //	     A superregion is multi-block region that
    //       holds subregions.  Superregions are put
    //       on a list, and never freed.
    //
    //    Paged Body Regions
    //
    //	     A paged body region is a multi-page block
    //       region used to hold object bodies whose
    //       size is an integral number of pages.  These
    //       are intermediate size object bodies, too
    //       large for variable or fixed size regions
    //       and too small for mono-body regions.
    //       Typically the bodies are F to F**2 pages
    //       in size.
    //
    //       Paged Body Regions are put on a list so the
    //       oldest object bodies are toward the begin-
    //       ning of the first (and oldest) paged body
    //       regions on the list.  The compactor copies
    //       bodies toward the beginning of this list
    //       and paged body regions may be freed from
    //       either the end or middle of the list.
    //	     Freed paged body regions may be deallocated
    //       or may be moved to the end of the list.
    //
    //    Mono Body Regions
    //
    //       A mono body region is a multi-page block
    //       region that holds nothing but a single
    //       object body.  These are used for very large
    //       object bodies, typicall those larger than
    //       F**2 pages.
    //
    //	     Mono body regions are put on the mono body
    //	     region list.
    //
    //    Stack Regions
    //
    //	     A stack region holds various stacks of
    //	     pointers to stubs.  These stacks are used
    //	     as lists; additions are made only at the
    //	     end of the stack; and deletions are made
    //	     either at the end of the stack, or by
    //       scanning the stack from beginning to end
    //       and deleting elements along the way,
    //       thereby `compacting' the stack.
    //
    //	     The stacks are made from stack segments
    //	     that all have the same size, a multiple
    //	     of the page size, and are stored in stack
    //       regions.  See the stack_segment stuct
    //	     below.
    //
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
	    // the word equals MINT::null_stub.
	    //
	    // In other cases, where this region control
	    // block is in the MACC::region_table, this
	    // word is unused.

        min::uns64 block_subcontrol;
	    // The type code field of this word is the
	    // type of the region and the value field
	    // is the size of the region in bytes.

	min::uns32 block_size;
	    // For fixed size block regions, the size
	    // in bytes of the fixed size blocks.
	    //
	    // For variable size block regions, the
	    // maximum size in bytes of the variable
	    // size blocks.

	min::uns32 free_count;
	    // For fixed size block regions, the number
	    // of free blocks.

	min::uns8 * begin, * next, * end;
	    // Location of the first allocatable byte of
	    // the region, the next byte to be alloca-
	    // ted, and the first byte after the last
	    // allocatable byte of the region.  For
	    // fixed and variable size block regions,
	    // the first allocatable byte is the first
	    // byte after the region's MACC::region
	    // struct that is on a MACC::cache_size
	    // boundary.

	MACC::region * region_previous,
		     * region_next;
	    // Regions of different kinds are put on
	    // doubly linked circular lists using these
	    // members.  E.g., there are lists for
	    // fixed size block regions of a given
	    // block size, for all variable size block
	    // regions, for superregions, for paged
	    // body regions, for mono body regions,
	    // for free subregions, and for stack
	    // regions.

	MINT::free_fixed_size_block * free_first;
	MINT::free_fixed_size_block * free_last;
	    // The first and last free block for a fixed
	    // size block region.

    };

    // Put region1 on the doubly linked region_previous/
    // next list after region2.
    //
    inline void insert_after
        ( region * region1, region * region2 )
    {
        region1->region_previous = region2;
	region1->region_next = region2->region_next;
	region1->region_previous->region_next = region1;
	region1->region_next->region_previous = region1;
    }

    // Put region1 on the end of the doubly linked list
    // whose last member is pointed at by `last', and
    // make region1 the new last member of the list.
    //
    inline void insert
        ( region * & last, region * region1 )
    {
        if ( last != NULL )
	    insert_after ( region1, last );
	last = region1;
    }

    // Remove region1 from the region_previous/next list
    // it is on and link it to itself.
    //
    inline void remove ( region * region1 )
    {
	region1->region_previous->region_next =
	    region1->region_next;
	region1->region_next->region_previous =
	    region1->region_previous;
        region1->region_previous = region1;
	region1->region_next = region1;
    }

    // Remove region1 from the end of the doubly linked
    // list whose last member is pointed at by `last'.
    //
    inline void remove
        ( region * & last, region * region1 )
    {
	if ( region1->region_previous == region1 )
	    last = NULL;
	else
	{
	    if ( region1 == last )
	       last = region1->region_previous;
	    remove ( region1 );
	}
    }

    // Beginning, end, and next-to-be-used region table
    // entry.  The region table can be an initial table
    // that can be moved and expanded later, or it can
    // be an initially maximum sized table that adds
    // physical pages as it grows.  Region indices in
    // the region table are limited to int16's > 0
    // (the entry at index 0 is unused).
    //
    // Region_table[0] is unused as it cannot be
    // accessed by a block control word locator.
    //
    extern region * region_table;
    extern region * region_next;
    extern region * region_end;

    const unsigned
          MAX_MULTI_PAGE_BLOCK_REGIONS = 1 << 15;

    // Pointers at the last region in various lists of
    // regions:

    extern region * last_superregion;
        // List of all superregions in order of
	// creation.

    extern region * last_free_subregion;
	// Freed subregions (fixed and variable block
	// regions), in the order they are freed.  They
	// are used in reverse order.

    extern region * last_variable_body_region;
        // All variable size block regions used for
	// object bodies, in the order they were
	// created, which is the order that the
	// object bodies in them were created.

    extern region * last_paged_body_region;
        // Ditto for paged body regions.

    extern region * last_mono_body_region;
        // Ditto for mono body regions.

    extern region * last_stack_region;
        // Ditto for stack regions.

    extern region * current_stack_region;
        // Stack region from which stack segments are
	// currently being allocated.

    // Sizes of various kind of region and size limits
    // on their contents:

    extern unsigned subregion_size;
        // Fixed block and variable block regions, which
	// have the same size so they can be freed and
	// converted from fixed to variable block or
	// vice versa.  Must be multiple of page size.

    extern unsigned superregion_size;
        // Normal size of superregion.  Superregions
	// may be as small as subregion_size if there
	// is a memory shortage.  Must be multiple of
	// page size.

    extern unsigned paged_body_region_size;
        // Size of paged body region.  Must be multiple
	// of page size.

    extern unsigned max_paged_body_size;
        // Maximum size of a paged body.  Must be
	// multiple of page size.

    extern unsigned stack_region_size;
        // Size of stack region.  Must be multiple of
	// MACC::stack_segment_size.

    extern unsigned stack_segment_size;
        // Size of a stack segment.  Must be a multiple
	// of the page size.

    extern unsigned stack_region_size;
        // Size of a stack segment in a stack region.
	// Must be a multiple of page size.

} }    

namespace min { namespace internal {

    struct fixed_block_list_extension
        // Extension of fixed_block_list struct with
	// data specific to this allocator.
    {
        MINT::fixed_block_list * fbl;
	    // Address of associated fixed_block_list.
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

namespace min { namespace acc {

    // Stub Stack Segments:

    // See Stack Regions above.

    struct stub_stack_segment
    {
        min::uns64 block_control;
	    // This is the block control word for the
	    // multi-page block containing the segment.
	    // The stub address of this field equals
	    // MINT::null_stub and the locator is the
	    // index of the Stack Region containing
	    // the segment.

        min::uns64 block_subcontrol;
	    // The type code field of this word is
	    // MACC::STACK_SEGMENT and the value field
	    // is the size of the segment in bytes.

        stub_stack_segment * previous_segment,
	                   * next_segment;
	    // Previous and next on the doubly linked
	    // list of stack segments.

	min::stub ** next, ** end;
	    // Next element of segment vector to be
	    // used, and address just after end of
	    // vector (i.e., end of segment).
	    //
	    // Note: every segment in a stack but the
	    // last must be FULL.

	min::stub * begin[];
	    // Beginning element of segment vector.
    };


    // A stub stack can be accessed in two ways.
    //
    // First, a value can be pushed into the stack using
    // push().
    //
    // Second, the stack comes with two pointers, named
    // `input' and `output'.  rewind() resets both
    // pointers to the beginning of the stack.  at_end()
    // is true if the input pointer is at the end of the
    // stack.  Otherwise current() is the value pointed
    // at by the input pointer.  remove() simply skips
    // this value.  keep() copies the value to the
    // output pointer location and increments both input
    // and output pointers.  The output pointer always
    // equals or trails the input pointer.  flush()
    // makes the output pointer position the current
    // end of the stack (the last element in the stack
    // is the last element kept by keep(), if any) and
    // then rewinds the stack.
    //
    struct stub_stack
    {
        stub_stack_segment * last_segment;
	    // Pointer to the last segment of the stack,
	    // or NULL if stack has no segments.

	stub_stack_segment * input_segment,
	                   * output_segment;
	min::stub ** input, ** output;
	    // Current input and output positions in the
	    // stack.  Each position is given by a
	    // segment and a pointer to an element in
	    // the segment.

	bool is_at_end;
	    // Value for at_end().


	stub_stack ( void ) :
	    last_segment ( NULL ),
	    input_segment ( NULL ),
	    output_segment ( NULL ),
	    input ( NULL ),
	    output ( NULL ),
	    is_at_end ( true ) {}

	void rewind ( void )
	{
	    if ( last_segment == NULL )
	    {
		input_segment = output_segment = NULL;
		input = output = NULL;
		is_at_end = true;
	    }
	    else
	    {
	        input_segment =
		    last_segment->next_segment;
		input = input_segment->begin;
		output_segment = input_segment;
		output = input;
		is_at_end =
		    ( input == input_segment->next );
	    }
	}

	bool at_end ( void )
	{
	    return is_at_end;
	}

	min::stub * current ( void )
	{
	    return * input;
	}

	void allocate_stub_stack_segment ( void );
	void push ( min::stub * s )
	{
	    if ( is_at_end
	         &&
	         (  last_segment == NULL
	            ||
		       last_segment->next
		    == last_segment->end ) )
	        allocate_stub_stack_segment();

	    * last_segment->next ++ = s;
	    is_at_end = false;
	}

	void remove ( void )
	{
	    assert ( ! is_at_end );

	    if ( ++ input == input_segment->next )
	    {
	        if ( input_segment != last_segment )
		{
		    input_segment =
		        input_segment->next_segment;
		    input = input_segment->begin;
		    is_at_end =
		        (    input
			  == input_segment->next );
		}
		else
		    is_at_end = true;
	    }
	}

	void keep ( void )
	{
	    min::stub * value = * input;
	    remove();
	    * output = value;
	    if ( ++ output == output_segment->next
	         &&
	         output_segment != last_segment )
	    {
		output_segment =
		    output_segment->next_segment;
		output = output_segment->begin;
	    }
	}
	void flush ( void );
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
    // where L is the object level, and S is the
    // sublevel.  An object in generation (L,S) that
    // survives a level L garbage collection is promoted
    // to generation (L,S-1) is S>0, or to generation
    // (L-1,T) if L>0, S=0, T is the highest sublevel of
    // level L-1, or is not promoted if L=0, S=0.
    //
    // Level 0 has only one sublevel S=0 and one
    // generation and objects are never promoted from
    // this generation even if they survive a level 0
    // garbage collection.
    //
    struct generation
    {
        unsigned level;
	unsigned sublevel;
	    // Level (L) and sublevel (S) of this
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

    };
    extern min::acc::generation * generations;
        // Vector of all generations in the order
	// (0,0), (1,0), (1,1), ..., (2,0), (2,1), ...
	// so promotions of objects are from
	// generations[i] to generations[i-1].

    struct level
    {
        generation * g;
	    // First generation with this level in the
	    // generations vector.

	unsigned number_of_sublevels;
	    // Number of sublevels (generations) on this
	    // level.  If N, then the sublevels of the
	    // generations of this level run from
	    // 1 through N, with N being the youngest.

	min::uns64 count;
	    // Number of stubs in this generation.

	MACC::stub_stack to_be_scavenged;
	MACC::stub_stack root;
	    // To-be-scavenged and root lists for
	    // the level.

    };
    extern min::acc::level * levels;

    // Ephemeral_levels is the actual number of
    // ephemeral levels, and for each ephemeral level L,
    // ephemeral_sublevels[L] is the number of sublevels
    // of ephemeral level L.
    //
    extern unsigned ephemeral_levels;
    extern unsigned * ephemeral_sublevels;

} }


// Compactor Interface
// --------- ---------

# endif // MIN_ACC_H
