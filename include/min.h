// MIN Language Data
//
// File:	min_data.h
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Thu Sep  2 20:29:19 EDT 2004
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2004/09/03 09:37:16 $
//   $RCSfile: min.h,v $
//   $Revision: 1.1 $

# define MIN_COMPACT_DATA 1
# define MIN_32_BIT_INT int
# define MIN_64_BIT_INT long long
# define MIN_BIG_ENDIAN 0

class min {

    typedef unsigned char uns8;
    typedef signed char int8;

    typedef unsigned short uns16;
    typedef signed short int16;

    typedef unsigned MIN_32_BIT_INT uns32;
    typedef signed MIN_32_BIT_INT int32;

    typedef unsigned MIN_64_BIT_INT uns64;
    typedef signed MIN_64_BIT_INT int64;


// If we have compact data, the 64 bit value is over-
// loaded as an IEEE floating point number or as a 16
// bit type fixed header with a 48 bit value.
//
# if MIN_COMPACT_DATA

    union gen64
    {
        float64 f;
        struct {
#	    if MIN_BIG_ENDIAN
		uns16 h; // header
		uns16 t; // type
		uns32 v; // value
#	    else
		uns32 v; // value
		uns16 t; // type
		uns16 h; // header
#	    endif
	} s;
    };
# endif

    struct body
    {
    };

    struct stub
    {
	union {
	    float64 f;
	    uns8 c[8];
	    body * p;
	    uns64 u;
	    int64 i;
	} v; // value

	union {
	    uns64 u;
	    struct {
#		if MIN_BIG_ENDIAN
		    uns8 t; // type
		    uns8 pad[3];
		    uns32 u;
#		else
		    uns32 u;
		    uns8 pad[3];
		    uns8 t; // type
#		endif
	    } s;
	} g; // gc and control
    };
};
