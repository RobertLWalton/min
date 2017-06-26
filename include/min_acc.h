// MIN Allocator/Collector/Compactor Interface
//
// File:	min_acc.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun 26 13:56:06 EDT 2017
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// The acc interfaces described here are interfaces
// for use within and between the Allocator, Collector,
// and Compactor.  These interfaces are private and
// subject to change without notice.

// Table of Contents
//
//	Usage and Setup
//	Stub Allocator Interface
//	Block Allocator Interface
//	Stub Stacks
//	Collector Interface
//	Compactor Interface
//	Statistics

// Usage and Setup
// ----- --- -----

# ifndef MIN_ACC_H
# define MIN_ACC_H

# include <min_acc_parameters.h>
# define MACC min::acc
# define MUP  min::unprotected
# define MINT min::internal

// Stub Allocator Interface
// ---- --------- ---------

namespace min { namespace acc {

    // The stub allocator begins by allocating the
    // stub region, a contiguous sequence of virtual
    // pages that can hold MACC::max_stubs stubs.  It
    // then allocates new stubs as needed from the
    // beginning of this region working up.  Newly
    // added stubs are appended to the acc stub list as
    // free stubs.  See min.h for a description of the
    // acc stub list.
    //
    // The Collector, and NOT the Allocator, frees
    // acc stubs and adds the freed stubs to the list
    // of free stubs.  The Allocator is only called to
    // allocate stubs when the list of free stubs on the
    // acc stub list is exhausted.  The following,
    // defined in min.h, is the interface to the stub
    // allocator:
    //
    //  min::stub * MINT::null_stub
    //    // Address of stub with index 0.  This stub
    //    // may not exist in real memory.
    //    //
    //    // This address is used instead of the value
    //    // NULL when a real stub address is required.
    //    // It may or may not address a real stub, and
    //    // may or may not == stub_begin (defined
    //    // below).
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
    //  min::stub * MINT::head_stub
    //    // The control word of this stub points at the
    //    // first stub on the acc stub list, or is
    //    // MINT::null_stub if the list is empty.
    //    // MINT::head_stub may or may not equal MINT::
    //    // null_stub.
    //  min::stub * MINT::last_allocated_stub
    //    // The control word of this stub points at the
    //    // first free stub on the acc stub list, or is
    //    // MINT::null_stub if there are no such stubs.
    //    // Last_allocated_stub may be the last alloca-
    //    // ted stub on the acc stub list, or may equal
    //    // MINT::head_stub if there are no allocated
    //	  // stubs on the acc stub list.
    //	min::stub * MINT::last_free_stub
    //    // Last stub on the acc free list.  If MINT::
    //    // number_of_free_stubs == 0 this is
    //    // meaningless.
    //  min::uns64 MINT::new_acc_stub_flags
    //    // Acc flags for newly allocated acc stubs.
    //
    // Auxiliary (aux) stubs are not managed by the
    // collector or put on the acc stub list.  They are
    // allocated from and returned to the free list by
    // the MUP::{new,free}_aux_stub functions in min.h.

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

    // Deficiency: If MIN_PTR_SIZE <= 32 stub memory
    // should be allocated as needed and not all at
    // once.  Possibly this should also be done for
    // larger pointer sizes, as reserving virtual memory
    // may consume system resources proportional to the
    // size of the reserved memory.

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

    // Page size used by acc.  Must be a multiple of the
    // hardware page size returned by MOS::pagesize(),
    // and is usually equal to this hardware page size.
    //
    extern min::unsptr page_size;

    // Sizes of various kinds of region and size limits
    // on their contents, in bytes.  See description of
    // regions below, and see min_acc_parameters.h for
    // defaults.

    extern min::uns64 deallocated_body_size;
        // Size of the inaccessible memory region to
	// which min::DEALLOCATED stubs are pointed.

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

    // Address of body of a min::DEALLOCATED stub.
    // Points at the address + 8 bytes of a inaccessible
    // memory region of MACC::deallocated_body_size
    // bytes.  The 8 bytes are for the body control word
    // which is also inaccessible.
    //
    extern void * deallocated_body;

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

	min::uns64 free_size;
	    // Number of bytes of free blocks in a
	    // variable sized block region.

	min::unsptr round_size;
	    // Block sizes in this region are rounded
	    // up to this size (which is generally
	    // either 8 or the page size or the block
	    // size for a fixed size block region).
	    // Note that the size rounded includes
	    // any block control word before a body.
	    //
	    // MUST be a power of 2.

	min::unsptr round_mask;
	    // Equals round_size - 1 so that you can
	    // round S to
	    //
	    //     ( S + round_mask ) & ~ round_mask

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

    inline min::unsptr type_of ( region * r )
    {
        return min::unprotected::type_of_control
	     ( r->block_subcontrol );
    }

    inline min::unsptr size_of ( region * r )
    {
        return min::unprotected::value_of_control
	     ( r->block_subcontrol );
    }

    // Put region r1 on the doubly linked region_
    // previous/next list after region r2.
    //
    inline void insert_after
        ( region * r1, region * r2 )
    {
        r1->region_previous = r2;
	r1->region_next = r2->region_next;
	r1->region_previous->region_next = r1;
	r1->region_next->region_previous = r1;
    }

    // Put region on the end of the doubly linked list
    // whose last member is pointed at by `last', and
    // make the region the new last member of the list.
    //
    inline void insert
        ( region * & last, region * r )
    {
        if ( last != NULL )
	    insert_after ( r, last );
	last = r;
    }

    // Remove region from the region_previous/next
    // list it is on and link it to itself.
    //
    inline void remove ( region * r )
    {
	r->region_previous->region_next =
	    r->region_next;
	r->region_next->region_previous =
	    r->region_previous;
        r->region_previous = r;
	r->region_next = r;
    }

    // Remove region from the doubly linked list
    // whose last member is pointed at by `last'.
    //
    inline void remove
        ( region * & last, region * r )
    {
	if ( r->region_previous == r )
	    last = NULL;
	else
	{
	    if ( r == last )
	       last = r->region_previous;
	    remove ( r );
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

    struct free_variable_size_block
        // Header of a FREE variable size block.
    {
        min::uns64 block_control;
	    // Block control word.  Stub is MINT::
	    // null_stub.  Locator locates region.

	min::uns64 block_subcontrol;
	    // Block subcontrol word.  Type is MACC::
	    // FREE.  Value is block size including
	    // block control word.
    };

    // Return the region of a body given a pointer to
    // the block control word of the body.
    //
    inline region * region_of_body ( min::uns64 * bp )
    {
	int locator = MUP::locator_of_control ( * bp );
	if ( locator > 0 )
	    return & region_table[locator];
	else
	{
	    min::unsptr pp = (min::unsptr) bp;
	    pp &= ~ ( page_size - 1 );
	    min::uns8 * p = (min::uns8 *) pp;
	    p += locator * page_size;
	    return ( MACC::region *) p;
	}
    }

    // Return the stub of a body given a pointer to the
    // block control word of the body.
    //
    inline min::stub * stub_of_body ( min::uns64 * bp )
    {
        return unprotected::stub_of_control ( * bp );
    }

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

// Stub Stacks
// ---- ------

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


    // A stub stack ss can be accessed in two ways.
    //
    // First, a value can be pushed into the stack using
    // ss.push().  It is also possible to batch push
    // operations using ss.begin_push() and ss.end_
    // push().  Note that there is no pop operation, so
    // if this were the only way a stack is used, the
    // stack could only grow and never shrink (a pop
    // could, however, be easily implemented).
    //
    // Second, the stack comes with two pointers, named
    // `input' and `output'.  ss.rewind() resets both
    // pointers to the beginning of the stack.  ss.at_
    // end() is true iff the input pointer is at the end
    // of the stack (just beyond the last element).
    // Otherwise ss.current() is the value pointed at by
    // the input pointer.  It is a programming error to
    // use ss.current() when ss.at_end() is true.
    //
    // One way of using this ignores the output pointer.
    // ss.next() moves the input pointer forward one
    // stack.  It is a programming error to attempt this
    // when ss.at_end() is true.  ss.push() can be used
    // to add elements to the stack at any time, making
    // ss.at_end() false even if it were previously
    // true.  ss.rewind() can be used to reset the input
    // pointer.
    //
    // The other way of using this is to copy elements
    // from the input pointer location to the output
    // pointer location.  The output pointer always
    // equals or trails the input pointer.  ss.next is
    // NOT used.  Instead ss.keep() is used to copy the
    // input element to the output element and increment
    // both pointers, and ss.remove() is used to incre-
    // ment the input pointer only, without any copy or
    // change to the output pointe.  ss.push() can also
    // be used to add elements to the end of the stack
    // during all this.  ss.flush() deletes all elements
    // at or after the output pointer position and does
    // an ss.rewind().  The last element in the flushed
    // stack will be the last element kept by ss.keep().
    //
    // Using ss.remove() causes stack segments between
    // the output and input pointer to be removed from
    // the stack.  That is, a segment is removed and
    // freed if the output pointer is not in that seg-
    // ment when the input pointer moves from that seg-
    // ment to the next segment.
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

	min::uns64 in, out;
	    // `in' counts the number of elements pushed
	    // into the stack, and is incremented by the
	    // push() and end_push() functions.   `out'
	    // counts the elements removed from the
	    // stack, and is incremented by the remove()
	    // function.

	min::uns32 segment_count, max_segment_count;
	    // The number of segments currently in this
	    // stack, and the maximum thereof over the
	    // lifetime of the stack.

	min::uns64 total_segment_count;
	    // Number of stub stack segments ever allo-
	    // cated to this stack.  The number freed
	    // equals this number - segment_count.


	stub_stack ( void ) :
	    last_segment ( NULL ),
	    input_segment ( NULL ),
	    output_segment ( NULL ),
	    input ( NULL ),
	    output ( NULL ),
	    is_at_end ( true ),
	    in ( 0 ),
	    out ( 0 ),
	    segment_count ( 0 ),
	    max_segment_count ( 0 ),
	    total_segment_count ( 0 ) {}

	void rewind ( void );

	bool at_end ( void )
	{
	    return is_at_end;
	}

	min::stub * current ( void )
	{
	    MIN_REQUIRE ( ! is_at_end );
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
	    ++ in;
	    is_at_end = false;
	}

	// ss.begin_push ( next, end ) returns the next
	// and end pointers of the stub stack ss.  min::
	// stub * values may be pushed into the stack
	// by * next ++ = ... until next >= end.  Then
	// ss.end_push ( next ) should be called to
	// store next back into the stub stack.  Other
	// stack functions should not be used between
	// calls to begin_push() and end_push().
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
		in += next - last_segment->next;
		last_segment->next = next;
		is_at_end = false;
	    }
	}

	void remove_jump ( void );
	void remove ( void )
	{
	    MIN_REQUIRE ( ! is_at_end );

	    ++ out;
	    if ( ++ input == input_segment->next )
	        remove_jump();
	}

	void next ( void )
	{
	    MIN_REQUIRE ( ! is_at_end );

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
	    MIN_REQUIRE ( ! is_at_end );

	    min::stub * value = * input;

	    if ( ++ input == input_segment->next )
	        remove_jump();

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
    // The collector can run one collection algorithm
    // execution for each level independently and in
    // parallel.  Such an execution for level L is known
    // as a `level L collection'.  This marks all stubs
    // on levels >= L that are pointed to by stubs in
    // the level L root list or by executing threads or
    // static memory, and recursively marks any stub
    // pointed at by a marked stub.  It then examines
    // all stubs on levels >=L and collects any unmarked
    // stubs.
    //
    // A collection is divided into pre-scavenging
    // phases, a.k.a. initing phases, during which
    // stub flags are initialized, scavenging phases,
    // during which stubs are marked and scavenged,
    // and post-scavenging phases, a.k.a collecting
    // phases, during which unmarked stubs are deleted.
    //
    // To scavenge a stub means to examine the stub data
    // for pointers to other stubs, and mark any of
    // these other stubs that satisify the criteria of
    // the current collection execution (i.e., the
    // stub's level is one being marked).  The stub data
    // consists of the stub, any body pointed at by the
    // stub, and any aux stubs pointed at by stub data
    // (recursively).  We say that a stub s1 points at a
    // stub s2 if the data of s1 contains a pointer to
    // s2.
    //
    // Whenever a stub is first marked it is put on a
    // to-be-scavenged list.  Each level L has a root
    // list that contains all stubs of level < L that
    // might point at a stub of level >= L, and the
    // stubs in the level L root list are also scavenged
    // by a level L collection.
    //
    // All stubs that are not aux stubs are chained
    // together by pointers in their control word to
    // form a list called the acc list.  The allocated
    // stubs appear first in this list, and then the
    // free stubs.  The allocated stubs are in order
    // of age, so the stubs of lower level appear before
    // the stubs of higher level in this list.
    //
    // The `mutator' refers to all code outside the
    // collector algorithm execution.  The mutator may
    // write pointers to stubs into other stubs or
    // their bodies, into threads (i.e., their stacks),
    // or into static memory.  Lists are maintained of
    // the locations where these pointers may be
    // written, and stubs of the appropriate level that
    // are pointed at by elements of these lists are
    // marked.
    //
    // Collector Lists:
    //
    //   For each level L the collector maintains three
    //   lists:
    //
    //	    Level List
    //		List of all stubs in level L.  This is
    //		a segment of the acc stub list as per
    //		min.h.  For L = 0, the xxx_acc_hash
    //		tables are also part of the level list.
    //		This list is scanned after scavenging
    //		to garbage collect unmarked stubs.
    //	
    //	    Root List
    //		List of all stubs in levels < L
    //		that might point at stubs in levels
    //		>= L.  The stubs on this list are
    //		scavenged by a level L scavenging.
    //
    //	    To Be Scavenged List
    //		List of all stubs of level >= L that
    //		remain to be scavenged during the
    //		current level L scavenging.
    //
    //   The collector also maintains lists not related
    //	 to any level:
    //
    //	     Thread List
    //		For each thread, a list of stubs pointed
    //          at by data in the thread's stack.
    //          Maintained by the min::locatable_...
    //          structures defined in min.h.
    //
    //	     Static List
    //		A list of stubs pointed at by static
    //		data (e.g., data allocated at link
    //		time).  Maintained by the min::
    //		locatable_... structures defined in
    //		min.h.
    //
    //		This static list is actually just an
    //		initial segment of the main thread
    //		list.
    //
    // As the last step in a scavenging, the thread and
    // static lists are scanned for pointers to stubs
    // of the correct levels, and when such are found,
    // the stubs pointed at are marked.
    //
    // Stub ACC Flags:
    //
    //   Each acc stub has 4 acc flags for each ephem-
    //   eral level L > 0.  These are:
    //
    //	     scavenged	The stub's datum has been
    //		        scavenged by the current level
    //			L scavenging.  If the stub level
    //			is >= L the stub must have been
    //			marked by the current (or last)
    //			level L scavenging, and if the
    //			stub has level < L it must be on
    //			the level L root list.
    //
    //	     unmarked	The stub level is >= L and the
    //			stub has NOT yet been marked
    //			by the current (or just
    //			completed) level L scavenging.
    //
    //	     non-root	The stub level is < L and the
    //			stub is NOT on the level L root
    //			list.  Or it is on the level L
    //			root list and is currently being
    //			scavenged by a level L collec-
    //			tion (there can be only one such
    //			stub at a time).
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
    //	    is pushed first.  See acc flags and min::
    //	    unprotected::acc_write_update in min.h.
    //
    // MINT::acc_stack Processing:
    //
    //   The MINT::acc_stack is processed separately by
    //   the MACC::process_acc_stack routine.  This is
    //   run in preference to other acc functions when-
    //   ever an acc algorithm is executed.  This pre-
    //   ference keeps the acc stack as small as pos-
    //   sible.
    //
    //   The MINT::acc_stack contains stub pointer pairs
    //   (s1,s2) such that a pointer to s2 has been
    //   stored in the s1 data (in the s1 stub, any body
    //   pointed at the stub, or any aux stub pointed at
    //   by s1 data, recursively).  The MINT::process_
    //   acc_stack routine repeatedly removes a pair
    //   from the end of stack and processes the pair.
    //
    //   If in any pair (s1,s2) either s1 or s2 has a
    //   flag in common with MACC::removal_request_flags
    //   then the pair is ignored.  This permits stubs
    //   designated for removal by a level L collection
    //   that sets the stubs' level L unmarked flag to
    //   be instantly removed from consideration by the
    //   collector merely be setting the level L
    //   unmarked flag of MACC::removal_request_flags.
    //
    //   If the MINT::process_acc_stack routine finds a
    //   level L scavenged stub s1 pointing at a level L
    //   unmarked stub s2 (i.e., the s1 data contains a
    //   pointer to s2), and if the acc_stack_mask level
    //   L unmarked flag is on, the routine turns off
    //   the s2 level L unmarked flag, and if s2 is
    //   scavengable, puts s2 on the level L to-be-sca-
    //   venged stack.  A stub is scavengable if and
    //   only if MINT::scavenger_routine[type_of(s2)] is
    //   not NULL.
    //   
    //   If the MINT::process_acc_stack routine finds a
    //   level L non-root stub s1 pointing at a level L
    //   collectible stub s2, and if the acc_stack_mask
    //   level L collectible flag is on, the routine
    //   turns off the s1 level L non-root flag.  If
    //   this happens, there are two cases to consider.
    //
    //   In the normal case, s1's level L scavenge flag
    //   is off because s1 is not on the level L root
    //   list.  In this case MINT::process_acc_stack
    //   puts s1 on the level L root list.  In addition,
    //   if the level L scavenged flag is set in MACC::
    //   acc_stack_scavenge_mask, which indicates that a
    //   level L collection is currently in a scavenging
    //   phase, then MINT::process_acc_stack turns on
    //   the level L scavenged flag of s1, and if the s2
    //   level L unmarked flag is on, it is turned off
    //   and s2 is put on the level L to-be-scavenged
    //   list if s2 is scavengable.  Note that in this
    //   case s1 contains only one pointer at a stub of
    //   L2 >= L, namely the pointer to s2, so there is
    //   no need to scavenge s1 to find other such
    //   pointers.
    //
    //   The level L scavenger turns on the level L
    //   scavenge flag in MACC::acc_stack_scavenge_mask
    //   when it starts to scan the level L root list,
    //   at a point when the level L scavenged flags of
    //   all stubs on the root list are off.  Then
    //   MINT::process_acc_stack may put new stubs on
    //   the END of the root list, but these will all
    //   have their level L scavenged flags on.   So
    //   the level L scavenger knows that when it gets
    //   to a stub on the root list with its level L
    //   scavenged flag set, the scavenger is done, as
    //   all subsequent stubs on the root list will have
    //   their level L scavenged flag set.
    //
    //   In the other case, s1's level L scavenge flag
    //   is on.  This occurs only when the level L
    //   scavenger has found s1 in the level L root list
    //   and is scavenging it, so in this case MINT::
    //   process_acc_stack merely turns off s1's level L
    //   non-root flag and does nothing else.
    //
    //   The level L scavenger uses this behavior as
    //   follows.  When the scavenger needs to scavenge
    //   a stub s1 that it finds in the level L root
    //   list, it must determine whether s1 should
    //   remain on the root list.  It does this by first
    //   turning s1's level L non-root flag and scavenge
    //   flags both on, then scavenging s1, then looking
    //   to see if s1 should remain on the level L root
    //   list, and lastly either turing s1's level L
    //   non-root flag off if it should remain on the
    //   list, or removing s1 from the list if it should
    //   not remain on the level L root list.
    //
    //   There are two cases where s1 should remain on
    //   the level L root list.  One is the case where
    //   during scavenging a level L3 >= L pointer s3
    //   was found in the the data of s1.  The other
    //   case is if MINT::process_acc_stack running
    //   during the scavenging of s1 found that a
    //   stub pointer s2 of level L2 >= L, i.e. with its
    //   level L collectible flag on, has been stored
    //   in the data of s1.  This last is indicated by
    //   the fact that s1's non-root flag is turned off
    //   by MINT::process_acc_stack during the scaveng-
    //   ing, as described above, so the scavenger
    //   merely looks at this flag at the end of sca-
    //   venging and keeps s1 on the level L root list
    //   if s1's level L non-root flag was turned off
    //   during scavenging.
    //
    //
    // Stub Allocation:
    //
    //   When a new stub is allocated, it is given the
    //   flags in MUP::new_acc_stub_flags.  The new stub
    //   is always put in the highest level.  Normally
    //   the flags set are as follows:
    //
    //	     all collectible flags are set (because a
    //		 new stub is in the highest level)
    //	     all unmarked flags are set according to
    //		 the state of the collector; during 
    //		 pre-scavenging and scavenging phases
    //		 of a level L collection, level L un-
    //		 marked flags are set; during post
    //		 scavenging phases, level L unmarked
    //		 flags are cleared
    //	     all scavenged flags are cleared
    //	     all non-root flags are cleared
    //
    //   During level L pre-scavenging and scavenging
    //   collector phases a new stub will have its level
    //   L unmarked flag set, and will remain unmarked
    //   unless it is marked by level L scavenging.
    //   Many temporary stubs will remain unmarked, and
    //   will be collected promptly after becoming
    //   inaccessible from threads and static memory.
    //
    //   At the end of level L scavenging, all reachable
    //   stubs have their level L unmarked flag clear.
    //   To maintain this invariant, at this time the
    //   level L unmarked flag in MUP::new_acc_stub_
    //   flags is cleared, so any newly allocated stubs
    //   will also have a clear level L unmarked flag.
    //
    //   If a currently running level L scavenging is
    //   having thrashing problems (see below), then
    //   immediately after allocating the stub with the
    //   flags given above it may be marked at level L.
    //   This means its level L unmarked flag will be
    //   cleared and if it is scavengable it will be put
    //   on the level L to-be-scavenged list.  This
    //   keeps pointers to unmarked stubs out of the
    //   thread stacks and makes further thrashing less
    //   likely, at the cost of retaining stubs beyond
    //   when they are accessible.
    //
    //   Hash tables are a special case, as they are
    //   not part of the root lists of any level, so
    //   a hashed stub may be collected.  When a hashed
    //   stub is found by an allocation, and returned
    //   in place of a newly allocated stub, its
    //   level L unmarked flag will be cleared during
    //   level L collection post scavenging phases, so
    //   all reachable stubs will have their level L
    //   unmarked flag clear.  This is done using MINT::
    //   hash_acc_clear_flags (see min.h).
    //
    // Acc List:
    //
    //   Stubs that are not aux stubs are kept in a list
    //   called the acc list.  Allocated stubs are first
    //   and free stubs are last.  The allocated stubs
    //   are in order of their age, oldest first, where
    //   age is measured from time of last allocation.
    //   The list is formed by using the control words
    //   of the stubs to point at succeeding stubs.
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
    //   pointed at by the control word of MINT::head_
    //   stub.  MINT::head_stub is itself NOT part of
    //   the acc list, and has no use other than for
    //   holding a pointer to the first stub on the acc
    //   list (and possibly doubling as MINT::null_
    //   stub).
    //
    //   Newly allocated stubs are put on the end of the
    //   acc list, thus keeping the acc list in order of
    //   stub age.  The last allocated stub in the acc
    //   list is MINT::last_allocated_stub.  If there
    //   are no allocated stubs, this equals MINT::
    //   head_stub (only happens briefly at the start of
    //   program initialization.)
    //
    //   Level 0 stubs in the hash tables for strings,
    //   numbers, and labels are not included in the acc
    //   list.  The parts of the hash tables that point
    //   at level 0 stubs (the xxxx_acc_hash tables) are
    //   conceptually part of the level 0 list, but they
    //   are not physically part of the acc list.
    //   
    // Collector Increments and Phases:
    //
    //   The collector executes in increments called
    //   `collector increments' that are of limited
    //   duration so they may run intermixed with normal
    //   mutator execution without consuming all the CPU
    //   time.  To implement collector increments, the
    //   collector must maintain state information for
    //   each level, and this includes a `collector
    //   phase' value that specifies which phase the
    //   collection at that level is in.
    //
    //   Phases are divided into three groups.  The pre-
    //   scavenging or initing phases set stub flags.
    //   The scavenging phases scavenge and mark stubs.
    //   The post-scavenging or collecting phases delete
    //   unmarked stubs and promote stubs from one level
    //   or sublevel to another.
    //
    // Locking
    //
    //   Each level has a `level struct' data structure
    //   whose root list can be locked, and each genera-
    //   tion (i.e., sublevel of a level) has a `genera-
    //   tion struct' data structure which can be
    //   locked.
    //
    //   The `last_before' member of a generation struct
    //   points at the stub just before the first stub
    //   of the generation on the acc list.  Any code
    //   that can change or use this member must lock
    //   the generation struct.  This means that code
    //   that accesses the portion of the acc list that
    //   belongs to a generation must lock that genera-
    //   tion's generation struct, the subsequent gener-
    //   ation's generation struct, and ALL generation
    //   structs that have the same `last_before' value
    //   as the subsequent generation.
    //
    //   The level L root list must be locked when it is
    //   accessed, as all accesses modify this list's
    //   input pointer.
    //
    //   The other level L data structures are not mod-
    //   ified by collections at levels other than L.
    //   In particular, the level L scavenge list is not
    //   modified or scanned by collections at levels
    //   other than L, though its counters are read by
    //   these collections.
    //
    //   If the function that runs the next collector
    //   can not get a lock to run the increment,
    //   it returns the level of the running collection
    //   that holds this lock.
    //
    //   A collector phase XXX often has associated
    //   phases START_XXX and LOCK_XXX.  START_XXX just
    //   initializes data for XXX and immediately starts
    //   XXX or LOCK_XXX.  LOCK_XXX gets locks required
    //   by XXX and if successful immediately starts
    //   XXX.  If XXX needs more locks it may return to
    //   LOCK_XXX.
    //
    // Phases:
    //
    //   Generally phases are executed in the order
    //   given below, which is the order of the phase
    //   enum code, with exceptions noted.  In the
    //   following phase descriptions, it is assumed
    //   that the that the phase is part of a level L
    //   collection.  Levels[L] is the level struct of
    //   this level.  The level and generation structs
    //   are described below.
    //
    //   The first thing that the collector_increment
    //   function which runs phases does is empty the
    //   acc stack, so phases all run with this stack
    //   empty.
    //
    enum
    {
	COLLECTOR_NOT_RUNNING = 0,
	    // A level L collection is NOT in progress.
	    // A level L collection cannot be started
	    // unless the levels collector_phase is
	    // COLLECTOR_NOT_RUNNING, and then the
	    // collection can be started by merely
	    // setting the collector_phase to COLLECTOR_
	    // START.

	COLLECTOR_START,
	    // The level L unmarked flag is set in MUP::
	    // new_acc_stub_flags so it will be set in
	    // any newly allocated stubs.  At the end of
	    // the initing phases all stubs with levels
	    // >= L should have their level L unmarked
	    // flags set, and all stubs should have
	    // their level L scavenged flag clear.
	    //
	    // The level L to-be-scavenged list should
	    // be empty at this point.  The level L
	    // unmarked flag should be cleared in MINT::
	    // acc_stack_mask, MINT::removal_request_
	    // flags, and MINT::hash_acc_clear_flags.
	    // The level L scavenged flag should be
	    // cleared in MACC::acc_stack_scavenge_mask.
	    //
	    // The level counts are saved in save_count
	    // so at the end of collection the differ-
	    // ences between the level counts and saved_
	    // count will reflect the actions of the
	    // current collection.

	START_INITING_COLLECTIBLE,
	LOCK_INITING_COLLECTIBLE,
	INITING_COLLECTIBLE,
	    // Iterates over all generations of levels
	    // >= L.  For each generation g, locks g and
	    // g + 1 and scans the stubs of g.  Each
	    // scanned stub has its level L unmarked
	    // flag set and its level L scavenged flag
	    // cleared.
	    //
	    // When generation g + 1 is locked, genera-
	    // tion g - 1 is unlocked.  At the end of
	    // the phase all generations are unlocked.

	START_INITING_HASH,
	LOCK_INITING_HASH,
	INITING_HASH,
	    // Done only for L == 0.  The first non-
	    // level 0 generation is locked (if there
	    // is only one level this will be end_g),
	    // as this is the generation that will be
	    // moved into the acc hash tables.  The acc
	    // hash tables are scanned, and each stub
	    // in these tables list has its unmarked
	    // flag set and its scavenged flag cleared.
	    // Then the generation lock is released.

	START_INITING_ROOT,
	LOCK_INITING_ROOT,
	INITING_ROOT,
	    // The level L root list is locked and scan-
	    // ned and each stub on this list has its
	    // scavenged flag cleared.  The lock is left
	    // on for the next phase.

	START_SCAVENGING_ROOT,
	LOCK_SCAVENGING_ROOT,
	SCAVENGING_ROOT,
	    // At the beginning of this phase, the first
	    // scavenging phase, the level L unmarked
	    // flag is set in MINT::acc_stack_mask so
	    // stores into scavenged stubs will be
	    // handled by acc stack processing, and the
	    // level L scavenged flag is set in MACC::
	    // acc_stack_scavenged_mask so acc stack
	    // processing that adds to the level L root
	    // list will set the scavenged flag of the
	    // new root stubs.
	    //
	    // Each stub on the level L root list is
	    // scavenged.  In the process stubs are put
	    // on the to-be-scavenged list, and these
	    // are also scavenged.
	    //
	    // Any stub found in either the level L root
	    // list or the level L to-be-scavenged list
	    // that has a flag set which is also set in
	    // MACC::removal_request_flags is removed
	    // from the respective list and otherwise
	    // ignored.  This permits stubs designated
	    // for removal by OTHER levels to be
	    // instantly removed from these lists by
	    // setting the unmarked flags of the other
	    // levels in MACC::removed_request_flags.
	    //
	    // To scavenge a stub s1, each pointer in s1
	    // or its body to another stub s2 is examin-
	    // ed, and if the level L unmarked flag of
	    // s2 is on, it is cleared and s2 is put on
	    // the level L to-be-scavenged list if it is
	    // scavengable.  s2 is scavengable if and
	    // only if MINT::scavenger_routines
	    // [type_of(s2)] is not NULL.
	    //
	    // Scavenging stubs is done by the MINT::
	    // scavenger_routines defined in min.h.
	    // The MINT::scavenger_controls[L] struct
	    // is used to control scavenging.  Scaveng-
	    // ing a large stub body is done with multi-
	    // ple collector increments, so the mutator
	    // can interrupt the scavenging of a large
	    // stub body.  If the mutator sets the state
	    // of MINT::scavenger_controls[L] to RESTART
	    // during such a scavenging, then scavenging
	    // of the stub is restarted.  See the sca-
	    // venger_routines in min.h for details.
	    //
	    // The level L scavenged flag of s1 is set
	    // just before s1 is scavenged, so that if
	    // the mutator stores a pointer to a level L
	    // unmarked stub s2 into s1 in the middle of
	    // scavenging s1, the acc_stack mechanism
	    // will clear the level L unmarked flag of
	    // s2 and put s2 on the level L to-be-sca-
	    // venged list.
	    //
	    // The level L not-root flag of s1 is set
	    // just before s1 is scavenged if s1 is on
	    // the level L root list.  If any pointer to
	    // a stub s2 that is of level >= L is dis-
	    // covered in s1, then after s1 is scavenged
	    // its non-root flag is cleared and it is
	    // left on the root list.  Also if s1's non-
	    // root flag is cleared while it is being
	    // scavenged, by the action of the mutator
	    // and acc stack processing, then s1 is
	    // left of the root list.  Otherwise s1 is
	    // removed from the root list (it may be
	    // restored to the root list by the action
	    // of the mutator and acc stack processing
	    // at any later time).
	    // 
	    // The goal of this phase is to scavenge all
	    // the level L root list stubs and empty the
	    // level L to-be-scavenged list.  Emptying
	    // the to-be-scavenged list is given prior-
	    // ity over scavenging the next root stub,
	    // in order to keep the to-be-scavenged list
	    // short.
	    //
	    // Stubs may be added to the end of the
	    // level L root list by the mutator via the
	    // acc stack during this phase, but such
	    // stubs have their level L scavenged flag
	    // set when they are added to the root list
	    // (see acc stack processing above), and
	    // need not be scavenged by this phase.
	    // When this phase encounters an already
	    // scavenged stub on the root list, it is
	    // skipped over and not rescavenged.
	    //
	    // The level L root list is locked during
	    // this phase, and the lock is released
	    // at the end of this phase.

	START_SCAVENGING_THREAD,
	SCAVENGING_THREAD,
	    // The thread list of each thread and the
	    // static list (see above) are treated as if
	    // they were the body of a single root stub
	    // and scavenged by the MINT::thread_
	    // scavenger_routine.  The goal of this
	    // phase is to scavenge the thread/static
	    // lists and empty the to-be-scavenged list
	    // (which may be added to by scavenging the
	    // thread/static lists).
	    //
	    // However, if this scavenging is interrup-
	    // ted by the mutator (i.e., if a phase col-
	    // lector increment terminates with the sca-
	    // venging unfinished), the thread/static
	    // scavenging must be restarted at its
	    // beginning.  This can cause thrashing, but
	    // as this phase runs it decreases the num-
	    // ber of pointers in the thread/static
	    // lists that point at unmarked stubs, and
	    // thereby decreases the likelihood that
	    // thread/static scavenging will be inter-
	    // rupted by the mutator.
	    //
	    // To prevent indefinite thrashing, the dur-
	    // ation of the collector increment is
	    // increased as a function of the number of
	    // times scavenging the thread/static lists
	    // has been restarted.  This means that the
	    // effective values of MACC::scan_limit and
	    // MACC::scavenge_limit are increased.
	    //
	    // As an alternative, the allocator may be
	    // set to both clear the level L unmarked
	    // flag of any newly allocated stub and put
	    // the stub on the to-be-scavenged list.
	    // This can greatly reduce the number of
	    // level L unmarked stubs encountered when
	    // scavenging the thread/static lists.  This
	    // is not currently implemented.
	    //
	    // At the end of this phase, any level L
	    // stub with its unmarked flag set is not
	    // reachable by the mutator.  Exactly at the
	    // end of this phase the level L unmarked
	    // flag is cleared in MINT::new_acc_stub_
	    // flags so any stubs allocated afterwards
	    // will not be collected by the level L
	    // collection, the level L unmarked flag is
	    // set in MACC::removal_request_flags so any
	    // unmarked level L stubs will be ignored if
	    // they are found in root or to-be-scavenged
	    // lists, or the acc stack, and the level L
	    // unmarked flag is set in MINT::hash_acc_
	    // clear_flags so that stubs returned from
	    // the hash tables after this point will not
	    // be collected by the level L collection.
	    //
	    // Also the level L unmarked flag is cleared
	    // from MINT::acc_stack_mask so the acc
	    // stack will no longer involve itself with
	    // level L marking, and the level L scaveng-
	    // ed flag is cleared from MACC::acc_stack_
	    // scavenge_mask for the same reason.

	START_REMOVING_TO_BE_SCAVENGED,
	REMOVING_TO_BE_SCAVENGED,
	    // This phase simply waits until ALL levels
	    // have processed any portion of their
	    // to-be-scavenged lists that existed when
	    // this phase started.  In conjunction with
	    // MACC::removal_request_flags this removes
	    // all stubs with level L unmarked flag set
	    // from all to-be-scavenged lists.

	START_REMOVING_ROOT,
	LOCK_REMOVING_ROOT,
	REMOVING_ROOT,
	    // For each level L1 > L, the L1 root list
	    // is locked, the level L1 root list is then
	    // scanned, and all scanned stubs with any
	    // MACC::removal_request_flags flag set are
	    // removed.

	START_COLLECTING_HASH,
	LOCK_COLLECTING_HASH,
	COLLECTING_HASH,
	    // Scan through the XXX_acc_hash tables and
	    // free all stubs with level L == 0 unmarked
	    // flag set.
	    //
	    // Only runs if L == 0.  Locks the first
	    // generation of level 1 when running, as it
	    // is level 1 promoting that adds to the
	    // hash table.

	START_COLLECTING,
	LOCK_COLLECTING,
	COLLECTING,
	    // Iterates over all generations of levels
	    // >= L.  For each generation g, gets a lock
	    // on g and then releases any lock on the
	    // previous generation.  Then locks genera-
	    // tion g+1 and also locks all subsequent
	    // generations whose last_before == (g+1)->
	    // last_before.
	    //
	    // Then scans the stubs of generation g.
	    // All scanned stubs with level L unmarked
	    // flag set are freed.
	    //
	    // Note that for ephemeral levels, any freed
	    // number, string, or label stub is removed
	    // from its XXX_aux_hash table.  Being in a
	    // XXX_aux/acc_hash table does not prevent
	    // collection of any stub.

	START_LEVEL_PROMOTING,
	LOCK_LEVEL_PROMOTING,
	LEVEL_PROMOTING,
	    // Only executed if L > 0.
	    //
	    // If g is the first generation of level L,
	    // obtains a lock on g and g+1, and for each
	    // stub in the generation clears its level L
	    // collectible flag, and if it is scaveng-
	    // able, puts it on the L root list while
	    // clearing its level L non-root flag.  The
	    // next level L collection will scavenge
	    // these and remove from the root list any
	    // that have no pointer to stubs of level
	    // >= L.
	    //
	    // If L == 1 and the stub is in an xxx_aux_
	    // hash table, the stub is moved to the
	    // corresponding XXX_acc_hash table.  All
	    // other stubs are moved one at a time to
	    // the last generation of level L-1.  This
	    // is done by adjusting g[-1].count and
	    // g->last_before.  For every stub g->count
	    // is decremented, so that by the end of
	    // this phase it equals 0.
	    //
	    // Ends releasing the lock on level g (but
	    // not the lock on g+1).

	START_GENERATION_PROMOTING,
	LOCK_GENERATION_PROMOTING,
	GENERATION_PROMOTING,
	    // Only executed if L > 0.
	    //
	    // Iterates over all generations but the
	    // first of level L.  For each generation g,
	    // gets a lock on g and g+1.  Then, sets g->
	    // last_before = (g+1)->last_before, adds
	    // g->count to (g-1)->count, and zeroes g->
	    // count, effectively promoting all the
	    // stubs in generation g to generation g-1.

	COLLECTOR_STOP
	    // Finishes up trace printouts, if any, and
	    // resets the level phase to COLLECTOR_NOT_
	    // RUNNING.
    };

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
	// collector is in its INITING_COLLECTIBLE,
	// INITING_HASH, or INITING_ROOT phases.
	//
	// Also maximum number of min::gen values to be
	// scanned during a collector increment of the
	// SCAVENGE_ROOT or SCAVENGE_THREAD phases.
	//
	// Also maximum number of stubs to be scanned
	// during a collector increment of the REMOVING_
	// ROOT or LEVEL_PROMOTING phases.

    extern min::uns64 scavenge_limit;
        // Maximum number of stubs to be scavenged
	// during a colector increment of the SCAVENGE_
	// ROOT or SCAVENGE_THREAD phases.

    extern min::uns64 collection_limit;
        // Maximum number of stubs to be collected
	// during a collector increment of the COLLEC-
	// TING or COLLECTING_HASH phases.

    extern min::uns32 collector_period;
        // Length in milliseconds of the collector
	// time period.  There is an interrupt at
	// the end of each such period.  0 if there
	// is no collector time period.

    extern min::uns32 collector_period_increments;
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
	    // Equals MINT::head_stub if (L,S)=(0,0).
	    //
	    // There is a fake generation end_g at the
	    // end of the generations vector, after the
	    // last real generation.  For this
	    //
	    //	  end_g->last_before =
	    //	      MINT::last_allocated_stub;
	    //
	    // is executed by any phases that might use
	    // end_g->last_before, so MINT::last_allo-
	    // cated_stub becomes the last stub of the
	    // real generations.

	min::uns64 count;
	    // Number of stubs currently in this
	    // generation.  For the last generation
	    // a correction equal to
	    //		MUP::acc_stubs_allocated
	    //	      - MACC::saved_acc_stubs_count
	    // must be added to this.  Phases
	    // that might use this `count' for the
	    // last generation execute
	    //
	    //	   end_g[-1].count +=
	    //		  MUP::acc_stubs_allocated
	    //		- MACC::saved_acc_stubs_count;
	    //	   MACC::saved_acc_stubs_count =
	    //		MUP::acc_stubs_allocated;
	    //
	    // This, in conjunction with end_g->last_
	    // before (see last_before above), maintains
	    // the specification that MUP::last_allo-
	    // cated_stub is the last stub of the last
	    // real generation.
	    //
	    // Also phases that need to set
	    //
	    //	    end_g->count = 0

	int lock;
	    // -1 if unlocked, or level number of
	    // locking level if locked.
	    //
	    // Set to lock the last_before value of this
	    // generation.  If the stubs of generation g
	    // are to be scanned, added to, or removed,
	    // then you must get this lock for both g
	    // and g+1.  If you merely want to change
	    // last_before for generation g, only the
	    // lock for g is required.

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
	// by some collector phases.  See last_before
	// above.

    extern min::uns64 saved_acc_stubs_count;
	// end_g[-1].count must be corrected by adding
	//	  MUP::acc_stubs_allocated
	//      - MACC::saved_acc_stubs_count
	// See the generation::count member above.

    // Each acc level maintains the following counters:
    //
    struct counters
    {
	min::uns64 collectible_inited;
	    // Number of stubs whose flags are set in
	    // INITING_COLLECTIBLE phases.

	min::uns64 root_inited;
	    // Number of stubs whose flags are set in
	    // INITING_ROOT phases.

	min::uns64 acc_hash_inited;
	    // Number of acc hash stubs whose flags are
	    // set in level 0 INITING_HASH phases.

	min::uns64 scanned;
	    // Number of min::gen or min::stub * values
	    // scanned by scavenger phases.  The values
	    // need not contain actual stub pointers
	    // (they may be non-stub min::gen values or
	    // NULL min::stub * values).

	min::uns64 stub_scanned;
	    // Number of stub pointer containing min::
	    // gen or min::stub * values scanned by the
	    // scavenger phases.  I.e., the part of
	    // the scanned counter that actually contain
	    // stub pointers.

	min::uns64 scavenged;
	    // Number of stubs scavenged by the scaven-
	    // ger phases.

	min::uns64 thrash;
	    // Number of times a SCAVENGING_THREAD phase
	    // has been restarted in order to rescavenge
	    // the thread and the static lists, because
	    // scavenging these lists took too long.

	min::uns64 root_kept;
	    // Number of root stubs kept during
	    // REMOVING_ROOT phases.

	min::uns64 root_removed;
	    // Number of root stubs removed during
	    // REMOVING_ROOT phases.

	min::uns64 acc_hash_collected;
	    // Number of acc hash table stubs collected
	    // by level 0 COLLECTING_HASH phases.

	min::uns64 acc_hash_kept;
	    // Number of acc hash table stubs kept by
	    // level 0 COLLECTING_HASH phases.

	min::uns64 aux_hash_collected;
	    // Number of aux hash table stubs collected
	    // by level > 0 COLLECTING_HASH phases.

	min::uns64 aux_hash_kept;
	    // Number of aux hash table stubs kept by
	    // level > 0 COLLECTING_HASH phases.

	min::uns64 collected;
	    // Number of non-hash stubs collected by
	    // COLLECTING phases.

	min::uns64 kept;
	    // Number of non-hash stubs kept by
	    // COLLECTING phases.

	min::uns64 hash_moved;
	    // Number of stubs moved from aux to acc
	    // hash tables by LEVEL_PROMOTING phases.

	min::uns64 promoted;
	    // Number of stubs promoted from one level
	    // to the next lower level by PROMOTING
	    // phases.
    };

    // Each acc level is described by a level struct.
    //
    struct level
    {
	// Statistics:

	// Collector statistics.  These accumulate
	// across all collections of this level.
	//
	counters count;

	// Save of collection statistics at the begin-
	// ning of the last collection.  Can be used to
	// compute the change in counters during the
	// collection.
	//
	counters saved_count;

	// State:

        generation * g;
	    // First generation with this level in the
	    // generations vector.

	unsigned number_of_sublevels;
	    // Number of sublevels (generations) on this
	    // level.  If N, then the sublevels of the
	    // generations of this level run from 0
	    // through N-1, with N-1 being the youngest.

        min::uns8 collector_phase;
	    // One of COLLECTOR_NOT_RUNNING,
	    // COLLECTOR_START, PRE_INITING_COLLECTIBLE,
	    // etc.

	int root_lock;
	    // -1 if unlocked, or level number of
	    // locking level if locked.
	    //
	    // Set to lock the root list of this level
	    // struct.  Set by the INITING_ROOT,
	    // SCAVENGING_ROOT, and REMOVING_ROOT
	    // phases.

	// Stub Lists:

	MACC::stub_stack to_be_scavenged;
	MACC::stub_stack root;
	    // To-be-scavenged and root lists for
	    // the level.

	// Substate (within Phase):

	min::uns8 next_level;
	    // Next level to be processed by REMOVING_
	    // ROOT phase.

	min::uns64 to_be_scavenged_wait
	              [1+MIN_MAX_EPHEMERAL_LEVELS];
	    // Set to the to_be_scavenged.in values of
	    // each level when the REMOVING_TO_BE_
	    // SCAVENGED phase starts.  This phase then
	    // simply waits for all the to_be_scavenged
	    // .out values to become >= these values.

	bool root_scavenge;
	    // True if the stub currently being sca-
	    // venged is from the root list and false
	    // if it is from the to-be-scavenged list.
	    // Used by the SCAVENGING_ROOT phase in
	    // cleaning up after a possibly interrupted
	    // scavenge of a stub.

	min::uns8 hash_table_id;
	    // Specifies the acc hash table CURRENTLY
	    // being scanned by INITING_HASH and
	    // COLLECTING_HASH.

	min::uns32 hash_table_index;
	    // Specifies the index of the NEXT acc hash
	    // table entry to be scanned by INITING_HASH
	    // or COLLECTING_HASH.

	min::stub * hash_stub;
	    // Specifies the NEXT stub to be scanned by
	    // INITING_HASH or COLLECTING_HASH, or
	    // equals MINT::null_stub to indicate the
	    // end of the current hash table entry
	    // list.

	min::uns32 restart_count;
	    // Number of times the current collection
	    // has restarted scavenging the thread and
	    // the static lists during the SCAVENGING_
	    // THREAD phase.  Added into count.thrash.

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
	    // Argument to the current collector incre-
	    // ment.  Pointer to a stub within a locked
	    // generation, or the last_before stub of
	    // a locked generation.
    };
    extern min::acc::level * levels;

    extern min::uns64 removal_request_flags;
	// If the UNMARKED(L) flag is set in this value,
	// then stubs with that flag set must be ignored
	// and removed when they occur in the to_be_
	// scavenged or root lists or in the acc stack.

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

    // Maximum size in bytes of the acc stack.  Must be
    // a multiple of the page size.
    //
    extern min::unsptr acc_stack_max_size;

    // Number of pairs allowed in the acc stack before
    // the stack triggers an interrupt.  acc_stack_limit
    // is set to acc_stack_begin + 2 * acc_stack_trigger
    // unless an interrupt is sceduled.
    //
    extern min::unsptr acc_stack_trigger;

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

    // If the SCAVENGED ( L ) flag is on in the acc_
    // stack_scavenge_mask, any stub put on the level
    // L root list by process_acc_stack will have its
    // scavenged flag set and the stub it points at,
    // if its unmarked flag is set, will have its
    // unmarked flag cleared and will be put on the
    // level L to-be-scavenged list if it is scaveng-
    // able.  Otherwise stubs put on the root list have
    // their scavenged flag clear.
    //
    extern min::uns64 acc_stack_scavenge_mask;

    // Process stub pointer pairs from the end of the
    // acc stack until the acc stack pointer is less
    // than or equal to acc_lower.  Return the number
    // of pointer pairs processed.
    //
    min::unsptr process_acc_stack
        ( min::stub ** acc_lower =
	      min::acc::acc_stack_begin );

    // Return the number of entries in the acc stack.
    // 0 if stack is empty.
    //
    inline min::unsptr acc_stack_count ( void )
    {
        return (   min::internal::acc_stack
	         - min::acc::acc_stack_begin )
	     / 2;
    }

    // Perform one increment of the current collector
    // execution at the given level.  To start a collec-
    // tor execution, wait until the level collector_
    // phase equals COLLECTOR_NOT_RUNNING, and then
    // set the level collector_phase to COLLECTOR_START.
    // The collector execution is finished when this
    // function returns with the level collector_phase
    // set to COLLECTOR_NOT_RUNNING.
    //
    // Returns its argument, `level', if the increment
    // ran, and otherwise if the increment was locked
    // out by a collection at a different level, returns
    // the level that locked the increment out (and
    // which must be run until the lock is cleared).
    //
    unsigned collector_increment ( unsigned level );

    // Perform or complete a collection at a given
    // level.  If no collection is in progress at the
    // level, set the level phase to COLLECTOR_START
    // to start the collection.  Then call collector_
    // increment until the collection is done.  If
    // collection is blocked by collection at another
    // level L2, call collector_increment for L2 just
    // as much as necessary to unblock the original
    // collection.
    //
    void collect ( unsigned level );
} }


// Compactor Interface
// --------- ---------


// Statistics
// ----------

namespace min { namespace acc {

    // Print statistics.  The numbers of used and free
    // stubs and fixed size blocks are printed.
    //
    void print_acc_statistics ( std::ostream & s );

    // Print generation counts.  The next column in the
    // output stream is given (0 is the first column),
    // and the indent and line width to use is given.
    // endl is NOT issued at the end.
    //
    struct print_generations
    {
        unsigned column, indent, width;
	print_generations
	    ( unsigned column = 0,
	      unsigned indent = 4,
	      unsigned width = 72 )
	    : column ( column ),
	      indent ( indent ),
	      width ( width ) {}
    };
} }

std::ostream & operator <<
    ( std::ostream & s,
      const min::acc::print_generations & pg );


# endif // MIN_ACC_H
