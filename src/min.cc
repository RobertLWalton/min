// MIN System Interface
//
// File:	min.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jun  8 16:32:42 EDT 2023
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Setup
//	Initialization
//	Names
//	Process Management
//	Allocator/Collector/Compactor
//	UNICODE Characters
//	Numbers
//	Strings
//	Labels
//	Packed Structures and Vectors
//	Files
//	Identifier Maps
//	UNICODE Name Tables
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level
//	Graph Typed Objects
//	Printers
//	Printing General values
//	Defined Formats

// Setup
// -----

# include <min.h>
# include <min_os.h>
# include <cstdlib>
# include <cstdio>
# include <cstring>
# include <cmath>
# include <cerrno>
# include <cctype>
# define MUP min::unprotected
# define MINT min::internal
# define UNI min::unicode

# define ERR min::init ( min::error_message ) \
    << min::set_indent ( 7 ) << "ERROR: "

void min::no_return ( void ) { abort(); }

// For debugging.
//
# include <iostream>
# include <iomanip>

// Initialization
// --------------

min::initializer * MINT::last_initializer = NULL;
bool MINT::initialization_done = false;

min::locatable_stub_ptr *
    MINT::locatable_stub_ptr_last = NULL;
min::locatable_gen *
    MINT::locatable_gen_last = NULL;

static const char * html_reserved_table[256];
    // See MINT::initialize and ::file_write_ostream
    // below.

static char const * type_name_vector[256];
char const ** min::type_name = type_name_vector + 128;

min::locatable_gen min::TRUE;
min::locatable_gen min::FALSE;
min::locatable_gen min::empty_str;
min::locatable_gen min::empty_lab;
min::locatable_gen min::doublequote;
min::locatable_gen min::line_feed;
min::locatable_gen min::colon;
min::locatable_gen min::semicolon;
min::locatable_gen min::dot_initiator;
min::locatable_gen min::dot_separator;
min::locatable_gen min::dot_terminator;
min::locatable_gen min::dot_type;
min::locatable_gen min::dot_position;

// Deprecated
//
min::locatable_gen min::new_line;
min::locatable_gen min::number_sign;
min::locatable_gen min::dot_middle;
min::locatable_gen min::dot_name;
min::locatable_gen min::dot_arguments;
min::locatable_gen min::dot_keys;
min::locatable_gen min::dot_operator;

static min::uns32 zero_disp[2] =
    { 0, min::DISP_END };

min::packed_vec<char>
    min::char_packed_vec_type
        ( "min::char_packed_vec_type" );
min::packed_vec<min::uns32>
    min::uns32_packed_vec_type
        ( "min::uns32_packed_vec_type" );
min::packed_vec<const char *>
    min::const_char_ptr_packed_vec_type
        ( "min::const_char_ptr_packed_vec_type" );
min::packed_vec<min::ustring>
    min::ustring_packed_vec_type
        ( "min::ustring_packed_vec_type" );
min::packed_vec<min::gen> min::gen_packed_vec_type
    ( "min::gen_packed_vec_type",
      ::zero_disp );

static const unsigned
    standard_special_names_length = 27;
static min::ustring standard_special_names_value
		  [::standard_special_names_length] =
    { NULL,
      (min::ustring) "\x0A\x0A" "SPECIAL -1",
      (min::ustring) "\x0A\x0A" "SPECIAL -2",
      (min::ustring) "\x0A\x0A" "SPECIAL -3",
      (min::ustring) "\x0A\x0A" "SPECIAL -4",
      (min::ustring) "\x0A\x0A" "SPECIAL -5",
      (min::ustring) "\x0A\x0A" "SPECIAL -6",
      (min::ustring) "\x0A\x0A" "SPECIAL -7",
      (min::ustring) "\x0A\x0A" "SPECIAL -8",
      (min::ustring) "\x0A\x0A" "SPECIAL -9",
      (min::ustring) "\x0B\x0B" "SPECIAL -10",
      (min::ustring) "\x0B\x0B" "SPECIAL -11",
      (min::ustring) "\x0B\x0B" "SPECIAL -12",
      (min::ustring) "\x0B\x0B" "SPECIAL -13",
      (min::ustring) "\x0C\x0C" "MULTI_VALUED",
      (min::ustring) "\x03\x03" "ANY",
      (min::ustring) "\x04\x04" "NONE",
      (min::ustring) "\x07\x07" "MISSING",
      (min::ustring) "\x08\x08" "DISABLED",
      (min::ustring) "\x07\x07" "ENABLED",
      (min::ustring) "\x09\x09" "UNDEFINED",
      (min::ustring) "\x06\x06" "UNUSED",
      (min::ustring) "\x07\x07" "SUCCESS",
      (min::ustring) "\x07\x07" "FAILURE",
      (min::ustring) "\x05\x05" "ERROR",
      (min::ustring) "\x0C\x0C" "LOGICAL_LINE",
      (min::ustring) "\x12\x12" "INDENTED_PARAGRAPH" };
min::packed_vec_ptr<min::ustring>
    min::standard_special_names;
static min::locatable_var
	< min::packed_vec_ptr<min::ustring> >
    standard_special_names;

static const unsigned
    standard_attr_flag_names_length = 64;
static min::ustring
    standard_attr_flag_names_value
	[::standard_attr_flag_names_length] =
    {
      (min::ustring) "\x01\x01" "*",
      (min::ustring) "\x01\x01" "+",
      (min::ustring) "\x01\x01" "-",
      (min::ustring) "\x01\x01" "/",
      (min::ustring) "\x01\x01" "@",
      (min::ustring) "\x01\x01" "&",
      (min::ustring) "\x01\x01" "#",
      (min::ustring) "\x01\x01" "=",
      (min::ustring) "\x01\x01" "$",
      (min::ustring) "\x01\x01" "%",
      (min::ustring) "\x01\x01" "<",
      (min::ustring) "\x01\x01" ">",

      (min::ustring) "\x01\x01" "a",
      (min::ustring) "\x01\x01" "b",
      (min::ustring) "\x01\x01" "c",
      (min::ustring) "\x01\x01" "d",
      (min::ustring) "\x01\x01" "e",
      (min::ustring) "\x01\x01" "f",
      (min::ustring) "\x01\x01" "g",
      (min::ustring) "\x01\x01" "h",
      (min::ustring) "\x01\x01" "i",
      (min::ustring) "\x01\x01" "j",
      (min::ustring) "\x01\x01" "k",
      (min::ustring) "\x01\x01" "l",
      (min::ustring) "\x01\x01" "m",
      (min::ustring) "\x01\x01" "n",
      (min::ustring) "\x01\x01" "o",
      (min::ustring) "\x01\x01" "p",
      (min::ustring) "\x01\x01" "q",
      (min::ustring) "\x01\x01" "r",
      (min::ustring) "\x01\x01" "s",
      (min::ustring) "\x01\x01" "t",
      (min::ustring) "\x01\x01" "u",
      (min::ustring) "\x01\x01" "v",
      (min::ustring) "\x01\x01" "w",
      (min::ustring) "\x01\x01" "x",
      (min::ustring) "\x01\x01" "y",
      (min::ustring) "\x01\x01" "z",

      (min::ustring) "\x01\x01" "A",
      (min::ustring) "\x01\x01" "B",
      (min::ustring) "\x01\x01" "C",
      (min::ustring) "\x01\x01" "D",
      (min::ustring) "\x01\x01" "E",
      (min::ustring) "\x01\x01" "F",
      (min::ustring) "\x01\x01" "G",
      (min::ustring) "\x01\x01" "H",
      (min::ustring) "\x01\x01" "I",
      (min::ustring) "\x01\x01" "J",
      (min::ustring) "\x01\x01" "K",
      (min::ustring) "\x01\x01" "L",
      (min::ustring) "\x01\x01" "M",
      (min::ustring) "\x01\x01" "N",
      (min::ustring) "\x01\x01" "O",
      (min::ustring) "\x01\x01" "P",
      (min::ustring) "\x01\x01" "Q",
      (min::ustring) "\x01\x01" "R",
      (min::ustring) "\x01\x01" "S",
      (min::ustring) "\x01\x01" "T",
      (min::ustring) "\x01\x01" "U",
      (min::ustring) "\x01\x01" "V",
      (min::ustring) "\x01\x01" "W",
      (min::ustring) "\x01\x01" "X",
      (min::ustring) "\x01\x01" "Y",
      (min::ustring) "\x01\x01" "Z"

    };
min::packed_vec_ptr<min::ustring>
    min::standard_attr_flag_names;
static min::locatable_var
	< min::packed_vec_ptr<min::ustring> >
    standard_attr_flag_names;

min::uns32 min::standard_flag_parser
	( min::uns32 * flag_numbers,
	  char * text_buffer,
	  const min::flag_parser * flag_parser )
{
    const min::uns32 * flag_map =
        flag_parser->flag_map;
    min::uns32 flag_map_length =
        flag_parser->flag_map_length;

    min::uns32 * outp = flag_numbers;
    const char * p = text_buffer;
    char * q = text_buffer;
    bool last_ok = false;
    bool digit_ok = true;
    while ( * p )
    {
	const char * pstart = p;
        min::Uchar c =
	    min::utf8_to_unicode ( p, p + 8 );
	if ( '0' <= c && c <= '9' )
	{
	    if ( digit_ok )
	    {
	        const min::uns32 LIMIT =
		    (min::uns32) 0xFFFFFFFF / 10;
	        min::uns32 flag_number = c - '0';
		while ( true )
		{
		    c = (uns8) * p ++;
		    if ( c < '0' || '9' < c )
		        break;
		    if ( flag_number > LIMIT )
		        flag_number = min::NO_FLAG;
		    else
		    {
			flag_number *= 10;
			flag_number += c - '0';
		    }
		}
		if ( flag_number == min::NO_FLAG )
		    -- p;
		else if ( c == 0 )
		{
		    * outp ++ = flag_number;
		    break;
		}
		else if ( c == ',' )
		{
		    * outp ++ = flag_number;
		    last_ok = true;
		    digit_ok = true;
		    continue;
		}
		else
		    -- p;
	    }
	}
	else if ( c == ',' )
	{
	    last_ok = true;
	    digit_ok = true;
	    continue;
	}
	else if ( c < flag_map_length
	          &&
	          flag_map[c] != min::NO_FLAG )
	{
	    * outp ++ = flag_map[c];
	    last_ok = true;
	    digit_ok = false;
	    continue;
	}

	// If we did not continue or break outer loop
	// above, then pstart .. p-1 is illegal.
	//
	if ( last_ok && q != text_buffer )
	    * q ++ = ',';
	last_ok = false;
	digit_ok = false;
	while ( pstart < p ) * q ++ = * pstart ++;
    }
    * q = 0;
    return ( outp - flag_numbers );
}

const min::uns32
    min::standard_attr_flag_map_length = 128;
static min::uns32 standard_attr_flag_map
	[min::standard_attr_flag_map_length];
    // F == ::standard_attr_flag_map['X'] if and only
    // if ::standard_attr_flag_names_value[F] ==
    // "\x01\x01" "X".  Elements not defined by this
    // equation have min::NO_FLAG values.  Initialized
    // below by MINT::initialize.
const min::uns32 * min::standard_attr_flag_map =
    min::standard_attr_flag_map;

static min::flag_parser
    standard_attr_flag_parser = { 
        min::standard_flag_parser,
	::standard_attr_flag_map,
	    // Must NOT use min::standard_attr_flag_map
	    // as that will be 0 when this is
	    // initialized.
	min::standard_attr_flag_map_length };
const min::flag_parser *
    min::standard_attr_flag_parser =
	& ::standard_attr_flag_parser;

static void ptr_scavenger_routine
	( MINT::scavenge_control & sc );
static void gtyped_ptr_scavenger_routine
	( MINT::scavenge_control & sc );
static void lab_scavenger_routine
	( MINT::scavenge_control & sc );
static void packed_struct_scavenger_routine
	( MINT::scavenge_control & sc );
static void packed_vec_scavenger_routine
	( MINT::scavenge_control & sc );
static void obj_scavenger_routine
	( MINT::scavenge_control & sc );

#define PTR_CHECK(...) \
    MIN_REQUIRE (    sizeof ( __VA_ARGS__ ) \
                  == sizeof ( min::stub * ) ); \
    MIN_REQUIRE ( ( __VA_ARGS__::DISP() == 0 ) );

static void init_standard_char_flags ( void );
static void init_pgen_formats ( void );
void MINT::initialize ( void )
{
    MINT::initialization_done = true;

    PTR_CHECK ( min::packed_struct_ptr<int> );
    PTR_CHECK ( min::packed_struct_updptr<int> );
    PTR_CHECK ( min::packed_vec_ptr<int,int> );
    PTR_CHECK ( min::packed_vec_updptr<int,int> );
    PTR_CHECK ( min::packed_vec_insptr<int,int> );

    // See ::file_write_ostream below.
    //
    ::html_reserved_table['&'] = "&amp;";
    ::html_reserved_table['<'] = "&lt;";
    ::html_reserved_table['>'] = "&gt;";
    ::html_reserved_table['"'] = "&quot;";
    ::html_reserved_table[0xC2] = "";
    ::html_reserved_table[0xE2] = "";

    type_name[ACC_FREE] = "ACC_FREE";
    type_name[DEALLOCATED] = "DEALLOCATED";
    type_name[PREALLOCATED] = "PREALLOCATED";
    type_name[FILLING] = "FILLING";
    type_name[NUMBER] = "NUMBER";
    type_name[SHORT_STR] = "SHORT_STR";
    type_name[LONG_STR] = "LONG_STR";
    type_name[LABEL] = "LABEL";
    type_name[TINY_OBJ] = "TINY_OBJ";
    type_name[SHORT_OBJ] = "SHORT_OBJ";
    type_name[LONG_OBJ] = "LONG_OBJ";
    type_name[HUGE_OBJ] = "HUGE_OBJ";
    type_name[PACKED_STRUCT] = "PACKED_STRUCT";
    type_name[PACKED_VEC] = "PACKED_VEC";
    type_name[AUX_FREE] = "AUX_FREE";
    type_name[LABEL_AUX] = "LABEL_AUX";
    type_name[LIST_AUX] = "LIST_AUX";
    type_name[SUBLIST_AUX] = "SUBLIST_AUX";
    type_name[HASHTABLE_AUX] = "HASHTABLE_AUX";
    type_name[RELOCATE_BODY] = "RELOCATE_BODY";

    MIN_REQUIRE ( sizeof ( MIN_INT32_TYPE ) == 4 );
    MIN_REQUIRE ( sizeof ( MIN_INT64_TYPE ) == 8 );
    MIN_REQUIRE
        ( sizeof ( void * ) == MIN_PTR_BITS / 8 );

    MIN_REQUIRE
      ( MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG <= 33 );

    MIN_REQUIRE
      (     sizeof ( MINT::tiny_obj )
         == MINT::obj_header_size ( TINY_OBJ ) );
    MIN_REQUIRE
      (     sizeof ( MINT::short_obj )
         == MINT::obj_header_size ( SHORT_OBJ ) );
    MIN_REQUIRE
      (     sizeof ( MINT::long_obj )
         == MINT::obj_header_size ( LONG_OBJ ) );
    MIN_REQUIRE
      (     sizeof ( MINT::huge_obj )
         == MINT::obj_header_size ( HUGE_OBJ ) );

    uns32 u = 1;
    char * up = (char *) & u;
    bool big_endian = ( up[3] == 1 );
    bool little_endian = ( up[0] == 1 );
    MIN_REQUIRE ( MIN_IS_BIG_ENDIAN == big_endian );
    MIN_REQUIRE
        ( MIN_IS_LITTLE_ENDIAN == little_endian );

#   if MIN_IS_LOOSE 

	// Tests of MIN_FLOAT64_SIGNALLING_NAN
	//
	min::gen missing = MISSING();
	float64 v = * (float64 *) & missing;

	MIN_REQUIRE ( std::isnan ( v ) );

	// Attemps to get any kind of NaN to raise an
	// exception failed, so we cannot test for that.

	// However, we can test that hardware does not
	// generate non-signalling NaNs with high order
	// 16 bits identical to v.

	float64 v2 = v + 1.0;

	MIN_REQUIRE ( std::isnan ( v2 ) );
	uns16 * vp = (uns16 *) & v;
	uns16 * v2p = (uns16 *) & v2;
	MIN_REQUIRE ( vp[3*little_endian]
		      !=
		      v2p[3*little_endian] );

	v2 = 0.0;
	v2 = v2 / v2;
	MIN_REQUIRE ( std::isnan ( v2 ) );
	MIN_REQUIRE ( vp[3*little_endian]
		      !=
		      v2p[3*little_endian] );
#   endif

    for ( unsigned j = 0;
          j < MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG-2;
	  ++ j )
    {
        MINT::fixed_block_lists[j].size =
	    1 << ( j + 3 );
        MINT::fixed_block_lists[j].count = 0;
        MINT::fixed_block_lists[j].last_free = NULL;
        MINT::fixed_block_lists[j].extension = NULL;
    }

    MINT::min_fixed_block_size =
        1
	<<
	MINT::log2ceil
	    ( sizeof ( MINT::free_fixed_size_block ) );
    MINT::max_fixed_block_size =
        1
	<<
	MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG;

    MINT::acc_initializer();

    MINT::scavenger_routines[PTR]
    	= & ptr_scavenger_routine;
    MINT::scavenger_routines[GTYPED_PTR]
    	= & gtyped_ptr_scavenger_routine;
    MINT::scavenger_routines[LABEL]
    	= & lab_scavenger_routine;
    MINT::scavenger_routines[PACKED_STRUCT]
    	= & packed_struct_scavenger_routine;
    MINT::scavenger_routines[PACKED_VEC]
    	= & packed_vec_scavenger_routine;
    MINT::scavenger_routines[TINY_OBJ]
    	= & obj_scavenger_routine;
    MINT::scavenger_routines[SHORT_OBJ]
    	= & obj_scavenger_routine;
    MINT::scavenger_routines[LONG_OBJ]
    	= & obj_scavenger_routine;
    MINT::scavenger_routines[HUGE_OBJ]
    	= & obj_scavenger_routine;

    min::TRUE =
        min::new_str_gen ( "TRUE" );
    min::FALSE =
        min::new_str_gen ( "FALSE" );
    min::empty_str =
        min::new_str_gen ( "" );
    min::empty_lab =
        min::new_lab_gen ( (const min::gen *) NULL, 0 );
    min::doublequote =
        min::new_str_gen ( "\"" );
    min::line_feed =
        min::new_str_gen ( "\n" );
    min::colon =
        min::new_str_gen ( ":" );
    min::semicolon =
        min::new_str_gen ( ";" );
    min::dot_initiator =
        min::new_str_gen ( ".initiator" );
    min::dot_separator =
        min::new_str_gen ( ".separator" );
    min::dot_terminator =
        min::new_str_gen ( ".terminator" );
    min::dot_type =
        min::new_str_gen ( ".type" );
    min::dot_position =
        min::new_str_gen ( ".position" );

    // Deprecated:
    //
    min::new_line =
        min::new_str_gen ( "\n" );
    min::number_sign =
        min::new_str_gen ( "#" );
    min::dot_middle =
        min::new_str_gen ( ".middle" );
    min::dot_name =
        min::new_str_gen ( ".name" );
    min::dot_arguments =
        min::new_str_gen ( ".arguments" );
    min::dot_keys =
        min::new_str_gen ( ".keys" );
    min::dot_operator =
        min::new_str_gen ( ".operator" );

    {
	min::packed_vec_insptr<min::ustring> p =
	    min::ustring_packed_vec_type.new_stub
		( ::standard_special_names_length );
	::standard_special_names = p;
	min::standard_special_names = p;
	min::push ( p, ::standard_special_names_length,
		       ::standard_special_names_value );
    }

    {
	min::packed_vec_insptr<min::ustring> p =
	    min::ustring_packed_vec_type.new_stub
		( ::standard_attr_flag_names_length );
	::standard_attr_flag_names = p;
	min::standard_attr_flag_names = p;

	min::push ( p,
	            ::standard_attr_flag_names_length,
		    ::standard_attr_flag_names_value );
    }

    {
        for ( unsigned c = 0;
	      c < min::standard_attr_flag_map_length;
	      ++ c )
	  ::standard_attr_flag_map[c] = min::NO_FLAG;
        for ( unsigned f = 0;
	      f < ::standard_attr_flag_names_length;
	      ++ f )
	{
	  min::uns8 c =
	      standard_attr_flag_names_value[f][2];
	  MIN_REQUIRE
	    ( c < min::standard_attr_flag_map_length );
	  ::standard_attr_flag_map[c] =
	      (min::uns32) f;
	}
    }

    init_standard_char_flags();
    init_pgen_formats();

    for ( min::initializer * i = MINT::last_initializer;
          i != NULL; i = i->previous )
        i->init();

    MIN_REQUIRE ( min::Uindex ( ' ' ) == ' ' );
        // Used by min::print_space
}

// Names
// -----

min::uns32 min::hash ( min::gen g )
{
    if ( is_num ( g ) )
        return numhash ( g );
    else if ( is_str ( g ) )
        return strhash ( g );
    else if ( is_lab ( g ) )
        return labhash ( g );
    else
	MIN_ABORT ( "argument to min::hash"
	            " is not a name" );

}

int min::compare ( min::gen g1, min::gen g2 )
{
    if ( is_num ( g1 ) )
    {
        if ( ! is_num ( g2 ) ) return -1;
	float64 f1 = float_of ( g1 );
	float64 f2 = float_of ( g2 );
	if ( f1 < f2 ) return -1;
	else if ( f1 == f2 ) return 0;
	else return +1;
    }
    else
    if ( is_str ( g1 ) )
    {
        if ( is_num ( g2 ) ) return +1;
        else if ( ! is_str ( g2 ) ) return -1;
	str_ptr p1 ( g1 );
	str_ptr p2 ( g2 );
	return ::strcmp ( MUP::str_of ( p1 ),
	                  MUP::str_of ( p2 ) );
    }
    else
    if ( is_lab ( g1 ) )
    {
        if ( is_num ( g2 ) ) return +1;
	else if ( is_str ( g2 ) ) return +1;
        else if ( ! is_lab ( g2 ) ) return -1;
	lab_ptr lp1 ( g1 );
	lab_ptr lp2 ( g2 );
	unsptr len1 = lablen ( lp1 );
	unsptr len2 = lablen ( lp2 );
	for ( unsptr i = 0; i < len1; ++ i )
	{
	    if ( i >= len2 ) return +1;
	    int t = min::compare ( lp1[i], lp2[i] );
	    if ( t != 0 ) return t;
	}
	if ( len1 == len2 ) return 0;
	else return -1;
    }
    else if ( is_name ( g2 ) ) return +1;
    else
    {
        unsgen v1 = MUP::value_of ( g1 );
        unsgen v2 = MUP::value_of ( g2 );
	return v1  < v2 ? -1 : v1 == v2 ? 0 : +1;
    }
}

min::int32 min::is_subsequence
	( min::gen v1, min::gen v2 )
{
    MIN_ASSERT ( is_name ( v1 ),
                 "first argument is not name" );
    MIN_ASSERT ( is_name ( v2 ),
                 "second argument is not name" );

    min::uns32 length1 =
        is_lab ( v1 ) ? lablen ( v1 ) : 1;
    min::uns32 length2 =
        is_lab ( v2 ) ? lablen ( v2 ) : 1;

    min::gen label1[length1];
    min::gen label2[length2];

    if ( is_lab ( v1 ) )
        labncpy ( label1, v1, length1 );
    else
        label1[0] = v1;
    if ( is_lab ( v2 ) )
        labncpy ( label2, v2, length2 );
    else
        label2[0] = v2;

    if ( length1 > length2 ) return -1;
    for ( min::uns32 i = 0;
          i <= length2 - length1; ++ i )
    {
	for ( min::uns32 j = 0; j <= length1; ++ j )
	{
	    if ( j == length1 ) return i;
	    if ( label1[j] != label2[i+j] ) break;
	}
    }
    return -1;
}

min::gen min::new_name_gen ( const char * s )
{
    //Count the number of components.
    //
    uns32 count = 0;
    const char * p = s;
    const char * ends = s + ::strlen ( s );
    bool last_is_space = true;
    while ( * p )
    {
	min::Uchar c = min::utf8_to_unicode ( p, ends );
	uns16 index = min::Uindex ( c );
	uns32 flags =
	    index < min::unicode::index_limit ?
	    min::standard_char_flags[index] :
	    min::IS_VHSPACE;
	if ( flags & min::IS_VHSPACE )
	    last_is_space = true;
	else
	{
	    if ( last_is_space ) ++ count;
	    last_is_space = false;
	}
    }

    // Compute the components.
    //
    min::locatable_gen component[count];
    uns32 i = 0;
    const char * q;
    p = s;
    last_is_space = true;
    bool done = false;
    while ( ! done )
    {
	const char * psave = p;
	uns32 flags = min::IS_VHSPACE;
	if  ( * p )
	{
	    min::Uchar c =
	        min::utf8_to_unicode ( p, ends );
	    uns16 index = min::Uindex ( c );
	    flags =
		index < min::unicode::index_limit ?
		min::standard_char_flags[index] :
		min::IS_VHSPACE;
	}
	else done = true;

	if ( flags & min::IS_VHSPACE )
	{
	    if ( ! last_is_space )
	        component[i++] =
		    min::new_str_gen ( q, psave - q );
	    last_is_space = true;
	}
	else
	{
	    if ( last_is_space ) q = psave;
	    last_is_space = false;
	}
    }

    if ( count == 1 ) return component[0];

    min::gen component_vec[count];
    for ( i = 0; i < count; ++ i )
        component_vec[i] = component[i];

    return min::new_lab_gen ( component_vec, count );
}

// Process Management
// ------- ----------

bool MINT::thread_interrupt_needed = false;
void MINT::thread_interrupt ( void ) {}  // TBD

// Allocator/Collector/Compactor 
// -----------------------------

min::gen min::new_preallocated_gen ( min::uns32 id )
{
    min::stub * s =
	min::unprotected::new_acc_stub();
    min::unprotected::set_type_of
	( s, min::PREALLOCATED );
    min::unprotected::set_value_of
        ( s, (1ull << 32) + (min::uns64) id );
    return min::new_stub_gen ( s );
}

static min::stub ZERO_STUB;
const min::stub * min::unprotected::ZERO_STUB =
    & ::ZERO_STUB;

min::unsptr MINT::number_of_free_stubs;

min::stub ** MINT::str_acc_hash;
min::stub ** MINT::str_aux_hash;
min::unsptr  MINT::str_hash_size;
min::unsptr  MINT::str_hash_mask;

# if MIN_IS_COMPACT
    min::stub ** MINT::num_acc_hash;
    min::stub ** MINT::num_aux_hash;
    min::unsptr  MINT::num_hash_size;
    min::unsptr  MINT::num_hash_mask;
# endif

min::stub ** MINT::lab_acc_hash;
min::stub ** MINT::lab_aux_hash;
min::unsptr  MINT::lab_hash_size;
min::unsptr  MINT::lab_hash_mask;

min::uns64 MINT::hash_acc_set_flags;
min::uns64 MINT::hash_acc_clear_flags;

# ifndef MIN_STUB_BASE
    min::unsptr MINT::stub_base;
    min::stub * MINT::null_stub;
# endif

min::uns64   MINT::acc_stack_mask;
min::stub ** MINT::acc_stack;
min::stub ** volatile MINT::acc_stack_limit;

min::uns64  MINT::new_acc_stub_flags;
min::stub * MINT::head_stub;
min::stub * MINT::last_allocated_stub;
min::stub * MINT::last_free_stub;

MINT::fixed_block_list MINT::fixed_block_lists
	[MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG-2];

min::unsptr MINT::min_fixed_block_size;
min::unsptr MINT::max_fixed_block_size;

min::uns64 MUP::acc_stubs_allocated = 0;
min::uns64 MUP::acc_stubs_freed = 0;
min::uns64 MUP::aux_stubs_allocated = 0;
min::uns64 MUP::aux_stubs_freed = 0;

MINT::scavenger_routine MINT::scavenger_routines[128];
MINT::scavenge_control MINT::scavenge_controls
	[ 1 + MIN_MAX_EPHEMERAL_LEVELS ];
unsigned MINT::number_of_acc_levels;

// Macro to process a stub pointer s2 for a scavenger.
// If the to_be_scavenged limit has been reached,
// this macro stores the `accumulator' and executes
// the FAIL statements.  Otherwise this macro increments
// sc.stub_count but does not access sc.gen_count/limit.
// Stubs with type < 0 are ignored.
//
#define MIN_SCAVENGE_S2(FAIL) \
    min::uns64 c = MUP::control_of ( s2 ); \
    int type = MUP::type_of_control ( c ); \
    if ( type >= 0 ) \
    { \
        if ( c & sc.clear_flag ) \
	{ \
	    if ( ! MINT::is_scavengable ( type ) ) \
	    { \
		accumulator |= c; \
		c |= sc.set_flag; \
	    } \
	    else if (    sc.to_be_scavenged \
		      >= sc.to_be_scavenged_limit ) \
	    { \
		sc.stub_flag_accumulator = \
		    accumulator; \
		FAIL; \
	    } \
	    else \
	    { \
		accumulator |= c; \
		* sc.to_be_scavenged ++ = s2; \
	    } \
	    c &= ~ sc.clear_flag; \
	    MUP::set_control_of ( s2, c ); \
        } \
	++ sc.stub_count; \
    }

// Scavenger routine for pointers.
//
static void ptr_scavenger_routine
	( MINT::scavenge_control & sc )
{
    const min::stub * saux =
        MUP::stub_of ( MUP::gen_of ( sc.s1 ) );
    min::uns64 cntl = MUP::control_of ( saux );
    min::stub * s2 =
        MUP::stub_of_control ( cntl );
    if ( s2 == NULL ) return;
    if ( s2 == MUP::ZERO_STUB ) return;
        // This is just an optimization as
	// MINT::is_scavengable ( ZERO_STUB ) is false.

    if ( sc.gen_count >= sc.gen_limit )
    {
	sc.state = sc.RESTART;
	return;
    }

    min::uns64 accumulator = sc.stub_flag_accumulator;
    MIN_SCAVENGE_S2 ( sc.state = sc.RESTART; return );
    sc.stub_flag_accumulator = accumulator;

    ++ sc.gen_count;
    sc.state = 0;
}

// Scavenger routine for graph typed pointers.  State
// == 0 to scavenge all and == 1 to scavenge just graph
// type.
//
static void gtyped_ptr_scavenger_routine
	( MINT::scavenge_control & sc )
{
    const min::stub * saux =
        MUP::stub_of ( MUP::gen_of ( sc.s1 ) );

    min::uns64 accumulator = sc.stub_flag_accumulator;
    min::stub * s2;
    if ( sc.state == 0 )
    {

	min::gen val = MUP::gen_of ( saux );
	s2 = MUP::stub_of ( val );
	MIN_REQUIRE ( s2 != NULL );

	if ( sc.gen_count >= sc.gen_limit )
	    return;

	MIN_SCAVENGE_S2 ( return );
	++ sc.gen_count;
    }

    min::uns64 cntl = MUP::control_of ( saux );
    s2 = MUP::stub_of_control ( cntl );
    MIN_REQUIRE ( s2 != NULL );

    if ( sc.gen_count >= sc.gen_limit )
    {
	sc.state = 1;
	return;
    }

    MIN_SCAVENGE_S2 ( sc.state = 1; return );
    ++ sc.gen_count;

    sc.stub_flag_accumulator = accumulator;

    sc.state = 0;
}

// Scavenger routine for labels.  State equals i + 1
// where i is the index next label element to scavenge.
//
static void lab_scavenger_routine
	( MINT::scavenge_control & sc )
{
    min::uns32 i = (min::uns32 ) sc.state;
    if ( i > 0 ) -- i;

    min::uns64 accumulator = sc.stub_flag_accumulator;
    min::lab_ptr labp ( sc.s1 );
    while ( i < min::lablen ( labp ) )
    {
        if ( sc.gen_count >= sc.gen_limit )
	{
            sc.stub_flag_accumulator = accumulator;
	    sc.state = i + 1;
	    return;
	}

	min::gen v = labp[i];

	if ( min::is_stub ( v ) )
	{
	    min::stub * s2 = MUP::stub_of ( v );
	    MIN_SCAVENGE_S2
	        ( sc.state = i + 1; return );
	}

	++ sc.gen_count;
	++ i;
    }
    sc.stub_flag_accumulator = accumulator;
    sc.state = 0;
}

// Scavenger routine for packed structs.  State equals
// ( i << 2 ) + j where next member of the packed
// struct to be scanned is:
//
//	gen_disp[i]	if j == 2
//	stub_disp[i]	if j == 3
//
// and i < (1<<30). 
//
static void packed_struct_scavenger_routine
	( MINT::scavenge_control & sc )
{
    min::uns8 * beginp =
        (min::uns8 *) MUP::ptr_of ( sc.s1 );
    min::uns32 subtype = * (min::uns32 *) beginp;
    subtype &= MINT::PACKED_CONTROL_SUBTYPE_MASK;
    MIN_REQUIRE
        ( subtype < MINT::packed_subtype_count );
    MINT::packed_struct_descriptor * psd =
        (MINT::packed_struct_descriptor *)
        (*MINT::packed_subtypes)[subtype];

    MIN_REQUIRE ( sc.state < (1ull << 32 ) );
    min::uns32 i = (min::uns32) sc.state >> 2;
    min::uns64 accumulator = sc.stub_flag_accumulator;

    if (    ( sc.state & 1 ) == 0
         && psd->gen_disp != NULL )
	while ( true )
    {
	MIN_REQUIRE ( i < (1<<30) );

        min::uns32 d = psd->gen_disp[i];
	if ( d == min::DISP_END )
	{
	    i = 0;
	    break;
	}

	if ( sc.gen_count >= sc.gen_limit )
	{
	    sc.stub_flag_accumulator =
		accumulator;
	    sc.state = ( i << 2 ) + 2;
	    return;
	}

	min::gen v = * (min::gen *) (beginp + d);

	if ( min::is_stub ( v ) )
	{
	    min::stub * s2 = MUP::stub_of ( v );
	    MIN_SCAVENGE_S2
	        ( sc.state = ( i << 2 ) + 2; return );
	}

	++ sc.gen_count;
	++ i;
    }

    if ( psd->stub_disp != NULL )
        while ( true )
    {
	MIN_REQUIRE ( i < (1<<30) );

        min::uns32 d = psd->stub_disp[i];
	if ( d == min::DISP_END ) break;

	if ( sc.gen_count >= sc.gen_limit )
	{
	    sc.stub_flag_accumulator =
		accumulator;
	    sc.state = ( i << 2 ) + 3;
	    return;
	}

	min::stub * s2 = * (min::stub **) (beginp + d );
	if ( s2 != NULL )
	{
	    MIN_SCAVENGE_S2
	        ( sc.state = ( i << 2 ) + 3; return );
	}

	++ sc.gen_count;
	++ i;
    }

    sc.stub_flag_accumulator = accumulator;
    sc.state = 0;
}

// Scavenger routine for packed vecs.  State equals
// ( k << 32 ) + ( i << 2 ) + j where next member of the
// packed vec to be scanned is:
//
//	gen_disp[i]		if k == 0 and j == 2
//	stub_disp[i]		if k == 0 and j == 3
//	element_gen_disp[i] of vector element k - 1
//		if k > 0 and j == 2
//	element_stub_disp[i] of vector element k - 1
//		if k > 0 and j == 3
//
// and i < (1<<30), k < (1<<32). 
//
static void packed_vec_scavenger_routine
	( MINT::scavenge_control & sc )
{

    min::uns8 * beginp =
        (min::uns8 *) MUP::ptr_of ( sc.s1 );
    min::uns32 subtype = * (min::uns32 *) beginp;
    subtype &= MINT::PACKED_CONTROL_SUBTYPE_MASK;
    MIN_ASSERT ( subtype < MINT::packed_subtype_count,
                 "system programming error" );
    MINT::packed_vec_descriptor * pvd =
        (MINT::packed_vec_descriptor *)
        (*MINT::packed_subtypes)[subtype];

    min::uns32 length = * (min::uns32 *)
    	( beginp + pvd->length_disp );

    min::uns32 i = ( (min::uns32) sc.state ) >> 2;
    min::uns32 k = (min::uns32) ( sc.state >> 32 );

    const min::uns32 * gen_disp =
	pvd->header_gen_disp;
    const min::uns32 * stub_disp =
        pvd->header_stub_disp;

    if ( k > 0 )
    {
        beginp += pvd->header_size
    	        + ( k - 1 ) * pvd->element_size;
	gen_disp = pvd->element_gen_disp;
	stub_disp = pvd->element_stub_disp;
    }

    min::uns64 accumulator = sc.stub_flag_accumulator;

    while ( k < length + 1 )
    {
	if ( ( sc.state & 1 ) == 0 && gen_disp != NULL )
	    while ( true )
	{
	    MIN_REQUIRE ( i < (1<<30) );

	    min::uns32 d = gen_disp[i];
	    if ( d == min::DISP_END )
	    {
		i = 0;
		break;
	    }

	    if ( sc.gen_count >= sc.gen_limit )
	    {
		sc.stub_flag_accumulator = accumulator;
		sc.state = ( (min::uns64) k << 32 )
			 + ( i << 2 ) + 2;
		return;
	    }

	    min::gen v = * (min::gen *) (beginp + d);

	    if ( min::is_stub ( v ) )
	    {
		min::stub * s2 = MUP::stub_of ( v );
		MIN_SCAVENGE_S2
		    ( sc.state =
		            ( (min::uns64) k << 32 )
		          + ( i << 2 ) + 2; return );
	    }

	    ++ sc.gen_count;
	    ++ i;
	}

	if ( stub_disp != NULL )
	    while ( true )
	{
	    MIN_REQUIRE ( i < (1<<30) );

	    min::uns32 d = stub_disp[i];
	    if ( d == min::DISP_END )
	    {
		i = 0;
		break;
	    }

	    if ( sc.gen_count >= sc.gen_limit )
	    {
		sc.stub_flag_accumulator =
		    accumulator;
		sc.state = ( (min::uns64) k << 32 )
			 + ( i << 2 ) + 3;
		return;
	    }

	    min::stub * s2 =
	        * (min::stub **) (beginp + d );
	    if ( s2 != NULL )
	    {
		MIN_SCAVENGE_S2
		    ( sc.state =
		            ( (min::uns64) k << 32 )
		          + ( i << 2 ) + 3; return );
	    }

	    ++ sc.gen_count;
	    ++ i;
	}

	if ( k == 0 )
	{
	    beginp += pvd->header_size;
	    gen_disp = pvd->element_gen_disp;
	    stub_disp = pvd->element_stub_disp;
	}
	else
	    beginp += pvd->element_size;
	++ k;
    }

    sc.stub_flag_accumulator = accumulator;
    sc.state = 0;
}

# if MIN_USE_OBJ_AUX_STUBS

    // Alternative to MIN_SCAVENGE_S2 for use when
    // s2 may point at an aux stub.
    //
#   define MIN_SCAVENGE_S2_WITH_AUX(FAIL) \
	min::uns64 c = MUP::control_of ( s2 ); \
	int type = MUP::type_of_control ( c ); \
        \
	if ( type < 0 ) \
	{ \
	    sc.stub_flag_accumulator = accumulator; \
	    if ( obj_aux_scavenge ( sc, s2 ) ) \
	    { \
		FAIL; \
	    } \
	    accumulator = sc.stub_flag_accumulator; \
	} \
	else \
	{ \
	    if ( c & sc.clear_flag ) \
	    { \
	        if ( ! MINT::is_scavengable ( type ) ) \
	        { \
	            accumulator |= c; \
		    c |= sc.set_flag; \
	        } \
	        else \
		if (    sc.to_be_scavenged \
		     >= sc.to_be_scavenged_limit ) \
	        { \
		    sc.stub_flag_accumulator = \
		        accumulator; \
		    FAIL; \
	        } \
	        else \
	        { \
	            accumulator |= c; \
		    * sc.to_be_scavenged ++ = s2; \
		} \
		c &= ~ sc.clear_flag; \
		MUP::set_control_of ( s2, c ); \
	    } \
	    ++ sc.stub_count; \
	}

    // Helper for obj_scavenger_routine that scavenges
    // an object aux stub.  Recursively scavenges the
    // value of the aux stub and of any pointer to stub
    // in the control of the aux stub.  Assumes all
    // aux stubs are LIST_AUX or SUBLIST_AUX stubs.
    //
    // Returns true if the to-be-scavenged stack is
    // exhausted and the aux stub scavenging is not
    // complete.  Returns false otherwise.  Increments
    // gen_count but ignores gen_limit as the work
    // required to save the state of an aux stub
    // scavenge is not done by the current version of
    // this routine (it could be done with an extra
    // stack).
    //
    static bool obj_aux_scavenge
            ( MINT::scavenge_control & sc,
	      min::stub * aux_s )
    {
	min::uns64 accumulator =
	    sc.stub_flag_accumulator;
	while ( true )
	{
	    min::gen v = MUP::gen_of ( aux_s );
	    if ( min::is_stub ( v ) )
	    {

		min::stub * s2 = MUP::stub_of ( v );
		MIN_SCAVENGE_S2_WITH_AUX
		    ( return true );
	    }
	    ++ sc.gen_count;

	    min::uns64 c = MUP::control_of ( aux_s );
	    if ( c & MUP::STUB_ADDRESS )
	        aux_s = MUP::stub_of_control ( c );
	    else
	        break;
	}
	sc.stub_flag_accumulator = accumulator;
	return false;
    }
# else
#	define MIN_SCAVENGE_S2_WITH_AUX(FAIL) \
	    MIN_SCAVENGE_S2 ( FAIL )
# endif

// Scavenger routine for objects.  If non-zero, sc.state
// is the index of the min::gen object vector element to
// be scanned next.  This increments but is skipped over
// the object header and unused areas.
//
static void obj_scavenger_routine
	( MINT::scavenge_control & sc )
{
    MINT::obj_vec_gcptr vp ( sc.s1 );

    min::unsptr next = (min::unsptr ) sc.state;
    if ( next <= MUP::var_offset_of ( vp ) )
        next = MUP::var_offset_of ( vp );
    else if ( MUP::unused_offset_of ( vp ) <= next
              &&
	      next < MUP::aux_offset_of ( vp ) )
	next = MUP::aux_offset_of ( vp );

    min::uns64 accumulator = sc.stub_flag_accumulator;
    while ( next < min::total_size_of ( vp ) )
    {
        if ( sc.gen_count >= sc.gen_limit )
	{
            sc.stub_flag_accumulator = accumulator;
	    sc.state = next;
	    return;
	}
	min::gen v = MUP::base(vp)[next];
	if ( min::is_stub ( v ) )
	{
	    min::stub * s2 = MUP::stub_of ( v );
	    MIN_SCAVENGE_S2_WITH_AUX
	        ( sc.state = next; return );
	}
	++ sc.gen_count;

	++ next;
	if ( next == MUP::unused_offset_of ( vp ) )
	    next = MUP::aux_offset_of ( vp );
    }
    sc.stub_flag_accumulator = accumulator;
    sc.state = 0;
}

void MINT::thread_scavenger_routine
	( MINT::scavenge_control & sc )
{
    if ( sc.thread_state == 0 )
    {
        sc.locatable_gen_last =
	    MINT::locatable_gen_last;
        sc.locatable_var_last =
	    MINT::locatable_stub_ptr_last;
    }

    min::uns64 accumulator = sc.stub_flag_accumulator;
    while ( sc.locatable_gen_last != NULL )
    {
	if ( sc.gen_count >= sc.gen_limit )
	{
	    sc.stub_flag_accumulator = accumulator;
	    sc.thread_state = 1;
	    return;
	}
	min::gen v = (min::gen) * sc.locatable_gen_last;
	if ( min::is_stub ( v ) )
	{
	    min::stub * s2 = MUP::stub_of ( v );
	    MIN_SCAVENGE_S2
	        ( sc.thread_state = 1; return );
	}
	++ sc.gen_count;
        sc.locatable_gen_last =
	    MINT::locatable_var_previous
	        ( sc.locatable_gen_last );
    }

    while ( sc.locatable_var_last != NULL )
    {
	if ( sc.gen_count >= sc.gen_limit )
	{
	    sc.stub_flag_accumulator = accumulator;
	    sc.thread_state = 1;
	    return;
	}
	min::stub * s2 =
	    (min::stub *)
	    (const min::stub *) * sc.locatable_var_last;
	if ( s2 != NULL )
	{
	    MIN_SCAVENGE_S2
	        ( sc.thread_state = 1; return );
	}
	++ sc.gen_count;
	sc.locatable_var_last =
	    MINT::locatable_var_previous
	        ( sc.locatable_var_last );
    }

    sc.stub_flag_accumulator = accumulator;
    sc.thread_state = 0;
}

// UNICODE Characters
// ------- ----------

static min::uns32 * standard_char_flags =
    new min::uns32[min::unicode::index_limit];
const min::uns32 * min::standard_char_flags =
    ::standard_char_flags;

min::unsptr min::utf8_to_unicode
    ( min::Uchar * & u, const min::Uchar * endu,
      const char * & s, const char * ends )
{
    min::Uchar * original_u = u;
    while ( u < endu && s < ends )
    {
        * u ++ = utf8_to_unicode ( s, ends );
    }
    return u - original_u;
}

min::unsptr min::unicode_to_utf8
    ( char * & s, const char * ends,
      const min::Uchar * & u,
      const min::Uchar * endu )
{
    char * original_s = s;
    while ( u < endu && s + 7 < ends )
    {
        unicode_to_utf8 ( s, * u ++ );
    }
    if ( u < endu && s < ends )
    {
        char buffer[7];
	while ( u < endu && s < ends )
	{
	    char * b = buffer;
	    min::unsptr len =
	        unicode_to_utf8 ( b, * u );
	    MIN_REQUIRE ( len <= 7 );
	    if ( s + len > ends ) break;
	    std::strncpy ( s, buffer, len );
	    s += len;
	    ++ u;
	}
    }
    return s - original_s;
}

static void init_standard_char_flags ( void )
{
    for ( unsigned i = 0;
          i < min::unicode::index_limit; ++ i )
    { 
	const char * cat = min::unicode::category[i];
	min::uns32 flags;

	// Set flags by category.
	//
	if ( cat == NULL )
	    flags = min::IS_UNSUPPORTED;
	else if ( cat[0] == 'L' )
	    flags = min::IS_GRAPHIC
	          + min::IS_LETTER;
	else if ( cat[0] == 'P' )
	{
	    switch ( cat[1] )
	    {
	    case 'i':
	    case 's':
		flags = min::IS_GRAPHIC
		      + min::IS_LEADING
		      + min::IS_SEPARATOR;
		break;

	    case 'f':
	    case 'e':
		flags = min::IS_GRAPHIC
		      + min::IS_TRAILING
		      + min::IS_SEPARATOR;
		break;

	    case 'c':
	    case 'd':
	        flags = min::IS_GRAPHIC
		      + min::CONDITIONAL_BREAK
		      + min::IS_MARK;
		break;

	    default:
		flags = min::IS_GRAPHIC
		      + min::IS_MARK;
	    }
	}
	else if ( cat[0] == 'S' )
	    flags = min::IS_GRAPHIC
		  + min::IS_MARK;
	else if ( strcmp ( cat, "Mn" ) == 0 )
	    flags = min::IS_GRAPHIC
	          + min::IS_NON_SPACING
		  + min::IS_MARK;
	else if ( strcmp ( cat, "Me" ) == 0 )
	    flags = min::IS_GRAPHIC
	          + min::IS_NON_SPACING
		  + min::IS_MARK;
	else if ( cat[0] == 'M' )
	    flags = min::IS_GRAPHIC
	          + min::IS_MARK;
	else if ( strcmp ( cat, "Nd" ) == 0 )
	    flags = min::IS_GRAPHIC
	          + min::IS_DIGIT;
	else if ( cat[0] == 'N' )
	    flags = min::IS_GRAPHIC
	          + min::IS_MARK;
	else if ( strcmp ( cat, "Zs" ) == 0 )
	    flags = min::IS_HSPACE
	          + min::IS_BHSPACE
	          + min::IS_VHSPACE
		  + min::IS_CONTROL;
	else if ( cat[0] == 'Z' )
	    flags = min::IS_CONTROL
		  + min::IS_NON_SPACING;
	else if ( cat[0] == 'C' )
	    flags = min::IS_CONTROL
		  + min::IS_NON_SPACING;
	else
	    flags = min::IS_UNSUPPORTED;

	// Fixup flags in special cases.
	//
	switch ( i )
	{
	case ' ':   flags = min::IS_SP
		          + min::IS_BHSPACE
		          + min::IS_HSPACE
			  + min::IS_VHSPACE
			  + min::IS_CONTROL;
		    break;	

	case '\t':  flags = min::IS_HSPACE
		          + min::IS_BHSPACE
			  + min::IS_VHSPACE
			  + min::IS_CONTROL;
		    break;

	case 0xA0:	// NBSP
	case 0x202F:	// NNBSP (Narrow NBSP)
		    flags = min::IS_HSPACE
			  + min::IS_VHSPACE
			  + min::IS_CONTROL;
		    break;
	case '\f':
	case '\v':
	case '\n':
	case '\r': flags = min::IS_VHSPACE
			 + min::IS_CONTROL
			 + min::IS_NON_SPACING;
		   break;

	case '`':
	case 0xA1:	// Inverted !
	case 0xBF:	// Inverted ?
		    flags = min::IS_GRAPHIC
		          + min::IS_LEADING
		          + min::IS_REPEATER
			  + min::IS_MARK;
		    break;
	
	case ',':
	case ';':
		    flags = min::IS_GRAPHIC
		          + min::IS_TRAILING
		          + min::IS_MARK;
		    break;

	case '|':
		    flags = min::IS_GRAPHIC
		          + min::IS_LEADING
			  + min::IS_TRAILING
			  + min::IS_SEPARATOR
			  + min::IS_REPEATER;
		    break;

	case '"':
		    flags = min::IS_GRAPHIC
		          + min::NEEDS_QUOTES;
		    break;

	case '\'':
	case '!':
	case '?':
	case '.':
	case ':':
		    flags = min::IS_GRAPHIC
		          + min::IS_TRAILING
		          + min::IS_REPEATER
			  + min::IS_MARK;
		    break;

	case '/':
	case '#':
	case '&':
		    flags = min::IS_GRAPHIC
			  + min::CONDITIONAL_BREAK
			  + min::IS_MARK;
		    break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		    flags = min::IS_GRAPHIC
			  + min::IS_DIGIT
			  + min::IS_NATURAL;
		    break;
	}

	::standard_char_flags[i] =
	    flags + min::unicode::support_sets[i];
    }
}


// Numbers
// -------

# if MIN_IS_COMPACT
    min::gen MINT::new_num_stub_gen
	    ( min::float64 v )
    {
	uns32 hash = floathash ( v );
	uns32 h = hash & MINT::num_hash_mask;
	min::stub * s = MINT::num_acc_hash[h];
	while ( s != MINT::null_stub )
	{
	    uns64 c = MUP::control_of ( s );

	    if ( MUP::float_of ( s ) == v )
	    {
	        c |= MINT::hash_acc_set_flags;
	        c &= ~ MINT::hash_acc_clear_flags;
		MUP::set_control_of ( s, c );
		return new_stub_gen ( s );
	    }
	    s = MUP::stub_of_acc_control ( c );
	}
	s = MINT::num_aux_hash[h];
	while ( s != MINT::null_stub )
	{
	    min::stub * s2 =
	        (min::stub *) MUP::ptr_of ( s );
	    s = (min::stub *)
		MUP::stub_of_control
		    ( MUP::control_of ( s ) );

	    if ( MUP::float_of ( s2 ) == v )
	    {
		uns64 c = MUP::control_of ( s2 );
	        c |= MINT::hash_acc_set_flags;
	        c &= ~ MINT::hash_acc_clear_flags;
		MUP::set_control_of ( s2, c );
		return new_stub_gen ( s2 );
	    }
	}

	min::stub * s2 = MUP::new_acc_stub();
	MUP::set_float_of ( s2, v );
	MUP::set_type_of ( s2, NUMBER );

	s = MUP::new_aux_stub ();
	MUP::set_ptr_of ( s, s2 );
	MUP::set_control_of
	    ( s,
	      MUP::new_control_with_type
	          ( HASHTABLE_AUX,
		    MINT::num_aux_hash[h] ) );
	MINT::num_aux_hash[h] = s;

	return new_stub_gen ( s2 );
    }
# endif

min::uns32 min::floathash ( min::float64 f )
{
    uns32 hash = 0;
    unsigned char * p = (unsigned char *) & f;
    int size = 8;
#   if MIN_IS_LITTLE_ENDIAN
	p += 8;
#   endif
    while ( size -- )
    {
#	if MIN_IS_BIG_ENDIAN
	    hash = ( hash * 65599 ) + * p ++;
#	elif MIN_IS_LITTLE_ENDIAN
	    hash = ( hash * 65599 ) + * -- p;
#	endif
    }
    return hash;
}

// Strings
// -------

min::unsptr min::max_id_strlen =
    min::DEFAULT_MAX_ID_STRLEN;

min::uns32 min::strnhash
	( const char * p, min::unsptr size )
{
    uns32 hash = 0;
    const unsigned char * q = (const unsigned char *) p;
    unsigned char c;
    while ( size -- && ( c = * q ++ ) )
    {
        hash = ( hash * 65599 ) + c;
    }
    if ( hash == 0 ) hash = 0xFFFFFFFF;
    return hash;
}

min::uns32 min::strhash ( const char * p )
{
    uns32 hash = 0;
    const unsigned char * q = (const unsigned char *) p;
    unsigned char c;
    while ( ( c = * q ++ ) != 0 )
    {
        hash = ( hash * 65599 ) + c;
    }
    if ( hash == 0 ) hash = 0xFFFFFFFF;
    return hash;
}

min::unsptr min::strlen ( min::gen g )
{
    if ( is_direct_str ( g ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = direct_str_of ( g );
	return ::strlen ( u.buf );
    }

    const min::stub * s = stub_of ( g );
    if ( type_of ( s ) == SHORT_STR )
    {
	const char * p = s->v.c8;
	const char * endp = p + 8;
	while ( * p && p < endp ) ++ p;
	return p - s->v.c8;
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == LONG_STR,
	             "argument is not string" );
	return MUP::long_str_of ( s )->length;
    }
}

min::uns32 min::strhash ( min::gen g )
{
    if ( is_direct_str ( g ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = direct_str_of ( g );
	return strhash ( u.buf );
    }

    const min::stub * s = min::stub_of ( g );
    if ( type_of ( s ) == SHORT_STR )
	return strnhash ( s->v.c8, 8 );
    else
    {
	MIN_ASSERT ( type_of ( s ) == LONG_STR,
	             "argument is not string" );
	return MUP::long_str_of ( s )->hash;
    }
}

char * min::strcpy ( char * p, min::gen g )
{
    if ( is_direct_str ( g ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = direct_str_of ( g );
	return ::strcpy ( p, u.buf );
    }

    const min::stub * s = min::stub_of ( g );
    if ( type_of ( s ) == SHORT_STR )
    {
	if ( s->v.c8[7] )
	{
	    p[8] = 0;
	    return ::strncpy ( p, s->v.c8, 8 );
	}
	return ::strcpy ( p, s->v.c8 );
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == LONG_STR,
	             "argument is not string" );
	return ::strcpy
	    ( p, MUP::str_of
		   ( MUP::long_str_of ( s ) ) );
    }
}

char * min::strncpy
	( char * p, min::gen g, min::unsptr n )
{
    if ( is_direct_str ( g ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = direct_str_of ( g );
	return ::strncpy ( p, u.buf, n );
    }

    const min::stub * s = min::stub_of ( g );
    if ( type_of ( s ) == SHORT_STR )
    {
	if ( s->v.c8[7] && n >= 9 )
	    p[8] = 0;
	return ::strncpy
		 ( p, s->v.c8, n < 8 ? n : 8 );
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == LONG_STR,
	             "argument is not string" );
	return ::strncpy
	    ( p, MUP::str_of
		   ( MUP::long_str_of ( s ) ),
		 n );
    }
}

int min::strcmp ( const char * p, min::gen g )
{
    if ( is_direct_str ( g ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = direct_str_of ( g );
	return ::strcmp ( p, u.buf );
    }

    const min::stub * s = min::stub_of ( g );
    if ( type_of ( s ) == SHORT_STR )
    {
	if ( s->v.c8[7] )
	{
	    int r = ::strncmp ( p, s->v.c8, 8 );
	    if ( r != 0 )
	        return r;
	    else
	        return ::strcmp ( p + 8, "" );
	}
	return ::strcmp ( p, s->v.c8 );
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == LONG_STR,
	             "argument is not string" );
	return ::strcmp
	    ( p, MUP::str_of
		   ( MUP::long_str_of ( s ) ) );
    }
}

int min::strncmp
	( const char * p, min::gen g, min::unsptr n )
{
    if ( is_direct_str ( g ) )
    {
        union { char buf[8]; min::uns64 str; } u;
	u.str = direct_str_of ( g );
	return ::strncmp ( p, u.buf, n );
    }

    const min::stub * s = min::stub_of ( g );
    if ( type_of ( s ) == SHORT_STR )
    {
	if ( s->v.c8[7] && n >= 9 )
	{
	    int r = ::strncmp ( p, s->v.c8, 8 );
	    if ( r != 0 )
	        return r;
	    else
	        return ::strcmp ( p + 8, "" );
	}
	return ::strncmp ( p, s->v.c8, n );
    }
    else
    {
	MIN_ASSERT ( type_of ( s ) == LONG_STR,
	             "argument is not string" );
	return ::strncmp
	    ( p, MUP::str_of
		   ( MUP::long_str_of ( s ) ),
		 n );
    }
}

// Perform the new_str_stub_gen operation where n is the
// exact number of characters in p that are to be used
// (instead of the maximum).  There must be no NULs in
// the first n characters of p.
//
min::gen MINT::new_str_stub_gen
	( min::ptr<const char> p, min::unsptr n )
{
    if ( n > min::max_id_strlen )
    {
	min::stub * s = MUP::new_acc_stub();
        MINT::new_long_str_stub ( p, n, s );
	return new_stub_gen ( s );
    }
    uns32 hash = strnhash ( ~ p, n );
    uns32 h = hash & MINT::str_hash_mask;
    const char * q;

    min::stub * s = MINT::str_acc_hash[h];
    while ( s != MINT::null_stub )
    {
	uns64 c = MUP::control_of ( s );

        if (    n <= 8
	     && type_of ( s ) == SHORT_STR
	     && ::strncmp ( ~ p, s->v.c8, n ) == 0
	     && (    n == 8
	          || s->v.c8[n] == 0 ) )
	{
	    c |= MINT::hash_acc_set_flags;
	    c &= ~ MINT::hash_acc_clear_flags;
	    MUP::set_control_of ( s, c );
	    return new_stub_gen ( s );
	}
	else if (    n > 8
	          && type_of ( s ) == LONG_STR
	          && ::strncmp
		       ( ~ p, q = MUP::str_of (
			            MUP::long_str_of
				        ( s ) ),
			 n )
		     == 0
		  && q[n] == 0 )
	{
	    c |= MINT::hash_acc_set_flags;
	    c &= ~ MINT::hash_acc_clear_flags;
	    MUP::set_control_of ( s, c );
	    return new_stub_gen ( s );
	}
	s = MUP::stub_of_acc_control ( c );
    }

    s = MINT::str_aux_hash[h];
    while ( s != MINT::null_stub )
    {
	min::stub * s2 =
	    (min::stub *) MUP::ptr_of ( s );
	s = (min::stub *)
	    MUP::stub_of_control
		( MUP::control_of ( s ) );

        if (    n <= 8
	     && type_of ( s2 ) == SHORT_STR
	     && ::strncmp ( ~ p, s2->v.c8, n ) == 0
	     && (    n == 8
	          || s2->v.c8[n] == 0 ) )
	{
	    uns64 c = MUP::control_of ( s2 );
	    c |= MINT::hash_acc_set_flags;
	    c &= ~ MINT::hash_acc_clear_flags;
	    MUP::set_control_of ( s2, c );
	    return new_stub_gen ( s2 );
	}
	else if (    n > 8
	          && type_of ( s2 ) == LONG_STR
	          && ::strncmp
		       ( ~ p, q = MUP::str_of (
			            MUP::long_str_of
				        ( s2 ) ),
			  n )
		     == 0
		  && q[n] == 0 )
	{
	    uns64 c = MUP::control_of ( s2 );
	    c |= MINT::hash_acc_set_flags;
	    c &= ~ MINT::hash_acc_clear_flags;
	    MUP::set_control_of ( s2, c );
	    return new_stub_gen ( s2 );
	}
    }

    min::stub * s2 = MUP::new_acc_stub();
    if ( n <= 8 )
    {
	MUP::set_type_of ( s2, SHORT_STR );
	s2->v.u64 = 0;
	::strncpy ( s2->v.c8, ~ p, n );
    }
    else
        MINT::new_long_str_stub ( p, n, s2, hash );

    s = MUP::new_aux_stub ();
    MUP::set_ptr_of ( s, s2 );
    MUP::set_control_of
	( s,
	  MUP::new_control_with_type
	      ( HASHTABLE_AUX,
		MINT::str_aux_hash[h] ));
    MINT::str_aux_hash[h] = s;

    return new_stub_gen ( s2 );
}

min::gen min::new_str_gen
	( const min::Uchar * p, min::unsptr n )
{
    char buffer[8*n+1];
    char * q = buffer;
    unsptr m = 0;
    while ( n -- )
        m += min::unicode_to_utf8 ( q, * p ++ );
    return internal::new_str_gen ( buffer, m );
}

bool min::strto ( min::int32 & value,
		  const min::str_ptr sp,
		  min::unsptr & i, int base )
{
    const char * beginp =
        ~ ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    const char * p = beginp;
    while ( isspace ( * p ) ) ++ p;
    min::int64 v = strtol ( p, & endp, base );
    if ( errno == EINVAL ) return false;
    if ( endp == p ) return false;
    if ( v < INT_MIN ) v = INT_MIN;
    else if ( v > INT_MAX ) v = INT_MAX;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::int64 & value,
		  const min::str_ptr sp,
		  min::unsptr & i, int base )
{
    const char * beginp =
        ~ ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    const char * p = beginp;
    while ( isspace ( * p ) ) ++ p;
    min::int64 v = strtoll ( p, & endp, base );
    if ( errno == EINVAL ) return false;
    if ( endp == p ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::uns32 & value,
		  const min::str_ptr sp,
		  min::unsptr & i, int base )
{
    const char * beginp =
        ~ ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    const char * p = beginp;
    while ( isspace ( * p ) ) ++ p;
    if ( * p == '-' ) return false;
    min::uns64 v = strtoul ( p, & endp, base );
    if ( errno == EINVAL ) return false;
    if ( endp == p ) return false;
    if ( v > UINT_MAX ) v = UINT_MAX;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::uns64 & value,
		  const min::str_ptr sp,
		  min::unsptr & i, int base )
{
    const char * beginp =
        ~ ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    const char * p = beginp;
    while ( isspace ( * p ) ) ++ p;
    if ( * p == '-' ) return false;
    min::uns64 v = strtoull ( p, & endp, base );
    if ( errno == EINVAL ) return false;
    if ( endp == p ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::float32 & value,
		  const min::str_ptr sp,
		  min::unsptr & i )
{
    const char * beginp =
        ~ ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    const char * p = beginp;
    while ( isspace ( * p ) ) ++ p;
    min::float32 v = strtof ( p, & endp );
    if ( endp == p ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::float64 & value,
		  const min::str_ptr sp,
		  min::unsptr & i )
{
    const char * beginp =
        ~ ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    const char * p = beginp;
    while ( isspace ( * p ) ) ++ p;
    min::float64 v = strtod ( p, & endp );
    if ( endp == p ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto
	( min::int32 & value, min::gen g, int base )
{
    min::str_ptr sp ( g );
    if ( ! sp ) return false;
    min::unsptr i = 0;
    min::int32 v;
    if ( ! strto ( v, sp, i, base ) ) return false;
    while ( isspace ( sp[i] ) ) ++ i;
    if ( sp[i] != 0 ) return false;
    value = v;
    return true;
}

bool min::strto
	( min::int64 & value, min::gen g, int base )
{
    min::str_ptr sp ( g );
    if ( ! sp ) return false;
    min::unsptr i = 0;
    min::int64 v;
    if ( ! strto ( v, sp, i, base ) ) return false;
    while ( isspace ( sp[i] ) ) ++ i;
    if ( sp[i] != 0 ) return false;
    value = v;
    return true;
}

bool min::strto
	( min::uns32 & value, min::gen g, int base )
{
    min::str_ptr sp ( g );
    if ( ! sp ) return false;
    min::unsptr i = 0;
    min::uns32 v;
    if ( ! strto ( v, sp, i, base ) ) return false;
    while ( isspace ( sp[i] ) ) ++ i;
    if ( sp[i] != 0 ) return false;
    value = v;
    return true;
}

bool min::strto
	( min::uns64 & value, min::gen g, int base )
{
    min::str_ptr sp ( g );
    if ( ! sp ) return false;
    min::unsptr i = 0;
    min::uns64 v;
    if ( ! strto ( v, sp, i, base ) ) return false;
    while ( isspace ( sp[i] ) ) ++ i;
    if ( sp[i] != 0 ) return false;
    value = v;
    return true;
}

bool min::strto ( min::float32 & value, min::gen g )
{
    min::str_ptr sp ( g );
    if ( ! sp ) return false;
    min::unsptr i = 0;
    min::float32 v;
    if ( ! strto ( v, sp, i ) ) return false;
    while ( isspace ( sp[i] ) ) ++ i;
    if ( sp[i] != 0 ) return false;
    value = v;
    return true;
}

bool min::strto ( min::float64 & value, min::gen g )
{
    min::str_ptr sp ( g );
    if ( ! sp ) return false;
    min::unsptr i = 0;
    min::float64 v;
    if ( ! strto ( v, sp, i ) ) return false;
    while ( isspace ( sp[i] ) ) ++ i;
    if ( sp[i] != 0 ) return false;
    value = v;
    return true;
}


// Labels
// ------

min::uns32 min::labhash
	( const min::gen * p, min::uns32 n )
{
    uns32 hash = min::labhash_initial;
    while ( n -- )
    {
        MIN_ASSERT ( is_name ( * p ),
	             "first argument contains element"
		     " that is not a name" );
        hash = labhash ( hash, min::hash ( * p ++ ) );
    }
    return hash;
}

min::gen min::new_lab_gen
	( min::ptr<const min::gen> p, min::uns32 n )
{
    uns32 hash = labhash ( ~ p, n );
    uns32 h = hash & MINT::lab_hash_mask;

    // Search for existing label stub with given
    // elements.
    //
    min::stub * s = MINT::lab_acc_hash[h];
    while ( s != MINT::null_stub )
    {
	uns64 c = MUP::control_of ( s );

	lab_ptr labp ( s );

	if ( hash != labhash ( labp ) ) continue;
	if ( n != lablen ( labp ) ) continue;

	uns32 i;
	for ( i = 0; i < n && p[i] == labp[i]; ++ i );
	if ( i == n )
	{
	    c |= MINT::hash_acc_set_flags;
	    c &= ~ MINT::hash_acc_clear_flags;
	    MUP::set_control_of ( s, c );
	    return new_stub_gen ( s );
	}
	s = MUP::stub_of_acc_control ( c );
    }
    s = MINT::lab_aux_hash[h];
    while ( s != MINT::null_stub )
    {
        min::stub * s2 =
	    (min::stub *) MUP::ptr_of ( s );
	s = MUP::stub_of_control
		    ( MUP::control_of ( s ) );

	lab_ptr labp ( s2 );

	if ( hash != labhash ( labp ) ) continue;
	if ( n != lablen ( labp ) ) continue;

	uns32 i;
	for ( i = 0; i < n && p[i] == labp[i]; ++ i );
	if ( i == n )
	{
	    uns64 c = MUP::control_of ( s2 );
	    c |= MINT::hash_acc_set_flags;
	    c &= ~ MINT::hash_acc_clear_flags;
	    MUP::set_control_of ( s2, c );
	    return new_stub_gen ( s2 );
	}
    }

    // Allocate new label.
    //
    min::stub * s2 = MUP::new_acc_stub ();
    MUP::new_body ( s2,   sizeof ( MINT::lab_header )
	                + n * sizeof (min::gen) );
    MINT::lab_header * lh = MINT::lab_header_of ( s2 );
    lh->length = n;
    lh->hash = hash;
    memcpy ( lh + 1, ~ p, n * sizeof ( min::gen ) );

    s = MUP::new_aux_stub ();
    MUP::set_ptr_of ( s, s2 );
    MUP::set_control_of
	( s,
	  MUP::new_control_with_type
	      ( HASHTABLE_AUX,
		MINT::lab_aux_hash[h] ));
    MINT::lab_aux_hash[h] = s;

    MUP::set_type_of ( s2, LABEL );
    return new_stub_gen ( s2 );
}

min::gen min::new_lab_gen
	( const char * s1,
	  const char * s2 )
{
    min::locatable_gen g1 = min::new_str_gen ( s1 );
    min::locatable_gen g2 = min::new_str_gen ( s2 );
    min::gen elements[2] = { g1, g2 };
    return min::new_lab_gen ( elements, 2 );
}

min::gen min::new_lab_gen
	( const char * s1,
	  const char * s2,
	  const char * s3 )
{
    min::locatable_gen g1 = min::new_str_gen ( s1 );
    min::locatable_gen g2 = min::new_str_gen ( s2 );
    min::locatable_gen g3 = min::new_str_gen ( s3 );
    min::gen elements[3] = { g1, g2, g3 };
    return min::new_lab_gen ( elements, 3 );
}

min::gen min::new_lab_gen
	( const char * s1,
	  const char * s2,
	  const char * s3,
	  const char * s4 )
{
    min::locatable_gen g1 = min::new_str_gen ( s1 );
    min::locatable_gen g2 = min::new_str_gen ( s2 );
    min::locatable_gen g3 = min::new_str_gen ( s3 );
    min::locatable_gen g4 = min::new_str_gen ( s4 );
    min::gen elements[4] = { g1, g2, g3, g4 };
    return min::new_lab_gen ( elements, 4 );
}

min::gen min::new_lab_gen
	( const char * s1,
	  const char * s2,
	  const char * s3,
	  const char * s4,
	  const char * s5 )
{
    min::locatable_gen g1 = min::new_str_gen ( s1 );
    min::locatable_gen g2 = min::new_str_gen ( s2 );
    min::locatable_gen g3 = min::new_str_gen ( s3 );
    min::locatable_gen g4 = min::new_str_gen ( s4 );
    min::locatable_gen g5 = min::new_str_gen ( s5 );
    min::gen elements[5] = { g1, g2, g3, g4, g5 };
    return min::new_lab_gen ( elements, 5 );
}

// Return true if v is a label with all string elements.
//
inline bool is_str_lab ( min::gen v )
{
    min::lab_ptr labp ( v );
    if ( labp == min::NULL_STUB ) return false;
    for ( unsigned i = 0; i < min::lablen ( labp );
                          ++ i )
    {
        if ( ! min::is_str ( labp[i] ) )
	    return false;
    }
    return true;
}

// Packed Structures and Vectors
// ------ ---------- --- -------

void *** MINT::packed_subtypes;
min::uns32 MINT::packed_subtype_count = 1;
min::uns32 MINT::max_packed_subtype_count = 0;

const min::stub * MINT::packed_struct_new_stub
	( MINT::packed_struct_descriptor * psd )
{
    min::stub * s = MUP::new_acc_stub();
    MUP::new_body ( s, psd->size );
    uns32 * tp = (uns32 *) MUP::ptr_of ( s );
    memset ( (void *) tp, 0, psd->size );
    * tp = psd->subtype;
    MUP::set_type_of ( s, PACKED_STRUCT );
    return s;
}

// Files
// -----

const min::position min::MISSING_POSITION =
    { 0xFFFFFFFF, 0xFFFFFFFF };

const min::phrase_position min::MISSING_PHRASE_POSITION =
    { min::MISSING_POSITION, min::MISSING_POSITION };

static min::uns32 file_gen_disp[2] =
    { min::DISP ( & min::file_struct::file_name ),
      min::DISP_END };

static min::uns32 file_stub_disp[6] =
    { min::DISP ( & min::file_struct::buffer ),
      min::DISP ( & min::file_struct::line_index ),
      min::DISP ( & min::file_struct::ifile ),
      min::DISP ( & min::file_struct::printer ),
      min::DISP ( & min::file_struct::ofile ),
      min::DISP_END };

static min::packed_struct<min::file_struct> file_type
    ( "min::file_type",
      ::file_gen_disp, ::file_stub_disp );

static min::packed_vec<char> file_buffer_type
    ( "min::file_buffer_type" );

static min::packed_vec<min::uns32> file_line_index_type
    ( "min::file_line_index_type" );

static min::uns32 phrase_position_vec_stub_disp[2] =
    { min::DISP ( & min::phrase_position_vec_header
                       ::file ),
      min::DISP_END };

static min::packed_vec<min::phrase_position,
                       min::phrase_position_vec_header>
    phrase_position_vec_type
    ( "min::phrase_position_vec_type",
      NULL, NULL,
      NULL, ::phrase_position_vec_stub_disp);

void min::init ( min::ref<min::file> file )
{
    if ( file == NULL_STUB )
    {
	::file_buffer_type.initial_max_length = 4096;
	::file_line_index_type.initial_max_length = 128;

        file = ::file_type.new_stub();
	buffer_ref(file) =
	    ::file_buffer_type.new_stub();
	file_name_ref(file) = MISSING();
    }
}

void min::init_line_format
	( min::ref<min::file> file,
	  const min::line_format * line_format )
{
    init ( file );
    file->line_format = line_format;
}

void min::init_file_name
	( min::ref<min::file> file,
	  min::gen file_name )
{
    init ( file );
    file_name_ref(file) = file_name;
}

void min::init_ostream
	( min::ref<min::file> file,
	  std::ostream & ostream )
{
    init ( file );
    file->ostream = & ostream;
}

void min::init_ofile
	( min::ref<min::file> file,
	  min::file ofile )
{
    init ( file );
    ofile_ref(file) = ofile;
}

void min::init_printer
	( min::ref<min::file> file,
	  min::printer printer )
{
    init ( file );
    printer_ref(file) = printer;
}

void min::init_input
	( min::ref<min::file> file,
	  const min::line_format * line_format,
	  min::uns32 spool_lines )
{
    init ( file );
    min::pop ( file->buffer,
	       file->buffer->length );
    min::resize ( file->buffer,
	          file_buffer_type.initial_max_length );
    file->end_offset = 0;
    file->end_count = 0;
    file->file_lines = min::NO_LINE;
    file->next_line_number = 0;
    file->next_offset = 0;

    file->line_format = line_format;
    file->spool_lines = spool_lines;
    if ( spool_lines != 0 )
    {
        if ( file->line_index == min::NULL_STUB )
	    min::line_index_ref(file) =
		::file_line_index_type.new_stub();
	else
	{
	    min::pop ( file->line_index,
		       file->line_index->length );
	    min::resize ( file->line_index,
			  file_line_index_type
			      .initial_max_length );
	}
    }
    else
    {
        if ( file->line_index != min::NULL_STUB )
	    min::line_index_ref(file) =
		min::NULL_STUB;
    }

    file->istream = NULL;
    ifile_ref(file) = NULL_STUB;
    file_name_ref(file) = MISSING();
}

void min::init_input_stream
	( min::ref<min::file> file,
	  std::istream & istream,
	  const min::line_format * line_format,
	  min::uns32 spool_lines )
{
    init_input ( file, line_format, spool_lines );
    file->istream = & istream;
}

void min::init_input_file
	( min::ref<min::file> file,
	  min::file ifile,
	  const min::line_format * line_format,
	  min::uns32 spool_lines )
{
    init_input ( file, line_format, spool_lines );
    ifile_ref(file) = ifile;
}

void min::init_input_string
	( min::ref<min::file> file,
	  min::ptr<const char> string,
	  const min::line_format * line_format,
	  min::uns32 spool_lines )
{
    init_input ( file, line_format, spool_lines );
    load_string ( file, string );
    complete_file ( file );
}

void min::load_string
	( min::file file,
	  min::ptr<const char> string )
{
    MIN_ASSERT ( ! min::file_is_complete ( file ),
                 "file is complete" );

    uns64 length = ::strlen ( ~ string );
    uns32 offset = file->buffer->length;
    MIN_ASSERT ( length <= ( 1ull << 32 ) - 1 - offset,
                 "string is too long or file buffer"
		 " is too full" );

    min::push ( file->buffer, length, string );

    for ( uns32 i = offset; i < length; ++ i )
    {
        if ( file->buffer[i] == '\n' )
	    min::end_line ( file, i );
    }
}

bool min::init_input_named_file
	( min::ref<min::file> file,
	  min::gen file_name,
	  const min::line_format * line_format,
	  min::uns32 spool_lines )
{
    init_input ( file, line_format, spool_lines );
    file_name_ref(file) = file_name;
    if ( ! load_named_file ( file, file_name ) )
        return false;
    else
    {
	min::complete_file ( file );
	return true;
    }
}

bool min::load_named_file
	( min::file file,
	  min::gen file_name )
{
    MIN_ASSERT ( ! min::file_is_complete ( file ),
                 "file is complete" );

    min::str_ptr fname ( file_name );
    min::uns32 offset = file->buffer->length;

    // Use OS independent min::os::file_size.
    //
    char error_buffer[512];
    uns64 file_size;
    if ( ! min::os::file_size
               ( file_size,
	         ~ min::begin_ptr_of ( fname ),
	         error_buffer ) )
    {
	ERR << "During attempt to find the size of"
	       " file "
	    << fname << ": "
	    << min::reserve ( 40 )
	    << error_buffer << min::eol;
        return false;
    }

    if ( file_size > ( 1ull << 32 ) - 1 - offset )
    {
        ERR << "File "
	    << fname << ": "
	    << min::reserve ( 40 )
	    << "File too large ( size = " << file_size
	    << " bytes)" << min::eol;
	return false;
    }

    // We use FILE IO because it is standard for C
    // while open/read is OS dependent.

    FILE * in =
        fopen ( ~ min::begin_ptr_of ( fname ), "r" );

    if ( in == NULL )
    {
        ERR << "Opening file "
	    << fname << ": "
	    << min::reserve ( 40 )
	    << strerror ( errno )
	    << min::eol;
	return false;
    }

    min::push ( file->buffer, file_size );

    errno = 0;
    uns64 bytes =
        fread ( ~ ( file->buffer + offset ), 1,
	        (size_t) file_size, in );

    if ( bytes < file_size )
        min::pop ( file->buffer, file_size - bytes );

    for ( uns32 i = offset;
          i < file->buffer->length; ++ i )
    {
	char c = file->buffer[i];
        if ( c == '\n' || c == 0 )
	    min::end_line ( file, i );
    }

    if ( bytes != file_size )
    {
	if ( errno != 0 )
	    ERR << "Reading file "
		<< fname << ": "
	        << min::reserve ( 40 )
		<< strerror ( errno )
		<< min::eol;
	else
	    ERR << "Reading file "
		<< fname << ": "
	        << min::reserve ( 40 )
		<< " Only " << bytes
		<< " bytes out of " << file_size
		<< " read"
		<< min::eol;
	fclose ( in );
	return false;
    }
    else if ( getc ( in ) != EOF )
    {
	ERR << "Reading file "
	    << fname << ": "
	    << min::reserve ( 40 )
	    << "File longer than expected (more than "
	    << file_size << " bytes were read)"
	    << min::eol;
	fclose ( in );
	return false;
    }

    fclose ( in );

    return true;
}

min::uns32 min::next_line ( min::file file )
{
    uns32 line_offset = file->next_offset;

    if ( line_offset >= file->end_offset )
    {
        if ( file->file_lines != min::NO_LINE )
	    return min::NO_LINE;

	// Input line.
	//
	if ( file->istream != NULL )
	{
	    MIN_ASSERT
	        ( file->ifile == NULL_STUB,
		  "file has non-NULL istream and ifile"
		  " members" );

	    int c;
	    while ( c = file->istream->get(),
		    c != EOF && c != '\n' && c != 0 )
		min::push(file->buffer) = (char) c;

	    if ( c == EOF )
	    {
		min::complete_file ( file );
	        return min::NO_LINE;
	    }
	}
	else if ( file->ifile != NULL_STUB )
	{
	    min::file ifile = file->ifile;
	    uns32 ioffset = min::next_line ( ifile );

	    if ( ioffset == min::NO_LINE )
	    {
		ioffset =
		    min::remaining_offset ( ifile );
		uns32 length =
		    min::remaining_length ( ifile );
	        if ( length > 0 )
		{
		    min::push
		        ( file->buffer, length,
			  ifile->buffer + ioffset );
		    min::skip_remaining ( ifile );
		}
		if ( min::file_is_complete ( ifile ) )
		    min::complete_file ( file );
		return min::NO_LINE;
	    }

	    uns32 length =
		::strlen
		    ( ~ ( ifile->buffer + ioffset ) );
	    min::push ( file->buffer, length,
	                ifile->buffer + ioffset );
	}
	else
	    return min::NO_LINE;

	min::end_line ( file );
    }

    file->next_offset +=
        1 + ::strlen
	        ( ~ ( file->buffer + line_offset ) );
    ++ file->next_line_number;

    if ( file->line_index != NULL_STUB )
	min::push(file->line_index) = line_offset;

    return line_offset;
}

min::uns32 min::line
	( min::file file, uns32 line_number )
{
    if ( file->line_index == NULL_STUB )
        return min::NO_LINE;
    else if ( line_number >= file->next_line_number )
        return min::NO_LINE;
    else if (   file->line_index->length
              < file->next_line_number - line_number )
        return min::NO_LINE;
    else
        return file->line_index
	           [  file->line_index->length
	            - (   file->next_line_number
		        - line_number)];
}

static min::uns32 end_line
        ( min::printer printer, min::uns32 op_flags );
min::uns32 min::print_line
	( min::printer printer,
	  min::file file,
	  min::uns32 line_number,
	  const min::line_format * line_format,
	  const min::phrase_position & position )
{
    if ( line_format == NULL )
        line_format = file->line_format;
    if ( line_format == NULL )
        line_format = printer->print_format.line_format;
    MIN_ASSERT ( line_format != NULL,
                 "printer->print_format.line_format"
		 " == NULL" );
    min::uns32 op_flags = line_format->op_flags;

    const min::uns32 * char_flags =
	printer->print_format.char_flags;
    uns32 offset = min::line ( file, line_number );
    unsptr file_line_length;

    bool html = (   printer->print_format.op_flags
                  & min::OUTPUT_HTML )
		&&
		line_format->line_class != NULL
		&&
		line_format->line_number_class != NULL;

    if ( html )
    {
        min::tag(printer)
	    << "<tr>"
	    << "<td class='"
	    << line_format->line_number_class
	    << "'>";
	printer << line_number + 1 << ":";
        min::tag(printer)
	    << "</td><td class='"
	    << line_format->line_class
	    << "'>";
    }
    
    const char * message = NULL;
    if ( offset == min::NO_LINE )
    {
	if ( line_number == file->file_lines )
	{
	    message = line_format->end_of_file;
	    file_line_length = partial_length ( file );
	    if ( file_line_length != 0 )
		offset = partial_offset ( file );
	    else
	        offset = 0;
	}
	else
	{
	    file_line_length = 0;
	    offset = 0;
	    message = line_format->unavailable_line;
	}
	op_flags &= ~ min::DISPLAY_EOL;

    }
    else
        file_line_length =
	    ::strlen ( ~ ( file->buffer + offset ) );

    // Move line to stack and convert to Uchars.
    //
    min::Uchar buffer[file_line_length+40];
	// +40 for message length and possible
	// end of line
    min::Uchar * bp = buffer;
    const char * cp = ~ ( file->buffer + offset );
    min::unsptr length = min::utf8_to_unicode
	( bp, buffer + file_line_length,
	  cp, cp + file_line_length );

    // Blank line check.
    //
    if ( line_format->blank_line == NULL )
	; // do nothing
    else if ( message != NULL )
	; // do nothing
    else if ( op_flags & min::DISPLAY_EOL )
	; // do nothing
    else if ( length == 0 )
	message = line_format->blank_line;
    else
    {
	bp = buffer;
	min::Uchar * endbp = bp + length;

	min::uns32 blank_flags = min::IS_HSPACE;
	if ( op_flags & min::DISPLAY_NON_GRAPHIC )
	    blank_flags = 0;

	for ( ; bp < endbp; ++ bp )
	{
	    uns16 i = min::Uindex ( *bp );
	    min::uns32 cflags = char_flags[i];
	    if ( cflags & blank_flags )
		continue;
	    else
		break;
	}

	if ( bp >= endbp )
	    message = line_format->blank_line;
    }

    if ( message != NULL )
    {
	// Add message to line.
	//
	const char * p = message;
	while ( * p )
	    buffer[length++] = (min::Uchar) * p ++;
    }

    if ( op_flags & min::DISPLAY_EOL )
	buffer[length++] =
	    min::unicode::SOFTWARE_NL;

    // Print line Uchars.
    //
    min::ptr<const min::Uchar> p =
	min::new_ptr<const min::Uchar> ( buffer );
    min::uns32 width = (min::uns32) -1;

    if ( html
         &&
	 line_format->line_mark_class != NULL
	 &&
	 position.begin
	 &&
	 line_number >= position.begin.line
	 &&
	 line_number <= position.end.line )
    {
        // Add <span class='...'>...</span> to line.
	//
	unsptr first = 0;
	unsptr last = length;
	if ( file_line_length > 0 )
	{
	    min::unsptr index = 0;
	    const char * beginp = ~ ( file->buffer + offset );
	    const char * endp = beginp + file_line_length;
	    const char * cp = beginp;
	    if ( line_number == position.begin.line )
	    {
		const char * pp =
		    beginp + position.begin.offset;
		while ( cp < endp )
		{
		    min::utf8_to_unicode ( cp, endp );
		    ++ index;
		    if ( pp < cp )
		    {
		        first = index - 1;
			break;
		    }
		}
	    }
	    if ( line_number == position.end.line )
	    {
		const char * pp =
		    beginp + position.end.offset;
		if ( pp < cp )
		    last = index;
		else while ( cp < endp )
		{
		    min::utf8_to_unicode ( cp, endp );
		    ++ index;
		    if ( pp < cp )
		    {
		        last = index - 1;
			break;
		    }
		}
	    }
	}

	min::unsptr n;
	if ( first > 0 )
	{
	    n = first;
	    MINT::print_unicode
		( printer, op_flags, n, p, width );
	}
	min::tag(printer) << "<span class='"
	                  << line_format->line_mark_class
			  << "'>";
	n = last - first;
	MINT::print_unicode
	    ( printer, op_flags, n, p, width );
	min::tag(printer) << "</span>";
	if ( last < length )
	{
	    n = length - last;
	    MINT::print_unicode
		( printer, op_flags, n, p, width );
	}
    }
    else
    {
        // Do NOT add <span class='...'>...</span>
	// to line.
	//
	MINT::print_unicode
	    ( printer, op_flags, length, p, width );
    }


    if ( html ) min::tag(printer) << "</td></tr>";


    // Mimic min::eol with different op_flags.
    //
    op_flags &= ~ min::DISPLAY_EOL;
        // EOL was displayed above if nessary to get
	// it inside <td>...</td> when html is true.
    min::uns32 column = ::end_line
	( printer, op_flags );
    if (   printer->print_format.op_flags
         & min::FLUSH_ON_EOL )
	min::flush_file ( printer->file );

    if ( html ) return column;
        // html highlight implemented above.

    if ( ! position.begin
	 ||
         line_format->mark == 0 )
        return column;

    if ( line_number < position.begin.line
         ||
	 line_number > position.end.line )
    {
        // Blank line of `marks' for this file line.
	//
        printer << min::eol;
	return column;
    }

    min::uns32 first_column = 0;
    min::uns32 end_column = column;

    if ( line_number == position.begin.line )
	first_column =
	    print_line_column
		( file, position.begin,
		  printer->print_format, line_format );
    if ( line_number == position.end.line )
	end_column =
	    print_line_column
		( file, position.end,
		  printer->print_format, line_format );

    if ( end_column <= first_column )
        end_column = first_column + 1;

    for ( uns32 i = 0; i < first_column; ++ i )
	printer << ' ';

    for ( uns32 i = first_column; i < end_column; ++ i )
	printer << line_format->mark;

    printer << min::eol;

    return column;
}

min::uns32 min::print_line_column
	( min::file file,
	  const min::position & position,
	  const min::print_format & print_format,
	  const min::line_format * line_format )
{
    if ( line_format == NULL )
        line_format = file->line_format;
    if ( line_format == NULL )
        line_format = print_format.line_format;
    MIN_ASSERT ( line_format != NULL,
                 "print_format.line_format == NULL" );

    min::uns32 column = 0;
    min::uns32 offset =
        min::line ( file, position.line );
    min::uns32 length;
    
    if ( offset == min::NO_LINE )
    {
	if ( position.line == file->file_lines )
	{
	    length = min::partial_length ( file );
	    if ( length != 0 )
		offset = min::partial_offset ( file );
	    else return 0;
	}
	else return 0;
    }
    else
        length =
	    ::strlen ( ~ ( file->buffer + offset ) );

    min::pwidth ( column, ~ ( file->buffer + offset ),
    		  position.offset <= length ?
		      position.offset : length,
		  print_format, line_format );
    return column;
}

void min::print_phrase_lines
	( min::printer printer,
	  min::file file,
	  const min::phrase_position & position,
	  const min::line_format * line_format )
{
    MIN_REQUIRE
        ( position.end.line >= position.begin.line );

    if ( line_format == NULL )
        line_format = file->line_format;
    if ( line_format == NULL )
        line_format = printer->print_format.line_format;
    MIN_ASSERT ( line_format != NULL,
                 "printer->print_format.line_format"
		 " == NULL" );

    min::position begin = position.begin;
    min::position end   = position.end;

    bool html = (   printer->print_format.op_flags
                  & min::OUTPUT_HTML )
		&&
		line_format->line_group_class != NULL
		&&
		line_format->line_class != NULL
		&&
		line_format->line_number_class != NULL;

    if ( html )
        min::tag(printer)
	    << "<table class='"
	    << line_format->line_group_class
	    << "'>" << std::endl;

    uns32 line = begin.line;

    min::print_line
        ( printer, file, line, line_format, position );

    while ( true )
    {
	if ( line == end.line )
	    break;

	++ line;

	if ( line == end.line && end.offset == 0 )
	    break;

	min::print_line
	    ( printer, file, line, line_format, position );
    }

    if ( html )
        min::tag(printer) << "</table>" << std::endl;
}

min::printer operator <<
	( min::printer printer,
	  const min::pline_numbers & pline_numbers )
{
    min::file file = pline_numbers.file;
    if ( file->file_name != min::MISSING() )
    {
        min::str_ptr sp ( file->file_name );
	printer << sp << ": ";
    }
    printer << min::save_print_format
            << min::no_auto_break;

    if ( pline_numbers.first == pline_numbers.last )
        printer << "line " << pline_numbers.first + 1;
    else
        printer << "lines " << pline_numbers.first + 1
	        << "-" << pline_numbers.last + 1;

    return printer << min::restore_print_format;
}

void min::flush_file
	( min::file file, bool copy_completion )
{
    while ( true )
    {
        uns32 offset = min::next_line ( file );
	if ( offset == min::NO_LINE ) break;
	min::flush_line ( file, offset );
    }
    if ( min::remaining_length ( file ) > 0 )
    {
	min::flush_remaining ( file );
	min::skip_remaining ( file );
    }
    if (    copy_completion
         && min::file_is_complete ( file )
	 && file->ofile != NULL_STUB )
	min::complete_file (file->ofile );
}

inline void file_write_ostream
    ( min::file file, min::uns32 offset,
                      min::uns32 length )
{
    const char * q = ~ ( file->buffer + offset );

    if ( file->op_flags & min::OUTPUT_HTML )
    {
        const char * p = q;
	const char * endp = q + length;

	for ( ; p < endp; ++ p )
	{
	    min::uns8 c = (min::uns8) * p;
	    const char * r = ::html_reserved_table[c];
	    if ( r == NULL ) continue;

	    if ( c < 127 )
	    {
	        // Most common case.
		//
	        if ( p > q )
		    (*file->ostream).write ( q, p - q );
		* file->ostream << r;
		q = p + 1;
		continue;
	    }

	    const char * pnext = p;
	    if ( c == 0xC2 )
	    {
	        if ( p + 1 >= endp ) continue;

	        c = (min::uns8) p[1];
		if      ( c == 0xA0 ) r = "&nbsp;";
		else if ( c == 0xA9 ) r = "&copy;";
		else if ( c == 0xAE ) r = "&reg;";
		else if ( c == 0xA3 ) r = "&pound;";
		else if ( c == 0xB0 ) r = "&deg;";
		else continue;
		++ pnext;
	    }
	    else if ( c == 0xE2 )
	    {
	        if ( p + 2 >= endp ) continue;

	        c = (min::uns8) p[1];
		if ( c == 0x80 )
		{
		    c = (min::uns8) p[2];
		    if      ( c == 0x93 ) r = "&ndash;";
		    else if ( c == 0x94 ) r = "&mdash;";
		    else continue;
		}
		else if ( c == 0x84 )
		{
		    c = (min::uns8) p[2];
		    if ( c == 0xA2 )      r = "&trade;";
		    else continue;
		}
		else if ( c == 0x89 )
		{
		    c = (min::uns8) p[2];
		    if      ( c == 0x88 ) r = "&asymp;";
		    else if ( c == 0xA0 ) r = "&ne;";
		    else continue;
		}
		else if ( c == 0x82 )
		{
		    c = (min::uns8) p[2];
		    if ( c == 0xAC )      r = "&euro;";
		    else continue;
		}
		else continue;

		pnext += 2;
	    }
	    else continue;

	    if ( p > q )
		(*file->ostream).write ( q, p - q );
	    * file->ostream << r;
	    p = pnext;
	    q = p + 1;
	    continue;
	}
	if ( q < endp )
	    (*file->ostream).write ( q, endp - q );
    }
    else
	(*file->ostream).write ( q, length );
}

void min::flush_line
	( min::file file, min::uns32 offset )
{
    MIN_ASSERT ( offset < file->end_offset,
                 "offset argument too large: at or"
		 " beyond end of file" );

    if ( file->ostream != NULL )
    {
        uns32 length =
	    ::strlen ( ~ ( file->buffer + offset ) );
	::file_write_ostream ( file, offset, length );
        * file->ostream << std::endl;
    }

    if ( file->ofile != NULL_STUB )
    {
	min::file ofile = file->ofile;
        uns32 length =
	    ::strlen ( ~ ( file->buffer + offset ) );
	min::push ( ofile->buffer, length,
	            file->buffer + offset );
	min::end_line ( ofile );
    }

    if ( file->printer != NULL_STUB )
        file->printer << min::save_print_format
		      << min::verbatim
		      << file->buffer + offset
		      << min::restore_print_format
		      << min::eol;
}

void min::flush_remaining ( min::file file )
{
    uns32 length = min::remaining_length ( file );
    uns32 offset = min::remaining_offset ( file );
    if ( length == 0 ) return;

    min::push(file->buffer) = 0;

    if ( file->ostream != NULL )
    {
	::file_write_ostream ( file, offset, length );
	std::flush ( * file->ostream );
    }

    if ( file->ofile != NULL_STUB )
	min::push ( file->ofile->buffer, length,
	            file->buffer + offset );

    if ( file->printer != NULL_STUB )
    {
        file->printer << min::save_print_format
		      << min::verbatim
		      << file->buffer + offset
		      << min::restore_print_format;
    }

    min::pop ( file->buffer );
}

void min::flush_spool
	( min::file file, min::uns32 line_number )
{
    if ( file->line_index == NULL_STUB )
        return;

    if ( line_number == NO_LINE )
        line_number = file->next_line_number;
    else
        MIN_ASSERT (    line_number
	             <= file->next_line_number,
		     "line_number argument too large:"
		     " beyond file next_line_number" );

    MIN_REQUIRE (    file->next_line_number
	          >= file->line_index->length );
    uns32 first_spool_line_number =
          file->next_line_number
	- file->line_index->length;
    if ( first_spool_line_number >= line_number )
        return;
    uns32 spool_lines_before_line_number =
        line_number - first_spool_line_number;
    if (    spool_lines_before_line_number
         <= file->spool_lines )
        return;

    uns32 lines_to_delete =
          spool_lines_before_line_number
	- file->spool_lines;
    MIN_REQUIRE (   lines_to_delete
                  < file->line_index->length );

    uns32 buffer_offset =
	file->line_index[lines_to_delete];
    if ( buffer_offset < file->buffer->length )
	memmove ( ~ ( file->buffer + 0 ) ,
		  ~ ( file->buffer + buffer_offset ),
		    file->buffer->length
		  - buffer_offset );
    min::pop ( file->buffer, buffer_offset );
    file->next_offset -= buffer_offset;
    if ( file->end_offset != 0 )
	file->end_offset -= buffer_offset;

    uns32 lines_to_keep = file->line_index->length
                        - lines_to_delete;
    for ( uns32 i = 0; i < lines_to_keep; ++ i )
        file->line_index[i] =
	      file->line_index[i+lines_to_delete]
	    - buffer_offset;
    min::pop ( file->line_index,
	       lines_to_delete );
}

void min::rewind
	( min::file file, min::uns32 line_number )
{
    min::uns32 line_offset =
        min::line ( file, line_number );

    if ( line_offset != min::NO_LINE )
    {
	MIN_ASSERT ( file->line_index != NULL_STUB,
	             "file has no line_index" );
	MIN_ASSERT
	    ( line_number < file->next_line_number,
	      "line_number argument too large:"
	      " at or beyond file next_line_number" );
	min::uns32 lines_to_back_up =
	    file->next_line_number - line_number;
	MIN_REQUIRE (    file->line_index->length
		      >= lines_to_back_up );
	file->next_offset = line_offset;
	file->next_line_number = line_number;
	min::pop ( file->line_index, lines_to_back_up );
    }
    else if (    line_number == 0
	      && file->spool_lines == 0 )
    {
	file->next_line_number = 0;
	file->next_offset = 0;
    }
    else if ( file->next_line_number != line_number )
	MIN_ABORT ( "bad rewind line_number" );
}

std::ostream & operator <<
	( std::ostream & out, min::file file )
{
    while ( true )
    {
        min::uns32 offset = min::next_line ( file );
	if ( offset == min::NO_LINE ) break;
	out << ~ ( file->buffer + offset ) << std::endl;
    }

    if ( file->next_offset < file->buffer->length )
    {
	min::push(file->buffer) = 0;
        out << ~ ( file->buffer + file->next_offset );
	min::pop ( file->buffer );
	file->next_offset = file->buffer->length;
    }

    return out;
}
min::file operator <<
	( min::file ofile, min::file ifile )
{
    while ( true )
    {
        min::uns32 offset = min::next_line ( ifile );
	if ( offset == min::NO_LINE ) break;
        min::uns32 length =
	    ::strlen ( ~ ( ifile->buffer + offset ) );
	min::push ( ofile->buffer, length,
	            ifile->buffer + offset );
	min::end_line ( ofile );
    }

    if (   ifile->next_offset
         < ifile->buffer->length )
    {
        min::uns32 length = ifile->buffer->length
	                  - ifile->next_offset;
	min::push ( ofile->buffer, length,
	            & ifile->buffer
		          [ifile->next_offset] );
	ifile->next_offset = ifile->buffer->length;
    }

    return ofile;
}

min::printer operator <<
	( min::printer printer, min::file file )
{
    printer << min::save_print_format
	    << min::verbatim;

    while ( true )
    {
        min::uns32 offset = min::next_line ( file );
	if ( offset == min::NO_LINE ) break;
	printer << ( file->buffer + offset )
	        << min::eol;
    }

    printer << min::restore_print_format;

    if ( file->next_offset < file->buffer->length )
    {
	min::push(file->buffer) = 0;
	printer << min::save_print_format
		<< min::verbatim
	        << & file->buffer
		         [file->next_offset]
		<< min::restore_print_format;
	min::pop ( file->buffer );
	file->next_offset = file->buffer->length;
    }

    return printer;
}


min::phrase_position_vec_insptr min::init
	( min::ref<min::phrase_position_vec_insptr> vec,
	  min::file file,
	  const min::phrase_position & position,
	  min::uns32 max_length )
{
    if ( vec == NULL_STUB )
        vec = ::phrase_position_vec_type.new_stub
	          ( max_length );
    else
    {
	min::phrase_position_vec_insptr v = vec;
	min::pop ( v, vec->length );
	min::resize ( v, max_length );
    }
    file_ref(vec) = file;
    vec->position = position;
    return vec;
}

min::phrase_position_vec min::position_of
	( min::obj_vec_ptr & vp )
{
    min::attr_ptr ap ( vp );
    min::locate ( ap, min::dot_position );
    min::gen v = min::get ( ap );
    return min::phrase_position_vec ( v );
}

// Identifier Maps
// ---------- ----

static min::uns32 uns32_id_map_stub_disp[2] =
    { min::DISP ( & min::id_map_header<min::uns32>
                       ::output_hash_table ),
      min::DISP_END };

static min::uns32 uns32_id_map_gen_disp[4] =
    { min::DISP ( & min::id_map_header<min::uns32>
                       ::ID_prefix ),
      min::DISP ( & min::id_map_header<min::uns32>
                       ::ID_assign ),
      min::DISP ( & min::id_map_header<min::uns32>
                       ::input_hash_table ),
      min::DISP_END };

static min::uns32 id_map_element_gen_disp[2] =
    { 0, min::DISP_END };

static min::packed_vec
	< min::gen,
	  min::id_map_header<min::uns32>,
	  min::uns32 >
    uns32_id_map_type
    ( "uns32_id_map_type",
      ::id_map_element_gen_disp, NULL,
      ::uns32_id_map_gen_disp,
      ::uns32_id_map_stub_disp );

template < typename L >
inline min::packed_vec_insptr
		< min::gen,
		  min::id_map_header<L>,
		  L >
    new_id_map ( L length );
template<> 
inline min::packed_vec_insptr
	    < min::gen,
	      min::id_map_header<min::uns32>,
	      min::uns32 >
    new_id_map<min::uns32> ( min::uns32 length )
{
    return ::uns32_id_map_type.new_stub ( length );
}

template < typename L >
inline min::packed_vec_insptr
		< L,
		  min::packed_vec_header<L>,
		  L >
    new_id_map_output_hash_table ( L length );
template<> 
inline min::packed_vec_insptr
		< min::uns32,
		  min::packed_vec_header<min::uns32>,
		  min::uns32 >
    new_id_map_output_hash_table<min::uns32>
	( min::uns32 length )
{
    return min::uns32_packed_vec_type.new_stub
    		( length );
}

template < typename L >
inline L hash
	( min::packed_vec_ptr
	      < min::gen,
		min::id_map_header<L>,
		L > map,
	  min::gen g )
{
    typedef min::packed_vec_insptr
    		< L, min::packed_vec_header<L>, L >
	output_hash_table_insptr;
    output_hash_table_insptr output_hash_table =
        map->output_hash_table;
    MIN_REQUIRE ( output_hash_table != min::NULL_STUB );
    min::uns64 h0 = MUP::value_of ( g );
    h0 *= map->hash_multiplier;
    h0 %= output_hash_table->length;
    return (L) h0;
}

template < typename L >
static void new_output_hash_table
	( min::packed_vec_ptr
	      < min::gen,
		min::id_map_header<L>,
		L > map )
{
    typedef min::packed_vec_insptr
    		< min::gen,
		  min::id_map_header<L>,
		  L > id_map_insptr;
    typedef min::packed_vec_insptr
    		< L, min::packed_vec_header<L>, L >
	output_hash_table_insptr;

    MIN_ASSERT
        ( map->input_hash_table == min::MISSING(),
	  "id map cannot have both an output and an"
	  " input hash table" );

    id_map_insptr map_insptr = (id_map_insptr) map;

    L length = map->occupied;
    MIN_REQUIRE
        (   length
          < ( (L) 1 << ( 8 * sizeof ( L ) - 2 ) ) );
    length *= 4;
    if ( length < 128 ) length = 128;

    output_hash_table_ref ( map ) =
	::new_id_map_output_hash_table<L> ( length );
    output_hash_table_insptr output_hash_table =
        map->output_hash_table;
    min::push ( output_hash_table, length );

    map_insptr->hash_multiplier = 1103515245;
        // Used by gcc for linear congrential random
	// number generator with modulus 2^32.
    map_insptr->hash_max_offset = 0;

    * (L *) & map_insptr->occupied = 0;
    for ( L id = 0; id < map->length; ++ id )
    {
	min::gen g = map[id];
	if ( g == min::NONE() ) continue;
	++ * (L *) & map_insptr->occupied;

	L h = ::hash ( map, g );
	L offset = 0;
	while ( output_hash_table[h] != 0 )
	{
	    h = ( h + 1 ) % output_hash_table->length;
	    ++ offset;
	}
	output_hash_table[h] = id;
	if ( map->hash_max_offset < offset )
	    map_insptr->hash_max_offset = offset;
    }
}

template < typename L >
inline min::packed_vec_ptr
	< min::gen,
	  min::id_map_header<L>, L >
    init
	( min::ref<min::packed_vec_ptr
		       < min::gen,
			 min::id_map_header<L>, L > >
	      map )
{
    typedef min::packed_vec_insptr
    		< min::gen,
		  min::id_map_header<L>, L >
	    id_map_insptr;

    id_map_insptr map_insptr =
        (id_map_insptr) (min::id_map) map;
    if ( map_insptr == min::NULL_STUB )
    {
	map_insptr = ::new_id_map<L> ( 16 );
	map = map_insptr;
    }
    else
    {
        output_hash_table_ref((min::id_map) map) =
	    min::NULL_STUB;
	min::pop ( map_insptr,
	           (min::unsptr) map->length );
	min::resize ( map_insptr, 16 );
    }
    min::push ( map_insptr ) = min::NONE();
    * (L *) & map_insptr->occupied = 0;
    map_insptr->next = 1;
    map_insptr->ID_character = U'@';
    ID_prefix_ref((min::id_map) map) =
	min::new_str_gen ( "!" );
    ID_assign_ref((min::id_map) map) =
	min::new_str_gen ( ":=" );
    map_insptr->id_gen_format =
        min::id_map_gen_format;
    input_hash_table_ref((min::id_map) map) =
	min::MISSING();

    return map;
}

template < typename L >
inline L find
	( min::packed_vec_ptr
	      < min::gen,
		min::id_map_header<L>, L > map,
	  min::gen g )
{
    typedef min::packed_vec_insptr
    		< L, min::packed_vec_header<L>, L >
	output_hash_table_insptr;

    if ( g == min::NONE() ) return 0;
    if ( map == min::NULL_STUB ) return 0;

    if ( map->output_hash_table == min::NULL_STUB )
	::new_output_hash_table ( map );
    output_hash_table_insptr output_hash_table =
        map->output_hash_table;
    L h = ::hash ( map, g );
    L offset = 0;
    while ( true )
    {
        L id = output_hash_table[h];
	if ( id == 0 ) return 0;
	if ( map[id] == g ) return id;
	if ( offset >= map->hash_max_offset ) return 0;
	h = ( h + 1 ) % output_hash_table->length;
	++ offset;
    }
}

template < typename L >
inline L find_or_add
	( min::packed_vec_ptr
	      < min::gen,
		min::id_map_header<L>, L > map,
	  min::gen g )
{
    typedef min::packed_vec_insptr
    		< min::gen,
		  min::id_map_header<L>, L >
	id_map_insptr;
    typedef min::packed_vec_insptr
    		< L, min::packed_vec_header<L>, L >
	output_hash_table_insptr;

    if ( g == min::NONE() ) return 0;
    if ( map == min::NULL_STUB ) return 0;

    if ( map->output_hash_table == min::NULL_STUB )
	::new_output_hash_table ( map );
    output_hash_table_insptr output_hash_table =
        map->output_hash_table;
    L h = ::hash ( map, g );
    L offset = 0;
    while ( true )
    {
        L id = output_hash_table[h];
	if ( id == 0 ) break;
	if ( map[id] == g ) return id;
	h = ( h + 1 ) % output_hash_table->length;
	++ offset;
    }
    L id = map->length;
    id_map_insptr map_insptr =
        (id_map_insptr) (min::id_map) map;
    min::push ( map_insptr ) = g;
    output_hash_table[h] = id;

    if ( map->hash_max_offset < offset )
        map_insptr->hash_max_offset = offset;

    ++ * (L *) & map_insptr->occupied;
    if ( output_hash_table->length < 2 * map->occupied )
        output_hash_table_ref ( map ) = min::NULL_STUB;

    return id;
}

template < typename L >
inline void map_set
	( min::packed_vec_ptr
	      < min::gen,
		min::id_map_header<L> > map,
	  L id,
	  min::gen g )
{
    typedef min::packed_vec_insptr
    		< min::gen,
		  min::id_map_header<L>, L >
        id_map_insptr;

    id_map_insptr map_insptr = (id_map_insptr) map;

    MIN_ASSERT
        ( map->output_hash_table == min::NULL_STUB,
	  "numeric map_set called when map has"
	  " an output hash table" );
    MIN_ASSERT ( id != 0, "id argument is zero" );
    MIN_ASSERT ( g != min::NONE(),
                 "value argument is NONE" );

    while ( id >= map->length )
        min::push(map_insptr) = min::NONE();
    if ( map_insptr[id] == min::NONE() )
	++ * (L *) & map_insptr->occupied;
    map_insptr[id] = g;
}

template < typename L >
inline void map_clear
	( min::packed_vec_ptr
	      < min::gen,
		min::id_map_header<L> > map,
	  L id )
{
    MIN_ASSERT
        ( map->output_hash_table == min::NULL_STUB,
	  "numeric map_clear called when map has"
	  " an output hash table" );

    if ( id >= map->length ) return;

    typedef min::packed_vec_insptr
    		< min::gen,
		  min::id_map_header<L>, L >
        id_map_insptr;

    id_map_insptr map_insptr = (id_map_insptr) map;

    if ( map_insptr[id] != min::NONE() )
	-- * (L *) & map_insptr->occupied;
    map_insptr[id] = min::NONE();
}

template < typename L >
inline void map_set
	( min::packed_vec_ptr
	      < min::gen,
		min::id_map_header<L> > map,
	  min::gen symbol,
	  min::gen g )
{
    if ( map->input_hash_table == min::MISSING() )
    {
	MIN_ASSERT
	    ( map->output_hash_table == min::NULL_STUB,
	      "id map cannot have both an output and an"
	      " input hash table" );
        input_hash_table_ref ( map ) =
	    min::new_obj_gen ( 500, 100, 0, true );
    }
    min::set ( map->input_hash_table, symbol, g );
}

template < typename L >
inline void map_clear
	( min::packed_vec_ptr
	      < min::gen,
		min::id_map_header<L> > map,
	  min::gen symbol )
{
    if ( map->input_hash_table != min::MISSING() )
	min::set ( map->input_hash_table, symbol,
	           min::NONE() );
}

min::id_map min::init
	( min::ref<min::id_map> map )
{
    return ::init ( map );
}

min::uns32 min::find
	( min::id_map map, min::gen g )
{
    return ::find ( map, g );
}

min::uns32 min::find_or_add
	( min::id_map map, min::gen g )
{
    return ::find_or_add ( map, g );
}

void min::map_set
	( min::id_map map, min::uns32 id, min::gen g )
{
    ::map_set ( map, id, g );
}

void min::map_clear
	( min::id_map map, min::uns32 id )
{
    ::map_clear ( map, id );
}

void min::map_set
	( min::id_map map, min::gen symbol, min::gen g )
{
    ::map_set ( map, symbol, g );
}

void min::map_clear
	( min::id_map map, min::gen symbol )
{
    ::map_clear ( map, symbol );
}

min::id_map min::set_id_map
	( min::printer printer,
	  min::id_map map )
{
    if ( map == min::NULL_STUB )
    {
        if ( printer->id_map == min::NULL_STUB )
	    min::init ( id_map_ref(printer) );
    }
    else
	id_map_ref(printer) = map;

    return printer->id_map;
}

// UNICODE Name Tables
// ------- ---- ------

static min::packed_vec<MINT::unicode_name_entry>
    unicode_name_table_type
	( "min::unicode_name_table_type" );

min::unicode_name_table min::init
	( min::ref<min::unicode_name_table>
	      table,
	  const min::uns32 * char_flags,
	  min::uns32 flags,
	  min::uns32 extras )
{
    if ( table == NULL_STUB )
    {
        uns32 length = extras;
	if ( flags != 0 )
	{
	    for ( unsigned i = 0;
	          i < min::unicode::index_limit; ++ i )
	    {
	        if ( ( char_flags[i] & flags ) != 0 )
		    ++ length;
	    }
	    for ( unsigned j = 0;
	          j < min::unicode::extra_names_number;
		  ++ j )
	    {
	        Uchar c =
		    min::unicode::extra_names[j].c;
		min::uns16 i = min::Uindex ( c );
	        if ( ( char_flags[i] & flags ) != 0 )
		    ++ length;
	    }
	}

	length *= 3;
        table = ::unicode_name_table_type
	        .new_stub ( length, length );

	for ( unsigned i = 0;
	      i < min::unicode::index_limit; ++ i )
	{
	    if ( ( char_flags[i] & flags ) == 0 )
		continue;
	    ustring name = min::unicode::name[i];
	    Uchar c = min::unicode::character[i];
	    if ( name == NULL ) continue;
	    MIN_ASSERT ( c != NO_UCHAR,
	                 "Bad UNICODE data base entry"
			 " at index %d: name used by"
			 " more than one character",
			 i );
	    min::add ( table,
	               min::ustring_chars ( name ),
		       c );
	}
	for ( unsigned j = 0;
	      j < min::unicode::extra_names_number;
	      ++ j )
	{
	    const min::unicode::extra_name & e =
		min::unicode::extra_names[j];
	    min::uns16 i = min::Uindex ( e.c );
	    if ( ( char_flags[i] & flags ) == 0 )
	        continue;
	    min::add ( table,
	               min::ustring_chars ( e.name ),
		       e.c );
	}
    }
    return table;
}

void min::add
	( min::unicode_name_table table,
	  const char * name,
	  min::Uchar c, bool replace_allowed )
{
    packed_vec_updptr<internal::unicode_name_entry> t =
        (packed_vec_updptr
	     <internal::unicode_name_entry>)
        table;
    uns32 length = t->length;
    uns32 i = min::strhash ( name ) % length;
    for ( uns32 j = 0; j < length;
          ++ j, ++ i, i %= length )
    {
        internal::unicode_name_entry e = t[i];
	if ( e.name == NULL )
	{
	    e.c = c;
	    e.name = name;
	}
	else if ( ::strcmp ( e.name, name ) == 0 )
	{
	    if ( e.c == c ) return;

	    MIN_ASSERT ( replace_allowed,
			 "name already assigned to a"
			 " different character" );
	    e.c = c;
	}
	else continue;

	t[i] = e;
	return;
    }
    MIN_ABORT ( "unicode name table too small" );
}

min::Uchar min::find
	( min::unicode_name_table table,
	  const char * name )
{
    uns32 length = table->length;
    uns32 i = min::strhash ( name ) % length;
    for ( uns32 j = 0; j < length;
                       ++ j, ++ i, i %= length )
    {
        internal::unicode_name_entry e = table[i];
	if ( e.name == NULL ) return NO_UCHAR;
	if ( ::strcmp ( e.name, name ) == 0 )
	    return e.c;
    }
    MIN_ABORT ( "unicode name table too small" );
}

// Objects
// -------

bool min::use_obj_aux_stubs = false;

inline min::unsptr EXP2 ( unsigned bits )
{
    return (min::unsptr) 1 << bits;
}

#define GSIZE MINT::obj_header_gen_size
#define HSIZE MINT::obj_header_hash_bits
#define TSIZE MINT::obj_header_offset_bits

// Given a total_size, var_size, and hash_size for an
// object that is new or being reallocated, compute
// the object type and header size and update the total
// size by adding the header size.  All sizes are in
// min::gen units.  The input total_size does NOT
// include the header, but the output total_size does
// include the header.
//
// If expand is true, the total size is expanded as much
// as possible using MUP::optimal_body_size.
// 
inline void compute_object_type
	( int         & type,
	  min::unsptr & total_size,
	  min::unsptr & header_size,
	  min::unsptr   var_size,
	  min::unsptr   hash_size,
	  bool          expand = false )
{
    if ( MIN_IS_COMPACT
         &&
	 var_size < EXP2 ( HSIZE ( min::TINY_OBJ ) )
         &&
         hash_size < EXP2 ( HSIZE ( min::TINY_OBJ ) )
	 &&
	    total_size + GSIZE ( min::TINY_OBJ )
         <= EXP2 ( TSIZE ( min::TINY_OBJ ) ) )
    {
	type = min::TINY_OBJ;
	header_size = GSIZE ( min::TINY_OBJ );
    }
    else
    if ( var_size < EXP2 ( HSIZE ( min::SHORT_OBJ ) )
         &&
         hash_size < EXP2 ( HSIZE ( min::SHORT_OBJ ) )
	 &&
	    total_size + GSIZE ( min::SHORT_OBJ )
         <= EXP2 ( TSIZE ( min::SHORT_OBJ ) ) )
    {
	type = min::SHORT_OBJ;
	header_size = GSIZE ( min::SHORT_OBJ );
    }
    else
    if ( var_size < EXP2 ( HSIZE ( min::LONG_OBJ ) )
         &&
         hash_size < EXP2 ( HSIZE ( min::LONG_OBJ ) )
	 &&
	    total_size + GSIZE ( min::LONG_OBJ )
         <= EXP2 ( TSIZE ( min::LONG_OBJ ) ) )
    {
	type = min::LONG_OBJ;
	header_size = GSIZE ( min::LONG_OBJ );
    }
    else
    if ( var_size < EXP2 ( HSIZE ( min::HUGE_OBJ ) )
         &&
         hash_size < EXP2 ( HSIZE ( min::HUGE_OBJ ) )
	 &&
	    total_size + GSIZE ( min::HUGE_OBJ )
         <= EXP2 ( TSIZE ( min::HUGE_OBJ ) ) )
    {
	type = min::HUGE_OBJ;
	header_size = GSIZE ( min::HUGE_OBJ );
    }
    else
        MIN_ABORT ( "humongous object requested" );

    total_size += header_size;

    if ( expand )
    {
	total_size =
	    MUP::optimal_body_size
		( total_size * sizeof ( min::gen ) )
	    / sizeof ( min::gen );
	min::unsptr max_total_size =
	    EXP2 ( TSIZE ( type ) );
	if ( total_size > max_total_size )
	    total_size = max_total_size;
    }
}

// Compute object header at beginning of body, given all
// the necessary data.
//
inline void compute_object_header
	( void *	& body,
	  int           type,
	  min::unsptr   var_size,
	  min::unsptr   hash_size,
	  min::unsptr   total_size,
	  min::unsptr   unused_offset,
	  min::unsptr   aux_offset,
	  min::uns8	total_size_flags = 0,
	  min::uns8     unused_offset_flags = 0,
	  min::uns8     aux_offset_flags = 0 )
{
    const unsigned SHIFT = MINT::OBJ_FLAG_BITS;
    const unsigned MASK = ( 1 << SHIFT ) - 1;

    switch ( type )
    {
    case min::TINY_OBJ:
    {
	MINT::tiny_obj * hp = (MINT::tiny_obj *) body;
	hp->total_size =
	      ( ( total_size - 1 ) << SHIFT )
	    + ( total_size_flags & MASK );
	hp->var_size = var_size;
	hp->hash_size = hash_size;
	hp->unused_offset =
	      ( ( unused_offset - 1 ) << SHIFT )
	    + ( unused_offset_flags & MASK );
	hp->aux_offset =
	      ( ( aux_offset - 1 ) << SHIFT )
	    + ( aux_offset_flags & MASK );
        break; 
    }
    case min::SHORT_OBJ:
    {
	MINT::short_obj * hp = (MINT::short_obj *) body;
	hp->total_size =
	      ( ( total_size - 1 ) << SHIFT )
	    + ( total_size_flags & MASK );
	hp->var_size = var_size;
	hp->hash_size = hash_size;
	hp->unused_offset =
	      ( ( unused_offset - 1 ) << SHIFT )
	    + ( unused_offset_flags & MASK );
	hp->aux_offset =
	      ( ( aux_offset - 1 ) << SHIFT )
	    + ( aux_offset_flags & MASK );
        break; 
    }
    case min::LONG_OBJ:
    {
	MINT::long_obj * hp = (MINT::long_obj *) body;
	hp->total_size =
	      ( ( total_size - 1 ) << SHIFT )
	    + ( total_size_flags & MASK );
	hp->var_size = var_size;
	hp->hash_size = hash_size;
	hp->unused_offset =
	      ( ( unused_offset - 1 ) << SHIFT )
	    + ( unused_offset_flags & MASK );
	hp->aux_offset =
	      ( ( aux_offset - 1 ) << SHIFT )
	    + ( aux_offset_flags & MASK );
        break; 
    }
    case min::HUGE_OBJ:
    {
	MINT::huge_obj * hp = (MINT::huge_obj *) body;
	hp->total_size =
	      ( ( total_size - 1 ) << SHIFT )
	    + ( total_size_flags & MASK );
	hp->var_size = var_size;
	hp->hash_size = hash_size;
	hp->unused_offset =
	      ( ( unused_offset - 1 ) << SHIFT )
	    + ( unused_offset_flags & MASK );
	hp->aux_offset =
	      ( ( aux_offset - 1 ) << SHIFT )
	    + ( aux_offset_flags & MASK );
        break; 
    }
    }
}

min::gen MINT::new_obj_gen
	    ( min::stub * s, 
	      min::unsptr unused_size,
	      min::unsptr hash_size,
	      min::unsptr var_size,
	      bool expand )
{
    unsptr total_size =
        unused_size + hash_size + var_size;
    int type;
    unsptr header_size;
    ::compute_object_type
	( type, total_size, header_size,
	  var_size, hash_size, expand );

    if ( s == min::NULL_STUB )
	s = MUP::new_acc_stub();
    else
    {
	MIN_ASSERT
	    ( MUP::type_of ( s ) == min::PREALLOCATED,
	      "first new_obj_gen argument is not"
	      " PREALLOCATED object" );
	MUP::set_type_of ( s, min::FILLING );
    }

    MUP::new_body ( s, sizeof (min::gen) * total_size );

    ::compute_object_header
	( MUP::ptr_ref_of ( s ),
	  type,
	  var_size,
	  hash_size,
	  total_size,
	  header_size + var_size + hash_size,
	  total_size );

    min::gen * p = (min::gen *) MUP::ptr_of ( s )
                 + header_size;
    min::gen * endp = p + var_size;
    while ( p < endp ) * p ++ = min::UNDEFINED();
    endp += hash_size;
    while ( p < endp ) * p ++ = min::LIST_END();

    MUP::set_type_of ( s, type );
    return min::new_stub_gen ( s );
}

// Object Vector Level
// ------ ------ -----

void min::resize
    ( min::obj_vec_insptr & vp,
      min::unsptr unused_size,
      min::unsptr var_size,
      bool expand )
{
    min::stub * s = vp.s;
    unsptr hash_size = min::hash_size_of ( vp );
    unsptr attr_size = min::attr_size_of ( vp );
    unsptr aux_size = min::aux_size_of ( vp );
    unsptr old_total_size = min::total_size_of ( vp );
    unsptr total_size =
          var_size + hash_size + attr_size
	+ unused_size + aux_size;

    int new_type;
    unsptr header_size;
    unsptr saved_total_size = total_size;
    ::compute_object_type
	( new_type, total_size, header_size,
	  var_size, hash_size, expand );
    unused_size += total_size - header_size
                 - saved_total_size;

    MUP::resize_body r
        ( s, total_size * sizeof ( min::gen ),
	     old_total_size * sizeof ( min::gen ) );
    MUP::retype_resize_body ( r, new_type );

    min::gen * & oldb = MUP::base ( vp );
    min::gen * & newb = * (min::gen **) &
	MUP::new_body_ptr_ref ( r );

    min::unsptr new_var_offset = header_size;
    min::unsptr aux_offset =
        total_size - aux_size;
    min::unsptr unused_offset =
        aux_offset - unused_size;

    compute_object_header
	( MUP::new_body_ptr_ref ( r ),
	  new_type,
	  var_size,
	  hash_size,
	  total_size,
	  unused_offset,
	  aux_offset,
	  vp.total_size_flags,
	  vp.unused_offset_flags,
	  vp.aux_offset_flags );

    // Copy variables vector.
    //
    unsptr from = vp.var_offset;
    unsptr to = new_var_offset;
    unsptr from_end = from + min::var_size_of ( vp );
    unsptr to_end = to + var_size;

    while ( from < from_end && to < to_end )
        newb[to++] = oldb[from++];
    while ( to < to_end )
        newb[to++] = min::UNDEFINED();
    from = from_end;

    // Copy hash table and attribute vector.
    //
    from_end = from + min::hash_size_of ( vp )
                    + min::attr_size_of ( vp );
    while ( from < from_end )
        newb[to++] = oldb[from++];

    // Initialize unused area.
    //
    from += min::unused_size_of ( vp );
    memset ( (void *) & newb[to], 0,
             unused_size * sizeof ( min::gen ) );
    to += unused_size;

    // Copy auxiliary area.
    //
    from_end = from + min::aux_size_of ( vp );
    while ( from < from_end )
        newb[to++] = oldb[from++];

    MIN_ASSERT ( from == vp.total_size,
                 "system programming error" );
    MIN_ASSERT ( to == total_size,
                 "system programming error" );

    // Fix vector pointer.
    //
    vp.var_offset = new_var_offset;
    vp.hash_offset = new_var_offset + var_size;
    vp.attr_offset = vp.hash_offset
                   + hash_size;
    vp.unused_offset = vp.attr_offset
                     + attr_size;
    vp.aux_offset = vp.unused_offset
		  + unused_size;
    vp.total_size = total_size;
}

# if MIN_USE_OBJ_AUX_STUBS

    static min::gen copy_aux_stub
	    ( const min::stub * s );
    inline min::gen copy_aux_stub ( min::gen v )
    {
	if ( ! min::is_stub ( v ) )
	    return v;
	const min::stub * s = MUP::stub_of ( v );
	int type = MUP::type_of ( s );
	if ( type == min::LIST_AUX
	     ||
	     type == min::SUBLIST_AUX )
	    return ::copy_aux_stub ( s );
	else
	    return v;
    }
    static min::gen copy_aux_stub
	    ( const min::stub * s )
    {
	min::stub * new_s = MUP::new_aux_stub();
	min::gen result = min::new_stub_gen ( new_s );
	while ( true )
	{
	    MUP::set_gen_of
		( new_s,
		  ::copy_aux_stub
		      ( MUP::gen_of ( s ) ) );
	    min::uns64 c = MUP::control_of ( s );
	    if ( c & MUP::STUB_ADDRESS )
	    {
		s = MUP::stub_of_control ( c );
		min::stub * next_s =
		    MUP::new_aux_stub();
		c = MUP::renew_acc_control_stub
		    ( c, next_s );
		MUP::set_control_of ( new_s, c );
		new_s = next_s;
	    }
	    else
	    {
	        MUP::set_control_of ( new_s, c );
		break;
	    }
	}
	return result;
    }

# else // ! MIN_USE_OBJ_AUX_STUBS

    inline min::gen copy_aux_stub ( min::gen v )
    {
        return v;
    }

# endif // MIN_USE_OBJ_AUX_STUBS

min::gen MINT::copy
    ( min::stub * s,
      min::obj_vec_ptr & vp,
      min::unsptr var_size,
      min::unsptr unused_size,
      bool expand )
{
    min::locatable_gen protect_source
        ( min::new_stub_gen
	    ( (const min::stub *) vp ) );
	// Must protect pointers IN original object
	// from GC while copying.

    unsptr hash_size = min::hash_size_of ( vp );
    unsptr attr_size = min::attr_size_of ( vp );
    unsptr aux_size = min::aux_size_of ( vp );
    unsptr total_size =
          var_size + hash_size + attr_size
	+ unused_size + aux_size;

    int new_type;
    unsptr header_size;
    unsptr saved_total_size = total_size;
    ::compute_object_type
	( new_type, total_size, header_size,
	  var_size, hash_size, expand );
    unused_size += total_size - header_size
                 - saved_total_size;

    if ( s == min::NULL_STUB )
	s = MUP::new_acc_stub();
    else
    {
	MIN_ASSERT
	    ( MUP::type_of ( s ) == min::PREALLOCATED,
	      "first object copy argument is not a"
	      " PREALLOCATED object" );
	MUP::set_type_of ( s, min::FILLING );
    }

    MUP::new_body ( s, sizeof (min::gen) * total_size );

    const min::gen * & oldb = MUP::base ( vp );
    min::gen * & newb =
        * (min::gen **) & MUP::ptr_ref_of ( s );

    min::unsptr new_var_offset = header_size;
    min::unsptr aux_offset =
        total_size - aux_size;
    min::unsptr unused_offset =
        aux_offset - unused_size;

    compute_object_header
	( MUP::ptr_ref_of ( s ),
	  new_type,
	  var_size,
	  hash_size,
	  total_size,
	  unused_offset,
	  aux_offset );

    // Copy variables vector.
    //
    unsptr from = vp.var_offset;
    unsptr to = new_var_offset;
    unsptr from_end = from + min::var_size_of ( vp );
    unsptr to_end = to + var_size;

    while ( from < from_end && to < to_end )
        newb[to++] = ::copy_aux_stub ( oldb[from++] );
    while ( to < to_end )
        newb[to++] = min::UNDEFINED();
    from = from_end;

    // Copy hash table and attribute vector.
    //
    from_end = from + min::hash_size_of ( vp )
                    + min::attr_size_of ( vp );
    while ( from < from_end )
        newb[to++] = ::copy_aux_stub ( oldb[from++] );

    // Initialize unused area.
    //
    from += min::unused_size_of ( vp );
    memset ( (void *) & newb[to], 0,
             unused_size * sizeof ( min::gen ) );
    to += unused_size;

    // Copy auxiliary area.
    //
    from_end = from + min::aux_size_of ( vp );
    while ( from < from_end )
        newb[to++] = ::copy_aux_stub ( oldb[from++] );

    MIN_ASSERT ( from == vp.total_size,
                 "system programming error" );
    MIN_ASSERT ( to == total_size,
                 "system programming error" );

    MUP::set_type_of ( s, new_type );

    min::obj_vec_ptr svp ( s );
    MINT::acc_write_update ( svp );

    return min::new_stub_gen ( s );
}

# if MIN_USE_OBJ_AUX_STUBS

    static void acc_write_update_aux_stub
            ( const min::stub * s1,
	      const min::stub * s2,
	      bool allow_interrupts );
    inline void acc_write_update_obj_element
            ( const min::stub * s1, min::gen v,
	      bool allow_interrupts )
    {
        if ( ! min::is_stub ( v ) ) return;
	const min::stub * s2 = MUP::stub_of ( v );
	int type = MUP::type_of ( s2 );
	if ( type == min::LIST_AUX
	     ||
	     type == min::SUBLIST_AUX )
	    ::acc_write_update_aux_stub
	        ( s1, s2, allow_interrupts );
	else
	{
	    MUP::acc_write_update ( s1, s2 );
	    if ( allow_interrupts ) min::interrupt();
	}
    }

    static void acc_write_update_aux_stub
	    ( const min::stub * s1,
	      const min::stub * s2,
	      bool allow_interrupts)
    {
	while ( true )
	{
	    ::acc_write_update_obj_element
		( s1, MUP::gen_of ( s2 ),
		  allow_interrupts );
	    min::uns64 c2 = MUP::control_of ( s2 );
	    if ( c2 & MUP::STUB_ADDRESS )
		s2 = MUP::stub_of_control ( c2 );
	    else
		break;
	}
    }

# else // ! MIN_USE_OBJ_AUX_STUBS

    inline void acc_write_update_obj_element
            ( const min::stub * s1, min::gen v,
	      bool allow_interrupts )
    {
        if ( ! min::is_stub ( v ) ) return;
	const min::stub * s2 = MUP::stub_of ( v );
        MUP::acc_write_update ( s1, s2 );
	if ( allow_interrupts ) min::interrupt();
    }

# endif // MIN_USE_OBJ_AUX_STUBS

void MINT::acc_write_update
    ( min::obj_vec_ptr & vp, bool allow_interrupts )
{
    unsptr var_offset = MUP::var_offset_of ( vp );
    unsptr unused_offset = MUP::unused_offset_of ( vp );
    unsptr aux_offset = MUP::aux_offset_of ( vp );
    unsptr total_size = min::total_size_of ( vp );

    const min::stub * s1 = MUP::stub_of ( vp );
    const min::gen * & base = MUP::base ( vp );

    for ( unsptr i = var_offset; i < unused_offset; )
        ::acc_write_update_obj_element
	    ( s1, base[i++], allow_interrupts );
    for ( unsptr i = aux_offset; i < total_size; )
        ::acc_write_update_obj_element
	    ( s1, base[i++], allow_interrupts );
}

// Object List Level
// ------ ---- -----

# if MIN_USE_OBJ_AUX_STUBS

// Allocate a chain of aux stubs containing the n
// min::gen values in p.  The type of the first stub is
// given and the other stubs have type min::LIST_AUX.
// Each stub but the last points at the next stub.  The
// control of the last, except for its type field,
// equals the end value, which may be a list aux value
// or a pointer to a stub.
//
// This function returns pointers to the first and last
// stubs allocated.  n > 0 is required.
//
void MINT::allocate_stub_list
	( min::stub * & first,
	  min::stub * & last,
	  int type,
	  const min::gen * p, min::unsptr n,
	  min::uns64 end )
{
    MIN_ASSERT ( n > 0,
                 "vector is zero length" );

    first = MUP::new_aux_stub ();
    MUP::set_gen_of ( first, * p ++ );
    min::stub * previous = first;
    last = first;
    while ( -- n )
    {
	last = MUP::new_aux_stub ();
	MUP::set_gen_of ( last, * p ++ );
	MUP::set_control_of
	     ( previous,
	       MUP::new_control_with_type
	           ( type, last, MUP::STUB_ADDRESS ) );
	type = min::LIST_AUX;
	previous = last;
    }
    MUP::set_control_of
        ( last, MUP::renew_control_type ( end, type ) );
}

# endif // MIN_USE_OBJ_AUX_STUBS

// Remove a list.  Free any aux stubs used and
// set any auxiliary area elements used to min::NONE().
// Index/s point at the first element of the list.
// Either index != 0 and s == NULL or index == 0
// and s != NULL.  Base is the base of an object
// vector pointer and total_size is its total size.
//
void MINT::remove_list
	( min::gen * & base,
	  min::unsptr total_size,
	  min::unsptr index
#	if MIN_USE_OBJ_AUX_STUBS
	  , min::stub * s // = NULL
#	endif
	)
{
    while ( true )
    {
#	if MIN_USE_OBJ_AUX_STUBS
	    if ( s != NULL )
	    {
		MINT::remove_sublist
		    ( base, total_size,
		      MUP::gen_of ( s ) );
		uns64 c = MUP::control_of ( s );
		MUP::free_aux_stub ( s );
		if ( c & MUP::STUB_ADDRESS )
		    s = MUP::stub_of_control ( c );
		else
		{
		    unsptr vc =
			MUP::value_of_control ( c );
		    if ( vc == 0 ) return;
		    index = total_size - vc;
		    s = NULL;
		}
	    }
	    else
#	endif
	{
	    MIN_ASSERT ( index != 0,
	                 "system programming error" );
	    MINT::remove_sublist
	        ( base, total_size, base[index] );
	    base[index] = min::NONE();
	    min::gen v = base[--index];
#	    if MIN_USE_OBJ_AUX_STUBS
		if ( min::is_stub ( v ) )
		{
		    min::stub * s2 = MUP::stub_of ( v );
		    if (    MUP::type_of ( s2 )
			 == LIST_AUX )
		    {
		        base[index] = min::NONE();
			index = 0;
		        s = s2;
		    }
		}
		else
#	    endif
	    if ( min::is_list_aux ( v ) )
	    {
		base[index] = min::NONE();
		index = min::list_aux_of ( v  );
		if ( index == 0 ) return;
		index = total_size - index;
	    }
	}
    }
}

void min::insert_before
	( min::list_insptr & lp,
	  const min::gen * p, min::unsptr n )
{
    MIN_ASSERT ( lp.reserved_insertions >= 1,
                 "no insertions reserved for list"
		 " pointer" );
    MIN_ASSERT ( lp.reserved_elements >= n,
                 "too few elements reserved for list"
		 " pointer" );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    if ( n == 0 ) return;
    else if ( n == 1
              &&
	      lp.current == min::LIST_END()
	      &&
	      lp.current_index == lp.head_index
	      &&
	      lp.current_index != 0 )
    {
	// Special case: empty list with LIST_END()
	// stored in the list head and only 1 element to
	// insert.
	//
	MIN_ASSERT ( min::is_list_legal ( * p ),
	             "value cannot legally be stored in"
		     " a list element" );
	MUP::acc_write_update
		( MUP::stub_of ( lp.vecp ), * p );
	lp.current = lp.base[lp.current_index] = * p;
	return;
    }

    unsptr unused_offset =
        MUP::unused_offset_of ( lp.vecp );
    unsptr aux_offset = lp.aux_offset;
    unsptr total_size = lp.total_size;
    MIN_ASSERT (    total_size
                 == min::total_size_of ( lp.vecp ),
		 "list pointer not up to date"
		 " (refreshed)" );

    const min::gen * pend = p + n;
    for ( const min::gen * q = p; q < pend; )
    {
	MIN_ASSERT ( min::is_list_legal ( * q ),
	             "value cannot legally be stored in"
		     " a list element" );
	MUP::acc_write_update
		( MUP::stub_of ( lp.vecp ), * q ++ );
    }

    if ( lp.current == min::LIST_END() )
    {

	// Contiguous means the previous pointer does
	// not exist and current_index == aux_
	// offset so we can add elements by copying them
	// into the aux area at and just before current_
	// index.
	//
	bool contiguous = false;

	// Previous_is_list_head means previous is in
	// the hash table or attribute vector.  This
	// can only happen if previous_index != 0 and
	// previous_is_sublist_head is false.
	//
	bool previous_is_list_head = false;

	// Pointer to the first new element; may replace
	// LIST_END() in current or previous pointer.
	//
	min::gen fgen;

	if ( lp.previous_index != 0 )
	    previous_is_list_head =
	        ! lp.previous_is_sublist_head;
		// If previous_index != 0 and current ==
		// LIST_END() then only two cases are
		// possible:
		//    1) previous is a list head
		//    2) previous is a sublist head

	else
#	if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.previous_stub == NULL )
#       endif
		contiguous =
		    ( lp.current_index == aux_offset );

#	if MIN_USE_OBJ_AUX_STUBS
	    if (    lp.use_obj_aux_stubs
		 &&     unused_offset
		      + n + ( ! contiguous )
		      + previous_is_list_head
		    > aux_offset )
	    {
	        // Not enough aux area available for
		// all the new elements, and aux stubs
		// are allowed.
		//
		min::stub * first, * last;
		MINT::allocate_stub_list
		    ( first, last,
		      lp.previous_is_sublist_head ?
			  min::SUBLIST_AUX :
		          min::LIST_AUX,
		      p, n,
		      MUP::new_control_with_type
			( 0, (uns64) 0 ) );
		MINT::set_aux_flag_of ( lp.vecp );

		fgen = min::new_stub_gen ( first );
		if ( lp.previous_stub != NULL )
		{
		    if ( lp.previous_is_sublist_head )
		       MUP::set_gen_of
		           ( lp.previous_stub, fgen );
		    else
		    {
			int type =
			    min::type_of
				( lp.previous_stub );
			MUP::set_control_of
			    ( lp.previous_stub,
			      MUP::new_control_with_type
				  ( type, first,
				    MUP::STUB_ADDRESS )
			    );
		    }
		}
		else if ( lp.previous_index != 0 )
		{
		    // previous is list head or sublist
		    // head, as noted above.

		    if ( previous_is_list_head )
		    {
		        min::stub * s =
			    MUP::new_aux_stub();
			MINT::set_aux_flag_of
			    ( lp.vecp );
			MUP::set_gen_of
			    ( s,
			      lp.base
				[lp.previous_index] );
			MUP::set_control_of
			    ( s,
			      MUP::new_control_with_type
			        ( min::LIST_AUX,
				  first,
				  MUP::STUB_ADDRESS ) );
			lp.base[lp.previous_index] =
			    min::new_stub_gen ( s );
			lp.previous_index = 0;
			lp.previous_stub = s;
		    }
		    else
			lp.base[lp.previous_index] =
			    fgen;
		}
		else
		{
		    lp.base[lp.current_index] = fgen;
		    lp.previous_index =
		        lp.current_index; 
		    lp.previous_is_sublist_head = false;
		}
		lp.current_stub = first;
		lp.current = MUP::gen_of ( first );
		lp.current_index = 0;
		return;
	    }
#	endif

	// Insertion will use aux area.

	MIN_ASSERT (      unused_offset
			+ n + ( ! contiguous )
			+ previous_is_list_head
		     <= aux_offset,
		     "too little space in object;"
		     " space reserved for this list"
		     " pointer must have been consumed"
		     " using a different list"
		     " pointer" );

#	if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.previous_stub != NULL )
	    {
	        if ( lp.previous_is_sublist_head )
		{
		    fgen = min::new_list_aux_gen
			       (   total_size
			         - aux_offset + 1 );
		    MUP::set_gen_of
			( lp.previous_stub, fgen );
		}
		else
		{
		    int type =
		        min::type_of
			    ( lp.previous_stub );
		    MUP::set_control_of
		        ( lp.previous_stub,
			  MUP::new_control_with_type
			      ( type,
			          total_size
				- aux_offset + 1 ) );
		}
	    }
	    else
#	endif
	if ( lp.previous_index != 0 )
	{
	    if ( previous_is_list_head )
	    {
	        lp.base[-- aux_offset] =
		    lp.base[lp.previous_index];
		lp.base[lp.previous_index] =
		    min::new_list_aux_gen
			( total_size - aux_offset );
		lp.previous_index = 0;
	    }
	    else
	    {
		lp.base[lp.previous_index] =
		    min::new_sublist_aux_gen
			( total_size - aux_offset + 1 );
	    }
	}
	else if ( contiguous )
	    ++ aux_offset;
	else
	{
	    lp.base[lp.current_index] =
		min::new_list_aux_gen
		   ( total_size - aux_offset + 1 );
	    lp.previous_index = lp.current_index;
	    lp.previous_is_sublist_head = false;
	}

	lp.current_index = aux_offset - 1;
	lp.current = p[0];
	while ( n -- )
	    lp.base[-- aux_offset] = * p ++;
	lp.base[-- aux_offset] = min::LIST_END();

	MUP::aux_offset_of ( lp.vecp ) =
	    lp.aux_offset = aux_offset;
	return;
    }

    // lp.current != min::LIST_END()

    // If there is no previous, we must move the current
    // element so we can replace it with a list pointer.
    // If the current element is in an aux stub, there
    // has to be a previous.
    //
    bool previous = ( lp.previous_index != 0 );

#   if MIN_USE_OBJ_AUX_STUBS
        if ( lp.previous_stub != NULL )
	    previous = true;
	if (    lp.use_obj_aux_stubs
	     &&     unused_offset
		  + n + 1 + ( ! previous )
		> aux_offset )
	{
	    // Not enough aux area available for all the
	    // new elements, and aux stubs are allowed.
	    // Prepare to call allocate_stub_list.

	    min::stub * s;
	        // Stub to which current value is moved
		// if previous pointer does not exist.

	    uns64 end;
	    int type = min::LIST_AUX;
	        // Parameters for call to allocate_stub_
		// list below.

	    if ( lp.current_stub != NULL )
	    {
		type = min::type_of ( lp.current_stub );
		MUP::set_type_of
		    ( lp.current_stub, min::LIST_AUX );
		end = MUP::new_control_with_type
		   ( 0, lp.current_stub,
		     MUP::STUB_ADDRESS );
		MIN_ASSERT ( previous,
		             "system programming"
			     " error" );
	    }
	    else if ( ! previous )
	    {
	        s = MUP::new_aux_stub();
		MINT::set_aux_flag_of ( lp.vecp );
		MUP::set_gen_of ( s, lp.current );
		end = MUP::new_control_with_type
		    ( 0, s, MUP::STUB_ADDRESS );
		unsptr next = lp.current_index;
		if ( next == lp.head_index )
		    next = 0;
		else if (    lp.base[-- next]
		          == min::LIST_END() )
		{
		    // Next element (the one immediately
		    // before the current element in the
		    // aux area) is LIST_END() will not
		    // be needed any more, hence we free
		    // it.
		    //
		    lp.base[next] = min::NONE();

		    next = 0;
		}
		else
		{
		    MIN_REQUIRE
		        ( next >= lp.aux_offset );
		    next = total_size - next;
		}
		MUP::set_control_of
		    ( s,
		      MUP::new_control_with_type
		        ( min::LIST_AUX, next ) );
	    }
	    else
	    {
	        if ( lp.previous_is_sublist_head )
		    type = min::SUBLIST_AUX;
		end = MUP::new_control_with_type
		   ( 0, total_size - lp.current_index );
	    }

	    min::stub * first, * last;
	    MINT::allocate_stub_list
		( first, last, type, p, n, end );
	    MINT::set_aux_flag_of ( lp.vecp );

	    if ( lp.previous_index != 0 )
		lp.base[lp.previous_index] =
		    min::new_stub_gen ( first );
	    else if ( lp.previous_stub != NULL )
	    {
		if ( lp.previous_is_sublist_head )
		    MUP::set_gen_of
			( lp.previous_stub,
			  min::new_stub_gen ( first ) );
		else
		{
		   type = min::type_of
			      ( lp.previous_stub );
		    MUP::set_control_of
			( lp.previous_stub,
			  MUP::new_control_with_type
			      ( type, first,
				MUP::STUB_ADDRESS ) );
		}
	    }
	    else
	    {
	        MIN_ASSERT ( lp.current_index != 0,
		             "system programming"
			     " error" );

		lp.base[lp.current_index] =
		    min::new_stub_gen ( first );
		lp.previous_index = lp.current_index;
		lp.previous_is_sublist_head = false;
	    }
	    lp.current_stub = first;
	    lp.current_index = 0;
	    lp.current = MUP::gen_of ( first );
	    return;
	}
#   endif

    // Insertion will use aux area.

    MIN_ASSERT (      unused_offset
		    + n + 1 + ( ! previous )
		 <= aux_offset,
		 "too little space in object;"
		 " space reserved for this list"
		 " pointer must have been consumed"
		 " using a different list"
		 " pointer" );

    unsptr first = aux_offset - 1;
    unsptr aux_first = total_size - first;

    while ( n -- )
	lp.base[-- aux_offset] = * p ++;

#   if MIN_USE_OBJ_AUX_STUBS
	if ( lp.current_stub != NULL )
	{
	    MIN_ASSERT ( previous,
	                 "system programming error" );
	    lp.base[-- aux_offset] =
		min::new_stub_gen ( lp.current_stub );
	    MUP::set_type_of
		( lp.current_stub, min::LIST_AUX );
	}
	else
#   endif
    {
	unsptr next = lp.current_index;
        if ( ! previous )
	{
	    lp.base[-- aux_offset] = lp.current;
	    if ( next == lp.head_index )
	        next = 0;
	    else if (    lp.base[-- next]
	              == min::LIST_END() )
	    {
		// Next element (the one immediately
		// before the current element in the aux
		// area) is LIST_END() will not be
		// needed any more, hence we free it.
		//
		lp.base[next] = min::NONE();

		next = 0;
	    }
	}
	if ( next != 0 )
	{
	    MIN_REQUIRE ( next >= lp.aux_offset );
	    next = total_size - next;
	}
        lp.base[-- aux_offset] =
	    min::new_list_aux_gen ( next );
    }

#   if MIN_USE_OBJ_AUX_STUBS
	if ( lp.previous_stub != NULL )
	{
	    if ( lp.previous_is_sublist_head )
	    {
	        MUP::set_gen_of
		    ( lp.previous_stub,
		      min::new_sublist_aux_gen
			  ( aux_first ) );
	    }
	    else
	    {
	        int type =
		    min::type_of ( lp.previous_stub );
		MUP::set_control_of
		    ( lp.previous_stub,
		      MUP::new_control_with_type
			  ( type, aux_first ) );
	    }
	}
	else
#   endif
    if ( lp.previous_index != 0 )
    {
	lp.base[lp.previous_index] =
	    lp.previous_is_sublist_head ?
	    min::new_sublist_aux_gen ( aux_first ) :
	    min::new_list_aux_gen ( aux_first );
    }
    else
    {
	MIN_ASSERT ( lp.current_index != 0,
	             "system programming error" );
	lp.base[lp.current_index] =
	    min::new_list_aux_gen ( aux_first );
	lp.previous_index = lp.current_index;
	lp.previous_is_sublist_head = false;
    }

    lp.current_index = first;
    lp.current = lp.base[first];
#   if MIN_USE_OBJ_AUX_STUBS
	lp.current_stub = NULL;
#   endif

    MUP::aux_offset_of ( lp.vecp ) =
	lp.aux_offset = aux_offset;
}

void min::insert_after
	( min::list_insptr & lp,
	  const min::gen * p, min::unsptr n )
{
    if ( n == 0 ) return;

    unsptr unused_offset =
        MUP::unused_offset_of ( lp.vecp );
    unsptr aux_offset = lp.aux_offset;
    unsptr total_size = lp.total_size;
    MIN_ASSERT (    total_size
                 == min::total_size_of ( lp.vecp ),
		 "list pointer is not up to date"
		 " (refreshed)" );

    MIN_ASSERT ( lp.reserved_insertions >= 1,
                 "no insertions reserved for list"
		 " pointer" );
    MIN_ASSERT ( lp.reserved_elements >= n,
                 "too few elements reserved for list"
		 " pointer" );
    MIN_ASSERT ( lp.current != min::LIST_END(),
                 "list pointer is at end of list" );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    const min::gen * pend = p + n;
    for ( const min::gen * q = p; q < pend; )
    {
	MIN_ASSERT ( min::is_list_legal ( * q ),
	             "value cannot legally be stored in"
		     " a list element" );
	MUP::acc_write_update
		( MUP::stub_of ( lp.vecp ), * q ++ );
    }

    bool previous = ( lp.previous_index != 0 );
#   if MIN_USE_OBJ_AUX_STUBS
	if ( lp.previous_stub != NULL )
	    previous = true;

	if (    lp.use_obj_aux_stubs
	     &&     unused_offset
		  + ( n + 1 + ! previous )
		> aux_offset )
	{
	    // Not enough aux area available for
	    // all the new elements, and aux stubs
	    // are allowed.
	    //
	    min::stub * first, * last;

	    if ( lp.current_stub != NULL )
	    {
		MINT::allocate_stub_list
		    ( first, last, min::LIST_AUX,
		      p, n,
		      MUP::control_of
		          ( lp.current_stub ) );
		MINT::set_aux_flag_of ( lp.vecp );

		MUP::set_control_of
		    ( lp.current_stub,
		      MUP::new_control_with_type
			 ( min::type_of
			       ( lp.current_stub ),
			   first,
			   MUP::STUB_ADDRESS ) );
		return;
	    }

	    min::stub * s = MUP::new_aux_stub();
	    MINT::set_aux_flag_of ( lp.vecp );
	    MUP::set_gen_of ( s, lp.current );
	    int type = lp.previous_is_sublist_head ?
	    	       min::SUBLIST_AUX :
		       min::LIST_AUX;

	    // If previous, we can copy the last new
	    // element to the old current element.

	    unsptr next =
	        lp.current_index == lp.head_index ?
	        0 :
	          total_size
	        - lp.current_index + ! previous;
	    uns64 end =
		MUP::new_control_with_type
		    ( type, next );

	    if ( n > previous )
	    {
		MINT::allocate_stub_list
		    ( first, last, min::LIST_AUX,
		      p, n - previous, end );
		MINT::set_aux_flag_of ( lp.vecp );
	    }

	    if ( previous )
	    {
		if ( n > 1 )
		    end = MUP::new_control_with_type
		              ( type, first,
			        MUP::STUB_ADDRESS );
		MUP::set_control_of ( s, end );
		lp.base[lp.current_index] = p[n-1];
		lp.current_index = 0;
		lp.current_stub = s;

		if ( lp.previous_stub != NULL )
		{
		    if ( lp.previous_is_sublist_head )
			MUP::set_gen_of
			    ( lp.previous_stub,
			      min::new_stub_gen ( s ) );
		    else
		    {
			int type =
			    min::type_of
				( lp.previous_stub );
			MUP::set_control_of
			  ( lp.previous_stub,
			    MUP::new_control_with_type
			      ( type, s,
			        MUP::STUB_ADDRESS ) );
		    }
		}
		else
		    lp.base[lp.previous_index] =
			MUP::new_stub_gen ( s );
	    }
	    else
	    {
	        // No previous.  Current aux element
		// has been moved to stub s and current
		// aux element is used to point at s.
		//
		MIN_ASSERT ( lp.current_index != 0,
		             "system programming"
			     " error" );

		MUP::set_control_of
		    ( s,
		      MUP::new_control_with_type
		          ( min::LIST_AUX, first,
			    MUP::STUB_ADDRESS ) );
		lp.base[lp.current_index] =
		    min::new_stub_gen ( s );
		lp.previous_index = lp.current_index;
		lp.current_index = 0;
		lp.current_stub = s;
	    }
	    return;
	}
#   endif

    // Insertion will use aux area.

    MIN_ASSERT (      unused_offset
		    + ( n + 1 + ! previous )
		 <= aux_offset,
		 "too little space in object;"
		 " space reserved for this list"
		 " pointer must have been consumed"
		 " using a different list"
		 " pointer" );

    unsptr first = aux_offset - 1;

#   if MIN_USE_OBJ_AUX_STUBS
    if ( lp.current_stub != NULL )
    {
	MIN_ASSERT ( previous,
	             "system programming error" );
	while ( n -- )
	    lp.base[-- aux_offset] = * p ++;
	uns64 c =
	    MUP::control_of ( lp.current_stub );
	if ( c & MUP::STUB_ADDRESS )
	    lp.base[-- aux_offset] =
		min::new_stub_gen
		    ( MUP::stub_of_control ( c ) );
	else
	    lp.base[-- aux_offset] =
	        min::new_list_aux_gen
		    ( MUP::value_of_control ( c ) );
	MUP::set_control_of
	    ( lp.current_stub,
	      MUP::new_control_with_type
	         ( min::type_of ( lp.current_stub ),
		   total_size - first ) );

	MUP::aux_offset_of ( lp.vecp ) =
	    lp.aux_offset = aux_offset;

	return;
    }
#   endif

    // Current element that we are inserting after
    // is moved to `first'.
    //
    lp.base[-- aux_offset] = lp.current;

    // If previous, we can copy the last new element to
    // the old current element.

    // Copy all the new elements BUT the last new
    // element.
    //
    while ( -- n )
	lp.base[-- aux_offset] = * p ++;

    if ( previous )
    {
	lp.base[-- aux_offset] =
	    min::new_list_aux_gen
	        ( total_size - lp.current_index );
	lp.base[lp.current_index] = * p ++;
	lp.current_index = first;

#	if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.previous_stub != NULL )
	    {
		if ( lp.previous_is_sublist_head )
		{
		    MUP::set_gen_of
			( lp.previous_stub,
			  min::new_sublist_aux_gen
			      ( total_size - first ) );
		}
		else
		{
		    MUP::set_control_of
			( lp.previous_stub,
			  MUP::new_control_with_type
			      ( min::type_of
				  ( lp.previous_stub ),
				total_size - first ) );
		}
	    }
	    else
#	endif
	{
	    lp.base[lp.previous_index] =
		MUP::renew_gen
		    ( lp.base[lp.previous_index],
		      total_size - first );
	}
    }
    else
    {
	// With no previous we must use current aux
	// element as pointer to copy of current
	// element.
	//
	MIN_ASSERT ( lp.current_index != 0,
	             "system programming error" );

	lp.base[-- aux_offset] = * p ++;
	unsptr next = lp.current_index;
	if ( next == lp.head_index )
	    next = 0;
	else if ( lp.base[-- next] == min::LIST_END() )
	{
	    // Next element (the one immediately
	    // before the current element in the aux
	    // area) is LIST_END() will not be needed
	    // any more, hence we free it.
	    //
	    lp.base[next] = min::NONE();

	    next = 0;
	}
	else
	    next = total_size - next;
	lp.base[-- aux_offset] =
	    min::new_list_aux_gen ( next );

	lp.base[lp.current_index] =
	    min::new_list_aux_gen
	        ( total_size - first );
	lp.previous_index = lp.current_index;
	lp.current_index = first;
    }

    MUP::aux_offset_of ( lp.vecp ) =
        lp.aux_offset = aux_offset;
}

min::unsptr min::remove
	( min::list_insptr & lp,
	  min::unsptr n )
{
    if ( n == 0 || lp.current == min::LIST_END() )
        return 0;

    unsptr total_size = lp.total_size;
    MIN_ASSERT (    total_size
                 == min::total_size_of ( lp.vecp ),
		 "list pointer is not up to date"
		 " (refreshed)" );

    if ( lp.current_index != 0
         &&
	 lp.current_index == lp.head_index )
    {
	// Special case: deleting list head of a list
	// with just 1 element.
	//
	lp.current = lp.base[lp.current_index]
	           = min::LIST_END();
	return 1;
    }

    // Save the current previous pointer and current
    // index.
    //
    unsptr previous_index = lp.previous_index;
    bool previous_is_sublist_head =
	lp.previous_is_sublist_head;
    unsptr current_index = lp.current_index;
#   if MIN_USE_OBJ_AUX_STUBS
	min::stub * previous_stub = lp.previous_stub;
#   endif

    // Count of elements removed; to be returned as
    // result.
    //
    unsptr count = 0;

    // Skip n elements (or until end of list).
    // Remove sublists and free aux stubs.
    // Set aux area elements to NONE().
    //
    while ( n -- )
    {
	if ( lp.current == min::LIST_END() ) break;
	++ count;
	MINT::remove_sublist
	    ( lp.base, total_size, lp.current );

#       if MIN_USE_OBJ_AUX_STUBS
	    if ( lp.current_stub != NULL )
	    {
		min::stub * last_stub = lp.current_stub;
		next ( lp );
		MUP::free_aux_stub ( last_stub );
	    }
	    else
#       endif
	{
	    MIN_ASSERT ( lp.current_index != 0,
	                 "system programming error" );
	    unsptr index = lp.current_index;
	    next ( lp );
	    lp.base[index] = min::NONE();
	}
    }

    // Now lp.current_index/lp.current_stub are the new
    // current element and we must either set the old
    // previous element to point to this, or if there
    // was no old previous element, we must make the
    // old current element into a previous element.

#   if MIN_USE_OBJ_AUX_STUBS

	if ( previous_stub != NULL )
	{
	    if ( lp.current_stub != NULL )
	    {
		if ( previous_is_sublist_head )
		{
		    MUP::set_type_of
			( lp.current_stub,
			  min::SUBLIST_AUX );
		    MUP::set_gen_of
		        ( previous_stub,
			  min::new_stub_gen
			      ( lp.current_stub ) );
		}
		else
		{
		    int type =
		        min::type_of ( previous_stub );
		    MUP::set_control_of
		        ( previous_stub,
			  MUP::new_control_with_type
			      ( type, lp.current_stub,
			        MUP::STUB_ADDRESS ) );
		}
	    }
	    else if ( lp.current == min::LIST_END() )
	    {
	        if ( previous_is_sublist_head )
		    MUP::set_gen_of
		        ( previous_stub,
			  min::EMPTY_SUBLIST() );
		else
		{
		    int type =
		        min::type_of ( previous_stub );
		    MUP::set_control_of
		        ( previous_stub,
			  MUP::new_control_with_type
			      ( type, uns64(0) ) );
		}
		if ( lp.current_index != 0 )
		{
		    lp.base[lp.current_index] =
		        min::NONE();
		    lp.current_index = 0;
		}
	    }
	    else
	    {
	        MIN_ASSERT ( lp.current_index != 0,
		             "system programming"
			     " error" );

	        if ( previous_is_sublist_head )
		    MUP::set_gen_of
		        ( previous_stub,
			  min::new_sublist_aux_gen
			      (   total_size
			        - lp.current_index ) );
		else
		{
		    int type =
		        min::type_of ( previous_stub );
		    MUP::set_control_of
		        ( previous_stub,
			  MUP::new_control_with_type
			      ( type,
			          total_size
				- lp.current_index ) );
		}
	    }

	    lp.previous_stub = previous_stub;
	    lp.previous_is_sublist_head =
		previous_is_sublist_head;
	    lp.previous_index = 0;
	}
	else
#   endif
    if ( previous_index != 0 )
    {
#	if MIN_USE_OBJ_AUX_STUBS
	    lp.previous_stub = NULL;

	    if ( lp.current_stub != NULL )
	    {
		if ( previous_is_sublist_head )
		    MUP::set_type_of
			( lp.current_stub,
			  min::SUBLIST_AUX );
		lp.base[previous_index] =
		    min::new_stub_gen
		        ( lp.current_stub );

		lp.previous_index = previous_index;
		lp.previous_is_sublist_head =
		    previous_is_sublist_head;
	    }
	    else
#	endif
	if ( lp.current == min::LIST_END() )
	{
	    if ( lp.current_index != 0 )
		lp.base[lp.current_index] = min::NONE();

	    if ( previous_is_sublist_head )
	    {
		lp.base[previous_index] =
		    min::EMPTY_SUBLIST();
		lp.current_index = 0;
		lp.previous_index = previous_index;
		lp.previous_is_sublist_head = true;
	    }
	    else
	    {
		lp.base[previous_index] =
		    min::LIST_END();
		lp.current_index = previous_index;
		lp.previous_index = 0;
		lp.previous_is_sublist_head = false;
	    }
	}
	else
	{
	    MIN_ASSERT ( lp.current_index != 0,
	                 "system programming error" );

	    if ( previous_is_sublist_head )
		lp.base[previous_index] =
		    min::new_sublist_aux_gen
			(   total_size
			  - lp.current_index );
	    else
		lp.base[previous_index] =
		    min::new_list_aux_gen
			(   total_size
			  - lp.current_index );

	    lp.previous_index = previous_index;
	    lp.previous_is_sublist_head =
		previous_is_sublist_head;
	}
    }
    else
    {
    	// No previous.  Then the first element
	// removed cannot be a stub, as stubs always
	// have a previous pointer.  Also, current
	// element cannot be the first element of
	// a sublist or a list head.

	MIN_ASSERT ( current_index != 0,
	             "system programming error" );

#       if MIN_USE_OBJ_AUX_STUBS
	    lp.previous_stub = NULL;
	    if ( lp.current_stub != NULL )
	    {
		lp.base[current_index] =
		    min::new_stub_gen
		        ( lp.current_stub );
		lp.previous_index = current_index;
	    }
	    else
#       endif
	if ( lp.current == min::LIST_END() )
	{
	    if ( lp.current_index != 0 )
		lp.base[lp.current_index] = min::NONE();
	    lp.base[current_index] = min::LIST_END();
	    lp.current_index = current_index;
	    lp.previous_index = 0;
	}
	else
	{
	    MIN_ASSERT
	        ( current_index != lp.head_index,
		  "system programming error" );
	    lp.base[current_index] =
		min::new_list_aux_gen
		    ( total_size - lp.current_index );
	    lp.previous_index = current_index;
	}

	lp.previous_is_sublist_head = false;
    }

    // Remove any min::NONE() aux values at beginning
    // of aux area.
    //
    unsptr aux_offset = lp.aux_offset;
    while (    aux_offset < total_size
            && lp.base[aux_offset] == min::NONE() )
        ++ aux_offset;
    MUP::aux_offset_of ( lp.vecp ) =
        lp.aux_offset = aux_offset;

    return count;
}

bool MINT::insert_reserve
	( min::list_insptr & lp,
	  min::unsptr insertions,
	  min::unsptr elements,
	  bool use_obj_aux_stubs )
{
    // Warning: it is OK if lp is not yet started.
 
    bool result = false;

#   if MIN_USE_OBJ_AUX_STUBS
	if ( use_obj_aux_stubs )
	    MINT::acc_reserve_stub_free_list
		( insertions + elements );
	else
#   endif
    {
	unsptr desired_size = 2 * insertions + elements;
	unsptr total_size =
	    min::total_size_of ( lp.vecp );
	if ( desired_size < 1000 )
	{
	    if ( desired_size < total_size / 2 )
		desired_size = total_size / 2;
	    if ( desired_size > 1000 )
	        desired_size = 1000;
	}
	min::expand ( lp.vecp, desired_size );
	min::insert_refresh ( lp );
	result = true;
    }

    lp.reserved_insertions = insertions;
    lp.reserved_elements = elements;
#   if MIN_USE_OBJ_AUX_STUBS
	lp.use_obj_aux_stubs = use_obj_aux_stubs;
#   endif

    return result;
}

// FORLIST(vp,i,s,v,free) ... code ... ENDFORLIST
//
// Scans through a list in object pointed at by
// min::obj_vec_ptr vp using unsptr i as the index
// and const min::stub * s as the aux stub pointer if
// aux stubs are in use.  The next value is put into
// min::gen v and ... code ... is run.  vp,i,s,v should
// be simple C++ names of pre-declared variables.
//
// free is true to free any aux stubs scanned, and false
// to not do this.  free should be `true' or `false'.
//
// If MIN_USE_OBJ_AUX_STUBS is not true, s and free are
// not used.
//
# if MIN_USE_OBJ_AUX_STUBS

#   define FORLIST(vp,i,s,v,free) \
    { \
	min::gen v; \
	while ( true ) \
	{ \
	    if ( s != NULL ) \
	    { \
		v = MUP::gen_of ( s ); \
		min::uns64 c = MUP::control_of ( s ); \
		if ( free ) \
		    MUP::free_aux_stub \
		        ( (min::stub *) s ); \
		if ( c & MUP::STUB_ADDRESS ) \
		    s = MUP::stub_of_control ( c ); \
		else \
		{ \
		    i = MUP::value_of_control ( c ); \
		    if ( i != 0 ) \
		        i = min::total_size_of ( vp ) \
			  - i; \
		    s = NULL; \
		} \
	    } \
	    else if ( i != 0 ) \
	    { \
		v = MUP::base(vp)[i]; \
		if ( min::is_list_aux ( v ) ) \
		{ \
		    if ( v == min::LIST_END() ) break; \
		    i = min::total_size_of ( vp ) \
		      - MUP::aux_of ( v ); \
		    continue; \
		} \
		else if ( min::is_stub ( v ) ) \
		{ \
		    const min::stub * s2 = \
			MUP::stub_of ( v ); \
		    if (    MUP::type_of ( s2 ) \
			 == min::LIST_AUX ) \
		    { \
			s = s2; \
			continue; \
		    } \
		} \
		-- i; \
	    } \
	    else break;

#   define ENDFORLIST } }

# else // ! MIN_USE_OBJ_AUX_STUBS

#   define FORLIST(vp,i,s,v,free) \
    { \
	min::gen v; \
	while ( true ) \
	{ \
	    v = MUP::base(vp)[i]; \
	    if ( min::is_list_aux ( v ) ) \
	    { \
		if ( v == min::LIST_END() ) break; \
		i = min::total_size_of ( vp ) \
		  - MUP::aux_of ( v ); \
		continue; \
	    } \
	    -- i;

#   define ENDFORLIST } }

# endif // MIN_USE_OBJ_AUX_STUBS

min::unsptr MUP::list_element_count
	( min::obj_vec_ptr & vp,
	  min::unsptr index
#	if MIN_USE_OBJ_AUX_STUBS
	  , const min::stub * s // = NULL
#	endif
	)
{
    unsptr count = 1;  // +1 for LIST_END
    FORLIST(vp,index,s,v,false)
        ++ count;
	if ( min::is_sublist_aux ( v ) )
	{
	    min::unsptr subindex = MUP::aux_of ( v );
	    if ( subindex != 0 )
		count += MUP::list_element_count
		    ( vp,   min::total_size_of ( vp )
			  - subindex );
	}
#	if MIN_USE_OBJ_AUX_STUBS

	    else if ( is_stub ( v ) )
	    {
		const min::stub * s2 =
		    MUP::stub_of ( v );
		if (    MUP::type_of ( s2 )
		     == SUBLIST_AUX )
		    count += MUP::list_element_count
				( vp, 0, s2 );
	    }
#	endif // MIN_USE_OBJ_AUX_STUBS

    ENDFORLIST

    return count;
}

min::unsptr min::list_element_count
	( min::obj_vec_ptr & vp )
{
    unsptr count = 0;
    unsptr index = MUP::var_offset_of ( vp );
    unsptr end_index = MUP::unused_offset_of ( vp );
    unsptr total_size = total_size_of ( vp );
    while ( index < end_index )
    {
        min::gen v = MUP::base(vp)[index++];

	if ( is_list_aux ( v )
	     ||
	     is_sublist_aux ( v ) )
	{
	    unsptr index2 = MUP::aux_of ( v );
	    if ( index2 != 0 )
	        count += MUP::list_element_count
			    ( vp, total_size - index2 );
	}
#	if MIN_USE_OBJ_AUX_STUBS
	    else if ( is_stub ( v ) )
	    {
	    	const min::stub * s =
		    MUP::stub_of ( v );
		int type = MUP::type_of ( s );
	        if ( type == LIST_AUX
		     ||
		     type == SUBLIST_AUX )
		    count += MUP::list_element_count
				( vp, 0, s );
	    }
#	endif // MIN_USE_OBJ_AUX_STUBS
    }

    return count;
}

min::unsptr min::hash_count_of
	( min::obj_vec_ptr & vp )
{
    unsptr count = 0;
    unsptr index = MUP::hash_offset_of ( vp );
    unsptr end_index = MUP::attr_offset_of ( vp );
    for ( ; index < end_index; ++ index )
    {
        min::gen v = MUP::base(vp)[index];
	if ( v == LIST_END() ) continue;

	// We could simplify this BUT we want to add
	// MIN_ASSERTs to be sure hash table element
	// is a list pointer.
	//
	min::unsptr index2 = index;
#       if MIN_USE_OBJ_AUX_STUBS
	    const min::stub * s = NULL;
	    if ( is_stub ( v ) )
	    {
	    	s = MUP::stub_of ( v );
		MIN_ASSERT
		    ( MUP::type_of ( s ) == LIST_AUX,
		      "system programming error" );
	    }
	    else
#       endif
	MIN_ASSERT ( is_list_aux ( v ),
		     "system programming error" );
	    
	bool label_is_next = true;
	FORLIST(vp,index2,s,v2,false)
	    if ( label_is_next )
	    {
	        ++ count;
		label_is_next = false;
	    }
	    else label_is_next = true;
	ENDFORLIST
	MIN_ASSERT ( label_is_next,
	             "system programming error" );
    }

    return count;
}

// The following functions copy parts of an object body
// to a work area.  The first pass copies the variable
// vector, hash table, and attribute vectors, and any
// lists or sublists they point directly at, to the work
// area.  What is in the work aux area at the end of
// this pass are clean lists which may contain pointers
// to sublists in the original object.  What is in the
// work variable vector, hash table, and attribute
// vector at the end of this pass is non-aux-pointer
// elements and aux pointers that point at the clean
// lists in the work aux area.
//
// Note that aux stubs in the original object whose
// values are copied to the clean work aux area lists
// are deallocated when the values in the stubs are
// copied to the work area.
//
// The work area is defined by
//	work_begin	// Location of first element
//			// of work area.
//	work_low	// Next element of variables
//			// vector, hash table, or
//			// attribute vector to be
//			// copied to in work area.
//	work_high	// Last element of auxiliary
//			// area to be copied to in
//			// work area.
//	work_end	// Location after last element
//			// of work area.
//
// Data are pushed into the work area by
//
//		* work_low ++ = ...
//		* -- work_high = ...
//
// and aux pointer indices reference
//
//		work_end[-index]
//
// After the first pass a second pass scans the auxili-
// ary area from end to beginning copying sublists
// pointed at by auxiliary area elements.
//
// After object has been prepared in work area, the new
// object contents from var_offset to unused_offset are
// in work_begin to work_low, and new auxiliary area is
// in work_high to work_end.

// Copy elements MUP::base(vp)[index .. end_index-1] to
// work as per pass1.
//
inline void copy_to_work
	( min::obj_vec_ptr & vp,
	  min::unsptr index,
	  min::unsptr end_index,
	  min::gen * & work_low,
	  min::gen * & work_high,
	  min::gen *   work_end )
{
    min::unsptr total_size = min::total_size_of ( vp );
    while ( index < end_index )
    {
	min::gen v = MUP::base(vp)[index++];
	MIN_REQUIRE ( work_low < work_high );
	* work_low ++ = v;
#           if MIN_USE_OBJ_AUX_STUBS
	    const min::stub * s = NULL;
#           endif
	min::unsptr index2 = 0;
	if ( min::is_sublist_aux ( v )
	     ||
	     min::is_list_aux ( v ) )
	{
	    index2 = MUP::aux_of ( v );
	    if ( index2 == 0 ) continue;
	    index2 = total_size - index2;
	    work_low[-1] =
		MUP::renew_gen
		    ( v, work_end - work_high + 1 );
	}
#           if MIN_USE_OBJ_AUX_STUBS
	    else if ( min::is_stub ( v ) )
	    {
		s = MUP::stub_of ( v );
		int type = MUP::type_of ( s );
		if ( type == min::LIST_AUX )
		    work_low[-1] =
			MUP::new_list_aux_gen
			    ( work_end - work_high
				       + 1 );
		else if ( type == min::SUBLIST_AUX )
		    work_low[-1] =
			MUP::new_sublist_aux_gen
			    ( work_end - work_high
				       + 1 );
		else continue;
	    }
#           endif // MIN_USE_OBJ_AUX_STUBS
	else continue;

	FORLIST(vp,index2,s,v2,true)
	    * -- work_high = v2;
	    MIN_REQUIRE ( work_low < work_high );
	ENDFORLIST
	* -- work_high = min::LIST_END();
    }
}

// Copy hash table to work as per pass 1, changing the
// size of the hash table.
//
inline void copy_hash_to_work
	( min::obj_vec_ptr & vp,
	  min::unsptr new_hash_size,
	  min::gen * & work_low,
	  min::gen * & work_high,
	  min::gen *   work_end )
{
    // In order to change the hash table size we must
    // precompute the length of the lists headed by
    // the new hash table elements.  Then we allocate
    // the new lists with elements equal to 0, and
    // then we fill in the elements.

    // hash_count[i] is the number of attribute/node
    // name-descriptor-pairs that will land in the new
    // hash_table element of index i.
    //
    min::unsptr old_hash_size =
        min::hash_size_of ( vp );
    min::unsptr hash_count[new_hash_size];
    memset ( (void *) hash_count, 0,
             sizeof ( hash_count ) );
    min::unsptr begin_index =
        MUP::hash_offset_of ( vp );
    min::unsptr end_index = begin_index + old_hash_size;
    for ( min::unsptr index = begin_index;
          index < end_index; ++ index )
    {
#       if MIN_USE_OBJ_AUX_STUBS
	    const min::stub * s = NULL;
#       endif
	min::unsptr index2 = index;
	bool label_is_next = true;
	FORLIST(vp,index2,s,v,false)
	    if ( label_is_next )
	    {
		min::uns32 hash = min::hash ( v )
			        % new_hash_size;
		++ hash_count[hash];
		label_is_next = false;
	    }
	    else label_is_next = true;
	ENDFORLIST
	MIN_ASSERT ( label_is_next,
	             "system programming error" );
    }

    // Now allocate new hash table lists, with the
    // correct length but elements equal to 0.  In the
    // new hash table use aux pointers to point at the
    // ENDs of the new lists.  As the lists are filled
    // these will be incremented to point to the last
    // element actually set in the list.
    //
    // Move work_low past the new hash table while we
    // do this.
    //
    min::gen * new_hash = work_low;
    for ( min::unsptr i = 0; i  < new_hash_size; ++ i )
    {
	min::unsptr c = hash_count[i];
	if ( c == 0 )
	    * work_low ++ = min::LIST_END();
	else
	{
	    work_high -= 2 * c + 1;
	    * work_low ++ =
		MUP::new_list_aux_gen
		    ( work_end - work_high );
	    * work_high = min::LIST_END();
	    memset ( (void *) ( work_high + 1 ), 0,
		     2 * c
		       * sizeof ( min::gen ) );
	}
    }

    // Now go back through the original hash table and
    // move the attribute/node-name-descriptor-pairs to
    // the new hash table lists.  When a pair is moved
    // into new_hash[i], increment the aux pointer in
    // new_hash[i] by 2 to point at the pair just moved.
    //
    // Deallocate aux stubs in the old hash table if
    // there are any.
    //
    for ( min::unsptr index = begin_index;
          index < end_index; ++ index )
    {
	min::unsptr index2 = index;
	bool label_is_next = true;
	min::gen * descriptor;
#       if MIN_USE_OBJ_AUX_STUBS
	    const min::stub * s = NULL;
#       endif

	FORLIST(vp,index2,s,v,true)
	    if ( label_is_next )
	    {
		min::uns32 hash = min::hash ( v )
			        % new_hash_size;
		min::unsptr index3 =
		    MUP::aux_of
			( new_hash[hash] );
		index3 -= 2;
		descriptor = work_end - index3;
		* descriptor -- = v;
		new_hash[hash] =
		    MUP::new_list_aux_gen ( index3 );
		label_is_next = false;
	    }
	    else
	    {
		* descriptor = v;
		label_is_next = true;
	    }
	ENDFORLIST
	MIN_ASSERT ( label_is_next,
	             "system programming error" );
    }
}

// Perform second pass on work area.
//
inline void scan_work_area
	( min::obj_vec_ptr & vp,
	  min::gen * & work_low,
	  min::gen * & work_high,
	  min::gen *   work_end )
{

    // Scan from the end of the new AUX area to the
    // beginning moving any sublists found.
    //
    min::gen * p = work_end;
    while ( p > work_high )
    {
        min::gen v = * -- p;
	if ( ! min::is_sublist ( v )
	     ||
	     v == min::EMPTY_SUBLIST() )
	    continue;

#       if MIN_USE_OBJ_AUX_STUBS
	    const min::stub * s = NULL;
	    min::unsptr index = 0;
	    if ( min::is_sublist_aux ( v ) )
		index = MUP::aux_of ( v );
	    else
		s = MUP::stub_of ( v );
#       else
	    min::unsptr index = MUP::aux_of ( v );
#       endif

	* p = min::new_sublist_aux_gen
	    ( work_end - work_high + 1 );
	FORLIST(vp,index,s,v2,true)
	    * -- work_high = v2;
	    MIN_REQUIRE ( work_low < work_high );
	ENDFORLIST
	* -- work_high = min::LIST_END();
    }
}

void min::reorganize
	( min::obj_vec_insptr & vp,
	  min::unsptr hash_size,
	  min::unsptr var_size,
	  min::unsptr unused_size,
	  bool expand )
{
    unsptr old_hash_size = min::hash_size_of ( vp );
    unsptr old_var_size = min::var_size_of ( vp );

    // Figure the size of the work area.  Object header
    // is NOT included.  Work area may be oversized.
    //
    unsptr work_size = var_size
                     + 2 * hash_size
		       // If hash_size > old_hash_size
		       // we need to add an a LIST_END
		       // for each new hash table entry.
                     + attr_size_of ( vp );
#   if MIN_USE_OBJ_AUX_STUBS
	// if ( MINT::aux_flag_of ( vp ) )
	    work_size += min::list_element_count ( vp );
	// else
	    // work_size += aux_size_of ( vp );
#   else
	work_size += aux_size_of ( vp );
#   endif

    min::gen work_begin[work_size];
    min::gen * work_low = work_begin;
    min::gen * work_high = work_begin + work_size;
    min::gen * work_end = work_begin + work_size;

    // Optimize a bit by making the first copy_to_work
    // as large as possible.  If var_size and hash_size
    // have not changed it can do the whole thing.
    //
    unsptr index = MUP::var_offset_of ( vp );
    unsptr end_index;
    if (var_size < old_var_size )
        end_index = var_size;
    else if ( var_size > old_var_size )
        end_index = old_var_size;
    else if ( hash_size != old_hash_size )
	end_index = MUP::hash_offset_of ( vp );
    else
	end_index = MUP::unused_offset_of ( vp );

    ::copy_to_work
	( vp, index, end_index,
	  work_low, work_high, work_end );

    if ( end_index != MUP::unused_offset_of ( vp ) )
    {
	while ( work_low < work_begin + var_size )
	    * work_low ++ = min::UNDEFINED();

	if ( hash_size != old_hash_size )
	{
	    ::copy_hash_to_work
		( vp, hash_size,
		work_low, work_high, work_end );
	    index = MUP::attr_offset_of ( vp );
	}
	else
	    index = MUP::hash_offset_of ( vp );

	end_index = MUP::unused_offset_of ( vp );
	::copy_to_work
	    ( vp, index, end_index,
	      work_low, work_high, work_end );
    }

    ::scan_work_area
	( vp, work_low, work_high, work_end );

    unsptr total_size =
          ( work_low - work_begin )
	+ ( work_end - work_high )
	+ unused_size;

    int type;
    unsptr header_size;
    ::compute_object_type
	( type, total_size, header_size,
	  var_size, hash_size, expand );

    MUP::resize_body rbody
        ( MUP::stub_of ( vp ),
	  total_size * sizeof ( min::gen ),
	    min::total_size_of ( vp )
	  * sizeof ( min::gen ) );

    unsptr var_offset = header_size;
    unsptr hash_offset = var_offset + var_size;
    unsptr attr_offset = hash_offset + hash_size;
    unsptr unused_offset = attr_offset
                         + min::attr_size_of ( vp );
    unsptr aux_offset = total_size
                      - ( work_end - work_high );

    void * & newb = MUP::new_body_ptr_ref ( rbody );

    ::compute_object_header
	( newb,
	  type,
	  var_size,
	  hash_size,
	  total_size,
	  unused_offset,
	  aux_offset,
	  vp.total_size_flags & ~ min::OBJ_AUX,
	  vp.unused_offset_flags,
	  vp.aux_offset_flags );

    memcpy ( (min::gen *) newb + header_size,
             work_begin,
	       ( work_low - work_begin )
	     * sizeof ( min::gen ) );
    memset ( (void *)
             ( (min::gen *) newb + unused_offset ), 0,
               ( aux_offset - unused_offset )
	     * sizeof ( min::gen ) );
    memcpy ( (min::gen *) newb + aux_offset,
             work_high,
	       ( work_end - work_high )
	     * sizeof ( min::gen ) );
    MUP::retype_resize_body ( rbody, type );

    vp.unused_offset = unused_offset;
    vp.aux_offset = aux_offset;
    vp.hash_size = hash_size;
    vp.total_size = total_size;
    vp.var_offset = var_offset;
    vp.hash_offset = hash_offset;
    vp.attr_offset = attr_offset;
}

void min::compact
	( min::obj_vec_insptr & vp,
	  min::unsptr var_size,
	  min::unsptr unused_size,
	  bool expand )
{
    unsptr old_hash_size = min::hash_size_of ( vp );

    // Figure the size of the new hash table.
    //
    unsptr hash_count = hash_count_of ( vp );
    unsptr hash_size = old_hash_size;
    if ( 3 * hash_count < 2 * old_hash_size
         ||
	 hash_count > 3 *old_hash_size )
        hash_size = 2 * hash_count;

    reorganize
	( vp, hash_size, var_size, unused_size,
	      expand );
}

bool min::list_equal
	( min::obj_vec_ptr & vp1,
	  min::obj_vec_ptr & vp2,
	  bool include_var,
	  bool include_attr,
	  bool include_hash )
{
    if ( include_var )
    {
        min::unsptr s = var_size_of ( vp1 );
	if ( s != var_size_of ( vp2 ) )
	    return false;
	for ( min::unsptr i = 0; i < s; ++ i )
	{
	    if ( var(vp1,i) != var(vp2,i) )
	        return false;
	}
    }
    if ( include_attr )
    {
        min::unsptr s = attr_size_of ( vp1 );
	if ( s != attr_size_of ( vp2 ) )
	    return false;
	for ( min::unsptr i = 0; i < s; ++ i )
	{
	    if ( attr(vp1,i) != attr(vp2,i) )
	    {
		min::list_ptr lp1 ( vp1 );
		min::list_ptr lp2 ( vp2 );
		start_attr ( lp1, i );
		start_attr ( lp2, i );
		if ( ! list_equal ( lp1, lp2 ) )
		    return false;
	    }
	}
    }
    if ( include_hash )
    {
        min::unsptr s = hash_size_of ( vp1 );
	if ( s != hash_size_of ( vp2 ) )
	    return false;
	for ( min::unsptr i = 0; i < s; ++ i )
	{
	    if ( hash(vp1,i) != hash(vp2,i) )
	    {
		min::list_ptr lp1 ( vp1 );
		min::list_ptr lp2 ( vp2 );
		if ( ! list_equal ( lp1, lp2 ) )
		    return false;
	    }
	}
    }
    return true;
}

min::printer min::list_print
	( min::printer printer,
	  min::obj_vec_ptr & vp,
	  bool include_var,
	  bool include_attr,
	  bool include_hash )
{
    printer << min::bom << min::no_auto_break;

    if ( include_var )
    {
        min::unsptr s = var_size_of ( vp );
	printer << min::indent << s << " variables:";
	for ( min::unsptr i = 0; i < s; ++ i )
	    printer << min::indent
	            << min::puns ( i, "%7d: " )
		    << min::save_indent
	            << min::pgen ( var(vp,i) )
		    << min::restore_indent;
    }
    if ( include_attr )
    {
        min::unsptr s = attr_size_of ( vp );
	printer << min::indent << s
	        << " vector elements:";
	for ( min::unsptr i = 0; i < s; ++ i )
	{
	    printer << min::indent
	            << min::puns ( i, "%7d: " )
		    << min::save_indent;
	    min::list_ptr lp ( vp );
	    min::start_attr ( lp, i );
	    min::gen v = min::current ( lp );
	    if ( ! min::is_list_end ( v )
	         &&
		 min::is_list_end ( min::peek ( lp ) ) )
	        printer << min::pgen ( v );
	    else
	        min::list_print ( printer, lp );
	    printer << min::restore_indent;
	}
    }
    if ( include_hash )
    {
        min::unsptr s = hash_size_of ( vp );
	printer << min::indent << s
	        << " hash table:";
	for ( min::unsptr i = 0; i < s; ++ i )
	{
	    printer << min::indent
	            << min::puns ( i, "%7d: " )
		    << min::save_indent;
	    min::list_ptr lp ( vp );
	    min::start_hash ( lp, i );
	    min::gen v = min::current ( lp );
	    if ( ! min::is_list_end ( v )
	         &&
		 min::is_list_end ( min::peek ( lp ) ) )
	        printer << min::pgen ( v );
	    else
	        min::list_print ( printer, lp );
	    printer << min::restore_indent;
	}
    }
    return printer << min::eom;
}

bool min::list_equal
	( min::list_ptr & lp1,
	  min::list_ptr & lp2 )
{
    min::gen v1 = min::current ( lp1 );
    min::gen v2 = min::current ( lp2 );
    while ( true )
    {
        if ( v1 != v2 )
	{
	    if ( ! is_sublist ( v1 )
	         ||
		 ! is_sublist ( v2 ) )
	        return false;
	    min::list_ptr slp1
	        ( min::obj_vec_ptr_of ( lp1 ) );
	    start_sublist ( slp1, lp1 );
	    min::list_ptr slp2
	        ( min::obj_vec_ptr_of ( lp2 ) );
	    start_sublist ( slp2, lp2 );
	    if ( ! list_equal ( slp1, slp2 ) )
	        return false;
	}
	else if ( is_list_end ( v1 ) )
	    break;
	v1 = min::next ( lp1 );
	v2 = min::next ( lp2 );
    }
    return true;
}

static void list_print
	( min::printer printer,
	  min::list_ptr & lp )
{
    printer << min::set_break
	    << "("
	    << min::save_indent;
    min::gen v = min::current ( lp );
    while ( true )
    {
        if ( min::is_list_end ( v ) ) break;

	if ( ! min::is_sublist ( v ) )
	    printer << " " << min::set_break
	            << min::pgen ( v );
	else
	{
	    min::list_ptr slp
	        ( min::obj_vec_ptr_of ( lp ) );
	    printer << " ";
	    ::list_print ( printer, slp );
	}
	v = min::next ( lp );
    }
    printer << " )" << min::restore_indent;
}

min::printer min::list_print
	( min::printer printer,
	  min::list_ptr & lp )
{
    printer << min::save_print_format
	    << min::no_auto_break;
    ::list_print ( printer, lp );
    return printer << min::restore_print_format;
}

// Object Attribute Level
// ------ --------- -----

# if MIN_ALLOW_PARTIAL_ATTR_LABELS

    template < class vecpt >
    void MINT::locate
	    ( MUP::attr_ptr_type<vecpt> & ap,
	      min::gen name,
	      bool allow_partial_labels )
    {
	typedef MUP::attr_ptr_type<vecpt> ap_type;

	ap.attr_name = name;
	ap.reverse_attr_name = min::NONE();

	// Set len to the number of elements in the
	// label and element[] to the vector of
	// label elements.
	//
	bool is_label = is_lab ( name );
	unsptr len;
	if ( is_label )
	{
	    len = min::lablen ( name );
	    MIN_ASSERT ( len > 0,
	                 "name argument is zero length"
			 " label" );
	}
	else
	{
	    MIN_ASSERT
		( is_name ( name ),
		  "name argument is not a name" );
	    len = 1;
	}

	min::gen element[len];
	if ( is_label )
	{
	    labncpy ( element, name, len );
	    if ( len == 1
	         &&
		 ( is_num ( element[0] )
		   ||
		   is_str ( element[0] ) ) )
	        ap.attr_name = element[0];
	}
	else element[0] = name;

	// Process element[0] and if found set
	// ap.length = 1.
	//
	// If element[0] is an integer in the right
	// range, locate attribute vector entry.
	// Otherwise locate hash table entry.
	// Set c to the node-descriptor if that is
	// found, and to LIST_END() if not.
	//
	float64 f;
	int i;
	min::gen c;
	if ( is_num ( element[0] )
	     &&
	     0 <= ( f = float_of ( element[0] ) )
	     &&
	     ( i = (int) f ) == f
	     &&
	     (unsigned) i < attr_size_of
	             ( obj_vec_ptr_of ( ap.dlp ) ) )
	{
	    start_attr ( ap.dlp, i );
	    ap.flags = ap_type::IN_VECTOR;
	    ap.index = i;
	    c = current ( ap.dlp );
	}
	else
	{
	    MIN_ASSERT ( is_name ( element[0] ),
			 "name argument is not"
			 " a name" );

	    ap.index = min::hash ( element[0] );
	    ap.flags = 0;

	    start_hash ( ap.dlp, ap.index );

	    for ( c = current ( ap.dlp );
		  ! is_list_end ( c );
	          next ( ap.dlp ),
		  c = next ( ap.dlp ) )
	    {
		if ( c == element[0] )
		{
		    c = next ( ap.dlp );
		    MIN_ASSERT ( ! is_list_end ( c ),
		                 "system programming"
				 " error" );
		    break;
		}
	    }
	}

	if ( is_list_end ( c ) )
	{
	    ap.length = 0;
	    ap.state = ap_type::LOCATE_FAIL;
	    return;
	}
	else ap.length = 1;

	unsptr locate_length = 0;
	    // ap.length for the last node descriptor
	    // with a non-empty value set.
	    // locate_dlp is value of dlp for this
	    // length.

	while ( true )
	{
	    if ( ! is_sublist ( c ) )
	    {
	        locate_length = ap.length;
		start_copy ( ap.locate_dlp, ap.dlp );
	        break;
	    }

	    start_sublist ( ap.lp, ap.dlp );
	    c = current ( ap.lp );
	    if (    ! is_list_end ( c )
	         && ! is_sublist ( c )
		 && ! is_control_code ( c ) )
	    {
	        locate_length = ap.length;
		start_copy ( ap.locate_dlp, ap.dlp );
	    }

	    if ( ap.length >= len ) break;

	    for ( ;
	             ! is_sublist ( c )
		  && ! is_list_end ( c );
		  c = next ( ap.lp ) );

	    if ( ! is_sublist ( c ) )
	        break;

	    start_sublist ( ap.lp );

	    for ( c = current ( ap.lp );
	          ! is_list_end ( c );
	          next ( ap.lp ),
		  c = next ( ap.lp ) )
	    {
		if ( c == element[ap.length] )
		{
		    c = next ( ap.lp );
		    MIN_ASSERT ( ! is_list_end ( c ),
		                 "system programming"
				 " error" );
		    break;
		}
	    }
	    if ( is_list_end ( c ) ) break;

	    start_copy ( ap.dlp, ap.lp );
	    ++ ap.length;
	}

	if ( allow_partial_labels
	     &&
	     locate_length > 0 )
	{
	    ap.length = locate_length;
	    start_copy ( ap.dlp, ap.locate_dlp );
	    ap.state = ap_type::LOCATE_NONE;
	}
	else if ( ap.length == len )
	{
	    start_copy ( ap.locate_dlp, ap.dlp );
	    ap.state = ap_type::LOCATE_NONE;
	}
	else
	{
	    start_copy ( ap.locate_dlp, ap.dlp );
	    ap.state = ap_type::LOCATE_FAIL;
	    // ap.length and ap.locate_dlp define
	    // recognized partial label even if
	    // this has empty value set.
	}

	return;
    }
    template void MINT::locate
	    ( min::attr_ptr & ap,
	      min::gen name,
	      bool allow_partial_labels );
    template void MINT::locate
	    ( min::attr_updptr & ap,
	      min::gen name,
	      bool allow_partial_labels );
    template void MINT::locate
	    ( min::attr_insptr & ap,
	      min::gen name,
	      bool allow_partial_labels );

    // Continue relocation after ap.locate_dlp is
    // positioned at beginning of hash-list or
    // vector-element treated as a list.  ap.length
    // must be >= 1.  Result is a setting of
    // ap.locate_dlp only.
    // 
    template < class vecpt >
    void MINT::relocate
	    ( MUP::attr_ptr_type<vecpt> & ap )
    {
	typedef MUP::attr_ptr_type<vecpt> ap_type;

	MIN_ASSERT ( ap.length > 0,
	             "system programming error" );

	bool is_label = is_lab ( ap.attr_name );
	unsptr len;
	if ( is_label )
	    len = min::lablen ( ap.attr_name );
	else len = 1;
	min::gen element[len];
	if ( is_label )
	    labncpy ( element, ap.attr_name, len );
	else element[0] = ap.attr_name;

	min::gen c;
	if ( ! ( ap.flags & ap_type::IN_VECTOR ) )
	{
	    for ( c = current ( ap.locate_dlp );
		  ! is_list_end ( c );
		  next ( ap.locate_dlp ),
		  c = next ( ap.locate_dlp ) )
	    {
		if ( c == element[0] )
		{
		    c = next ( ap.locate_dlp );
		    MIN_ASSERT ( ! is_list_end ( c ),
		                 "system programming"
				 " error" );
		    break;
		}
	    }
	}

	MIN_ASSERT ( ap.length <= len,
	             "system programming error" );
	unsptr length = 1;
	start_copy ( ap.dlp, ap.locate_dlp );
	while ( length < ap.length )
	{
	    if ( ! is_sublist ( current ( ap.dlp ) ) )
	        break;
	    start_sublist ( ap.dlp );
	    for ( c = current ( ap.dlp );
	             ! is_sublist ( c )
		  && ! is_list_end ( c );
		  c = next ( ap.dlp ) );

	    if ( ! is_sublist ( c ) )
	        break;
	    start_sublist ( ap.dlp );

	    for ( c = current ( ap.dlp );
	          ! is_list_end ( c );
		  next ( ap.dlp ),
		  c = next ( ap.dlp ) )
	    {
		if ( c == element[ap.length] )
		{
		    c = next ( ap.dlp );
		    MIN_ASSERT ( ! is_list_end ( c ),
		                 "system programming"
				 " error" );
		    break;
		}
	    }
	    if ( is_list_end ( c ) ) break;

	    start_copy ( ap.locate_dlp, ap.dlp );
	    ++ length;
	}

	MIN_ASSERT ( length == ap.length,
	             "system programming error" );
    }
    template void MINT::relocate
	    ( min::attr_ptr & ap );
    template void MINT::relocate
	    ( min::attr_updptr & ap );
    template void MINT::relocate
	    ( min::attr_insptr & ap );

    void MINT::attr_create
	    ( min::attr_insptr & ap,
	      min::gen v )
    {
	typedef min::attr_insptr ap_type;

	MIN_ASSERT
	    ( ap.state == ap_type::LOCATE_FAIL,
	      "system programming error" );

	bool is_label = is_lab ( ap.attr_name );
	unsptr len;
	if ( is_label )
	    len = min::lablen ( ap.attr_name );
	else
	    len = 1;
	min::gen element[len];
	if ( is_label )
	    labncpy ( element, ap.attr_name, len );
	else element[0] = ap.attr_name;

	MIN_ASSERT ( ap.length < len,
	             "system programming error" );

	update_refresh ( ap.locate_dlp );
	if ( insert_reserve
	           ( ap.locate_dlp,
	             2 * ( len - ap.length ),
	             4 * ( len - ap.length ) + 1 ) )
	{
	    insert_refresh ( ap.dlp );
	    insert_refresh ( ap.lp );
	}

	if ( ap.length == 0 )
	{
	    if ( ap.flags & ap_type::IN_VECTOR )
	    {
		start_attr
		    ( ap.locate_dlp, ap.index );
		min::gen c = current ( ap.locate_dlp );
		if ( is_list_end ( c ) )
		{
		    min::gen elements[1] =
			{ len == 1 ? v :
			  min::EMPTY_SUBLIST() };
		    insert_before ( ap.locate_dlp,
				    elements, 1 );
		}
	    }
	    else
	    {
		start_hash ( ap.locate_dlp, ap.index );
		min::gen elements[2] =
		    { element[0], len == 1 ?
		      v :
		      min::EMPTY_SUBLIST() };
		insert_before ( ap.locate_dlp,
		                elements, 2 );
		next ( ap.locate_dlp );
	    }

	    ap.length = 1;
	}

	while ( ap.length < len )
	{
	    min::gen c = current ( ap.locate_dlp );

	    if ( ! is_sublist ( c ) )
	    {
		update ( ap.locate_dlp,
		         min::EMPTY_SUBLIST() );
		start_sublist ( ap.locate_dlp );
		min::gen elements[2] =
		    { c, min::EMPTY_SUBLIST() };
		insert_before
		    ( ap.locate_dlp, elements, 2 );
		next ( ap.locate_dlp );
	    }
	    else
	    {
	        start_sublist ( ap.locate_dlp );
		for ( c = current ( ap.locate_dlp );
		         ! is_sublist ( c )
		      && ! is_control_code ( c )
		      && ! is_list_end ( c );
		      c = next ( ap.locate_dlp ) );

		if ( ! is_sublist ( c ) )
		{
		    min::gen elements[1] =
			{ min::EMPTY_SUBLIST() };
		    insert_before
		        ( ap.locate_dlp, elements, 1 );
		}
	    }
	    start_sublist ( ap.locate_dlp );

	    min::gen elements[2] =
		{ element[ap.length],
		  len == ap.length + 1 ? v :
	          min::EMPTY_SUBLIST() };
	    insert_before
	        ( ap.locate_dlp, elements, 2 );

	    next ( ap.locate_dlp );
	    ++ ap.length;
	}
	start_copy ( ap.dlp, ap.locate_dlp );

	if ( ap.reverse_attr_name == min::NONE() )
	    ap.state = ap_type::LOCATE_NONE;
	else if (    ap.reverse_attr_name
	          == min::ANY() )
	    ap.state = ap_type::LOCATE_ANY;
	else
	    ap.state = ap_type::REVERSE_LOCATE_FAIL;
    }

# else // ! MIN_ALLOW_PARTIAL_ATTR_LABELS

    template < class vecpt >
    void MINT::locate
	    ( MUP::attr_ptr_type<vecpt> & ap,
	      min::gen name )
    {
	typedef MUP::attr_ptr_type<vecpt> ap_type;

	ap.reverse_attr_name = min::NONE();

	// If name is label whose only element is an
	// atom, set name = the atom.
	//
	if ( is_lab ( name ) )
	{
	    if ( min::lablen ( name ) == 1 )
	    {
		min::gen atom;
		labncpy ( & atom, name, 1 );
		if ( is_num ( atom )
		     ||
		     is_str ( atom ) )
		    name = atom;
	    }
	}
	else
	    MIN_ASSERT
		( is_name ( name ),
		  "name argument is not a name" );

	ap.attr_name = name;

	// If name is an integer in the right range,
	// locate attribute vector entry and return.
	//
	if ( is_num ( name ) )
	{
	    float64 f = float_of ( name );
	    int i = (int) f;

	    if ( i == f
		 &&
		 0 <= i
		 &&
		 (unsigned) i < attr_size_of
		        ( obj_vec_ptr_of ( ap.dlp ) ) )
	    {
		start_attr ( ap.locate_dlp, i );
		ap.index = i;
		ap.flags = ap_type::IN_VECTOR;

		min::gen c = current ( ap.locate_dlp );
		if ( is_list_end ( c ) )
		    ap.state = ap_type::LOCATE_FAIL;
		else
		{
		    ap.state = ap_type::LOCATE_NONE;
		    start_copy
		        ( ap.dlp, ap.locate_dlp );
		}
		return;
	    }
	}

	ap.index = min::hash ( name );
	ap.flags = 0;
	start_hash ( ap.locate_dlp, ap.index );

	for ( min::gen c = current ( ap.locate_dlp );
	      ! is_list_end ( c );
	      next ( ap.locate_dlp ),
	      c = next ( ap.locate_dlp ) )
	{
	    if ( c == name )
	    {
		c = next ( ap.locate_dlp );
		MIN_ASSERT ( ! is_list_end ( c ),
		             "system programming"
			     " error" );
		start_copy ( ap.dlp, ap.locate_dlp );
		ap.flags = 0;
		ap.state = ap_type::LOCATE_NONE;
		return;
	    }
	}

        // Name not found.
	//
	ap.state = ap_type::LOCATE_FAIL;
	return;

    }
    template void MINT::locate
	    ( min::attr_ptr & ap,
	      min::gen name );
    template void MINT::locate
	    ( min::attr_updptr & ap,
	      min::gen name );
    template void MINT::locate
	    ( min::attr_insptr & ap,
	      min::gen name );

    // Continue relocation after ap.relocate_dlp is
    // positioned at beginning of hash-list.  State
    // must be >= LOCATE_NONE.  Is NOT called if IN_
    // VECTOR flag is set.  Result is a setting of
    // ap.locate_dlp only.
    // 
    template < class vecpt >
    void MINT::relocate
	    ( MUP::attr_ptr_type<vecpt> & ap )
    {
	for ( min::gen c = current ( ap.locate_dlp );
	      ! is_list_end ( c );
	      next ( ap.locate_dlp ),
	      c = next ( ap.locate_dlp ) )
	{
	    if ( c == ap.attr_name )
	    {
		c = next ( ap.locate_dlp );
		MIN_ASSERT ( ! is_list_end ( c ),
		             "system programming"
			     " error" );
		return;
	    }
	}

	MIN_ABORT ( "relocate could not find"
	            " attribute" );
    }
    template void MINT::relocate
	    ( min::attr_ptr & ap );
    template void MINT::relocate
	    ( min::attr_updptr & ap );
    template void MINT::relocate
	    ( min::attr_insptr & ap );

    void MINT::attr_create
	    ( min::attr_insptr & ap,
	      min::gen v )
    {
	typedef min::attr_insptr ap_type;

	MIN_ASSERT
	    ( ap.state == ap_type::LOCATE_FAIL,
	      "system programming error" );

	if ( ap.flags & ap_type::IN_VECTOR )
	{
	    start_attr ( ap.locate_dlp, ap.index );
	    min::gen c = current ( ap.locate_dlp );
	    MIN_ASSERT ( is_list_end ( c ),
			 "system programming error" );
	    if ( insert_reserve
	             ( ap.locate_dlp, 1, 1 ) )
	    {
	        insert_refresh ( ap.dlp );
	        insert_refresh ( ap.lp );
	    }
	    insert_before ( ap.locate_dlp, & v, 1 );
	    start_copy ( ap.dlp, ap.locate_dlp );
	}
	else
	{
	    start_hash ( ap.locate_dlp, ap.index );

	    if ( insert_reserve
	             ( ap.locate_dlp, 1, 2 ) )
	    {
		insert_refresh ( ap.dlp );
		insert_refresh ( ap.lp );
	    }

	    min::gen elements[2] = { ap.attr_name, v };
	    insert_before
	        ( ap.locate_dlp, elements, 2 );

	    next ( ap.locate_dlp );
	    start_copy ( ap.dlp, ap.locate_dlp );
	}

	if ( ap.reverse_attr_name == min::NONE() )
	    ap.state = ap_type::LOCATE_NONE;
	else if (    ap.reverse_attr_name
	          == min::ANY() )
	    ap.state = ap_type::LOCATE_ANY;
	else
	    ap.state = ap_type::REVERSE_LOCATE_FAIL;
    }

# endif

void MINT::reverse_attr_create
	( min::attr_insptr & ap,
	  min::gen v )
{
    typedef min::attr_insptr ap_type;

    MIN_ASSERT
	( ap.state == ap_type::REVERSE_LOCATE_FAIL,
	  "system programming error" );

    update_refresh ( ap.locate_dlp );
    start_copy ( ap.dlp, ap.locate_dlp );

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	if ( insert_reserve ( ap.dlp, 2, 5 ) )
#   else
	if ( insert_reserve ( ap.dlp, 2, 4 ) )
#   endif
	{
	    insert_refresh ( ap.locate_dlp );
	    insert_refresh ( ap.lp );
	}

    min::gen c = current ( ap.dlp );
    if ( ! is_sublist ( c ) )
    {
	update ( ap.dlp, min::EMPTY_SUBLIST() );
	start_sublist ( ap.dlp );
#       if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    min::gen elements[3] =
		{ c, min::EMPTY_SUBLIST(),
		     min::EMPTY_SUBLIST() };
	    insert_before ( ap.dlp, elements, 3 );
	    next ( ap.dlp );
#       else
	    min::gen elements[2] =
		{ c, min::EMPTY_SUBLIST() };
	    insert_before ( ap.dlp, elements, 2 );
#       endif
	next ( ap.dlp );
    }
    else
    {
	start_sublist ( ap.dlp );
	for ( c = current ( ap.dlp );
		 ! is_sublist ( c )
	      && ! is_control_code ( c )
	      && ! is_list_end ( c );
	      c = next ( ap.dlp ) );

#       if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    if ( ! is_sublist ( c ) )
	    {
		min::gen elements[2] =
		    { min::EMPTY_SUBLIST(),
		      min::EMPTY_SUBLIST() };
		insert_before ( ap.dlp, elements, 2 );
		next ( ap.dlp );
	    }
	    else if ( ! is_sublist ( next ( ap.dlp ) ) )
	    {
		min::gen elements[1] =
		    { min::EMPTY_SUBLIST() };
		insert_before ( ap.dlp, elements, 1 );
	    }
#       else
	    if ( ! is_sublist ( c ) )
	    {
		min::gen elements[1] =
		    { min::EMPTY_SUBLIST() };
		insert_before ( ap.dlp, elements, 1 );
	    }
#       endif
    }
    start_sublist ( ap.dlp );
    min::gen elements[2] =
	{ ap.reverse_attr_name, v };
    insert_before ( ap.dlp, elements, 2 );
    next ( ap.dlp );

    ap.state = ap_type::REVERSE_LOCATE_SUCCEED;
}

template < class vecpt >
void min::locate_reverse
	( MUP::attr_ptr_type<vecpt> & ap,
	  min::gen reverse_name )
{
    typedef MUP::attr_ptr_type<vecpt> ap_type;

    // If reverse_name is label whose only element is an
    // atom, set reverse_name = the atom.
    //
    if ( is_lab ( reverse_name )
	 &&
	 lablen ( reverse_name ) == 1 )
    {
	min::gen atom;
	labncpy ( & atom, reverse_name, 1 );
	if ( is_str ( atom )
	     ||
	     is_num ( atom ) )
	    reverse_name = atom;
    }

    update_refresh ( ap.locate_dlp );

    MIN_ASSERT
        ( is_name ( reverse_name ),
          "reverse name argument is not a name" );
    ap.reverse_attr_name = reverse_name;

    switch ( ap.state )
    {
    case ap_type::INIT:
	    MIN_ABORT
	        ( "bad attribute locate_reverse call" );
    case ap_type::LOCATE_FAIL:
    	    return;
    case ap_type::LOCATE_NONE:
	    if ( reverse_name == min::NONE() )
	        return;
	    else if ( reverse_name == min::ANY() )
	    {
	        ap.state = ap_type::LOCATE_ANY;
		return;
	    }
	    break;
    case ap_type::LOCATE_ANY:
	    if ( reverse_name == min::ANY() )
	        return;
	    else if ( reverse_name == min::NONE() )
	    {
	        ap.state = ap_type::LOCATE_NONE;
		return;
	    }
	    break;
    case ap_type::REVERSE_LOCATE_FAIL:
    case ap_type::REVERSE_LOCATE_SUCCEED:
	    if ( reverse_name == min::NONE() )
	    {
		start_copy ( ap.dlp, ap.locate_dlp );
	        ap.state = ap_type::LOCATE_NONE;
		return;
	    }
	    else if ( reverse_name == min::ANY() )
	    {
		start_copy ( ap.dlp, ap.locate_dlp );
	        ap.state = ap_type::LOCATE_ANY;
		return;
	    }
	    break;
    }

    // ap.locate_dlp is as set by previous successful
    // locate and reverse_name is not NONE() or ANY().

    start_copy ( ap.dlp, ap.locate_dlp );

    min::gen c = current ( ap.dlp );
    if ( ! is_sublist ( c ) )
    {
	ap.state = ap_type::REVERSE_LOCATE_FAIL;
	return;
    }
    start_sublist ( ap.dlp );
    for ( c = current ( ap.dlp );
	  ! is_sublist ( c ) && ! is_list_end ( c );
	  c = next ( ap.dlp ) );

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
    if ( ! is_sublist ( c ) )
    {
	ap.state = ap_type::REVERSE_LOCATE_FAIL;
	return;
    }
    c = next ( ap.dlp );
#   endif
    if ( ! is_sublist ( c ) )
    {
	ap.state = ap_type::REVERSE_LOCATE_FAIL;
	return;
    }
    start_sublist ( ap.dlp );

    for ( min::gen c = current ( ap.dlp );
	  ! is_list_end ( c );
	  next ( ap.dlp ), c = next ( ap.dlp ) )
    {
	if ( c == reverse_name )
	{
	    c = next ( ap.dlp );
	    MIN_ASSERT ( ! is_list_end ( c ),
	                 "system programming error" );
	    ap.state =
		ap_type::REVERSE_LOCATE_SUCCEED;
	    return;
	}
    }

    ap.state = ap_type::REVERSE_LOCATE_FAIL;
}
template void min::locate_reverse
	( min::attr_ptr & ap,
	  min::gen reverse_name );
template void min::locate_reverse
	( min::attr_updptr & ap,
	  min::gen reverse_name );
template void min::locate_reverse
	( min::attr_insptr & ap,
	  min::gen reverse_name );

template < class vecpt >
void min::relocate
	( MUP::attr_ptr_type<vecpt> & ap )
{
    typedef MUP::attr_ptr_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
        return;
    case ap_type::LOCATE_FAIL:
#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    if ( ap.length == 0 ) return;
#	else
	    return;
#	endif
    }

    if ( ap.flags & ap_type::IN_VECTOR )
    {
        start_attr ( ap.locate_dlp, ap.index );
#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    if ( ap.length != 1 ) MINT::relocate ( ap );
#	endif
    }
    else
    {
	start_hash ( ap.locate_dlp, ap.index );
	MINT::relocate ( ap );
    }

    start_copy ( ap.dlp, ap.locate_dlp );

    if ( ap.state != ap_type::REVERSE_LOCATE_SUCCEED )
        return;

    start_copy ( ap.dlp, ap.locate_dlp );

    min::gen c = current ( ap.dlp );
    if ( ! is_sublist ( c ) )
    {
	MIN_ABORT ( "relocate could not find"
	            " reverse attribute" );
    }
    start_sublist ( ap.dlp );
    for ( c = current ( ap.dlp );
	  ! is_sublist ( c ) && ! is_list_end ( c );
	  c = next ( ap.dlp ) );

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
    if ( ! is_sublist ( c ) )
    {
	MIN_ABORT ( "relocate could not find"
	            " reverse attribute" );
    }
    c = next ( ap.dlp );
#   endif
    if ( ! is_sublist ( c ) )
    {
	MIN_ABORT ( "relocate could not find"
	            " reverse attribute" );
    }
    start_sublist ( ap.dlp );

    for ( min::gen c = current ( ap.dlp );
	  ! is_list_end ( c );
	  next ( ap.dlp ), c = next ( ap.dlp ) )
    {
	if ( c == ap.reverse_attr_name )
	{
	    c = next ( ap.dlp );
	    MIN_ASSERT ( ! is_list_end ( c ),
	                 "system programming error" );
	    return;
	}
    }

    MIN_ABORT ( "relocate could not find reverse"
                " attribute" );
}
template void min::relocate
	( min::attr_ptr & ap );
template void min::relocate
	( min::attr_updptr & ap );
template void min::relocate
	( min::attr_insptr & ap );

template < class vecpt >
min::unsptr MINT::get
	( min::gen * out, min::unsptr n,
	  MUP::attr_ptr_type<vecpt> & ap )
{
    typedef MUP::attr_ptr_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT ( "min::get called before locate" );
    case ap_type::LOCATE_ANY:
        break;
    default:
	MIN_ABORT ( "abnormal call to min::get" );
    }

    // state == LOCATE_ANY:

    min::gen c;
    update_refresh ( ap.locate_dlp );
    start_copy ( ap.dlp, ap.locate_dlp );

    if ( ! is_sublist ( current ( ap.dlp ) ) )
        return 0;
    start_sublist ( ap.dlp );
    for ( c = current ( ap.dlp );
	  ! is_sublist ( c ) && ! is_list_end ( c );
	  c = next ( ap.dlp ) );

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	if ( ! is_sublist ( c ) ) return 0;
	c = next ( ap.dlp );
#   endif
    if ( ! is_sublist ( c ) ) return 0;
    start_sublist ( ap.dlp );

    unsptr result = 0;
    for ( min::gen c = current ( ap.dlp );
	  ! is_list_end ( c );
	  c = next ( ap.dlp ) )
    {
        c = next ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    if ( result < n ) * out ++ = c;
	    ++ result;
	}
	else
	{
	    start_sublist ( ap.lp, ap.dlp );
	    c = current ( ap.lp );
	    while ( ! is_list_end ( c ) )
	    {
		if ( result < n ) * out ++ = c;
		++ result;
		c = next ( ap.lp );
	    }
	}
    }

    return result;
}
template min::unsptr MINT::get
	( min::gen * out, min::unsptr n,
	  min::attr_ptr & ap );
template min::unsptr MINT::get
	( min::gen * out, min::unsptr n,
	  min::attr_updptr & ap );
template min::unsptr MINT::get
	( min::gen * out, min::unsptr n,
	  min::attr_insptr & ap );

template < class vecpt >
min::gen MINT::get
	( MUP::attr_ptr_type<vecpt> & ap )
{
    typedef MUP::attr_ptr_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT ( "min::get called before locate" );
    case ap_type::LOCATE_ANY:
        break;
    default:
	MIN_ABORT ( "abnormal call to min::get" );
    }

    // state == LOCATE_ANY:

    min::gen c;
    update_refresh ( ap.locate_dlp );
    start_copy ( ap.dlp, ap.locate_dlp );

    if ( ! is_sublist ( current ( ap.dlp ) ) )
        return min::NONE();
    start_sublist ( ap.dlp );
    for ( c = current ( ap.dlp );
	  ! is_sublist ( c ) && ! is_list_end ( c );
	  c = next ( ap.dlp ) );

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
    if ( ! is_sublist ( c ) ) return min::NONE();
    c = next ( ap.dlp );
#   endif
    if ( ! is_sublist ( c ) ) return min::NONE();
    start_sublist ( ap.dlp );

    min::gen result = min::NONE();
    for ( min::gen c = current ( ap.dlp );
	  ! is_list_end ( c );
	  c = next ( ap.dlp ) )
    {
        c = next ( ap.dlp );
	if ( ! is_sublist ( c ) )
	{
	    if ( result == min::NONE() )
	        result = c;
	    else
	        return min::MULTI_VALUED();
	}
	else
	{
	    start_sublist ( ap.lp, ap.dlp );
	    c = current ( ap.lp );
	    while ( ! is_list_end ( c ) )
	    {
		if ( result == min::NONE() )
		    result = c;
		else
		    return min::MULTI_VALUED();
		c = next ( ap.lp );
	    }
	}
    }

    return result;
}
template min::gen MINT::get
	( min::attr_ptr & ap );
template min::gen MINT::get
	( min::attr_updptr & ap );
template min::gen MINT::get
	( min::attr_insptr & ap );

template < class vecpt >
min::unsptr MINT::get_flags
	( min::gen * p, min::unsptr n,
	  MUP::attr_ptr_type<vecpt> & ap )
{
    typedef MUP::attr_ptr_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::get_flags called before locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::get_flags" );
    }
}
template min::unsptr MINT::get_flags
	( min::gen * p, min::unsptr n,
	  min::attr_ptr & ap );
template min::unsptr MINT::get_flags
	( min::gen * p, min::unsptr n,
	  min::attr_updptr & ap );
template min::unsptr MINT::get_flags
	( min::gen * p, min::unsptr n,
	  min::attr_insptr & ap );

template < class vecpt >
bool MINT::test_flag
	( MUP::attr_ptr_type<vecpt> & ap,
	  unsigned n )
{
    typedef MUP::attr_ptr_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::test_flag called before locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::test_flag" );
    }
}
template bool MINT::test_flag
	( min::attr_ptr & ap,
	  unsigned n );
template bool MINT::test_flag
	( min::attr_updptr & ap,
	  unsigned n );
template bool MINT::test_flag
	( min::attr_insptr & ap,
	  unsigned n );

// Helper functions for min::attr_info_of.

// Called with current(lp) being start of
// double-arrow-sublist.  Return number of reverse
// attribute names with non-empty value sets.
//
inline min::unsptr count_reverse_attrs
	( min::list_ptr & lp )
{
    min::unsptr result = 0;
    min::list_ptr lpr ( min::obj_vec_ptr_of ( lp ) );
    start_sublist ( lpr, lp );
    for ( min::gen c = min::current ( lpr );
          ! min::is_list_end ( c );
	  c = min::next ( lpr ) )
    {
        c = min::next ( lpr );
	if ( c != min::EMPTY_SUBLIST() )
	    ++ result;
    }
    return result;
}

bool min::internal::compute_attr_info
	( min::attr_info & info,
	  min::list_ptr & lp,
	  bool include_reverse_attr )
{
    min::unsptr flag_count = 0;
    const min::gen zero_cc =
	min::new_control_code_gen ( 0 );
    for ( min::gen c = min::current ( lp );
	  ! min::is_list_end ( c );
	  c = min::next ( lp ) )
    {
#   	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    if ( min::is_sublist ( c ) )
	    {
		c = min::next ( lp );
		if ( min::is_list_end ( c ) )
		    break;
	    }
#   	    endif
	if ( min::is_sublist ( c ) )
	{
	    if ( include_reverse_attr )
		info.reverse_attr_count =
		    count_reverse_attrs ( lp );
	}
	else if ( min::is_control_code ( c ) )
	{
	    min::unsptr shift =
		flag_count * min::VSIZE;
	    if ( shift < 64 )
		info.flags |=
		    (min::uns64)
		    MUP::control_code_of ( c )
		    <<
		    shift;
	    ++ flag_count;
	    if ( c != zero_cc )
		info.flag_count = flag_count;
	}
	else if ( ++ info.value_count == 1 )
	    info.value = c;
	else
	    info.value = min::MULTI_VALUED();
    }

    return (    info.value_count > 0
	     || info.flag_count > 0
	     || info.reverse_attr_count > 0 );
}

// Called with current(lp) equal to attribute-/node-
// descriptor for attribute.  Compute counts for
// attribute in info, and also the value and flags
// members.  Does NOT set info.name.  Return true if
// some count is non-zero and false if all counts are
// zero.
//
static bool compute_counts
	( min::attr_info & info,
	  min::list_ptr & lp,
	  bool include_reverse_attr )
{
    info.value_count = 0;
    info.flag_count = 0;
    info.reverse_attr_count = 0;
    info.value = min::NONE();
    info.flags = 0;

    min::gen c = min::current ( lp );
    if ( ! min::is_sublist ( c ) )
    {
        ++ info.value_count;
	info.value = c;
	return true;
    }
    else if ( c == min::EMPTY_SUBLIST() )
        return false;
    else
    {
        min::list_ptr lpv
	    ( min::obj_vec_ptr_of ( lp ) );
	min::start_sublist ( lpv, lp );
	return min::internal::compute_attr_info
	    ( info, lpv, include_reverse_attr );
    }
}

# if MIN_ALLOW_PARTIAL_ATTR_LABELS
    // Compute labels associated with the children of
    // the node whose node-descriptor is pointed at
    // by lp.  The node label is
    //		components[0 .. depth-1].
    //
    // Output is stored in out[m] if m < n and m is
    // incremented.
    //
    static void compute_children
	( min::list_ptr & lp,
	  min::gen * components, min::unsptr depth,
	  min::attr_info * out, min::unsptr n,
	  min::unsptr & m,
	  bool include_reverse_attr )
    {
	min::gen c = min::current ( lp );
	if ( ! min::is_sublist ( c ) ) return;
	min::list_ptr lpv
	    ( min::obj_vec_ptr_of ( lp ) );
	min::start_sublist ( lpv, lp );
	for ( c = min::current ( lpv );
		 ! min::is_list_end ( c )
	      && ! min::is_sublist ( c );
	      c = min::next ( lpv ) );

	if ( ! min::is_sublist ( c ) ) return;
	if ( c == min::EMPTY_SUBLIST() ) return;
	start_sublist ( lpv );

	min::gen labvec[depth+1];
	for ( min::unsptr i = 0; i < depth; ++ i )
	    labvec[i] = * components ++;

	min::locatable_gen new_label;
	for ( c = min::current ( lpv );
	      ! min::is_list_end ( c );
	      c = min::next ( lpv ) )
	{
	    labvec[depth] = c;
	    min::next ( lpv );
	    min::attr_info info;
	    if ( compute_counts
	             ( info, lpv,
		       include_reverse_attr ) )
	    {
		info.name = new_label =
		    min::new_lab_gen
			  ( labvec, depth + 1 );
		if ( m < n ) out[m] = info;
		++ m;
	    }
	    compute_children
	        ( lpv, labvec, depth + 1, out, n, m,
		  include_reverse_attr );
	}
    }
# endif

min::unsptr min::attr_info_of
	( min::attr_info * out, min::unsptr n,
	  min::obj_vec_ptr & vp,
	  bool include_reverse_attr,
	  bool include_attr_vec )
{
    min::unsptr m = 0;  // Return value.
    attr_info info;

    min::list_ptr lp ( vp );

    for ( unsptr i = 0;
          i < hash_size_of ( vp );
	  ++ i )
    {
	start_hash ( lp, i );
	for ( min::gen c = current ( lp );
	      ! is_list_end ( c );
	      c = next ( lp ) )
	{
	    info.name = c;
#	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		// If info.name is a label then being a
		// the first component of a one
		// component name it must be embedded
		// in a label.
		if ( is_lab ( info.name ) )
		    info.name =
		        new_lab_gen ( & info.name, 1 );
#	    endif
	    next ( lp );
	    if ( compute_counts
		   ( info, lp, include_reverse_attr ) )
	    {
		if ( m < n ) out[m] = info;
		++ m;
	    }
#	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		compute_children
		    ( lp, & c, 1, out, n, m,
		      include_reverse_attr );
#	    endif
	}
    }

    if ( include_attr_vec )
    for ( unsptr i = 0;
          i < attr_size_of ( vp );
	  ++ i )
    {
	start_attr ( lp, i );
	if ( is_list_end ( current ( lp ) ) )
	    continue;

	info.name = new_num_gen ( i );
	if ( compute_counts
	         ( info, lp, include_reverse_attr ) )
	{
	    if ( m < n ) out[m] = info;
	    ++ m;
	}
#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    compute_children
	        ( lp, & info.name, 1, out, n, m,
		  include_reverse_attr );
#	endif
    }

    return m;
}

template < class vecpt >
min::unsptr min::reverse_attr_info_of
	( min::reverse_attr_info * out, min::unsptr n,
	  MUP::attr_ptr_type < vecpt > & ap )
{
    typedef MUP::attr_ptr_type<vecpt> ap_type;
    min::unsptr m = 0;  // Return value

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::reverse_attr_info_of called before"
	      " locate" );
    case ap_type::LOCATE_FAIL:
        return m;
    }

    min::gen c = update_refresh ( ap.locate_dlp );
    if ( ! is_sublist ( c ) ) return m;
    start_sublist ( ap.lp, ap.locate_dlp );
    for ( c = current ( ap.lp );
             ! is_sublist ( c )
	  && ! is_list_end ( c );
	  c = next ( ap.lp ) );

#   if MIN_ALLOW_PARTIAL_ATTR_LABELS
	if ( ! is_sublist ( c ) ) return m;
        next ( ap.lp );
#   endif
    if ( ! is_sublist ( c ) ) return m;
    start_sublist ( ap.lp );

    list_ptr lpv ( obj_vec_ptr_of ( ap.lp ) );
    reverse_attr_info info;
    for ( c = current ( ap.lp );
          ! is_list_end ( c );
	  c = next ( ap.lp ) )
    {
        info.name = c;
	info.value_count = 0;
	c = next ( ap.lp );
	if ( ! is_sublist ( c ) )
	{
	    ++ info.value_count;
	    info.value = c;
	}
	else if ( c == min::EMPTY_SUBLIST() )
	    continue;
	else
	{
	    start_sublist ( lpv, ap.lp );
	    for ( c = current ( lpv );
	          ! is_list_end ( c );
		  c = next ( lpv ) )
	    {
	        if ( ++ info.value_count == 1 )
		    info.value = c;
		else
		    info.value = min::MULTI_VALUED();
	    }
	}
	if ( m < n ) out[m] = info;
	++ m;
    }

    return m;
}
template min::unsptr min::reverse_attr_info_of
	( min::reverse_attr_info * out, min::unsptr n,
	  min::attr_ptr & ap );
template min::unsptr min::reverse_attr_info_of
	( min::reverse_attr_info * out, min::unsptr n,
	  min::attr_updptr & ap );
template min::unsptr min::reverse_attr_info_of
	( min::reverse_attr_info * out, min::unsptr n,
	  min::attr_insptr & ap );

// Compare function to qsort attr_info packed vector.
//
static int compare_attr_info
	( const void * aip1, const void * aip2 )
{
    min::gen name1 = ( (min::attr_info *) aip1 )->name;
    min::gen name2 = ( (min::attr_info *) aip2 )->name;
    return min::compare ( name1, name2 );
}
void min::sort_attr_info
	( min::attr_info * out, min::unsptr n )
{
    qsort ( out, n, sizeof ( min::attr_info ),
	    compare_attr_info );
}

// Compare function to qsort reverse_attr_info packed
// vector.
//
static int compare_reverse_attr_info
	( const void * aip1, const void * aip2 )
{
    min::gen name1 =
        ( (min::reverse_attr_info *) aip1 )->name;
    min::gen name2 =
        ( (min::reverse_attr_info *) aip2 )->name;
    return min::compare ( name1, name2 );
}
void min::sort_reverse_attr_info
	( min::reverse_attr_info * out, min::unsptr n )
{
    qsort ( out, n, sizeof ( min::reverse_attr_info ),
	    compare_reverse_attr_info );
}

template < class vecpt >
min::gen MINT::update
	( MUP::attr_ptr_type<vecpt> & ap,
	  min::gen v )
{
    typedef MUP::attr_ptr_type<vecpt> ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::update called before locate" );
    case ap_type::LOCATE_FAIL:
	MIN_ABORT
	    ( "min::update called with no previous"
	      " value" );
    case ap_type::REVERSE_LOCATE_FAIL:
    case ap_type::REVERSE_LOCATE_SUCCEED:
    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::update called with reverse name"
	      " other than min::NONE()" );
    default:
	MIN_ABORT
	    ( "min::update called with >= 2 previous"
	      " values" );
    }
}
template min::gen MINT::update
	( min::attr_updptr & ap,
	  min::gen v );
template min::gen MINT::update
	( min::attr_insptr & ap,
	  min::gen v );

// Helper functions for various set and remove
// functions.  See min.h for documentation.
//
void MINT::remove_reverse_attr_value
	( min::attr_insptr & ap,
	  min::obj_vec_insptr & vp )
{
    typedef min::attr_insptr ap_type;

    attr_insptr rap ( vp );
    min::gen v = min::new_stub_gen
        ( MUP::stub_of ( obj_vec_ptr_of ( ap.dlp ) ) );

    min::locate ( rap, ap.reverse_attr_name );
    min::locate_reverse ( rap, ap.attr_name );
    MIN_ASSERT
        (    rap.state
	  == ap_type::REVERSE_LOCATE_SUCCEED,
	  "system programming error" );

    min::gen c = current ( rap.dlp );
    if ( ! is_sublist ( c ) )
    {
        MIN_ASSERT ( c == v,
	             "system programming error" );
	min::update ( rap, min::EMPTY_SUBLIST() );
	return;
    }
    start_sublist ( rap.dlp );
    for ( c = current ( rap.dlp );
          c != v && ! is_list_end ( c );
	  c = next ( rap.dlp ) );

    MIN_ASSERT ( c == v,
                 "system programming error" );
    min::remove ( rap.dlp, 1 );
}
    
void MINT::remove_reverse_attr_value
	( min::attr_insptr & ap,
          min::gen v )
{
    obj_vec_insptr & apvp =
        obj_vec_ptr_of ( ap.dlp );
    min::stub * aps = MUP::stub_of ( apvp );
    const min::stub * s = min::stub_of ( v );
    if ( s == aps )
    {
        if ( ap.attr_name == ap.reverse_attr_name )
	    return;
	MINT::remove_reverse_attr_value ( ap, apvp );
    }
    else
    {
        obj_vec_insptr vp ( v );
	MINT::remove_reverse_attr_value ( ap, vp );
    }
}

void MINT::add_reverse_attr_value
	( min::attr_insptr & ap,
	  min::obj_vec_insptr & vp )
{
    typedef min::attr_insptr ap_type;

    attr_insptr rap ( vp );
    min::gen v = min::new_stub_gen
        ( MUP::stub_of ( obj_vec_ptr_of ( ap.dlp ) ) );

    min::locate ( rap, ap.reverse_attr_name );
    if ( rap.state != ap_type::LOCATE_NONE )
        attr_create ( rap, min::EMPTY_SUBLIST() );
    min::locate_reverse ( rap, ap.attr_name );
    if ( rap.state != ap_type::REVERSE_LOCATE_SUCCEED )
    {
        reverse_attr_create ( rap, v );
	return;
    }
    MIN_ASSERT
        (    rap.state
	  == ap_type::REVERSE_LOCATE_SUCCEED,
	  "system programming error" );

    min::gen c = current ( rap.dlp );
    if ( ! is_sublist ( c ) )
    {
	min::update ( rap.dlp, min::EMPTY_SUBLIST() );
	start_sublist ( rap.dlp );
	insert_reserve ( rap.dlp, 1, 2 );
	min::gen elements[2] = { c, v };
	insert_before ( rap.dlp, elements, 2 );
	return;
    }
    start_sublist ( rap.dlp );
    insert_reserve ( rap.dlp, 1, 1 );
    min::gen elements[1] = { v };
    insert_before ( rap.dlp, elements, 1 );
}
    
void MINT::add_reverse_attr_value
	( min::attr_insptr & ap,
          min::gen v )
{
    obj_vec_insptr & apvp =
        obj_vec_ptr_of ( ap.dlp );
    min::stub * aps = MUP::stub_of ( apvp );
    const min::stub * s = min::stub_of ( v );
    if ( s == aps )
    {
        if ( ap.attr_name == ap.reverse_attr_name )
	    return;
	MINT::add_reverse_attr_value ( ap, apvp );
	insert_refresh ( ap.locate_dlp );
	insert_refresh ( ap.dlp );
	insert_refresh ( ap.lp );
    }
    else
    {
        obj_vec_insptr vp ( v );
	MINT::add_reverse_attr_value ( ap, vp );
    }
}

void MINT::set
	( min::attr_insptr & ap,
	  const min::gen * in, min::unsptr n )
{
    typedef min::attr_insptr ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::set called before locate" );

    case ap_type::LOCATE_ANY:

	if ( n > 0 )
	    MIN_ABORT
		( "min::set called after reverse locate"
		  " of min::ANY()" );
	else
	{
	    min::gen c =
	        update_refresh ( ap.locate_dlp );
	    if ( ! is_sublist ( c ) ) return;
	    start_sublist ( ap.dlp, ap.locate_dlp );
	    for ( c = current ( ap.dlp );
		     ! is_sublist ( c )
		  && ! is_list_end ( c );
		  c = next ( ap.dlp ) );

	    if ( ! is_sublist ( c ) ) return;
#   	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		c = next ( ap.dlp );
		if ( ! is_sublist ( c ) ) return;
#   	    endif
	    start_sublist ( ap.dlp );
	    for ( c = current ( ap.dlp );
		  ! is_list_end ( c );
		  c = next ( ap.lp ) )
	    {
	        c = next ( ap.dlp );
		if ( is_sublist ( c ) )
		{
		    start_sublist ( ap.lp, ap.dlp );
		    for ( c = current ( ap.lp );
		          ! is_list_end ( c );
			  c = next ( ap.lp ) )
			remove_reverse_attr_value
			    ( ap, c );
		}
		else
		    remove_reverse_attr_value ( ap, c );

		update ( ap.dlp, min::EMPTY_SUBLIST() );
	    }
	}
	return;

    case ap_type::LOCATE_FAIL:
	if ( n == 0 )
	    return;
	else if ( ap.reverse_attr_name == min::ANY() )
	    MIN_ABORT
		( "min::set called after reverse locate"
		  " of min::ANY()" );
        else if ( n == 1
	          &&
		  ap.reverse_attr_name == min::NONE() )
	{
	    MIN_ASSERT ( is_attr_legal ( * in ),
			 "value cannot legally be an"
			 " attribute value " );
	    MINT::attr_create ( ap, * in );
	    return;
	}
        else
	    MINT::attr_create
	        ( ap, min::EMPTY_SUBLIST() );

	if ( ap.reverse_attr_name == min::NONE() )
	    break;

	MIN_FALLTHROUGH

    case ap_type::REVERSE_LOCATE_FAIL:
	if ( n == 0 )
	    return;
        else if ( n == 1 )
	{
	    MIN_ASSERT ( is_obj ( * in ),
			 "reverse attribute value must"
			 " be an object stub" );
	    MINT::reverse_attr_create ( ap, * in );
	    MINT::add_reverse_attr_value ( ap, * in );
	    return;
	}
        else
	    MINT::reverse_attr_create
	        ( ap, min::EMPTY_SUBLIST() );
	break;
    }

    const min::gen * endin = in + n;
    if ( ap.reverse_attr_name == min::NONE() )
        for ( const min::gen * p = in; p < endin; )
	    MIN_ASSERT ( is_attr_legal ( *p ++ ),
			 "value cannot legally be an"
			 " attribute value " );
    else
        for ( const min::gen * p = in; p < endin; )
	    MIN_ASSERT ( is_obj ( * p ++ ),
			 "reverse attribute value must"
			 " be an object stub" );

    min::gen c = update_refresh ( ap.dlp );

    bool is_reverse =
         (    ap.state
	   == ap_type::REVERSE_LOCATE_SUCCEED );
    if ( n == 0 )
    {
        if ( is_sublist ( c ) )
	{
	    if ( is_empty_sublist ( c ) ) return;

	    if ( is_reverse )
	    {
		start_sublist ( ap.lp, ap.dlp );
		for ( c = current ( ap.lp );
		      ! is_list_end ( c );
		      c = next ( ap.lp ) )
		    remove_reverse_attr_value
		        ( ap, c );
		update ( ap.dlp, min::EMPTY_SUBLIST() );
	    }
	    else
	    {
		start_sublist ( ap.lp, ap.dlp );
		while ( c = current ( ap.lp ),
		           ! is_list_end ( c )
		        && ! is_sublist ( c )
		        && ! is_control_code ( c ) )
		    remove ( ap.lp, 1 );
	    }
	}
	else
	{
	    if ( is_reverse )
		remove_reverse_attr_value
		    ( ap, current ( ap.dlp ) );
	    update ( ap.dlp, min::EMPTY_SUBLIST() );
	}
	return;
    }

    if ( ! is_sublist ( c ) )
    {
	if ( is_reverse )
	    remove_reverse_attr_value ( ap, c );

        if ( n == 1 )
	    update ( ap.dlp, * in );
	else
	{
	    update ( ap.dlp, min::EMPTY_SUBLIST() );
	    start_sublist ( ap.lp, ap.dlp );
	    if ( insert_reserve ( ap.lp, 1, n ) )
	    {
		insert_refresh ( ap.dlp );
		insert_refresh ( ap.locate_dlp );
	    }
	    insert_before ( ap.lp, in, n );
	}
    }
    else
    {
    	start_sublist ( ap.lp, ap.dlp );
	unsptr k = 0;
	for ( c = current ( ap.lp );
	         n > k
	      && ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c );
	      c = next ( ap.lp ) )
	{
	    if ( is_reverse )
		remove_reverse_attr_value ( ap, c );
	    update ( ap.lp, in[k++] );
	}
	if ( n > k )
	{
	    if ( insert_reserve ( ap.lp, 1, n - k ) )
	    {
		insert_refresh ( ap.dlp );
		insert_refresh ( ap.locate_dlp );
	    }
	    insert_before ( ap.lp, in + k, n - k );
	}
	else if (    ! is_list_end ( c )
	          && ! is_sublist ( c )
		  && ! is_control_code ( c ) )
	{
	    do
	    {
		if ( is_reverse )
		    remove_reverse_attr_value ( ap, c );
		remove ( ap.lp, 1 );
		c = current ( ap.lp );
	    } while (    ! is_list_end ( c )
	              && ! is_sublist ( c )
		      && ! is_control_code ( c ) );
	}
    }
    if ( is_reverse ) while ( n -- )
	MINT::add_reverse_attr_value ( ap, * in ++ );
}

void min::add_to_set
	( min::attr_insptr & ap,
	  const min::gen * in, min::unsptr n )
{
    typedef min::attr_insptr ap_type;

    if ( n == 0 ) return;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::add_to_set called before"
	      " locate" );

    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::add_to_set called after"
		  " reverse locate of min::ANY()" );

    case ap_type::LOCATE_FAIL:
	if ( ap.reverse_attr_name == min::ANY() )
	    MIN_ABORT
		( "min::add_to_set called after"
		  " reverse locate of min::ANY()" );
	MIN_FALLTHROUGH

    case ap_type::REVERSE_LOCATE_FAIL:
	add_to_multiset ( ap, in, n );
	return;
    }

    min::gen c = update_refresh ( ap.dlp );

    if ( ! is_sublist ( c ) )
    {
        min::gen additions[n];
	unsptr m = 0;
	while ( n -- )
	{
	    min::gen v = * in ++;
	    if ( c != v ) additions[m++] = v;
	}
	if ( m > 0 ) add_to_multiset
	                 ( ap, additions, m );
    }
    else
    {
	// Copy input to additions, and then replace
	// every value already in multiset by
	// min::NONE().  Lastly compact additions and
	// call add_to_multiset.
	//
        min::gen additions[n];
	unsptr m = 0;
	while ( n -- ) additions[m++] = * in ++;
	start_sublist ( ap.lp, ap.dlp );
	for ( c = current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c );
	      c = next ( ap.lp ) )
	for ( unsptr i = 0; i < m; ++ i )
	{
	    if ( additions[i] == c )
	        additions[i] = min::NONE();
	}
	unsptr j = 0;
	for ( unsptr k = 0; k < m; ++ k )
	{
	    if ( additions[k] != min::NONE() )
	        additions[j++] = additions[k];
	}
	add_to_multiset ( ap, additions, j );
    }
}

void min::add_to_multiset
	( min::attr_insptr & ap,
	  const min::gen * in, unsptr n )
{
    typedef min::attr_insptr ap_type;

    if ( n == 0 ) return;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::add_to_multiset called before"
	      " locate" );

    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::add_to_multiset called after"
		  " reverse locate of min::ANY()" );

    case ap_type::LOCATE_FAIL:
	if ( ap.reverse_attr_name == min::ANY() )
	    MIN_ABORT
		( "min::add_to_multiset called after"
		  " reverse locate of min::ANY()" );
        else if ( n == 1
	          &&
		  ap.reverse_attr_name == min::NONE() )
	{
	    MIN_ASSERT ( is_attr_legal ( * in ),
			 "value cannot legally be an"
			 " attribute value " );
	    MINT::attr_create ( ap, * in );
	    return;
	}
        else
	    MINT::attr_create
	        ( ap, min::EMPTY_SUBLIST() );

	if ( ap.reverse_attr_name == min::NONE() )
	    break;

	MIN_FALLTHROUGH

    case ap_type::REVERSE_LOCATE_FAIL:
        if ( n == 1 )
	{
	    MIN_ASSERT ( is_obj ( * in ),
			 "reverse attribute value must"
			 " be an object stub" );
	    MINT::reverse_attr_create ( ap, * in );
	    MINT::add_reverse_attr_value ( ap, * in );
	    return;
	}
        else
	    MINT::reverse_attr_create
	        ( ap, min::EMPTY_SUBLIST() );
	break;
    }

    const min::gen * endin = in + n;
    if ( ap.reverse_attr_name == min::NONE() )
        for ( const min::gen * p = in; p < endin; )
	    MIN_ASSERT ( is_attr_legal ( *p ++ ),
			 "value cannot legally be an"
			 " attribute value " );
    else
        for ( const min::gen * p = in; p < endin; )
	    MIN_ASSERT ( is_obj ( * p ++ ),
			 "reverse attribute value must"
			 " be an object stub" );

    min::gen c = update_refresh ( ap.dlp );

    if ( ! is_sublist ( c ) )
    {
	update ( ap.dlp, min::EMPTY_SUBLIST() );
	start_sublist ( ap.lp, ap.dlp );
	if ( insert_reserve ( ap.lp, 2, n + 1 ) )
	{
	    insert_refresh ( ap.dlp );
	    insert_refresh ( ap.locate_dlp );
	}
	min::gen element[1] = { c };
	insert_before ( ap.lp, element, 1 );
	insert_after ( ap.lp, in, n );
    }
    else
    {
    	start_sublist ( ap.lp, ap.dlp );
	if ( insert_reserve ( ap.lp, 1, n ) )
	{
	    insert_refresh ( ap.dlp );
	    insert_refresh ( ap.locate_dlp );
	}
	insert_before ( ap.lp, in, n );
    }

    if (    ap.state
	 == ap_type::REVERSE_LOCATE_SUCCEED )
	while ( n -- )
	    MINT::add_reverse_attr_value
	        ( ap, * in ++ );
}

min::unsptr min::remove_one
	( min::attr_insptr & ap,
	  const min::gen * in, min::unsptr n )
{
    typedef min::attr_insptr ap_type;

    if ( n == 0 ) return 0;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::remove_one called before"
	      " locate" );

    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::remove_one called after"
		  " reverse locate of min::ANY()" );

    case ap_type::LOCATE_FAIL:
	if ( ap.reverse_attr_name == min::ANY() )
	    MIN_ABORT
		( "min::remove_one called after"
		  " reverse locate of min::ANY()" );
	MIN_FALLTHROUGH

    case ap_type::REVERSE_LOCATE_FAIL:
	return 0;
    }

    min::gen c = update_refresh ( ap.dlp );

    bool is_reverse =
         (    ap.state
	   == ap_type::REVERSE_LOCATE_SUCCEED );
    if ( ! is_sublist ( c ) )
    {
	while ( n -- )
	{
	    if ( * in ++ == c )
	    {
		if ( is_reverse )
		    MINT::remove_reverse_attr_value
		        ( ap, c );
	        update ( ap.dlp, min::EMPTY_SUBLIST() );
		return 1;
	    }
	}
	return 0;
    }
    else
    {
	// Copy input to removals, and then replace
	// every value already in removed by
	// min::NONE().  If we have removed each input
	// once, we can terminate early.
	//
        min::gen removals[n];
	for ( unsptr j = 0; j < n; )
	    removals[j++] = * in ++;

	unsptr result = 0;
	unsptr i;
	start_sublist ( ap.lp, ap.dlp );
	for ( c = current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c )
	      && result < n;
	    )
	{
	    for ( i = 0; i < n; ++ i )
	    {
		if ( removals[i] == c )
		    break;
	    }
	    if ( i < n )
	    {
		if ( is_reverse )
		    MINT::remove_reverse_attr_value
			( ap, c );
		remove ( ap.lp, 1 );
		removals[i] = min::NONE();
		++ result;
		c = current ( ap.lp );
	    }
	    else c = next ( ap.lp );
	}
        return result;
    }
}

min::unsptr min::remove_all
	( min::attr_insptr & ap,
	  const min::gen * in, min::unsptr n )
{
    typedef min::attr_insptr ap_type;

    if ( n == 0 ) return 0;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::remove_all called before"
	      " locate" );

    case ap_type::LOCATE_ANY:
	MIN_ABORT
	    ( "min::remove_all called after"
		  " reverse locate of min::ANY()" );

    case ap_type::LOCATE_FAIL:
	if ( ap.reverse_attr_name == min::ANY() )
	    MIN_ABORT
		( "min::remove_all called after"
		  " reverse locate of min::ANY()" );
	MIN_FALLTHROUGH

    case ap_type::REVERSE_LOCATE_FAIL:
	return 0;
    }

    min::gen c = update_refresh ( ap.dlp );

    bool is_reverse =
         (    ap.state
	   == ap_type::REVERSE_LOCATE_SUCCEED );
    if ( ! is_sublist ( c ) )
    {
	while ( n -- )
	{
	    if ( * in ++ == c )
	    {
		if ( is_reverse )
		    MINT::remove_reverse_attr_value
		        ( ap, c );
	        update ( ap.dlp, min::EMPTY_SUBLIST() );
		return 1;
	    }
	}
	return 0;
    }
    else
    {
	unsptr result = 0;
	unsptr i;
	start_sublist ( ap.lp, ap.dlp );
	for ( c = current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_sublist ( c )
	      && ! is_control_code ( c );
	    )
	{
	    for ( i = 0; i < n; ++ i )
	    {
		if ( in[i] == c )
		    break;
	    }
	    if ( i < n )
	    {
		if ( is_reverse )
		    MINT::remove_reverse_attr_value
			( ap, c );
		remove ( ap.lp, 1 );
		++ result;
		c = current ( ap.lp );
	    }
	    else c = next ( ap.lp );
	}
        return result;
    }
}

void MINT::set_flags
	( min::attr_insptr & ap,
	  const min::gen * in, unsigned n )
{
    typedef min::attr_insptr ap_type;

    for ( unsigned i = 0; i < n; ++ i )
        MIN_ASSERT ( is_control_code ( in[i] ),
	             "vector argument element is not"
		     " control code" );

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::set_flags called before locate" );
    case ap_type::LOCATE_FAIL:
    	    MINT::attr_create
	              ( ap, min::EMPTY_SUBLIST() );
    }

    min::gen c = update_refresh ( ap.locate_dlp );
    start_copy ( ap.lp, ap.locate_dlp );
    if ( ! is_sublist ( c ) )
    {
        if ( n == 0 ) return;

        if ( insert_reserve ( ap.lp, 2, n + 1 ) )
	{
	    insert_refresh ( ap.dlp );
	    insert_refresh ( ap.locate_dlp );
	}
	update ( ap.lp, min::EMPTY_SUBLIST() );
	start_sublist ( ap.lp );
	min::gen element[1] = { c };
	insert_before ( ap.lp, element, 1 );
	insert_after ( ap.lp, in, n );
    }
    else
    {
        start_sublist ( ap.lp );
	const min::gen zero_cc =
	    new_control_code_gen ( 0 );

	for ( c = current ( ap.lp );
	         ! is_list_end ( c )
	      && ! is_control_code ( c );
	      c = next ( ap.lp ) );

	for ( ; is_control_code ( c );
	        c = next ( ap.lp ) )
	{
	    if ( n > 0 )
	    {
		update ( ap.lp, * in ++ );
		-- n;
	    }
	    else
		update ( ap.lp, zero_cc );
	}
	MIN_ASSERT ( c == min::LIST_END(),
	             "system programming error" );

	if ( n > 0 )
	{
	    if ( insert_reserve ( ap.lp, 1, n ) )
	    {
		insert_refresh ( ap.dlp );
		insert_refresh ( ap.locate_dlp );
	    }
	    insert_before ( ap.lp, in, n );
	}
    }
}

// Appends control codes to the current attribute's
// existing list of control codes.  ap.lp points at
// place to do insertion.
//
void MINT::set_more_flags
	( min::attr_insptr & ap,
	  const min::gen * in, unsigned n )
{
    for ( unsigned i = 0; i < n; ++ i )
        MIN_ASSERT ( is_control_code ( in[i] ),
	             "vector argument element is not"
		     " control code" );

    if ( insert_reserve ( ap.lp, 1, n ) )
    {
	insert_refresh ( ap.dlp );
	insert_refresh ( ap.locate_dlp );
    }
    insert_before ( ap.lp, in, n );
}

void MINT::set_some_flags
	( min::attr_insptr & ap,
	  const min::gen * in, unsigned n )
{
    typedef attr_insptr ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::set_some_flags called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::set_some_flags" );
    }
}

void MINT::clear_some_flags
	( min::attr_insptr & ap,
	  const min::gen * in, unsigned n )
{
    typedef attr_insptr ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::clear_some_flags called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to"
	      " min::clear_some_flags" );
    }
}

void MINT::flip_some_flags
	( min::attr_insptr & ap,
	  const min::gen * in, unsigned n )
{
    typedef attr_insptr ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::flip_some_flags called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::flip_some_flags" );
    }
}

// n is the flag number within otherwise 0 control codes
// that are to be added to the end of the attribute-/
// node-descriptor.
//
bool MINT::set_flag
	( min::attr_insptr & ap,
	  unsigned n )
{
    typedef attr_insptr ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::set_flag called before locate" );
    case ap_type::LOCATE_FAIL:
    	    MINT::attr_create
	              ( ap, min::EMPTY_SUBLIST() );
    }

    min::gen elements[n/VSIZE + 2];
    unsptr j = 0;
    min::gen c = current ( ap.locate_dlp );
    if ( ! is_sublist ( c ) )
    {
        elements[j++] = c;
	update ( ap.locate_dlp, min::EMPTY_SUBLIST() );
    }
    start_sublist ( ap.lp, ap.locate_dlp );
    for ( c = current ( ap.lp );
          ! is_list_end ( c );
	  c = next ( ap.lp ) );

    unsigned base = 0;
    while ( base <= n )
    {
        unsigned next = base + VSIZE;
	if ( n < next )
	{
	    elements[j++] = new_control_code_gen
	        ( (unsgen) 1 << ( n - base ) );
	    break;
	}
	else
	    elements[j++] = new_control_code_gen ( 0 );

	base = next;
    }
    if ( insert_reserve ( ap.lp, 1, j ) )
    {
        insert_refresh ( ap.locate_dlp );
        insert_refresh ( ap.dlp );
    }
    insert_before ( ap.lp, elements, j );
    return false;
}

bool MINT::clear_flag
	( min::attr_insptr & ap,
	  unsigned n )
{
    typedef attr_insptr ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::clear_flag called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::clear_flag" );
    }
    return false;
}

bool MINT::flip_flag
	( min::attr_insptr & ap,
	  unsigned n )
{
    typedef attr_insptr ap_type;

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::flip_flag called before"
	      " locate" );
    default:
	MIN_ABORT
	    ( "abnormal call to min::flip_flag" );
    }
    return false;
}

// Short-cut functions:
//
min::gen min::get
    ( min::gen obj, min::gen attr )
{
    if ( ! min::is_obj ( obj ) ) return min::NONE();
    min::obj_vec_ptr vp ( obj );
    min::attr_ptr ap ( vp );
    min::locate ( ap, attr );
    return min::get ( ap );
}
min::unsptr min::get
    ( min::gen * out, min::unsptr n,
      min::gen obj, min::gen attr )
{
    if ( ! min::is_obj ( obj ) ) return 0;
    min::obj_vec_ptr vp ( obj );
    min::attr_ptr ap ( vp );
    min::locate ( ap, attr );
    return min::get ( out, n, ap );
}
bool min::test_flag
    ( min::gen obj, min::gen attr, unsigned n )
{
    min::obj_vec_ptr vp ( obj );
    min::attr_ptr ap ( vp );
    min::locate ( ap, attr );
    return min::test_flag ( ap, n );
}
unsigned min::get_flags
    ( min::gen * out, min::unsptr n,
      min::gen obj, min::gen attr )
{
    min::obj_vec_ptr vp ( obj );
    min::attr_ptr ap ( vp );
    min::locate ( ap, attr );
    return min::get_flags ( out, n, ap );
}
min::gen min::update
    ( min::gen obj, min::gen attr, min::gen v )
{
    min::obj_vec_updptr vp ( obj );
    min::attr_updptr ap ( vp );
    min::locate ( ap, attr );
    return min::update ( ap, v );
}
void min::set
    ( min::gen obj, min::gen attr, min::gen v )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::set ( ap, v );
}
void min::set
    ( min::gen obj, min::gen attr,
      const min::gen * in, min::unsptr n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::set ( ap, in, n );
}
void min::add_to_set
    ( min::gen obj, min::gen attr, min::gen v )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::add_to_set ( ap, v );
}
void min::add_to_set
    ( min::gen obj, min::gen attr,
      const min::gen * in, min::unsptr n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::add_to_set ( ap, in, n );
}
void min::add_to_multiset
    ( min::gen obj, min::gen attr, min::gen v )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::add_to_multiset ( ap, v );
}
void min::add_to_multiset
    ( min::gen obj, min::gen attr,
      const min::gen * in, min::unsptr n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::add_to_multiset ( ap, in, n );
}
min::unsptr min::remove_one
    ( min::gen obj, min::gen attr, min::gen v )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    return min::remove_one ( ap, v );
}
min::unsptr min::remove_one
    ( min::gen obj, min::gen attr,
      const min::gen * in, min::unsptr n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    return min::remove_one ( ap, in, n );
}
min::unsptr min::remove_all
    ( min::gen obj, min::gen attr, min::gen v )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    return min::remove_all ( ap, v );
}
min::unsptr min::remove_all
    ( min::gen obj, min::gen attr,
      const min::gen * in, min::unsptr n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    return min::remove_all ( ap, in, n );
}
void min::set_flags
    ( min::gen obj, min::gen attr,
      const min::gen * in, min::unsptr n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::set_flags ( ap, in, n );
}
void min::set_some_flags
    ( min::gen obj, min::gen attr,
      const min::gen * in, min::unsptr n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::set_some_flags ( ap, in, n );
}
void min::clear_some_flags
    ( min::gen obj, min::gen attr,
      const min::gen * in, min::unsptr n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::clear_some_flags ( ap, in, n );
}
void min::flip_some_flags
    ( min::gen obj, min::gen attr,
      const min::gen * in, min::unsptr n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    min::flip_some_flags ( ap, in, n );
}
bool min::set_flag
    ( min::gen obj, min::gen attr, unsigned n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    return min::set_flag ( ap, n );
}
bool min::clear_flag
    ( min::gen obj, min::gen attr, unsigned n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    return min::clear_flag ( ap, n );
}
bool min::flip_flag
    ( min::gen obj, min::gen attr, unsigned n )
{
    min::obj_vec_insptr vp ( obj );
    min::attr_insptr ap ( vp );
    min::locate ( ap, attr );
    return min::flip_flag ( ap, n );
}

// Graph Typed Objects
// ----- ----- -------

min::locatable_gen min::standard_varname;

static const int GTYPE_ERROR = -1e9;

int gtype_error
	( min::gen parent, const char * message )
{
    min::init ( min::error_message )
	<< min::set_max_depth ( 3 )
        << "ERROR CREATING GRAPH TYPE: " << message
	<< " in "
	<< min::save_indent
	<< min::set_indent ( 4 )
	<< min::indent;

    if ( parent == min::MISSING() )
        min::error_message << "top level expression";
    else
        min::error_message << min::pgen ( parent );

    min::error_message
	<< min::eol
	<< min::restore_indent;

    return GTYPE_ERROR;
}

// Stack of objects that are ancestors of the
// current object.  Used to check for cycles.
//
struct gtype_stack
{
    min::gen gtype;
    gtype_stack * previous;
};

// Element is the element of parent that is to be
// processed.  If it is an index, return the index
// value.  If it points at a non-index non-object or a
// public object that is not a graph type, return 0.
// If it points at an object that is already a graph
// type, return the maximum index in this graph type.
// If it is a variable, convert the variable to an index
// and return MINUS the index.  If it points at an
// object should become a graph type, make the object
// a graph type and return the maximum index found in
// this graph type.  If it points at a non-public
// object that is the root of an acyclic graph that
// contains no indices, make the object public and
// return 0.
//
// Returns GTYPE_ERROR if there is an error, and puts
// an error message in min::error_message.
//
static int make_gtype
	( min::gen element,
	  min::gen parent,
	  gtype_stack * stackp,
	  min::unsptr max_attributes,
	  min::packed_vec_insptr<min::gen> vartab,
	  min::gen varname )
{
    min::obj_vec_ptr vp ( element );

    if ( ! vp )
    {
	if ( min::is_index ( element ) )
	{
	    int index = min::index_of ( element);
	    if ( index == 0 )
		return gtype_error
		    ( parent, "attribute or element"
		              " has 0 index value" );
	    else if (   index
		      > MIN_CONTEXT_SIZE_LIMIT )
		return gtype_error
		    ( parent, "attribute or element"
		              " has too large"
			      " index value" );
	    else return index;
	}
	else
	    return 0;
    }

    if ( min::public_flag_of ( vp ) )
    {
        if ( min::gtype_flag_of ( vp ) )
	    return (int) min::int_of
	        ( min::var ( vp, 0 ) );
	else
	    return 0;
    }

    bool found = false;
    for ( gtype_stack * sp = stackp;
	  ! found && sp; sp = sp->previous )
    {
	found = ( sp->gtype == element );
    }
    if ( found )
	return gtype_error
	    ( parent, "graph type is cyclic" );
	    // printer->print_format.max_depth
	    // is set to 3 for error messages.

    min::attr_ptr ap ( vp );
    min::attr_info info[max_attributes];
    min::unsptr count = min::attr_info_of
        ( info, max_attributes, ap );
	// We do not include attribute vector here.

    if ( count > max_attributes )
    {
        vp = min::NULL_STUB;
        return make_gtype
	    ( element, parent, stackp, count, vartab,
	      varname );
    }

    // Object that might become a graph type or public
    // object or might be a variable.
    //
    int max_index = 0;
    gtype_stack stack = { element, stackp };
    min::gen type = min::NONE();
    for ( min::unsptr i = 0; i < count; ++ i )
    {
        min::attr_info & ai = info[i];
	if ( ai.value_count > 1 )
	    return gtype_error
	        ( element, "attribute has more than one"
		           " value" );
	if ( ai.reverse_attr_count > 0 )
	    return gtype_error
	        ( element, "attribute has double arrow"
		           " values" );
	if ( ai.value_count == 0 ) continue;

	if ( ai.name == min::dot_type )
	    type = ai.value;

	int index = make_gtype
	    ( ai.value, element, & stack, 10,
	      vartab, varname );
	if ( index == GTYPE_ERROR ) return index;
	else if ( index < 0 )
	{
	    index = - index;
	    vp = min::NULL_STUB;
	    min::obj_vec_updptr vup ( element );

	    min::attr_updptr aup ( vup );
	    min::locate ( aup, ai.name );
	    min::update
	        ( aup, min::new_index_gen ( index ) );

	    vup = min::NULL_STUB;
	    vp = element;
	}

	if ( index > max_index )
	    max_index = index;
    }

    if (    vartab != min::NULL_STUB
         && type == varname
	 && min::size_of ( vp ) == 1
	 && (    min::is_str ( vp[0] )
	      || min::is_lab ( vp[0] ) ) )
    {
    	min::uns32 i;
	min::uns32 length = vartab->length;
	
	for ( i = 0; i < length; ++ i )
	{
	    if ( vartab[i] == vp[0] ) break;
	}
	if ( i == length )
	    min::push ( vartab ) = vp[0];

	MIN_ASSERT ( i > 0,
	             "vartab[0] not min::MISSING()" );

	return - i;
    }

    // We process vector separately so that
    // max_attributes can be kept small.
    //
    for ( min::unsptr i = 0;
          i < min::size_of ( vp ) ; ++ i )
    {
	int index = make_gtype
	    ( vp[i], element, & stack, 10,
	      vartab, varname );
	if ( index == GTYPE_ERROR ) return index;
	else if ( index < 0 )
	{
	    index = - index;
	    vp = min::NULL_STUB;
	    min::obj_vec_updptr vup ( element );

	    vup[i] = min::new_index_gen ( index );

	    vup = min::NULL_STUB;
	    vp = element;
	}

	if ( index > max_index )
	    max_index = index;
    }

    vp = min::NULL_STUB;
    min::obj_vec_insptr vip ( element );

    min::compact ( vip, max_index > 0, 0, false );
    if ( max_index > 0 )
    {
	min::var ( vip, 0 ) =
	    min::new_num_gen ( max_index + 1 );
	min::set_gtype_flag_of ( vip );
    }
    else
	min::set_public_flag_of ( vip );

    return max_index;
}

min::gen min::new_gtype
	( min::gen gtype,
	  min::packed_vec_insptr<min::gen> vartab,
	  min::gen varname )
{
    gtype_stack stack = { gtype, NULL };
    int index = make_gtype
        ( gtype, min::MISSING(), & stack, 10,
	  vartab, varname );
    if ( index == GTYPE_ERROR )
        return min::ERROR();
    else if ( index < 0 )
        return min::new_index_gen ( - index );
    else
        return gtype;
}

min::gen min::new_context ( min::gen gtype )
{
    min::obj_vec_ptr vp ( gtype );
    MIN_ASSERT ( min::gtype_flag_of ( vp ),
                 "argument to new_context is not"
		 " gtype" );
    min::unsgen index_limit =
        (min::unsgen)
	    min::int_of ( min::var ( vp, 0 ) );

    min::locatable_gen c
        ( min::new_obj_gen
	      ( 0, 0, index_limit, false ) );
    min::obj_vec_insptr vip ( c );

    min::var ( vip, 0 ) = gtype;
    for ( min::unsptr i = 1; i < index_limit; ++ i )
        min::var ( vip, i ) = min::UNDEFINED();
    min::set_context_flag_of ( vip );

    return c;
}

static void graph_type_initialize ( void )
{
    min::standard_varname =
        min::new_str_gen ( "*VARNAME*" );
}

static min::initializer graph_type_initializer
    ( :: graph_type_initialize );


// Printers
// --------

min::locatable_var<min::printer> min::error_message;

const min::support_control
        min::ascii_support_control =
{
    min::IS_ASCII, min::IS_UNSUPPORTED
};

const min::support_control
        min::latin1_support_control =
{
    min::IS_LATIN1 + min::IS_ASCII,
    min::IS_UNSUPPORTED
};

const min::support_control
        min::support_all_support_control =
{
    min::ALL_CHARS, min::IS_UNSUPPORTED
};

const min::display_control
        min::graphic_and_sp_display_control =
{
    min::IS_SP + min::IS_GRAPHIC, 0
};

const min::display_control
        min::graphic_and_hspace_display_control =
{
    min::IS_HSPACE + min::IS_GRAPHIC, 0
};

const min::display_control
        min::graphic_only_display_control =
{
    min::IS_GRAPHIC, 0
};

const min::display_control
        min::graphic_and_vhspace_display_control =
{
    min::IS_GRAPHIC + min::IS_VHSPACE, 0
};

const min::display_control
        min::display_all_display_control =
{
    min::ALL_CHARS, 0
};


const min::break_control
	min::no_auto_break_break_control =
{
    0, 0, 0, 0
};

const min::break_control
	min::break_after_space_break_control =
{
    min::IS_BHSPACE, 0, 0, 0
};

const min::break_control
	min::break_before_all_break_control =
{
    0, min::ALL_CHARS, 0, 0
};

const min::break_control
   min::break_after_hyphens_break_control =
{
    min::IS_BHSPACE, 0,
    min::CONDITIONAL_BREAK, 4
};

static min::char_name_format standard_char_name_format =
{
    (min::ustring) "\x01\x01" "<",
    (min::ustring) "\x01\x01" ">"
};
const min::char_name_format *
	min::standard_char_name_format =
    & ::standard_char_name_format;

const min::line_break min::default_line_break =
{
    0, 0, 72, 4
};

static min::printer space_if_none_pstring
	( min::printer printer )
{
    return min::print_space_if_none ( printer );
}
min::pstring min::space_if_none_pstring =
    & ::space_if_none_pstring;

static min::printer leading_always_pstring
	( min::printer printer )
{
    return min::print_leading_always ( printer );
}
min::pstring min::leading_always_pstring =
    & ::leading_always_pstring;

static min::printer trailing_always_pstring
	( min::printer printer )
{
    return min::print_trailing_always ( printer );
}
min::pstring min::trailing_always_pstring =
    & ::trailing_always_pstring;

static min::printer left_square_angle_space_pstring
	( min::printer printer )
{
    min::print_item ( printer, "[<", 2, 2,
	   min::IS_LEADING + min::IS_GRAPHIC );
    return min::print_space ( printer );
}
min::pstring min::left_square_angle_space_pstring =
    & ::left_square_angle_space_pstring;

static min::printer space_right_angle_square_pstring
	( min::printer printer )
{
    min::print_space_if_none ( printer );
    return min::print_item
        ( printer, ">]", 2, 2,
	  min::IS_TRAILING + min::IS_GRAPHIC );
}
min::pstring min::space_right_angle_square_pstring =
    & ::space_right_angle_square_pstring;

static min::printer left_square_dollar_space_pstring
	( min::printer printer )
{
    min::print_item ( printer, "[$", 2, 2,
	  min::IS_LEADING + min::IS_GRAPHIC );
    return min::print_space ( printer );
}
min::pstring min::left_square_dollar_space_pstring =
    & ::left_square_dollar_space_pstring;

static min::printer space_dollar_right_square_pstring
	( min::printer printer )
{
    min::print_space_if_none ( printer );
    return min::print_item
        ( printer, "$]", 2, 2,
	  min::IS_TRAILING + min::IS_GRAPHIC );
}
min::pstring min::space_dollar_right_square_pstring =
    & ::space_dollar_right_square_pstring;

static min::printer left_curly_right_curly_pstring
	( min::printer printer )
{
    min::print_item
        ( printer, "{", 1, 1,
	  min::IS_LEADING + min::IS_GRAPHIC );
    return min::print_item
        ( printer, "}", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
}
min::pstring min::left_curly_right_curly_pstring =
    & ::left_curly_right_curly_pstring;

static min::printer left_curly_leading_pstring
	( min::printer printer )
{
    min::print_item
	( printer, "{", 1, 1,
	  min::IS_LEADING + min::IS_GRAPHIC );
    return min::print_leading ( printer );
}
min::pstring min::left_curly_leading_pstring =
    & ::left_curly_leading_pstring;

static min::printer trailing_right_curly_pstring
	( min::printer printer )
{
    min::print_trailing ( printer );
    return min::print_item
        ( printer, "}", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
}
min::pstring min::trailing_right_curly_pstring =
    & ::trailing_right_curly_pstring;

static min::printer trailing_vbar_leading_pstring
	( min::printer printer )
{
    min::print_trailing ( printer );
    min::print_item
        ( printer, "|", 1, 1,
	  min::IS_TRAILING + min::IS_LEADING
	                   + min::IS_GRAPHIC );
    return min::print_leading ( printer );
}
min::pstring min::trailing_vbar_leading_pstring =
    & ::trailing_vbar_leading_pstring;

static min::printer trailing_always_colon_space_pstring
	( min::printer printer )
{
    min::print_trailing_always ( printer );
    min::print_item
        ( printer, ":", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    return min::print_space ( printer );
}
min::pstring min::trailing_always_colon_space_pstring =
    & ::trailing_always_colon_space_pstring;

static min::printer trailing_always_comma_pstring
	( min::printer printer )
{
    min::print_trailing_always ( printer );
    return min::print_item
        ( printer, ",", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
}
min::pstring min::trailing_always_comma_pstring =
    & ::trailing_always_comma_pstring;

static min::printer trailing_always_comma_space_pstring
	( min::printer printer )
{
    min::print_trailing_always ( printer );
    min::print_item
        ( printer, ",", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
    return min::print_space ( printer );
}
min::pstring min::trailing_always_comma_space_pstring =
    & ::trailing_always_comma_space_pstring;

static min::printer erase_all_space_colon_pstring
	( min::printer printer )
{
    min::print_erase_space
        ( printer, printer->column );
    return min::print_item
        ( printer, ":", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
}
min::pstring min::erase_all_space_colon_pstring =
    & ::erase_all_space_colon_pstring;

static min::printer erase_all_space_double_colon_pstring
	( min::printer printer )
{
    min::print_erase_space
        ( printer, printer->column );
    return min::print_item
        ( printer, "::", 2, 2,
	  min::IS_TRAILING + min::IS_GRAPHIC );
}
min::pstring min::erase_all_space_double_colon_pstring =
    & ::erase_all_space_double_colon_pstring;

static min::printer no_space_pstring
	( min::printer printer )
{
    min::print_item
        ( printer, "no", 2, 2, min::IS_GRAPHIC );
    return min::print_space ( printer );
}
min::pstring min::no_space_pstring =
    & ::no_space_pstring;

static min::printer space_equal_space_pstring
	( min::printer printer )
{
    min::print_space_if_none ( printer );
    min::print_chars ( printer, "=", 1, 1 );
    return min::print_space ( printer );
}
min::pstring min::space_equal_space_pstring =
    & ::space_equal_space_pstring;

static min::printer left_curly_star_space_pstring
	( min::printer printer )
{
    min::print_item
        ( printer, "{*", 2, 2,
	  min::IS_LEADING + min::IS_GRAPHIC );
    return min::print_space ( printer );
}
min::pstring min::left_curly_star_space_pstring =
    & ::left_curly_star_space_pstring;

static min::printer space_star_right_curly_pstring
	( min::printer printer )
{
    min::print_space_if_none ( printer );
    return min::print_item
        ( printer, "*}", 2, 2,
	  min::IS_TRAILING + min::IS_GRAPHIC );
}
min::pstring min::space_star_right_curly_pstring =
    & ::space_star_right_curly_pstring;

static min::printer
    left_square_leading_always_pstring
	( min::printer printer )
{
    min::print_item
        ( printer, "[", 1, 1,
	  min::IS_LEADING + min::IS_GRAPHIC );
    return min::print_leading_always ( printer );
}
min::pstring
    	min::left_square_leading_always_pstring =
    & ::left_square_leading_always_pstring;

static min::printer
    trailing_always_right_square_pstring
	( min::printer printer )
{
    min::print_trailing_always ( printer );
    return min::print_item
        ( printer, "]", 1, 1,
	  min::IS_TRAILING + min::IS_GRAPHIC );
}
min::pstring
    	min::trailing_always_right_square_pstring =
    & ::trailing_always_right_square_pstring;

// const min::print_format min::default_print_format
// defined below after compact_gen_format.

static min::packed_vec<min::line_break>
    line_break_stack_type
        ( "min::line_break_stack_type" );

static min::packed_vec<min::print_format>
    print_format_stack_type
        ( "min::print_format_stack_type" );

static min::uns32 printer_stub_disp[5] =
    { min::DISP ( & min::printer_struct::file ),
      min::DISP ( & min::printer_struct
                       ::line_break_stack ),
      min::DISP ( & min::printer_struct
                       ::print_format_stack ),
      min::DISP ( & min::printer_struct
                       ::id_map ),
      min::DISP_END };

static min::packed_struct<min::printer_struct>
    printer_type ( "min::printer_type",
                   NULL, ::printer_stub_disp );

min::printer min::init
	( min::ref<min::printer> printer,
	  min::file file )
{
    if ( printer == NULL_STUB )
    {
        printer = ::printer_type.new_stub();
	line_break_stack_ref ( printer ) =
	    ::line_break_stack_type.new_stub();
	print_format_stack_ref ( printer ) =
	    ::print_format_stack_type.new_stub();
    }
    else
    {
        min::pop ( printer->line_break_stack,
	           printer->line_break_stack->length );
        min::pop ( printer->print_format_stack,
	           printer->print_format_stack
		          ->length );
    }

    id_map_ref(printer) = min::NULL_STUB;

    if ( file != NULL_STUB )
        file_ref(printer) = file;
    else
	init_input ( file_ref(printer) );

    printer->column = 0;
    printer->line_break = min::default_line_break;
    printer->print_format = min::default_print_format;
    printer->file->op_flags =
	printer->print_format.op_flags;
    printer->state = 0;
    printer->last_str_class = 0;
    * (min::uns32 *) & printer->depth = 0;

    return printer;
}

min::printer min::init_ostream
	( min::ref<min::printer> printer,
	  std::ostream & ostream )
{
    init ( printer );
    init_ostream ( file_ref(printer), ostream );
    return printer << min::flush_on_eol;
}

inline void push
	( min::packed_vec_insptr<char> buffer,
	  min::ustring str )
{
    min::uns32 length = min::ustring_length ( str );
    const char * p = min::ustring_chars ( str );
    min::push ( buffer, length, p );
}

static min::uns32 end_line
        ( min::printer printer, min::uns32 op_flags )
{
    // Remove line ending horizontal spaces.
    //
    min::packed_vec_insptr<char> buffer =
        printer->file->buffer;
    min::uns32 offset = buffer->length;
    while ( offset > 0
            &&
	    ( buffer[offset-1] == ' '
	      ||
	      buffer[offset-1] == '\t' ) )
	-- offset;
    min::pop ( buffer, buffer->length - offset );

    // Add displayed eol.
    //
    if ( op_flags & min::DISPLAY_EOL )
    {
        min::Uchar c = min::unicode::SOFTWARE_NL;
	min::ptr<const min::Uchar> p =
	    min::new_ptr<const min::Uchar> ( & c );
	min::uns32 width = (min::uns32) -1;
	min::unsptr length = 1;
	MINT::print_unicode
	    ( printer, op_flags, length, p, width );
    }

    min::end_line ( printer->file );

    min::uns32 width = printer->column;

    printer->column = 0;
    printer->line_break.offset = buffer->length;
    printer->line_break.column = 0;
    printer->state &= min::AFTER_PARAGRAPH
                    + min::AFTER_LINE_TERMINATOR;
    printer->last_str_class = 0;

    return width;
}

min::printer operator <<
	( min::printer printer,
	  const min::op & op )
{
    char buffer[256];
    switch ( op.opcode )
    {
    case min::op::PGEN:
        return min::print_gen
	    ( printer, MUP::new_gen ( op.v1.g ) );
    case min::op::PGEN_FORMAT:
        return min::print_gen
	    ( printer, MUP::new_gen ( op.v1.g ),
	      (const min::gen_format *) op.v2.p );
    case min::op::FLUSH_ONE_ID:
        return min::print_one_id ( printer );
    case min::op::FLUSH_ID_MAP:
        return min::print_id_map ( printer );

    case min::op::PUNICODE1:
    {
        min::unsptr length = 1;
	min::ptr<const min::Uchar> p =
	    min::new_ptr<const min::Uchar>
	        ( & op.v1.u32 );
	return min::print_unicode
	    ( printer, length, p );
    }
    case min::op::PUNICODE2:
    {
	min::unsptr length = op.v1.uptr;
	min::ptr<const min::Uchar> p =
	    MUP::new_ptr<const min::Uchar>
		( (const min::stub *) op.v2.p,
		  op.v3.uptr );
	return min::print_unicode
	    ( printer, length, p );
    }
    case min::op::PINT32:
	{
	    min::unsptr n =
		sprintf ( buffer,
		          (const char *) op.v2.p,
			  op.v1.i32 );
	    return min::print_item
		( printer, buffer, n, n,
		  min::IS_GRAPHIC );
	}
    case min::op::PINT64:
	{
	    min::unsptr n =
		sprintf ( buffer,
		          (const char *) op.v2.p,
			  op.v1.i64 );
	    return min::print_item
		( printer, buffer, n, n,
		  min::IS_GRAPHIC );
	}
    case min::op::PUNS32:
	{
	    min::unsptr n =
		sprintf ( buffer,
		          (const char *) op.v2.p,
			  op.v1.u32 );
	    return min::print_item
		( printer, buffer, n, n,
		  min::IS_GRAPHIC );
	}
    case min::op::PUNS64:
	{
	    min::unsptr n =
		sprintf ( buffer,
		          (const char *) op.v2.p,
			  op.v1.u64 );
	    return min::print_item
		( printer, buffer, n, n,
		  min::IS_GRAPHIC );
	}
    case min::op::PFLOAT64:
	{
	    min::unsptr n =
		sprintf ( buffer,
		          (const char *) op.v2.p,
			  op.v1.f64 );
	    return min::print_item
		( printer, buffer, n, n,
		  min::IS_GRAPHIC );
	}
    case min::op::SET_LINE_LENGTH:
	printer->line_break.line_length = op.v1.u32;
	return printer;
    case min::op::SET_INDENT:
	printer->line_break.indent = op.v1.u32;
	return printer;
    case min::op::PLACE_INDENT:
	if ( op.v1.i32 < 0
	     &&
	       printer->column
	     < (min::uns32) - op.v1.i32 )
	    printer->line_break.indent = 0;
	else
	    printer->line_break.indent =
	        printer->column + op.v1.i32;
	return printer;
    case min::op::ADJUST_INDENT:
	if ( op.v1.i32 < 0
	     &&
	       printer->line_break.indent
	     < (min::uns32) - op.v1.i32 )
	    printer->line_break.indent = 0;
	else
	    printer->line_break.indent += op.v1.i32;
	return printer;
    case min::op::SET_GEN_FORMAT:
	printer->print_format.gen_format =
	    (const min::gen_format *) op.v1.p;
	return printer;
    case min::op::SET_PRINT_OP_FLAGS:
	if ( op.v1.u32 & min::OUTPUT_HTML )
	{
	    MIN_ASSERT
		( printer->file->ostream != NULL,
	          "setting printer OUTPUT_HTML when"
	          " printer->file->ostream = NULL" );
	    min::flush_file ( printer->file );
	}
	printer->print_format.op_flags |= op.v1.u32;
	printer->file->op_flags =
	    printer->print_format.op_flags;
	return printer;
    case min::op::CLEAR_PRINT_OP_FLAGS:
	if ( op.v1.u32 & min::OUTPUT_HTML )
	    min::flush_file ( printer->file );
	printer->print_format.op_flags &= ~ op.v1.u32;
	printer->file->op_flags =
	    printer->print_format.op_flags;
	return printer;
    case min::op::SET_SUPPORT_CONTROL:
        printer->print_format.support_control =
	    * (const min::support_control *) op.v1.p;
	return printer;
    case min::op::SET_DISPLAY_CONTROL:
        printer->print_format.display_control =
	    * (const min::display_control *) op.v1.p;
	return printer;
    case min::op::SET_QUOTED_DISPLAY_CONTROL:
        printer->print_format.quoted_display_control =
	    * (const min::display_control *) op.v1.p;
	return printer;
    case min::op::SET_BREAK_CONTROL:
        printer->print_format.break_control =
	    * (const min::break_control *) op.v1.p;
	return printer;
    case min::op::SET_MAX_DEPTH:
        printer->print_format.max_depth = op.v1.u32;
	return printer;
    case min::op::VERBATIM:
	printer->print_format.op_flags &=
	    ~ min::EXPAND_HT;
	printer->file->op_flags =
	    printer->print_format.op_flags;
	printer->print_format.support_control =
	    min::support_all_support_control;
	printer->print_format.display_control =
	    min::display_all_display_control;
	printer->print_format.break_control =
	    min::no_auto_break_break_control;
	return printer;
    case min::op::SPACE:
	return min::print_space ( printer );
    case min::op::SAVE_LINE_BREAK:
        min::push ( printer->line_break_stack ) =
	    printer->line_break;
	return printer;
    case min::op::SAVE_INDENT:
        if (   printer->state
	     & (   min::AFTER_LEADING
	         | min::AFTER_TRAILING ) )
	{
	    printer->state |= min::AFTER_SAVE_INDENT;
	    return printer;
	}
    save_indent:
        min::push ( printer->line_break_stack ) =
	    printer->line_break;
	printer->line_break.indent = printer->column;
	return printer;
    case min::op::RESTORE_LINE_BREAK:
    case min::op::RESTORE_INDENT:
        MIN_REQUIRE ( ! (   printer->state
	                  & min::AFTER_SAVE_INDENT ) );
	printer->line_break =
	    min::pop ( printer->line_break_stack );
	return printer;
    case min::op::SAVE_PRINT_FORMAT:
        min::push ( printer->print_format_stack ) =
	    printer->print_format;
	return printer;
    case min::op::RESTORE_PRINT_FORMAT:
    restore_print_format:
	printer->print_format =
	    min::pop ( printer->print_format_stack );
	printer->file->op_flags =
	    printer->print_format.op_flags;
	return printer;
    case min::op::BOM:
        min::push ( printer->print_format_stack ) =
	    printer->print_format;
	goto save_indent;
    case min::op::EOM:
	printer->line_break =
	    min::pop ( printer->line_break_stack );
        if ( printer->column != 0 )
	{
	    ::end_line
	        ( printer,
	          printer->print_format.op_flags );
	    if (   printer->print_format.op_flags
		 & min::FLUSH_ON_EOL )
		min::flush_file ( printer->file );
	}

	if (   printer->print_format.op_flags
	     & min::FLUSH_ID_MAP_ON_EOM )
	    min::print_id_map ( printer );

	goto restore_print_format;
    case min::op::EOL_IF_AFTER_INDENT:
        if (    printer->column
	     <= printer->line_break.indent )
	    return printer;
	else
	    goto eol;
    case min::op::BOL:
        if ( printer->column == 0 ) return printer;
	MIN_FALLTHROUGH // to EOL.
    eol:
    case min::op::EOL:
	::end_line
	    ( printer,
	      printer->print_format.op_flags );
	if (   printer->print_format.op_flags
	     & min::FLUSH_ON_EOL )
	    min::flush_file ( printer->file );
	return printer;
    case min::op::FLUSH:
	min::flush_file ( printer->file );
	return printer;
    case min::op::SET_BREAK:
        if (   printer->state
	     & (   min::AFTER_LEADING
	         | min::AFTER_TRAILING ) )
	{
	    printer->state |= min::AFTER_SET_BREAK;
	    return printer;
	}
    set_break:
	printer->line_break.offset =
	    printer->file->buffer->length;
	printer->line_break.column = printer->column;
	printer->state &= ~ min::BREAK_AFTER;
	return printer;
    case min::op::LEFT:
        if (   printer->column
	     <   printer->line_break.column
	       + op.v1.u32 )
	    min::print_space
	        ( printer,
	            printer->line_break.column
	          + op.v1.u32
                  - printer->column );
	goto set_break;
    case min::op::RIGHT:
        if (   printer->column
	     < printer->line_break.column + op.v1.u32 )
	{
	    min::packed_vec_insptr<char> buffer =
	        printer->file->buffer;
	    min::uns32 line_length =
	        printer->line_break.line_length;
	    min::uns32 indent =
	        printer->line_break.indent;

	    min::uns32 offset =
	        printer->line_break.offset;
	    min::uns32 len =
	        buffer->length - offset;
	    min::uns32 n =
	          printer->line_break.column
		+ op.v1.u32 - printer->column;

	    // See if inserting n spaces will put a
	    // non-space character past line_length, and
	    // if yes, call insert_line_break.  Note
	    // that buffer may end in space characters,
	    // which we have to discount.
	    //
	    if (   line_length
	         <   printer->line_break.column
		   + op.v1.u32
		 &&
		 indent < printer->line_break.column )
	    {
	        min::uns32 column = printer->column;
		min::uns32 i = buffer->length;
		while (    i > offset
		        && buffer[i-1] == ' ' )
		    -- column, -- i;

		if (    i > offset
		     && column + n > line_length )
		{
		    MINT::insert_line_break ( printer );
		    offset = printer->line_break.offset;
		}
	    }

	    min::push ( buffer, n );
	    if ( len > 0 )
	        memmove ( ~ & buffer[offset + n],
		          ~ & buffer[offset],
			  len );

	    printer->column += n;
	    while ( n -- ) buffer[offset++] = ' ';
	}
	goto set_break;
    case min::op::RESERVE:
        if (    printer->column + op.v1.u32
	     <= printer->line_break.line_length )
	    return printer;
	MIN_FALLTHROUGH // to INDENT.
    case min::op::INDENT:
        if (   printer->column
	     > printer->line_break.indent )
	    ::end_line
		( printer,
		  printer->print_format.op_flags );
    execute_indent:
	if (   printer->column
	     < printer->line_break.indent )
	    min::print_space
	        ( printer,
		    printer->line_break.indent
		  - printer->column );
	goto set_break;
    case min::op::SPACES_IF_BEFORE_INDENT:
	if (   printer->column
	     < printer->line_break.indent )
	    goto execute_indent;
	else
	    return printer;
    case min::op::SPACE_IF_AFTER_INDENT:
        if (   printer->column
	     > printer->line_break.indent )
	    min::print_space ( printer );
	return printer;
    case min::op::SPACE_IF_NONE:
	return min::print_space_if_none ( printer );
    case min::op::ERASE_SPACE:
	return min::print_erase_space ( printer );
    case min::op::ERASE_ALL_SPACE:
	return min::print_erase_space
	           ( printer, printer->column );
    case min::op::LEADING:
	return print_leading ( printer );
    case min::op::TRAILING:
	return print_trailing ( printer );
    case min::op::LEADING_ALWAYS:
	return print_leading_always ( printer );
    case min::op::TRAILING_ALWAYS:
	return print_trailing_always ( printer );
    case min::op::PPRINTF:
        return printer << (const char *) op.v1.p;
    case min::op::PNOP:
        return printer;
    case min::op::PRINT_ASSERT:
        // For debugging only.
	// Put add hoc MIN_REQUIRE statements here.
        return printer;
    default:
        MIN_ABORT ( "bad min::OPCODE" );
    }
}

const min::op min::save_line_break
    ( min::op::SAVE_LINE_BREAK );
const min::op min::restore_line_break
    ( min::op::RESTORE_LINE_BREAK );
const min::op min::save_indent
    ( min::op::SAVE_INDENT );
const min::op min::restore_indent
    ( min::op::RESTORE_INDENT );
const min::op min::save_print_format
    ( min::op::SAVE_PRINT_FORMAT );
const min::op min::restore_print_format
    ( min::op::RESTORE_PRINT_FORMAT );

const min::op min::eol ( min::op::EOL );
const min::op min::bol ( min::op::BOL );
const min::op min::flush ( min::op::FLUSH );
const min::op min::bom ( min::op::BOM );
const min::op min::eom ( min::op::EOM );
const min::op min::set_break ( min::op::SET_BREAK );
const min::op min::indent ( min::op::INDENT );
const min::op min::eol_if_after_indent
    ( min::op::EOL_IF_AFTER_INDENT );
const min::op min::spaces_if_before_indent
    ( min::op::SPACES_IF_BEFORE_INDENT );
const min::op min::space_if_after_indent
    ( min::op::SPACE_IF_AFTER_INDENT );
const min::op min::space_if_none
    ( min::op::SPACE_IF_NONE );
const min::op min::erase_space
    ( min::op::ERASE_SPACE );
const min::op min::erase_all_space
    ( min::op::ERASE_ALL_SPACE );

const min::op min::disable_line_breaks
    ( min::op::SET_PRINT_OP_FLAGS,
      min::DISABLE_LINE_BREAKS );
const min::op min::nodisable_line_breaks
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::DISABLE_LINE_BREAKS );

const min::op min::expand_ht
    ( min::op::SET_PRINT_OP_FLAGS,
      min::EXPAND_HT );
const min::op min::noexpand_ht
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::EXPAND_HT );

const min::op min::display_eol
    ( min::op::SET_PRINT_OP_FLAGS,
      min::DISPLAY_EOL );
const min::op min::nodisplay_eol
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::DISPLAY_EOL );

const min::op min::display_picture
    ( min::op::SET_PRINT_OP_FLAGS,
      min::DISPLAY_PICTURE );
const min::op min::nodisplay_picture
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::DISPLAY_PICTURE );

const min::op min::display_non_graphic
    ( min::op::SET_PRINT_OP_FLAGS,
      min::DISPLAY_NON_GRAPHIC );
const min::op min::nodisplay_non_graphic
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::DISPLAY_NON_GRAPHIC );

const min::op min::output_html
    ( min::op::SET_PRINT_OP_FLAGS,
      min::OUTPUT_HTML );
const min::op min::nooutput_html
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::OUTPUT_HTML );

const min::op min::flush_on_eol
    ( min::op::SET_PRINT_OP_FLAGS,
      min::FLUSH_ON_EOL );
const min::op min::noflush_on_eol
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::FLUSH_ON_EOL );

const min::op min::flush_id_map_on_eom
    ( min::op::SET_PRINT_OP_FLAGS,
      min::FLUSH_ID_MAP_ON_EOM );
const min::op min::noflush_id_map_on_eom
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::FLUSH_ID_MAP_ON_EOM );

const min::op min::force_space
    ( min::op::SET_PRINT_OP_FLAGS,
      min::FORCE_SPACE );
const min::op min::noforce_space
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::FORCE_SPACE );
const min::op min::disable_str_breaks
    ( min::op::SET_PRINT_OP_FLAGS,
      min::DISABLE_STR_BREAKS );
const min::op min::nodisable_str_breaks
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::DISABLE_STR_BREAKS );
const min::op min::force_pgen
    ( min::op::SET_PRINT_OP_FLAGS,
      min::FORCE_PGEN );
const min::op min::noforce_pgen
    ( min::op::CLEAR_PRINT_OP_FLAGS,
      min::FORCE_PGEN );

const min::op min::leading
    ( min::op::LEADING );
const min::op min::trailing
    ( min::op::TRAILING );
const min::op min::leading_always
    ( min::op::LEADING_ALWAYS );
const min::op min::trailing_always
    ( min::op::TRAILING_ALWAYS );
const min::op min::pnop
    ( min::op::PNOP );

const min::op min::verbatim
    ( min::op::VERBATIM );
const min::op min::space
    ( min::op::SPACE );

const min::op min::ascii
    ( min::op::SET_SUPPORT_CONTROL,
      & min::ascii_support_control );
const min::op min::latin1
    ( min::op::SET_SUPPORT_CONTROL,
      & min::latin1_support_control );
const min::op min::support_all
    ( min::op::SET_SUPPORT_CONTROL,
      & min::support_all_support_control );

const min::op min::graphic_and_sp
    ( min::op::SET_DISPLAY_CONTROL,
      & min::graphic_and_sp_display_control );
const min::op min::graphic_and_hspace
    ( min::op::SET_DISPLAY_CONTROL,
      & min::graphic_and_hspace_display_control );
const min::op min::graphic_only
    ( min::op::SET_DISPLAY_CONTROL,
      & min::graphic_only_display_control );
const min::op min::graphic_and_vhspace
    ( min::op::SET_DISPLAY_CONTROL,
      & min::graphic_and_vhspace_display_control );
const min::op min::display_all
    ( min::op::SET_DISPLAY_CONTROL,
      & min::display_all_display_control );

const min::op min::no_auto_break
    ( min::op::SET_BREAK_CONTROL,
      & min::no_auto_break_break_control );
const min::op min::break_after_space
    ( min::op::SET_BREAK_CONTROL,
      & min::break_after_space_break_control );
const min::op min::break_before_all
    ( min::op::SET_BREAK_CONTROL,
      & min::break_before_all_break_control );
const min::op min::break_after_hyphens
    ( min::op::SET_BREAK_CONTROL,
      & min::
      break_after_hyphens_break_control );

const min::op min::print_assert
    ( min::op::PRINT_ASSERT );

// Called when we are about to insert non-horizontal
// space characters representing a single character
// into the line and the result would exceed line
// length.  Return true if there is no enabled line
// break and false if we installed a line break and
// there MIGHT be another enabled line break.  The
// return value can be used to avoid repeated calls
// to check for break insertion if no break points
// are set between calls.
//
bool MINT::insert_line_break ( min::printer printer )
{
    min::packed_vec_insptr<char> buffer =
        printer->file->buffer;
    min::line_break_stack line_break_stack =
        printer->line_break_stack;

    min::line_break line_break = printer->line_break;
        // We copy this from its relocatable position
	// in the stack so we do not have to use a
	// min::ptr to access it.
    min::uns32 i;
        // i is also needed at end to copy line_break
	// back.
    for ( i = 0; i < line_break_stack->length; ++ i )
    {
        if (   (&line_break_stack[i])->column
	     > (&line_break_stack[i])->indent
	     &&
                (&line_break_stack[i])->offset
	     >= printer->file->end_offset )
	{
	    line_break = line_break_stack[i];
	    break;
	}
    }
    if ( line_break.column <= line_break.indent )
	return true;
    if ( line_break.offset < printer->file->end_offset )
	return true;

    // The following deals with line breaks created by
    // erase_space.
    //
    if ( line_break.column > printer->column )
	return true;

    // buffer[begoff..endoff-1] are horizontal spaces
    // to be deleted.
    //
    min::uns32 endoff = line_break.offset;
    min::uns32 begoff = endoff;
    while ( begoff > 0
            &&
	    ( buffer[begoff-1] == ' '
	      ||
	      buffer[begoff-1] == '\t' ) )
	    -- begoff;

    // Create indent+1 buffer elements beginning at
    // begoff into which to insert NUL and indent single
    // spaces.
    //
    min::uns32 gap = endoff - begoff;
    min::uns32 movelen = buffer->length - endoff;
    if ( gap > line_break.indent + 1 )
    {
	// Move down.
	//
	// Notes: Use memmove instead of memcpy.
	//        Also do NOT move 0 bytes as then
	//        source address is off the end of
	// 	      the vector and is not legal.
	//
	if ( movelen > 0 )
	    memmove
	        ( ~ & buffer[ begoff
		             +line_break.indent+1],
		  ~ & buffer[endoff],
		  movelen );
	min::pop
	    ( buffer, gap - line_break.indent - 1 );
    }
    else if ( gap < line_break.indent + 1 )
    {
	// Move up.
	//
	min::push
	    ( buffer, line_break.indent + 1 - gap );
	if ( movelen > 0 )
	    memmove
	        ( ~ & buffer[begoff+line_break.indent
		                   +1],
		  ~ & buffer[endoff],
		  movelen );
    }

    // Insert NUL and indent spaces.
    //
    min::end_line ( printer->file, begoff++ );

    for ( min::uns32 i = 0; i < line_break.indent;
                            ++ i )
	buffer[begoff++] = ' ';

    // Adjust parameters.
    //
    min::uns32 offset_adj = begoff - line_break.offset;
    min::uns32 column_adj =
        line_break.indent - line_break.column;
    line_break.offset += offset_adj;
    line_break.column += column_adj;
    printer->column += column_adj;

    if ( i != printer->line_break_stack->length )
    {
        line_break_stack[i++] = line_break;
	for ( ; i < line_break_stack->length; ++ i )
	{
	    (&line_break_stack[i])->offset +=
	        offset_adj;
	    (&line_break_stack[i])->column +=
	        column_adj;
	    (&line_break_stack[i])->indent +=
	        column_adj;
	}
	printer->line_break.offset += offset_adj;
	printer->line_break.column += column_adj;
	printer->line_break.indent += column_adj;
    }
    else
        printer->line_break = line_break;

    return false;
}

void MINT::print_item_preface
	( min::printer printer,
	  min::uns32 str_class )
{
    min::uns32 flags = printer->state
                     & (   min::AFTER_LEADING
		         + min::AFTER_TRAILING
			 + min::FORCE_SPACE_OK );
    if ( flags )
    {
        printer->state &= ~ flags;

	if ( ! ( str_class & min::IS_GRAPHIC ) )
	    flags = 0; // Do NOT print space.
	else if ( ! (   printer->last_str_class
	              & min::IS_GRAPHIC ) )
	    flags = 0; // Do NOT print space.
	else if ( (   printer->print_format.op_flags
	            & min::FORCE_SPACE )
		  &&
		  ( flags & min::FORCE_SPACE_OK ) )
	    /* Do nothing, i.e., print space. */;
	else if ( flags & min::AFTER_TRAILING
		  &&
		  ( str_class & min::IS_TRAILING ) )
	    flags = 0; // Do NOT print space.
	else if ( flags & min::AFTER_LEADING
	          &&
		  (   printer->last_str_class
		    & min::IS_LEADING ) )
	    flags = 0; // Do NOT print space.
	else
	    /* Do nothing, i.e., print space. */{}

	if ( flags )
	{
	    min::push ( printer->file->buffer ) = ' ';
	    ++ printer->column;
	    if ( printer->print_format.break_control
				      .break_after
		 &
		 printer->print_format.char_flags[' '] )
		printer->state |= min::BREAK_AFTER;
	    else
		printer->state &= ~ min::BREAK_AFTER;
	}

	if ( printer->state & min::AFTER_SAVE_INDENT )
	    printer << min::save_indent;

	if ( printer->state & min::AFTER_SET_BREAK )
	    printer << min::set_break;
    }
    else
        MIN_REQUIRE
	    ( (   printer->state
	        & (   min::AFTER_SAVE_INDENT
		    | min::AFTER_SET_BREAK ) )
	      == 0 );

    printer->state &= ~ (   min::AFTER_SAVE_INDENT
	                  | min::AFTER_SET_BREAK );
}

min::printer MINT::print_unicode
	( min::printer printer,
	  min::uns32 line_op_flags,
	  min::unsptr & n,
	  min::ptr<const min::Uchar> & p,
	  min::uns32 & width,
	  const min::break_control * break_control,
	  const min::display_control * display_control,
	  const min::Uchar * substring,
	  min::unsptr substring_length,
	  min::ustring replacement )
{
    if ( n == 0 ) return printer;

    char temp[32];

    min::support_control sc =
        printer->print_format.support_control;
    min::break_control bc =
        break_control != NULL ? * break_control :
        printer->print_format.break_control;
    min::display_control dc =
        display_control != NULL ? * display_control :
        printer->print_format.display_control;
    if (   line_op_flags
         & min::DISPLAY_NON_GRAPHIC )
    {
        dc.display_char &= min::IS_GRAPHIC;
        dc.display_suppress &= min::IS_GRAPHIC;
    }
    const min::uns32 * char_flags =
	printer->print_format.char_flags;

    min::uns32 line_length =
        printer->line_break.line_length;

    min::packed_vec_insptr<char> buffer =
        printer->file->buffer;
    min::uns32 expand_ht =
        line_op_flags & min::EXPAND_HT;

    bool no_line_break_enabled = false;
        // This prevents repeated checks for an enabled
	// line break that does not exist.

    bool rep_is_space;
    while ( n )
    {
        min::Uchar c = * p;
	min::uns16 cindex = min::Uindex ( c );
	min::uns32 cflags = char_flags[cindex];
	if ( ( cflags & sc.support_mask ) == 0 )
	    cflags = sc.unsupported_char_flags;

        // Compute the character representative.
	//
	min::uns32 columns =
	    ( cflags & min::IS_NON_SPACING ? 0 : 1 );
	min::uns32 length = 1;
	min::unsptr clength = 1;
	const char * rep = temp;
	min::ustring prefix = NULL;
	min::ustring postfix = NULL;
	rep_is_space = false;

	if (    substring != NULL
	     && c == substring[0]
	     && n >= substring_length
	     &&    memcmp ( ~p, substring,
	                          substring_length
				* sizeof ( min::Uchar) )
		== 0 )
	{
	    // Found substring; output replacement.
	    //
	    length =
	        min::ustring_length ( replacement );
	    columns =
	        min::ustring_columns ( replacement );
	    MIN_ASSERT ( columns > 0,
	                 "replacement ustring_columns"
			 " is zero" );
	    rep = min::ustring_chars ( replacement );
	    clength = substring_length;
	}
	else if ( cflags & dc.display_char )
	{
	    if ( c == '\t' )
	    {
		rep_is_space = true;

		min::uns32 spaces =
		    8 - printer->column % 8;
		columns = spaces;
	        if ( expand_ht )
		{
		    rep = "        ";
		    length = spaces;
		}
		else
		    temp[0] = (char) c;
	    }
	    else if ( 0 < c && c < 128 )
	    {
	        temp[0] = (char) c;
		rep_is_space = ( c == ' ' );
	    }
	    else
	    {
	        char * q = temp;
		length = min::unicode_to_utf8 ( q, c );
	    }
	}
	else if ( cflags & dc.display_suppress )
	{
	    n --;
	    p ++;
	    continue;
	}
	else if ( (   line_op_flags
	            & min::DISPLAY_PICTURE )
	          &&
		     min::unicode::picture[cindex]
		  != NULL )
	{
	    min::ustring picture =
	        min::unicode::picture[cindex];

	    length = min::ustring_length ( picture );
	    columns = min::ustring_columns ( picture );
	    MIN_ASSERT ( columns > 0,
	                 "picture ustring_columns"
			 " is zero" );
	    rep = min::ustring_chars ( picture );
	}
	else
	{
	    if ( min::unicode::name[cindex] != NULL )
	    {
		min::ustring name =
		    min::unicode::name[cindex];
		length = min::ustring_length ( name );
		columns = min::ustring_columns ( name );
		MIN_ASSERT ( columns > 0,
		             "name ustring_columns"
			     " is zero" );
		rep = min::ustring_chars ( name );
	    }
	    else
	    {
		sprintf ( temp + 1, "%02X", c );
		if ( isdigit ( temp[1] ) ) ++ rep;
		else temp[0] = '0';
		length = columns = ::strlen ( rep );
	    }

	    prefix = printer->print_format
		     	     .char_name_format
			    ->char_name_prefix;
	    postfix = printer->print_format
		     	     .char_name_format
			    ->char_name_postfix;

	    min::uns32 prefix_columns =
	        min::ustring_columns ( prefix );
	    MIN_ASSERT ( prefix_columns > 0,
	                 "char_name_prefix"
			 " ustring_columns is zero" );
	    min::uns32 postfix_columns =
	        min::ustring_columns ( postfix );
	    MIN_ASSERT ( postfix_columns > 0,
	                 "char_name_postfix"
			 " ustring_columns is zero" );
	    columns += prefix_columns + postfix_columns;
	}

	if ( columns > width )
	    return printer;

	n -= clength;
	p = p + clength;

	if ( line_op_flags & min::DISABLE_LINE_BREAKS )
	    /* Do Nothing */;
	else
	{
	    if ( columns > 0 )
	    {
		if ( ( cflags & bc.break_before )
		     ||
		     ( ( cflags & bc.break_after ) == 0
		       &&
		       (   printer->state
			 & min::BREAK_AFTER ) ) )
		{
		    printer->line_break.offset =
			buffer->length;
		    printer->line_break.column =
			printer->column;
		    no_line_break_enabled = false;
		}

		if (   printer->column + columns
		     > line_length
		     &&
		     ! rep_is_space
		     &&
		     ! no_line_break_enabled )
		{
		    no_line_break_enabled =
			MINT::insert_line_break
			    ( printer );
		}
	    }

	    if ( ( cflags & bc.break_after )
		 ||
		 ( ( cflags & bc.conditional_break )
		   &&
			printer->column
		      - printer->line_break.column
		   >= bc.conditional_columns ) )
		printer->state |= min::BREAK_AFTER;
	    else
		printer->state &= ~ min::BREAK_AFTER;
	}
	   
	if ( prefix != NULL )
	    min::push
	        ( buffer,
		  min::ustring_length ( prefix ),
		  min::ustring_chars  ( prefix ) );
	min::push ( buffer, length, rep );
	if ( postfix != NULL )
	    min::push
	        ( buffer,
		  min::ustring_length ( postfix ),
		  min::ustring_chars  ( postfix ) );
	printer->column += columns;
	width -= columns;

    }

    return printer;
}

min::printer print_quoted_unicode
	( min::printer printer,
	  min::unsptr length,
	  min::ptr<const min::Uchar> p,
	  const min::str_format * sf )
{
    min::print_item_preface
        ( printer, min::IS_GRAPHIC );

    min::quote_format qf = sf->quote_format;
    min::ustring prefix = qf.str_prefix;
    min::ustring postfix = qf.str_postfix;
    min::ustring replacement =
        qf.str_postfix_replacement;
    MIN_ASSERT ( prefix != NULL,
                 "str_prefix is NULL" );
    MIN_ASSERT ( postfix != NULL,
                 "str_postfix is NULL" );
    MIN_ASSERT ( replacement != NULL,
                 "str_postfix_replacement is NULL" );

    min::ustring break_begin = sf->str_break_begin;
    min::ustring break_end = sf->str_break_end;

    min::line_break_stack line_break_stack =
        printer->line_break_stack;

    min::line_break line_break = printer->line_break;
        // We copy this from its relocatable position
	// in the stack so we do not have to use a
	// min::ptr to access it.
    for ( min::uns32 i = 0;
          i < line_break_stack->length; ++ i )
    {
        if (   (&line_break_stack[i])->column
	     > (&line_break_stack[i])->indent
	     &&
                (&line_break_stack[i])->offset
	     >= printer->file->end_offset )
	{
	    line_break = line_break_stack[i];
	    break;
	}
    }

    min::uns32 prefix_columns =
        min::ustring_columns ( prefix );
    MIN_ASSERT ( prefix_columns > 0,
                 "str_prefix ustring_columns"
		 " is zero" );
    min::uns32 postfix_columns =
        min::ustring_columns ( postfix );
    MIN_ASSERT ( postfix_columns > 0,
                 "str_postfix ustring_columns"
		 " is zero" );

    min::uns32 break_begin_columns =
        ( break_begin == NULL ? 0 :
	  min::ustring_columns ( break_begin ) );
    min::uns32 break_end_columns =
        ( break_end == NULL ? 0 :
	  min::ustring_columns ( break_end ) );

    min::uns32 reduced_width =
          printer->line_break.line_length
        - line_break.indent
	- prefix_columns
        - postfix_columns
	- break_end_columns;
    if ( reduced_width < 10 ) reduced_width = 10;

    min::uns32 postfix_length =
        min::ustring_length ( postfix );
    min::Uchar postfix_string[postfix_length+1];
    {
	const char * p = min::ustring_chars ( postfix );
	const char * endp = p + postfix_length;
	min::Uchar * q = postfix_string;
	while ( p != endp )
	    * q ++ = min::utf8_to_unicode ( p, endp );
	MIN_REQUIRE (    q - postfix_string
	              <= postfix_length + 1 );
	postfix_length = q - postfix_string;
    }


    min::uns32 width =
        ( printer->state & min::DISABLE_STR_BREAKS ?
	  0xFFFFFFFF : reduced_width );
    if ( printer->state & min::BREAK_AFTER )
	printer << min::set_break;
    min::print_ustring ( printer, prefix );
    while ( length > 0 )
    {
	MINT::print_unicode
	    ( printer, printer->print_format.op_flags,
	               length, p, width,
		       & min::
		            no_auto_break_break_control,
	               & printer->print_format
		                .quoted_display_control,
	               postfix_string,
		       postfix_length,
		       replacement );

	if ( length == 0 ) break;

	min::print_ustring ( printer, postfix );
	min::print_ustring
	        ( printer, break_begin );
	min::print_space ( printer );
	printer << min::set_break;
	min::print_ustring
	        ( printer, break_end );
	min::print_ustring ( printer, prefix );

	width = reduced_width - break_begin_columns;
    }
    min::print_ustring ( printer, postfix );

    return printer;
}

min::printer print_breakable_unicode
	( min::printer printer,
	  min::unsptr length,
	  min::ptr<const min::Uchar> p,
	  const min::str_format * sf )
{

    min::ustring break_begin = sf->str_break_begin;
    min::ustring break_end = sf->str_break_end;

    min::print_item_preface
        ( printer, min::IS_GRAPHIC );

    min::line_break_stack line_break_stack =
        printer->line_break_stack;

    min::line_break line_break = printer->line_break;
        // We copy this from its relocatable position
	// in the stack so we do not have to use a
	// min::ptr to access it.
    for ( min::uns32 i = 0;
          i < line_break_stack->length; ++ i )
    {
        if (   (&line_break_stack[i])->column
	     > (&line_break_stack[i])->indent
	     &&
                (&line_break_stack[i])->offset
	     >= printer->file->end_offset )
	{
	    line_break = line_break_stack[i];
	    break;
	}
    }

    min::uns32 break_begin_columns =
        ( break_begin == NULL ? 0 :
	  min::ustring_columns ( break_begin ) );
    min::uns32 break_end_columns =
        ( break_end == NULL ? 0 :
	  min::ustring_columns ( break_end ) );

    min::uns32 reduced_width =
          printer->line_break.line_length
        - line_break.indent
	- break_end_columns;
    if ( reduced_width < 10 ) reduced_width = 10;

    min::uns32 width =
        ( printer->state & min::DISABLE_STR_BREAKS ?
	  0xFFFFFFFF : reduced_width );
    if ( printer->state & min::BREAK_AFTER )
	printer << min::set_break;
    while ( length > 0 )
    {
	MINT::print_unicode
	    ( printer, printer->print_format.op_flags,
	      length, p, width,
	      & min::no_auto_break_break_control );

	if ( length == 0 ) break;

	min::print_ustring ( printer, break_begin );
	min::print_space ( printer );
	printer << min::set_break;
	min::print_ustring ( printer, break_end );

	width = reduced_width - break_begin_columns;
    }

    return printer;
}

void min::debug_str_class
	( const char * header,
	  min::uns32 str_class,
	  min::unsptr n,
	  min::ptr<const min::Uchar> p )
{
    char buffer [7*n+1];
    char * bp = buffer;
    const char * endbp = buffer + sizeof ( buffer );
    const Uchar * q = ~ p;
    unicode_to_utf8 ( bp, endbp, q, q + n );
    MIN_REQUIRE ( bp < endbp );
    * bp = 0;
    std::cout << header << " `" << buffer << "' "
              << std::hex << str_class << std::dec
              << std::endl;
}

bool min::is_number
	( min::unsptr n,
	  min::ptr<const min::Uchar> p )
{
    if ( n == 0 ) return false;
    min::Uchar c = * p ++;
	// n = number of characters left in p
        //   + 1 (the character in c).
	// When n == 0 there is no character in c.

    if ( c == U'+' || c == U'-' )
    {
	if ( -- n == 0 ) return false;
	    // "+" and "-" are not converted by strtod.
        c = * p ++;
    }

    // Count digits and points.
    //
    min::unsptr digits = 0;
    min::unsptr points = 0;
    while ( true )
    {
        if ( U'0' <= c && c <= U'9' ) ++ digits;
        else if ( c == U'.' )
	{
	    ++ points;
	    if ( points > 1 ) return false;
	}
        else break;

	if ( -- n == 0 ) break;
        c = * p ++;
    }

    if ( digits == 0 )
    {
	if ( points > 0 ) return false;
	if ( n != 3 ) return false;
        if ( c == U'n' || c == U'N' )
	{
	    // Check for "nan".
	    //
	    c = * p ++;
	    if ( c != U'a' && c != U'A' )
		return false;
	    c = * p ++;
	    if ( c != U'n' && c != U'N' )
		return false;
	    return true;
	}
        if ( c == U'i' || c == U'I' )
	{
	    // Check for "inf".
	    //
	    c = * p ++;
	    if ( c != U'n' && c != U'N' )
		return false;
	    c = * p ++;
	    if ( c != U'f' && c != U'F' )
		return false;
	    return true;
	}
	return false;
    }

    if ( n == 0 ) return true;

    // Check for exponent.
    //
    if ( c != U'e' && c != U'E' ) return false;
    if ( -- n == 0 ) return false;
    c = * p ++;
    if ( c == U'+' || c == U'-' )
    {
        if ( -- n == 0 ) return false;
	    // "e+" and "e-" are not accepted by strtod.
        c = * p ++;
    }
    while ( true )
    {
        if ( c < U'0' || U'9' < c ) return false;
	if ( -- n == 0 ) break;
        c = * p ++;
    }
    return true;
}

static min::uns32 standard_str_classifier_function
	( const min::uns32 * char_flags,
	  min::support_control sc,
	  min::unsptr n,
	  min::ptr<const min::Uchar> p )
{
    if ( n == 0 ) return 0;

    const min::uns32 SELECTOR = min::IS_LETTER
                              | min::IS_DIGIT;

    min::ptr<const min::Uchar> q = p;
    min::Uchar first = * q ++;
    min::uns32 first_cflags =
	    min::char_flags ( char_flags, sc, first );
    min::uns32 last_cflags = first_cflags;
    min::uns32 and_of_cflags = first_cflags;
    min::uns32 or_of_cflags = first_cflags;
    min::uns32 result = ( first_cflags & SELECTOR );
    bool repeating = true;

    for ( min::unsptr i = 1; i < n; ++ i )
    {
	min::Uchar c = * q ++;
	last_cflags =
	    min::char_flags ( char_flags, sc, c );
	and_of_cflags &= last_cflags;
	or_of_cflags |= last_cflags;
	if ( c != first ) repeating = false;
	if ( result == 0
	     &&
	     ( last_cflags & SELECTOR ) )
	    result = ( last_cflags & SELECTOR );
    }

    result |= and_of_cflags;

    if ( ( result & ( SELECTOR + min::IS_MARK ) )
         &&
	 ( and_of_cflags & min::IS_GRAPHIC )
	 &&
	 ! ( or_of_cflags & (   min::NEEDS_QUOTES
	                      + min::IS_SEPARATOR ) )
	 &&
	 ! ( first_cflags & min::IS_LEADING )
	 &&
	 ! ( last_cflags & min::IS_TRAILING ) )
        result |= min::IS_BREAKABLE;

    if ( result & min::IS_NATURAL )
        return result | min::NEEDS_QUOTES;
    else if ( or_of_cflags & min::NEEDS_QUOTES )
        return result | min::NEEDS_QUOTES;
    else if ( ! ( and_of_cflags & min::IS_GRAPHIC ) )
        return result | min::NEEDS_QUOTES;
    else if ( min::is_number ( n, p ) )
	return result | min::NEEDS_QUOTES;
    else if ( ( or_of_cflags & min::IS_SEPARATOR )
	      ||
              ( first_cflags & min::IS_LEADING )
	      ||
              ( last_cflags & min::IS_TRAILING ) )
    {
        if ( ! repeating
	     ||
	     ( ! ( and_of_cflags & min::IS_REPEATER )
	       &&
	       n > 1 ) )
	    return result | min::NEEDS_QUOTES;
    }

    return result;
}
const min::str_classifier min::standard_str_classifier =
    & ::standard_str_classifier_function;

static min::uns32
    quote_separator_str_classifier_function
	( const min::uns32 * char_flags,
	  min::support_control sc,
	  min::unsptr n,
	  min::ptr<const min::Uchar> p )
{
    min::uns32 str_class =
        ::standard_str_classifier_function
	    ( char_flags, sc, n, p );
    if ( str_class & ( min::IS_LEADING
                       |
		       min::IS_TRAILING
		       |
		       min::IS_SEPARATOR ) )
	str_class |= min::NEEDS_QUOTES;
    return str_class;
}
const min::str_classifier
    min::quote_separator_str_classifier =
    & ::quote_separator_str_classifier_function;

static min::uns32
    quote_value_str_classifier_function
	( const min::uns32 * char_flags,
	  min::support_control sc,
	  min::unsptr n,
	  min::ptr<const min::Uchar> p )
{
    min::uns32 str_class =
        ::standard_str_classifier_function
	    ( char_flags, sc, n, p );
    if ( ( str_class & min::IS_LETTER ) == 0
         &&
	 ( str_class & min::IS_DIGIT ) == 0 )
	str_class |= min::NEEDS_QUOTES;
    return str_class;
}
const min::str_classifier
    min::quote_value_str_classifier =
    &
    ::quote_value_str_classifier_function;

static min::uns32 quote_all_str_classifier_function
	( const min::uns32 * char_flags,
	  min::support_control sc,
	  min::unsptr n,
	  min::ptr<const min::Uchar> p )
{
    return min::NEEDS_QUOTES;
}
const min::str_classifier
    min::quote_all_str_classifier =
    & ::quote_all_str_classifier_function;

static min::uns32 null_str_classifier_function
	( const min::uns32 * char_flags,
	  min::support_control sc,
	  min::unsptr n,
	  min::ptr<const min::Uchar> p )
{
    for ( min::unsptr i = 0; i < n; ++ i )
    {
	min::Uchar c = * p ++;
	min::uns32 cflags =
	    min::char_flags ( char_flags, sc, c );
	if ( ! ( cflags & min::IS_VHSPACE ) )
	    return min::IS_GRAPHIC;
    }
    return 0;
}
const min::str_classifier min::null_str_classifier =
    & ::null_str_classifier_function;

min::printer min::print_unicode
	( min::printer printer,
	  min::unsptr n,
	  min::ptr<const min::Uchar> p,
	  const min::str_format * sf )
{
    min::uns32 str_class =
        sf == NULL ?
	    ::null_str_classifier_function
	       ( printer->print_format.char_flags,
	         printer->print_format
		         .support_control,
	         n, p ) :
	    min::str_class
	       ( printer->print_format.char_flags,
	         printer->print_format
		         .support_control,
	         n, p, sf->str_classifier );
    if ( sf != NULL )
    {
        if ( ( str_class & min::NEEDS_QUOTES )
	     ||
	     ! ( str_class & min::IS_GRAPHIC ) )
	    return ::print_quoted_unicode
		( printer, n, p, sf );
	else if ( str_class & min::IS_BREAKABLE )
	    return ::print_breakable_unicode
		( printer, n, p, sf );
    }

    if ( n == 0 ) return printer;
    else
    {
	min::uns32 width = 0xFFFFFFFF;

	min::print_item_preface ( printer, str_class );
	return MINT::print_unicode
	    ( printer, printer->print_format.op_flags,
	      n, p, width );
    }
}



// NOTE: Importantly this function copies s to the
// stack BEFORE calling anything that might relocate
// memory.
//
min::printer MINT::print_cstring
	( min::printer printer, const char * s,
	  const min::str_format * sf,
	  bool allow_force_pgen )
{
    if ( sf == NULL
         &&
	 allow_force_pgen
	 &&
	   printer->print_format.op_flags
	 & min::FORCE_PGEN )
        return min::print_gen
	    ( printer, min::new_str_gen ( s ) );

    // We translate this case to the punicode case
    // because we need to be able to perform look-ahead
    // for combining diacritics, and in the future for
    // other things.
    
    min::uns32 length = ::strlen ( s );
    const char * ends = s + length;

    min::Uchar buffer[length+1];
    min::unsptr i = 0;

    while ( s < ends )
    {
	min::Uchar c = (min::uns8) * s ++;
        if ( c >= 0x80 )
	{
	    // UTF-8 encoded character.  Might be
	    // overlong encoded ASCII character such
	    // as NUL.

	    -- s;
	    c = min::utf8_to_unicode ( s, ends );
	}
	buffer[i++] = c;
    }

    min::ptr<const min::Uchar> p =
        min::new_ptr<const min::Uchar> ( buffer );

    return min::print_unicode ( printer, i, p, sf );
}

min::printer operator <<
	( min::printer printer, min::int64 i )
{
    if (   printer->print_format.op_flags
	 & min::FORCE_PGEN )
        return min::print_gen
	    ( printer,
	      min::new_num_gen ( (min::float64) i ) );

    char buffer[32];
    min::unsptr n = sprintf ( buffer, "%lld", i );
    return min::print_item
        ( printer, buffer, n, n, min::IS_GRAPHIC );
}

min::printer operator <<
	( min::printer printer, min::uns64 u )
{
    if (   printer->print_format.op_flags
	 & min::FORCE_PGEN )
        return min::print_gen
	    ( printer,
	      min::new_num_gen ( (min::float64) u ) );

    char buffer[32];
    min::unsptr n = sprintf ( buffer, "%llu", u );
    return min::print_item
        ( printer, buffer, n, n, min::IS_GRAPHIC );
}

min::printer operator <<
	( min::printer printer, min::float64 f )
{
    if (   printer->print_format.op_flags
	 & min::FORCE_PGEN )
        return min::print_gen
	    ( printer, min::new_num_gen ( f ) );

    char buffer[64];
    min::unsptr n = sprintf ( buffer, "%.15g", f );
    return min::print_item
        ( printer, buffer, n, n, min::IS_GRAPHIC );
}

min::uns32 min::pwidth
	( min::uns32 & column,
          const char * s, min::unsptr n,
	  const min::print_format & print_format,
	  const min::line_format * line_format )
{
    min::uns32 op_flags =
        ( line_format != NULL ?
	  line_format->op_flags :
          print_format.op_flags );
    min::support_control sc =
        print_format.support_control;
    min::display_control dc =
        print_format.display_control;
    if ( op_flags & min::DISPLAY_NON_GRAPHIC )
    {
        dc.display_char &= min::IS_GRAPHIC;
        dc.display_suppress &= min::IS_GRAPHIC;
    }
    const min::uns32 * char_flags =
	print_format.char_flags;

    const char * ends = s + n;
    char temp[32];

    min::uns32 original_column = column;
    while ( s < ends )
    {
        Uchar c = utf8_to_unicode ( s, ends );
	uns16 cindex = Uindex ( c );
	uns32 cflags = char_flags[cindex];
	if ( ( cflags & sc.support_mask ) == 0 )
	    cflags = sc.unsupported_char_flags;

        // Compute column increment.
	//
	if ( cflags & dc.display_char )
	{
	    if ( c == '\t' )
	        column += 8 - column % 8;
	    else if ( ! (   cflags
	                  & min::IS_NON_SPACING ) )
	        column += 1;
	}
	else if ( cflags & dc.display_suppress )
	    continue;
	else if ( ( op_flags & min::DISPLAY_PICTURE )
		  &&
		  unicode::picture[cindex] != NULL )
	{
	    ustring picture =
	        unicode::picture[cindex];
	    min::uns32 picture_columns =
	        ustring_columns ( picture );
	    MIN_ASSERT ( picture_columns > 0,
	                 "picture ustring_columns"
			 " is zero" );
	    column += picture_columns;
	}
	else
	{
	    if ( unicode::name[cindex] != NULL )
	    {
		ustring name =
		    unicode::name[cindex];
		min::uns32 name_columns =
		    ustring_columns ( name );
		MIN_ASSERT ( name_columns > 0,
			     "name ustring_columns"
			     " is zero" );
		column += name_columns;
	    }
	    else
	    {
		column += sprintf ( temp, "%02X", c );
		if ( ! isdigit ( temp[0] ) ) ++ column;
	    }

	    min::uns32 prefix_columns =
		min::ustring_columns
		    ( print_format.char_name_format
				 ->char_name_prefix );
	    MIN_ASSERT ( prefix_columns > 0,
		         "char_name_prefix"
			 " ustring_columns is zero" );
	    min::uns32 postfix_columns =
		min::ustring_columns
		    ( print_format.char_name_format
				 ->char_name_postfix );
	    MIN_ASSERT ( postfix_columns > 0,
		         "char_name_postfix"
			 " ustring_columns is zero" );

	    column += prefix_columns + postfix_columns;
	}
    }
    return column - original_column;
}


// Printing General values
// -------- ------- ------

std::ostream & operator <<
	( std::ostream & out, min::gen g )
{
    if ( min::is_stub ( g )
         &&
	 MUP::stub_of ( g ) == MINT::null_stub )
    {
        // Must do this BEFORE executing is_num,
	// is_str, etc.  Must use MINT::null_stub
	// instead of min::NULL_STUB.
	//
	return
	    out << "new_stub_gen ( MINT::null_stub )";
    }
    else if ( min::is_num ( g ) )
    {
        char buffer[100];
	sprintf ( buffer, "%.15g",
	          min::unprotected::float_of ( g ) );
	return out << buffer;
    }
    else if ( min::is_str ( g ) )
    {
        min::str_ptr sp ( g );
	return out << '"'
	           << min::unprotected::str_of ( sp )
		   << '"';
    }
    else if ( min::is_lab ( g ) )
    {
        min::unprotected::lab_ptr lp ( g );
	min::uns32 n = min::lablen ( lp );
	out << "[<";
	for ( unsigned i = 0; i < n; ++ i )
	    out << " " << lp[i];
	return out << " >]";
    }
    else if ( min::is_preallocated ( g ) )
    {
        min::uns32 id =
	    min::id_of_preallocated ( g );
        min::uns32 count =
	    min::count_of_preallocated ( g );
	return out << "PREALLOCATED("
	           << count << "*" << id << ")";
    }
    else if ( min::is_stub ( g ) )
    {
	min::stub * s = MUP::stub_of ( g );
	if ( s == min::NULL_STUB )
	    return out << "new_stub_gen"
	                      " ( min::NULL_STUB )";
	else
	    return out << "new_stub_gen (0x"
	               << std::hex << (min::unsptr) s
		       << std::dec << ")";
    }
    else if ( min::is_list_aux ( g ) )
	return out << "LIST_AUX("
		   << min::unprotected
		         ::list_aux_of ( g )
		   << ")";
    else if ( min::is_sublist_aux ( g ) )
	return out << "SUBLIST_AUX("
		   << min::unprotected
		         ::sublist_aux_of ( g )
		   << ")";
    else if ( min::is_indirect_aux ( g ) )
	return out << "INDIRECT_AUX("
		   << min::unprotected
		         ::indirect_aux_of ( g )
		   << ")";
    else if ( min::is_index ( g ) )
	return out << "INDEX("
		   << min::unprotected
		         ::index_of ( g )
		   << ")";
    else if ( min::is_control_code ( g ) )
	return out << "CONTROL_CODE(0x" << std::hex
		   << min::unprotected
		         ::control_code_of ( g )
		   << std::dec << ")";
    else if ( min::is_special ( g ) )
	return out << "SPECIAL(0x" << std::hex
		   << min::unprotected
		         ::special_index_of ( g )
		   << std::dec << ")";
    else
	return out << "UNDEFINED_GEN(0x" << std::hex
		   << min::unprotected::value_of ( g )
		   << std::dec << ")";
}

static min::uns32 standard_divisors[] =
{
    2,3,4,5,6,7,8,9,10,11,12,
    16,32,64,128,256,512,1024,
    0
};
const min::uns32 * min::standard_divisors =
    ::standard_divisors;

static min::num_format short_num_format =
{
    "%.0f", 1e7, "%.6g", NULL, 0
};
const min::num_format * min::short_num_format =
    & ::short_num_format;

static min::num_format long_num_format =
{
    "%.0f", 1e15, "%.15g", NULL, 0
};
const min::num_format * min::long_num_format =
    & ::long_num_format;

static min::num_format fraction_num_format =
{
    "%.0f", 1e7, "%.6g", min::standard_divisors, 1e-9
};
const min::num_format * min::fraction_num_format =
    & ::fraction_num_format;

min::printer min::print_num
	( min::printer printer,
	  min::float64 value,
	  const min::num_format * nf )
{
    if ( nf == NULL )
    {
        const gen_format * f =
	    printer->print_format.gen_format;
	MIN_REQUIRE ( f != NULL );
	nf = f->num_format;
	MIN_REQUIRE ( nf != NULL );
    }

    char buffer[128];
    min::uns32 len;
    if ( fabs ( value ) < nf->non_float_bound )
    {
	long long I = (long long) floor ( value );

	if ( I == value )
	    len = sprintf ( buffer,
		            nf->int_printf_format,
		            value );

	else if ( nf->fraction_divisors != NULL )
	{
	    min::uns32 N, D;
	    double f = value - I;
	    if ( I < 0 ) f = 1 - f;

	    const min::uns32 * p =
	        nf->fraction_divisors;
	    while ( ( D = * p ++ ) )
	    {
		N = (min::uns32) round ( D * f );
		if ( fabs ( (double) N / D - f )
		     < nf->fraction_accuracy )
		    break;
	    }

	    if ( D != 0 )
	    {
		char * p = buffer;
		if ( I > 0 )
		    p += sprintf ( p, "%lld ", I );
		else if ( I < -1 )
		    p += sprintf ( p, "%lld ", I + 1 );
		else if ( I < 0 )
		    * p ++ = '-';
	        p += sprintf ( p, "%d/%d", N, D );
		len = p - buffer;
	    }
	    else
		len = sprintf ( buffer,
		                nf->float_printf_format,
			        value );
	}
	else
	    len = sprintf ( buffer,
			    nf->float_printf_format,
			    value );
    }
    else
	len = sprintf ( buffer, nf->float_printf_format,
	                value );

    return min::print_item
	( printer, buffer, len, len );
}

const min::quote_format min::standard_quote_format =
{
    (min::ustring) "\x01\x01" "\"",
    (min::ustring) "\x01\x01" "\"",
    (min::ustring) "\x03\x03" "<Q>"
};

static min::str_format quote_separator_str_format =
{
    min::quote_separator_str_classifier,
    (min::ustring) "\x01\x01" "#",
    (min::ustring) "\x01\x01" "#",
    min::standard_quote_format,
    0xFFFFFFFF
};
const min::str_format *
    min::quote_separator_str_format =
	& ::quote_separator_str_format;

static min::str_format
    quote_value_str_format =
{
    min::quote_value_str_classifier,
    (min::ustring) "\x01\x01" "#",
    (min::ustring) "\x01\x01" "#",
    min::standard_quote_format,
    0xFFFFFFFF
};
const min::str_format *
    min::quote_value_str_format =
	& ::quote_value_str_format;

static min::str_format
    quote_value_id_str_format =
{
    min::quote_value_str_classifier,
    (min::ustring) "\x01\x01" "#",
    (min::ustring) "\x01\x01" "#",
    min::standard_quote_format,
    0
};
const min::str_format *
    min::quote_value_id_str_format =
	& ::quote_value_id_str_format;

static min::str_format quote_all_str_format =
{
    min::quote_all_str_classifier,
    (min::ustring) "\x01\x01" "#",
    (min::ustring) "\x01\x01" "#",
    min::standard_quote_format,
    0xFFFFFFFF
};
const min::str_format * min::quote_all_str_format =
    & ::quote_all_str_format;

static min::str_format standard_str_format =
{
    min::standard_str_classifier,
    (min::ustring) "\x01\x01" "#",
    (min::ustring) "\x01\x01" "#",
    min::standard_quote_format,
    0xFFFFFFFF
};
const min::str_format * min::standard_str_format =
    & ::standard_str_format;

static min::lab_format name_lab_format =
{
    NULL,
    min::space_if_none_pstring,
    NULL
};
const min::lab_format * min::name_lab_format =
    & ::name_lab_format;

static min::lab_format bracket_lab_format =
{
    min::left_square_angle_space_pstring,
    min::space_if_none_pstring,
    min::space_right_angle_square_pstring
};
const min::lab_format * min::bracket_lab_format =
    & ::bracket_lab_format;

static min::lab_format leading_always_lab_format =
{
    NULL,
    min::leading_always_pstring,
    NULL
};
const min::lab_format *
    min::leading_always_lab_format =
	& ::leading_always_lab_format;

static min::lab_format trailing_always_lab_format =
{
    NULL,
    min::trailing_always_pstring,
    NULL
};
const min::lab_format *
    min::trailing_always_lab_format =
	& ::trailing_always_lab_format;

static min::special_format name_special_format =
{
    NULL, NULL,
    min::NULL_STUB	    // special_names*
};
const min::special_format * min::name_special_format =
    & ::name_special_format;

static min::special_format bracket_special_format =
{
    min::left_square_dollar_space_pstring,
    min::space_dollar_right_square_pstring,
    min::NULL_STUB	    // special_names*
};
const min::special_format *
	min::bracket_special_format =
    & ::bracket_special_format;

static min::flag_format standard_attr_flag_format =
{
    min::left_square_leading_always_pstring,
    min::trailing_always_right_square_pstring,
    min::NULL_STUB	    // standard_attr_flag_names*
};
const min::flag_format *
	min::standard_attr_flag_format =
    & ::standard_attr_flag_format;

static min::obj_format compact_obj_format =
{
    min::ENABLE_COMPACT,    // obj_op_flags

    NULL,		    // element_format*
    NULL,		    // top_element_format
    NULL,		    // label_format*
    NULL,		    // value_format*

    NULL,		    // initiator_format*
    NULL,		    // separator_format*
    NULL,		    // terminator_format*

    min::standard_str_classifier,
    			    // mark_classifier

    min::left_curly_right_curly_pstring,
			    // obj_empty

    min::left_curly_leading_pstring,
			    // obj_bra
    min::trailing_vbar_leading_pstring,
			    // obj_braend
    min::trailing_vbar_leading_pstring,
			    // obj_ketbegin
    min::trailing_right_curly_pstring,
			    // obj_ket

    min::space_if_none_pstring,
			    // obj_sep

    min::trailing_always_colon_space_pstring,
			    // obj_attrbegin
    min::trailing_always_comma_space_pstring,
			    // obj_attrsep

    min::erase_all_space_colon_pstring,
			    // obj_attreol

    min::space_equal_space_pstring,
			    // obj_attreq

    min::no_space_pstring,  // obj_attrneg

    min::standard_attr_flag_format,
    			    // flag_format
    min::standard_attr_hide_flags,
    			    // hide_flags

    min::left_curly_star_space_pstring,
			    // obj_valbegin
    min::trailing_always_comma_space_pstring,
			    // obj_valsep
    min::space_star_right_curly_pstring,
			    // obj_valend

    min::space_equal_space_pstring,
			    // obj_valreq
};
const min::obj_format * min::compact_obj_format =
    & ::compact_obj_format;

static min::obj_format line_obj_format =
{
      min::ENABLE_COMPACT   // obj_op_flags
    + min::ENABLE_LOGICAL_LINE,

    NULL,		    // element_format*
    NULL,		    // top_element_format*
    NULL,		    // label_format*
    NULL,		    // value_format*

    NULL,		    // initiator_format*
    NULL,		    // separator_format*
    NULL,		    // terminator_format*

    min::standard_str_classifier,
    			    // mark_classifier

    min::left_curly_right_curly_pstring,
			    // obj_empty

    min::left_curly_leading_pstring,
			    // obj_bra
    min::trailing_vbar_leading_pstring,
			    // obj_braend
    min::trailing_vbar_leading_pstring,
			    // obj_ketbegin
    min::trailing_right_curly_pstring,
			    // obj_ket

    min::space_if_none_pstring,
			    // obj_sep

    min::trailing_always_colon_space_pstring,
			    // obj_attrbegin
    min::trailing_always_comma_space_pstring,
			    // obj_attrsep

    min::erase_all_space_colon_pstring,
			    // obj_attreol

    min::space_equal_space_pstring,
			    // obj_attreq

    min::no_space_pstring,  // obj_attrneg

    min::standard_attr_flag_format,
    			    // flag_format
    min::standard_attr_hide_flags,
    			    // hide_flags

    min::left_curly_star_space_pstring,
			    // obj_valbegin
    min::trailing_always_comma_space_pstring,
			    // obj_valsep
    min::space_star_right_curly_pstring,
			    // obj_valend

    min::space_equal_space_pstring,
			    // obj_valreq
};
const min::obj_format *
	min::line_obj_format =
    & ::line_obj_format;

static min::obj_format paragraph_obj_format =
{
      min::ENABLE_COMPACT   // obj_op_flags
    + min::ENABLE_INDENTED_PARAGRAPH,

    NULL,		    // element_format*
    NULL,		    // top_element_format*
    NULL,		    // label_format*
    NULL,		    // value_format*

    NULL,		    // initiator_format*
    NULL,		    // separator_format*
    NULL,		    // terminator_format*

    min::standard_str_classifier,
    			    // mark_classifier

    min::left_curly_right_curly_pstring,
			    // obj_empty

    min::left_curly_leading_pstring,
			    // obj_bra
    min::trailing_vbar_leading_pstring,
			    // obj_braend
    min::trailing_vbar_leading_pstring,
			    // obj_ketbegin
    min::trailing_right_curly_pstring,
			    // obj_ket

    min::space_if_none_pstring,
			    // obj_sep

    min::trailing_always_colon_space_pstring,
			    // obj_attrbegin
    min::trailing_always_comma_space_pstring,
			    // obj_attrsep

    min::erase_all_space_colon_pstring,
			    // obj_attreol

    min::space_equal_space_pstring,
			    // obj_attreq

    min::no_space_pstring,  // obj_attrneg


    min::standard_attr_flag_format,
    			    // flag_format
    min::standard_attr_hide_flags,
    			    // hide_flags

    min::left_curly_star_space_pstring,
			    // obj_valbegin
    min::trailing_always_comma_space_pstring,
			    // obj_valsep
    min::space_star_right_curly_pstring,
			    // obj_valend

    min::space_equal_space_pstring,
			    // obj_valreq
};
const min::obj_format * min::paragraph_obj_format =
    & ::paragraph_obj_format;

static min::obj_format compact_id_obj_format =
{
      min::ENABLE_COMPACT   // obj_op_flags
    + min::DEFERRED_ID,

    NULL,		    // element_format*
    NULL,		    // top_element_format
    NULL,		    // label_format*
    NULL,		    // value_format*

    NULL,		    // initiator_format*
    NULL,		    // separator_format*
    NULL,		    // terminator_format*

    min::standard_str_classifier,
    			    // mark_classifier

    min::left_curly_right_curly_pstring,
			    // obj_empty

    min::left_curly_leading_pstring,
			    // obj_bra
    min::trailing_vbar_leading_pstring,
			    // obj_braend
    min::trailing_vbar_leading_pstring,
			    // obj_ketbegin
    min::trailing_right_curly_pstring,
			    // obj_ket

    min::space_if_none_pstring,
			    // obj_sep

    NULL,		    // obj_attrbegin
    NULL,		    // obj_attrsep

    NULL,		    // obj_attreol

    NULL,		    // obj_attreq

    NULL,		    // obj_attrneg

    NULL,		    // flag_format
    0,  		    // hide_flags

    NULL,		    // obj_valbegin
    NULL,		    // obj_valsep
    NULL,		    // obj_valend

    NULL,		    // obj_valreq

};
const min::obj_format * min::compact_id_obj_format =
    & ::compact_id_obj_format;

static min::obj_format id_obj_format =
{
    min::FORCE_ID,          // obj_op_flags

    NULL,		    // element_format
    NULL,		    // top_element_format
    NULL,		    // label_format
    NULL,		    // value_format

    NULL,		    // initiator_format
    NULL,		    // separator_format
    NULL,		    // terminator_format

    NULL,		    // mark_classifier

    NULL,		    // obj_empty

    NULL,		    // obj_bra
    NULL,		    // obj_braend
    NULL,		    // obj_ketbegin
    NULL,		    // obj_ket

    NULL,		    // obj_sep

    NULL,		    // obj_attrbegin
    NULL,		    // obj_attrsep

    NULL,		    // obj_attreol

    NULL,		    // obj_attreq

    NULL,		    // obj_attrneg

    NULL,		    // flag_format
    0,  		    // hide_flags

    NULL,		    // obj_valbegin
    NULL,		    // obj_valsep
    NULL,		    // obj_valend

    NULL,		    // obj_valreq
};
const min::obj_format * min::id_obj_format =
    & ::id_obj_format;

static min::obj_format embedded_line_obj_format =
{
    min::EMBEDDED_LINE,     // obj_op_flags

    NULL,		    // element_format*
    NULL,		    // top_element_format
    NULL,		    // label_format*
    NULL,		    // value_format*

    NULL,		    // initiator_format*
    NULL,		    // separator_format*
    NULL,		    // terminator_format*

    NULL,		    // mark_classifier

    NULL,                   // obj_empty

    min::left_curly_leading_pstring,
			    // obj_bra
    min::trailing_vbar_leading_pstring,
			    // obj_braend
    min::trailing_vbar_leading_pstring,
			    // obj_ketbegin
    min::trailing_right_curly_pstring,
			    // obj_ket

    min::space_if_none_pstring,
			    // obj_sep

    min::trailing_always_colon_space_pstring,
			    // obj_attrbegin
    min::trailing_always_comma_pstring,
    			    // obj_attrsep

    min::erase_all_space_colon_pstring,
			    // obj_attreol

    min::space_equal_space_pstring,
			    // obj_attreq

    min::no_space_pstring,  // obj_attrneg


    min::standard_attr_flag_format,
    			    // flag_format
    min::standard_attr_hide_flags,
    			    // hide_flags

    min::left_curly_star_space_pstring,
			    // obj_valbegin
    min::trailing_always_comma_space_pstring,
			    // obj_valsep
    min::space_star_right_curly_pstring,
			    // obj_valend

    min::space_equal_space_pstring,
			    // obj_valreq
};
const min::obj_format * min::embedded_line_obj_format =
    & ::embedded_line_obj_format;

static min::obj_format isolated_line_id_obj_format =
{
    min::ISOLATED_LINE,     // obj_op_flags

    NULL,		    // element_format*
    NULL,		    // top_element_format
    NULL,		    // label_format*
    NULL,		    // value_format*

    NULL,		    // initiator_format*
    NULL,		    // separator_format*
    NULL,		    // terminator_format*

    NULL,		    // mark_classifier

    NULL,		    // obj_empty

    NULL,		    // obj_bra
    NULL,		    // obj_braend
    NULL,		    // obj_ketbegin
    NULL,		    // obj_ket

    min::space_if_none_pstring,
			    // obj_sep

    NULL,		    // obj_attrbegin
    NULL,		    // obj_attrsep

    min::erase_all_space_colon_pstring,
			    // obj_attreol

    min::space_equal_space_pstring,
			    // obj_attreq

    min::no_space_pstring,  // obj_attrneg


    min::standard_attr_flag_format,
    			    // flag_format
    0,			    // hide_flags

    min::left_curly_star_space_pstring,
			    // obj_valbegin
    min::trailing_always_comma_space_pstring,
			    // obj_valsep
    min::space_star_right_curly_pstring,
			    // obj_valend

    min::space_equal_space_pstring,
			    // obj_valreq
};
const min::obj_format *
	min::isolated_line_id_obj_format =
    & ::isolated_line_id_obj_format;

static min::gen_format compact_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_separator_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL
};
const min::gen_format * min::compact_gen_format =
    & ::compact_gen_format;

static min::gen_format line_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_separator_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::line_obj_format,
    NULL
};
const min::gen_format *
	min::line_gen_format =
    & ::line_gen_format;

static min::gen_format paragraph_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_separator_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::paragraph_obj_format,
    NULL
};
const min::gen_format *
	min::paragraph_gen_format =
    & ::paragraph_gen_format;

static min::gen_format compact_value_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_value_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL
};
const min::gen_format * min::compact_value_gen_format =
    & ::compact_value_gen_format;

static min::gen_format compact_id_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_value_id_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::compact_id_obj_format,
    NULL
};
const min::gen_format * min::compact_id_gen_format =
    & ::compact_id_gen_format;

static min::gen_format id_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_value_id_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::id_obj_format,
    NULL
};
const min::gen_format * min::id_gen_format =
    & ::id_gen_format;

static min::gen_format id_map_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_value_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::isolated_line_id_obj_format,
    NULL
};
const min::gen_format * min::id_map_gen_format =
    & ::id_map_gen_format;

static min::gen_format name_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_value_str_format,
    & ::name_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL
};
const min::gen_format * min::name_gen_format =
    & ::name_gen_format;

static min::gen_format
	leading_always_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::standard_str_format,
    & ::leading_always_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL
};
const min::gen_format * min::leading_always_gen_format =
    & ::leading_always_gen_format;

static min::gen_format
	trailing_always_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::standard_str_format,
    & ::trailing_always_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL
};
const min::gen_format *
	min::trailing_always_gen_format =
    & ::trailing_always_gen_format;

static min::gen_format
	always_quote_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_all_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL
};
const min::gen_format * min::always_quote_gen_format =
    & ::always_quote_gen_format;

static min::gen_format never_quote_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    NULL,
    & ::name_lab_format,
    & ::name_special_format,
    & ::compact_obj_format,
    NULL
};
const min::gen_format * min::never_quote_gen_format =
    & ::never_quote_gen_format;

static const min::line_format standard_line_format =
{
    min::DISABLE_LINE_BREAKS + min::EXPAND_HT,
    0,
    NULL,
    "<END-OF-FILE>",
    "<UNAVALABLE-LINE>",
    "MIN-LINE-GROUP",
    "MIN-LINE",
    "MIN-LINE-NUMBER",
    NULL
};
const min::line_format * min::standard_line_format =
    & ::standard_line_format;

static const min::line_format marked_line_format =
{
    min::DISABLE_LINE_BREAKS + min::EXPAND_HT,
    '^',
    "<BLANK-LINE>",
    "<END-OF-FILE>",
    "<UNAVALABLE-LINE>",
    "MIN-LINE-GROUP",
    "MIN-LINE",
    "MIN-LINE-NUMBER",
    "MIN-LINE-MARK"
};
const min::line_format * min::marked_line_format =
    & ::marked_line_format;

static const min::line_format picture_line_format =
{
    min::DISABLE_LINE_BREAKS + min::EXPAND_HT
    			     + min::DISPLAY_PICTURE,
    '^',
    "<BLANK-LINE>",
    "<END-OF-FILE>",
    "<UNAVALABLE-LINE>",
    "MIN-LINE-GROUP",
    "MIN-LINE",
    "MIN-LINE-NUMBER",
    "MIN-LINE-MARK"
};
const min::line_format * min::picture_line_format =
    & ::picture_line_format;

static const min::line_format non_graphic_line_format =
{
    min::DISABLE_LINE_BREAKS + min::EXPAND_HT
    			     + min::DISPLAY_PICTURE
    			     + min::DISPLAY_NON_GRAPHIC,
    '^',
    "<BLANK-LINE>",
    "<END-OF-FILE>",
    "<UNAVALABLE-LINE>",
    "MIN-LINE-GROUP",
    "MIN-LINE",
    "MIN-LINE-NUMBER",
    "MIN-LINE-MARK"
};
const min::line_format * min::non_graphic_line_format =
    & ::non_graphic_line_format;

static const min::line_format eol_line_format =
{
    min::DISABLE_LINE_BREAKS + min::EXPAND_HT
    			     + min::DISPLAY_PICTURE
    			     + min::DISPLAY_NON_GRAPHIC
    			     + min::DISPLAY_EOL,
    '^',
    "<BLANK-LINE>",
    "<END-OF-FILE>",
    "<UNAVALABLE-LINE>",
    "MIN-LINE-GROUP",
    "MIN-LINE",
    "MIN-LINE-NUMBER",
    "MIN-LINE-MARK"
};
const min::line_format * min::eol_line_format =
    & ::eol_line_format;

const min::print_format min::default_print_format =
{
    min::EXPAND_HT,
    ::standard_char_flags,

    min::latin1_support_control,
    min::graphic_and_hspace_display_control,
    min::graphic_and_sp_display_control,
    min::break_after_space_break_control,

    & ::standard_char_name_format,
    & ::compact_gen_format,
    & ::standard_line_format,
    10
};

static void init_pgen_formats ( void )
{
    ::name_special_format.special_names =
        min::standard_special_names;

    ::bracket_special_format.special_names =
        min::standard_special_names;

    ::standard_attr_flag_format.flag_names =
        min::standard_attr_flag_names;

    ::compact_obj_format.element_format =
        min::compact_gen_format;
    ::compact_obj_format.label_format =
        min::name_gen_format;
    ::compact_obj_format.value_format =
        min::compact_value_gen_format;
    ::compact_obj_format.initiator_format =
        min::leading_always_gen_format;
    ::compact_obj_format.separator_format =
        min::trailing_always_gen_format;
    ::compact_obj_format.terminator_format =
        min::trailing_always_gen_format;

    ::line_obj_format.element_format =
        min::compact_gen_format;
    ::line_obj_format.top_element_format =
        min::paragraph_gen_format;
    ::line_obj_format.label_format =
        min::name_gen_format;
    ::line_obj_format.value_format =
        min::compact_value_gen_format;
    ::line_obj_format.initiator_format =
        min::leading_always_gen_format;
    ::line_obj_format.separator_format =
        min::trailing_always_gen_format;
    ::line_obj_format.terminator_format =
        min::trailing_always_gen_format;

    ::paragraph_obj_format.element_format =
        min::compact_gen_format;
    ::paragraph_obj_format.top_element_format =
        min::line_gen_format;
    ::paragraph_obj_format.label_format =
        min::name_gen_format;
    ::paragraph_obj_format.value_format =
        min::compact_value_gen_format;
    ::paragraph_obj_format.initiator_format =
        min::leading_always_gen_format;
    ::paragraph_obj_format.separator_format =
        min::trailing_always_gen_format;
    ::paragraph_obj_format.terminator_format =
        min::trailing_always_gen_format;

    ::compact_id_obj_format.element_format =
        min::compact_id_gen_format;
    ::compact_id_obj_format.label_format =
        min::name_gen_format;
    ::compact_id_obj_format.value_format =
        min::compact_id_gen_format;
    ::compact_id_obj_format.initiator_format =
        min::leading_always_gen_format;
    ::compact_id_obj_format.separator_format =
        min::trailing_always_gen_format;
    ::compact_id_obj_format.terminator_format =
        min::trailing_always_gen_format;

    ::embedded_line_obj_format.element_format =
        min::compact_gen_format;
    ::embedded_line_obj_format.label_format =
        min::name_gen_format;
    ::embedded_line_obj_format.value_format =
        min::compact_value_gen_format;
    ::embedded_line_obj_format.initiator_format =
        min::leading_always_gen_format;
    ::embedded_line_obj_format.separator_format =
        min::trailing_always_gen_format;
    ::embedded_line_obj_format.terminator_format =
        min::trailing_always_gen_format;

    ::isolated_line_id_obj_format.element_format =
        min::id_gen_format;
    ::isolated_line_id_obj_format.label_format =
        min::name_gen_format;
    ::isolated_line_id_obj_format.value_format =
        min::id_gen_format;
    ::isolated_line_id_obj_format.initiator_format =
        min::leading_always_gen_format;
    ::isolated_line_id_obj_format.separator_format =
        min::trailing_always_gen_format;
    ::isolated_line_id_obj_format.terminator_format =
        min::trailing_always_gen_format;
}

const min::op min::flush_one_id
    ( min::op::FLUSH_ONE_ID );
const min::op min::flush_id_map
    ( min::op::FLUSH_ID_MAP );

min::printer min::print_id
	( min::printer printer,
	  min::gen v )
{
    if ( printer->id_map == min::NULL_STUB )
    	min::init ( min::id_map_ref ( printer ) );
    min::uns32 ID =
        min::find_or_add ( printer->id_map, v );
    return MINT::print_id ( printer, ID );
}

min::printer min::print_one_id
	( min::printer printer,
	  min::id_map id_map,
	  const min::gen_format * f )
{
    if ( id_map == min::NULL_STUB )
        id_map = printer->id_map;

    if ( id_map == min::NULL_STUB
         ||
	 id_map->next >= id_map->length )
        return printer;

    if ( f == NULL ) f = id_map->id_gen_format;

    min::uns32 ID = id_map->next;
    if ( ID >= id_map->length )
        return printer;

    * ( min::uns32 * ) & id_map->next = ID + 1;

    return min::print_mapped_id
		( printer, ID, id_map, f );
}

min::printer min::print_id_map
	( min::printer printer,
	  min::id_map id_map,
	  const min::gen_format * f )
{
    if ( id_map == min::NULL_STUB )
        id_map = printer->id_map;

    if ( id_map == min::NULL_STUB )
        return printer;

    while ( id_map->next < id_map->length )
        min::print_one_id ( printer, id_map, f );

    return printer;
}

min::printer min::print_mapped_id
	( min::printer printer,
	  min::uns32 ID,
	  min::id_map id_map,
	  const min::gen_format * f )
{
    if ( id_map == min::NULL_STUB )
        id_map = printer->id_map;
    MIN_REQUIRE ( id_map != min::NULL_STUB );

    if ( f == NULL ) f = id_map->id_gen_format;
    MIN_REQUIRE ( f != NULL );

    MIN_REQUIRE ( ID < id_map->length );

    printer << min::bol;
    min::print_str ( printer, id_map->ID_prefix );
    MINT::print_id ( printer, ID );
    min::print_space ( printer );
    printer << id_map->ID_assign;
    min::print_space ( printer );

    printer << min::save_indent;
    min::print_gen ( printer, id_map[ID], f );
    return printer << min::bol << min::restore_indent;
}

min::printer min::print_mapped
	( min::printer printer,
	  min::gen v,
	  min::id_map id_map,
	  const min::gen_format * f )
{
    if ( id_map == min::NULL_STUB )
    {
	if ( printer->id_map == min::NULL_STUB )
	    min::init
		( min::id_map_ref ( printer ) );
        id_map = printer->id_map;
    }

    if ( f == NULL ) f = id_map->id_gen_format;

    min::uns32 ID = min::find_or_add ( id_map, v );

    if ( ID < id_map->next )
	min::print_mapped_id ( printer, ID, id_map, f );

    return min::print_id_map ( printer, id_map, f );
}

// Return true if attributes printed and false if
// nothing printed.  If line_format is false and
// an attribute is printed, this has begun with
//
//   printer << objf->obj_attrbegin
//           << min::save_indent;
//
// and NOT done a corresponding min::restore_indent.
//
static bool print_attributes
	( min::printer printer, 
	  const min::obj_format * objf,
	  min::obj_vec_ptr & vp,
	  min::attr_ptr & ap,
	  min::attr_info * info,
	  min::unsptr m,
	  min::uns32 line_format,
	  min::gen type = min::NONE() )
{
    bool first_attr = true;
    min::int32 adjust =
	  (min::int32) printer->line_break.line_length
	- (min::int32) printer->line_break.indent;
    adjust = ( adjust > 20 ? 4 : 2 );
    for ( min::unsptr i = 0; i < m; ++ i )
    {
	if ( info[i].flags & objf->hide_flags )
	    continue;
	min::unsptr vc = info[i].value_count;
	min::unsptr rc = info[i].reverse_attr_count;
	min::unsptr fc = info[i].flag_count;
	if ( vc == 0 && rc == 0 && fc == 0 )
	    continue;
	else if ( info[i].name == min::dot_type
		  &&
		  vc == 1
		  &&
		  info[i].value == type )
	    continue;

	if ( first_attr )
	{
	    first_attr = false;
	    if ( line_format )
	    {
	        printer << (    line_format
		             == min::EMBEDDED_LINE ?
			     objf->obj_ketbegin :
			     objf->obj_attreol )
			<< min::eol << min::indent
			<< min::adjust_indent
			      ( adjust );
	    }
	    else
		printer << objf->obj_attrbegin
		        << min::save_indent;
	}
	else
	{
	    if ( line_format )
	    {
	        if (    line_format
		     == min::EMBEDDED_LINE )
		    printer << objf->obj_attrsep;
		printer << min::adjust_indent
			      ( - adjust )
	                << min::indent
			<< min::adjust_indent
			      ( adjust );
	    }
	    else
	    {
		printer << objf->obj_attrsep
		        << min::set_break;
	    }
	}

	bool suppress_value =
	    ( vc == 1 && rc == 0 && fc == 0 );
	if ( suppress_value )
	{
	    if ( info[i].value == min::FALSE )
		printer << objf->obj_attrneg;
	    else if ( info[i].value != min::TRUE )
		suppress_value = false;
	}

	min::print_gen
	    ( printer, info[i].name,
		       objf->label_format );

	if ( suppress_value ) continue;
 
	if ( vc > 1 || rc > 0 || fc * min::VSIZE > 64 )
		min::locate ( ap, info[i].name );

	if ( fc > 0 )
	{
	    const min::flag_format * ff =
	        objf->flag_format;
	    min::packed_vec_ptr<min::ustring> names =
	        ff->flag_names;
	    min::unsptr length =
	        ( names == min::NULL_STUB ?
		  0 : names->length );

	    printer << ff->flag_prefix;
	    min::print_item_preface
	        ( printer, min::IS_GRAPHIC );
	    if ( fc * min::VSIZE <= 64 && length >= 64 )
	    {
		min::uns64 f = info[i].flags;
		for ( unsigned j = 0; j < 64; ++ j )
		{
		    if ( f & 1 )
		        min::print_ustring
			    ( printer, names[j] );
		    f >>= 1;
		}
	    }
	    else
	    {
		min::gen flags[fc];
		min::get_flags ( flags, fc, ap );
		min::uns32 n = 0;
		bool first = true;
		for ( min::unsptr j = 0;
		      j < fc; ++ j )
		{
		    min::unsgen flags2 =
			MUP::control_code_of
			    ( flags[j] );
		    for ( unsigned k = 0;
			  k < min::VSIZE; ++ k )
		    {
			if ( flags2 & 1 )
			{

			    if ( n < length )
				min::print_ustring
				    ( printer,
				      names[n] );
			    else
			    {
			        char buffer[100];
				min::uns32 m = sprintf
				    ( buffer, ",%u",
				      n );
				min::print_chars
				    ( printer,
				      buffer + first,
				      m - first,
				      m - first );
			    }
			    first = false;
			}
			flags2 >>= 1;
			++ n;
		    }
		}
	    }
	    printer << ff->flag_postfix;
	}

	if ( vc > 0 )
	{
	    printer << objf->obj_attreq
	            << min::set_break;

	    min::gen value[vc];
	    if ( vc > 1 ) min::get ( value, vc, ap );
	    else if ( vc == 1 )
	        value[0] = info[i].value;
	    if ( vc > 1 )
		printer << objf->obj_valbegin
			<< min::save_indent;

	    bool first_value = true;
	    for ( min::unsptr j = 0; j < vc; ++ j )
	    {
		if ( first_value ) first_value = false;
		else
		    printer << objf->obj_valsep
			    << min::set_break;
		min::print_gen
		    ( printer, value[j],
			       objf->value_format );
	    }
	    if ( vc > 1 )
		printer << objf->obj_valend
			<< min::restore_indent;
	}

	if ( rc > 0 )
	{
	    min::reverse_attr_info rinfo[rc];
	    min::reverse_attr_info_of ( rinfo, rc, ap );

	    for ( min::unsptr j = 0; j < rc; ++ j )
	    {
		min::unsptr rvc = rinfo[j].value_count;
		min::gen rvalue[rvc];
		if ( rvc > 1 )
		{
		    min::locate_reverse
			( ap, rinfo[j].name );
		    min::get ( rvalue, rvc, ap );
		}
		else
		    rvalue[0] = rinfo[j].value;

		if ( vc > 0 || j != 0 )
		{
		    if ( line_format )
			printer << min::adjust_indent
				      ( - adjust )
				<< min::indent
				<< min::adjust_indent
				      ( adjust );
		    else
		    {
			printer << objf->obj_attrsep
				<< min::set_break;
		    }
		    min::print_gen
			( printer, info[i].name,
				   objf->label_format );
		    printer << objf->obj_attreq
			    << min::set_break;
		}
		else if ( vc == 0 )
		    printer << objf->obj_attreq
			    << min::set_break;

		if ( rvc > 1 )
		    printer << objf->obj_valbegin
			    << min::save_indent;

		bool first_rvalue = true;
		for ( min::unsptr k = 0; k < rvc; ++ k )
		{
		    if ( first_rvalue )
			first_rvalue = false;
		    else
			printer << objf->obj_valsep
				<< min::set_break;
		    min::print_id
		        ( printer, rvalue[k] );
		}

		if ( rvc > 1 )
		    printer << objf->obj_valend
			    << min::restore_indent;
		printer << min::set_break
			<< objf->obj_valreq;
		min::print_gen
		    ( printer, rinfo[j].name,
			       objf->label_format );
	    }
	}
    }

    if ( ! first_attr && line_format )
	printer << min::adjust_indent ( - adjust );

    return ! first_attr;
}

min::printer min::print_obj
	( min::printer printer,
	  min::gen v,
	  const min::gen_format * f,
	  const min::obj_format * objf,
	  min::uns32 obj_op_flags,
	  bool disable_mapping,
	  min::unsptr max_attrs )
{
    min::uns32 saved_printer_state = printer->state;
    printer->state &=
        ~ (   min::IN_PARAGRAPH
	    + min::IN_LOGICAL_LINE
	    + min::AT_LOGICAL_LINE_END
	    + min::AFTER_LINE_TERMINATOR
	    + min::AFTER_PARAGRAPH 
	    + min::CONTAINS_PARAGRAPH );

    min::obj_vec_ptr vp ( v );
    min::unsptr vsize = min::size_of ( vp );
    min::attr_ptr ap ( vp );

    min::attr_info info[max_attrs];
    min::unsptr m =
        min::attr_info_of ( info, max_attrs, ap );
    if ( m > max_attrs )
        return min::print_obj
	    ( printer, v, f, objf, obj_op_flags,
	      disable_mapping, m );

    min::gen separator = min::NONE();
    min::gen initiator = min::NONE();
    min::gen terminator = min::NONE();
    min::gen type = min::NONE();

    // Figure out if type is needed for normal format
    // or embedded line format.
    //
    bool need_type =
        ( ( obj_op_flags & min::ISOLATED_LINE ) == 0 );

    // Loop until we determine compact_format and
    // need_type are BOTH false.
    //
    bool compact_format =
	( obj_op_flags & min::ENABLE_COMPACT );

    for ( min::unsptr i = 0;
             ( compact_format || need_type )
	  && i < m;
	  ++ i )
    {
        if ( info[i].flags & objf->hide_flags )
	    continue;
	min::unsptr vc = info[i].value_count;
	min::unsptr rc = info[i].reverse_attr_count;
	min::unsptr fc = info[i].flag_count;
	if ( vc == 0 && rc == 0 && fc == 0 )
	    continue;

	if ( vc != 1 || fc != 0 || rc != 0 )
	    compact_format = false;
	else if ( ! min::is_str ( info[i].value )
	          &&
		  ! ::is_str_lab ( info[i].value )
		  &&
	          ! ( (   obj_op_flags
		        & min::ENABLE_LOGICAL_LINE )
		      &&
		      info[i].name == min::dot_initiator
		      &&
		         info[i].value
		      == min::LOGICAL_LINE() )
		  &&
	          ! ( (   obj_op_flags
		        & min::
			  ENABLE_INDENTED_PARAGRAPH )
		      &&
		         info[i].name
		      == min::dot_terminator
		      &&
		         info[i].value
		      == min::INDENTED_PARAGRAPH() ) )
	    compact_format = false;
        else if ( info[i].name == min::dot_type )
	{
	    type = info[i].value;
	    need_type = false;
	}
        else if ( info[i].name == min::dot_separator )
	    separator = info[i].value;
        else if ( info[i].name == min::dot_initiator )
	    initiator = info[i].value;
        else if ( info[i].name == min::dot_terminator )
	    terminator = info[i].value;
	else compact_format = false;
    }

    // Compact_ok does not allow just one of initiator
    // and terminator.
    //
    if ( compact_format )
    {
	if ( initiator != min::NONE() )
	{
	    if ( terminator == min::NONE()
	         ||
		 type != min::NONE() )
	        compact_format = false;
	}
	else if ( terminator != min::NONE() )
	    compact_format = false;
    }

    if ( ( obj_op_flags & min::FORCE_ID )
         ||
	 (    ! compact_format
           && ( obj_op_flags & min::DEFERRED_ID ) ) )
    {
        min::uns32 ID =
	    min::find_or_add ( printer->id_map, v );
	if ( ID != 0 )
	    return MINT::print_id ( printer, ID );
    }

    bool isolated_line_format =
           ! compact_format
        && ( obj_op_flags & min::ISOLATED_LINE );

    bool embedded_line_format =
           ! compact_format
        && ! isolated_line_format
        && ( obj_op_flags & min::EMBEDDED_LINE );

    printer << min::save_print_format
            << min::no_auto_break
            << min::set_break;

    min::gen mark_end_type = min::NONE();
    bool attributes_printed = false;
    if ( compact_format )
    {

	if (    type != min::NONE()
	     && separator == min::NONE()
	     && ! disable_mapping
	     &&    MINT::defined_format_type_map
		!= min::MISSING() )
	{
	    min::gen dfg =
		min::get
		    ( MINT::defined_format_type_map,
		      type );
	    if ( dfg != min::NONE() )
	    {
		min::defined_format df =
		    (min::defined_format)
		    min::stub_of ( dfg );
		if ( df != min::NULL_STUB )
		{
		    vp = min::NULL_STUB;
		    ( * df->defined_format_function )
			( printer, v, f, df );
		    return printer
		           << min::restore_print_format;
		}
	    }
	}

	if ( obj_op_flags & min::ENABLE_LOGICAL_LINE
	     &&
	     initiator == min::LOGICAL_LINE()
	     &&
	     separator == min::NONE() )
	{
	    min::uns32 state = min::IN_LOGICAL_LINE;
	        // Add min::CONTAINS_PARAGRAPH when
		// appropriate.
	    for ( min::unsptr i = 0; i < vsize; ++ i )
	    {
		if ( i != 0 )
		{
		    if (   printer->state
			 & min::AFTER_PARAGRAPH )
		    {
			printer->state &=
			    ~ min::AFTER_PARAGRAPH;
			state |=
			    min::CONTAINS_PARAGRAPH;
		        printer << min::indent;
		    }
		    else
			min::print_space ( printer );

		    if ( i == vsize - 1
		         &&
			 terminator == min::line_feed )
		        printer->state |=
			    min::AT_LOGICAL_LINE_END;
		}
		printer << min::set_break;

		printer->state |= state;

		min::print_gen
		    ( printer,
		      vp[i], objf->top_element_format );
	    }
	    printer->state &=
	        ~ (   min::IN_LOGICAL_LINE
		    + min::AT_LOGICAL_LINE_END
		    + min::AFTER_PARAGRAPH
		    + min::CONTAINS_PARAGRAPH );

	    if ( terminator != min::line_feed )
	    {
		min::print_trailing ( printer );
		min::print_gen
		    ( printer, terminator,
		      objf->terminator_format );
		if (   saved_printer_state
		     & min::IN_PARAGRAPH )
		    printer->state |=
			min::AFTER_LINE_TERMINATOR;
	    }
	    return printer << min::restore_print_format;
	}
	else if ( (   obj_op_flags
	            & min::ENABLE_INDENTED_PARAGRAPH )
	          &&
	             terminator
	          == min::INDENTED_PARAGRAPH()
		  &&
		  separator == min::NONE()
		  &&
		  (   saved_printer_state
		    & min::IN_LOGICAL_LINE ) )
	{
	    min::print_erase_space
		( printer, printer->column );
	    min::print_gen
		( printer, initiator,
		  objf->terminator_format );
	    printer << min::eol;
		    // As we are printing this from
		    // inside a line, indent is already
		    // adjusted to line indent + 4.

	    bool extra_indent =
	        (   saved_printer_state
		  & min::CONTAINS_PARAGRAPH )
		||
		! (   saved_printer_state
		    & min::AT_LOGICAL_LINE_END );

	    if ( extra_indent )
	        printer << min::save_line_break
		        << min::adjust_indent ( +4 );

	    bool indent_saved = false;
	    for ( min::unsptr i = 0; i < vsize; ++ i )
	    {
		if (   printer->state
	             & min::AFTER_LINE_TERMINATOR )
		{
		    printer->state &=
			~ min::AFTER_LINE_TERMINATOR;
		    min::print_space ( printer );
		}
		else
		{
		    if ( indent_saved )
		        printer << min::restore_indent;

		    printer << min::indent
		            << min::save_indent
			    << min::place_indent ( 4 );

		    indent_saved = true;
		}

		printer->state |= min::IN_PARAGRAPH;
		min::print_gen
		    ( printer,
		      vp[i], objf->top_element_format );
	    }
	    printer->state &=
	        ~ (   min::IN_PARAGRAPH
	            + min::AFTER_LINE_TERMINATOR );


	    if ( indent_saved )
		printer << min::restore_indent;

	    if ( extra_indent )
		printer << min::restore_line_break;

	    if (   saved_printer_state
		 & min::IN_LOGICAL_LINE )
		printer->state |= min::AFTER_PARAGRAPH;

	    return printer << min::bol
	                   << min::restore_print_format;
	}
        else if ( initiator != min::NONE() )
	{
	    min::print_gen
		( printer, initiator,
		  objf->initiator_format );
	    min::print_leading ( printer );
	}
	else if ( vsize == 0 && type == NONE() )
	    return
	        printer << objf->obj_empty
			<< min::restore_print_format;
	else
	{
	    printer << objf->obj_bra;

	    const min::print_format & pf =
	        printer->print_format;
	    min::gen mark_begin_type = min::NONE();

	    if ( objf->mark_classifier == NULL )
	        /* Do nothing. */;
	    else if ( min::is_str ( type ) )
	    {
	        if (   min::IS_MARK
		     & min::str_class
			 ( pf.char_flags,
			   pf.support_control, 
			   type,
			   objf->mark_classifier ) )
		{
		    mark_begin_type = type;
		    mark_end_type = type;
		}
	    }
	    else if ( min::is_lab ( type ) )
	    {
	        min::lab_ptr lab = type;
		if ( lablen ( lab ) == 2
		     &&
		     min::is_str ( lab[0] )
		     &&
		     min::is_str ( lab[1] ) )
		{
		    min::uns32 class0 =
		       min::str_class
			   ( pf.char_flags,
			     pf.support_control, 
			     lab[0],
			     objf->mark_classifier );
		    min::uns32 class1 =
		       min::str_class
			   ( pf.char_flags,
			     pf.support_control, 
			     lab[1],
			     objf->mark_classifier );
		    if ( ( ( min::IS_MARK & class0 )
		           &&
			   ( min::IS_MARK & class1 ) )
			 ||
			 ( ( min::IS_LEADING & class0 )
			   &&
			   ( min::IS_TRAILING & class1 )
			 ) )
		    {
			mark_begin_type = lab[0];
			mark_end_type = lab[1];
		    }
		}
	    }

	    bool is_prefix =
	        ( vsize == 0
	          &&
		  mark_begin_type == mark_end_type );
	        

	    if ( mark_begin_type != min::NONE() )
	    {
		min::print_gen
		    ( printer, mark_begin_type,
		      min::never_quote_gen_format );
		if ( ! is_prefix )
		    min::print_space ( printer );
	    }
	    else
	    {
	        if ( type != min::NONE() )
		    min::print_gen
			( printer, type,
			  objf->label_format );
		if ( ! is_prefix )
		    printer << objf->obj_braend;
	    }
	    if ( is_prefix )
	        return printer
		    << objf->obj_ket
		    << min::restore_print_format;
	}
    }
    else if ( embedded_line_format )
    {
	printer << min::indent;
	printer << objf->obj_bra;
	if ( type != min::NONE() )
	    min::print_gen
		( printer, type, objf->label_format );
	printer << objf->obj_braend;
    }
    else if ( ! isolated_line_format )
    {
	printer << objf->obj_bra;
	min::print_gen
	    ( printer, type != min::NONE() ?
		       type : min::empty_str,
		       objf->label_format );
	attributes_printed = ::print_attributes
	    ( printer, objf, vp, ap, info, m,
	      0, type );
	if ( vsize == 0 )
	{
	    if ( attributes_printed )
	         printer << min::restore_indent;
	    return printer << objf->obj_ket
		           << min::restore_print_format;
	}
        printer << objf->obj_braend;
    }

    bool first = true;
    for ( min::unsptr i = 0; i < vsize; ++ i )
    {
        if ( first ) 
	{
	    first = false;
	    if ( isolated_line_format )
		printer << min::save_indent
			<< min::adjust_indent ( 8 );
	    else if ( embedded_line_format )
		printer << min::save_indent;
	    else if ( compact_format )
		printer << min::save_indent;
	    else
		printer << min::set_break;
	}
	else
	{
	    if (    ! compact_format
	         || separator == min::NONE() )
		printer << objf->obj_sep;
	    else
	    {
		min::print_trailing_always ( printer );
		min::print_gen
		    ( printer, separator,
	              objf->separator_format );
		min::print_space ( printer );
	    }
	    printer << min::set_break;
	}

	min::print_gen
	    ( printer, vp[i], objf->element_format );
    }

    if ( embedded_line_format )
    {
	if ( first )
	    printer << min::save_line_break
	            << min::adjust_indent ( 4 );

	attributes_printed = ::print_attributes
		( printer, objf, vp, ap, info, m,
		  min::EMBEDDED_LINE, type );
	if ( attributes_printed )
	    printer << min::restore_indent
	            << min::indent;
	else
	    printer << objf->obj_ketbegin;
	if (    (   obj_op_flags
		  & min::NO_TRAILING_TYPE )
	     == 0
	     &&
	     type != min::NONE() )
	{
	    if ( attributes_printed )
		printer << objf->obj_attrbegin;
	    min::print_gen
		( printer, type, objf->label_format );
	}
	printer << objf->obj_ket;
	if ( ! attributes_printed )
	    printer << min::restore_indent;
	printer << min::eol;
    }
    else if ( isolated_line_format )
    {
	if ( ! first ) printer << min::restore_indent;

	printer << min::adjust_indent ( 4 );
        ::print_attributes
	    ( printer, objf, vp, ap, info, m,
	      min::ISOLATED_LINE );
	printer << min::adjust_indent ( -4 );
	printer << min::eol;
    }
    else
    {
	if (    compact_format
	     && terminator != min::NONE() )
	{
	    min::print_trailing ( printer );
	    min::print_gen
	        ( printer, terminator,
		  objf->terminator_format );
	}
	else if ( mark_end_type != min::NONE() )
	{
	    min::print_space ( printer );
	    min::print_gen
		( printer, mark_end_type,
		  min::never_quote_gen_format );
	    printer << objf->obj_ket;
	}
	else
	{
	    printer << objf->obj_ketbegin;
	    if (    (   obj_op_flags
	              & min::NO_TRAILING_TYPE )
		 == 0
		 &&
		 type != min::NONE() )
		min::print_gen
		    ( printer, type,
		      objf->label_format );
	    printer << objf->obj_ket;
	}

	if (    ( ! first && compact_format )
	     || attributes_printed )
	    printer << min::restore_indent;
    }

    return printer << min::restore_print_format;
}

// Execute min::pgen.
//
min::printer min::standard_pgen
	( min::printer printer,
	  min::gen v,
	  const min::gen_format * f,
	  bool disable_mapping )
{
    if (   printer->depth
         > printer->print_format.max_depth )
        return min::print_item ( printer, "...", 3, 3 );

    if ( v == min::new_stub_gen ( MINT::null_stub ) )
    {
        // This must be first as MINT::null_stub
	// may not exist and therefore cannot have its
	// type tested.
	//
	static char buffer[] =
	    "new_stub_gen ( MINT::null_stub )";
	min::uns32 n = ::strlen ( buffer );
	return min::print_item
	    ( printer, buffer, n, n );
    }
    else if ( min::is_num ( v ) )
    {
        min::float64 value = MUP::float_of ( v );
	const min::num_format * nf = f->num_format;
	return min::print_num ( printer, value, nf );
    }
    else if ( min::is_str ( v ) )
    {
	const min::str_format * sf = f->str_format;
	if ( sf != NULL )
	{
	    min::uns32 id_strlen = sf->id_strlen;
	    if ( id_strlen < min::max_id_strlen )
		id_strlen = min::max_id_strlen;
	    if ( min::strlen ( v ) > id_strlen )
	    {
	        min::uns32 ID =
		     min::find_or_add
		         ( printer->id_map, v );
		if ( ID != 0 )
		    return MINT::print_id
		               ( printer, ID );
	    }
	}

	return min::print_str
	    ( printer, v, f->str_format );
    }
    else if ( min::is_lab ( v ) )
    {
        const min::lab_format * lf = f->lab_format;
	if ( lf == NULL ) lf = min::bracket_lab_format;

	MUP::lab_ptr labp ( MUP::stub_of ( v ) );
        min::uns32 len = min::lablen ( labp );

	printer << lf->lab_prefix;

	for ( min::unsptr i = 0; i < len; ++ i )
	{
	    if ( i != 0 ) printer << lf->lab_separator;
	    min::print_gen ( printer, labp[i], f );
	}

	printer << lf->lab_postfix;

	return printer;
    }
    else if ( min::is_special ( v ) )
    {
	const min::special_format * sf =
	    f->special_format;
	if ( sf == NULL )
	    sf = min::bracket_special_format;
        min::unsgen index = MUP::special_index_of ( v );
	min::packed_vec_ptr<min::ustring>
		special_names =
	    sf ? sf->special_names
	       : (min::packed_vec_ptr<min::ustring>)
	         min::NULL_STUB;

	printer << sf->special_prefix;

	min::print_item_preface
	    ( printer, min::IS_GRAPHIC );
	min::unsgen j = 0x1000000 - index;
	char buffer[64];
	if ( special_names != min::NULL_STUB
	     &&
	     1 <= j
	     &&
             j < special_names->length )
	{
	    min::print_item_preface
	        ( printer, min::IS_GRAPHIC );
	    min::print_ustring
	        ( printer, special_names[j] );
	}
	else
	{
	    min::uns32 n =
	        sprintf ( buffer, "SPECIAL %ld",
		          index < 0x800000 ?
		          (long) index :
			  - (long) j );
	    min::print_item ( printer, buffer, n, n );
	}

	printer << sf->special_postfix;
	return printer;
    }

    else if ( min::is_obj ( v ) )
        return min::print_obj
	    ( printer, v, f, f->obj_format,
	      f->obj_format->obj_op_flags,
	      disable_mapping );

    else if ( min::is_preallocated ( v ) )
    {
        min::uns32 id =
	    min::id_of_preallocated ( v );
        min::uns32 count =
	    min::count_of_preallocated ( v );
	char buffer[100];
	min::uns32 n =
	    sprintf ( buffer,
	              "PREALLOCATED(%u*%u)",
		      count, id );
	return min::print_item
	    ( printer, buffer, n, n );
    }

    else if ( min::is_stub ( v ) )
    {
        const min::stub * s = MUP::stub_of ( v );
	int type = min::type_of ( s );
	const char * type_name = min::type_name[type];
	char buffer[100];
	if ( type_name == NULL )
	{
	    sprintf ( buffer, "TYPE(%d)", type );
	    type_name = buffer;
	}
	min::uns32 n = ::strlen ( type_name );
	return min::print_item
	    ( printer, type_name, n, n );
    }
    else
    {
	min::uns64 index;
	const char * type;
	int length = min::VSIZE / 4;
	    // Number hex digits.

	if ( min::is_list_aux ( v ) )
	    type = "LIST_AUX",
	    index = MUP::list_aux_of ( v );
	else if ( min::is_sublist_aux ( v ) )
	    type = "SUBLIST_AUX",
	    index = MUP::sublist_aux_of ( v );
	else if ( min::is_indirect_aux ( v ) )
	    type = "INDIRECT_AUX",
	    index = MUP::indirect_aux_of ( v );
	else if ( min::is_index ( v ) )
	    type = "INDEX",
	    index = MUP::index_of ( v );
	else if ( min::is_control_code ( v ) )
	    type = "CONTROL_CODE",
	    index = MUP::control_code_of ( v );
	else
	    type = "UNDEFINED_GEN",
	    index = MUP::value_of ( v ),
	    length = sizeof ( min::unsptr ) / 2;

        char buffer[100];
	sprintf ( buffer, "%s(0x%0*llX)",
	          type, length, index );

	const min::special_format * sf =
	    f->special_format;

	if ( sf ) printer << sf->special_prefix;
	min::print_cstring ( printer, buffer );
	if ( sf ) printer << sf->special_postfix;
	return printer;
    }
}

// Defined Formats
// ------- -------

static min::packed_vec
	    <min::gen,min::defined_format_header>
	defined_format_type
    ( "min::defined_format_type",
      ::zero_disp );
           

min::defined_format_insptr min::new_defined_format
    ( min::defined_format_function f,
      min::uns32 number_of_arguments )
{
    min::defined_format_insptr p =
        (min::defined_format_insptr)
        ::defined_format_type.new_stub
	    ( number_of_arguments );

    * (min::defined_format_function *) &
      ( p->defined_format_function) = f;

    return p;
}

min::locatable_var
	<MINT::defined_format_packed_map_insptr>
    MINT::defined_format_packed_map
	( min::NULL_STUB );

static min::packed_vec<min::defined_format>
	defined_format_packed_map_type
    ( "::defined_format_packed_map_type",
      NULL, ::zero_disp );

void min::map_packed_subtype
    ( min::uns32 subtype,
      min::defined_format defined_format )
{
    if (    MINT::defined_format_packed_map
         == min::NULL_STUB )
    {
      MINT::defined_format_packed_map =
	(MINT::defined_format_packed_map_insptr)
	::defined_format_packed_map_type.new_stub();
    }
    while (    MINT::defined_format_packed_map->
    			length
            <= subtype )
	min::push(MINT::defined_format_packed_map) =
	    min::NULL_STUB;

    MINT::defined_format_packed_map[subtype] =
        defined_format;
}

min::locatable_gen MINT::defined_format_type_map
				( min::MISSING() );

void min::map_type
    ( min::gen type,
      min::defined_format defined_format )
{
    if (    MINT::defined_format_type_map
         == min::MISSING() )
        MINT::defined_format_type_map =
	    min::new_obj_gen ( 4096, 1024 );
    min::set ( MINT::defined_format_type_map, type,
               min::new_stub_gen ( defined_format ) );
}

min::printer min::quote_defined_format_function
	( min::printer printer,
	  min::gen v,
	  const min::gen_format * gen_format,
	  min::defined_format )
{
    min::obj_vec_ptr vp ( v );
    if ( min::size_of ( vp ) != 1
         ||
	 ! min::is_str ( vp[0] ) )
    {
        vp = min::NULL_STUB;
        return min::print_gen
	    ( printer, v, gen_format, true );
    }
    else
	return min::print_str
	    ( printer, vp[0],
	      min::quote_all_str_format );
}

static void defined_format_initialize ( void )
{
    min::locatable_var<min::defined_format>
        quote_defined_format =
	    min::new_defined_format
		( min::quote_defined_format_function,
		  0 );
    min::map_type
        ( min::doublequote, quote_defined_format );
}

static min::initializer defined_format_initializer
    ( :: defined_format_initialize );
