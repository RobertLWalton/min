// MIN UNICODE Data
//
// File:	min_unicode.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun 30 16:32:25 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <cstdlib>
# include <min_unicode.h>
# include "../unicode/unicode_types.cc"

# define UNI min::unicode

const unsigned UNI::unicode_index_size =
    UNICODE_INDEX_SIZE;
const unsigned short UNI::unicode_index[] =
    { UNICODE_INDEX };

const unsigned UNI::unicode_types_size =
    UNICODE_TYPES_SIZE;
extern const UNI::unicode_type UNI::unicode_types[] =
    { UNICODE_TYPES };

const unsigned UNI::unicode_names_size =
    UNICODE_NAMES_SIZE;
const UNI::Uchar UNI::unicode_names[] =
    { UNICODE_NAMES };

const unsigned UNI::unicode_categories_size =
    UNICODE_CATEGORIES_SIZE;
const UNI::unicode_category UNI::unicode_categories[] =
    { UNICODE_CATEGORIES };
