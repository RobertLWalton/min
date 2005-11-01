// MIN Language Protected Interface
//
// File:	min.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Tue Nov  1 09:46:03 EST 2005
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2005/11/01 14:46:03 $
//   $RCSfile: min.h,v $
//   $Revision: 1.11 $

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

	// Layout
	//   0x00-0xDF	stubs
	//   0xE0-0xEF  direct integers
	//   0xF0-0xF6  direct string and other
	//   0xF7-0xFF  illegal

	const unsigned GEN_STUB
	    = 0;
	const unsigned GEN_DIRECT_FLOAT
	    = 0xF7; // illegal
	const unsigned GEN_DIRECT_INT
	    = 0xE0;
	const unsigned GEN_DIRECT_STR
	    = 0xF0;
	const unsigned GEN_LIST_AUX
	    = 0xF1;
	const unsigned GEN_SUBLIST_AUX
	    = 0xF2;
	const unsigned GEN_INDIRECT_PAIR_AUX
	    = 0xF3;
	const unsigned GEN_INDIRECT_INDEXED_AUX
	    = 0xF4;
	const unsigned GEN_INDEX
	    = 0xF5;
	const unsigned GEN_CONTROL_CODE
	    = 0xF6;
	const unsigned GEN_ILLEGAL
	    = 0xF7;  // Upper limit for legal.
#   else // if MIN_IS_LOOSE

	// Layout with base MIN_FLOAT_SIGNALLING_NAN:
	//
	//   0x00-0x0F	stub
	//   0x10-0x16	direct string and others
	//   0x17-0x1F	illegal
	//   other	floating point

	const unsigned GEN_STUB
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x00;
	const unsigned GEN_DIRECT_FLOAT
	    = 0;
	const unsigned GEN_DIRECT_INT // illegal
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x17;
	const unsigned GEN_DIRECT_STR
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x10;
	const unsigned GEN_LIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x11;
	const unsigned GEN_SUBLIST_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x12;
	const unsigned GEN_INDIRECT_PAIR_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x13;
	const unsigned GEN_INDIRECT_INDEXED_AUX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x14;
	const unsigned GEN_INDEX
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x15;
	const unsigned GEN_CONTROL_CODE
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x16;
	const unsigned GEN_ILLEGAL
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x17;
	const unsigned GEN_UPPER
	    = MIN_FLOAT64_SIGNALLING_NAN + 0x1F;
#   endif

#   if MIN_IS_COMPACT
	inline bool is_direct_stub ( min::gen v )
	{
	    return ( v < GEN_DIRECT_INT )
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
	inline bool gen_subtype_of ( min::gen v )
	{
	    v >>= 24;
	    if ( v < GEN_DIRECT_INT )
	        return GEN_STUB;
	    else if ( v < GEN_DIRECT_STR)
	        return GEN_DIRECT_INT;
	    else if ( v < GEN_ILLEGAL)
	        return v;
	    else
	        return GEN_ILLEGAL;
	}
#   else // if MIN_IS_LOOSE
	inline bool is_direct_stub ( min::gen v )
	{
	    return ( v >> 44 == GEN_STUB >> 4 );
	}
	inline bool is_direct_float ( min::gen v )
	{
	    // Low order 45 bits and high order bit
	    // are masked for this test.
	    //
	    return
	        (    uns32 ( v >> 45 )
	          != MIN_FLOAT64_SIGNALLING_NAN >> 5 );
	}
	inline bool is_direct_int ( min::gen v )
	{
	    return false;
	}
	inline bool is_direct_str ( min::gen v )
	{
	    return ( v >> 40 == GEN_DIRECT_STR );
	}
	inline bool is_list_aux ( min::gen v )
	{
	    return ( v >> 40 == GEN_LIST_AUX );
	}
	inline bool is_sublist_aux ( min::gen v )
	{
	    return ( v >> 40 == GEN_SUBLIST_AUX );
	}
	inline bool is_indirect_pair_aux ( min::gen v )
	{
	    return
	        ( v >> 40 == GEN_INDIRECT_PAIR_AUX );
	}
	inline bool is_indirect_indexed_aux
		( min::gen v )
	{
	    return
	    	( v >> 40 == GEN_INDIRECT_INDEXED_AUX );
	}
	inline bool is_index ( min::gen v )
	{
	    return ( v >> 40 == GEN_INDEX );
	}
	inline bool is_control_code ( min::gen v )
	{
	    return ( v >> 40 == GEN_CONTROL_CODE );
	}
	inline bool gen_subtype_of ( min::gen v )
	{
	    v >>= 40;
	    if ( v < GEN_STUB )
	        return GEN_DIRECT_FLOAT;
	    else if ( v < GEN_DIRECT_STR)
	        return GEN_STUB;
	    else if ( v < GEN_ILLEGAL)
	        return v;
	    else if ( v <= GEN_UPPER)
	        return GEN_ILLEGAL;
	    else
	        return GEN_DIRECT_FLOAT;
	}
#   endif

    // Stub type codes.
    //
    const int DEALLOCATED		= 1;
    const int NUMBER			= 1;
    const int SHORT_STR			= 2;
    const int LONG_STR			= 3;
    const int LABEL			= 4;
    const int SHORT_OBJ			= 5;
    const int LONG_OBJ			= 6;
    const int LIST_AUX			= 7;
    const int SUBLIST_AUX		= 8;
    const int VARIABLE_VECTOR		= 9;

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
