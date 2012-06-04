// MIN Data System Program to Make UNICODE Class Table 
//
// File:	make_unicode_class.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun  4 07:10:55 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <iostream>
# include <iomanip>
# include <fstream>
# include <cstdlib>
# include <cctype>
using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::setiosflags;
using std::ios_base;

const unsigned unicode_class_size = 0x30000;
char unicode_class[unicode_class_size];
    // Class table.  Initially full of 0's.

struct range { unsigned low; unsigned high; };

// Store c into the given range.  If anything already
// stored there, complain.  Return count of complaints.
//
unsigned store ( char c, range & r )
{
    unsigned count = 0;
    for ( unsigned i = r.low; i <= r.high; ++ i )
    {
        if ( unicode_class[i] != 0 )
	{
	    cout << "ERROR: class conflict between "
	         << c << " and " << unicode_class[i]
		 << " for character code 0x"
		 << hex << i << dec << endl;
	    ++ count;
	}
	else unicode_class[i] = c;
    }
    return count;
}

// Current input data:
//
char const * file;
ifstream in;
const unsigned line_size = 10000;
char line[line_size+1];
    // Input line.
unsigned line_count;

// Print error message mentioning `file', `line_count',
// and `line', and if `fatal' is true, exit.
//
void line_error
	( const char * message, bool fatal = true )
{
    cout << "ERROR: " << message << endl
         << "       in line " << line_count
	 << " of " << file
	 << ":" << endl << line << endl;
    if ( fatal ) exit ( 1 );
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
// success, false on end of file or read error, and
// print error and exit if line too long.
//
bool read_line ( void )
{
    ++ line_count;
    if ( in.getline ( line, line_size ) )
    {
        line[line_size] = 0;
        if ( strlen ( line ) <= line_size ) return true;
	line_error ( "line too long" );
    }
    return false;
}

// Given a pointer to a character in a line, return a
// pointer to the next non-whitespace character.  Skip
// `#' comments, so if the next character is a line end,
// it is NUL.
//
const char * skip ( const char * line )
{
    while ( isspace ( * line ) ) ++ line;
    if ( * line == '#' ) while ( * line ) ++ line;
    return line;
}

// Given a pointer to a non-whitespace character in a
// line, read a character range.  Return a pointer to
// just after the range, or NULL if no range found.
//
const char * read_range
	( const char * line,
	  ::range & range,
	  unsigned limit = unicode_class_size - 1 )
{
    char * p;
    long n = strtol ( line, & p, 16 );
    if ( p == line ) return NULL;
    if ( n > limit ) return NULL;
    range.low = (unsigned) n;
    line = p;
    if ( line[0] == '.' && line[1] == '.' )
    {
        line += 2;
	n = strtol ( line, & p, 16 );
	if ( p == line ) return NULL;
	if ( n > limit ) return NULL;
	range.high = (unsigned) n;
	if ( range.high < range.low ) return NULL;
	line = p;
    }
    else range.high = range.low;

    return line;
}

// Read DerivedGeneralCategory.txt
//
void read_general_category ( void )
{
    ::open ( "DerivedGeneralCategory.txt" );

    while ( read_line() )
    {
        const char * p = line;
	p = skip ( p );
	if ( * p == 0 ) continue;

	range r;
	p = read_range ( p, r, 0xFFFFFF );
	if ( p == NULL )
	    line_error ( "bad range" );

	p = skip ( p );
	if ( * p ++ != ';' )
	    line_error ( "`;' not found" );

	p = skip ( p );
	const char * q = p;
	while ( * q && ! isspace ( * q ) && * q != ';'
	            && * q != '#' ) ++ q;
	if ( q != p + 2 )
	    line_error
	        ( "general category not"
		  " 2 characters long" );

	const char * s = skip ( q );
	if ( * s  && * s != ';' )
	    line_error
	        ( "bad stuff after general category" );

	if ( r.high >= unicode_class_size )
	{
	    char message[200];
	    sprintf ( message, "code above 0x%X used",
	                       unicode_class_size - 1 );
	    if ( strncmp ( "Cn", p, 2 ) != 0
	         &&
	         strncmp ( "Co", p, 2 ) != 0 )
		line_error ( message, false );
	    if ( r.low >= unicode_class_size )
		continue;
	    r.high = unicode_class_size - 1;
	}

	char c =
	    strncmp ( "Lu", p, 2 ) == 0 ? 'U' :
	    strncmp ( "Ll", p, 2 ) == 0 ? 'L' :
	    strncmp ( "Lt", p, 2 ) == 0 ? 'T' :
	    strncmp ( "Lm", p, 2 ) == 0 ? 'M' :
	    strncmp ( "Lo", p, 2 ) == 0 ? 'O' :

	    strncmp ( "Mn", p, 2 ) == 0 ? 'N' :
	    strncmp ( "Mc", p, 2 ) == 0 ? 'S' :
	    strncmp ( "Me", p, 2 ) == 0 ? 'E' :

	    strncmp ( "Nd", p, 2 ) == 0 ? 'D' :
	    strncmp ( "Nl", p, 2 ) == 0 ? 'R' :
	    strncmp ( "No", p, 2 ) == 0 ? 'H' :

	    strncmp ( "Pc", p, 2 ) == 0 ? 'n' :
	    strncmp ( "Pd", p, 2 ) == 0 ? 'd' :
	    strncmp ( "Ps", p, 2 ) == 0 ? 'o' :
	    strncmp ( "Pe", p, 2 ) == 0 ? 'c' :
	    strncmp ( "Pi", p, 2 ) == 0 ? 'i' :
	    strncmp ( "Pf", p, 2 ) == 0 ? 'f' :
	    strncmp ( "Po", p, 2 ) == 0 ? 'h' :

	    strncmp ( "Sm", p, 2 ) == 0 ? 'm' :
	    strncmp ( "Sc", p, 2 ) == 0 ? 'y' :
	    strncmp ( "Sk", p, 2 ) == 0 ? 'r' :
	    strncmp ( "So", p, 2 ) == 0 ? 't' :

	    strncmp ( "Zs", p, 2 ) == 0 ? 's' :
	    strncmp ( "Zl", p, 2 ) == 0 ? 'l' :
	    strncmp ( "Zp", p, 2 ) == 0 ? 'p' :

	    strncmp ( "Cc", p, 2 ) == 0 ? 'x' :
	    strncmp ( "Cf", p, 2 ) == 0 ? 'z' :
	    strncmp ( "Cs", p, 2 ) == 0 ? 'g' :
	    strncmp ( "Co", p, 2 ) == 0 ? 'v' :
	    strncmp ( "Cn", p, 2 ) == 0 ? 'u' :
	    0;

	if ( c == 0 )
	    line_error
	        ( "unknown general category" );
	if ( store ( c, r ) > 0 )
	    line_error
	        ( "conflicting general categories"
		  " (see above ERRORs)", true );
    }

    ::close();
}

// Find missing general categories and set them to `w'.
// If a missing general category is found for a
// character with code < `ok_lower_limit', print an
// error message.  Return the count of error messages
// printed.
//
unsigned find_missing_classes
    ( unsigned ok_lower_limit = unicode_class_size )
{
    unsigned count = 0;
    for ( unsigned i = 0; i < unicode_class_size; ++ i )
    {
        if ( unicode_class[i] != 0 ) continue;
	unicode_class[i] = 'w';
	if ( i >= ok_lower_limit ) continue;
	++ count;
	cout << "ERROR: missing general category for"
	     << " character code 0x" << hex << i << dec
	     << endl;
    }
    return count;
}

// Read DerivedCombiningClass.txt
//
void read_combining_class ( void )
{
    ::open ( "DerivedCombiningClass.txt" );

    while ( read_line() )
    {
        const char * p = line;
	p = skip ( p );
	if ( * p == 0 ) continue;

	range r;
	p = read_range ( p, r, 0xFFFFFF );
	if ( p == NULL )
	    line_error ( "bad range" );

	if ( r.high >= unicode_class_size )
	{
	    char message[200];
	    sprintf ( message, "code above 0x%X used",
	                       unicode_class_size - 1 );
	    line_error ( message, false );
	    if ( r.low >= unicode_class_size )
	        continue;
	    else r.high = unicode_class_size - 1;
	}

	p = skip ( p );
	if ( * p ++ != ';' )
	    line_error ( "`;' not found" );

	p = skip ( p );
	char * q;
	long cc = strtol ( p, & q, 10 );
	if ( q == p || cc < 0 )
	    line_error ( "bad combining class" );

	p = q;
	p = skip ( p );
	if ( * p  && * p != ';' )
	    line_error
	        ( "bad stuff after combining class" );

	for ( unsigned i = r.low; i <= r.high; ++ i )
	{
	    if ( ( cc = 0 && unicode_class[i] != 'N' )
	         ||
		 ( cc > 0 && unicode_class[i] != 'S'
		          && unicode_class[i] != 'E' )
	       )
	        cout << "ERROR: for character code 0x"
		     << hex << i << dec
		     << " combining class " << cc
		     << " conflicts with UNICODE class "
		     << unicode_class[i] << endl;
	}
    }

    ::close();
}

// Read DerivedNumericValues.txt
//
void read_numeric_values ( void )
{
    ::open ( "DerivedNumericValues.txt" );

    while ( read_line() )
    {
        const char * p = line;
	p = skip ( p );
	if ( * p == 0 ) continue;

	range r;
	p = read_range ( p, r, 0xFFFFFF );
	if ( p == NULL )
	    line_error ( "bad range" );

	if ( r.high >= unicode_class_size )
	{
	    char message[200];
	    sprintf ( message, "code above 0x%X used",
	                       unicode_class_size - 1 );
	    line_error ( message, false );
	    if ( r.low >= unicode_class_size )
	        continue;
	    else r.high = unicode_class_size - 1;
	}

	p = skip ( p );
	if ( * p ++ != ';' )
	    line_error ( "`;' not found" );

	while ( * p && * p != ';' ) ++ p;
	p = skip ( p );
	if ( * p ++ != ';' )
	    line_error ( "third field not found" );

	while ( * p && * p != ';' ) ++ p;
	p = skip ( p );
	if ( * p ++ != ';' )
	    line_error ( "fourth field not found" );

	p = skip ( p );
	const char * q = p;
	while ( * q && ! isspace ( * q )
	            && * q != ';' && * q != '#' ) ++ q;

	const char * s = skip ( q );
	if ( * s && * s != ';' )
	    line_error ( "bad numeric value field" );

	if ( ! isdigit ( * p ) || q != p + 1 )
	    continue;

	for ( unsigned i = r.low; i <= r.high; ++ i )
	{
	    if ( unicode_class[i] != 'D'
	         &&
		 unicode_class[i] != 'R'
	         &&
		 unicode_class[i] != 'H'
	         &&
		 unicode_class[i] != 'O' )

	    {
		cout << "ERROR: for character code 0x"
		     << hex << i << dec
		     << " decimal value"
		     << " conflicts with UNICODE class "
		     << unicode_class[i] << endl;
		line_error ( "above error", false );
	    }
	    else
	        unicode_class[i] = * p;
	}
    }

    ::close();
}

void output ( void )
{
    ofstream out ( "unicode_class.h" );
    if ( ! out )
    {
        cout << "ERROR: could not open unicode_class.h"
	        " for output" << endl;
	exit ( 1 );
    }
    out << setiosflags ( ios_base::uppercase );

    out <<
      "// UNICODE Character Class Data\n"
      "//\n"
      "// File:	unicode_class.h\n"
      "\n"
      "// The c+1'st character in UNICODE_CLASS\n"
      "// is the character class of the UNICODE\n"
      "// character with code c.\n"
      "\n"
      "// UNICODE_CLASS_SIZE is the number of\n"
      "// characters in UNICODE_CLASS.\n"
      "\n"
      "// Character Classes:\n"
      "//\n"
      "//     U    Upper case letter.\n"
      "//     L    Lower case letter.\n"
      "//     T    Title case letter.\n"
      "//     M    Modifier letter.\n"
      "//     O    Other letter.\n"
      "//\n"
      "//     N    Nonspacing mark.\n"
      "//     S    Spacing mark.\n"
      "//     E    Enclosing mark.\n"
      "//\n"
      "//     D    Decimal number.\n"
      "//     R    Letter number.\n"
      "//     H    Other number.\n"
      "//\n"
      "//     n    Connector punctuation.\n"
      "//     d    Dash punctuation.\n"
      "//     o    Open punctuation.\n"
      "//     c    Close punctuation.\n"
      "//     i    Initial punctuation.\n"
      "//     f    Final punctuation.\n"
      "//     h    Other punctuation.\n"
      "//\n"
      "//     m    Math symbol.\n"
      "//     y    Currency symbol.\n"
      "//     r    Modifier symbol.\n"
      "//     t    Other symbol.\n"
      "//\n"
      "//     s    Space Separator.\n"
      "//     l    Line Separator.\n"
      "//     p    Paragraph Separator.\n"
      "//\n"
      "//     x    Control.\n"
      "//     z    Format.\n"
      "//     g    Surrogate.\n"
      "//     v    Private Use.\n"
      "//     u    Unassigned.\n"
      "//\n"
      "//     w    Unspecified.\n"
      "\n";

    out << "# define UNICODE_CLASS_SIZE 0x"
        << hex << unicode_class_size << dec
	<< endl << endl;

    out << "# define UNICODE_CLASS \\\n";
    for ( unsigned i = 0; i < unicode_class_size; ++ i )
    {
        if ( i % 64 == 0 )
	{
	    if ( i > 0 ) out << "\" \\" << endl;
	    out << "    \"";
	}
	out << unicode_class[i];
    }
    out << "\"";

    out.close();
}

int main ( int argc )
{
    cout << setiosflags ( ios_base::uppercase );
    read_general_category();
    find_missing_classes();
    read_combining_class();
    read_numeric_values();
    output();
}
