// MIN OS Interface Code
//
// File:	min_os.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Jun  6 02:09:43 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/06/06 06:10:13 $
//   $RCSfile: min_os.cc,v $
//   $Revision: 1.6 $

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

// Annouce fatal OS error for function named fname and
// return error output stream.
//
ostream & fatal_error ( const char * fname )
{
    return cerr << fname
                << ": FATAL OPERATING SYSTEM ERROR:"
                << endl << "    ";
}

// Announce fatal OS error involving errno code and exit
// program with errno code as status.
//
static void fatal_error
	( const char * fname, int errno_code )
{
    fatal_error ( fname ) << strerror ( errno_code )
    			  << endl;
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
static const int pool_limit = 7;
static const char * pool_message[pool_limit] = {
/*0*/
    "",
/*1*/
    "new_pool: requested memory pool overlaps"
             " existing memory pool"
/*2*/
    "new_pool: requested pool size is too large"
/*3*/
    "new_pool: not enough memory or page map space"
             " is available"
};
inline void * error ( int n ) { return (void *) n; }

const char * MOS::pool_error ( void * address )
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
static void used_pools_push
	( void * start, void * end,
	  const char * fname )
{
    if ( used_pools_count >= used_pools_size )
    {
        used_pool * new_used_pools =
	    new used_pool[used_pools_size + 100];
	if ( new_used_pools == NULL )
	    fatal_error ( fname, ENOMEM );
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

// Check if segment overlaps an existing used pool.
//
static bool overlap ( void * start, void * end )
{
    assert ( end >= start );
    for ( unsigned i = 0; i < used_pools_count; ++ i )
    {
        if ( end <= used_pools[i].start ) continue;
        if ( used_pools[i].end <= start ) continue;
	return true;
    }
    return false;
}

// Read /proc/<process-id>/maps into used_pools.
//
static void read_used_pools
	( const char * fname = "read_used_pools",
	  bool print = false )
{
    char name[100];
    sprintf ( name, "/proc/%d/maps", (int) getpid() );
    ifstream maps ( name );
    if ( ! maps )
    {
        fatal_error ( fname )
	    << "cannot read " << name << endl;
	exit ( 1 );
    }
    used_pools_count = 0;
    char line[1000];
    while ( maps.getline ( line, 1000 ) )
    {
        line[999] = 0;
	if ( strlen ( line ) >= 999 )
	{
	    fatal_error ( fname )
		<< "bad " << name << " line:" << endl
		<< "    " << line << endl;
	    exit ( 2 );
	}
	istringstream str ( line );
	min::uns64 start, end;
	char c1, c2;
	str >> hex >> start >> c1 >> end;
	c2 = str.peek();
	if ( ! str || c1 != '-' || ! isspace ( c2 ) )
	{
	    fatal_error ( fname )
		<< "bad " << name << " line format:"
		<< endl << "    " << line << endl;
	    exit ( 2 );
	}
	if ( start > end
	     ||
	     end == 0
#	   if MIN_POINTER_BITS < 64
	     ||    end
	        >= (1ull << MIN_POINTER_BITS)
#	   endif
	  )
	{
	    fatal_error ( fname )
		<< "bad " << name << " line values:"
		<< endl << "    " << line << endl;
	    exit ( 2 );
	}
	if ( overlap ( (void *)
	               (MINT::pointer_uns) start,
		       (void *)
	               (MINT::pointer_uns) end ) )
	{
	    fatal_error ( fname )
	        << "bad " << name
		<< " line: overlaps previous:"
		<< endl << "    " << line << endl;
	    exit ( 2 );
	}
	used_pools_push
	    ( (void *)(MINT::pointer_uns) start,
	      (void *)(MINT::pointer_uns) end,
	      fname );
	if ( print ) cout << line << endl;
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

// Compare current pools to what is in used_pools.
//
// Specifically, save used_pools, call read_used_pools,
// delete all pools in the new used_pools and the
// saved used_pools that are common to both, put
// the remaining new used pools at the beginning of
// used_pools and the remaining saved old used pools
// after them, and return the number of remaining new
// used pools in new_count and the number of remaining
// old used pools in old_count.
//
static void compare_pools
	( unsigned & new_count, unsigned & old_count,
	  bool print = false )
{
    unsigned count = used_pools_count;
    used_pool * old_pools = new used_pool[count];
    memcpy ( old_pools, used_pools,
             count * sizeof ( used_pool ) );

    read_used_pools ( "compare_pools", print );

    // Iterate over new used pools.
    //
    new_count = 0;
    for ( unsigned i = 0; i < used_pools_count; ++ i )
    {
        void * start = used_pools[i].start;
        void * end   = used_pools[i].end;

	// Search old_pools for used_pools[i].
	//
        unsigned j;
	for ( j = 0; j < count; ++ j )
	{
	    if ( start == old_pools[j].start
	         &&
		 end   == old_pools[j].end )
	    {
	        // used_pools[i] == old_pools[j].
		// NULL old_pools[j] and break.
		//
	        old_pools[j].end = NULL;
		break;
	    }
	}
	if ( j == count )
	{
	    // used_pools[i] not found
	    // move it toward beginning of used_pools.
	    //
	    used_pools[new_count].start = start;
	    used_pools[new_count].end   = end;
	    ++ new_count;
	}
    }

    // Move non-NULLed old_pools to end of used_pools.
    //
    used_pools_count = new_count;
    old_count = 0;
    for ( unsigned j = 0; j < count; ++ j )
    {
        if ( old_pools[j].end == NULL ) continue;
	used_pools_push
	    ( old_pools[j].start, old_pools[j].end,
	      "compare_pools" );
	++ old_count;
    }
}

// Find the lowest address that ends a used pool, has
// size unused bytes following it, and preceeds some
// used pool.
//
static void * find_unused ( min::uns64 pages )
{
    MINT::pointer_uns size =
        (MINT::pointer_uns) pages * ::pagesize();
    void * best = NULL;
    for ( unsigned i = 0; i < used_pools_count; ++ i )
    {
        void * start = used_pools[i].end;
	if ( best != NULL && best < start ) continue;
	void * end = (void *) ( (char *) start + size );
	if ( end < start ) continue;
	    // Wraparound check.

	bool above_found = false;
	unsigned j;
	for ( j = 0; j < used_pools_count; ++ j )
	{
	    if ( i == j ) continue;
	    if ( used_pools[j].start >= end )
	        above_found = true;
	    else if ( used_pools[j].end >= start )
	        break;
	}
	if ( above_found && j == used_pools_count ) 
	    best = start;
    }
    return best;
}

void * MOS::new_pool ( min::uns64 pages )
{
    MINT::pointer_uns size =
        (MINT::pointer_uns) pages * ::pagesize();
    MINT::pointer_uns mask = ::pagesize() - 1;
    void * result = mmap ( NULL, (size_t) size,
    			   PROT_READ | PROT_WRITE,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1, 0 );

    if ( result == MAP_FAILED )
    {
        switch ( errno )
	{
	case EINVAL:	return error(2);
	case ENOMEM:	return error(3);
	default:
	    fatal_error ( "new_pool", errno );
	}
    }
    else if ( ( mask & (MINT::pointer_uns) result ) != 0 )
    {
	fatal_error ( "new_pool" )
	    << "OS returned pool address that is not on a page"
	       " boundary" << endl;
	exit ( 2 );
    }
    return result;
}

void * MOS::new_pool_at
	( min::uns64 pages, void * start )
{
    MINT::pointer_uns size =
        (MINT::pointer_uns) pages * ::pagesize();
    void * end = (void *) ( (char *) start + size );
    MINT::pointer_uns mask = ::pagesize() - 1;
    if ( ( (MINT::pointer_uns) start & mask ) != 0 )
    {
        fatal_error ( "new_pool_at" )
	    << "start address is not a multiple of page"
	       " size" << endl;
	exit ( 2 );
    }

    read_used_pools ( "new_pool_at" );
    if ( overlap ( start, end ) ) return error(1);

    void * result = mmap ( start, (size_t) size,
    			   PROT_READ | PROT_WRITE,
			   MAP_FIXED | MAP_PRIVATE
			             | MAP_ANONYMOUS,
			   -1, 0 );

    if ( result == MAP_FAILED )
    {
        switch ( errno )
	{
	case EINVAL:	return error(2);
	case ENOMEM:	return error(3);
	default:
	    fatal_error ( "new_pool_at", errno );
	}
    }

    assert ( result == start );
    return result;
}

void * MOS::new_pool_below
	( min::uns64 pages, void * end )
{
    MINT::pointer_uns size =
        (MINT::pointer_uns) pages * ::pagesize();
    MINT::pointer_uns mask = ::pagesize() - 1;

    read_used_pools ( "new_pool_below" );

    void * start = find_unused ( pages );
    if ( start == NULL )
        return error(3);
    if ( (void *) ( (char *) start + size ) > end )
        return error(3);

    void * result = mmap ( start, (size_t) size,
    			   PROT_READ | PROT_WRITE,
			   MAP_FIXED | MAP_PRIVATE
			             | MAP_ANONYMOUS,
			   -1, 0 );

    if ( result == MAP_FAILED )
    {
        switch ( errno )
	{
	case EINVAL:	return error(2);
	case ENOMEM:	return error(3);
	default:
	    fatal_error ( "new_pool_below", errno );
	}
    }

    assert ( result == start );
    return result;
}

void MOS::free_pool
	( min::uns64 pages, void * start )
{
    MINT::pointer_uns size =
        (MINT::pointer_uns) pages * ::pagesize();
    MINT::pointer_uns mask = ::pagesize() - 1;
    if ( ( (MINT::pointer_uns) start & mask ) != 0 )
    {
        fatal_error ( "free_pool" )
	    << "start address is not a multiple of page"
	       " size" << endl;
	exit ( 2 );
    }

    int result = munmap ( start, (size_t) size );

    if ( result == -1 )
	fatal_error ( "free_pool", errno );
}

inline void remap
	( void * new_start, void * old_start,
	  size_t size )
{
    void * result =
	mremap ( old_start, size, size,
		 MREMAP_MAYMOVE | MREMAP_FIXED,
		 new_start );

    if ( result == MAP_FAILED )
	fatal_error ( "move_pool", errno );

    assert ( result == new_start );
}

inline void renew
	( void * start, size_t size,
	  int protection = PROT_READ | PROT_WRITE )
{
    void * result = mmap ( start, size,
    			   protection,
			   MAP_FIXED | MAP_PRIVATE
			             | MAP_ANONYMOUS,
			   -1, 0 );

    if ( result == MAP_FAILED )
	fatal_error ( "move_pool", errno );

    assert ( result == start );
}

void MOS::move_pool
	( min::uns64 pages,
	  void * new_start, void * old_start )

{
    MINT::pointer_uns size =
        (MINT::pointer_uns) pages * ::pagesize();
    MINT::pointer_uns mask = ::pagesize() - 1;

    if ( ( (MINT::pointer_uns) new_start & mask ) != 0 )
    {
        fatal_error ( "move_pool" )
	    << "new start address is not a multiple of"
	       " page size" << endl;
	exit ( 2 );
    }

    if ( ( (MINT::pointer_uns) old_start & mask ) != 0 )
    {
        fatal_error ( "move_pool" )
	    << "old start address is not a multiple of"
	       " page size" << endl;
	exit ( 2 );
    }

    void * new_end =
        (void *) ( (char *) new_start + size );
    void * old_end =
        (void *) ( (char *) old_start + size );

    if ( new_end > old_start
         &&
	 old_end > new_start )
    {
        // Overlap: we must eliminate it.

	if ( new_start < old_start )
	{
	    // Move lower addresses first.
	    //
	    size_t step =
	        (char *) old_start - (char *) new_start;
	    while ( size > 0 )
	    {
		size_t s = step;
		if ( s > size ) s = size;
		remap ( new_start, old_start, s );
		new_start =
		    (void *) ( (char *) new_start + s );
		old_start =
		    (void *) ( (char *) old_start + s );
		size -= s;
	    }
	    renew ( new_end, step );
	}
	else if ( new_start > old_start )
	{
	    // Move higher addresses first.
	    //
	    size_t step =
	        (char *) new_start - (char *) old_start;
	    while ( size > 0 )
	    {
		size_t s = step;
		if ( s > size ) s = size;
		new_end =
		    (void *) ( (char *) new_end - s );
		old_end =
		    (void *) ( (char *) old_end - s );
		remap ( new_end, old_end, s );
		size -= s;
	    }
	    renew ( old_start, step );
	}
	else return;
    }
    else
    {
	remap ( new_start, old_start, size );
	renew ( old_start, size );
    }
}

void MOS::non_extant_pool
	( min::uns64 pages, void * start )

{
    MINT::pointer_uns size =
        (MINT::pointer_uns) pages * ::pagesize();
    MINT::pointer_uns mask = ::pagesize() - 1;

    if ( ( (MINT::pointer_uns) start & mask ) != 0 )
    {
        fatal_error ( "non_extant_pool" )
	    << "start address is not a multiple of"
	       " page size" << endl;
	exit ( 2 );
    }
    renew ( start, size, PROT_NONE );
}

void MOS::extant_pool
	( min::uns64 pages, void * start )

{
    MINT::pointer_uns size =
        (MINT::pointer_uns) pages * ::pagesize();
    MINT::pointer_uns mask = ::pagesize() - 1;

    if ( ( (MINT::pointer_uns) start & mask ) != 0 )
    {
        fatal_error ( "extant_pool" )
	    << "start address is not a multiple of"
	       " page size" << endl;
	exit ( 2 );
    }
    renew ( start, size );
}
