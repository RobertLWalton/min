// MIN Routines to Output UNICODE Data Tables
//
// File:	output_unicode_data.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jul 19 21:52:09 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This file contains routines to check and output the
// MIN UNICODE database.  This file is to be `#include'd
// in other files such as make_unicode_data.cc and
// min_unicode_test.cc.

# include <cmath>

// Vector of ASCII name equivalents for properties:
//
const char * property_name[] = {
    "ASCII_Hex_Digit",
    "Bidi_Control",
    "Dash",
    "Deprecated",
    "Diacritic",
    "Extender",
    "Hex_Digit",
    "Hyphen",
    "Ideographic",
    "IDS_Binary_Operator",
    "IDS_Trinary_Operator",
    "Join_Control",
    "Logical_Order_Exception",
    "Noncharacter_Code_Point",
    "Other_Alphabetic",
    "Other_Default_Ignorable_Code_Point",
    "Other_Grapheme_Extend",
    "Other_ID_Continue",
    "Other_ID_Start",
    "Other_Lowercase",
    "Other_Math",
    "Other_Uppercase",
    "Pattern_Syntax",
    "Pattern_White_Space",
    "Quotation_Mark",
    "Radical",
    "Soft_Dotted",
    "STerm",
    "Terminal_Punctuation",
    "Unified_Ideograph",
    "Variation_Selector",
    "White_Space"
};
unsigned const property_name_size =
    sizeof property_name / sizeof property_name[0];

// Return true if strings are equal, false otherwise.
// NULL is treated as a string that is only equal
// to itself.
//
inline bool eq ( const char * s1, const char * s2 )
{
    if ( s1 == NULL )  return s2 == NULL;
    else if ( s2 == NULL ) return false;
    else return strcmp ( s1, s2 ) == 0;
}

// Ditto, but for ustring's for which columns == 0 is
// possible.  The ustring_chars values determine the
// length and columns uniquely.
//
inline bool ustring_eq
    ( const ustring * s1, const ustring * s2 )
{
    if ( s1 == NULL )  return s2 == NULL;
    else if ( s2 == NULL ) return false;
    else return strcmp ( ustring_chars ( s1 ),
                         ustring_chars ( s2 ) ) == 0;
}

// Return true if index[.] values i1 and i2 have
// identical values in all the tables and so can be
// merged.
//
inline bool index_eq ( unsigned i1, unsigned i2 )
{
    return
           eq ( category[i1], category[i2] )
        && combining_class[i1] == combining_class[i2]
        && eq ( bidi_class[i1], bidi_class[i2] )
        && numerator[i1] == numerator[i2]
        && denominator[i1] == denominator[i2]
	// Numeric value is determined by
	// numerator/denominator.
        && bidi_mirrored[i1] == bidi_mirrored[i2]
        && properties[i1] == properties[i2]
        && ustring_eq ( name[i1], name[i2] )
        && ustring_eq ( picture[i1], picture[i2] )
        && eq ( support_set[i1], support_set[i2] );
}

// Print data for index i, and list character codes c
// with i == index[c].
//
char line[502];  // Also used in make_unicode_data.cc.
const unsigned line_width = 80;
unsigned indent;
char * linep;
void putprop ( ostream & out, const char * prop )
{
    unsigned length = strlen ( prop );
    unsigned column = linep - line;
    if ( column > indent )
        * linep ++ = ',', ++ column;
    if ( column + length + 2 > line_width )
    {
	* linep = 0;
	out << line << endl;
	linep = line;
	for ( unsigned i = 0; i < indent; ++ i )
	    * linep ++ = ' ';
    }
    * linep ++ = ' ';
    strcpy ( linep, prop );
    linep += strlen ( linep );
}

void print_index ( unsigned i, ostream & out = cout )
{
    assert ( i < index_limit );

    char combining[20] = "";
    if ( combining_class[i] != -1 ) 
        sprintf ( combining, "%4d",
	          combining_class[i] );
    sprintf ( line,
    	      "%4d"	// i
	      " %-4s"	// name
	      " %s"	// picture
	      " %-8s"	// support_set
	      " %-3s"	// category
	      "%4s"	// combining
	      " %-4s"	// bidi_class
	      " %c",	// bidi_mirrored
	      i,
	      name[i] == NULL ? " " :
	          ustring_chars ( name[i] ),
	      picture[i] == NULL ? " " :
	          ustring_chars ( picture[i] ),
	      support_set[i] == NULL ? "" :
	          support_set[i],
	      category[i] == NULL ? "" :
	          category[i],
	      combining,
	      bidi_class[i] == NULL ? "" :
		  bidi_class[i],
	      bidi_mirrored[i] == 0 ? ' ' :
		  bidi_mirrored[i] );

    indent = strlen ( line );
    linep = line + indent;

    if ( denominator[i] != 0 )
    {
	char buffer[100];
	if ( denominator[i] == 1 )
	    sprintf ( buffer, "%.15g", numerator[i] );
	else
	    sprintf ( buffer, "%.15g/%.15g",
	              numerator[i], denominator[i] );
	putprop ( out, buffer );
    }
    for ( unsigned j = 0; j < property_name_size; ++ j )
    {
        if ( ( properties[i] & ( 1 << j ) ) != 0 )
	    putprop ( out, property_name[j] );
    }
    for ( Uchar c = 0; c < index_size; ++ c )
    {
	char buffer[10];
        if ( i != index[c] ) continue;
	sprintf ( buffer, "%02X", c );
	putprop ( out, buffer );
    }
 
    * linep = 0;
    out << line << endl;
}

void dump ( const char * filename )
{
    ofstream out ( filename );
    if ( ! out )
    {
        cout << "ERROR: could not open " << filename
	     << " for output" << endl;
	exit ( 1 );
    }

    out << std::setprecision ( 100 );
        // Output double integers of arbitrary
	// length.

    out << "==================== UNICODE DATA DUMP:"
	<< endl;

    out << endl;
    for ( unsigned i = 0; i < index_limit; ++ i )
        print_index ( i, out );

    out.close();
}

// Output:    `    /* [0x...] = ['.'] */ '
//
// where 0x... is c in hex and '.' if c as an ASCII
// graphic character, with the ` = ['.']' being replaced
// by spaces if c is not an ASCII graphic character.
// n is the number of hex digits to be output.  Return
// `out'.
//
ostream & index_comment
	( ostream & out, Uchar c, int n )
{
    char buffer[100];
    sprintf ( buffer, "0x%0*X", n, c );
    out << "    /* [" << buffer;

    if ( c < 127 && isgraph ( c ) )
	out << "] = ['" << (char) c << "'] */ ";
    else
	out << "]         */ ";
    return out;
}

// Output values for a vector of const char *'s that are
// presumed to contain only graphic ASCII characters.
//
void output ( ostream & out,
              const char * const * vector )
{
    const char * finish = "";
    for ( unsigned i = 0; i < index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	if ( vector[i] == NULL ) out << "NULL";
	else out << '"' << vector[i] << '"';
    }
    out << endl;
}

// Output values for a vector of ustrings.
//
void ustring_output
	( ostream & out,
	  const ustring * const * vector )
{
    const char * finish = "";
    for ( unsigned i = 0; i < index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	if ( vector[i] == NULL ) out << "NULL";
	else
	{
	    char buffer[100];
	    sprintf ( buffer, "\"\\x%02X\\x%02X\"",
	              ustring_length ( vector[i] ),
	              ustring_columns ( vector[i] ) );
	    out << "(const ustring *) " << buffer
	        << " \"";
	    const char * p =
	        ustring_chars ( vector[i] );
	    const char * q = p;
	    while ( 0x20 < * q && * q < 0x7F ) ++ q;
	    if ( * q == 0 )
	        out << p;
	    else
	    {
	        while ( * p )
		{ 
		    sprintf ( buffer, "\\x%02X",
		              (unsigned)
			      (unsigned char) * p ++ );
		    out << buffer;
		}
	    }
	    out << '"';
	}
    }
    out << endl;
}

// Output values for a vector of char's.
//
void output ( ostream & out, const char * vector )
{
    const char * finish = "";
    for ( unsigned i = 0; i < index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	if ( vector[i] == 0 ) out << "0";
	else out << "'" << vector[i] << "'";
    }
    out << endl;
}

// Output values for a vector of short's.
//
void output ( ostream & out, const short * vector )
{
    const char * finish = "";
    for ( unsigned i = 0; i < index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	out << vector[i];
    }
    out << endl;
}

// Output values for a vector of unsigned's.
//
void output ( ostream & out, const unsigned * vector )
{
    const char * finish = "";
    for ( unsigned i = 0; i < index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	out << vector[i];
    }
    out << endl;
}

// Output values for a vector of unsigned long long's
// each containing at most b bits.
//
void output ( ostream & out,
              const unsigned long long * vector,
	      unsigned b )
{
    char format[20];
    sprintf ( format, "0x%%0%dX", ( b + 3 ) / 4 );

    const char * finish = "";
    for ( unsigned i = 0; i < index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	char buffer[100];
	sprintf ( buffer, format, vector[i] );
	out << buffer;
    }
    out << endl;
}

// Output values for a vector of double's that contain
// integer values.
//
void output ( ostream & out, const double * vector )
{
    const char * finish = "";
    for ( unsigned i = 0; i < index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	char buffer[100];
	sprintf ( buffer, "%.15g", vector[i] );
	out << buffer;
    }
    out << endl;
}

// Output fraction values for a vector of double's whose
// contents are numerator/denominator or NaN if
// denominator == 0.
//
void output ( ostream & out,
              const double * numerator,
	      const double * denominator )
{
    const char * finish = "";
    for ( unsigned i = 0; i < index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	char buffer[100];
	if ( denominator[i] == 0 ) out << "NAN";
	else
	{
	    sprintf ( buffer, "%.15g", numerator[i] );
	    out << buffer;
	    if ( denominator[i] != 1 )
	    {
		sprintf ( buffer, ".0/%.15g",
		          denominator[i] );
	        out << buffer;
	    }
	}
    }
    out << endl;
}

void output ( const char * filename )
{
    ofstream out ( filename );
    if ( ! out )
    {
        cout << "ERROR: could not open " << filename
	     << " for output" << endl;
	exit ( 1 );
    }

    out << std::setprecision ( 100 );
        // Output double integers of arbitrary
	// length.

    out <<
      "// UNICODE Character Data\n"
      "//\n"
      "// File:	unicode_data.cc\n"
      "\n"
      "// Generated by make_unicode_data.cc\n";

    out <<
      "\n"
      "// UNICODE_INDEX is the list of element values\n"
      "// of the `index' vector and UNICODE_INDEX_SIZE"
      						"\n"
      "// is the size of the vector.\n";

    out << endl << "# define UNICODE_INDEX_SIZE 0x"
        << hex << index_size << dec
	<< endl;

    out << endl << "# define UNICODE_INDEX";
    const char * finish = "";
    for ( Uchar c = 0; c < index_size; ++ c )
    {
        out << finish << " \\" << endl;
	finish = ",";

	index_comment ( out, c, 8 ) << index[c];
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_INDEX_LIMIT is size of various\n"
      "// vectors below.\n";

    out << endl << "# define UNICODE_INDEX_LIMIT "
        << index_limit << endl;

    out <<
      "\n"
      "// UNICODE_CATEGORY is the list of element\n"
      "// values of the `category' vector whose size\n"
      "// is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_CATEGORY";
    output ( out, category );

    out <<
      "\n"
      "// UNICODE_COMBINING_CLASS is the list of\n"
      "// element values of the `combining_class'\n"
      "// vector whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_COMBINING_CLASS";
    output ( out, combining_class );

    out <<
      "\n"
      "// UNICODE_BIDI_CLASS is the list of element\n"
      "// values of the `bidi_class' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_BIDI_CLASS";
    output ( out, bidi_class );

    out <<
      "\n"
      "// UNICODE_NUMERATOR is the list of element\n"
      "// values of the `numerator' vector whose size\n"
      "// is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_NUMERATOR";
    output ( out, numerator );

    out <<
      "\n"
      "// UNICODE_DENOMINATOR is the list of element\n"
      "// values of the `denominator' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_DENOMINATOR";
    output ( out, denominator );

    out <<
      "\n"
      "// UNICODE_NUMERIC_VALUE is the list of\n"
      "// element values of the `numeric_value'\n"
      "// vector whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_NUMERIC_VALUE";
    output ( out, numerator, denominator );

    out <<
      "\n"
      "// UNICODE_BIDI_MIRRORED is the list of"
      					" element\n"
      "// values of the `bidi_mirrored' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_BIDI_MIRRORED";
    output ( out, bidi_mirrored );

    out <<
      "\n"
      "// UNICODE_PROPERTIES is the list of element\n"
      "// values of the `properties' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_PROPERTIES";
    output ( out, properties, property_name_size );

    out <<
      "\n"
      "// UNICODE_NAME is the list of element\n"
      "// values of the `name' vector whose size\n"
      "// is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_NAME";
    ustring_output ( out, name );

    out <<
      "\n"
      "// UNICODE_PICTURE is the list of element\n"
      "// values of the `picture' vector whose size\n"
      "// is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_PICTURE";
    ustring_output ( out, picture );

    out <<
      "\n"
      "// UNICODE_SUPPORT_SET is the list of element\n"
      "// values of the `support_set' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_SUPPORT_SET";
    output ( out, support_set );

    out <<
      "\n"
      "// UNICODE_REFERENCE_COUNT is the list of\n"
      "// element values of the `reference_count'\n"
      "// vector whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_REFERENCE_COUNT";
    output ( out, reference_count );

    out.close();
}

// Do some integrity checking of the tables.
//
void final_check ( void )
{
    unsigned count[index_limit];
    for ( unsigned i = 0;
          i < index_limit; ++ i )
        count[i] = 0;
    for ( Uchar c = 0; c < index_size; ++ c )
    {
        unsigned i = index[c];
	assert ( i < index_limit );
        ++ count[i];
    }

    for ( unsigned i = 0; i < index_limit; ++ i )
    {
	for ( unsigned i2 = i + 1;
	      i2 < index_limit; ++ i2 )
	{
	    if ( i2 < 256 ) continue;
	    if ( ! index_eq ( i, i2 ) ) continue;
	        
	    cout << "FINAL CHECK ERROR: "
	         << "duplicate indices:" << endl;
	    print_index ( i );
	    print_index ( i2 );
	}

	const char * error = NULL;

	// Check numeric value.
	//
	if ( denominator[i] == 0
	     &&
	     numerator[i] != 0 )
	    error = "denominator == 0 and numerator"
	            " != 0";
	else if ( denominator[i] == 0
	          &&
		  ! isnan ( numeric_value[i] ) )
	    error = "denominator == 0 and numeric_value"
	            " is not NaN";
	else if ( denominator[i] != 0
	          &&
		  isnan ( numeric_value[i] ) )
	    error = "denominator != 0 and numeric_value"
	            " is NaN";
	else if ( denominator[i] != 0
	          &&
		     numeric_value[i]
		  != numerator[i] / denominator[i] )
	    error = "numeric_value !="
	            " numerator / denominator";

	if ( count[i] != reference_count[i] )
	    error = "wrong reference_count";

	if ( error != NULL )
	{
	    cout << "FINAL CHECK ERROR: " << error
	         << ":" << endl << "in ";
	    print_index ( i );
	}
    }
}
