// MIN Data System Program to Make UNICODE Type Tables 
//
// File:	make_unicode_types.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Jun 29 15:11:43 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <iostream>
# include <iomanip>
# include <fstream>
# include <cstdlib>
# include <cstdarg>
# include <cctype>
# include <cstring>
using std::cout;
using std::endl;
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
void line_error ( const char * message, ... );

const unsigned unicode_index_size = 0x30000;
unsigned char unicode_index[unicode_index_size];
    // Index table.  Initially full of 0's.
    //
    // unicode_index[x] is the type of x.
    //
    // Each x < 256 that is not a letter has its own
    // private type.  Each character with a name has its
    // own private type.  The types of other characters
    // are in 1-1 correspondence with their characters
    // categories.

struct unicode_type
    // unicode_types[t] is the description of type t.
    //
    // Names must be stored first, then numerators and
    // denominators, then categories.  A type may NOT
    // have both a name and numerator/denominator.
{
    char category;
        // Character category.  See table output in
	// unicode_types.h.

    short name;
    char name_length;
    char name_columns;
        // If name != 0, then the character has a name
	// that begins with unicode_names[name] and
	// has name_length unicode characters that take
	// name_columns print columns when printed
	// consecutively.

    signed char numerator, denominator;
        // For number characters, the value of the
	// character as numerator/denominator.  For
	// integers denominator is 1.  numerator
	// may be negative, denominator must be > 0.

} unicode_types[256];
unsigned unicode_types_size = 0;
    //unicode_types_size is number of elements actually
    //used.

Uchar unicode_names[16000];
unsigned unicode_names_size = 0;
    // See unicode_type.  `unsigned int' must be 32
    // bits.  unicode_names_size is number of elements
    // actually used.

// Encode category as a single letter.  If category is
// unknown, return 0 instead.
//
char encode_category ( const char * category )
{
    return
	strncmp ( "Lu", category, 2 ) == 0 ? 'U' :
	strncmp ( "Ll", category, 2 ) == 0 ? 'L' :
	strncmp ( "Lt", category, 2 ) == 0 ? 'T' :
	strncmp ( "Lm", category, 2 ) == 0 ? 'M' :
	strncmp ( "Lo", category, 2 ) == 0 ? 'O' :

	strncmp ( "Mn", category, 2 ) == 0 ? 'N' :
	strncmp ( "Mc", category, 2 ) == 0 ? 'S' :
	strncmp ( "Me", category, 2 ) == 0 ? 'E' :

	strncmp ( "Nd", category, 2 ) == 0 ? 'D' :
	strncmp ( "Nl", category, 2 ) == 0 ? 'R' :
	strncmp ( "No", category, 2 ) == 0 ? 'H' :

	strncmp ( "Pc", category, 2 ) == 0 ? 'n' :
	strncmp ( "Pd", category, 2 ) == 0 ? 'd' :
	strncmp ( "Ps", category, 2 ) == 0 ? 'o' :
	strncmp ( "Pe", category, 2 ) == 0 ? 'c' :
	strncmp ( "Pi", category, 2 ) == 0 ? 'i' :
	strncmp ( "Pf", category, 2 ) == 0 ? 'f' :
	strncmp ( "Po", category, 2 ) == 0 ? 'h' :

	strncmp ( "Sm", category, 2 ) == 0 ? 'm' :
	strncmp ( "Sc", category, 2 ) == 0 ? 'y' :
	strncmp ( "Sk", category, 2 ) == 0 ? 'r' :
	strncmp ( "So", category, 2 ) == 0 ? 't' :

	strncmp ( "Zs", category, 2 ) == 0 ? 's' :
	strncmp ( "Zl", category, 2 ) == 0 ? 'l' :
	strncmp ( "Zp", category, 2 ) == 0 ? 'p' :

	strncmp ( "Cc", category, 2 ) == 0 ? 'x' :
	strncmp ( "Cf", category, 2 ) == 0 ? 'z' :
	strncmp ( "Cs", category, 2 ) == 0 ? 'g' :
	strncmp ( "Co", category, 2 ) == 0 ? 'v' :
	strncmp ( "Cn", category, 2 ) == 0 ? 'u' :
	0;
}

// Store name into a character.  Complain if character
// has previously been given a name.  Complain if name
// already used by another character.
//
// The name must be ASCII graphic characters presented
// as a character string.  This is converted to unicode.
// The output database allows more complex names, but
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
	line_error ( "character 0x%X assigned more than"
	             " one name", c );
	return;
    }

    int name_length = strlen ( name );
    assert ( name_length > 0 );

    // Allocate new type.
    //
    unicode_index[c] = unicode_types_size;
    unicode_type & type =
        unicode_types[unicode_types_size++];
    type.name = unicode_names_size;
    type.name_length = name_length;
    type.name_columns = name_length;
    type.numerator = type_denominator
                   = type.category = 0;
    for ( int i = 0; i < name_length; ++ i )
        unicode_names[unicode_name_size++] = name[i];

    // Check for name being used more than once.
    //
    for ( int t = 0; t < unicode_types_size - 1; ++ t )
    {
        unicode_type & type2 = unicode_types[t];
	if ( type2.name_length != type.name_length )
	    continue;
	Uchar p = unicode_names + type.name;
	Uchar p2 = unicode_names + type2.name;
	int i = 0;
	while ( i < type.name_length
	        &&
		* p ++ == * p2 ++ ) ++ i;
        if ( i != type.name_length ) continue;

	line_error ( "character name `%s' used more"
		     " than once", name );
	break;
    }
}

// Store numerator/demominator into a character.
// Complain if character has previously been given a
// name.  Complain if character already has a numerator/
// denominator.  Complain if numerator or demonimator
// out of range.  Do nothing if there is a complaint.
//
void store_number_value
    ( Uchar c, long numerator, long denominator )
{
    assert ( c < unicode_index_size );
    if ( unicode_index[c] != 0 )
    {
        unicode_type & type =
	    unicode_types[unicode_index[c]];
	line_error (
	    type.name_length > 0 ?
	    "character 0x%X previously assigned a name"
	    " also has a numeric value" :
	    "character 0x%X has two numeric values",
	    c );
	return;
    }

    if ( numerator < -127 || 127 < numerator
         ||
	 denominator <= 0 || 127 < denominator )
    {
	line_error ( "character 0x%X is assigned a"
	             " numerator or denominator out of"
	             " range", c );
	return;
    }

    // Search for a type with the same numeric value.
    //
    for ( int t = 0; t < unicode_types_size; ++ t )
    {
        unicode_type & type = unicode_types[t];
	if ( type.numerator != numerator ) continue;
	if ( type.denominator != denominator ) continue;
	unicode_index[c] = t;
	return;
    }

    // Allocate new type.
    //
    unicode_index[c] = unicode_types_size;
    unicode_type & type =
        unicode_types[unicode_types_size++];
    type.name = type.name_length
              = type.name_columns
	      = type.category = 0;
    type.numerator = numerator;
    type.denominator = denominator;

    return;
}

// Store category into a character.  Complain if charac-
// ter has previously been given a different category.
// Complain if category is not legal.  Do nothing if
// there is a complaint.
//
void store_category
    ( Uchar c, const char * category )
{
    assert ( c < unicode_index_size );

    char cat = encode_category ( category );
    if ( cat == 0 )
    {
	line_error ( "character 0x%X is assigned an"
	             " unknown category `%s'",
		     c, category );
	return;
    }

    if ( unicode_index[c] != 0 )
    {
        unicode_type & type =
	    unicode_types[unicode_index[c]];
	if ( type.name_length == 0 && c < 256 )
	    cat = c;

	if ( type.category == 0 )
	    type.category = cat;
	else if ( type.category != cat )
	    line_error ( "character 0x%X assigned"
			 " more than one catagory",
			 c );
	return;
    }

    if ( c < 256 ) cat = c;

    // Search for a type with the same category value
    // and no name, denominator, or numerator.
    //
    for ( int t = 0; t < unicode_types_size; ++ t )
    {
        unicode_type & type = unicode_types[t];
	if ( type.category != cat ) continue;
	if ( type.numerator != 0 ) continue;
	if ( type.denominator != 0 ) continue;
	if ( type.name_length != 0 ) continue;
	unicode_index[c] = t;
	return;
    }

    // Allocate new type.
    //
    unicode_index[c] = unicode_types_size;
    unicode_type & type =
        unicode_types[unicode_types_size++];
    type.name = type.name_length
              = type.name_columns
	      = type.numerator
	      = type.denominator = 0;
    type.category = cat;

    return;
}

// Current input data:
//
char const * file;
ifstream in;
const unsigned line_size = 80;
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
    vsfprint ( buffer, format, args );

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
    if ( in.getline ( line, sizeof ( line ) ) )
    {
        line[line_size+1] = 0;
        if ( strlen ( line ) > line_size )
	    line_error ( "line too long" );
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
    if ( high >= unicode_class_size )
    {
	if ( print_on_error )
	    line_error ( "code above 0x%X used",
			 unicode_index_size - 1 );
	high = unicode_class_size - 1;
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
    while ( * p && ! whitespace ( * p )
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

	const char * q;
	long numerator = strtol ( p, & q, 10 );
	if ( q == p )
	    line_error ( "bad numeric third field" );

	long denominator = 1;
	if ( * q == '/' )
	{
	    p = ++ q;
	    denominator = strtol ( p, & q, 10 );
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
	p = skip ( p );

	if ( ! read_field ( p ) )
	    line_error ( "first field not found" );

	if ( strlen ( field ) != 2 )
	{
	    line_error
	        ( "general category not"
		  " 2 characters long" );
	    continue;
	}

	char category = encode_category ( field );
	if ( category == 0 )
	{
	    line_error
	        ( "unrecognized category `%s'",
		  field );
		  " 2 characters long" );
	    continue;
	}

	check_range ( low, high, category != 'u'
	                         &&
				 category != 'v' );

	for ( Uchar c = low; c <= high; ++ c  )
	    store_category ( c, category );
    }

    ::close();
}

// Find missing general categories and set them to `w'.
//
void set_missing_types ( void )
{
    Uchar c;
    for ( Uchar c = 0; c < unicode_index_size; ++ c )
    {
        if ( unicode_index[c] == 0 )
	    store_category ( c, 'w' );
    }
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
	p = skip ( p );
	if ( * p == 0 ) continue;
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
		    ( "for character code 0x%X"
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

    read_numeric_values();
    read_general_category();
    find_missing_classes();
    read_combining_class();
    output();
}
