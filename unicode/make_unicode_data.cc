// MIN Data System Program to Make UNICODE Type Tables 
//
// File:	make_unicode_data.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan  6 00:10:51 EST 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <iostream>
# include <iomanip>
# include <fstream>
# include <cstdlib>
# include <cstdarg>
# include <climits>
# include <cctype>
# include <cstring>
# include <cassert>
using std::cout;
using std::endl;
using std::setw;
using std::hex;
using std::dec;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::setiosflags;
using std::ios_base;

# define index INDEX
    // Avoid use of `index' in <cstring>

namespace unicode {

#   include "unicode_data.h"

}
using unicode::UNKNOWN_UCHAR;
using unicode::SOFTWARE_NL;
using unicode::Uchar;
using unicode::uns64;
using unicode::uns32;
using unicode::uns16;
using unicode::uns8;
using unicode::NO_UCHAR;
using unicode::ustring;
using unicode::ustring_length;
using unicode::ustring_columns;
using unicode::ustring_chars;
using unicode::utf8_to_unicode;
using unicode::unicode_to_utf8;

// For each of the extern'ed data in unicode_data.h we
// have a corresponding datum here with no `const' and
// with vectors of size index_size instead index_limit
// and of max_support_sets instead of ss/cc_support_
// sets_size.
//
// WARNING: This means that indexes are uns32's and
// NOT uns16's in this program only.

const uns32 max_support_sets = 4096;
    // This is so large its ridiculous, and should
    // never been exceeded.
const char * ss_support_sets_name[max_support_sets];
uns8 ss_support_sets_shift[max_support_sets];
uns32 ss_support_sets_size = 0;
const char * cc_support_sets_name[max_support_sets];
uns32 cc_support_sets_mask[max_support_sets];
uns32 cc_support_sets_size = 0;

const Uchar index_size = 0x30001;
uns32 index[index_size];
uns32 index_limit = 0;

Uchar character[index_size];
const char * category[index_size];
short combining_class[index_size];
const char * bidi_class[index_size];
double numerator[index_size];
double denominator[index_size];
double numeric_value[index_size];
char bidi_mirrored[index_size];
uns64 properties[index_size];
const ustring * name[index_size];
const ustring * picture[index_size];
uns32 support_sets[index_size];
uns32 reference_count[index_size];

# include "output_unicode_data.cc"
    // Includes:
    //
    //		property_name
    //		property_name_size
    // 		eq
    // 		ustring_eq
    // 		index_eq

// Print error message mentioning file, line number,
// and printing the current line.
//
void line_error ( const char * format, ... );

// Copy table entries for index[.] value i2 to entries
// for index[.] value i1.
//
inline void index_copy ( uns32 i1, uns32 i2 )
{
    assert ( i1 <= i2 );
    category[i1] = category[i2];
    combining_class[i1] = combining_class[i2];
    bidi_class[i1] = bidi_class[i2];
    numerator[i1] = numerator[i2];
    denominator[i1] = denominator[i2];
    numeric_value[i1] = numeric_value[i2];
    bidi_mirrored[i1] = bidi_mirrored[i2];
    properties[i1] = properties[i2];
    name[i1] = name[i2];
    picture[i1] = picture[i2];
    support_sets[i1] = support_sets[i2];
}

// Initialize tables.  category[0] is set to 'w'
// and 0 becomes the index for characters not
// assigned any category, name, or numeric value.
//
void initialize ( void )
{
    for ( Uchar c = 0; c < index_size; ++ c )
    {
	index[c] = c;
        reference_count[c] = 1;
	combining_class[c] = -1;
	numeric_value[c] = NAN;
    }
}

// Construct index.  Compute `character' data base
// vector.
//
void finalize ( void )
{
    index_limit = 0;
    for ( Uchar c = 0; c < index_size; ++ c )
    {
        if ( c <= 0xFF )
	{
	    assert ( index[c] == c );
	    ++ index_limit;
	    continue;
	}

	uns32 i;
	for ( i = 0x100; i < index_limit; ++ i )
	{
	    if ( index_eq ( i, c ) ) break;
	}

	if ( i == index_limit )
	{
	    index_copy ( index_limit, c );
	    -- reference_count[c];
	    ++ reference_count[index_limit];
	    assert
	        ( reference_count[index_limit] == 1 );
	    index[c] = index_limit ++;
	}
	else
	{
	    ++ reference_count[i];
	    -- reference_count[c];
	    assert ( reference_count[c] == 0 );
	    index[c] = i;
	}
    }

    uns32 i = index[index_size-1];
    assert ( ::category[i] == NULL );
    assert ( ::combining_class[i] == -1 );
    assert ( ::bidi_class[i] == NULL );
    assert ( ::denominator[i] == 0 );
    assert ( ::bidi_mirrored[i] == 0 );
    assert ( ::properties[i] == 0 );
    assert ( ::name[i] == NULL );
    assert ( ::picture[i] == NULL );
    assert ( ::support_sets[i] == 0 );


    for ( Uchar c = 0; c < index_size; ++ c )
        character[index[c]] = c;
    for ( uns32 i = 0; i < index_limit; ++ i )
    {
        if ( reference_count[i] != 1 )
	    character[i] = NO_UCHAR;
    }
}

// Store UNICODE name for a character.  Complain if
// character has previously been given a different name.
// Complain if name already used by another character.
//
// The name n must be a string of ASCII graphic charac-
// ters presented as a const char * string.  This is
// converted to a ustring.  This allows more complex
// names with combining diacritics, but currently these
// are not used by the UNICODE standard.
//
void store_name ( Uchar c, const char * n )
{
    unsigned length = strlen ( n );
    assert ( length > 0 );
    assert ( length < 256 );
    ustring buffer[length+3];
    buffer[0] = buffer[1] = length;
    memcpy ( buffer + 2, n, length );
    buffer[length+2] = 0;

    if ( ustring_eq ( name[c], buffer ) ) return;

    if ( name[c] != NULL )
    {
	line_error ( "character %02X was previously"
	             " assigned name %s; line ignored",
		     c, ustring_chars ( name[c] ) );
	return;
    }

    name[c] = (ustring *) strdup ( (char *) buffer );

    // Check for name being used more than once.
    //
    for ( Uchar c2 = 0; c2 < index_size; ++ c2 )
    {
	if ( ! ustring_eq ( name[c], name[c2] ) )
	    continue;
	if ( c == c2 ) continue;

	line_error ( "character name `%s' used more"
		     " than once; line accepted", n );
	break;
    }
}

// Current input data:
//
char const * file;
ifstream in;
const unsigned line_size = 500;
    // Derived file lines are automatically generated
    // and can be long.
// char line[line_size+2];
    // Input line.
    // Imported from output_unicode_data.cc.
unsigned line_count;

// Print error message composed as printf would
// and following with current file name, line number,
// and line.
//
void line_error ( const char * format, ... )
{
    va_list args;
    va_start ( args, format );
    char buffer[200];
    vsprintf ( buffer, format, args );

    cout << "ERROR: " << buffer << endl
         << "       in line " << line_count
	 << " of " << file
	 << ":" << endl << line << endl;
}

// Open file.  Print error message and exit on failure.
//
void open ( char const * file )
{
    ::file = file;
    line_count = 0;
    in.open ( file );
    if ( ! in )
    {
        cout << "ERROR: cannot open " << file
	     << " for reading" << endl;
	exit ( 1 );
    }
}

// Close file.  Print error message and exit if file
// not at EOF.
//
void close ( void )
{
    if ( ! in.eof() )
    {
        cout << "ERROR: cannot read line "
	     << line_count << " of " << file << endl;
	exit ( 1 );
    }
    in.close();
}


// Read next input line into `line'.  Return true on
// success, false on end of file.  Print error if line
// line too long and truncate line.
//
bool read_line ( void )
{
    assert ( sizeof ( line ) >= line_size + 2 );
    ++ line_count;
    while ( in.getline ( line, sizeof ( line ) ),
            ! in.eof() )
    {
        line[line_size+1] = 0;
        if ( strlen ( line ) > line_size )
	{
	    line_error
	        ( "line too long; line ignored" );
	    in.clear();
	    continue;
	}
	return true;
    }

    return false;
}

// Given a pointer to a character in a line, return a
// pointer to the next non-whitespace character.  Skip
// `#' comments and continue until next character is
// NUL.
//
const char * skip_whitespace ( const char * line )
{
    while ( isspace ( * line ) ) ++ line;
    if ( * line == '#' ) while ( * line ) ++ line;
    return line;
}

// Given a pointer to a character in a line, skip to
// the next field, or end of line if none.
//
const char * skip_to_next ( const char * line )
{
    while ( * line && * line != ';' && * line != '#' )
        ++ line;
    if ( * line == '#' ) while ( * line ) ++ line;
    if ( * line ) ++ line;
    return line;
}

// Given a pointer to a non-whitespace character in a
// line, read a character range.  Return a pointer to
// just after the range, or NULL if no range found or
// range bound larger than 0xFFFFFFF read.
//
const char * read_range
	( const char * line,
	  Uchar & low, Uchar & high )
{
    char * p;
    unsigned long n = strtoul ( line, & p, 16 );
    if ( p == line ) return NULL;
    if ( n > 0xFFFFFFF ) return NULL;
    low = (Uchar) n;
    line = p;
    if ( line[0] == '.' && line[1] == '.' )
    {
        line += 2;
	n = strtoul ( line, & p, 16 );
	if ( p == line ) return NULL;
	if ( n > 0xFFFFFFF ) return NULL;
	high = (Uchar) n;
	if ( high < low ) return NULL;
	line = p;
    }
    else high = low;

    return line;
}

// Ditto but call read_line to get line until a line
// with a range is found.  Return true if such a line
// was found and set `low' and `high' to the range.
// Print error messages for lines with bad ranges.
// Return false at end of file.
//
bool read_range ( Uchar & low, Uchar & high )
{
    while ( read_line() )
    {
        const char * p = line;
	p = skip_whitespace ( p );
	if ( * p == 0 )
	    continue;

	p = read_range ( p, low, high );
	if ( p == NULL )
	{
	    line_error ( "bad range; line ignored" );
	    continue;
	}

	return true;
    }
    return false;
}

// Check range to be sure hi < index_size - 1.  If
// yes, do nothing.  If no, print error message if
// print_on_error is true, and reset `high' to
// index_size in any case.
//
// Return `low <= high' after any resetting of `high'.
//
bool check_range ( Uchar low, Uchar & high,
                   bool print_on_error = true )
{
    if ( high >= index_size - 1 )
    {
	high = index_size - 2;
	if ( print_on_error )
	{
	    if ( low <= high )
		line_error ( "code above %02X used;"
			     " range reset to"
			     " %02X..%02X",
			     high, low, high );
	    else
		line_error ( "code above %02X used;"
			     " line ignored",
			     index_size - 1 );
	}
    }
    return low <= high;
}

// Read current field into `field'.  Return true if
// success, false if failure.  Note that skip_to_next
// must be called after reading this field to get to the
// next field.
//
// Field may contain whitespace.  Whitespace at ends of
// field is trimmed (so all whitespace field is == "").
//
char field[line_size+1];
char * subfieldp;
bool read_field ( const char * line )
{
    line = skip_whitespace ( line );
    if ( * line == 0 ) return false;
    const char * p = line;
    while ( * p && * p != ';' && * p != '#' )
        ++ p;
    while ( p > line && isspace ( p[-1] ) ) -- p;
    strncpy ( field, line, p - line );
    field[p - line] = 0;
    subfieldp = field;
    return true;
}

// Get next subfield inside field.  Subfields are
// separated by whitespace.  Upon return the
// subfield ends in NUL and is trimmed of whitespace.
//
// Return NULL if there is no next subfield.
//
const char * get_next_subfield ( void )
{
    subfieldp = (char *) skip_whitespace ( subfieldp );
    const char * returnp = subfieldp;
    while ( * subfieldp && ! isspace ( * subfieldp ) )
        ++ subfieldp;
    if ( subfieldp == returnp ) return NULL;
    if ( * subfieldp ) * subfieldp ++ = 0;
    return returnp;
}

// Get next field.  Return true if found, false if not
// found.  If not found, output error message saying
// "<name> field not found; line ignored".
//
bool get_next_field
    ( const char * & p, const char * name )
{
    p = skip_to_next ( p );
    if ( ! read_field ( p ) )
    {
	line_error ( "%s field not found;"
	             " line ignored", name );
	return false;
    }
    return true;
}

// Find an cc_support set and return its mask.  Return
// 0 if not found.
//
uns32 find_cc_support_set ( const char * name )
{
    uns32 j;
    for ( j = 0; j < cc_support_sets_size; ++ j )
    {
	if ( eq ( name, cc_support_sets_name[j] ) )
	    break;
    }
    return j == cc_support_sets_size ? 0 :
    	   cc_support_sets_mask[j];
}

// Read NameAliases.txt
//
void read_names ( void )
{
    ::open ( "NameAliases.txt" );

    Uchar low, high;
    while ( read_range ( low, high ) )
    {
        if ( ! check_range ( low, high, false ) )
	    continue;

        const char * p = line;
	p = skip_to_next ( p );
	const char * q = skip_to_next ( p );
	if ( ! read_field ( q ) )
	{
	    line_error ( "second field not found;"
	                 " line ignored" );
	    continue;
	}
	if ( ! eq ( field, "abbreviation" ) )
	    continue;
	if ( ! read_field ( p ) )
	{
	    line_error ( "first field not found;"
	                 " line ignored" );
	    continue;
	}
	if ( low < high )
	{
	    line_error ( "multiple characters given the"
	                 " same name; line ignored" );
	    continue;
	}
	else if ( low > high )
	{
	    line_error ( "no characters given this"
	                 " name; line ignored" );
	    continue;
	}

	store_name ( low, field );
    }

    ::close();
}

// Read SupportSets.txt
//
void read_support_sets ( void )
{
    ::open ( "SupportSets.txt" );

    while ( read_line() )
    {
        const char * p = line;
	p = skip_whitespace ( p );
	if ( * p == 0 ) continue;

	if ( ! read_field ( p ) )
	{
	    line_error ( "first field not found;"
			 " line ignored" );
	    continue;
	}

	uns32 i;
	for ( i = 0; i < ss_support_sets_size; ++ i )
	{
	    if ( eq ( field, ss_support_sets_name[i] ) )
	        break;
	}
	if ( i < ss_support_sets_size )
	{
	    line_error ( "ss support set (%s)"
	                 " duplicated; line ignored",
			 field );
	    continue;
	}
	if ( ss_support_sets_size == max_support_sets )
	{
	    line_error ( "too many ss support sets;"
	                 " line ignored" );
	    continue;
	}

	i = ss_support_sets_size;
	ss_support_sets_name[i] = strdup ( field );

	if ( ! get_next_field ( p, "second" ) )
	    continue;
	char * q;
	unsigned long shift =
	    strtoul ( field, & q, 10 );
	if (    ! isdigit ( field[0] ) || * q != 0
	     || shift >= 32 )
	{
	    line_error ( "bad second field (shift);"
	                 " line ignored" );
	    continue;
	}

	if ( ! get_next_field ( p, "third" ) )
	    continue;

	// From this point on line will not be ignored.
	//
	ss_support_sets_shift[i] = shift;
	++ ss_support_sets_size;

	const char * subfield;
	while ( ( subfield = get_next_subfield() ) )
	{
	    uns32 j;
	    for ( j = 0; j < cc_support_sets_size; ++ j )
	    {
		if ( eq ( subfield,
		          cc_support_sets_name[j] ) )
		    break;
	    }
	    if ( j == cc_support_sets_size )
	    {
		if (    cc_support_sets_size
		     == max_support_sets )
		{
		    line_error ( "too many cc support"
		                 " sets; support set %s"
				 " ignored in line",
				 subfield );
		    continue;
		}
		cc_support_sets_name[j] =
		    strdup ( subfield );
		cc_support_sets_mask[j] = 0;
		++ cc_support_sets_size;
	    }

	    cc_support_sets_mask[j] |= 1 << shift;
	}
    }

    ::close();
}


// Read CompositeCharacters.txt
//
void read_composite_characters ( void )
{
    ::open ( "CompositeCharacters.txt" );

    Uchar low, high;
    while ( read_range ( low, high ) )
    {
        if ( ! check_range ( low, high ) )
	    continue;

        const char * p = line;

	if ( ! get_next_field ( p, "first" ) )
	    continue;
	if ( ! get_next_field ( p, "second" ) )
	    continue;
	if ( ! get_next_field ( p, "third" ) )
	    continue;

	uns32 support_sets = 0;
	const char * subfield;
	while ( ( subfield = get_next_subfield() ) )
	    support_sets |=
	        find_cc_support_set ( subfield );

	for ( Uchar c = low; c <= high; ++ c )
	    ::support_sets[c] |= support_sets;
    }

    ::close();
}

// Read UnicodeData.txt
//
void read_unicode_data ( void )
{
    ::open ( "UnicodeData.txt" );

    Uchar low, high;
    while ( read_range ( low, high ) )
    {
        if ( ! check_range ( low, high, false ) )
	    continue;

        const char * p = line;

	if ( ! get_next_field ( p, "first" ) )
	    continue;
	if ( ! get_next_field ( p, "second" ) )
	    continue;
	const char * category =
	    eq ( field, "" ) ? NULL : strdup ( field );

	if ( ! get_next_field ( p, "third" ) )
	    continue;
	long long combining_class = -1;
	if ( ! eq ( field, "" ) )
	{
	    combining_class = atoll ( field );
	    const char * p = field;
	    while ( isdigit ( * p ) ) ++ p;
	    if ( * p != 0 || combining_class < 0
	                  || combining_class >= 256 )
	    {
		line_error ( "combining class (third"
		             " field) illegal;"
			     " line ignored" );
		continue;
	    }
	}

	if ( ! get_next_field ( p, "fourth" ) )
	    continue;
	const char * bidi_class =
	    eq ( field, "" ) ? NULL : strdup ( field );

	if ( ! get_next_field ( p, "fifth" ) )
	    continue;
	if ( ! get_next_field ( p, "sixth" ) )
	    continue;
	double n1 = -1;
	if ( ! eq ( field, "" ) )
	{
	    n1 = atof ( field );
	    char * p = field;
	    while ( isdigit ( * p ) ) ++ p;
	    if ( * p != 0 || n1 < 0 || n1 > 9 )
	    {
		line_error ( "decimal value (sixth"
		             " field) illegal;"
			     " line ignored" );
		continue;
	    }
	}

	if ( ! get_next_field ( p, "seventh" ) )
	    continue;
	double n2 = -1;
	if ( ! eq ( field, "" ) )
	{
	    n2 = atof ( field );
	    char * p = field;
	    while ( isdigit ( * p ) ) ++ p;
	    if ( * p != 0 || n2 < 0 || n2 > 9 )
	    {
		line_error ( "digit value (seventh"
		             " field) illegal;"
			     " line ignored" );
		continue;
	    }
	}

	if ( ! get_next_field ( p, "eighth" ) )
	    continue;
	double d = 0;
	double n = 0;
	if ( ! eq ( field, "" ) )
	{
	    n = atof ( field );
	    char * p = field;
	    if ( * p == '-' ) ++ p;
	    while ( isdigit ( * p ) ) ++ p;
	    char * q = p;
	    if ( * p == 0 ) d = 1;
	    else
	    {
	        if ( * q == '/' )
		{
		    * q ++ = 0;
		    d = atof ( q );
		    while ( isdigit ( * q ) ) ++ q;
		}
	    }
	    if ( * p != 0 || * q != 0 ||
	         d < 1 || d > 1e15 ||
		 n < -1e15 || n > 1e15 )
	    {
		line_error ( "numeric value (eighth"
		             " field) illegal;"
			     " line ignored" );
		continue;
	    }
	}

	if ( n1 != -1 && ( d != 1 || n != n1 ) )
	{
	    line_error ( "decimal value (sixth field)"
			 " does not equal numeric"
			 " value (eighth field);"
			 " line ignored" );
	    continue;
	}
	if ( n2 != -1 && ( d != 1 || n != n2 ) )
	{
	    line_error ( "digit value (seventh field)"
			 " does not equal numeric"
			 " value (eighth field);"
			 " line ignored" );
	    continue;
	}

	if ( ! get_next_field ( p, "ninth" ) )
	    continue;
	if (    ! eq ( field, "" )
	     && ! eq ( field, "N" )
	     && ! eq ( field, "Y" ) )
	{
	    line_error ( "bidi mirrored value"
	                 " (ninth field) is not"
			 " legal;"
			 " line ignored" );
	    continue;
	}
	const char bidi_mirrored = field[0];

	for ( Uchar c = low; c <= high; ++ c )
	{
	    ::category[c] = category;
	    ::combining_class[c] = combining_class;
	    ::bidi_class[c] = bidi_class;
	    if ( d > 0 )
	    {
		::numerator[c] = n;
		::denominator[c] = d;
		::numeric_value[c] = n / d;
	    }
	    ::bidi_mirrored[c] = bidi_mirrored;
	}
    }

    ::close();
}

// Read PropList.txt
//
void read_prop_list ( void )
{
    ::open ( "PropList.txt" );

    Uchar low, high;
    while ( read_range ( low, high ) )
    {
        if ( ! check_range ( low, high, false ) )
	    continue;

        const char * p = line;

	if ( ! get_next_field ( p, "first" ) )
	    continue;

	unsigned i;
	for ( i = 0; i < property_name_size; ++ i )
	{
	    if ( eq ( property_name[i], field ) )
	        break;
	}

	if ( i == property_name_size )
	{
	    line_error ( "unknown property name (%s);"
			 " line ignored", field );
	    continue;
	}

	for ( Uchar c = low; c <= high; ++ c )
	    properties[c] |= ( 1ull << i );
    }

    ::close();
}

// support_sets[c] is set to include "ascii" for
// c <= 0x7F, and to include "special" for UNKNOWN_UCHAR
// and SOFTWARE_NL.
//
void set_support_sets ( void )
{
    uns32 special_ss =
        find_cc_support_set ( "special" );

    support_sets[UNKNOWN_UCHAR] |= special_ss;
    support_sets[SOFTWARE_NL] |= special_ss;

    uns32 ascii_ss =
        find_cc_support_set ( "ascii" );

    for ( Uchar c = 0; c <= 0x7F; ++ c )
        support_sets[c] |= ascii_ss;
}

// picture[c] is set for 0 <= c <= 0x20, c == 0x7F,
// c == 0xA0 (NBSP), c == SOFTWARE_NL, c == UNKNOWN_UCHAR.
//
void set_pictures ( void )
{
    for ( Uchar c = 0; c <= UNKNOWN_UCHAR; )
    {
        Uchar pic =
	    c  < 0x20 ? c + 0x2400 :  // NUL ... US
	    c == 0x20 ? 0x2423 :     // SP
	    c == 0x7F ? 0x2421 :     // DEL
	    c == 0xA0 ? 0x2422 :     // NBSP
	    c == SOFTWARE_NL ? 0x2424 :
	    c == UNKNOWN_UCHAR ? 0x2425 :
		 UNKNOWN_UCHAR;


	char buffer[11];
	char * s = buffer + 2;
	unsigned length =
	    unicode_to_utf8 ( s, pic );
	buffer[0] = length;
	buffer[1] = 1;
	buffer[length+2] = 0;
	picture[c] = (ustring *) strdup ( buffer );

	if ( c == 0x20 ) c = 0x7F;
	else if ( c == 0x7F) c = 0xA0;
	else if ( c == 0xA0) c = SOFTWARE_NL;
	else if ( c == SOFTWARE_NL) c = UNKNOWN_UCHAR;
	else ++ c;
    }
}

int main ( int argc, const char ** argv )
{
    assert ( sizeof(Uchar) == 4 );
    assert ( sizeof(uns64) == 8 );
    assert ( sizeof(uns32) == 4 );
    assert ( sizeof(uns16) == 2 );
    assert ( sizeof(uns8)  == 1 );

    cout << setiosflags ( ios_base::uppercase );

    initialize();

    read_names();
    read_support_sets();
    read_composite_characters();
    read_unicode_data();
    read_prop_list();

    
    ::file = "Builtin Defaults";
    ::line_count = 0;
    strcpy ( ::line, "set builtin defaults" );

    store_name ( SOFTWARE_NL, "NL" );
    store_name ( UNKNOWN_UCHAR, "UUC" );

    set_support_sets();
    set_pictures();

    finalize();
    final_check();
    if ( argc > 1 ) output_data ( argv[1] );
    if ( argc > 2 ) dump ( argv[2] );
    if ( argc > 3 ) output_support_sets ( argv[3] );
}
