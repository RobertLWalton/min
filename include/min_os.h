// MIN Operating System Interface
//
// File:	min_os.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Fri Aug 21 05:18:45 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/08/21 19:47:40 $
//   $RCSfile: min_os.h,v $
//   $Revision: 1.9 $

// Table of Contents
//
//	Usage and Setup
//	Parameters
//	Memory Management

// Usage and Setup
// ----- --- -----

# ifndef MIN_OS_H
# define MIN_OS_H

# include <min.h>
# include <iostream>

// Parameters
// ----------

namespace min { namespace os {

    // Return the named program parameter value as a
    // pointer to a character string that is terminated
    // by either whitespace or a NUL.  This character
    // string can then be input to the strtod, strtoll,
    // or strtoul cstdlib functions as desired.
    //
    // Return NULL if there is no parameter of the given
    // name.
    //
    const char * get_parameter ( const char * name );

} }

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

    // Dump error information, including details of last
    // error and memory layout.  Useful when exiting
    // because of error.
    //
    void dump_error_info ( std::ostream & s );

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

    // Mark a segment as being inaccessible memory.  Any
    // contents they previously had will be lost.  The
    // segment must have been allocated with new_pool.
    // The segment will remain allocated, so it cannot
    // be reallocated, but it will not be readable or
    // writable.  Errors will be fatal if detected, but
    // may not be detected.
    //
    void inaccess_pool
        ( min::uns64 pages, void * start );

    // Mark a segment as being accessible memory.  The
    // pages of the segment will become readable and
    // writable.  The segment must have been allocated
    // with new_pool.  The pages will be given arbitrary
    // contents.  Errors will be fatal if detected, but
    // may not be detected.
    //
    void access_pool
        ( min::uns64 pages, void * start );

    // If the following switch is set to true, a trace
    // is printed of each of the above operations.  The
    // default is false.
    //
    extern bool trace_pools;

} }

# endif // MIN_OS_H
