// MIN UNICODE Data
//
// File:	min_unicode.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jul 21 14:36:21 EDT 2014
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
    UNI::ss_support_sets_size =
	UNICODE_SS_SUPPORT_SETS_SIZE;
const char * const
    UNI::ss_support_sets_name
        [UNI::ss_support_sets_size] =
	    { UNICODE_SS_SUPPORT_SETS_NAME };
const unsigned char
    UNI::ss_support_sets_shift
        [UNI::ss_support_sets_size] =
	    { UNICODE_SS_SUPPORT_SETS_SHIFT };

const unsigned
    UNI::cc_support_sets_size =
	UNICODE_CC_SUPPORT_SETS_SIZE;
const char * const
    UNI::cc_support_sets_name
        [UNI::cc_support_sets_size] =
	    { UNICODE_CC_SUPPORT_SETS_NAME };
const unsigned long
    UNI::cc_support_sets_mask
        [UNI::cc_support_sets_size] =
	    { UNICODE_CC_SUPPORT_SETS_MASK };

const unsigned
    UNI::index_size = UNICODE_INDEX_SIZE;
const unsigned short
    UNI::index[UNI::index_size] =
        { UNICODE_INDEX };
const unsigned
    UNI::index_limit = UNICODE_INDEX_LIMIT;
const char * const
    UNI::category[UNI::index_limit] =
        { UNICODE_CATEGORY };
short const
    UNI::combining_class[UNI::index_limit] =
        { UNICODE_COMBINING_CLASS };
const char * const
    UNI::bidi_class[UNI::index_limit] =
        { UNICODE_BIDI_CLASS };
const double
    UNI::numerator[UNI::index_limit] =
        { UNICODE_NUMERATOR };
const double
    UNI::denominator[UNI::index_limit] =
        { UNICODE_DENOMINATOR };
const double
    UNI::numeric_value[UNI::index_limit] =
        { UNICODE_NUMERIC_VALUE };
extern char const
    UNI::bidi_mirrored[UNI::index_limit] =
        { UNICODE_BIDI_MIRRORED };
extern unsigned long long const
    UNI::properties[UNI::index_limit] =
        { UNICODE_PROPERTIES };
const UNI::ustring * const
    UNI::name[UNI::index_limit] =
        { UNICODE_NAME };
const UNI::ustring * const
    UNI::picture[UNI::index_limit] =
        { UNICODE_PICTURE };
extern const unsigned long
    UNI::support_sets[UNI::index_limit] =
        { UNICODE_SUPPORT_SETS };
const unsigned
    UNI::reference_count[UNI::index_limit] =
        { UNICODE_REFERENCE_COUNT };
