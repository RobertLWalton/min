// MIN Allocator/Collector/Compactor Interface
//
// File:	min_acc.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Jun  1 06:20:31 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/06/02 07:53:34 $
//   $RCSfile: min_acc.h,v $
//   $Revision: 1.1 $

// The ACC interfaces described here are interfaces
// for use within and between the Allocator, Collector,
// and Compactor.  There interfaces are subject to
// change without notice.

// Table of Contents
//
//	Usage and Setup
//	Allocator Interface
//	Collector Interface
//	Compactor Interface

// Usage and Setup
// ----- --- -----

# ifndef MIN_ACC_H
# define MIN_ACC_H

# include <min.h>
# define MACC min::acc

// Allocator Inferface
// --------- ---------

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
    //	 if L >= 0:	R = pageaddress + pagesize * L
    //
    //   if L < 0:	R = & MACC::region_table[-L]
    //
    // where `pageaddress' is the address of the page
    // containing the body control word containing L.
    //
    // If a block contains an object body, the stub
    // address points at the object's stub.  Otherwise
    // the stub address == MINT::end_stub and the block
    // control word is immediately followed in memory
    // by a block subcontrol word whose type field
    // gives the block type and whose value field
    // specifies the size in bytes of the block.
    // region containing the block 
    //
    enum block_type
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
    //	     for reuse.  See `Free List Management'
    //	     below.
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
    //	     There are fewer than 8 unused padding
    //	     bytes.
    //
    //	     The MACC::region control struct for the
    //	     region is allocated at the beginning of the
    //	     region.  There is no limit to the number of
    //	     variable size block regions.
    //
    //	     Variable size blocks in a region are always
    //	     allocated after the last block allocated in
    //	     the region.  When an object body in a var-
    //	     iable size block is freed, the block is
    //	     marked as free, but cannot be reused until
    //	     the variable size block region is compac-
    //	     ted.  Compaction moves all used blocks in
    //	     the region to the beginning of the region,
    //	     or to another region, thus reclaiming the
    //	     space occupied by variable size free
    //	     blocks.
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
    //	     A multi-page block region has a control
    //	     block in the MACC::region_table vector.
    //	     There is a limit of about 2**15 such
    //	     regions.
    //
    //	     Multi-page blocks in a region are always
    //	     allocated after the last block allocated in
    //	     the region.  When a multi-page block is
    //	     is freed, the block is marked as free, but
    //	     cannot be reused until the multi-page block
    //	     region is compacted.  Compaction moves all
    //	     used blocks in the region to the beginning
    //	     of the region, or to another region, thus
    //	     reclaiming the space occupied by multi-page
    //	     free blocks.  Because all blocks are multi-
    //	     page, moving a block can be done by moving
    //	     page table entries, which is faster than
    //	     moving bytes.

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
	    //	   MACC::region_table[-L]
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

	MACC::region * previous, * next;
	    // Previous and next region on a doubly
	    // linked list of regions.  There is one
	    // such list for every multi-page region
	    // which includes that region and all of its
	    // subregions.

	min::uns64 offset;
	    // For variable size block and multi-page
	    // block regions, the offset of the first
	    // free byte in the region.  New blocks are
	    // always allocated at that offset.

	min::uns32 block_size;
	    // For fixed size block regions, the size
	    // in bytes of fixed size blocks.

	min::uns32 free_count;
	    // For fixed size block regions, the number
	    // of free blocks.

        min::uns64 stunted_block_control;
	    // Block control word for the stunted block
	    // that follows this region control struct
	    // in a fixed block region.
	    //
	    // WARNING: This must be the last thing in
	    // a region control stuct and must be
	    // aligned.
} }


// Collector Interface
// --------- ---------

namespace min { namespace acc {

    // Mutator (non-acc execution engine) action:
    //
    // If a pointer to stub s2 is stored in a datum with
    // stub s1, then for each pair of ACC flags the sca-
    // venged flag of the pair of s1 and the unmarked
    // flag of the pair s2 are logically ANDed, and if
    // the corresponding bit is on in MUP::acc_stack_
    // mask, then push a pointer to s1 and then a
    // pointer to s2 into the MUP::acc_stack.
    //
    // When a new stub is allocated, it is given the
    // flags in MUP::acc_new_stub_flags.
    //
    // One use of this mutator action is as follows.
    //
    // To mark the those members of a set of stubs S
    // that are pointed at by members of a set of stubs
    // R, use a pair of ACC flags, and set the corres-
    // ponding bit in MUP::acc_stack_mask.  Clear the
    // scavenged flag of the flag pair in each stub in
    // R or S, set the unmarked flag in every stub in S
    // that is not in R, and clear the unmarked flag of
    // EVERY other stub.
    //
    // At the end of the algorithm, the scavenged flag
    // will be set for each stub in R, and the unmarked
    // flag will be cleared for each stub of S pointed
    // at by a stub of R, and for each such stub of S,
    // that stub will have been added to the set R.
    // The algorithm may err by clearing the unmarked
    // flag of a few additional stubs in S.
    //
    // The algorithm does the following until done.  For
    // each stub s1 in R whose scavenged flag is off,
    // s1's scavenged flag is set, and then for each
    // pointer from s1's object to a stub s2 in S, if
    // the unmarked flag of s2 is set, it is cleared and
    // s2 is added to the set R.  If a pair of pointers
    // s1, s2 appears in the MUP::acc_stack with the
    // scavenged flag of s1 on and the unmarked flag of
    // s2 on, the unmarked and scavenged flags of s2 are
    // cleared, and s2 is added to the set R.
    //
    // It is only necessary to keep track of the stubs
    // in R whose scavenged flag is off.  These can be
    // listed in a stack of stub pointers, set initially
    // to contain pointers to all the stubs in R.  Then
    // to add a stub to R, merely push a pointer to the
    // stub into this stack.  The above algorithm can
    // then pop a pointer of this stack and proceed as
    // above for stubs in R whose scavenged flag was
    // is not set.
    //
    // If a brand new stub is created, it can be added
    // to S by clearing its scavenged flag and setting
    // its unmarked flag, as being new it cannot yet be
    // pointed at by a stub in R.
    // 
    // Another use of the mutator action is as follows.
    //
    // To mark members of a set of stubs R that point at
    // members of a set of stubs S, use a pair of ACC
    // pointers, and set the corresponding bit in MUP::
    // acc_stack_mask.  Set the unmarked flag of the
    // flag pair in every stub of S, and clear that flag
    // in EVERY other stub.  Clear the scavenged flag of
    // EVERY stub.
    //
    // At the end of the algorithm the unmarked flags
    // will not have been changed, and the scavenged
    // flags will set for every member of R that does
    // NOT point at a member of S.  The algorithm may
    // err by setting the scavenged flag of a few
    // additional stubs in R.
    //
    // The algorithm does the following until done.  For
    // each stub s1 in R, taken in some pre-determined
    // order, the scavenged flag of s1 is turned on, the
    // object is checked to see if it has any pointers
    // to stubs in S, and if so, its scavenged flag is
    // turned back off.  If a pair of pointers s1, s2
    // appears in the MUP::acc_stack with the scavenged
    // flag of s1 on and the unmarked flag of s2 on, the
    // scavenged flag of s1 is cleared.
    //
    // A stack of pointers to stubs in R whose scavenged
    // flags have been cleared at some point in the
    // above algorithm can be easily kept.  After all
    // the stubs in R have been gone through in the
    // pre-determined order, this will include all stubs
    // in R whose object point at stubs in S, whenever
    // the MUP::acc_stack is empty.
    //
    // If a brand new stub is created, it can be added
    // to S by clearing its scavenged flag and setting
    // its unmarked flag.

    // Allocation of bodies is from a stack-like region
    // of memory.  Bodies are separated by body control
    // structures.
    //
    struct body_control {
        uns64 control;
	    // If not free, the type is any value but
	    // min::FREE, and the control contains a
	    // pointer to stub associated with the
	    // following body.  The stub and body itself
	    // must contain enough information to deter-
	    // mine the size of the body (in particular
	    // the location of the body_control follow-
	    // ing the body).
	    //
	    // If free, the type is min::FREE and the
	    // control value is the length of the free
	    // body following the body_control.
	int64 size_difference;
	    // Size of the body following the body_
	    // control - size of the body preceding the
	    // body_control, in bytes.  If there is no
	    // body after the body control, that size
	    // is 0, and if there is body before the
	    // body control, that size is 0.
    };

    // Free_body_control is the body_control before a
    // free body that is used as a stack to allocate
    // new bodies.  A new body is allocated to the
    // beginning of the free body, and free_body_control
    // is moved to the end of the newly allocated body.
    //
    extern body_control * free_body_control;

    // Out of line function to returns a value which
    // may be directly returned by new_body (see
    // below).  This function is called by new_body
    // when the body stack is too short to service an
    // allocation request.  This function may or may not
    // reset free_body_control.
    //
    // Here n must be the argument to new_body rounded
    // up to a multiple of 8.
    //
    body_control * acc_new_body ( unsigned n );

    // Function to return the address of the body_con-
    // trol in front of a newly allocated body with n'
    // bytes, where n' is n rounded up to a multiple of
    // 8.  The control member of the returned body
    // control is set to zero.
    //
    inline body_control * new_body ( unsigned n )
    {
        // We must be careful to convert unsigned and
	// sizeof values to int64 BEFORE negating them.

        n = ( n + 7 ) & ~ 07;
	body_control * head = free_body_control;
	min::internal::pointer_uns size =
	    value_of_control ( head->control );
	if ( size < n + sizeof ( body_control ) )
	    return acc_new_body ( n );

	// The pointers are:
	//
	//	head --------->	body_control
	//			n bytes		---+
	//	free ---------> body_control       |
	//			size - n           | ifb
	//			     - sizeof bc   |
	//			   bytes        ---+
	//	tail ---------> body_control
	//		
	// where
	//
	//	head = initial free_body_control
	//	     = body_control before returned body
	//	free = final free_body_control
	//	     = body_control after returned body
	//	     = body_control before new free body
	//	tail = body_control after original and
	//	       new free bodies
	//	sizeof bc = sizeof ( body_control )
	//	ifb = initial free body

	uns8 * address = (uns8 *) head
	               + sizeof ( body_control );
	body_control * free =
	    (body_control *) ( address + n );
	body_control * tail =
	    (body_control *) ( address + size );
	// Reset size to size of new free body.
	size -= min::internal::pointer_uns 
		    ( n + sizeof ( body_control ) );
	head->size_difference -=
	    int64 ( size + sizeof ( body_control ) );
	free->size_difference =
	    int64 ( size ) - int64 ( n );
	tail->size_difference +=
	    int64 ( n + sizeof ( body_control ) );
	head->control = 0;
	free->control = new_control ( min::FREE, size );
	free_body_control = free;
	return head;
    }

} }

# endif // MIN_ACC_H
