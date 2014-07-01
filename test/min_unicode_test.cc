// MIN UNICODE Data Base Test Program
//
// File:	min_unicode_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jul  1 06:31:23 EDT 2014
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
using UNI::unicode_category;
using UNI::unicode_categories;
using UNI::unicode_categories_size;
using UNI::unicode_names;
using UNI::unicode_names_size;
using UNI::unicode_type;
using UNI::unicode_types;
using UNI::unicode_types_size;
using UNI::unicode_index;
using UNI::unicode_index_size;

# include "../unicode/output_unicode_types.cc"

int main ( int argc, const char ** argv )
{
    assert ( sizeof ( Uchar ) == 4 );

    output ( "min_unicode_test.code" );
    dump ( "min_unicode_test.dump" );

    return 0;
}
