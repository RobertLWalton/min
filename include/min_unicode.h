// MIN UNICODE Data
//
// File:	min_unicode.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jul 15 05:22:27 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# ifndef MIN_UNICODE
# define MIN_UNICODE

namespace min { namespace unicode {

# include "../unicode/unicode_data.h"

} }

namespace min {

    using min::unicode::Uchar;
    using min::unicode::Ustring;
    using min::unicode::Ustring_length;
    using min::unicode::Ustring_columns;
    using min::unicode::Ustring_chars;
    using min::unicode::UNKNOWN_UCHAR;
    using min::unicode::utf8_to_unicode;
    using min::unicode::unicode_to_utf8;
    using min::unicode::SOFTWARE_NL_CATEGORY;

}
# endif
