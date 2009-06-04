// MIN OS Interface Code
//
// File:	min_os.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Thu Jun  4 07:29:12 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/06/04 20:36:21 $
//   $RCSfile: min_os.cc,v $
//   $Revision: 1.2 $

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
# include <fstream>
# include <sstream>
# include <cstring>
# include <cassert>
using std::cerr;
using std::endl;
using std::ostream;
using std::ifstream;
using std::istringstream;
using std::cout;
using std::hex;
using std::dec;

// Annouce fatal OS error and return error output
// stream.
//
ostream & fatal_error ( void )
{
    return cerr << "FATAL OPERATING SYSTEM ERROR:"
                << endl << "    ";
}

// Announce fatal OS error involving errno code and exit
// program with errno code as status.
//
static void fatal_error ( int errno_code )
{
    fatal_error() << strerror ( errno_code ) << endl;
    exit ( errno_code );
}

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

// new_poolXX error messages:
//
static const int pool_limit = 5;
static const char * pool_message[pool_limit] = {
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

const char * pool_error ( void * address )
{
    MINT::pointer_uns mask = ::pagesize() - 1;
    unsigned n = (MINT::pointer_uns) address & mask;
    if ( n == 0 ) return NULL;
    assert ( n < pool_limit );
    return pool_message[n];
}

// Read /proc/<this-process-id>/maps and save the info
// in used_pools.
//
static struct used_pool
    // List of memory pools in use.
{
    void * start, * end;
} * used_pools;
static unsigned used_pools_count;
    // Current number of used_pools.
static unsigned used_pools_size;
    // Current limit on used_pools vector index.

// Push entry into used_pools.
//
static void used_pools_push ( void * start, void * end )
{
    if ( used_pools_count >= used_pools_size )
    {
        used_pool * new_used_pools =
	    new used_pool[used_pools_size + 100];
	if ( new_used_pools == NULL )
	    fatal_error ( ENOMEM );
	memcpy ( new_used_pools, used_pools,
		   used_pools_count
		 * sizeof ( used_pool ) );
	delete [] used_pools;
	used_pools = new_used_pools;
    }
    used_pools[used_pools_count].start = start;
    used_pools[used_pools_count].end   = end;
    ++ used_pools_count;
}

// Read /proc/<process-id>/maps into used_pools.
//
static void read_used_pools ( void )
{
    char name[100];
    sprintf ( name, "/proc/%d/maps", (int) getpid() );
    ifstream maps ( name );
    if ( ! maps )
    {
        fatal_error() << "cannot read " << name << endl;
	exit ( 1 );
    }
    used_pools_count = 0;
    char line[1000];
    while ( maps.getline ( line, 1000 ) )
    {
        line[999] = 0;
	if ( strlen ( line ) >= 999 )
	{
	    fatal_error() << "bad " << name << " line:"
	                  << endl << "    " << line
			  << endl;
	    exit ( 2 );
	}
	istringstream str ( line );
	min::uns64 start, end;
	char c1, c2;
	str >> hex >> start >> c1 >> end >> c2;
	if ( ! str || c1 != '-' || ! isspace ( c2 )
	           || start > end
#		   if MIN_POINTER_BITS < 64
		      ||
			 end
		      >= (1ull << MIN_POINTER_BITS)
#		   endif
	  )
	{
	    fatal_error() << "bad " << name << " line:"
	                  << endl << "    " << line
			  << endl;
	    exit ( 2 );
	}
	used_pools_push
	    ( (void *)(MINT::pointer_uns) start,
	      (void *)(MINT::pointer_uns) end );
    }
    maps.close();
}

// Dump used_pools for debugging.
//
static void dump_used_pools ( void )
{
    for ( unsigned i = 0; i < used_pools_count; ++ i )
        cout << hex
	     << (MINT::pointer_uns) used_pools[i].start
	     << "-"
	     << (MINT::pointer_uns) used_pools[i].end
	     << dec << endl;
}

// Check if segment overlaps NO used pool.
//
static bool no_overlap ( void * start, min::uns64 size )
{
    void * end = (void *)
        ( (char *) start + (MINT::pointer_uns) size );
    assert ( end >= start );
    for ( unsigned i = 0; i < used_pools_count; ++ i )
    {
        if ( end <= used_pools[i].start ) continue;
        if ( used_pools[i].end <= start ) continue;
	return false;
    }
    return true;
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

