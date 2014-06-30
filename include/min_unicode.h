// MIN UNICODE Data
//
// File:	min_unicode.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun 30 16:19:45 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

namespace min { namespace unicode {

typedef min::uns32 Uchar;
    // Must be 32 bit integer.

extern const unsigned unicode_index_size;
extern const unsigned short unicode_index[];
    // Index table.
    //
    // unicode_index[c] is the index of c.  Indices are
    // very changeable, and may not be the same week
    // to week (unlike the category of c which should
    // never change).
    //
    // unicode_types[unicode_index[c]] is the type of c.

extern const unsigned unicode_types_size;
struct unicode_type
    // unicode_types[unicode_index[c]] is the type c.
{
    unsigned char category;
        // Character category.  See the unicode_
	// categories table for general categories.
	// However, as an exception, characters c < 256
	// are their own categories (i.e., the category
	// of `+' is `+') if they are not a letter or
	// digit and do not have a name (i.e., are not
	// SP or DEL, etc.).

    unsigned short name;
    unsigned char name_length;
    unsigned char name_columns;
        // If name != 0, then the character has a name
	// that begins with unicode_names[name] and
	// has name_length unicode characters that take
	// name_columns print columns when printed
	// consecutively.  Examples are HT, SP, LF, DEL;
	// e.g., control characters and space characters
	// have names.  Names are assigned by the
	// UNICODE standard and are NOT made up.

    signed long long numerator, denominator;
        // For number characters, the numeric value of
	// the character as numerator/denominator.  For
	// integer values the denominator is 1.  The
	// numerator may be negative or 0, but the
	// denominator must be > 0.
	//
	// Some languages have numeric values of one
	// trillion.

    unsigned reference_count;
        // Number of index entries pointing at this
	// type.  Just used for integrity checking.

};
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
