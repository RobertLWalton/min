// MIN Data System Program to Make UNICODE Type Tables 
//
// File:	make_unicode_types.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun 30 02:15:06 EDT 2014
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

typedef unsigned int Uchar;
    // Must be 32 bit integer.

// Print error message mentioning file, line number,
// and printing the current line.   If fatal is true,
// exit program.
//
void line_error ( const char * format, ... );

const unsigned unicode_index_size = 0x30000;
unsigned short unicode_index[unicode_index_size];
    // Index table.  Initially full of 0's.
    //
    // unicode_index[c] is the index of c.  Indices are
    // very changeable, and may not be the same week
    // to week (unlike the category of c which should
    // never change).
    //
    // unicode_types[unicode_index[c]] is the type of c.
    //
    // Each character with a name has its own private
    // type.  The types of other characters are shared
    // so all characters with no name but with the same
    // category and numeric value (which may be missing)
    // have the same type.  Note that as each character
    // c < 256 that is not a letter or digit and that
    // does not have a name is its own category, this
    // means that all these characters have their own
    // private type.

const unsigned unicode_types_size = 512;
struct unicode_type
    // unicode_types[unicode_index[c]] is the type c.
    //
    // Names must be stored first, then numerators and
    // denominators, then categories.  See the store_...
    // functions below.
{
    unsigned char category;
        // Character category.  See the category table
	// below for general categories.  However, as an
	// exception, characters c < 256 are their own
	// categories (i.e., the category of `+' is `+')
	// if they are not a letter or digit and do not
	// have a name (i.e., are not SP or DEL, etc.).

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
	// numerator may be negative, but the denomina-
	// tor must be > 0.
	//
	// Some languages have numeric values of one
	// trillion.

} unicode_types[unicode_types_size];
unsigned unicode_max_index = 0;
    // unicode_max_index is maximum character index and
    // therefore is the number of unicode_types elements
    // actually used.

Uchar unicode_names[16000];
unsigned unicode_names_size = 0;
    // See unicode_type.  unicode_names_size is number
    // of elements of unicode_names actually used.

// Table that encodes UNICODE general category to our
// MIN category map.  The UNICODE general category is
// 2 letters, while the MIN category is 1 letter and is
// better suited to making lookup-by-category tables.
// We also add the special MIN category `w' for
// characters with no UNICODE category.
//
struct category_description
{
    const char * unicode_name;
    unsigned char category;  // MIN category.
    const char * unicode_description;
} category_descriptions[] = {
    { "Lu", 'U', "Upper case letter" },
    { "Ll", 'L', "Lower case letter" },
    { "Lt", 'T', "Title case letter" },
    { "Lm", 'M', "Modifier letter" },
    { "Lo", 'O', "Other letter" },

    { "Mn", 'N', "Nonspacing mark" },
    { "Mc", 'S', "Spacing mark" },
    { "Me", 'E', "Enclosing mark" },

    { "Nd", 'D', "Decimal number" },
    { "Nl", 'R', "Letter number" },
    { "No", 'H', "Other number" },

    { "Pc", 'n', "Connector punctuation" },
    { "Pd", 'd', "Dash punctuation" },
    { "Ps", 'o', "Open punctuation" },
    { "Pe", 'c', "Close punctuation" },
    { "Pi", 'i', "Initial punctuation" },
    { "Pf", 'f', "Final punctuation" },
    { "Po", 'h', "Other punctuation" },

    { "Sm", 'm', "Math symbol" },
    { "Sc", 'y', "Currency symbol" },
    { "Sk", 'r', "Modifier symbol" },
    { "So", 't', "Other symbol" },

    { "Zs", 's', "Space Separator" },
    { "Zl", 'l', "Line Separator" },
    { "Zp", 'p', "Paragraph Separator" },

    { "Cc", 'x', "Control" },
    { "Cf", 'z', "Format" },
    { "Cs", 'g', "Surrogate" },
    { "Co", 'v', "Private Use" },

    { "Cn", 'u', "Unassigned" },
    { NULL, 'w', "Unspecified" }
};
const unsigned number_of_category_descriptions =
      sizeof ( category_descriptions )
    / sizeof ( category_description );
const unsigned char UNSPECIFIED_CATEGORY = 'w';

// Initialize tables.  unicode_types[0] gets category
// 'w' and 0 becomes the index for characters not
// assigned any category, name, or numeric value.
//
void initialize ( void )
{
    assert ( unicode_max_index == 0 );

    unicode_type & type =
        unicode_types[unicode_max_index++];

    type.name = type.name_length
              = type.name_columns = 0;
    type.numerator = type.denominator = 0;
    type.category = UNSPECIFIED_CATEGORY;
}

// Do some integrity checking of the tables.
//
void final_check ( void )
{
    unsigned count[unicode_max_index];
    for ( int i = 0; i < unicode_max_index; ++ i )
        count[i] = 0;
    for ( Uchar c = 0; c < unicode_index_size; ++ c )
    {
        unsigned i = unicode_index[c];
	assert ( i < unicode_max_index );
        ++ count[i];
    }
    for ( int i = 0; i < unicode_max_index; ++ i )
    {
        if ( count[i] > 0 ) continue;
	unicode_type & type = unicode_types[i];
	cout << "ERROR: unused type:" << endl
	     << "       name_length = "
	     << type.name_length << endl
	     << "       numerator = "
	     << type.numerator << endl
	     << "       denominator = "
	     << type.denominator << endl
	     << "       category = "
	     << type.category << endl;
    }
}

// Store UNICODE name for a character.  Complain if
// character has previously been given a name.  Complain
// if name already used by another character.
//
// The name must be a string of ASCII graphic characters
// presented as a const char * string.  This is convert-
// ed to unicode.  The unicode_types database allows
// more complex names with combining diacritics, but
// currently these are not used by the UNICODE standard.
//
void store_name ( Uchar c, const char * name )
{
    assert ( c < unicode_index_size );

    if ( unicode_index[c] != 0 )
    {
        unicode_type & type =
	    unicode_types[unicode_index[c]];
	assert ( type.name_length > 0 );
	    // Names should be set before number values
	    // and categories.
	line_error ( "character %02X assigned more than"
	             " one name; line ignored", c );
	return;
    }

    int name_length = strlen ( name );
    assert ( name_length > 0 );

    // Allocate new type.
    //
    assert ( unicode_max_index < unicode_types_size );
    unicode_index[c] = unicode_max_index;
    unicode_type & type =
        unicode_types[unicode_max_index++];
    type.name = unicode_names_size;
    type.name_length = name_length;
    type.name_columns = name_length;
    type.numerator = type.denominator = 0;
    type.category = UNSPECIFIED_CATEGORY;
    for ( int j = 0; j < name_length; ++ j )
        unicode_names[unicode_names_size++] = name[j];

    // Check for name being used more than once.
    //
    for ( unsigned i = 0; i < unicode_max_index - 1;
                          ++ i )
    {
        unicode_type & type2 = unicode_types[i];
	if ( type2.name_length != type.name_length )
	    continue;

	if ( memcmp ( unicode_names + type.name,
	              unicode_names + type2.name,
		        type.name_length
		      * sizeof ( Uchar ) )
             == 0 )
	{
	    line_error ( "character name `%s' used more"
			 " than once; line accepted",
			 name );
	    break;
	}
    }
}

// Store numeric value numerator/demominator for a
// character.  Complain if character has previously been
// given a numeric value.  Complain if numerator or
// demonimator out of range.
//
void store_numeric_value
    ( Uchar c,
      long long numerator, long long denominator )
{
    assert ( c < unicode_index_size );

    if ( numerator < LLONG_MIN || LLONG_MAX < numerator
         ||
	 denominator <= 0 || LLONG_MAX < denominator )
    {
	line_error ( "character %02X is assigned a"
	             " numerator or denominator out of"
	             " range; line ignored", c );
	return;
    }

    unsigned i = unicode_index[c];
    if ( i != 0 )
    {
	if ( unicode_types[i].denominator != 0 )
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
	for ( i = 0; i < unicode_max_index; ++ i )
	{
	    unicode_type & type = unicode_types[i];
	    if ( type.name_length > 0 )
	        continue;
	    if ( type.numerator != numerator )
	        continue;
	    if ( type.denominator != denominator )
	        continue;
	    unicode_index[c] = i;
	    return;
	}

	assert
	    ( unicode_max_index < unicode_types_size );
	i = unicode_max_index ++;
	unicode_index[c] = i;
	unicode_type & type = unicode_types[i];
	type.name = type.name_length
	          = type.name_columns = 0;
	type.category = UNSPECIFIED_CATEGORY;
    }

    unicode_types[i].numerator = numerator;
    unicode_types[i].denominator = denominator;
}

// Store category into a character.  Complain if charac-
// ter has previously been given a different category
// (other then UNSPECIFIED_CATEGORY).  (Also if charac-
// has a numeric value detecting whether it has been
// given a different category may not be possible.)
// Complain if category is not legal.
//
// Note that categories of characters c < 256 may be
// changed to equal c; see above.
//
void store_category ( Uchar c, unsigned char category )
{
    assert ( c < unicode_index_size );

    unsigned i = unicode_index[c];
    if ( i != 0 )
    {
        unicode_type & type = unicode_types[i];

	if ( type.name_length != 0 )
	{
	    if ( type.category == UNSPECIFIED_CATEGORY )
		type.category = category;
	    else
		line_error ( "character %02X assigned"
			     " more than one category;"
			     " line ignored", c );

	}
	else if ( type.denominator != 0 )
	{
	    if ( type.category == UNSPECIFIED_CATEGORY )
	        type.category = category;
	    else if ( type.category != category )
	    {
	        // Type is shared between several
		// characters with no name but the same
		// numeric value; find another type
		// with the same numeric value and the
		// new category, or make such a type.
		//
		for ( i = 0; i < unicode_max_index;
		             ++ i )
		{
		    unicode_type & type2 =
			unicode_types[i];
		    if ( type2.name_length > 0 )
		        continue;
		    if (    type2.numerator
		         != type.numerator )
		        continue;
		    if (    type2.denominator
		         != type.denominator )
		        continue;
		    if ( type2.category != category )
		    {
		        assert ( type2.category
			         !=
				 UNSPECIFIED_CATEGORY );
			continue;
		    }
		    unicode_index[c] = i;
		    return;
		}

		// Search failed.  Allocate new type.
		//
		assert (   unicode_max_index
		         < unicode_types_size );
		i = unicode_max_index ++;
		unicode_type & type2 =
		    unicode_types[i];
		type2.name = type2.name_length
		           = type2.name_columns = 0;
		type2.numerator = type.numerator;
		type2.denominator = type.denominator;
		type2.category = category;
	    }
	}
	else
	{
	    assert (    type.category
	             != UNSPECIFIED_CATEGORY );
	    line_error ( "character %02X assigned"
			 " more than one category;"
			 " line ignored", c );
	}

	return;
    }

    if ( c < 256 && category != 'L' && category != 'U' )
    {
        assert ( ! isdigit ( c ) );
        assert ( ! isalpha ( c ) );
	category = c;
    }
    else
    {
	// Search for a type with the same category
	// value and no name, denominator, or numerator.
	//
	for ( i = 0; i < unicode_max_index; ++ i )
	{
	    unicode_type & type = unicode_types[i];
	    if ( type.category != category ) continue;
	    if ( type.denominator != 0 ) continue;
	    if ( type.name_length != 0 ) continue;
	    unicode_index[c] = i;
	    return;
	}
    }

    // Allocate new type.
    //
    assert (   unicode_max_index < unicode_types_size );
    i = unicode_max_index ++;
    unicode_index[c] = i;
    unicode_type & type = unicode_types[i];
    type.name = type.name_length
              = type.name_columns
	      = type.numerator
	      = type.denominator = 0;
    type.category = category;
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
// print_on_error is true, and reset hi to unicode_
// index_size in any case.
//
void check_range ( Uchar low, Uchar & high,
                   bool print_on_error = true )
{
    if ( high >= unicode_index_size )
    {
	if ( print_on_error )
	    line_error ( "code above %02X used",
			 unicode_index_size - 1 );
	high = unicode_index_size - 1;
    }
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

// Read DerivedNumericValues.txt.
//
void read_numeric_values ( void )
{
    ::open ( "DerivedNumericValues.txt" );

    Uchar low, high;
    while ( read_range ( low, high ) )
    {
        check_range ( low, high );

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
	        ( c, numerator, denominator );
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

	unsigned char category = 0;
	for ( int j = 0;
	      j < number_of_category_descriptions;
	      ++ j )
	{
	    category_description & d =
		category_descriptions[j];
	    if ( strcmp ( field, d.unicode_name ) == 0 )
	    {
		category = d.category;
		break;
	    }
	}
	if ( category == 0 )
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
	check_range ( low, high, print_on_error );

	for ( Uchar c = low; c <= high; ++ c  )
	    store_category ( c, category );
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
        check_range ( low, high );

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
	    unicode_type & type =
	        unicode_types[unicode_index[c]];
	    char cat = type.category;
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

void output ( void )
{
    ofstream out ( "unicode_types.h" );
    if ( ! out )
    {
        cout << "ERROR: could not open unicode_types.h"
	        " for output" << endl;
	exit ( 1 );
    }
    out << setiosflags ( ios_base::uppercase );

    out <<
      "// UNICODE Character Type Data\n"
      "//\n"
      "// File:	unicode_types.h\n"
      "\n"
      "// Generated by make_unicode_types.cc\n"
      "\n"
      "// Character Categories:\n"
      "//\n"
      "# define UNICODE_CATEGORIES \\\n"
      "   { 'U', \"Upper case letter\" }, \\\n"
      "   { 'L', \"Lower case letter\" }, \\\n"
      "   { 'T', \"Title case letter\" }, \\\n"
      "   { 'M', \"Modifier letter\" }, \\\n"
      "   { 'O', \"Other letter\" }, \\\n"
      "   \\\n"
      "   { 'N', \"Nonspacing mark\" }, \\\n"
      "   { 'S', \"Spacing mark\" }, \\\n"
      "   { 'E', \"Enclosing mark\" }, \\\n"
      "   \\\n"
      "   { 'D', \"Decimal number\" }, \\\n"
      "   { 'R', \"Letter number\" }, \\\n"
      "   { 'H', \"Other number\" }, \\\n"
      "   \\\n"
      "   { 'n', \"Connector punctuation\" }, \\\n"
      "   { 'd', \"Dash punctuation\" }, \\\n"
      "   { 'o', \"Open punctuation\" }, \\\n"
      "   { 'c', \"Close punctuation\" }, \\\n"
      "   { 'i', \"Initial punctuation\" }, \\\n"
      "   { 'f', \"Final punctuation\" }, \\\n"
      "   { 'h', \"Other punctuation\" }, \\\n"
      "   \\\n"
      "   { 'm', \"Math symbol\" }, \\\n"
      "   { 'y', \"Currency symbol\" }, \\\n"
      "   { 'r', \"Modifier symbol\" }, \\\n"
      "   { 't', \"Other symbol\" }, \\\n"
      "   \\\n"
      "   { 's', \"Space Separator\" }, \\\n"
      "   { 'l', \"Line Separator\" }, \\\n"
      "   { 'p', \"Paragraph Separator\" }, \\\n"
      "   \\\n"
      "   { 'x', \"Control\" }, \\\n"
      "   { 'z', \"Format\" }, \\\n"
      "   { 'g', \"Surrogate\" }, \\\n"
      "   { 'v', \"Private Use\" }, \\\n"
      "   { 'u', \"Unassigned\" }, \\\n"
      "   \\\n"
      "   { 'w', \"Unspecified\" }\n"
      "\n"
      "// The c+1'st integer in UNICODE_INDICIES\n"
      "// is the character type index of the UNICODE\n"
      "// character with code c.\n"
      "\n"
      "// UNICODE_INDEX_SIZE is the number of\n"
      "// integers in the character type index.\n"
      "\n";

    out << "# define UNICODE_INDEX_SIZE 0x"
        << hex << unicode_index_size << dec
	<< endl << endl;

    out << "# define UNICODE_INDICES \\\n";
    for ( Uchar c = 0; c < unicode_index_size; ++ c )
    {
        if ( c % 16 == 0 )
	{
	    if ( c > 0 ) out << ", \\" << endl;
	    out << "    ";
	}
	else
	    cout << ", ";
	out << setw ( 3 ) << unicode_index[c];
    }
    out << "" << endl;

    out.close();
}

int main ( int argc, const char ** argv )
{
    assert ( sizeof(Uchar) == 4 );

    cout << setiosflags ( ios_base::uppercase );

    initialize();
    read_numeric_values();
    read_general_category();
    read_combining_class();
    final_check();
    output();
}
