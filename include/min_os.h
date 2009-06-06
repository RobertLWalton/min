// MIN Operating System Interface
//
// File:	min_os.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Jun  6 08:47:01 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/06/06 12:47:22 $
//   $RCSfile: min_os.h,v $
//   $Revision: 1.6 $

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
    // If there is an error, a value is returned that
    // is not really an address but is an error code:
    // see pool_error below.
    //
    void * new_pool ( min::uns64 pages );

    // Ditto but allocate the segment at a given start
    // address.  This is guaranteed to fail if any part
    // of the new segment is already in use.  The start
    // address must be a multiple of the page size.
    //
    void * new_pool_at
        ( min::uns64 pages, void * start );

    // Ditto but allocate the segment so it is complete-
    // ly below the given end address.  The end address
    // must be a multiple of the page size.  This may
    // fail when new_pool would succeed.
    //
    void * new_pool_below
        ( min::uns64 pages, void * end );

    // Return NULL if the argument is really the address
    // of a segment returned by new_pool, and not an
    // error code.  Return a character string giving a
    // short description of the error if the argument is
    // really an error code returned by new_pool.
    //
    const char * pool_error ( void * start );

    // Free the segment with given number of pages and
    // start address.  The segment must have been
    // allocated with new_pool, and may not be used
    // again (the same virtual memory pages may or may
    // not be reallocated by another call to new_pool).
    // Errors will be fatal if detected, but may not be
    // detected.
    //
    void free_pool ( min::uns64 pages, void * start );

    // Move a segment of memory to another location by
    // manipulating memory maps instead of by copying.
    // The source and target segments may overlap,
    // and the source segment will likely lose its
    // contents, but will remain allocated.  Both source
    // and target segments must contain pages allocated
    // by new_pool.  Errors will be fatal if detected,
    // but may not be detected.
    //
    void move_pool
        ( min::uns64 pages,
	  void * new_start, void * old_start );

    // Mark a segment as being non-extant memory.  The
    // pages of the segment will become inaccessible,
    // and any contents they previously had will be
    // lost.  The segment must have been allocated with
    // new_pool.  Errors will be fatal if detected, but
    // may not be detected.
    //
    void non_extant_pool
        ( min::uns64 pages, void * start );

    // Mark a segment as being extant memory.  The pages
    // of the segment will become readable and writable.
    // The segment must have been allocated with new_
    // pool.  The pages will be given arbitrary con-
    // tents.  Errors will be fatal if detected, but may
    // not be detected.
    //
    void extant_pool
        ( min::uns64 pages, void * start );

    // If the following switch is set to true, a trace
    // is printed of each of the above operations.  The
    // default is false.
    //
    extern bool trace_pools;

} }

# endif // MIN_OS_H
