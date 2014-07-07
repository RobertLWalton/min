// MIN UNICODE Data Base Test Program
//
// File:	min_unicode_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jul  7 01:45:16 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

//	Setup
//	Main

// Setup
// -----

# include <min_unicode.h>
# define UNI min::unicode

# include <iostream>
# include <iomanip>
# include <fstream>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <cctype>
# include <cassert>
using std::cout;
using std::endl;
using std::setw;
using std::hex;
using std::dec;
using std::ostream;
using std::ofstream;
using UNI::Uchar;
using UNI::utf8_to_unicode;
using UNI::unicode_to_utf8;
using UNI::Ustring;
using UNI::Ustring_length;
using UNI::Ustring_columns;
using UNI::Ustring_chars;
using UNI::unicode_index;
using UNI::unicode_index_size;
using UNI::unicode_index_limit;

using UNI::unicode_category;
using UNI::unicode_Uname;
using UNI::unicode_name;
using UNI::unicode_Upicture;
using UNI::unicode_picture;
using UNI::unicode_numerator;
using UNI::unicode_denominator;
using UNI::unicode_numeric_value;
using UNI::unicode_reference_count;

using UNI::unicode_Ustrings_size;
using UNI::unicode_Ustrings;

using UNI::unicode_category_limit;
using UNI::unicode_category_name;
using UNI::unicode_category_description;

# include "../unicode/output_unicode_data.cc"

int main ( int argc, const char ** argv )
{
    assert ( sizeof ( UNI::Uchar ) == 4 );

    output ( "min_unicode_test.code" );
    dump ( "min_unicode_test.dump" );

    return 0;
}
