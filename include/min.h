// MIN Language Data
//
// File:	min_data.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Sep  4 05:41:12 EDT 2004
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2004/09/06 09:18:51 $
//   $RCSfile: min.h,v $
//   $Revision: 1.4 $

struct min {
    struct data;
};

# define MIN_COMPACT_DATA 1
# define MIN_32_BIT_INT int
# define MIN_64_BIT_INT long long
# define MIN_BIG_ENDIAN 0
# define MIN_POINTER_LENGTH 32

struct min::data {

    typedef unsigned char uns8;
    typedef signed char int8;

    typedef unsigned short uns16;
    typedef signed short int16;

    typedef unsigned MIN_32_BIT_INT uns32;
    typedef signed MIN_32_BIT_INT int32;

    typedef unsigned MIN_64_BIT_INT uns64;
    typedef signed MIN_64_BIT_INT int64;

    typedef double float64;

    struct stub;
    struct body;


# if MIN_COMPACT_DATA

    // A general datum can hold either an IEEE 64 bit
    // floating point number, a pointer to a stub, or
    // a 6 byte string.  The high order 16 bits deter-
    // mine what is held.  There bits have one special
    // value for pointers to stubs, another for strings,
    // and all other values are for IEEE floating point
    // numbers.

    typedef uns64 gen64;

    inline bool gen_is_float ( gen64 v ) {
        return    (v & 0x7FFF000000000000)
	       != 0x7FFF000000000000;
    }
    inline bool gen_is_stub ( gen64 v ) {
        return    (v & 0xFFFF000000000000)
	       == 0xFFFF000000000000;
    }
    inline bool gen_is_string ( gen64 v ) {
        return    (v & 0xFFFF000000000000)
	       == 0x7FFF000000000000;
    }
    inline float64 gen_to_float ( gen64 v ) {
        return * (float64 *) & v;
    }
    inline stub * gen_to_stub ( gen64 v ) {
        return (stub *) (unsigned)
	       ( v & 0xFFFFFFFFFFFF );
    }
    // p[8] must be on 8 byte boundary.
    inline void gen_to_string ( gen64 v, char p[8] ) {
#	if MIN_BIG_ENDIAN
	    * (uns64 *) p = v << 16;
#	else
	    * (uns64 *) p = v & 0xFFFFFFFFFFFF;
#	endif
    }
    inline gen64 float_to_gen ( float64 v ) {
        return * (gen64 *) & v;
    }
    inline gen64 stub_to_gen ( stub * p ) {
        return   ((uns64) (unsigned) p)
	       | 0xFFFF000000000000;
    }
    // p[8] must be on 8 byte boundary.
    inline gen64 string_to_gen ( char p[8] ) {
#	if MIN_BIG_ENDIAN
	    return   ( ( * (uns64 *) p ) >> 16 )
	           | 0x7FFF000000000000;
#	else
	    return   (   ( * (uns64 *) p )
	               & 0xFFFFFFFFFFFF )
	           | 0x7FFF000000000000;
#	endif
    }
# endif

    struct body
    {
    };

    struct stub
    {
	union {
	    float64 f;
	    char c[8];
	    uns64 u;
	    int64 i;
	    struct {
#		if MIN_BIG_ENDIAN
		    uns32 hi;
		    uns32 lo;
#		else
		    uns32 lo;
		    uns32 hi;
#		endif
	    } s;
	} v; // value

	union {
	    uns64 u;
	    struct {
#		if MIN_BIG_ENDIAN
		    uns8 t; // type
		    uns8 st; // subtype
		    uns16 s;
		    uns32 lo;
#		else
		    uns32 lo;
		    uns16 s;
		    uns8 st; // subtype
		    uns8 t; // type
#		endif
	    } s;
	} g; // gc and control
    };

    // Get value from a stub.
    //
    inline float64 get_float64 ( const stub * p ) {
        return p->v.f;
    }
    inline int64 get_int64 ( const stub * p ) {
        return p->v.i;
    }
    inline uns64 get_uns64 ( const stub * p ) {
        return p->v.u;
    }
    inline void get_string
	    ( char v[8], const stub * p ) {
        * (uns64 *) v = p->v.u;
    }
    inline body * get_body ( const stub * p ) {
#	if MIN_POINTER_LENGTH == 32
	    return (body *) p->v.s.lo;
#	else
	    return (body *) p->v.u;
#	endif
    }

    // Put value into a stub.
    //
    inline void put_float64 ( stub * p, float64 v ) {
        p->v.f = v;
    }
    inline void put_uns64 ( stub * p, uns64 v ) {
        p->v.u = v;
    }
    inline void put_int64 ( stub * p, int64 v ) {
        p->v.i = v;
    }
    inline void put_string ( stub * p, char v[8] ) {
        p->v.u = * (uns64 *) v;
    }
    inline void put_body ( stub * p, body * v ) {
#	if MIN_POINTER_LENGTH == 32
	    p->v.s.hi = 0;
	    p->v.s.lo = (uns32) v;
#	else
	    p->v.u = (uns64) v;
#	endif
    }

    // Get type, chain, etc. from a stub.
    //
    inline unsigned get_type ( const stub * p ) {
        return p->g.s.t;
    }
    inline unsigned get_subtype ( const stub * p ) {
        return p->g.s.st;
    }
    inline stub * get_chain ( const stub * p ) {
#	if MIN_POINTER_LENGTH == 32
	    return (stub *) p->g.s.lo;
#	else
	    return (stub *) ( p->g.u & 0xFFFFFFFFFFFF );
#	endif
    }
};
