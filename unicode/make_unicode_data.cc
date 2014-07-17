// MIN Data System Program to Make UNICODE Type Tables 
//
// File:	make_unicode_data.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jul 17 02:28:32 EDT 2014
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

# include "unicode_data.h"

const unsigned index_limit_max = 512;
    // Upper bound on unicode_index_limit; used to allo-
    // cate vectors before the value of unicode_max_
    // index is computed.

// For each of the extern'ed data in unicode_data.h we
// have a corresponding datum here with no `const' and
// no `unicode_' preface and for vectors often with size
// XXX_limit in place of XXX.
//
const unsigned index_size = 0x30000;
unsigned index_limit = 0;
unsigned short index[index_size];

unsigned char category[index_limit_max];
const ustring * name[index_limit_max];
const ustring * picture[index_limit_max];
double numerator[index_limit_max];
double denominator[index_limit_max];
double numeric_value[index_limit_max];
unsigned reference_count[index_limit_max];

const unsigned supported_set_limit = 16;
const unsigned supported_set_shift = 12;
const unsigned short index_mask = unicode_index_mask;
const char * supported_set[supported_set_limit] = {
    /*  0 */ "ascii",
    /*  1 */ "latin1"
};

const unsigned category_limit = 256;
const char * category_name[category_limit];
const char * category_description[category_limit];

// For each of the extern'ed data in unicode_data.h
// that is needed by output_unicode_data.cc we map
// the extern'ed datum name to the datum above in the
// following code, but NOT in the preceding code.
// This ploy is needed to make output_unicode_data.cc
// work for us.
//
# define unicode_index_limit index_limit
# define unicode_index index
# define unicode_index_size index_size
# define unicode_category category
# define unicode_name name
# define unicode_picture picture
# define unicode_numerator numerator
# define unicode_denominator denominator
# define unicode_numeric_value numeric_value
# define unicode_reference_count reference_count
# define unicode_category_name category_name
# define unicode_category_description \
		category_description
# define unicode_supported_set_limit supported_set_limit
# define unicode_supported_set_shift supported_set_shift
# define unicode_supported_set supported_set

# include "output_unicode_data.cc"

// Table from which we generate the unicode_catagory_
// name and unicode_category_description vectors, and
// which we use to map UNICODE general category names
// to our categories.
//
// The UNICODE general category is 2 letters, while our
// category is 1 letter and is better suited to making
// lookup-by-category tables.  We also add the special
// category `w' for characters with no UNICODE category.
//
struct category_datum
{
    const char * category_name;
        // UNICODE category name; e.g., "Lu".
	// NULL for the unspecified category 'w'.
    unsigned char category;
        // Our category.  E.g. 'U'.
    const char * category_description;
        // UNICODE category description; e.g.,
	// "Upper Case Letter".
} category_data[] = {
    { "Lu", 'U', "Upper Case Letter" },
    { "Ll", 'L', "Lower Case Letter" },
    { "Lt", 'T', "Title Case Letter" },
    { "Lm", 'M', "Modifier Letter" },
    { "Lo", 'O', "Other Letter" },

    { "Mn", 'N', "Nonspacing Mark" },
    { "Mc", 'S', "Spacing Mark" },
    { "Me", 'E', "Enclosing Mark" },

    { "Nd", 'D', "Decimal Number" },
    { "Nl", 'R', "Letter Number" },
    { "No", 'H', "Other Number" },

    { "Pc", 'n', "Connector Punctuation" },
    { "Pd", 'd', "Dash Punctuation" },
    { "Ps", 'o', "Open Punctuation" },
    { "Pe", 'c', "Close Punctuation" },
    { "Pi", 'i', "Initial Punctuation" },
    { "Pf", 'f', "Final Punctuation" },
    { "Po", 'h', "Other Punctuation" },

    { "Sm", 'm', "Math Symbol" },
    { "Sc", 'y', "Currency Symbol" },
    { "Sk", 'r', "Modifier Symbol" },
    { "So", 't', "Other Symbol" },

    { "Zs", 's', "Space Separator" },
    { "Zl", 'l', "Line Separator" },
    { "Zp", 'p', "Paragraph Separator" },

    { "Cc", 'x', "Control" },
    { "Cf", 'z', "Format" },
    { "Cs", 'g', "Surrogate" },
    { "Co", 'v', "Private Use" },

    { "Cn", 'u', "Unassigned" },
    { NULL, 'w', "Unspecified" },
    { NULL, 'e', "Software New Line" }
};
unsigned category_data_size =
      sizeof ( category_data )
    / sizeof ( category_datum );

// Print error message mentioning file, line number,
// and printing the current line.   If fatal is true,
// exit program.
//
void line_error ( const char * format, ... );

// Allocate a new index and set index[c] to the new
// index.  Return the new index.
//
unsigned new_index ( Uchar c )
{
    assert ( index_limit < index_limit_max );

    unsigned i = unicode_index[c];
    -- reference_count[i];
    i = index_limit ++;
    index[c] = i;

    category[i] = UNSPECIFIED_CATEGORY;
    name[i] = NULL;
    picture[i] = NULL;
    numerator[i] = denominator[i] = 0;
    numeric_value[i] = NAN;
    reference_count[i] = 1;
    return i;
}

// Initialize tables.  category[0] is set to 'w'
// and 0 becomes the index for characters not
// assigned any category, name, or numeric value.
//
void initialize ( void )
{
    assert ( new_index ( 0 ) == 0 );
    reference_count[0] = index_size;

    for ( unsigned i = 0; i < category_data_size; ++ i )
    {
	category_datum & d = category_data[i];
	unsigned char cat = d.category;
        assert ( category_description[cat] == NULL );

        category_name[cat] = d.category_name;
        category_description[cat] =
	    d.category_description;
    }
}

// Finalize tables.  picture[index[c]] is set for
// 0 <= c <= 0x20, c == 0x7F, and c == SOFTWARE_NL.
//
void finalize ( void )
{
    unsigned i = 0; 
    for ( Uchar c = 0; c <= SOFTWARE_NL; )
    {
        Uchar pic =
	    c  < 0x20 ? c + 0x2400 :  // NUL ... US
	    c == 0x20 ? 0x2423 :     // SP
	    c == 0x7F ? 0x2421 :     // DEL
	    c == SOFTWARE_NL ? 0x2424 :
		 UNKNOWN_UCHAR;


	char buffer[11];
	char * s = buffer + 2;
	unsigned length =
	    unicode_to_utf8 ( s, pic );
	buffer[0] = length;
	buffer[1] = 1;
	buffer[length+2] = 0;
	picture[index[c] & index_mask] =
	    (ustring *) strdup ( buffer );

	i += 2;
	if ( c == 0x20 ) c = 0x7F;
	else if ( c == 0x7F) c = SOFTWARE_NL;
	else ++ c;
    }
}


// Store UNICODE name for a character.  Complain if
// character has previously been given a name.  Complain
// if name already used by another character.
//
// The name n must be a string of ASCII graphic charac-
// ters presented as a const char * string.  This is
// converted to a ustring.  This allows more complex
// names with combining diacritics, but currently these
// are not used by the UNICODE standard.
//
void store_name ( Uchar c, const char * n )
{
    assert ( c < index_size );

    unsigned i = index[c];

    if ( i != 0 )
    {
	assert ( name[i] != NULL );
	    // Names should be set before number values
	    // and categories.
	line_error ( "character %02X previously"
	             " assigned the name `%s'; line"
	             " ignored", c,
		     ustring_chars ( name[i] ) );
	return;
    }

    unsigned name_length = strlen ( n );
    assert ( name_length > 0 );

    // Allocate new index.
    //
    i = new_index ( c );

    // Set name[i].
    //
    assert ( name_length < 256 );
    ustring buffer[name_length+3];
    buffer[0] = buffer[1] = name_length;
    memcpy ( buffer + 2, n, name_length );
    buffer[name_length+2] = 0;

    name[i] = (ustring *) strdup ( (char *) buffer );

    // Check for name being used more than once.
    //
    for ( unsigned i2 = 0; i2 < index_limit - 1; ++ i2 )
    {
	const ustring * name2 = name[i2];
	if ( name2 == NULL )
	    continue;
	if ( ustring_length ( name2 ) != name_length )
	    continue;
	if ( strcmp (    ustring_chars ( name2 ), n )
	              != 0 )
	    continue;

	line_error ( "character name `%s' used more"
		     " than once; line accepted", n );
	break;
    }
}

// Store numeric value n/d for a character.  Complain if
// character has previously been given a numeric value.
// Complain if numerator n or demonimator d out of
// range.
//
void store_numeric_value ( Uchar c, double n, double d )
{
    assert ( c < index_size );

    if ( n < -1e15 || +1e15 < n
         ||
	 d <= 0 || +1e15 < d )
    {
	line_error ( "character %02X is assigned a"
	             " numerator or denominator out of"
	             " range; line ignored", c );
	return;
    }

    unsigned i = index[c];
    if ( i != 0 )
    {
	if ( denominator[i] != 0 )
	{
	    line_error (
		"character %02X assigned more than one"
		" numeric value; line ignored", c );
	    return;
	}
    }
    else
    {
	// Search for a type with the same numeric
	// value and no name.
	//
	for ( unsigned i2 = 0; i2 < index_limit; ++ i2 )
	{
	    if ( name[i2] != NULL )
	        continue;
	    if ( numerator[i2] != n )
	        continue;
	    if ( denominator[i2] != d )
	        continue;

	    assert ( index[c] == 0 );
	    -- reference_count[0];
	    ++ reference_count[i2];
	    index[c] = i2;
	    return;
	}

	i = new_index ( c );
    }

    numerator[i] = n;
    denominator[i] = d;
    numeric_value[i] = n / d;
}

// Store category into a character.  Complain if charac-
// ter has previously been given a different category
// (other then UNSPECIFIED_CATEGORY).  (Also if charac-
// has a numeric value detecting whether it has been
// given a different category may not be possible.)
// Complain if category is not legal.
//
// Note that categories of characters c < 256 may be
// changed to equal c; see unicode_data.h.
//
void store_category ( Uchar c, unsigned char cat )
{
    assert ( c < index_size );

    if ( c < 256 && ( c < 'A' || 'Z' < c )
		 && ( c < 'a' || 'z' <= c ) )
    {
        assert (    cat != 'N'
	         && cat != 'S'
		 && cat !='E' );
	    // Replacing spacing mark categories will
	    // get us into trouble when DerivedCombin-
	    // ingClass.txt is read.

        cat = c;
    }

    unsigned i = index[c];

    if ( i != 0 )
    {
	if ( name[i] != NULL )
	{
	    if ( category[i] == UNSPECIFIED_CATEGORY )
		category[i] = cat;
	    else
		line_error ( "character %02X (%s)"
		             " assigned more than one"
			     " category; line ignored",
			     c, name[i] );

	}
	else if ( denominator[i] != 0 )
	{
	    if ( category[i] == UNSPECIFIED_CATEGORY )
	        category[i] = cat;
	    else if ( category[i] != cat )
	    {
	        // Index is shared between several
		// characters with no name but the same
		// numeric value; find another index
		// with the same numeric value and the
		// new category, or allocate a new
		// index.
		//
		for ( unsigned i2 = 0; i2 < index_limit;
		                       ++ i2 )
		{
		    if ( name[i2] != NULL )
		        continue;
		    if ( numerator[i2] != numerator[i] )
		        continue;
		    if (    denominator[i2]
		         != denominator[i] )
		        continue;
		    if ( category[i2] != cat )
		    {
		        assert ( category[i2]
			         !=
				 UNSPECIFIED_CATEGORY );
			continue;
		    }
		    -- reference_count[i];
		    index[c] = i2;
		    ++ reference_count[i2];
		    return;
		}

		// Search failed.  Allocate new index.
		//
		unsigned i2 = new_index ( c );
		numerator[i2] = numerator[i];
		denominator[i2] = denominator[i];
		numeric_value[i2] = numeric_value[i];
		category[i2] = cat;
	    }
	}
	else
	{
	    assert (    category[i]
	             != UNSPECIFIED_CATEGORY );
	    line_error ( "character %02X assigned"
			 " more than one category;"
			 " line ignored", c );
	}

	return;
    }

    // Search for a type with the same category
    // value and no name, denominator, or numerator.
    //
    for ( i = 0; i < index_limit; ++ i )
    {
	if ( category[i] != cat ) continue;
	if ( denominator[i] != 0 ) continue;
	if ( name[i] != NULL ) continue;

	assert ( index[c] == 0 );
	-- reference_count[0];
	++ reference_count[i];
	index[c] = i;
	return;
    }

    // Allocate new index.
    //
    i = new_index ( c );
    category[i] = cat;
}

// Current input data:
//
char const * file;
ifstream in;
const unsigned line_size = 500;
    // Derived file lines are automatically generated
    // can be long.
char line[line_size+2];
    // Input line.
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
    ++ line_count;
    if ( in.getline ( line, sizeof ( line ) ),
         ! in.eof() )
    {
        line[line_size+1] = 0;
        if ( strlen ( line ) > line_size )
	{
	    line_error ( "line too long" );
	    in.clear();
	}
	return true;
    }
    else
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
    while ( * line && * line != ';' ) ++ line;
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
	    line_error ( "bad range" );
	    continue;
	}

	return true;
    }
    return false;
}

// Check range to be sure hi < unicode_index_size.  If
// yes, do nothing.  If no, print error message if
// print_on_error is true, and reset `high' to unicode_
// index_size in any case.
//
// Return `low <= high' after any resetting of `high'.
//
bool check_range ( Uchar low, Uchar & high,
                   bool print_on_error = true )
{
    if ( high >= unicode_index_size )
    {
	high = unicode_index_size - 1;
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
			     unicode_index_size - 1 );
	}
    }
    return low <= high;
}

// Read current field into `field'.  Return true if
// success, false if failure.  Note that skip_to_next
// must be called after reading this field to get to the
// next field. 
//
char field[line_size+1];
bool read_field ( const char * line )
{
    line = skip_whitespace ( line );
    if ( * line == 0 ) return false;
    const char * p = line;
    while ( * p && ! isspace ( * p )
                && * p != ';' && * p != '#' )
        ++ p;
    if ( p == line ) return false;
    strncpy ( field, line, p - line );
    field[p - line] = 0;
    return true;
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
	if ( strcmp ( field, "abbreviation" ) != 0 )
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
	    continue;

	store_name ( low, field );
    }

    ::close();
}

// Read DerivedNumericValues.txt.
//
void read_numeric_values ( void )
{
    ::open ( "DerivedNumericValues.txt" );

    Uchar low, high;
    while ( read_range ( low, high ) )
    {
        if ( ! check_range ( low, high ) )
	    continue;

        const char * p = line;
	p = skip_to_next ( p );
	p = skip_to_next ( p );
	p = skip_to_next ( p );
	if ( ! read_field ( p ) )
	    line_error ( "third field not found" );

	char * q;
	long long numerator = strtoll ( p, & q, 10 );
	if ( q == p )
	    line_error ( "bad numeric third field" );

	long long denominator = 1;
	if ( * q == '/' )
	{
	    p = ++ q;
	    denominator = strtoll ( p, & q, 10 );
	    if ( q == p )
		line_error
		    ( "bad numeric third field" );
	}
	for ( Uchar c = low; c <= high; ++ c  )
	    store_numeric_value
	        ( c, (double) numerator,
		     (double) denominator );
    }

    ::close();
}

// Read DerivedGeneralCategory.txt.  Set character
// classes that are not already set.  Check that
// the classes of already set characters would be
// D, R, H, or O but for the class already having
// been set to 0 ... 9.
//
void read_general_category ( void )
{
    ::open ( "DerivedGeneralCategory.txt" );

    Uchar low, high;
    while ( read_range ( low, high ) )
    {
        const char * p = line;
	p = skip_to_next ( p );

	if ( ! read_field ( p ) )
	    line_error ( "first field not found" );

	if ( strlen ( field ) != 2 )
	{
	    line_error
	        ( "general category not"
		  " 2 characters long" );
	    continue;
	}

	unsigned char cat = 0;
	for ( unsigned j = 0;
	      j < category_data_size; ++ j )
	{
	    category_datum & d = category_data[j];
	    if ( d.category_name != NULL
	         &&
		    strcmp ( field, d.category_name )
		 == 0 )
	    {
		cat = d.category;
		break;
	    }
	}
	if ( cat == 0 )
	{
	    line_error ( "characters assigned an"
			 " unknown category `%s'; line"
			 " ignored", field );
	    return;
	}

	bool print_on_error =
	    strcmp ( field, "Co" ) != 0
	    &&
	    strcmp ( field, "Cn" ) != 0;
	if ( ! check_range ( low, high,
	                     print_on_error ) )
	    continue;

	for ( Uchar c = low; c <= high; ++ c  )
	    store_category ( c, cat );
    }

    ::close();
}

// Read DerivedCombiningClass.txt
//
void read_combining_class ( void )
{
    ::open ( "DerivedCombiningClass.txt" );

    Uchar low, high;
    while ( read_range ( low, high ) )
    {
        if ( ! check_range ( low, high, false ) )
	    continue;

        const char * p = line;
	p = skip_to_next ( p );
	if ( ! read_field ( p ) )
	    line_error ( "first field not found" );

	char * q;
	long cc = strtol ( field, & q, 10 );
	if ( * q != 0 || cc < 0 )
	{
	    line_error ( "bad combining class" );
	    continue;
	}

	for ( Uchar c = low; c <= high; ++ c )
	{
	    unsigned i = index[c];
	    char cat = category[i];
	    if ( ( cc = 0 && cat != 'N' )
	         ||
		 ( cc > 0 && cat != 'S' && cat != 'E' )
	       )
	        line_error
		    ( "for character code %02X"
		      " combining class %d conflicts"
		      " with UNICODE category %c",
		      c, cc, cat );
	}
    }

    ::close();
}
//
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
	p = skip_to_next ( p );
	if ( ! read_field ( p ) )
	    line_error ( "first field not found" );

	unsigned i;
	for ( i = 0; i < supported_set_limit; ++ i )
	{
	    if ( supported_set[i] == NULL ) continue;
	    if (    strcmp ( supported_set[i], field )
	         == 0 )
		break;
	}
	if ( i == supported_set_limit )
	{
	    line_error ( "unrecognized supported"
	                 " character set name (%s);"
			 " line ignored",
			 field );
	    continue;
	}

	for ( Uchar c = low; c <= high; ++ c )
	{
	    unsigned j =
	        index[c] >> supported_set_shift;
	    if ( j == i ) continue;
	    if ( j != 0 && supported_set[j] != NULL )
	    {
	        line_error ( "for character code %02X"
		             " new supported set name"
			     " conflicts with previous"
			     " name (%s); new name "
			     " ignored",
			     c, supported_set[j] );
		continue;
	    }
	    index[c] |= i << supported_set_shift;
	}
    }

    ::close();

    for ( Uchar c = 128; c < index_size; ++ c )
    {
        unsigned i = index[c] >> supported_set_shift;
	if ( i != 0 ) continue;
	index[c] |=    ( supported_set_limit - 1 )
	            << supported_set_shift;
    }
}

int main ( int argc, const char ** argv )
{
    assert ( sizeof(Uchar) == 4 );

    cout << setiosflags ( ios_base::uppercase );

    initialize();
    read_names();
    store_name ( SOFTWARE_NL, "NL" );
    read_numeric_values();
    read_general_category();
    read_combining_class();
    read_composite_characters();

    unsigned short nlindex = index[SOFTWARE_NL];
    assert ( ( nlindex >> supported_set_shift )
             ==
	     supported_set_limit - 1 );
    nlindex &= index_mask;
    assert ( category[nlindex] == 'v' );
    assert ( denominator[nlindex] == 0 );
    assert ( strcmp ( ustring_chars ( name[nlindex] ),
		      "NL" )
             == 0 );

    // Change category fo SOFTWARE_NL.
    //
    category[nlindex] = SOFTWARE_NL_CATEGORY;

    finalize();
    final_check();
    if ( argc > 1 ) output ( argv[1] );
    if ( argc > 2 ) dump ( argv[2] );
}
