// MIN Allocator/Collector/Compactor Interface
//
// File:	min_acc.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jul  3 15:19:43 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/07/04 04:06:00 $
//   $RCSfile: min_acc.h,v $
//   $Revision: 1.79 $

// The acc interfaces described here are interfaces
// for use within and between the Allocator, Collector,
// and Compactor.  These interfaces are private and
// subject to change without notice.

// Table of Contents
//
//	Usage and Setup
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
    // stub region, a contiguous sequence of virtual
    // pages that can hold MACC::max_stubs stubs.  It
    // then allocates new stubs as needed from the
    // beginning of this region working up.  Newly
    // added stubs are appended to the list of free
    // stubs headed by the control word of MINT::
    // last_allocated_stub.
    //
    // The Collector, and NOT the Allocator, frees
    // acc stubs and adds the freed stubs to the list
    // of free stubs.  The Allocator is only called to
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
    //    // first acc or free stub or is MINT::
    //    // null_stub if there are no acc stubs or
    //    // free stubs.  MINT::first_allocated_stub
    //    // may or may not equal MINT::null_stub.
    //  min::stub * MINT::last_allocated_stub
    //    // The control word of this stub points at the
    //    // first free stub or is MINT::null_stub if
    //    // there are no free stubs.  This is the
    //    // last acc stub, or equals MINT::first_
    //    // allocated_stub if there are no acc stubs.
    //  min::uns64 MINT::new_acc_stub_flags
    //    // Flags for newly allocated acc stubs.
    //
    // Auxiliary (aux) stubs are not managed by the
    // acc.  They are allocated from and returned to
    // the free list by non-acc code, and are NOT
    // placed on the acc list (defined below).

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
    //	  region is F**2 pages.  As the size of a fixed
    //    size block region must be <= 2**15 pages,
    //    because locators are only 16 bits, and because
    //    F must be a power of two, F <= 2**7 = 128.
    //
    // See min_acc_parameters.h for more info.
    //
    extern min::unsptr space_factor;

    // Cache Line Size.  Fixed blocks of equal or
    // smaller size are aligned so they are inside a
    // cache line.
    //
    extern min::unsptr cache_line_size;

    // Sizes of various kinds of region and size limits
    // on their contents, in bytes.  See description of
    // regions below, and see min_acc_parameters.h for
    // defaults.

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

    // Blocks:

    // Bodies are stored in blocks.  Each block begins
    // with a block control word, and this is followed
    // by either a body or, if the block does not
    // contain a body, by a block subcontrol word.
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
    // If a block contains a body, the stub address
    // points at the body's stub.  Otherwise the stub
    // address == MINT::null_stub and the block control
    // word is immediately followed in memory by a block
    // subcontrol word whose type field gives the block_
    // type and whose value field specifies the size in
    // bytes of the block.
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
	// that are NOT bodies.
	//
	// Also stored in the block_subcontrol word of
	// region structs that are elements of the
	// region_table.
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
    //	     A body of size S is allocated to fixed size
    //       block of size B as follows:
    //
    //		8 bytes			control word
    //		S bytes			body
    //		B - S - 8 bytes		unused padding
    //
    //		where B/2 < S + 8 <= B
    //
    //	     Note that unused padding can take up 50% of
    //	     a `full' fixed block region.  Fixed size
    //	     block regions are intended for ephemeral
    //       bodies that may have short life-times.  If
    //       a body lives longer, it will be moved to a
    //       variable size block region.
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
    //	     list.  When an body in a fixed size block
    //       is freed, the block is put back on its
    //       region's free list and becomes immediately
    //       available for reuse.  See fixed_block_list_
    //       extension struct below.
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
    //	     A body of size S is allocated to a variable
    //       size block of size B as follows:
    //
    //		8 bytes			control word
    //		S bytes			body
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
    //	     allocated to the region.  When a body in a
    //       variable size block is freed, the block is
    //       marked as free, but cannot be reused until
    //       the variable size block region is compac-
    //       ted.  Compaction moves all used blocks in
    //       the region to the beginning of the region,
    //       or to another region, thus reclaiming the
    //       space occupied by variable size free
    //       blocks.
    //
    //	     Variable size block regions are intended
    //	     for longer lived bodies.
    //
    //	 Paged Block Regions
    //
    //	     The region consists of variable size blocks
    //	     each of which is a sequence of pages.
    //  
    //	     If a body of size S is allocated to a
    //       variable size block of size B as follows:
    //
    //		8 bytes			control word
    //		S bytes			body
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
    //	     Paged block regions may be used for larger
    //       bodies.  When a paged block is freed, the
    //       pages of a block are typically freed,
    //       except for the first page which holds the
    //       block header, even though pages surrounding
    //       the block may still be in use.  Also, the
    //	     pages of a paged block are not generally
    //       allocated until they are first used.
    //
    //	     Fixed and variable size block regions are
    //	     allocated as paged blocks in a paged block
    //       region.  The fixed and variable size
    //	     regions are `subregions' of the paged block
    //	     region, which is the `superregion'.
    //       Subregions begin with a MACC::region
    //       struct which in turn begins with a block
    //       control word followed by a block subcontrol
    //       word (see below).
    //
    //       A paged block region may be used as a paged
    //       body region (see below) in which large
    //       bodies are directly allocated.  It may
    //       also be used as a mono-body region (also
    //       see below) in which just one very large
    //       body is allocated.
    //
    //	     A paged block region has a region control
    //       block in the MACC::region_table vector.
    //       This vector can have at most 2**15 ele-
    //       ments, thereby limiting the number of
    //       paged block regions.
    //
    //	     A block in a paged block region is always
    //       allocated after the last block previously
    //       allocated to the region.  When a paged
    //       block is freed, the block is marked as
    //       free, but cannot be reused until the paged
    //       block region is compacted.  Compaction
    //       moves all used blocks in the region to the
    //       beginning of the region, or to another
    //       region, thus reclaiming the virtual memory
    //       space occupied by free paged blocks.
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
    //       Subregions are used to hold smaller bodies,
    //       typically bodies up to F pages in size,
    //       where F is the space factor.  The size of a
    //       subregion is typically F**2 pages.
    //
    //    Superregions
    //
    //	     A superregion is paged block region that
    //       holds subregions.  Superregions are put on
    //       a list, and never freed.
    //
    //    Paged Body Regions
    //
    //	     A paged body region is a paged block region
    //       used to hold bodies whose size is an inte-
    //       gral number of pages.  These are intermed-
    //       iate size bodies, too large for variable or
    //       fixed size regions and too small for mono-
    //       body regions.  Typically the bodies are F
    //       to F**2 pages in size.
    //
    //       Paged Body Regions are put on a list so the
    //       oldest bodies are toward the beginning of
    //       the first (and oldest) paged body regions
    //       on the list.  The compactor copies bodies
    //       toward the beginning of this list and paged
    //       body regions may be freed from either the
    //       end or middle of the list.  Freed paged
    //       body regions may be deallocated or may be
    //       moved to the end of the list.
    //
    //    Mono Body Regions
    //
    //       A mono body region is a paged block region
    //       that holds nothing but a single body.
    //       These are used for very large bodies, typi-
    //       cally those larger than F**2 pages.
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
    //       regions which are paged block regions.
    //
    //       See the stub_stack_segment stuct below.
    //
    struct region
    { 
        // Note: unless specified otherwise, super-
	// regions and stub stack regions are treated
	// in the following as fixed size block regions
	// whose block size is the size of a subregion
	// or stub stack segment.

        min::uns64 block_control;
	    // If this region is a subregion, it is a
	    // paged block allocated inside its
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
	    // block_type of the region (see above)
	    // and the value field is the size of the
	    // region in bytes.

	min::unsptr block_size;
	    // For fixed size block regions, the size
	    // in bytes of the fixed size blocks.
	    //
	    // For variable size block regions, the
	    // maximum size in bytes of the variable
	    // size blocks.

	min::unsptr free_count;
	    // For fixed size block regions, the number
	    // of free blocks in the region.
	    //
	    // For variable size block regions, the
	    // number of bytes in free blocks that are
	    // in the region.
	    //
	    // In no case does memory between `next' and
	    // `end' (see below) count as free in the
	    // sense of this value.

	min::unsptr max_free_count;
	    // For fixed size block regions, the value
	    // of free_count that indicates that the
	    // regions consists of nothing but free
	    // blocks.

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
	    // For fixed size block regions the `next'
	    // pointer points at memory that has never
	    // been allocated to a block.  Blocks that
	    // are on the region free list have
	    // addresses that are >= begin and < next.
	    // The value `end - begin' is always an
	    // exact multiple of block_size, and
	    // consequently `end' may not point beyond
	    // the region itself.

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
	    // stub stack regions.  If a region is not
	    // on a list, these point at the region
	    // itself.

	MINT::free_fixed_size_block * last_free;
	    // The last free block in the circular fixed
	    // size block region's free list.  Note that
	    // these blocks have addresses `< next'.  If
	    // there are no free blocks, equals NULL.

    };

    inline min::unsptr size_of ( region * r )
    {
        return min::unprotected::value_of_control
	     ( r->block_subcontrol );
    }

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

    extern region * last_free_region;
        // List of all free region struct's.  If empty,
	// new regions structs are to be allocated
	// at region_next.

    extern region * last_superregion;
        // List of all superregions in order of
	// creation.

    extern region * current_superregion;
        // Superregion from which subregions are
	// currently being allocated.  When this
	// is exhausted, it is reset to the first
	// superregion with free subregions.

    extern region * last_variable_body_region;
        // All variable size block regions used for
	// bodies, in the order they were created,
	// which is the order that the bodies in them
	// were created.

    extern region * last_paged_body_region;
        // Ditto for paged body regions.

    extern region * last_mono_body_region;
        // Ditto for mono body regions.

    extern region * last_stub_stack_region;
        // Ditto for stub stack regions.

    extern region * current_stub_stack_region;
        // Stub stack region from which stub stack
	// segments are currently being allocated.
	// When this is exhausted, it is reset to
	// the first stub stack region with free
	// stub stack segments.

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
	    // region_previous/next fields, oldest
	    // first.
	    //
	    // fbl is set from info in current_region.
	    // When current_region runs out of free
	    // blocks, an oldest first search is made
	    // to find a new current region with free
	    // blocks, and if none is found, a new
	    // fixed block region is allocated to be
	    // the new current region.
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
	    //
	    // Here `full' means `next == end' and
	    // `empty' means `next == begin'.

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
    // push().  It is also possible to batch push
    // operations using begin_push() and end_push().
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
    // if the output pointer is not in that segment when
    // the input pointer moves from that segment to the
    // next segment.
    //
    // It is also possible to ignore the output pointer
    // by using next() instead of remove().  This merely
    // iterates through the stack without removing any
    // elements.
    //
    // Currently there is NO pop operation on stacks.
    // So the stub stacks are closer to FIFO lists than
    // true stacks.  A pop operation could easily be
    // implemented.
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

	// ss.begin_push ( next, end ) returns the next
	// and end pointers of the stub stack ss.  min::
	// stub * values may be pushed into the stack
	// by * next ++ = ... until next >= end.  Then
	// ss.end_push ( next ) should be called to
	// store next back into the stub stack.
	//
	void begin_push
	    ( min::stub ** & next, min::stub ** & end )
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

    // The `collector' segregates stubs into N+1
    // levels, with level 0 being the `base level',
    // and levels 1, ..., N being `ephemeral levels'.
    // Stubs in lower levels are older than stubs
    // in higher levels.  Newly allocated stubs are
    // put in the highest level N.
    //
    // The collector can run one collector marking
    // algorithm execution for each level independently
    // and in parallel.  These are known as `markings'.
    // A level L marking marks all stubs in levels
    // >= L that are pointed to by stubs in the level
    // L root list or by executing threads or static
    // memory, and recursively marks any stub pointed
    // at by a marked stub.
    //
    // The `mutator' refers to all code outside the
    // collector algorithm execution.  The mutator may
    // write pointers to stubs into other stubs or
    // their bodies, into threads (i.e., their stacks),
    // or into static memory.
    //
    // The operation of `scavenging' a stub refers to
    // examining all the pointers in the stub and
    // marking any stubs pointed at if the stubs are
    // of the proper level.  Whenever a stub is first
    // marked it is put on a to-be-scavenged list.
    //
    // Collector Lists:
    //
    //   For each level L the collector maintains three
    //   lists:
    //
    //	    Level List
    //		List of all stubs in level L.  This is
    //		a segment of the acc stub list as per
    //		min.h.
    //	
    //	    Root List
    //		List of all stubs in levels < L
    //		that might point at stubs in levels
    //		>= L.
    //
    //	    To Be Scavenged List
    //		List of all stubs of level >= L that
    //		remain to be scavenged during the
    //		current level L marking.
    //
    //   The collector also maintains lists not related
    //	 to any level:
    //
    //	     Thread List
    //		For each thread, a list of stubs pointed
    //          at by data in the thread's stack.
    //          Maintained by the min::stack_{num_}gen
    //          structures defined in min.h.
    //
    //	     Static List
    //		A list of stubs pointed at by static
    //		data (e.g., data allocated at link
    //		time).  Maintained by the min::static_
    //		{num_}gen structures defined in min.h.
    //
    // Stub ACC Flags:
    //
    //   Each acc stub has 4 acc flags for each ephem-
    //   eral level L > 0.  These are:
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
    //			stub has NOT yet been marked
    //			by the current (or last) level
    //			L marking.
    //
    //	     non-root	The stub level is < L and the
    //			stub is NOT on the level L root
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
    //   Whenever the mutator stores a pointer to stub
    //   s2 in a datum with stub s1, then the mutator
    //   executes the following:
    //
    //     if for any level L:
    //
    //		the s1 level L scavenged flag is on
    //		and
    //		the s2 level L unmarked flag is on
    //		and
    //		the MINT::acc_stack_mask level L
    //              unmarked flag is on
    //       or
    //		the s1 level L non-root flag is on
    //          and
    //          the s2 level L collectible flag is on
    //		and
    //		the MINT::acc_stack_mask level L
    //		    collectible flag is on
    //
    //	    then the pointers to s1 and s2 are pushed
    //      into the MINT::acc_stack; the pointer to s1
    //	    is pushed first.
    //
    // MINT::acc_stack Processing:
    //
    //   The MINT::acc_stack is processed separately by
    //   the MACC::process_acc_stack routine which is
    //   run at the same times and in preference to
    //   other acc functions.
    //
    //   The MINT::acc_stack contains stub pointer pairs
    //   (s1,s2) such that a pointer to s2 has been
    //   stored in the datum of s1 (in s1 or its body).
    //   The MINT::process_acc_stack routine processes
    //   these pairs.
    //
    //   If the MINT::process_acc_stack routine finds a
    //   level L scavenged stub s1 pointing at a level L
    //   unmarked stub s2, and if the acc_stack_mask
    //   level L unmarked flag is on, the routine turns
    //   off the s2 level L unmarked flag, and if s2 is
    //   scavengable, puts s2 on the level L to-be-sca-
    //   venged stack.  A stub is scavengable if and
    //   only if MINT::scavenger_routine[type_of(s2)] is
    //   not NULL.
    //   
    //   If the MINT::process_acc_stack routine finds a
    //   level L non-root stub s1 pointing at a level L
    //   collectible stub s2, and if the acc_stack_mask
    //   level L collectible flag is on, the routine
    //   turns off the s1 level L non-root flag and puts
    //   s1 on the level L root stack.  In addition, if
    //   a level L collection is in progress and is in
    //   the SCAVENGING_ROOT or SCAVENGING_THREAD
    //   phases, then the s1 level L scavenged flag is
    //   turned on, and if the s2 level L unmarked flag
    //   is on, it is turned off and s2 is put on the
    //   level L to-be-scavenged list.  This is an
    //   efficiency measure as the new root s1 contains
    //   only one pointer at a stub of level >= L,
    //   namely the pointer to s2, so there is no need
    //   to scavenge s1 to find other such pointers.
    //
    // Stub Allocation:
    //
    //   When a new stub is allocated, it is given the
    //   flags in MUP::new_acc_stub_flags.  The new stub
    //   is always put in the highest level.  Normally
    //   the flags set are as follows:
    //
    //	     all collectible flags are set
    //	     all unmarked flags are set according to
    //		 the state of the collector; most of
    //		 the time each unmarked flag is cleared
    //	     all scavenged flags are cleared
    //	     all non-root flags are cleared
    //
    //   In this case the new stub will remain unmarked
    //   until a pointer to it is found in a thread,
    //   in a stub, or in static memory.  Many
    //   temporary stubs will never be marked and will
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
    //   unmarked stubs out of the thread stacks and
    //   makes further thrashing less likely, at the
    //   cost of retaining stubs beyond when they are
    //   accessible.
    //
    // Acc List:
    //
    //   Acc stubs are kept in a list, oldest stub
    //   first, called the acc stub list.  Stubs point
    //   at the next stub on this list using their
    //   control words.
    //
    //   Stubs are assigned generations, each identified
    //   by a pair (L,S) of indices, where L is the stub
    //   level, and S is the sublevel.  A stub in gen-
    //   eration (L,S) that survives a level L collec-
    //   tion is promoted to generation (L,S-1) if S>0,
    //   or to generation (L-1,H) if L>0, S=0, where H
    //   is the highest sublevel of level L-1, or is not
    //   promoted if L=0, S=0.
    //
    //   Level 0 has only one sublevel, S=0, and one
    //   generation and stubs are never promoted from
    //   this generation even if they survive a level 0
    //   collection.
    //
    //   The stubs in the acc list are in order of
    //   increasing (L,S) indices, with (L1,S1)<(L2,S2)
    //   if and only if L1<L2 or L1=L2&S1<S2.
    //
    //   The first stub on the acc list is the stub
    //   pointed at by the control word of MINT::first_
    //   allocated_stub.  MINT::first_allocated_stub is
    //   itself NOT part of the acc list, and has no use
    //   other than as pointing at the first stub on the
    //   acc list (and possibly doubling as MINT::null_
    //   stub).
    //
    //   Newly allocated stubs are put on the end of the
    //   acc list, thus keeping the acc list in order of
    //   stub age.  The end of the acc list is MINT::
    //   last_allocated_stub.  If this equals MINT::
    //   first_allocated_stub the acc list is empty
    //   (only happens briefly at the start of program
    //   initialization.)
    //
    //   Level 0 stubs in the hash tables for strings,
    //   numbers, and labels are not included in the acc
    //   list.  The parts of the hash tables that point
    //   at level 0 stubs are conceptually part of the
    //   level 0 list, but they are not physically part
    //   of the acc list.
    //   
    // Collector Increments:
    //
    //   The collector executes in increments called
    //   `collector increments' of limited duration in
    //   order to run intermixed with normal mutator
    //   execution without consuming all the CPU time.
    //   To implement collector increments the collector
    //   must maintain state information for each level,
    //   and this includes a `phase code' that specifies
    //   which phase the collection at that level is in.
    //
    //   Some collector increments lock and unlock
    //   certain data.  It is an assert error if a
    //   collector increment tries to lock already
    //   locked data, and it is up to the collector
    //   scheduler, which decides which collector
    //   increment to run next, to avoid such errors.
    //   In general, that state PRE_XXX sets locks and
    //   immediately switches to the state XXX.  A
    //   collector increment in state XXX may end with
    //   the state reset to PRE_XXX and all the phases
    //   locks unlocked, permitting interrupts at that
    //   point by other collector increments.
    //
    //   The data that is locked and the phases that
    //   set and clear these locks are as follows:
    //
    //	     Let
    //	       L = the level of the collector increment
    //         level = levels[L].collection_level >= L
    //         sublevel = levels[L].collection_sublevel
    //			= sublevel of level
    //
    //       levels[level].g[sublevel].lock
    //       levels[level].g[sublevel+1].lock
    //		Set by:
    //		PRE_INITING_COLLECTIBLE (level, sublevel)
    //		PRE_PROMOTING (level, sublevel)
    //		PRE_COLLECTING (level, sublevel)
    //		    each sets the locks for
    //			levels[level].g+sublevel
    //			and
    //			levels[level].g+sublevel+1
    //	        Cleared at end of:
    //		INITING_COLLECTIBLE
    //		PROMOTING
    //		COLLECTING
    //
    //	     levels[L].lock
    //		Set by:
    //		COLLECTOR_START
    //		Cleared at end of:
    //		Last phase of collection.
    //
    //	     levels[level].lock
    //		Set by:
    //		PRE_REMOVING_ROOT ( level )
    //	        Cleared at end of:
    //		REMOVING_ROOT ( level )
    //
    //   The possible phases for level L are follow in
    //   the order the phases are executed, except the
    //	 execution order may include PRE_XXX XXX PRE_XXX
    //   XXX ....
    //
    enum { COLLECTOR_NOT_RUNNING = 0,
    		// A level L collection is NOT in
		// progress.
           COLLECTOR_START,
		// The levels[L].lock is set, a level L
		// collection is started and collection
		// variables are initialized.  The lock
		// is kept until the last phase of the
		// collection is done.
		//
		// The level L to-be-scavenged list
		// should be empty at this point.  The
		// MINT::acc_stack_mask level L unmarked
		// flag is set to enable the mutator
		// along with acc stack processing to
		// mark stubs and add to the level L to-
		// be-scavenged list.  The level L
		// unmarked flag is set in MUP::new_acc_
		// stub_flags so it will be set in any
		// newly allocated stubs.
	   PRE_INITING_COLLECTIBLE,
	   INITING_COLLECTIBLE,
		// Iterates over all levels >= L and all
		// sublevels of each level.  For each
		// level and sublevel locks
		//   levels[level].g[sublevel].lock
		//   levels[level].g[sublevel+1].lock
		// These locks are only kept until the
		// sublevel iteration is done.
		//
	        // For each iteration all stubs of the
		// sublevel in the acc list are scanned,
		// and each scanned stub has its
		// unmarked flag set and its scavenged
		// flag cleared.
	   INITING_ROOT,
	   	// The level L root list is scanned and
		// each stub on this list has its
		// scavenged flag cleared.
	   INITING_HASH,
	   	// Done only for L == 0.  The acc hash
		// tables are scanned, and each stub in
		// these tables list has its unmarked
		// flag set and its scavenged flag
		// cleared.
	   SCAVENGING_ROOT,
	        // Each stub on the level L root list
		// is scavenged.  In the process stubs
		// are put on the to-be-scavenged list,
		// and these are also scavenged.
		//
		// To scavenge a stub s1, each pointer
		// in s1 or its body to another stub s2
		// is examined, and if the level L
		// unmarked flag of s2 is on, it is
		// cleared and s2 is put on the level L
		// to-be-sca-venged list if it is
		// scavengable.  s2 is scavengable if
		// and only if MINT::scavenger_routines
		// [type_of(s2)] is not NULL.
		//
		// Scavenging object is done by the
		// MINT::scavenger_routines.
		//
		// Scavenging a large stub body is done
		// with multiple collector increments,
		// so the mutator can interrupt the
		// scavenging of a large stub body.
		//
		// The level L scavenged flag of s1 is
		// set just before s1 is scavenged, so
		// that if the mutator stores a pointer
		// to a level L unmarked stub s2 into
		// s1 in the middle of scavenging s1, 
		// the acc_stack mechanism will clear
		// the level L unmarked flag of s2 and
		// put s2 on the level L to-be-scavenged
		// list.
		//
		// The level L not-root flag of s1 is
		// set just before s1 is scavenged if
		// s1 is on the level L root list.
		// If any pointer to a stub s2 that
		// is of level >= L is discovered in
		// s1, then after s1 is scavenged its
		// non-root flag is cleared and it is
		// left on the root list.  Also if s1's
		// non-root flag is cleared while it is
		// being scavenged by the action of the
		// mutator and acc stack processing,
		// then s1 is left of the root list.
		// Otherwise s1 is removed from the root
		// list (it may be restored to the root
		// list by the action of the mutator
		// and acc stack at any time).
		// 
		// The goal of this phase is to scavenge
		// all the level L root list stubs and
		// empty the level L to-be-scavenged
		// list.  Emptying the to-be-scavenged
		// list is given priority over scaveng-
		// ing the next root stub, in order to
		// keep the to-be-scavenged list short.
		//
		// Stubs may be added to the end of the
		// level L root list by the mutator via
		// the acc stack during this phase, but
		// such stubs have their level L
		// scavenged flag set when they are
		// added to the root list (see acc
		// stack processing above), and need not
		// be scavenged by this phase.  When
		// this phase encounters the first root
		// list stub with its scavenged flag
		// already set, this phase knows it has
		// finished with the root list, and
		// merely needs to empty the to-be-
		// scavenged list to finish.
	   SCAVENGING_THREAD,
	        // The thread list of each thread and
		// the static list (see above) are
		// treated as if they were the body of
		// a single root stub and scavenged by
		// the MINT::thread_scavenger_routine.
		// The goal of this phase is to scavenge
		// the thread/static lists and empty the
		// to-be-scavenged list (which may be
		// added to by scavenging the thread/
		// static lists).
		//
		// However, if this scavenging is
		// interrupted by the mutator (i.e., if
		// a phase collector increment termin-
		// ates with the scavenging unfinished),
		// the thread/static scavenging must be
		// restarted at its beginning.  This can
		// cause thrashing, but as this phase
		// runs it decreases the number of
		// pointers in the thread/static lists
		// that point at unmarked stubs, and
		// thereby decreases the likelyhood that
		// thread/static scavenging will be
		// interrupted by the mutator.
		//
		// To prevent indefinite thrashing, the
		// duration of the collector increment
		// is increased as a function of the
		// number of times scavenging the
		// thread/static lists has been restart-
		// ed.
		//
		// As an alternative, the allocator may
		// be set to both clear the level L
		// unmarked flag of any newly allocated
		// stub and put the stub on the to-be-
		// scavenged list.  This can greatly
		// reduce the number of level L unmarked
		// stubs encountered when scavenging the
		// thread/static lists.
		//
	        // At the end of this phase, all level L
		// stubs with unmarked flag set cannot
		// be reached by the mutator.  Exactly
		// at the end of this phase the level L
		// unmarked flag is cleared in MUP::new_
		// acc_stub_flags so any stubs allocated
		// afterwards will not be collected by
		// the level L collection.
	        //
	   PRE_COLLECTING,
	   COLLECTING,
		// Iterates over all levels >= L and all
		// sublevels of each level.  For each
		// level and sublevel locks
		//   levels[level].g[sublevel].lock
		//   levels[level].g[sublevel+1].lock
		// These locks are only kept until the
		// sublevel iteration is done.
		//
	        // The portion of the acc list for
		// level L stubs that was not scanned
		// in the PROMOTING phase is scanned and
		// all scanned stubs with level L
		// unmarked flag set are deallocated.
		// The sublevel S of each stub is
		// in effect decremented if S > 0; this
		// is done by modifying last_before
		// pointers in generation stucts.
		//
		// At the end of this phase the level L
		// collector returns to the COLLECTOR_
		// NOT_RUNNING state.
	   PRE_PROMOTING,
	   PROMOTING,
		// Iterates over all levels >= L and all
		// sublevels of each level.  For each
		// level and sublevel locks
		//   levels[level].g[sublevel].lock
		//   levels[level].g[sublevel+1].lock
		// These locks are only kept until the
		// sublevel iteration is done.
		//
	        // This is done only for levels L > 0.
		// The portion of the acc list for level
		// L and sublevel 0 is scanned.  Each
		// scanned stub with unmarked flag set
		// is deallocated.  Each scanned stub
		// with unmarked flag clear is put on
		// the level L root list, and its level
		// L collectible and non-root flags are
		// cleared.
		//
		// Then the level L sublevel 0 portion
		// of the acc list is promoted to be the
		// end of the level L-1 sublevel H por-
		// tion of the acc list, where H is the
		// highest sublevel of level L-1.  This
		// is done by modifying last_before
		// pointers in generation stucts.
		//
		// Note that any deallocated number,
		// string, or label stub must be removed
		// from its hash table.  Being in a hash
		// table does not prevent collection of
		// any stub.
	   REMOVING_ROOT };
		// Iterates over all levels > L.  For
		// For each level locks
		//   levels[level]lock
		// This lock is only kept until the
		// level iteration is done.
		//
	   	// For each level its root list is
		// scanned and all scanned stubs with
		// level L collectible and unmarked
		// flags both set are removed.

    // The amount of work done in a collector increment
    // is controlled by the following parameters.  Note
    // that collection increments cannot be split among
    // more than one collection execution, and each
    // collection execution has its own level, so each
    // collection increment has a unique level.
    //
    // Also note that at the end of a time period zero
    // or more collector increments may be executed
    // depending upon how many collector increments have
    // been executed during the period.
    //
    extern min::uns64 scan_limit;
        // Maximum number of stubs whose flags can be
	// set during a collector increment when the
	// collector is in its INITING_COLLECTIBLE or
	// INITING_ROOT phases.
	//
	// Also maximum number of min::gen values to be
	// scanned during a collector increment of the
	// SCAVENGE_ROOT or SCAVENGE_THREAD phases.
	//
	// Also maximum number of stubs to be scanned
	// during a collector increment of the PROMOTING
	// or COLLECTING phases.

    extern min::uns64 scavenge_limit;
        // Maximum number of stubs to be scavenged
	// during a colector increment of the SCAVENGE_
	// ROOT or SCAVENGE_THREAD phases.

    extern min::uns64 collection_limit;
        // Maximum number of stubs to be collected
	// during a collector increment of the PROMO-
	// TING or COLLECTING phases.

    extern min::uns32 collector_period;
        // Length in milliseconds of the collector
	// time period.  There is an interrupt at
	// the end of each such period.  0 if there
	// is not collector time period.

    extern min::uns32 period_increments;
        // The number of collector increments that are
	// to be executed each period.  If fewer have
	// been executed by the end of the period, the
	// remainder are executed when the period ends.

    // Each generation is described by a generation
    // struct.
    //
    struct level;
    struct generation
    {
        MACC::level * level;
	    // Level of this generation.

	stub * last_before;
	    // Last stub on the acc stub list BEFORE the
	    // first stub on the list whose generation
	    // is the same as or later than (L,S).
	    // Equals MINT::first_allocated_stub if
	    // (L,S)=(0,0).

	min::uns64 count;
	    // Number of stubs currently in this
	    // generation.

	bool lock;
	    // Set `true' to lock this generation
	    // structure.  Locked by the {PRE_}INITING_
	    // COLLECTIBLES, {PRE_}PROMOTING, and
	    // {PRE_COLLECTING} phases both of this
	    // generation's level and of lower levels.

    };
    extern min::acc::generation * generations;
        // Vector of all generations in the order
	// (0,0), (1,0), (1,1), ..., (2,0), (2,1), ...
	// so promotions of stubs are from
	// generations[i] to generations[i-1].  Also
	// see end_g.

    extern min::acc::generation * end_g;
	// Points at an extra generation struct at the
	// end of the generations vector which is used
	// by some collector phases which set
	//	end_g->last_before =
	//	    levels[level].last_allocated
	// and  end_g->lock = true.

    // Each acc level is described by a level struct.
    //
    struct level
    {
	// Collector statistics.  These accumulate
	// across all collections of this level.

	min::uns64 root_flag_set_count;
	    // Number of root stubs whose flags were
	    // reset in the initial phase of collection.

	min::uns64 collectible_flag_set_count;
	    // Number of stubs collectible at this level
	    // whose flags were reset in the initial
	    // phase of collection.

	min::uns64 scanned_count;
	    // Number of min::gen or min::stub * values
	    // scanned by the scavenger.

	min::uns64 stub_scanned_count;
	    // Number of stub pointer containing
	    // min::gen or min::stub * values
	    // scanned by the scavenger.

	min::uns64 scavenged_count;
	    // Number of stubs scavenged by the
	    // scavenger.

	min::uns64 root_kept_count;
	    // Number of root stubs kept during
	    // REMOVING_ROOT phase.

	min::uns64 root_removed_count;
	    // Number of root stubs removed during
	    // REMOVING_ROOT phase.

	min::uns64 collected_count;
	    // Number of stubs collected by the
	    // collection phases of the collector.

	min::uns64 kept_count;
	    // Number of stubs kept but by the
	    // collection phases of the collector.

	min::uns64 promoted_count;
	    // Number of stubs promoted from one level
	    // to the next lower level by the
	    // collection phases of the collector.

	min::uns64 thrash_count;
	    // Number of times the this level's collec-
	    // tion's have restarted scavenging the
	    // thread and the static lists.

	// Level working data.

        generation * g;
	    // First generation with this level in the
	    // generations vector.

	unsigned number_of_sublevels;
	    // Number of sublevels (generations) on this
	    // level.  If N, then the sublevels of the
	    // generations of this level run from 0
	    // through N-1, with N-1 being the youngest.

	MACC::stub_stack to_be_scavenged;
	MACC::stub_stack root;
	    // To-be-scavenged and root lists for
	    // the level.

        min::uns8 collector_state;
	    // One of COLLECTOR_NOT_RUNNING,
	    // COLLECTOR_START, PRE_INITING_COLLECTIBLE,
	    // etc.

	bool lock;
	    // Set `true' to lock this level struct.
	    // Set by COLLECTOR_START and cleared when
	    // the last phase of a collection of this
	    // level is finished.  Also set by the
	    // {PRE_}REMOVING_ROOT of a collection
	    // at a level lower than this level.

	bool root_scavenge;
	    // True if the stub currently being sca-
	    // venged is from the root list.

	min::uns8 hash_table_id;
	    // Iterates from 0 to identify acc hash
	    // table being scanned during a level 0
	    // collection.

	min::uns32 hash_table_index;
	    // Index of acc hash table element that
	    // is the head of the hash table list
	    // being scanned during a level 0
	    // collection.

	min::uns32 restart_count;
	    // Number of times the current collection
	    // has restarted scavenging the thread and
	    // the static lists during the SCAVENGING_
	    // THREAD phase.

	min::uns32 scavenge_limit;
	    // Maximum number of stubs that may be
	    // scavenged in a SCAVENGING_THREAD phase
	    // collector increment.

	min::acc::generation * first_g, * last_g;
	    // Arguments to the current collector
	    // increment.  Range of generations that
	    // are locked.  Only used by phases that
	    // scan the acc list.

	min::stub * last_stub;
	    // Pointer to a stub within a locked
	    // generation, or the last_before stub of
	    // a locked generation.

	min::stub * last_allocated;
	    // Pointer to a dummy stub allocated when
	    // a collection at this level finishes
	    // scavenging.  This stub marks the end of
	    // stubs collectible by the collection
	    // COLLECTING phase.
	    //
	    // A collection at this or lower level
	    // must clear the unmarked flag of this
	    // stub so it will not be collected.

    };
    extern min::acc::level * levels;

    extern min::uns64 removal_request_flags;
	// If the UNMARKED(L) is set in this value,
	// then stubs with that flag set must be
	// ignored and removed when they occur in
	// the to_be_scavenged or root lists of
	// levels >= L.

    // Ephemeral_levels is the actual number of
    // ephemeral levels, and for each ephemeral level L,
    // ephemeral_sublevels[L] is the number of sublevels
    // of ephemeral level L.
    //
    extern unsigned ephemeral_levels;
    extern unsigned * ephemeral_sublevels;

    // The acc stack is separately allocated from the
    // memory pool, and is not a stub stack (i.e., is
    // NOT as per Stub Stacks).  This is because there
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
    // Bit:	        of i:   Use:
    //
    // M - 1 + i        1 .. E  Level i collectible flag
    // M + E + i        0 .. E  Level i unmarked flag
    // M + 2E + i       1 .. E  Level i non-root flag
    // M + 3E + 1 + i   0 .. E  Level i scavenged flag
    //
    // Note: the MINT::acc_stack_mask flags that are
    // used for level i are the collectible and unmarked
    // flags, as the mask is applied after downshifting
    // the non-root and scavenged flag to line up with
    // the collectible and unmarked flags.
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
    // acc level of the stub, using the fact that the
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
