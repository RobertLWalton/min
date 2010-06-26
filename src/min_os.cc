// MIN OS Interface Code
//
// File:	min_os.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jun 26 12:37:11 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/06/26 16:48:47 $
//   $RCSfile: min_os.cc,v $
//   $Revision: 1.25 $

// Table of Contents:
//
//	Setup
//	Parameters
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
# include <cstdlib>
# include <cstring>
# include <cctype>
# include <cassert>
using std::cerr;
using std::endl;
using std::ostream;
using std::ifstream;
using std::istringstream;
using std::cout;
using std::hex;
using std::dec;

// Function name for error messages.
//
static const char * fname;

// Annouce fatal OS error for function named fname and
// return error output stream.
//
ostream & fatal_error ( void )
{
    return cerr << fname
                << ": FATAL OPERATING SYSTEM ERROR:"
                << endl << "    ";
}

// Announce fatal OS error involving errno code and exit
// program with errno code as status.
//
static void fatal_error ( int errno_code )
{
    fatal_error() << strerror ( errno_code )
    			  << endl;
    exit ( errno_code );
}

// Parameters
// ----------

static const char * MIN_CONFIG_value = NULL;
    // The value of the MIN_CONFIG environment parameter
    // obtained from getenv in cstdlib.  NULL if not yet
    // gotten.  If getenv returns NULL, this is set to
    // "".
const char * MOS::get_parameter ( const char * name )
{
    if ( MIN_CONFIG_value == NULL )
    {
        MIN_CONFIG_value = getenv ( "MIN_CONFIG" );
	if ( MIN_CONFIG_value == NULL )
	    MIN_CONFIG_value = "";
    }

    int length = ::strlen ( name );
    const char * p = MIN_CONFIG_value;
    while ( * p )
    {
        while ( isspace ( * p ) ) ++ p;
	const char * q = p;
	while ( * q && ! isspace ( * q ) ) ++ q;
	if ( q >= p + length + 1
	     &&
	     p[length] == '='
	     &&
	     ::strncmp ( name, p, length ) == 0 )
	    return p + length + 1;
	p = q;
    }
    return NULL;
}


// Memory Management
// ------ ----------

unsigned MOS::trace_pools = 0;

static min::unsptr max_mmap_size = 1ul << 31;
    // Largest size that can be given to mmap
    // without it returning an ENOMEM error.
    // This is because the system does not support
    // arbitarily large blocks of contiguous
    // virtual memory.

// Switch used by min_os_test to create compare_pools
// output without the printouts associated with trace_
// pools.
//
static bool create_compare = false;

static min::unsptr saved_pagesize;
inline min::unsptr pagesize ( void )
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
static const int pool_limit = 4;
static const char * pool_message[pool_limit] = {
/*0*/
    "",
/*1*/
    "new_pool: requested memory pool overlaps"
             " existing memory pool",
/*2*/
    "new_pool: requested pool size is too large",
/*3*/
    "new_pool: not enough memory or page map space"
             " is available"
};
static int saved_errno;
inline void * error ( int n )
{
    saved_errno = errno;
    if ( MOS::trace_pools >= 1 )
        cout << "TRACE: " << fname
	     << " failed due to error:" << endl
	     << "       " << pool_message[n] << endl;
    return (void *) n;
}

const char * MOS::pool_error ( void * address )
{
    min::unsptr mask = ::pagesize() - 1;
    unsigned n = (min::unsptr) address & mask;
    if ( n == 0 ) return NULL;
    assert ( n < pool_limit );
    return pool_message[n];
}

void MOS::dump_error_info ( ostream & s )
{
    s << "Operating System (OS) Extra Error"
         " Information:" << endl;
    if ( saved_errno == 0 )
        s << "    There was NO OS error." << endl;
    else
        s << "    Last OS Error: "
	  << strerror ( saved_errno ) << endl;

    dump_memory_layout ( s );
}

void MOS::dump_memory_layout ( ostream & s )
{
    char name[100];
    sprintf ( name, "/proc/%d/maps", (int) getpid() );
    s << "    OS Process Memory Map (" << name << "):"
       << endl;
    ifstream maps ( name );
    if ( ! maps )
        s << "    Cannot read " << name << endl;
    else
    {
	char line[1000];
	while ( maps.getline ( line, 1000 ) )
	{
	    if ( isspace ( line[71] ) )
	    {
		char * p = line + 71;
		while ( p > line && isspace ( p[-1] ) )
		    -- p;
		if ( p == line ) continue;
	        * p = 0;
		p = line + 72;
		while ( isspace ( * p ) ) ++ p;
		if ( * p )
		    s << "      " << line << " \\"
		      << endl
		      << "        " << p << endl;
		else
		    s << "      " << line << endl;
	    }
	    else
		s << "      " << line << endl;
	}
    }
}

// /proc/<this-process-id>/maps can be read and its
// information saved in used_pools.
//
static struct used_pool
    // List of memory pools in use.
{
    void * start, * end;

    char permissions[10];

    used_pool ( void ) {}

    used_pool ( void * start, void * end )
        : start ( start ), end ( end )
    {
        permissions[0] = 0;
    }

    used_pool ( min::uns64 pages, void * start )
        : start ( start )
    {
        end = (void *)
	    ( (char *) start + pages * ::pagesize() );
        permissions[0] = 0;
    }
} * used_pools;
static unsigned used_pools_count;
    // Current number of used_pools.
static unsigned used_pools_size;
    // Current limit on used_pools vector index.

static ostream & operator <<
	( ostream & out, const used_pool & p )
{
    out << hex
        << (min::unsptr) p.start << "-"
        << (min::unsptr) p.end << dec;
    if ( p.permissions[0] != 0 )
        out << " " << p.permissions;
    return out;
}

// Push entry into used_pools.
//
static void used_pools_push
	( void * start, void * end,
	  const char * permissions = "" )
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
    strcpy ( used_pools[used_pools_count].permissions,
             permissions );
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
static void read_used_pools ( void )
{
    char name[100];
    sprintf ( name, "/proc/%d/maps", (int) getpid() );
    ifstream maps ( name );
    if ( ! maps )
    {
        fatal_error()
	    << "cannot read " << name << endl;
	exit ( 1 );
    }
    used_pools_count = 0;
    char line[1000], permissions[1000];
    while ( maps.getline ( line, 1000 ) )
    {
        line[999] = 0;
	if ( strlen ( line ) >= 999 )
	{
	    fatal_error()
		<< "bad " << name << " line:" << endl
		<< "    " << line << endl;
	    exit ( 2 );
	}
	istringstream str ( line );
	min::uns64 start, end;
	char c1;
	str >> hex >> start >> c1 >> end >> permissions;
	if ( ! str || c1 != '-'
	           || strlen ( permissions ) > 5 )
	{
	    fatal_error()
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
	    fatal_error()
		<< "bad " << name << " line values:"
		<< endl << "    " << line << endl;
	    exit ( 2 );
	}
	if ( overlap ( (void *) (min::unsptr) start,
		       (void *) (min::unsptr) end ) )
	{
	    fatal_error()
	        << "bad " << name
		<< " line: overlaps previous:"
		<< endl << "    " << line << endl;
	    exit ( 2 );
	}
	used_pools_push
	    ( (void *)(min::unsptr) start,
	      (void *)(min::unsptr) end,
	      permissions );
    }
    maps.close();
}

// Dump used_pools entries for debugging.
//
static void dump_used_pools
    ( unsigned first = 0,
      unsigned count = used_pools_count )
{
    for ( unsigned i = first; i < first + count; ++ i )
        cout << used_pools[i] << endl;
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
static unsigned new_count, old_count;
static void compare_pools ( void )
{
    unsigned count = used_pools_count;
    used_pool * old_pools = new used_pool[count];
    memcpy ( old_pools, used_pools,
             count * sizeof ( used_pool ) );

    read_used_pools();

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
	    memcpy ( & used_pools[new_count],
	    	     & used_pools[i],
		     sizeof ( used_pool ) );
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
	      old_pools[j].permissions );
	++ old_count;
    }
    delete[] old_pools;
}

// Output results of compare_pools.
//
static void dump_compare_pools ( void )
{
    if ( new_count + old_count == 0 )
        cout << "MAP NOT CHANGED" << endl;
    else
    {
	if ( old_count > 0 )
	{
	    cout << "MAP ENTRIES DELETED:" << endl;
	    dump_used_pools ( new_count, old_count );
	}
	if ( new_count > 0 )
	{
	    cout << "MAP ENTRIES CREATED:" << endl;
	    dump_used_pools ( 0, new_count );
	}
    }
}

// Find the lowest address not less than begin that ends
// a used pool, has size unused bytes following it, and
// preceeds some used pool.
//
static void * find_unused
    ( void * begin, min::unsptr size )
{
    void * best = NULL;
    for ( unsigned i = 0; i < used_pools_count; ++ i )
    {
        void * next = used_pools[i].end;
	if ( next < begin ) continue;
	if ( best != NULL && best < next ) continue;
	void * end = (void *) ( (char *) next + size );
	if ( end < next ) continue;
	    // Wraparound check.

	bool above_found = false;
	unsigned j;
	for ( j = 0; j < used_pools_count; ++ j )
	{
	    if ( i == j ) continue;
	    if ( used_pools[j].start >= end )
	        above_found = true;
	    else if ( used_pools[j].end > next )
	        break;
	}
	if ( above_found && j == used_pools_count ) 
	    best = next;
    }
    return best;
}

// Execute new_pool_at without tracing.
//
static void * new_pool_at_internal
	( min::unsptr size, void * start )
{
    min::unsptr offset = 0;
    while ( offset < size )
    {
        min::unsptr increment = size - offset;
	if ( increment > max_mmap_size )
	    increment = max_mmap_size;

	void * result =
	    mmap ( (min::uns8 *) start + offset,
	           (size_t) increment,
		   PROT_READ | PROT_WRITE,
		   MAP_FIXED | MAP_PRIVATE
		             | MAP_ANONYMOUS,
		    -1, 0 );

	if ( result == MAP_FAILED )
	{
	    while ( offset > 0 )
	    {
	        offset -= max_mmap_size;

		int saved_errno = errno;
		if (    munmap (   (min::uns8 *) start
		                 + offset,
		                 (size_t)
				     max_mmap_size )
		     == -1 ) fatal_error ( errno );
		errno = saved_errno;
	    }

	    switch ( errno )
	    {
	    case EINVAL:	return error(2);
	    case ENOMEM:	return error(3);
	    default:
		fatal_error ( errno );
	    }
	}

	assert (    result
	         == (min::uns8 *) start + offset );

	offset += increment;
    }

    return start;
}

inline void prolog
	( const char * action, bool force_read = false )
{
    if ( MOS::trace_pools >= 2 )
    {
	read_used_pools();
	if ( MOS::trace_pools >= 3 )
	{
	    cout << "MEMORY MAP BEFORE " << action
	         << ":" << endl;
	    dump_used_pools();
	}
    }
    else if ( force_read || create_compare )
	read_used_pools();
}

inline void postlog
    ( const char * action, void * result,
                           min::uns64 pages )
{
    if (    MOS::trace_pools >= 2
         && MOS::pool_error ( result ) == NULL )
    {
	compare_pools();
        cout << action << ": "
	     << used_pool ( pages, result ) << endl;
	dump_compare_pools();
    }
    else if ( create_compare )
	compare_pools();
}

void * MOS::new_pool_at
	( min::uns64 pages, void * start )
{
    fname = "new_pool_at";

    if ( trace_pools >= 1 )
        cout << "TRACE: new_pool_at ( "
	     << pages << ", 0x" << hex
	     << (min::unsptr) start << " )"
	     << dec << endl;

    min::unsptr size =
        (min::unsptr) pages * ::pagesize();
    void * end = (void *) ( (char *) start + size );
    min::unsptr mask = ::pagesize() - 1;
    if ( ( (min::unsptr) start & mask ) != 0 )
    {
        fatal_error()
	    << "start address is not a multiple of page"
	       " size" << endl;
	exit ( 2 );
    }

    prolog ( "ALLOCATION", true );

    if ( overlap ( start, end ) ) return error(1);

    void * result =
        new_pool_at_internal ( size, start );

    postlog ( "ALLOCATED", result, pages );

    return result;
}

void * MOS::new_pool_between
	( min::uns64 pages, void * begin, void * end )
{
    fname = "new_pool_between";

    if ( trace_pools >= 1 )
        cout << "TRACE: new_pool_between ( "
	     << pages
	     << ", 0x" << hex
	     << (min::unsptr) begin
	     << ", 0x" << hex
	     << (min::unsptr) end
	     << " )" << dec << endl;

    min::unsptr size =
        (min::unsptr) pages * ::pagesize();
    min::unsptr mask = ::pagesize() - 1;

    prolog ( "ALLOCATION", true );

    void * address = find_unused ( begin, size );
    errno = 0;
    if ( address == NULL )
        return error(3);
    if ( end != NULL 
         &&
	 (void *) ( (char *) address + size ) > end )
        return error(3);

    void * result =
        new_pool_at_internal ( size, address );

    postlog ( "ALLOCATED", result, pages );

    return result;
}

void * MOS::new_pool ( min::uns64 pages )
{
    fname = "new_pool";

    if ( trace_pools >= 1 )
        cout << "TRACE: new_pool ( " << pages << " )"
	     << endl;

    min::unsptr size =
        (min::unsptr) pages * ::pagesize();

    void * result;

    if ( size <= max_mmap_size )
    {
    	// As an optimization, call mmap directly if
	// size is not too large.

	prolog ( "ALLOCATION" );

	result = mmap ( NULL, (size_t) size,
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
		fatal_error ( errno );
	    }
	}

	min::unsptr mask = ::pagesize() - 1;
	if (    ( mask & (min::unsptr) result )
	     != 0 )
	{
	    fatal_error()
		<< "OS returned pool address that is"
		   " not on a page boundary" << endl;
	    exit ( 2 );
	}
    }
    else
    {
	prolog ( "ALLOCATION", true );

	void * address = find_unused ( NULL, size );
	errno = 0;
	if ( address == NULL ) return error(3);
	result = new_pool_at_internal ( size, address );
    }

    postlog ( "ALLOCATED", result, pages );

    return result;
}

void MOS::free_pool
	( min::uns64 pages, void * start )
{
    fname = "free_pool";

    if ( trace_pools >= 1 )
        cout << "TRACE: free_pool ( "
	     << pages << ", 0x" << hex
	     << (min::unsptr) start << " )"
	     << dec << endl;

    min::unsptr size =
        (min::unsptr) pages * ::pagesize();
    min::unsptr mask = ::pagesize() - 1;
    if ( ( (min::unsptr) start & mask ) != 0 )
    {
        fatal_error()
	    << "start address is not a multiple of page"
	       " size" << endl;
	exit ( 2 );
    }

    prolog ( "DEALLOCATION" );

    int result = munmap ( start, (size_t) size );

    if ( result == -1 )
	fatal_error ( errno );

    postlog ( "DEALLOCATED", start, pages );
}

void MOS::purge_pool
	( min::uns64 pages, void * start )
{
    fname = "purge_pool";

    if ( trace_pools >= 1 )
        cout << "TRACE: purge_pool ( "
	     << pages << ", 0x" << hex
	     << (min::unsptr) start << " )"
	     << dec << endl;

    min::unsptr size =
        (min::unsptr) pages * ::pagesize();
    min::unsptr mask = ::pagesize() - 1;
    if ( ( (min::unsptr) start & mask ) != 0 )
    {
        fatal_error()
	    << "start address is not a multiple of page"
	       " size" << endl;
	exit ( 2 );
    }

    prolog ( "PURGING" );

    min::unsptr offset = 0;
    while ( offset < size )
    {
        min::unsptr increment = size - offset;
	if ( increment > max_mmap_size )
	    increment = max_mmap_size;

	void * result =
	    mmap ( (min::uns8 *) start + offset,
	           (size_t) increment,
		   PROT_READ | PROT_WRITE,
		   MAP_FIXED | MAP_PRIVATE
		             | MAP_ANONYMOUS,
		    -1, 0 );

	if ( result == MAP_FAILED )
	    fatal_error ( errno );

	assert (    result
	         == (min::uns8 *) start + offset );

	offset += increment;
    }

    postlog ( "PURGING", start, pages );
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
	fatal_error ( errno );

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
	fatal_error ( errno );

    assert ( result == start );
}

void MOS::move_pool
	( min::uns64 pages,
	  void * new_start, void * old_start )

{
    fname = "move_pool";

    if ( trace_pools >= 1 )
        cout << "TRACE: move_pool ( "
	     << pages << ", 0x" << hex
	     << (min::unsptr) new_start << ", 0x"
	     << (min::unsptr) old_start << " )"
	     << dec << endl;

    min::unsptr size =
        (min::unsptr) pages * ::pagesize();
    min::unsptr mask = ::pagesize() - 1;

    if ( ( (min::unsptr) new_start & mask ) != 0 )
    {
        fatal_error()
	    << "new start address is not a multiple of"
	       " page size" << endl;
	exit ( 2 );
    }

    if ( ( (min::unsptr) old_start & mask ) != 0 )
    {
        fatal_error()
	    << "old start address is not a multiple of"
	       " page size" << endl;
	exit ( 2 );
    }

    void * new_end =
        (void *) ( (char *) new_start + size );
    void * old_end =
        (void *) ( (char *) old_start + size );

    prolog ( "MOVE" );

    void * original_new_start = new_start;
    void * original_old_start = old_start;

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
    }
    else
    {
	remap ( new_start, old_start, size );
	renew ( old_start, size );
    }

    if ( trace_pools >= 2 )
    {
	compare_pools();
        cout << "MOVED: "
	     << used_pool ( pages, original_old_start )
	     << " TO "
	     << used_pool ( pages, original_new_start )
	     << endl;
	dump_compare_pools();
    }
    else if ( create_compare )
	compare_pools();
}

void MOS::inaccess_pool
	( min::uns64 pages, void * start )

{
    fname = "inaccess_pool";

    if ( trace_pools >= 1 )
        cout << "TRACE: inaccess_pool ( "
	     << pages << ", 0x" << hex
	     << (min::unsptr) start << " )"
	     << dec << endl;

    min::unsptr size =
        (min::unsptr) pages * ::pagesize();
    min::unsptr mask = ::pagesize() - 1;

    if ( ( (min::unsptr) start & mask ) != 0 )
    {
        fatal_error()
	    << "start address is not a multiple of"
	       " page size" << endl;
	exit ( 2 );
    }

    prolog ( "PERMISSION CHANGE" );

    renew ( start, size, PROT_NONE );

    postlog ( "MADE INACCESSIBLE", start, pages );
}

void MOS::access_pool
	( min::uns64 pages, void * start )

{
    fname = "access_pool";

    if ( trace_pools >= 1 )
        cout << "TRACE: access_pool ( "
	     << pages << ", 0x" << hex
	     << (min::unsptr) start << " )"
	     << dec << endl;

    min::unsptr size =
        (min::unsptr) pages * ::pagesize();
    min::unsptr mask = ::pagesize() - 1;

    if ( ( (min::unsptr) start & mask ) != 0 )
    {
        fatal_error()
	    << "start address is not a multiple of"
	       " page size" << endl;
	exit ( 2 );
    }

    prolog ( "PERMISSION CHANGE" );

    renew ( start, size );

    postlog ( "MADE ACCESSIBLE", start, pages );
}
