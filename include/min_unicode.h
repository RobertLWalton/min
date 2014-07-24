// MIN UNICODE Data
//
// File:	min_unicode.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jul 24 05:28:35 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# ifndef MIN_UNICODE
# define MIN_UNICODE

namespace min { namespace unicode {

# include "../unicode/unicode_data.h"
# include "../unicode/unicode_data_support_sets.h"

} }

namespace min {

    using min::unicode::Uchar;

    using min::unicode::ustring;
    using min::unicode::ustring_length;
    using min::unicode::ustring_columns;
    using min::unicode::ustring_chars;
    using min::unicode::ustring_flags;
    using min::unicode::USTRING_LEADING;
    using min::unicode::USTRING_TRAILING;

    using min::unicode::UNKNOWN_UCHAR;
    using min::unicode::SOFTWARE_NL;

    using min::unicode::utf8_to_unicode;
    using min::unicode::unicode_to_utf8;
}
# endif
