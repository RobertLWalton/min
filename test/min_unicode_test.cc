// MIN UNICODE Data Base Test Program
//
// File:	min_unicode_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jul 21 14:36:38 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

//	Setup
//	Main

// Setup
// -----

# define index INDEX
    // Avoid use of `index' in <cstring>

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

# undef index

# include <min_unicode.h>
# define UNI min::unicode

using UNI::Uchar;
using UNI::utf8_to_unicode;
using UNI::unicode_to_utf8;

using UNI::ustring;
using UNI::ustring_length;
using UNI::ustring_columns;
using UNI::ustring_chars;

using UNI::ss_support_sets_size;
using UNI::ss_support_sets_name;
using UNI::ss_support_sets_shift;

using UNI::cc_support_sets_size;
using UNI::cc_support_sets_name;
using UNI::cc_support_sets_mask;

using UNI::index_size;
using UNI::index;
using UNI::index_limit;

using UNI::category;
using UNI::combining_class;
using UNI::bidi_class;
using UNI::numerator;
using UNI::denominator;
using UNI::numeric_value;
using UNI::bidi_mirrored;
using UNI::properties;
using UNI::name;
using UNI::picture;
using UNI::support_sets;
using UNI::reference_count;

# include "../unicode/output_unicode_data.cc"

int main ( int argc, const char ** argv )
{
    assert ( sizeof ( UNI::Uchar ) == 4 );

    output_data ( "unicode_data.cc" );
    output_support_sets ( "unicode_data_support_sets.h" );
    dump ( "min_unicode_test.dump" );

    return 0;
}
