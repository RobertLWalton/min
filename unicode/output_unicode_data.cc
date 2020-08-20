// MIN Routines to Output UNICODE Data Tables
//
// File:	output_unicode_data.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Aug 20 14:06:37 EDT 2020
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
    "Prepended_Concatenation_Mark",
    "Regional_Indicator",
    "Quotation_Mark",
    "Radical",
    "Sentence_Terminal",
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
inline bool ustring_eq ( ustring s1, ustring s2 )
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
// WARNING: as this function is used by make_unicode_
// data.cc, indexes must be uns32's and NOT uns16's.
//
inline bool index_eq ( uns32 i1, uns32 i2 )
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
        && support_sets[i1] == support_sets[i2];
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

void print_ss_support_set
	( uns32 i, ostream & out = cout )
{
    assert ( i < ss_support_sets_size );

    uns8 k = ss_support_sets_shift[i];
    sprintf ( line, "%12s = ( 1 << %2d ):",
                    ss_support_sets_name[i], k );
    indent = strlen ( line );
    linep = line + indent;

    for ( uns32 j = 0; j < cc_support_sets_size;
                          ++ j )
    {
        if ( cc_support_sets_mask[j] & ( 1 << k ) )
	    putprop ( out, cc_support_sets_name[j] );
    }
 
    * linep = 0;
    out << line << endl;
}

void print_index ( uns16 i, ostream & out = cout )
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
	      " %-3s"	// category
	      "%4s"	// combining
	      " %-4s"	// bidi_class
	      " %c",	// bidi_mirrored
	      i,
	      name[i] == NULL ? " " :
	          ustring_chars ( name[i] ),
	      picture[i] == NULL ? " " :
	          ustring_chars ( picture[i] ),
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

    for ( uns32 j = 0; j < ss_support_sets_size; ++ j )
    {
	uns8 k = ss_support_sets_shift[j];
        if ( ( support_sets[i] & ( 1 << k ) ) != 0 )
	    putprop ( out, ss_support_sets_name[j] );
    }
    for ( unsigned j = 0; j < property_name_size; ++ j )
    {
        if ( ( properties[i] & ( 1ull << j ) ) != 0 )
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

    out << endl
        << "===== SUPPORT SETS:"
	<< endl;

    out << endl;
    for ( uns32 i = 0; i < ss_support_sets_size;
                          ++ i )
        print_ss_support_set ( i, out );

    out << endl
        << "===== INDEXED DATA:"
	<< endl;

    out << endl;
    for ( uns16 i = 0; i < index_limit; ++ i )
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
              const char * const * vector,
	      unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
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
	  ustring const * vector,
	  unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
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
	    out << "(ustring) " << buffer
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

// Output values for a vector of extra_name's.
//
void extra_name_output
	( ostream & out,
	  extra_name const * vector,
	  unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";
        extra_name const & e = vector[i];
	char buffer[200];
	char name[100];
	sprintf ( name, "\"%s\"",
	          ustring_chars ( e.name ) );
	sprintf ( buffer, "    { (ustring)"
	                  " \"\\x%02X\\x%02X\""
	                  " %12s, 0x%08X }",
		  ustring_length ( e.name ),
		  ustring_columns ( e.name ),
	          name, e.c );
	out << buffer;
    }
    out << endl;
}

// Output values for a vector of char's.
//
void output ( ostream & out, const char * vector,
	      unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	if ( vector[i] == 0 ) out << "0";
	else out << "'" << vector[i] << "'";
    }
    out << endl;
}

// Output values for a vector of uns8's.
//
void output ( ostream & out,
              const uns8 * vector,
	      unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	out << (unsigned) vector[i];
    }
    out << endl;
}

// Output values for a vector of short's.
//
void output ( ostream & out, const short * vector,
	      unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	out << vector[i];
    }
    out << endl;
}

// Output values for a vector of uns32's.
//
void output ( ostream & out, const uns32 * vector,
	      unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	out << vector[i];
    }
    out << endl;
}

// Output values for a vector of uns64's
// each containing at most b bits.
//
void output ( ostream & out,
              const uns64 * vector,
	      unsigned b,
	      unsigned size )
{
    char format[20];
    sprintf ( format, "0x%%0%dllX", ( b + 3 ) / 4 );

    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
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

// Output values for a vector of uns32's
// each containing at most b bits.
//
void output ( ostream & out,
              const uns32 * vector,
	      unsigned b,
	      unsigned size )
{
    char format[20];
    sprintf ( format, "0x%%0%dX", ( b + 3 ) / 4 );

    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
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
void output ( ostream & out, const double * vector,
	      unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
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
	      const double * denominator,
	      unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
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

// Output Uchar vector.
//
void output_Uchar
	( ostream & out, const Uchar * character,
	                 unsigned size )
{
    const char * finish = "";
    for ( unsigned i = 0; i < size; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	char buffer[100];
	sprintf ( buffer, "0x%08X", character[i] );
	out << buffer;
    }
    out << endl;
}

// Output unicode_data_support_sets.h style file.
//
void output_support_sets ( const char * filename )
{
    ofstream out ( filename );
    if ( ! out )
    {
        cout << "ERROR: could not open " << filename
	     << " for output" << endl;
	exit ( 1 );
    }

    out <<
      "// UNICODE Support Set Definitions\n"
      "//\n"
      "// File:	" << filename << "\n"
      "\n"
      "// Generated by make_unicode_data.cc\n"
      "\n"
      "// See unicode_data.h for documentation.\n"
      "\n"
      "\n"
      "enum {\n";

    const char * first = "";
    for ( unsigned i = 0; i < ss_support_sets_size;
                          ++ i )
    {
	out << first << endl;
	first = ",";
        char buffer[100];
	sprintf ( buffer, "    %-16s = ( 1 << %2d )",
		  ss_support_sets_name[i],
		  ss_support_sets_shift[i] );
	out << buffer;
    }

    out << endl <<
      "\n"
      "};\n";

    out.close();
}

// Output unicode_data.cc style file.
//
void output_data ( const char * filename )
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
      "// File:	" << filename << "\n"
      "\n"
      "// Generated by make_unicode_data.cc\n";

    out <<
      "\n"
      "// UNICODE_SS_SUPPORT_SETS_NAME/SHIFT are\n"
      "// lists of element values giving the names\n"
      "// and shifts of ss support sets, and UNICODE_\n"
      "// SS_SUPPORT_SETS_SIZE is the size of these\n"
      "// vectors.\n";

    out << endl
        << "# define UNICODE_SS_SUPPORT_SETS_SIZE "
        << ss_support_sets_size << endl;

    out << endl
        << "# define UNICODE_SS_SUPPORT_SETS_NAME";
    output ( out, ss_support_sets_name,
    		  ss_support_sets_size );

    out << endl
        << "# define UNICODE_SS_SUPPORT_SETS_SHIFT";
    output ( out, ss_support_sets_shift,
    		  ss_support_sets_size );

    out <<
      "\n"
      "// UNICODE_CC_SUPPORT_SETS_NAME/MASK are\n"
      "// lists of element values giving the names\n"
      "// and masks of cc support sets, and UNICODE_\n"
      "// CC_SUPPORT_SETS_SIZE is the size of these\n"
      "// vectors.\n";

    out << endl
        << "# define UNICODE_CC_SUPPORT_SETS_SIZE "
        << cc_support_sets_size << endl;

    out << endl
        << "# define UNICODE_CC_SUPPORT_SETS_NAME";
    output ( out, cc_support_sets_name,
    		  cc_support_sets_size );

    out << endl
        << "# define UNICODE_CC_SUPPORT_SETS_MASK";
    output ( out, cc_support_sets_mask, 32,
    		  cc_support_sets_size );


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
      "// UNICODE_CHARACTER is the list of element\n"
      "// values of the `character' vector whose size\n"
      "// is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_CHARACTER";
    output_Uchar ( out, character, index_limit );

    out <<
      "\n"
      "// UNICODE_CATEGORY is the list of element\n"
      "// values of the `category' vector whose size\n"
      "// is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_CATEGORY";
    output ( out, category, index_limit );

    out <<
      "\n"
      "// UNICODE_COMBINING_CLASS is the list of\n"
      "// element values of the `combining_class'\n"
      "// vector whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_COMBINING_CLASS";
    output ( out, combining_class, index_limit );

    out <<
      "\n"
      "// UNICODE_BIDI_CLASS is the list of element\n"
      "// values of the `bidi_class' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_BIDI_CLASS";
    output ( out, bidi_class, index_limit );

    out <<
      "\n"
      "// UNICODE_NUMERATOR is the list of element\n"
      "// values of the `numerator' vector whose size\n"
      "// is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_NUMERATOR";
    output ( out, numerator, index_limit );

    out <<
      "\n"
      "// UNICODE_DENOMINATOR is the list of element\n"
      "// values of the `denominator' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_DENOMINATOR";
    output ( out, denominator, index_limit );

    out <<
      "\n"
      "// UNICODE_NUMERIC_VALUE is the list of\n"
      "// element values of the `numeric_value'\n"
      "// vector whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_NUMERIC_VALUE";
    output ( out, numerator, denominator, index_limit );

    out <<
      "\n"
      "// UNICODE_BIDI_MIRRORED is the list of"
      					" element\n"
      "// values of the `bidi_mirrored' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_BIDI_MIRRORED";
    output ( out, bidi_mirrored, index_limit );

    out <<
      "\n"
      "// UNICODE_PROPERTIES is the list of element\n"
      "// values of the `properties' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_PROPERTIES";
    output ( out, properties, property_name_size,
                  index_limit );

    out <<
      "\n"
      "// UNICODE_NAME is the list of element\n"
      "// values of the `name' vector whose size\n"
      "// is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_NAME";
    ustring_output ( out, name, index_limit );

    out <<
      "\n"
      "// UNICODE_EXTRA_NAMES_NUMBER is size of\n"
      "// the UNICODE_EXTRA_NAMES vector below.\n";

    out << endl
        << "# define UNICODE_EXTRA_NAMES_NUMBER "
        << extra_names_number << endl;

    out <<
      "\n"
      "// UNICODE_EXTRA_NAMES is the list of element\n"
      "// values of the `extra_names' vector whose\n"
      "// size is UNICODE_EXTRA_NAMES_NUMBER.\n";

    out << endl << "# define UNICODE_EXTRA_NAMES";
    extra_name_output
        ( out, extra_names, extra_names_number );

    out <<
      "\n"
      "// UNICODE_PICTURE is the list of element\n"
      "// values of the `picture' vector whose size\n"
      "// is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_PICTURE";
    ustring_output ( out, picture, index_limit );

    out <<
      "\n"
      "// UNICODE_SUPPORT_SETS is the list of element\n"
      "// values of the `support_sets' vector whose\n"
      "// size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_SUPPORT_SETS";
    output ( out, support_sets, 32, index_limit );

    out <<
      "\n"
      "// UNICODE_REFERENCE_COUNT is the list of\n"
      "// element values of the `reference_count'\n"
      "// vector whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_REFERENCE_COUNT";
    output ( out, reference_count, index_limit );

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
		  ! std::isnan ( numeric_value[i] ) )
	    error = "denominator == 0 and numeric_value"
	            " is not NaN";
	else if ( denominator[i] != 0
	          &&
		  std::isnan ( numeric_value[i] ) )
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
