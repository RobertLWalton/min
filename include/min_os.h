// MIN Operating System Interface
//
// File:	min_os.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue May 19 10:09:19 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/05/25 20:34:25 $
//   $RCSfile: min_os.h,v $
//   $Revision: 1.1 $

// Table of Contents
//
//	Usage and Setup
//	Memory Management

// Usage and Setup
// ----- --- -----

# ifndef MIN_OS_H
# define MIN_OS_H

# include <min.h>

// Memory Management
// ------ ----------

namespace min { namespace os {

    // Return the current page size.
    //
    min::uns64 pagesize ( void );

    // In the following a segment is a contiguous piece
    // of memory consisting of an integral number of
    // pages starting on a page boundary.

    // Allocate a segment of virtual memory with the
    // given number of pages and return its address.
    // If there is an error return NULL.
    //
    void * new_pool ( min::uns64 pages );

    // Ditto but allocate the segment at a given
    // address.  This will fail if any part of the
    // new segment is already in use.  The new
    // address must be a multiple of the page size.
    //
    void * new_pool ( min::uns64 pages, void * start );

    // Free the segment with given number of pages and
    // start address.  The segment must have been
    // allocated with new_pool, and may not be used
    // again (the same virtual memory pages may be
    // reallocated by another call to new_pool).
    //
    void free_pool ( min::uns64 pages, void * start );

    // Move a segment of memory to another location by
    // manipulating memory maps instead of by copying.
    // The source and target segments may overlap,
    // and the source segment will likely lose its
    // contents, but will remain allocated.  Both source
    // and target segments must contain pages allocated
    // by new_pool.
    //
    void move_pool
        ( min::uns64 pages,
	  void * new_start, void * old_start );

} }

# endif // MIN_OS_H
