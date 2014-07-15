// MIN Routines to Output UNICODE Data Tables
//
// File:	output_unicode_data.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jul 15 12:53:47 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This file contains routines to check and output the
// MIN UNICODE database.  This file is to be `#include'd
// in other files such as make_unicode_data.cc and
// min_unicode_test.cc.

# include <cmath>

// Print unicode_categories.
//
void print_categories ( ostream & out = cout )
{
    out << endl
        << "CATEGORIES:"
	<< endl
	<< endl;
    for ( unsigned cat = 0; cat < 127; ++ cat )
    {
        if ( ! isalpha ( cat ) )
	    continue;
	if ( unicode_category_description[cat] == NULL )
	    continue;

        out << "    " << (char) cat << " "
	    << ( unicode_category_name[cat] == NULL ?
	         "  " : unicode_category_name[cat] )
	    << "  " << unicode_category_description[cat]
	    << endl;
    }
}

// Print unicode_supported_sets.
//
void print_supported_sets ( ostream & out = cout )
{
    out << endl
        << "SUPPORTED CHARACTER SETS:"
	<< endl
	<< endl;
    for ( unsigned i = 0;
          i < unicode_supported_set_limit; ++ i )
    {
	if ( unicode_supported_set[i] == NULL )
	    continue;

	char buffer[20];
	sprintf ( buffer, "0x%X = %2d", i, i );

        out << "    " << buffer << ":  "
	    << unicode_supported_set[i]
	    << endl;
    }
}

// Print data for index i, and list character codes c
// with i == unicode_index[c].
//
void print_index ( unsigned i, ostream & out = cout )
{
    assert ( i < unicode_index_limit );

    out << "index " << i << ":" << endl;
    if ( unicode_name[i] != NULL )
        out << "       name = `"
	    << ustring_chars ( unicode_name[i] )
	    << "'" << endl;
    if ( unicode_picture[i] != NULL )
        out << "       picture = `"
	    << ustring_chars ( unicode_picture[i] )
	    << "'" << endl;

    char catname[100];
    unsigned char cat = unicode_category[i];
    sprintf ( catname, "0x%02X", cat );
    if ( cat < 127 && isgraph ( cat ) )
    {
	char * p = catname + strlen ( catname );
	sprintf ( p, " = %c", (char) cat );
    }

    out << "       numerator/denominator = "
	<< unicode_numerator[i]
	<< "/" << unicode_denominator[i]
	<< endl
	<< "       category = "
	<< catname << endl
	<< "       reference count = "
	<< unicode_reference_count[i] << endl;

    unsigned count = 0;
    for ( Uchar c = 0; c < unicode_index_size; ++ c )
    {
	if ( i != unicode_index[c] ) continue;

	Uchar c2 = c + 1;
	while ( c2 < unicode_index_size
		&&
		i == unicode_index[c2] )
	    ++ c2;

	count += (c2 - c );
	char buffer[200];
	if ( c2 == c + 1 )
	    sprintf ( buffer,
	              "       code = %02X", c );
	else
	    sprintf ( buffer,
	              "       codes = %02X..%02X",
		      c, c2-1 );
	out << buffer << endl;

	c = c2;
    }
    if ( count != unicode_reference_count[i] )
        out << "    ERROR: reference count should be"
	       " = " << count << endl;
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
    print_categories ( out );
    out << endl;
    print_supported_sets ( out );

    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
    {
	out << endl;
        print_index ( i, out );
    }

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
	( ostream & out, unsigned c, int n )
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

// Output:    '.' if c is an ASCII graphic
//	      0x.. otherwise
//
// Return `out'.
//
ostream & output ( ostream & out, unsigned c )
{
    if ( c == '\\' )
        out << "'\\\\'";
    else if ( c == '\'' )
        out << "'\\''";
    else if ( c < 127 && isgraph ( c ) )
	out << "'" << (char) c << "'";
    else
    {
	char buffer[20];
	sprintf ( buffer, "0x%02X", c );
	out << buffer;
    }
    return out;
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
      "// UNICODE_CATEGORY_NAME is the list of\n"
      "// element values of the unicode_category_\n"
      "// name vector;  UNICODE_CATEGORY_DESCRIPTION\n"
      "// is a list of element values of the unicode_\n"
      "// category_description vector; and UNICODE_\n"
      "// CATEGORY_LIMIT is the size of either\n"
      "// vector.\n";

    out << endl
        << "# define UNICODE_CATEGORY_LIMIT "
        << unicode_category_limit << endl;

    out << endl
        << "# define UNICODE_CATEGORY_NAME";
    const char * finish = "";
    for ( unsigned cat = 0;
          cat < unicode_category_limit; ++ cat )
    {
        const char * name = unicode_category_name[cat];

        out << finish << " \\" << endl;
	finish = ",";

        index_comment ( out, cat, 2 );
	if ( name == NULL )
	    out << "NULL";
	else
	    out << "\"" << name << "\"";
    }
    out << endl;

    out << endl
        << "# define UNICODE_CATEGORY_DESCRIPTION";
    finish = "";
    for ( unsigned cat = 0;
          cat < unicode_category_limit; ++ cat )
    { 
        const char * description =
	    unicode_category_description[cat];

        out << finish << " \\" << endl;
	finish = ",";

        index_comment ( out, cat, 2 );
	if ( description == NULL )
	    out << "NULL";
	else
	    out << "\"" << description << "\"";
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_SUPPORTED_SET is the vector of\n"
      "// element values of the unicode_supported_set\n"
      "// vector;  UNICODE_SUPPORTED_SET_LIMIT is the\n"
      "// size of the vector; UNICODE_SUPPORTED_SET_\n"
      "// shift is the number of bits to the right\n"
      "// of the supported set vector index in a\n"
      "// unicode_index element: see below.\n";

    out << endl
        << "# define UNICODE_SUPPORTED_SET_LIMIT "
        << unicode_supported_set_limit << endl
        << "# define UNICODE_SUPPORTED_SET_SHIFT "
        << unicode_supported_set_shift << endl;

    out << endl
        << "# define UNICODE_SUPPORTED_SET";
    finish = "";
    for ( unsigned i = 0;
          i < unicode_supported_set_limit; ++ i )
    {
        const char * name = unicode_supported_set[i];

        out << finish << " \\" << endl;
	finish = ",";

	char buffer[20];
	sprintf ( buffer, "0x%02X = %2d", i, i );
	out << "    /* [" << buffer << "] */ ";
	if ( name == NULL )
	    out << "NULL";
	else
	    out << "\"" << name << "\"";
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_INDEX is the list of element values\n"
      "// of the unicode_index vector and UNICODE_\n"
      "// INDEX_SIZE is the size of the vector.\n";

    out << endl << "# define UNICODE_INDEX_SIZE 0x"
        << hex << unicode_index_size << dec
	<< endl;

    out << endl << "# define UNICODE_INDEX";
    finish = "";
    for ( Uchar c = 0; c < unicode_index_size; ++ c )
    {
        out << finish << " \\" << endl;
	finish = ",";

	index_comment ( out, c, 8 ) << unicode_index[c];
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_INDEX_LIMIT is size of various\n"
      "// vectors below.\n";

    out << endl << "# define UNICODE_INDEX_LIMIT "
        << unicode_index_limit << endl;

    out <<
      "\n"
      "// UNICODE_CATEGORY is the list of element\n"
      "// values of the unicode_category vector\n"
      "// whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_CATEGORY";

    finish = "";
    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	output ( out, unicode_category[i] );
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_NAME is the list of element\n"
      "// values of the unicode_name vector\n"
      "// whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_NAME";

    finish = "";
    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	const ustring * n = unicode_name[i];
	if ( n == NULL ) out << "NULL";
	else
	{
	    char buffer[20];
	    unsigned length = ustring_length ( n );
	    sprintf ( buffer, "\\x%02X\\x%02X",
	              length, length );
	    out << "(const ustring *) "
	        << "\"" << buffer << "\" \""
	        << ustring_chars ( n ) << "\"";
	}
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_PICTURE is the list of element\n"
      "// values of the unicode_picture vector\n"
      "// whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_PICTURE";

    finish = "";
    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	const ustring * n = unicode_picture[i];
	if ( n == NULL ) out << "NULL";
	else
	{
	    assert ( ustring_columns ( n ) > 0 );

	    out << "(const ustring *) " << "\"";
	    char buffer[10];
	    while ( * n )
	    {
	        sprintf ( buffer, "\\x%02X",
		          (unsigned)
			  (unsigned char) * n ++ );
		out << buffer;
	    }
	    out << "\"";
	}
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_NUMERATOR is the list of element\n"
      "// values of the unicode_numerator vector\n"
      "// whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_NUMERATOR";

    finish = "";
    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	out << unicode_numerator[i];
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_DENOMINATOR is the list of element\n"
      "// values of the unicode_denominator vector\n"
      "// whose size is UNICODE_INDEX_LIMIT.\n";

    out << endl << "# define UNICODE_DENOMINATOR";

    finish = "";
    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	out << unicode_denominator[i];
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_NUMERIC_VALUE is the list of\n"
      "// element values of the unicode_numeric_\n"
      "// value vector whose size is UNICODE_INDEX_\n"
      "// LIMIT.\n";

    out << endl << "# define UNICODE_NUMERIC_VALUE";

    finish = "";
    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	if ( unicode_denominator[i] == 0 )
	    out << "NAN";
	else
	    out << unicode_numerator[i] << ".0/"
	        << unicode_denominator[i];
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_REFERENCE_COUNT is the list of\n"
      "// element values of the unicode_reference_\n"
      "// count vector whose size is UNICODE_INDEX_\n"
      "// LIMIT.\n";

    out << endl << "# define UNICODE_REFERENCE_COUNT";

    finish = "";
    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
    {
        out << finish << " \\" << endl;
	finish = ",";

	out << "    /* [" << setw ( 3 ) << i << "] */ ";

	out << unicode_reference_count[i];
    }
    out << endl;

    out.close();
}

// Do some integrity checking of the tables.
//
void final_check ( void )
{
    unsigned count[unicode_index_limit];
    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
        count[i] = 0;
    for ( Uchar c = 0; c < unicode_index_size; ++ c )
    {
        unsigned i = unicode_index[c];
	assert ( i < unicode_index_limit );
        ++ count[i];
    }

    for ( unsigned i = 0;
          i < unicode_index_limit; ++ i )
    {
	for ( unsigned i2 = 0; i2 < i; ++ i2 )
	{
	    if (    unicode_category[i]
	         != unicode_category[i2] )
	        continue;
	    if ( unicode_name[i] == NULL ?
	             unicode_name[i2] != NULL :
	         unicode_name[i2] == NULL ? true :
		    strcmp ( ustring_chars
		                 ( unicode_name[i] ),
		             ustring_chars
			         ( unicode_name[i2] ) )
		 != 0 )
	        continue;
	    if (    unicode_numerator[i]
	         != unicode_numerator[i2] )
	        continue;
	    if (    unicode_denominator[i]
	         != unicode_denominator[i2] )
	        continue;
	        
	    cout << "  (1) ";
	    print_index ( i );
	    cout << "  (2) ";
	    print_index ( i2 );
	}

	const char * error = NULL;

	// Check numeric value.
	//
	if ( unicode_denominator[i] == 0
	     &&
	     unicode_numerator[i] != 0 )
	    error = "denominator == 0 and numerator"
	            " != 0";
	else if ( unicode_denominator[i] == 0
	          &&
		  ! isnan ( unicode_numeric_value[i] ) )
	    error = "denominator == 0 and numeric_value"
	            " is not NaN";
	else if ( unicode_denominator[i] != 0
	          &&
		  isnan ( unicode_numeric_value[i] ) )
	    error = "denominator != 0 and numeric_value"
	            " is NaN";
	else if ( unicode_denominator[i] != 0
	          &&
		     unicode_numeric_value[i]
		  !=   unicode_numerator[i]
		     / unicode_denominator[i] )
	    error = "numeric_value !="
	            " numerator / denominator";

	if ( count[i] != unicode_reference_count[i] )
	    error = "wrong reference_count";

	if ( error != NULL )
	{
	    cout << "FINAL CHECK ERROR: " << error
	         << ":" << endl << "in ";
	    print_index ( i );
	}
    }
}
