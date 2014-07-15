// MIN UNICODE Data
//
// File:	min_unicode.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jul 15 12:45:24 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <cstdlib>
# include <cmath>
# include <min_unicode.h>
# include "../unicode/unicode_data.cc"

# define UNI min::unicode

const unsigned
    UNI::unicode_index_size = UNICODE_INDEX_SIZE;
const unsigned short
    UNI::unicode_index[UNI::unicode_index_size] =
        { UNICODE_INDEX };
const unsigned
    UNI::unicode_index_limit = UNICODE_INDEX_LIMIT;
const unsigned char
    UNI::unicode_category[UNI::unicode_index_limit] =
        { UNICODE_CATEGORY };
const UNI::ustring * const
    UNI::unicode_name[UNI::unicode_index_limit] =
        { UNICODE_NAME };
const UNI::ustring * const
    UNI::unicode_picture[UNI::unicode_index_limit] =
        { UNICODE_PICTURE };
const double
    UNI::unicode_numerator[UNI::unicode_index_limit] =
        { UNICODE_NUMERATOR };
const double
    UNI::unicode_denominator[UNI::unicode_index_limit] =
        { UNICODE_DENOMINATOR };
const double
    UNI::unicode_numeric_value
            [UNI::unicode_index_limit] =
        { UNICODE_NUMERIC_VALUE };
const unsigned
    UNI::unicode_reference_count
            [UNI::unicode_index_limit] =
        { UNICODE_REFERENCE_COUNT };
const char * const
    UNI::unicode_category_name
        [UNI::unicode_category_limit] =
        { UNICODE_CATEGORY_NAME };
const char * const
    UNI::unicode_category_description
	    [UNI::unicode_category_limit] =
	        { UNICODE_CATEGORY_DESCRIPTION };
const char * const
    UNI::unicode_supported_set
	    [UNI::unicode_supported_set_limit] =
	        { UNICODE_SUPPORTED_SET };
