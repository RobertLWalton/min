// MIN OS Interface Code
//
// File:	min_os.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Jun  3 13:14:18 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/06/03 20:31:22 $
//   $RCSfile: min_os.cc,v $
//   $Revision: 1.1 $

// Table of Contents:
//
//	Setup
//	Memory Management

// Setup
// -----

# include <min_os.h>
extern "C" {
#   include <unistd.h>
#   include <errno.h>
#   include <sys/mman.h>
}
# define MUP min::unprotected
# define MINT min::internal
# define MOS min::os

# include <iostream>
# include <iomanip>
using std::cerr;
using std::endl;

// For debugging
using std::cout;
using std::hex;
using std::dec;

// Memory Management
// ------ ----------

static MINT::pointer_uns saved_pagesize;
inline MINT::pointer_uns pagesize ( void )
{
    if ( saved_pagesize == 0 )
        saved_pagesize = sysconf ( _SC_PAGESIZE );
    return saved_pagesize;
}
min::uns64 MOS::pagesize ( void )
{
    return ::pagesize();
}

static const char * error_messages[] = {
/*0*/
    "",
/*1*/
    "new_pool: address returned by operating"
             " system is not on a page boundary",
/*2*/
    "new_pool: requested pool size"
             " not a multiple of page size"
/*3*/
    "new_pool: requested pool size is too large"
/*4*/
    "new_pool: not enough memory or page map space"
             " is available"

};
inline void * error ( int n ) { return (void *) n; }

static void fatal_error ( int n )
{
    cerr << "FATAL OPERATING SYSTEM ERROR:" << endl
         << "    " << strerror ( n ) << endl;
    exit ( n );
}

void * MOS::new_pool ( min::uns64 size )
{
    MINT::pointer_uns mask = ::pagesize() - 1;
    if ( size & mask != 0 )
        return error(2);
    void * result = mmap ( NULL, (size_t) size,
    			   PROT_READ | PROT_WRITE,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1, 0 );

    if ( result == MAP_FAILED )
    {
        switch ( errno )
	{
	case EINVAL:	return error(3);
	case ENOMEM:	return error(4);
	default:
	    fatal_error ( errno );
	}
    }
    else if ( mask & (MINT::pointer_uns) result != 0 )
	return error(1);
    return result;
}
