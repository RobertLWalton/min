// MIN Language Protected Interface
//
// File:	min.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Nov  1 07:06:17 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/01 12:06:19 $
//   $RCSfile: min.h,v $
//   $Revision: 1.9 $

# ifndef MIN_H

// Include parameters.
//
# include "min_parameters.h"
# include <cassert>

namespaces min {

    // Number Types

    typedef unsigned char uns8;
    typedef signed char int8;

    typedef unsigned short uns16;
    typedef signed short int16;

    typedef unsigned MIN_32_BIT_INT uns32;
    typedef signed MIN_32_BIT_INT int32;
    typedef float float32;

    typedef unsigned MIN_64_BIT_INT uns64;
    typedef signed MIN_64_BIT_INT int64;
    typedef double float64;

    // We assume the machine has integer registers that
    // are the most efficient place for min::gen values.
    //
#   if MIN_IS_COMPACT
	typedef uns32 gen;
#   else // if MIN_IS_LOOSE
	typedef uns64 gen;
#   endif

#   if MIN_IS_COMPACT
	const unsigned GEN_STUB
	    = 0;
	const unsigned GEN_DIRECT_FLOAT
	    = 0x100; // illegal
	const unsigned GEN_DIRECT_INT
	    = 0xF0;
	const unsigned GEN_DIRECT_STR
	    = 0xE1;
	const unsigned GEN_LIST_AUX
	    = 0xE2;
	const unsigned GEN_SUBLIST_AUX
	    = 0xE3;
	const unsigned GEN_INDIRECT_PAIR_AUX
	    = 0xE4;
	const unsigned GEN_INDIRECT_INDEXED_AUX
	    = 0xE5;
	const unsigned GEN_INDEX
	    = 0xE6;
	const unsigned GEN_CONTROL_CODE
	    = 0xE7;
	const unsigned GEN_ILLEGAL
	    = 0x100;
#   else // if MIN_IS_LOOSE
	const unsigned GEN_STUB
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x10;
	const unsigned GEN_DIRECT_FLOAT
	    = 0;
	const unsigned GEN_DIRECT_INT
	    = 0x1000000; // illegal
	const unsigned GEN_DIRECT_STR
	    = MIN_FLOAT64_SIGNALLING_NAN + 1;
	const unsigned GEN_LIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 2;
	const unsigned GEN_SUBLIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 3;
	const unsigned GEN_INDIRECT_PAIR_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 4;
	const unsigned GEN_INDIRECT_INDEXED_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 5;
	const unsigned GEN_INDEX
	    = MIN_FLOAT64_SIGNALLING_NAN + 6;
	const unsigned GEN_CONTROL_CODE
	    = MIN_FLOAT64_SIGNALLING_NAN + 7;
	const unsigned GEN_ILLEGAL
	    = 0x1000000;
#   endif

#   if MIN_IS_COMPACT
	inline bool is_direct_stub ( min::gen v )
	{
	    return ( v >> 28 < 0xE )
	}
	inline bool is_direct_float ( min::gen v )
	{
	    return false;
	}
	inline bool is_direct_int ( min::gen v )
	{
	    return ( v >> 28 == GEN_DIRECT_INT >> 4 );
	}
	inline bool is_direct_str ( min::gen v )
	{
	    return ( v >> 24 == GEN_DIRECT_STR );
	}
	inline bool is_list_aux ( min::gen v )
	{
	    return ( v >> 24 == GEN_LIST_AUX );
	}
	inline bool is_sublist_aux ( min::gen v )
	{
	    return ( v >> 24 == GEN_SUBLIST_AUX );
	}
	inline bool is_indirect_pair_aux ( min::gen v )
	{
	    return
	        ( v >> 24 == GEN_INDIRECT_PAIR_AUX );
	}
	inline bool is_indirect_indexed_aux
		( min::gen v )
	{
	    return
	    	( v >> 24 == GEN_INDIRECT_INDEXED_AUX );
	}
	inline bool is_index ( min::gen v )
	{
	    return ( v >> 24 == GEN_INDEX );
	}
	inline bool is_control_code ( min::gen v )
	{
	    return ( v >> 24 == GEN_CONTROL_CODE );
	}
#   else // if MIN_IS_LOOSE
#   endif

    // Type code for stub.
    //
    enum stub_type
    {
        NUMBER		= 1,
	SHORT_STRING	= 2,
	LONG_STRING	= 3,
	SHORT_OBJECT	= 4,
	LONG_OBJECT	= 5,
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

    struct long_string
    {
        union {
	    uns8 small_length;
	    uns32 large_length;
	} u;
    };

    struct short_object
    {
        uns16	maximum_list_length;
        uns16	list_length;
        uns16	maximum_hash_length;
        uns16	hash_length;
    };

    struct long_object
    {
        uns32	maximum_list_length;
        uns32	list_length;
        uns32	maximum_hash_length;
        uns32	hash_length;
    };

    // Get non-pointer values from a stub.
    //
    friend inline float64 get_float64 ( const stub * p )
    {
        return p->v.f;
    }
    friend inline int64 get_int64 ( const stub * p )
    {
        return p->v.i;
    }
    friend inline uns64 get_uns64 ( const stub * p )
    {
        return p->v.u;
    }
    friend inline void get_string
	    ( char v[8], const stub * p )
    {
        * (uns64 *) v = p->v.u;
    }

    // Get pointer values from stub.  There are more
    // efficient min::unprotected versions of these
    // that have no asserts.
    //
    friend inline long_string * get_long_string
    	( const stub * p )
    {
        assert ( get_type ( p ) == LONG_STRING );
#	if MIN_POINTER_LENGTH == 32
	    return (long_string *) p->v.s.lo;
#	else
	    return (long_string *) p->v.u;
#	endif
    }
    friend inline short_object * get_short_object
    	( const stub * p )
    {
        assert ( get_type ( p ) == SHORT_OBJECT );
#	if MIN_POINTER_LENGTH == 32
	    return (short_object *) p->v.s.lo;
#	else
	    return (short_object *) p->v.u;
#	endif
    }
    friend inline long_object * get_long_object
    	( const stub * p )
    {
        assert ( get_type ( p ) == LONG_OBJECT );
#	if MIN_POINTER_LENGTH == 32
	    return (long_object *) p->v.s.lo;
#	else
	    return (long_object *) p->v.u;
#	endif
    }

    // Get type from a stub.
    //
    friend inline unsigned get_type ( const stub * p )
    {
        return p->g.s.t;
    }

    // Get subtype, chain, etc. from non-collectable
    // stub.
    //
    // There are unprotected versions of the following
    // functions in min::unprotected that are more
    // efficient because they have no assert.
    //
    friend inline unsigned get_subtype
    	( const stub * p )
    {
	assert ( p->v.i < 0 );
        return p->g.s.st;
    }
    friend inline stub * get_chain ( const stub * p )
    {
	assert ( p->v.i < 0 );
#	if MIN_POINTER_LENGTH == 32
	    return (stub *) p->g.s.lo;
#	else
	    return (stub *) ( p->g.u & 0xFFFFFFFFFFFF );
#	endif
    }
};

# endif // MIN_H
