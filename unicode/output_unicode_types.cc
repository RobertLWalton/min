// MIN Routines to Output UNICODE Type Tables
//
// File:	output_unicode_types.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jul  1 12:32:50 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// This file contains routines to check and output the
// MIN UNICODE database.  This file is to be `#include'd
// in other files such as make_unicode_types.cc and
// min_unicode_test.cc.

// Print unicode_categories.
//
void print_categories ( ostream & out = cout )
{
    out << endl
        << "CATEGORIES:"
	<< endl
	<< endl;
    for ( unsigned i = 0; i < unicode_categories_size;
                          ++ i )
    {
        const unicode_category & cat =
	    unicode_categories[i];
        out << ( cat.unicode_name == NULL ? "  " :
	         cat.unicode_name )
	    << "  " << cat.category
	    << "  " << cat.unicode_description
	    << endl;
    }
}

// Print unicode_types[i], list the unicode_index
// entries pointing at it, and print its name (if any)
// from unicode_names.
//
void print_unicode_type
	( unsigned t, ostream & out = cout )
{
    assert ( t < unicode_types_size );
    const unicode_type & type = unicode_types[t];

    out << "type " << t << ":" << endl;
    if ( type.name_length > 0 )
    {
        out << "       name = `";
	for ( unsigned i = 0; i < type.name_length;
	                      ++ i )
	{
	    Uchar c = unicode_names[type.name+i];
	    assert ( ' ' < c && c < 0xFF );
	        // Currently only ASCII names are
		// supported.
	    out << (char) c;
	}
	out << "'" << endl;
    }
    char category[100];
    sprintf ( category, "0x%02X", type.category );
    if (    type.category < 127
         && isgraph ( type.category ) )
    {
	char * p = category + strlen ( category );
	sprintf ( p, " = %c", (char) type.category );
    }

    out << "       name_length = "
	<< (int) type.name_length << endl
	<< "       name_columns = "
	<< (int) type.name_columns << endl
	<< "       numerator = "
	<< type.numerator << endl
	<< "       denominator = "
	<< type.denominator << endl
	<< "       category = "
	<< category << endl
	<< "       reference count = "
	<< type.reference_count << endl;

    unsigned count = 0;
    for ( Uchar c = 0; c < unicode_index_size; ++ c )
    {
	if ( t != unicode_index[c] ) continue;

	Uchar c2 = c + 1;
	while ( c2 < unicode_index_size
		&&
		t == unicode_index[c2] )
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
    if ( count != type.reference_count )
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
    out << "==================== UNICODE DATA DUMP:"
	<< endl;

    out << endl;
    print_categories ( out );

    for ( unsigned t = 0; t < unicode_types_size; ++ t )
    {
	out << endl;
        print_unicode_type ( t, out );
    }

    out.close();
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

    out <<
      "// UNICODE Character Type Data\n"
      "//\n"
      "// File:	unicode_types.cc\n"
      "\n"
      "// Generated by make_unicode_types.cc\n";

    out <<
      "\n"
      "// UNICODE_CATEGORIES is the list of element\n"
      "// values of the unicode_categories vector\n"
      "// and UNICODE_CATEGORIES_SIZE is the size of\n"
      "// the vector.\n";

    out << endl
        << "# define UNICODE_CATEGORIES_SIZE "
        << unicode_categories_size << endl;

    out << endl << "# define UNICODE_CATEGORIES";
    for ( unsigned j = 0; j < unicode_categories_size;
                          ++ j )
    {
	const unicode_category & d =
	    unicode_categories[j];
	if ( j != 0 ) out << ",";
	out << " \\\n    { ";
	if ( d.unicode_name == NULL )
	    out << "NULL";
	else out << '"' << d.unicode_name << '"';
	out << ", '" << d.category << "', "
	    << '"' << d.unicode_description << '"'
	    << " }";
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_TYPES is the list of element values\n"
      "// of the unicode_types vector and UNICODE_\n"
      "// TYPES_SIZE is the size of the vector.\n";

    out << endl << "# define UNICODE_TYPES_SIZE "
        << unicode_types_size << endl;

    out << endl << "# define UNICODE_TYPES";

    for ( unsigned t = 0; t < unicode_types_size; ++ t )
    {
	const unicode_type & type = unicode_types[t];
	char category[20];
	sprintf ( category, "%02X",
	          (short) type.category );
	if ( t != 0 ) out << ",";
        out << " \\\n    { 0x" << category
	    << ", " << type.name
	    << ", " << (short) type.name_length
	    << ", " << (short) type.name_columns
	    << ", " << type.numerator
	    << ", " << type.denominator
	    << ", " << type.reference_count
	    << " }";
    }
    out << endl;

    out <<
      "\n"
      "// UNICODE_NAMES is the list of element values\n"
      "// of the unicode_names vector and UNICODE_\n"
      "// NAMES_SIZE is the size of the vector.\n";

    out << endl << "# define UNICODE_NAMES_SIZE "
        << unicode_names_size << endl;

    out << endl << "# define UNICODE_NAMES ";
    for ( unsigned i = 0; i < unicode_names_size; ++ i )
    {
	char buf[20];
	Uchar c = unicode_names[i];
	if ( ' ' <= c && c < 0xFF )
	    sprintf ( buf, "'%c'", (char) c );
	else
	    sprintf ( buf, "%02X", c );

	if ( i != 0 ) out << ", ";
        if ( i % 5 == 0 ) out << "\\\n    ";
	out << setw ( 5 ) << buf;
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

    out << endl << "# define UNICODE_INDEX ";
    for ( Uchar c = 0; c < unicode_index_size; ++ c )
    {
	if ( c != 0 ) out << ", ";
        if ( c % 8 == 0 ) out << "\\\n    ";
	out << setw ( 3 ) << unicode_index[c];
    }
    out << endl;

    out.close();
}

// Do some integrity checking of the tables.
//
void final_check ( void )
{
    unsigned count[unicode_types_size];
    for ( unsigned i = 0; i < unicode_types_size; ++ i )
        count[i] = 0;
    for ( Uchar c = 0; c < unicode_index_size; ++ c )
    {
        unsigned i = unicode_index[c];
	assert ( i < unicode_types_size );
        ++ count[i];
    }
    for ( unsigned i = 0; i < unicode_types_size; ++ i )
    {
	const unicode_type & type = unicode_types[i];
        if ( count[i] == type.reference_count
	     &&
	     ( count[i] > 0 || i == 0 )
	     &&
	     ( type.denominator > 0
	       ||
	       type.numerator ==  0 )
	     &&
	     type.name_columns <= type.name_length )
           continue;

	cout << "ERROR: in final check of ";
	print_unicode_type ( i );

	for ( unsigned i2 = 0; i2 < i; ++ i2 )
	{
	    const unicode_type & type2 =
	        unicode_types[i2];
	    if ( type.category != type2.category )
	        continue;
	    if ( type.name_length != type2.name_length )
	        continue;
	    if (    memcmp ( unicode_names + type.name,
	                     unicode_names + type2.name,
			       type.name_length
			     * sizeof ( Uchar ) )
	         != 0 )
	        continue;
	    if ( type.denominator != type2.denominator )
	        continue;
	    if ( type.numerator != type2.numerator )
	        continue;

	    cout << "ERROR: in final check; two"
	            "distinct types match:" << endl;
	    cout << "  (1) ";
	    print_unicode_type ( i );
	    cout << "  (2) ";
	    print_unicode_type ( i2 );
	}
    }
}