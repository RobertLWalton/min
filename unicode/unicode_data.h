// UNICODE Data
//
// File:	unicode_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jul 15 06:57:36 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This can be include in a namespace.

typedef unsigned int Uchar;
    // Unicode character.  Must be 32 bit integer.

typedef unsigned char ustring;
    // A `ustring *' value is a sequence of 8 bit bytes
    // the first two of which are the `header' and the
    // remainder of which is a NUL terminated `const
    // char *' UTF8 encoded string (the ending NUL is
    // NOT part of the encoded string).
    // 
    // The header encodes the length of the UTF8 string
    // in bytes, excluding the header and terminating
    // NUL, in its first byte, and the number of print
    // columns taken the Uchars encoded by UTF8 string,
    // in its second byte.
    //
    // To compute the number of columns, it is assumed
    // all UNICODE characters take one column, except
    // non-spacing marks, with category 'N', take zero
    // columns.  This further assumes control characters
    // are not in ustrings for which the columns value
    // is used.

// Return length and number of print columns from the
// header of a ustring, and return a pointer to the
// `const char *' string in the ustring.
//
inline unsigned ustring_length
	( const ustring * p )
{
    return p[0];
}
inline unsigned ustring_columns
	( const ustring * p )
{
    return p[1];
}
inline const char * ustring_chars
	( const ustring * p )
{
    return (const char *) ( p + 2 );
}

const Uchar UNKNOWN_UCHAR = 0xFFFD;
    // == `unicode replacement character',

// Return the UTF8 encoded unicode character at the
// beginning of the string s and move s to just after
// that character.  Ends is just after the last
// character of the string, and if the string runs out
// before a legal unicode character is found, s is set
// to ends and UNKNOWN_UCHAR is returned.  If an illegal
// UTF8 encoding is encountered inside the string,
// UNKNOWN_UCHAR is returned and s is set to point just
// after the illegal UTF8 encoding.
//
// Overlong encodings and 7 bytes encodings of 32-bit
// unicode characters are supported.  Overlong encodings
// are treated as legal.
//
inline Uchar utf8_to_unicode
    ( const char * & s, const char * ends )
{
    if ( s >= ends ) return UNKNOWN_UCHAR;

    unsigned char c = (unsigned char) * s ++;
    if ( c < 0x80 ) return c;

    unsigned bytes = 0;
    Uchar unicode = c;
    if ( c < 0xC0 )
	unicode = UNKNOWN_UCHAR;
    else if ( c < 0xE0 )
	unicode &= 0x1F, bytes = 1;
    else if ( c < 0xF0 )
	unicode &= 0x0F, bytes = 2;
    else if ( c < 0xF8 )
	unicode &= 0x07, bytes = 3;
    else if ( c < 0xFC )
	unicode &= 0x03, bytes = 4;
    else if ( c < 0xFE )
	unicode &= 0x01, bytes = 5;
    else
	unicode &= 0x00, bytes = 6;

    if ( s + bytes > ends )
    {
	s = ends;
	return UNKNOWN_UCHAR;
    }

    while ( bytes -- )
    {
	c = (unsigned char) * s ++;
	if ( c < 0x80 || 0xC0 <= c )
	{
	    unicode = UNKNOWN_UCHAR;
	    -- s;
	    break;
	}
	unicode <<= 6;
	unicode += ( c & 0x3F );
    }
    return unicode;
}

// Output extended UTF8 byte string encoding a unicode
// character.  NUL is encoded as C0,80.  s points at
// the output buffer, which must be at least 7 charac-
// ters long, and s is updated to point after the out-
// putted byte string.
//
// Return the number of bytes output.
//
inline unsigned unicode_to_utf8
	( char * & s, Uchar unicode )
{
    if ( unicode == 0 )
    {
	* s ++ = 0xC0;
	* s ++ = 0x80;
	return 2;
    }
    else if ( unicode < 0x80 )
    {
	* s ++ = (char) unicode;
	return 1;
    }

    char * initial_s = s;
    unsigned shift = 0;
    unsigned char c = 0;
    if ( unicode < 0x7FF )
	shift = 6, c = 0xC0;
    else if ( unicode < 0xFFFF )
	shift = 12, c = 0xE0;
    else if ( unicode < 0x1FFFFF )
	shift = 18, c = 0xF0;
    else if ( unicode < 0x3FFFFFF )
	shift = 24, c = 0xF8;
    else if ( unicode < 0x7FFFFFFF )
	shift = 30, c = 0xFC;
    else
	shift = 36, c = 0xFE;
    while ( true )
    {
	* s ++ = (char)
	    ( c + ( ( unicode >> shift ) & 0x3F ) );
	if ( shift == 0 ) break;
	shift -= 6;
	c = 0x80;
    }
    return s - initial_s;
}

const Uchar SOFTWARE_NL = 0xF0FF;
const unsigned char SOFTWARE_NL_CATEGORY = 'e';
    // This is the private use UNICODE character that
    // is assigned to represent a `softare new-line',
    // such as the end of a MIN file line.  It is
    // given the name "NL" and corresponding control
    // picture character (as opposed to line-feed
    // which has the name "LF").

extern const unsigned unicode_index_size;
extern const unsigned short unicode_index[];
extern const unsigned unicode_index_limit;
    // Index table.
    //
    // unicode_index[c] is the index of c.  Indices are
    // very changeable, and may not be the same week
    // to week (unlike the category of c which should
    // never change).
    //
    // c < unicode_index_size is required.  Other
    // characters are not covered, are not given a name
    // or numeric value, and are given category 'w'.
    //
    // unicode_index[c] < unicode_index_limit for all c.
    //
    // There is a 1-1 correspondence
    //
    //    i <-> ( unicode_category[i],
    //            unicode_name[i],
    //            unicode_numerator[i],
    //            unicode_denominator[i] )
    //
    // between index values i and values of the given
    // 4-tuple.

const unsigned char UNSPECIFIED_CATEGORY = 'w';

extern const unsigned char unicode_category[];
    // unicode_category[unicode_index[c]] is the
    // character category of c.
    //
    // See the unicode_categories table for general
    // categories.
    //
    // However, as an exception, characters c < 256 are
    // their own categories (i.e., the category of `+'
    // is `+') if they are not a letter.  All other
    // categories are letters.

extern const ustring * const unicode_name[];
    // unicode_name[unicode_index[c]] is the ustring
    // name of c, or NULL if c has no name.  For
    // example, character code 0A has as ustring name
    // "\x02\x02" "LF" (where the two strings are
    // concatenated, which is done to prevent the
    // first character of the second from being
    // interpreted as a hexadecimal digit).
    //
    // Names are assigned by the UNICODE standard and
    // are NOT made up.

extern const ustring * const unicode_picture[];
    // unicode_picture[unicode_index[c]] is the ustring
    // picture name of c, or NULL if c has no picture
    // name.  E.g., character code 0A has as unicode_
    // picture the ustring "\x03\x01\xE2\x90\x8A", which
    // encodes 240A, the LF `control picture' UNICODE
    // character.

extern const double unicode_numerator[];
extern const double unicode_denominator[];
extern const double unicode_numeric_value[];
    // For a character c with a numeric value,
    //    unicode_numerator[unicode_index[c]]
    //    unicode_denominator[unicode_index[c]]
    //    unicode_numeric_value[unicode_index[c]]
    // give the numeric value of c.
    //
    // The numerator and denominator are integers, but
    // because the numerator may be as large as 10e12,
    // these are encoded as double's.
    //
    // If a character has NO numeric value, the
    // denominator is 0 and the numeric_value is NaN.
    //
    // Otherwise the denominator is > 0, the numerator
    // may be signed, and the numeric value is the ratio
    // of numerator/denominator (e.g., 1/3'rd is given
    // as the C++ constant expression `1.0/3.0' to
    // provide maximum accuracy).

extern const unsigned unicode_reference_count[];
    // unicode_numerator[t] is the number of character
    // codes c with t == unicode_index[c].  This is
    // just for integrety checking purposes.

const unsigned unicode_category_limit = 256;
extern const char * const
	unicode_category_name[unicode_category_limit];
    // unicode_category_name[cat] is the UNICODE stan-
    // dard general category name associated with
    // unicode_category[] value cat.  For example:
    //   unicode_category_name['U'] == "Lu"
    //   unicode_category_name['+'] == "Sm"
    // NULL if cat is NOT a category value.  Also
    // NULL for the `Unspecified' category 'w'.

extern const char * const
	unicode_category_description
	    [unicode_category_limit];
    // Ditto but instead of the 2-letter name a
    // longish description is given.  For example:
    //   unicode_category_description['U']
    //       == "Upper Case Letter"
    //   unicode_category_description['+']
    //       == "Math Symbol"
    //   unicode_category_description['w']
    //       == "Unspecified"

const unsigned unicode_supported_set_limit = 16;
const unsigned unicode_supported_set_shift = 12;
extern const char * const
	unicode_supported_set
	    [unicode_supported_set_limit];
    // unicode_supported_set[i] is the set name associ-
    // ated with the set index i.  E.g., 0 <--> "ascii"
    // and 1 <--> "latin1".  limit - 1 <--> NULL and
    // is not used.
