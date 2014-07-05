// UNICODE Data
//
// File:	unicode_types.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jul  5 06:12:07 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This can be include in a namespace.

typedef unsigned int Uchar;
    // Unicode character.  Must be 32 bit integer.

typedef Uchar Ustring;
    // A `Ustring *' value is a sequence of 32 bit
    // integers the first of which is the `header'
    // and the remainder of which is a vector of Uchars.
    // 
    // The header encodes the length of the Uchar
    // vector, excluding the header, in its low order
    // 16 bits, and the number of print columns taken
    // by Uchars in the vector, in its high order 16
    // bits.
    //
    // To compute the number of columns, it is assumed
    // all UNICODE characters take one column, except
    // non-spacing marks, with category 'N', take zero
    // columns.  This further assumes control characters
    // are not in Ustrings for which the columns value
    // is used.

inline min::uns32 Ustring_length
	( const Ustring * p )
{
    return * p & 0xFFFF;
}
inline min::uns32 Ustring_columns
	( const Ustring * p )
{
    return * p >> 16;
}
inline const Uchar * Ustring_chars
	( const Ustring * p )
{
    return p + 1;
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
inline unsptr unicode_to_utf8
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
    uns32 shift = 0;
    uns8 c = 0;
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

extern const unsigned unicode_index_size;
extern const unsigned short unicode_index[];
    // Index table.
    //
    // unicode_index[c] is the index of c.  Indices are
    // very changeable, and may not be the same week
    // to week (unlike the category of c which should
    // never change).

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

extern const Ustring * unicode_Uname[];
    // unicode_Uname[unicode_index[c]] is the Ustring
    // name of c, or NULL if c has no name.  For
    // example, character code 0A has as Ustring name
    // { 0x2002, 'L', 'F' }.
    //
    // Names are assigned by the UNICODE standard and
    // are NOT made up.

extern const char * unicode_name[];
    // unicode_name[unicode_index[c]] is the ASCII name
    // of c, or NULL if c has no name.  For example,
    // character code 0A has as ASCII name "LF".

extern const double unicode_numerator[];
extern const double unicode_denominator[];
extern const double unicode_number_value[];
    // For a character c with a number value,
    //    unicode_numerator[unicode_index[c]]
    //    unicode_denominator[unicode_index[c]]
    //    unicode_number_value[unicode_index[c]]
    // give the numeric value of c.
    //
    // The numerator and denominator are integers, but
    // because the numerator may be as large as 10e12,
    // these are encoded as double's.
    //
    // If a character has NO number value, the
    // denominator is 0 and the number_value is NaN.
    //
    // Otherwise the denominator is > 0, the numerator
    // may be signed, and the number value is the ratio
    // of numerator/denominator (e.g., 1/3'rd is given
    // as the C++ constant expression `1.0/3.0' to
    // provide maximum accuracy).

extern const unicode_reference_count[];
    // unicode_numerator[t] is the number of character
    // codes c with t == unicode_index[c].  This is
    // just for integrety checking purposes.

extern const unsigned unicode_types_size;
extern const unicode_type unicode_types[];

extern const unsigned unicode_names_size;
extern const Uchar unicode_names[];

// Table that encodes UNICODE general category to our
// MIN category map.  The UNICODE general category is
// 2 letters, while the MIN category is 1 letter and is
// better suited to making lookup-by-category tables.
// We also add the special MIN category `w' for
// characters with no UNICODE category.
//
struct unicode_category
{
    const char * unicode_name;
    unsigned char category;  // MIN category.
    const char * unicode_description;
};
extern const unsigned unicode_categories_size;
extern const unicode_category unicode_categories[];
const unsigned char UNSPECIFIED_CATEGORY = 'w';

} }

