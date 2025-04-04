// UNICODE Data
//
// File:	unicode_data.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu May 25 13:37:00 EDT 2023
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This file should be included inside a namespace.

typedef unsigned int Uchar;
    // Unicode character.  Must be 32 bit unsigned
    // integer.

typedef unsigned long long uns64;
typedef unsigned int uns32;
typedef unsigned short uns16;
typedef unsigned char uns8;
    // Must be 64, 32, 16, and 8 bit unsigned integers.

const Uchar UNKNOWN_UCHAR = 0xFFFD;
    // == `unicode replacement character',
    //
    // Returned when UTF8 decoder finds an illegal
    // encoding.

const Uchar SOFTWARE_NL = 0xF0FF;
    // This is the private use UNICODE character that
    // is assigned to represent a `software new-line',
    // such as the end of a MIN file line.  It is
    // given the name "SWNL" and corresponding control
    // picture character (as opposed to line-feed
    // which has the names "LF" and "NL").

const Uchar NO_UCHAR = 0xFFFFFFFF;
    // Denotes a missing Uchar value.

typedef const uns8 * ustring;
    // A `ustring' value is a vector of 8 bit bytes, the
    // first two of which are the `header' and the
    // remainder of which is a NUL terminated `const
    // char *' UTF8 encoded string.
    // 
    // The header encodes the length of the UTF8 string
    // in bytes, excluding the header and terminating
    // NUL, in its first byte, and the number of print
    // columns taken the Uchars encoded by UTF8 string,
    // in its second byte.  In some applications the
    // high order 2 bits of these two types may be used
    // for flags, limiting the size of the allowed
    // length and number of columns to 63.  Ustrings in
    // unicode_data.cc do NOT have any flags.
    //
    // To compute the number of columns, it is assumed
    // all UNICODE characters take one column, except
    // non-spacing marks, with category 'Mn', take zero
    // columns.  This further assumes control characters
    // are not in ustrings for which the columns value
    // is used.

// Return length and number of print columns from the
// header of a ustring, and return a pointer to the
// `const char *' UTF8 string in the ustring.
//
inline uns32 ustring_length
	( ustring s )
{
    return s[0] & 0x3F;
}
inline uns32 ustring_columns
	( ustring s )
{
    return s[1] & 0x3F;
}
inline const char * ustring_chars
	( ustring s )
{
    return (const char *) ( s + 2 );
}

// Return the UTF8 encoded unicode character at the
// beginning of the string s and move s to just after
// that character.  Ends is just after the last
// character of the string.  If s >= ends when this
// function is called, NO_UCHAR is returned.  Otherwise
// if the string runs out before a legal unicode char-
// acter is found, s is set to ends and UNKNOWN_UCHAR
// is returned.  If an illegal UTF8 encoding is encount-
// ered inside the string, UNKNOWN_UCHAR is returned and
// s is set to point just after the illegal UTF8 encod-
// ing.
//
// Overlong encodings and 7 bytes encodings of 32-bit
// unicode characters are supported.  Overlong encodings
// are treated as legal.
//
inline Uchar utf8_to_unicode
    ( const char * & s, const char * ends )
{
    if ( s >= ends ) return NO_UCHAR;

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

// Support sets are sets of characters, and a character
// is only supported if it belongs to some support set.
//
// There are two kinds of support sets.
//
// cc support sets are those listed in the file
// CombiningCharacter.txt, plus in addition the sets
//
//   ascii	for character c, 0 <= c <= 0x7F
//   special	for UNKNOWN_UCHAR and SOFTWARE_NL
//
// ss support sets are those listed in the file unicode_
// data_support_sets.h which contains just the code:
//
// 	enum {
// 	   . . . . .
// 	   s = ( 1 << k ),
// 	   . . . . .
// 	};
//
// for every ss support set s with shift k.  Each ss
// support set has a shift k with 0 <= k < 32.
//
// Each cc support set c has a 32 bit unsigned integer
// mask m such that c is contained in an ss support set
// s with shift k if and only if m & ( 1 << k ) != 0.
//
// Each character c has a 32 bit unsigned integer mask m
// such that c is in an ss support set s with shift k if
// and only if m & ( 1 << k ) != 0.  Here
//
// 	m == support_sets[index[c]]
//
// where support_sets and index are described below.

extern uns32 const ss_support_sets_size;
extern const char * const ss_support_sets_name[];
extern uns8 const ss_support_sets_shift[];
    // An ss support set s has shift k if and only if
    // for some i, 0 <= i < ss_support_sets_size,
    //
    //     "s" == ss_support_sets_name[i]
    //      k  == ss_support_sets_shift[i]

extern uns32 const cc_support_sets_size;
extern const char * const cc_support_sets_name[];
extern uns32 const cc_support_sets_mask[];
    // A cc support set c has mask m if and only if
    // for some j, 0 <= j < cc_support_sets_size,
    //
    //     "c" == cc_support_sets_name[j]
    //      m  == cc_support_sets_mask[j]

extern Uchar const index_size;
extern uns16 const index[];
extern uns16 const index_limit;
    // Index table.
    //
    // index[c] is the index of c.  For c <= 0xFF,
    // index[c] == c.  For other c, indices may change
    // at any time.
    //
    // index[c] is only defined for c < index_size.  For
    // c >= index_size, the index of c is defined to be
    // index[index_size-1], which is given category
    // NULL.
    //
    // index[c] < index_limit for all c.
    //
    // index_limit is made to be as small as possible
    // subject to the constraints that index[c] == c 
    // for c <= 0xFF and no two characters with the same
    // index have the same:
    //
    //    From UnicodeData.txt:
    // 		general category
    // 		canonical combining class
    // 		bidi class
    // 		numeric value
    // 		bidi mirrored indicator
    //
    // 	  From PropList.txt:
    // 		properties (see table below)
    //
    // 	  From NameAliases.txt:
    // 		name (aka UNICODE `alias')
    //
    // 	  As indicated below:
    // 		picture
    //
    // 	  From CompositeCharacters.txt:
    // 	        Supported Set


extern const Uchar character[];
    // character[i] == c iff i = index[c] and exactly
    // one character c has index i.  Otherwise
    // character[i] == NO_UCHAR.

extern const char * const category[];
    // category[index[c]] is the UNICODE General
    // Category of c; e.g., the category of `A' is "Lu".
    //
    // category[index_size-1] == NULL.

extern short const combining_class[];
    // combining_class[index[c]] is the UNICODE
    // Combining Class of c, expressed as a small
    // non-negative integer.  -1 if missing.

extern const char * const bidi_class[];
    // bidi_class[index[c]] is the UNICODE Bidi Class of
    // c, expressed as its abbreviation; e.g., "L" for
    // Latin letters.  NULL if missing.

extern double const numerator[];
extern double const denominator[];
extern double const numeric_value[];
    // For a character c with a numeric value,
    //    numerator[index[c]]
    //    denominator[index[c]]
    //    numeric_value[index[c]]
    // give the numeric value of c.
    //
    // The numerator and denominator are integers, but
    // because the numerator may be as large as 1e12,
    // these are encoded as double's.
    //
    // If a character has NO numeric value, the
    // denominator is 0 and the numeric_value is NaN.
    //
    // Otherwise the denominator is > 0, the numerator
    // may be signed, and the numeric value is the ratio
    // of numerator/denominator (e.g., 1/3'rd is given
    // as the C++ constant expression `1.0/3' to
    // provide maximum accuracy).

extern char const bidi_mirrored[];
    // bidi_mirrored[index[c]] is the UNICODE Bidi
    // Mirrored Indicator of c, 'Y' or 'N' or 0 if
    // missing.

extern uns64 const properties[];
    // properties[index[c]] & ( 1 << P ) is true if
    // c has property P, where P is one of the
    // following (the items in this table and their
    // order may change in the future):

// Cannot use enum for the following because they
// are 64 bits and enum may only be 32 bits.
//
const uns64 ASCII_Hex_Digit                    = 0;
const uns64 Bidi_Control                       = 1;
const uns64 Dash                               = 2;
const uns64 Deprecated                         = 3;
const uns64 Diacritic                          = 4;
const uns64 Extender                           = 5;
const uns64 Hex_Digit                          = 6;
const uns64 Hyphen                             = 7;
const uns64 Ideographic                        = 8;
const uns64 IDS_Binary_Operator                = 9;
const uns64 IDS_Trinary_Operator               = 10;
const uns64 Join_Control                       = 11;
const uns64 Logical_Order_Exception            = 12;
const uns64 Noncharacter_Code_Point            = 13;
const uns64 Other_Alphabetic                   = 14;
const uns64 Other_Default_Ignorable_Code_Point = 15;
const uns64 Other_Grapheme_Extend              = 16;
const uns64 Other_ID_Continue                  = 17;
const uns64 Other_ID_Start                     = 18;
const uns64 Other_Lowercase                    = 19;
const uns64 Other_Math                         = 20;
const uns64 Other_Uppercase                    = 21;
const uns64 Pattern_Syntax                     = 22;
const uns64 Pattern_White_Space                = 23;
const uns64 Prepended_Concatenation_Mark       = 24;
const uns64 Regional_Indicator                 = 25;
const uns64 Quotation_Mark                     = 26;
const uns64 Radical                            = 27;
const uns64 Sentence_Terminal                  = 28;
const uns64 Soft_Dotted                        = 29;
const uns64 STerm                              = 30;
const uns64 Terminal_Punctuation               = 31;
const uns64 Unified_Ideograph                  = 32;
const uns64 Variation_Selector                 = 33;
const uns64 White_Space                        = 34;

extern ustring const name[];
    // name[index[c]] is the ustring name of c, or NULL
    // if c has no name.  For example, character code
    // 0A has as ustring name "\x02\x02" "LF" (where
    // the two strings are concatenated, which is done
    // to prevent the first character of the second
    // from being interpreted as a hexadecimal digit).
    //
    // Names are `alias'es read from NameAliases.txt.
    // NULL if missing.

struct extra_name
{
    ustring name;
    Uchar c;
};
extern extra_name const extra_names[];
extern uns32 const extra_names_size;
extern uns32 const extra_names_number;
    // A few characters have more than one name.  For
    // these an entry is made in this table for the
    // 2nd, 3rd, ... name of a character.  The size
    // of extra_names is extra_names_size and the
    // actual number of extra_names is extra_names_
    // number.

extern ustring const picture[];
    // picture[index[c]] is the ustring picture name of
    // c, or NULL if c has no picture name.  E.g.,
    // character code 0A has as picture the ustring
    // "\x03\x01\xE2\x90\x8A", which encodes 240A,
    // the LF `control picture' UNICODE character.
    // NULL if missing.
    //
    // Currently all pictures are UNICODE control
    // pictures and are assigned as follows:
    //
    //     c < 0x20         ---> 0x2400 + c
    //     0x20	            ---> 0x2423     SP
    //     0xA0	            ---> 0x2422     NBSP
    //     0x7F	    	    ---> 0x2421     DEL
    //     SOFTWARE_NL	    ---> 0x2424     NL

extern ustring const html_reserved[];
    // html_reserved[index[c]] is the ustring used as
    // the entity that represents c in HTML.  NULL
    // if missing.
    //
    // Currently all html_reserved values are:
    //
    //     &        0x0026     ---> "&amp;"
    //     <	    0x003C     ---> "&lt;"
    //     >	    0x003E     ---> "&gt;"
    //     "        0x0022     ---> "&quot;"
    //     <NBSP>   0x00A0     ---> "&nbsp;"
    //     –        0x2013     ---> "&ndash;"   en dash
    //     —        0x2014     ---> "&mdash;"   em dash
    //     ©        0x00A9     ---> "&copy;"
    //     ®        0x00AE     ---> "&reg;"
    //     ™        0x2122     ---> "&trade;"
    //     ≈        0x2248     ---> "&asymp;"
    //     ≠        0x2260     ---> "&ne;"
    //     £        0x00A3     ---> "&pound;"
    //     €        0x20AC     ---> "&euro;"
    //     °        0x00B0     ---> "&deg;"

extern uns32 const support_sets[];
    // support_sets[index[c]] & ( 1 << k ) != 0 if
    // and only if c is in support set s where
    //
    //     "s" == ss_support_sets_name[i]
    //      k  == ss_support_sets_shift[i]
    //
    // See discussion of support sets above.

extern uns32 const reference_count[];
    // reference_count[i] is the number of character
    // codes c with i == index[c].  This is just for
    // integrity checking purposes.
