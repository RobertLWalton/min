// MIN Allocator/Collector/Compactor Interface
//
// File:	min_acc.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Fri Jun  4 12:37:57 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/06/04 17:15:00 $
//   $RCSfile: min_acc.h,v $
//   $Revision: 1.51 $

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
    // is the interface to the stub allocator:
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
    //  min::unsptr MINT::stub_base
    //    // Ditto but as a min::unsptr integer.
    //	void MINT::acc_expand_free_stub_list
    //		( min::unsptr n )
    //	  // Function to allocate n new stubs and add
    //	  // them to the free list.
    //
    //    // (This actually tries to allocate n +
    //    // MACC::stub_increment new stubs, increment-
    //    // ing MACC::stub_next below, and announcing
    //    // an error if it cannot allocate at least n
    //    // new stubs.)
    //  min::unsptr MINT::number_of_free_stubs
    //    // Count of stubs on free stub list.
    //  min::stub * MINT::first_allocated_stub
    //    // The control word of this stub points at the
    //    // first garbage collectible stub or is MINT::
    //    // null_stub if there are no garbage collec-
    //    // tible stubs.
    //  min::stub * MINT::last_allocated_stub
    //    // The control word of this stub points at the
    //    // first free stub or is MINT::null_stub if
    //    // there are no free stubs.  This is the
    //    // last garbage collectible stub, or equals
    //    // MINT::first_allocated_stub if there are
    //    // no garbage collectable stubs.
    //  min::uns64 MINT::acc_new_stub_flags
    //    // Flags for newly allocated garbage
    //	  // collectable stubs.

    extern min::uns64 max_stubs;
        // The value of the max_stubs parameter.  The
	// maximum number of stubs that can be allocated
	// to the stub region.  Value may only be
	// changed when the program starts.

    extern min::unsptr stub_increment;
        // The number of extra new stubs allocated by a
	// call to MINT::acc_expand_free_stub_list.

    extern min::stub * stub_begin;
    extern min::stub * stub_next;
    extern min::stub * stub_end;
        // Beginning and end of stub region, and next
	// location to be allocated in the region.
	// The end is the address just after the
	// region: stub_end = stub_begin + max_stubs.

    // Deficiency: If MIN_POINTER_SIZE <= 32 stub
    // memory should be allocated as needed and not
    // all at once.  Possibly this should also be done
    // for larger pointer sizes, as reserving virtual
    // memory may consume system resources proportional
    // to the size of the reserved memory.

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
    extern min::unsptr space_factor;

    // Cache Line Size.  Fixed blocks of equal or
    // smaller size are aligned so they are inside a
    // cache line.
    //
    extern min::unsptr cache_line_size;

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
	STUB_STACK_SEGMENT		= 7,
	STUB_STACK_REGION		= 8
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
    //	     a `full' fixed block region.  Fixed size
    //	     block regions are intended for ephemeral
    //       object bodies that may have short life-
    //	     times.  If an object lives longer, it will
    //       be moved to a variable size block region.
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
    //	     block is freed, the block is put back on
    //	     its region's free list and becomes immedi-
    //       ately available for reuse.  See fixed_
    //       block_list_extension struct below.
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
    //       Subregions are used to hold smaller object
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
    //       object bodies, typically those larger than
    //       F**2 pages.
    //
    //	     Mono body regions are put on the mono body
    //	     region list.  Freed mono body regions are
    //       deallocated and removed from this list.
    //
    //    Stub Stack Regions
    //
    //	     A stub stack is a stack whose elements are
    //	     pointers to stubs.  These stacks are used
    //	     as lists; additions are made only at the
    //	     end of the stack; and deletions are made
    //	     either at the end of the stack, or by scan-
    //       ning the stack from beginning to end and
    ///      deleting elements along the way, thereby
    //       `compacting' the stack.
    //
    //	     Stub stacks are made from stub stack
    //	     segments that all have the same size, a
    //       multiple of the page size.   Stub stack
    //       segments are allocated to stub stack
    //       regions which are multi-page block regions.
    //
    //       See the stub_stack_segment stuct below.
    //
    struct region
    { 

        min::uns64 block_control;
	    // If this region is a subregion, it is a
	    // multi-page block allocated inside its
	    // superregion, and this is the block
	    // control word for this multi-page block.
	    // In this case the locator field L of
	    // this control word is such that
	    //
	    //	   MACC::region_table[L]
	    //
	    // is the region_table entry for the
	    // superregion, and the pointer field of
	    // this block control word equals MINT::
	    // null_stub.
	    //
	    // If this region is NOT a subregion, then
	    // this region control block is in the
	    // MACC::region_table, and this block
	    // control word is unused.

        min::uns64 block_subcontrol;
	    // The type code field of this word is the
	    // type of the region (see block_type above)
	    // and the value field is the size of the
	    // region in bytes.

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
	    // a fixed size block region, the first
	    // allocatable byte is the first byte after
	    // the region's MACC::region struct that is
	    // on either a MACC::cache_size boundary or
	    // is on a block size boundary.  For
	    // variable size block regions the first
	    // allocatable byte is the first byte after
	    // the region's MACC:region struct that is
	    // on an 8 byte boundary.
	    //
	    // For fixed size block regions the next
	    // pointer points at memory that has never
	    // been allocated to a block.  Blocks that
	    // are on the region free list have
	    // addresses that are >= begin and < next.

	MACC::region * region_previous,
		     * region_next;
	    // Regions are put on doubly linked circular
	    // lists using these members.  Each kind of
	    // region has a different list.  E.g., there
	    // are lists for fixed size block regions of
	    // a given block size, for all variable size
	    // block regions, for superregions, for
	    // paged body regions, for mono body
	    // regions, for free subregions, and for
	    // stub stack regions.

	MINT::free_fixed_size_block * free_first;
	MINT::free_fixed_size_block * free_last;
	    // The first and last free block for a fixed
	    // size block region's free list.  Note that
	    // these blocks have addresses `< next'.

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

    // Remove region1 from the doubly linked list whose
    // last member is pointed at by `last'.
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
    // the region table are limited to int16's > 0.
    //
    // Region_table[0] is unused as it cannot be
    // accessed by a block control word locator.
    //
    extern region * region_table;
    extern region * region_next;
    extern region * region_end;

    const min::unsptr
          MAX_MULTI_PAGE_BLOCK_REGIONS = 1 << 15;

    // Pointers at the last region in various lists of
    // regions:

    extern region * last_superregion;
        // List of all superregions in order of
	// creation.

    extern region * last_free_subregion;
	// Freed subregions (fixed and variable block
	// regions), with the most recently freed first.
	// The most recently freed region is the first
	// one reused.  The compactor works to free more
	// recently allocated regions first, so the
	// order of this list is likely to be approxi-
	// mately least recently allocated first.

    extern region * last_variable_body_region;
        // All variable size block regions used for
	// object bodies, in the order they were
	// created, which is the order that the
	// object bodies in them were created.

    extern region * last_paged_body_region;
        // Ditto for paged body regions.

    extern region * last_mono_body_region;
        // Ditto for mono body regions.

    extern region * last_stub_stack_region;
        // Ditto for stub stack regions.

    extern region * current_stub_stack_region;
        // Stub stack region from which stub stack
	// segments are currently being allocated.

    // Sizes of various kinds of region and size limits
    // on their contents, in bytes.  See min_acc_
    // parameters.h for defaults.

    extern min::unsptr subregion_size;
        // The size of fixed size block and variable
	// size block regions.  These regions have the
	// same size so they can be freed and converted
	// from fixed size to variable size block or
	// vice versa.  Must be multiple of the page
	// size.

    extern min::unsptr superregion_size;
        // Normal size of superregion.  Superregions
	// may be as small as subregion_size if there
	// is a memory shortage.  Must be multiple of
	// subregion_size.

    extern min::unsptr max_paged_body_size;
        // Maximum size of a paged body.  Must be
	// multiple of page size.

    extern min::unsptr paged_body_region_size;
        // Size of paged body regions used for bodies
	// (and not for subregions or stack segments).
	// Must be multiple of page size.

    extern min::unsptr stub_stack_segment_size;
        // Size of a stub stack segment.  Must be a
	// multiple of the page size.

    extern min::unsptr stub_stack_region_size;
        // Size of stub stack regions.  Must be multiple
	// of MACC::stub_stack_segment_size.

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

	MACC::region * last_region;
	MACC::region * current_region;
	    // Last region from which free blocks are
	    // being allocated, and current region from
	    // which free blocks are being allocated,
	    // in the list of all fixed block regions
	    // for this size block.  The regions in the
	    // list are chained together with their
	    // region_previous/next fields.
	    //
	    // After a GC, current_region is reset to
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

    // See Stub Stack Regions above.

    struct stub_stack_segment
    {
        min::uns64 block_control;
	    // This is the block control word for the
	    // multi-page block containing the segment.
	    // The stub address of this field equals
	    // MINT::null_stub and the locator is the
	    // index of the stub stack region containing
	    // the segment.

        min::uns64 block_subcontrol;
	    // The type code field of this word is
	    // MACC::STUB_STACK_SEGMENT and the value
	    // field is the size of the segment in
	    // bytes, which equals MACC::stub_stack_
	    // segment_size;

        stub_stack_segment * previous_segment,
	                   * next_segment;
	    // Previous and next on the doubly linked
	    // list of stub stack segments that make
	    // up a single stack.
	    //
	    // Note: All segments of a stack must be
	    // full, but the last.  The last may be
	    // empty.  An empty stack may have just
	    // one empty segment or may have no
	    // segments.

	min::stub ** next, ** end;
	    // Next element of segment vector to be
	    // used, and address just after end of
	    // vector (i.e., end of segment).

	min::stub * begin[];
	    // Beginning element of segment vector.
    };


    // A stub stack can be accessed in two ways.
    //
    // First, a value can be pushed into the stack using
    // push().  It is also to batch push operations
    // using begin_push() and end_push().
    //
    // Second, the stack comes with two pointers, named
    // `input' and `output'.  rewind() resets both
    // pointers to the beginning of the stack.  at_end()
    // is true if the input pointer is at the end of the
    // stack.  Otherwise current() is the value pointed
    // at by the input pointer.  remove() skips this
    // value by incrementing just the input pointer.
    // keep() copies the value to the output pointer
    // location and increments both input and output
    // pointers.  The output pointer always equals or
    // trails the input pointer.  flush() makes the
    // output pointer position the current end of the
    // stack (the last element in the stack is the last
    // element kept by keep(), if any) and then rewinds
    // the stack.
    //
    // When remove() is used in this way stack segments
    // are freed whenever they are between the output
    // and input pointer.  That is, a segment is freed
    // if the input pointer moves to the next segment
    // and the output pointer is not in the segment.
    //
    // It is also possible to ignore the output pointer
    // by using next() instead of remove.  This merely
    // iterates through the stack without removing any
    // elements.
    //
    struct stub_stack
    {
        stub_stack_segment * last_segment;
	    // Pointer to the last segment of the stack,
	    // or NULL if stack is empty.

	stub_stack_segment * input_segment,
	                   * output_segment;
	min::stub ** input, ** output;
	    // Current input and output positions in the
	    // stack.  Each position is given by a
	    // segment and a pointer to an element in
	    // the segment.  NULL if stack is empty.
	    //
	    // input == input_segment->next is possible
	    // only if is_at_end is true.
	    //
	    // output == output_segment->next is
	    // possible only if is_at_end is true and
	    // output pointer equals input pointer.

	bool is_at_end;
	    // Value for at_end().


	stub_stack ( void ) :
	    last_segment ( NULL ),
	    input_segment ( NULL ),
	    output_segment ( NULL ),
	    input ( NULL ),
	    output ( NULL ),
	    is_at_end ( true ) {}

	void rewind ( void );

	bool at_end ( void )
	{
	    return is_at_end;
	}

	min::stub * current ( void )
	{
	    assert ( ! is_at_end );
	    return * input;
	}

	void allocate_stub_stack_segment ( void );

	void push ( min::stub * s )
	{
	    if ( last_segment == NULL
	         ||
		    last_segment->next
		 == last_segment->end )
	        allocate_stub_stack_segment();

	    * last_segment->next ++ = s;
	    is_at_end = false;
	}

	void begin_push
	    ( min::stub ** & next, min::stub ** &end )
	{
	    if ( last_segment == NULL
	         ||
		    last_segment->next
		 == last_segment->end )
	        allocate_stub_stack_segment();

	    next = last_segment->next;
	    end = last_segment->end;
	}

	void end_push ( min::stub ** next )
	{
	    if ( last_segment->next != next )
	    {
		last_segment->next = next;
		is_at_end = false;
	    }
	}

	void remove_jump ( void );
	void remove ( void )
	{
	    assert ( ! is_at_end );

	    if ( ++ input == input_segment->next )
	        remove_jump();
	}

	void next ( void )
	{
	    assert ( ! is_at_end );

	    if ( ++ input == input_segment->next )
	    {
	        if ( input_segment != last_segment )
		{
		    input_segment =
		        input_segment->next_segment;
		    input = input_segment->begin;
		    assert (    input
			     != input_segment->next );
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
    // in higher levels.  Newly allocated objects are
    // put in the highest level N.
    //
    // The collector can run one collector marking
    // algorithm execution for each level independently
    // and in parallel.  These are known as `markings'.
    // A level L marking marks all objects in levels
    // >= L that are pointed to by objects in the level
    // L root list or by executing threads or static
    // memory, and recursively marks any object pointed
    // at by a marked object.
    //
    // The `mutator' refers to all code outside the
    // collector algorithm execution.  The mutator may
    // write pointers to objects into other objects,
    // into threads (their stacks), or into static
    // memory.
    //
    // The operation of `scavenging' an object refers to
    // examining all the pointers in the object and
    // marking any objects pointed at if the objects are
    // of the proper level.  Whenever an object is first
    // marked it is put on a to-be-scavenged list.
    //
    // Collector Lists:
    //
    //   For each level L the collector maintains three
    //   lists:
    //
    //	    Level List
    //		List of all objects in level L.  This is
    //		the ACC Stub List as per min.h.
    //	
    //	    Root List
    //		List of all objects in levels < L
    //		that might point at objects in levels
    //		>= L.
    //
    //	    To Be Scavenged List
    //		List of all objects of level >= L that
    //		remain to be scavenged during the
    //		current level L marking.
    //
    //   The collector also maintains lists not related
    //	 to any level:
    //
    //	     Thread List
    //		For each thread, a list of objects
    //		pointed at by data in the thread's
    //		stack.  Maintained by the min::
    //		stack_{num_}gen structures defined
    //		in min.h.
    //
    //	     Static List
    //		A list of objects pointed at by static
    //		data (e.g., data allocated at link
    //		time).  Maintained by the min::static_
    //		{num_}gen structures defined in min.h.
    //
    // Stub ACC Flags:
    //
    //
    //   Each stub has 4 ACC flags for each ephemeral
    //   level L > 0.  These are:
    //
    //	     scavenged	The stub's datum has been
    //		        scavenged by the current level
    //			L marking.  If the stub level is
    //			>= L the stub must have been
    //			marked by the current (or last)
    //			level L marking, and if the stub
    //			has level < L it must be on the
    //			level L root list.
    //
    //	     unmarked	The stub level is >= L and the
    //			stub has not yet been marked
    //			by the current (or last) level
    //			L marking.
    //
    //	     non-root	The stub level is < L and the
    //			stub is not on the level L root
    //			list.
    //
    //	     collectible  The stub level is >= L.  Note
    //			  that collectible flags for all
    //			  levels can be use to determine
    //			  the level of a stub.
    //
    //   There are only 2 flags for level 0 (the non-
    //   ephemeral level): the scavenged and unmarked
    //   flags.
    //
    //   Whenever a pointer to stub s2 is stored in a
    //   datum with stub s1, then:
    //
    //     if for any level L:
    //
    //		the s1 level L scavenged flag is on
    //		and
    //		the s2 level L unmarked flag is on
    //		and
    //		the MINT::acc_stack_mask level L
    //              scavenged/unmarked flag is on
    //       or
    //		the s1 level L non-root flag is on
    //          and
    //          the s2 level L collectible flag is on
    //		and
    //		the MINT::acc_stack_mask level L
    //		    non-root/collectible flag is on
    //
    //	    then the pointers to s1 and s2 are pushed
    //      into the MINT::acc_stack; the pointer to s1
    //	    is pushed first.
    //
    // MINT::acc_stack Processing:
    //
    //   The MINT::acc_stack is processed separately
    //   by the collector.  The stack contains stub
    //   pointer pairs (s1,s2) such that a pointer to
    //   s2 has been stored the datum of s1.  If the
    //   collector finds a level L scavenged stub s1
    //   pointing at a level L unmarked stub s2, it
    //   turns off the s2 level L unmarked flag, and
    //   puts s2 on the level L to-be-scavenged stack.
    //   If it finds a level L non-root stub s1 pointing
    //   at a level L collectible stub s2, it turns
    //   off the s1 level L non-root flag and puts s1
    //   on the level L root stack.
    //
    // Stub Allocation:
    //
    //   When a new stub is allocated, it is given the
    //   flags in MUP::acc_new_stub_flags.  The new stub
    //   is always put in the highest level.  Normally
    //   the flags set are as follows:
    //
    //	     all collectible flags are set
    //	     all unmarked flags are set
    //	     all scavenged flags are cleared
    //	     all non-root flags are cleared
    //
    //   In this case the new stub will remain unmarked
    //   until a pointer to it is found in a thread,
    //   in an object, or in static memory.  Many
    //   temporary objects will never be marked and will
    //   be collected promptly after becoming inacces-
    //   sible from threads and static memory.
    //
    //   If a currently running level L marking is
    //   having thrashing problems (see below), then
    //   immediately after allocating the stub with the
    //   flags given above it may be marked at level L.
    //   This means its level L unmarked flag will be
    //   cleared and it will be put on the level L
    //   to-be-scavenged list.  This keeps pointers to
    //   unmarked objects out of the thread stacks and
    //   makes further thrashing less likely, at the
    //   cost of retaining objects beyond when they are
    //   accessible.
    //   
    // Marking Execution:
    //
    //   A level L marking begins by setting the level
    //   L unmarked flag of all level >= L objects (this
    //   flag is always clear for level < L objects) and
    //   clearing the level L scavenged flag of all
    //   objects (this flag is always clear for level
    //   < L objects not on the level L root list).  The
    //   level L to-be-scavenged list is set to empty.
    //
    //   The level L marking then scavenges all objects
    //   on the level L root list and all objects on
    //   the level L to-be-scavenged list.  It treats
    //   the current thread and static lists as if they
    //   were part of a level L root object.
    //
    //   To scavenge an object s1, each pointer in s1 to
    //   another object s2 is examined, and if the level
    //   L unmarked flag of s2 is on, it is cleared and
    //   s2 is put on the level L to-be-scavenged list.
    //   So scavenging can add to the level L to-be-
    //   scavenged list.
    //
    //   Just before an object is scavenged by a level L
    //   marking, the level L scavenged flag of the
    //   object is set, and if the object is on the
    //   level L root list, its non-root flag is set.
    //   After begin scavenged, if the object is on the
    //   level L to-be-scavenged list it is removed from
    //   that list, and if it is on the level L root
    //   list and no pointers to level L collectible
    //   objects were found, it is removed from the
    //   root list, but if pointers to level L collec-
    //   tible objects were found, its non-root flag
    //   is cleared and it is kept on the list.  While
    //   an object is being scavenged the mutator may
    //   run but the MINT::acc_stack processing
    //   algorithm may not run.
    //   
    //   Marking grows the to-be-scavenged list, and
    //   prefers scavenging objects in this list to keep
    //   the list shorter.  The goal of level L marking
    //   is to scavenge all objects in the level L root
    //   list, have an empty level L to-be-scavenged
    //   list, and have scavenged the thread and static
    //   lists.  When the level L marking gets to the
    //   point when the level L root list has been
    //   scavenged and the level L to-be-scavenged list
    //   is empty, it runs a `marking termination'
    //   algorithm.
    //
    //   The marking termination algorithm scavenges the
    //   thread and static lists and any objects put on
    //   the initially empty to-be-scavenged list during
    //   scavenging.  The mutator cannot run during
    //   marking termination least it change the thread
    //   or static lists.  If the marking termination
    //   takes too long, it is aborted, the mutator is
    //   allowed to run, and after the to-be-scavenged
    //   list is empties, the marking termination
    //   algorithm is restarted from the beginning.
    //   Thrashing is theoretically possible, but should
    //   rarely occur in practice as each repeat should
    //   generate fewer objects in the to-be-scavenged
    //   list.  Thrashing can be avoided by refusing to
    //   stop and repeat marking termination if it has
    //   been run too many times at the end of a
    //   marking.  Another option is to switch to a new
    //   mode in which new objects are level L marked
    //   when they are created (and put on the to-be-
    //   scavenged list).  This further limits the
    //   number of objects that must be marked during
    //   the termination algorithm.
    //
    // Level List:
    //
    //   Object stubs are kept in a list, oldest stub
    //   first.  Stubs point at the next stub on the
    //   list using their control words.  All stubs on
    //   the list are garbage collectible.
    //
    //   The first stub on the level list is the stub
    //   pointed at by the control word of MINT::first_
    //   allocated_stub.  MINT::first_allocated_stub is
    //   itself NOT part of the level list, and has no
    //   use other than as pointing at the first stub
    //   on the level list (and possibly doubling as
    //   MINT::null_stub).
    //
    //   Newly allocated stubs are put on the end of
    //   of this list, so the list is in order of stub
    //   age.  The end of the list is MINT::last_
    //   allocated_stub.  If this equals MINT::first_
    //   allocated_stub the level list is empty (only
    //   happens briefly at the start of program ini-
    //   tialization.)
    //
    //   Stubs are assigned generations, each identified
    //   by a pair (L,S) of indices, where L is the
    //   object level, and S is the sublevel.  An object
    //   in generation (L,S) that survives a level L
    //   garbage collection is promoted to generation
    //   (L,S-1) if S>0, or to generation (L-1,T) if
    //   L>0, S=0, where T is the highest sublevel of
    //   level L-1, or is not promoted if L=0, S=0.
    //
    //   Level 0 has only one sublevel, S=0, and one
    //   generation and objects are never promoted from
    //   this generation even if they survive a level 0
    //   garbage collection.
    //
    //   The stubs in the level list are in order of
    //   increasing (L,S) indices, with (L1,S1)<(L2,S2)
    //   if and only if L1<L2 or L1=L2&S1<S2.
    //
    //   Level 0 objects in the hash tables for strings,
    //   numbers, and labels are not included in the
    //   level list.  The parts of the hash tables that
    //   point at level 0 objects are conceptually part
    //   of the level list for level 0 marking purposes,
    //   but they are not physically part of the level
    //   list.
    //
    // Collection:
    //
    //   After a level L marking, a level L collection
    //   occurs.  Objects with the level L unmarked flag
    //   are collected, beginning with the first (i.e.,
    //   oldest) level L object in the level list and
    //   ending with the last object allocated before
    //   the end of the level L marking.  Objects allo-
    //   cated after the end of the level L marking are
    //   excluded from collection.
    //
    //   Although all objects with level L unmarked flag
    //   may be collected, they have to be removed from
    //   ANY root or to-be-scavenged lists they are on,
    //   and any hash table entry pointing at them must
    //   be removed.  Fo facilitate this, level L1 > L
    //   markings are not run during a level L
    //   collection.  Note that the hash tables are not
    //   part of the root set, and having a hash table
    //   entry does NOT prevent an object from being
    //   collected.
    //
    //   Collection moves objects in level L sublevels.
    //   Each sublevel consists of a set of objects that
    //   are contiguous in the level list and that have
    //   survived a certain number of level L markings.
    //   Level L objects in sublevels S > 0 are promoted
    //   to the next lower sublevel S-1.  Objects in
    //   sublevel 0 are promoted to the highest level of
    //   the next lower level L-1.  These objects that
    //   are promoted to level L-1 are put on the level
    //   L root list so the next level L marking can
    //   clear their level L scavenged flag and check to
    //   see if they have any pointers to level >= L
    //   objects.  These objects also have their level L
    //   collectible flag and non-root flags cleared.

    // The collector execution is incremental, and as a
    // result its state must be remembered.  It is one
    // of the following:
    //
    enum { COLLECTOR_NOT_RUNNING = 0,
           COLLECTOR_START,
	   INITING_ROOT_FLAGS,
	   INITING_COLLECTIBLE_FLAGS,
	   SCAVENGING,
	   COLLECTOR_TERMINATION,
	   ROOT_REMOVAL,
	   COLLECTING,
	   COLLECTOR_FINISHED };

    // The collector execution is incremental, with a
    // `collector increment' occuring whenever an
    // allocation is done, whenever the acc_stack
    // overflows, or whenever an appropriate time period
    // has expired.  The amount of work done in a col-
    // lector increment is controlled by the following
    // parameters.
    //
    // Note then when a level L collection is running,
    // all level L1 < L collections are suspended
    // (interrupted).  So only one collection execution
    // executes in each collector increment.
    //
    // Also note that at the end of a time period zero
    // or more collector increments may be executed
    // depending upon how many collector increments have
    // been executed during the period.
    //
    extern min::uns64 scan_limit;
        // Maximum number of stubs whose flags can be
	// set during a collector increment when the
	// collector is in its flag initialization
	// phases.
	//
	// Also maximum number of min::gen values to be
	// scanned during a collector increment of the
	// scavenger.
	//
	// Also maximum number of stubs to be scanned
	// during a collector increment of the collec-
	// tion phase of the collector.

    extern min::uns64 scavenge_limit;
        // Maximum number of stubs to be scavenged
	// during a colector increment of the scavenger.

    extern min::uns64 collection_limit;
        // Maximum number of stubs to be collected
	// during a collector increment of the collec-
	// tion phase of the collector.

    extern min::uns32 collector_period;
        // Length in milliseconds of the collector
	// time period.  There is an interrupt at
	// the end of each such period.

    extern min::uns32 period_increments;
        // The number of collector increments that are
	// to be executed each period.  If fewer have
	// been executed by the end of the period, the
	// remainder are executed when the period ends.

    // Each generation is described by a generation
    // struct.
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
	    // Equals MINT::first_allocated_stub if
	    // (L,S)=(0,0).

	min::uns64 count;
	    // Number of stubs currently in this
	    // generation.

    };
    extern min::acc::generation * generations;
        // Vector of all generations in the order
	// (0,0), (1,0), (1,1), ..., (2,0), (2,1), ...
	// so promotions of objects are from
	// generations[i] to generations[i-1].

    // Each level is described by a level struct.
    //
    struct level
    {
	// Collector statistics.  These accumulate
	// across all collections of this level.

	min::uns64 root_flag_set_count;
	    // Number of root stubs whose flags were
	    // reset in the initial phase of collection.

	min::uns64 level_flag_set_count;
	    // Number of stubs collectible at this level
	    // whose flags were reset in the initial
	    // phase of collection.

	min::uns64 scanned_count;
	    // Number of min::gen or min::stub * values
	    // scanned by the scavenger.

	min::uns64 scavenge_count;
	    // Number of stubs scavenged by the
	    // scavenger.

	min::uns64 allocated_count;
	    // Number of stubs assigned to the level.

	min::uns64 promoted_count;
	    // Number of stubs promoted to next lower
	    // level by the collection phase of the
	    // collector.

	min::uns64 collected_count;
	    // Number of stubs collected by the
	    // collection phase of the collector.

	// Level working data.

        generation * g;
	    // First generation with this level in the
	    // generations vector.

	unsigned number_of_sublevels;
	    // Number of sublevels (generations) on this
	    // level.  If N, then the sublevels of the
	    // generations of this level run from
	    // 1 through N, with N being the youngest.

	MACC::stub_stack to_be_scavenged;
	MACC::stub_stack root;
	    // To-be-scavenged and root lists for
	    // the level.

	min::stub * last_allocated_stub;
	    // MINT::last_allocated_stub at the time
	    // a collector execution starts.
	min::stub * last_stub;
	    // Some collector phases iterate from
	    // level.g->last_before throught
	    // last_allocated_stub using this
	    // as a pointer.

        min::uns8 collector_state;
	    // One of MACC::collector_state.

    };
    extern min::acc::level * levels;

    // Ephemeral_levels is the actual number of
    // ephemeral levels, and for each ephemeral level L,
    // ephemeral_sublevels[L] is the number of sublevels
    // of ephemeral level L.
    //
    extern unsigned ephemeral_levels;
    extern unsigned * ephemeral_sublevels;

    // The acc stack is separately allocated from the
    // memory pool, and is not a stub stack allocated
    // by the garbage collector.  This is because there
    // is no means to switch acc stack segments, and the
    // acc stack can always be emptied, which will often
    // move its pointers to to-be-scavenged or root
    // stacks.
    //
    extern min::stub ** acc_stack_begin;
    extern min::stub ** acc_stack_end;

    // Size in bytes of the acc stack.  Must be a
    // multiple of the page size.
    //
    extern min::unsptr acc_stack_size;

    // Flag bit assignments.
    //
    const unsigned M = 56 - MIN_ACC_FLAG_BITS + 2;
    const unsigned E = MIN_MAX_EPHEMERAL_LEVELS;
    //
    //		        Range
    // Bit:	        of i:    Use:
    //
    // M - 1 + i        1 .. E   Level i collectible bit
    // M + E + i        0 .. E   Level i unmarked bit
    // M + 2E + i       1 .. E   Level i non-root bit
    // M + 3E + 1 + i   0 .. E   Level i scavenged bit
    //
    inline min::uns64 COLLECTIBLE ( unsigned i )
    {
        return min::uns64(1) << ( M - 1 + i );
    }
    inline min::uns64 UNMARKED ( unsigned i )
    {
        return min::uns64(1) << ( M + E + i );
    }
    inline min::uns64 NON_ROOT ( unsigned i )
    {
        return min::uns64(1) << ( M + 2*E + i );
    }
    inline min::uns64 SCAVENGED ( unsigned i )
    {
        return min::uns64(1) << ( M + 3*E + 1 + i );
    }

    // Given the control word of a stub return the
    // ACC level of the stub, using the fact that the
    // level i collectible flag is on iff the stub level
    // is >= i.
    //
    inline unsigned stub_acc_level
	    ( min:: uns64 stub_control )
    {
        unsigned c =
	    stub_control
	    >>
	    ( 56 - MIN_ACC_FLAG_BITS + 2 - 1 );

	// Bit 1 << i is not level i collectible bit
	// for i = 1 .. e.

	c &=   ( 1 << ( MIN_MAX_EPHEMERAL_LEVELS + 1 ) )
	     - 1;
	c |= 1;
	return min::internal::log2floor ( c );
    }

    // Process stub pointer pairs from the end of the
    // acc stack until the acc stack pointer is less
    // than or equal to acc_lower.
    //
    void process_acc_stack
        ( min::stub ** acc_lower =
	      min::acc::acc_stack_begin );
} }


// Compactor Interface
// --------- ---------

# endif // MIN_ACC_H
