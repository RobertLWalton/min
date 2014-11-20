// MIN Language Out-of-Line Code
//
// File:	min.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Nov 20 14:47:42 EST 2014
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
//	Objects
//	Object Vector Level
//	Object List Level
//	Object Attribute Level
//	Printers
//	Printing General values

// Setup
// -----

# include <min.h>
# include <min_os.h>
# include <cstdlib>
# include <cstdio>
# include <cmath>
# include <cerrno>
# include <cctype>
# define MUP min::unprotected
# define MINT min::internal
# define UNI min::unicode

# define ERR min::init ( min::error_message ) \
    << min::set_indent ( 7 ) << "ERROR: "

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

static char const * type_name_vector[256];
char const ** min::type_name = type_name_vector + 128;

min::locatable_gen min::TRUE;
min::locatable_gen min::FALSE;
min::locatable_gen min::dot_initiator;
min::locatable_gen min::dot_separator;
min::locatable_gen min::dot_terminator;
min::locatable_gen min::dot_position;
min::locatable_gen min::dot_type;
min::locatable_gen min::new_line;
min::locatable_gen min::empty_string;
min::locatable_gen min::doublequote;
min::locatable_gen min::number_sign;

// Deprecated
//
min::locatable_gen min::dot_middle;
min::locatable_gen min::dot_name;
min::locatable_gen min::dot_arguments;
min::locatable_gen min::dot_keys;
min::locatable_gen min::dot_operator;

static min::uns32 gen_element_disp[2] =
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
min::packed_vec<min::gen> min::gen_packed_vec_type
    ( "min::gen_packed_vec_type",
      gen_element_disp );

static const unsigned standard_special_names_length = 8;
static char const * standard_special_names_value
		  [::standard_special_names_length] =
    { "MISSING", "NONE", "ANY", "MULTI_VALUED",
      "UNDEFINED", "SUCCESS", "FAILURE", "ERROR" };
min::packed_vec_ptr<const char *>
    min::standard_special_names;
static min::locatable_var
	< min::packed_vec_ptr<const char *> >
    standard_special_names;

static const unsigned
    standard_attr_flag_names_length = 256;
static char const *
    standard_attr_flag_names_value
	[::standard_attr_flag_names_length] =
    { "!", "#", "$", "%", "&", "*",
      "+", "-", "/", "=", "?", "@",

      "a", "b", "c", "d", "e", "f", "g",
      "h", "i", "j", "k", "l", "m",
      "n", "o", "p", "q", "r", "s", "t",
      "u", "v", "w", "x", "y", "z",

      "A", "B", "C", "D", "E", "F", "G",
      "H", "I", "J", "K", "L", "M",
      "N", "O", "P", "Q", "R", "S", "T",
      "U", "V", "W", "X", "Y", "Z",

      "~!", "~#", "~$", "~%", "~&", "~*",
      "~+", "~-", "~/", "~=", "~?", "~@",

      "~a", "~b", "~c", "~d", "~e", "~f", "~g",
      "~h", "~i", "~j", "~k", "~l", "~m",
      "~n", "~o", "~p", "~q", "~r", "~s", "~t",
      "~u", "~v", "~w", "~x", "~y", "~z",

      "~A", "~B", "~C", "~D", "~E", "~F", "~G",
      "~H", "~I", "~J", "~K", "~L", "~M",
      "~N", "~O", "~P", "~Q", "~R", "~S", "~T",
      "~U", "~V", "~W", "~X", "~Y", "~Z",

      "^!", "^#", "^$", "^%", "^&", "^*",
      "^+", "^-", "^/", "^=", "^?", "^@",

      "^a", "^b", "^c", "^d", "^e", "^f", "^g",
      "^h", "^i", "^j", "^k", "^l", "^m",
      "^n", "^o", "^p", "^q", "^r", "^s", "^t",
      "^u", "^v", "^w", "^x", "^y", "^z",

      "^A", "^B", "^C", "^D", "^E", "^F", "^G",
      "^H", "^I", "^J", "^K", "^L", "^M",
      "^N", "^O", "^P", "^Q", "^R", "^S", "^T",
      "^U", "^V", "^W", "^X", "^Y", "^Z",

      "\\!", "\\#", "\\$", "\\%", "\\&", "\\*",
      "\\+", "\\-", "\\/", "\\=", "\\?", "\\@",

      "\\a", "\\b", "\\c", "\\d", "\\e", "\\f", "\\g",
      "\\h", "\\i", "\\j", "\\k", "\\l", "\\m",
      "\\n", "\\o", "\\p", "\\q", "\\r", "\\s", "\\t",
      "\\u", "\\v", "\\w", "\\x", "\\y", "\\z",

      "\\A", "\\B", "\\C", "\\D", "\\E", "\\F", "\\G",
      "\\H", "\\I", "\\J", "\\K", "\\L", "\\M",
      "\\N", "\\O", "\\P", "\\Q", "\\R", "\\S", "\\T",
      "\\U", "\\V", "\\W", "\\X", "\\Y", "\\Z"
    };
min::packed_vec_ptr<const char *>
    min::standard_attr_flag_names;
static min::locatable_var
	< min::packed_vec_ptr<const char *> >
    standard_attr_flag_names;

static void lab_scavenger_routine
	( MINT::scavenge_control & sc );
static void packed_struct_scavenger_routine
	( MINT::scavenge_control & sc );
static void packed_vec_scavenger_routine
	( MINT::scavenge_control & sc );
static void obj_scavenger_routine
	( MINT::scavenge_control & sc );

#define PTR_CHECK(...) \
    assert (    sizeof ( __VA_ARGS__ ) \
             == sizeof ( min::stub * ) ); \
    assert ( ( __VA_ARGS__::DISP() == 0 ) );

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

    type_name[ACC_FREE] = "ACC_FREE";
    type_name[DEALLOCATED] = "DEALLOCATED";
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

    assert ( sizeof ( MIN_INT32_TYPE ) == 4 );
    assert ( sizeof ( MIN_INT64_TYPE ) == 8 );
    assert
        ( sizeof ( void * ) == MIN_PTR_BITS / 8 );

    assert
      ( MIN_ABSOLUTE_MAX_FIXED_BLOCK_SIZE_LOG <= 33 );

    assert
      (     sizeof ( MINT::tiny_obj )
         == MINT::obj_header_size ( TINY_OBJ ) );
    assert
      (     sizeof ( MINT::short_obj )
         == MINT::obj_header_size ( SHORT_OBJ ) );
    assert
      (     sizeof ( MINT::long_obj )
         == MINT::obj_header_size ( LONG_OBJ ) );
    assert
      (     sizeof ( MINT::huge_obj )
         == MINT::obj_header_size ( HUGE_OBJ ) );

    uns32 u = 1;
    char * up = (char *) & u;
    bool big_endian = ( up[3] == 1 );
    bool little_endian = ( up[0] == 1 );
    assert ( MIN_IS_BIG_ENDIAN == big_endian );
    assert ( MIN_IS_LITTLE_ENDIAN == little_endian );

#   if MIN_IS_LOOSE 

	// Tests of MIN_FLOAT64_SIGNALLING_NAN
	//
	min::gen missing = MISSING();
	float64 v = * (float64 *) & missing;

	assert ( isnan ( v ) );

	// Attemps to get any kind of NaN to raise an
	// exception failed, so we cannot test for that.

	// However, we can test that hardware does not
	// generate non-signalling NaNs with high order
	// 16 bits identical to v.

	float64 v2 = v + 1.0;

	assert ( isnan ( v2 ) );
	uns16 * vp = (uns16 *) & v;
	uns16 * v2p = (uns16 *) & v2;
	assert ( vp[3*little_endian]
		 !=
		 v2p[3*little_endian] );

	v2 = 0.0;
	v2 = v2 / v2;
	assert ( isnan ( v2 ) );
	assert ( vp[3*little_endian]
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
    min::dot_initiator =
        min::new_str_gen ( ".initiator" );
    min::dot_separator =
        min::new_str_gen ( ".separator" );
    min::dot_terminator =
        min::new_str_gen ( ".terminator" );
    min::dot_position =
        min::new_str_gen ( ".position" );
    min::dot_type =
        min::new_str_gen ( ".type" );
    min::new_line =
        min::new_str_gen ( "\n" );
    min::empty_string =
        min::new_str_gen ( "" );
    min::doublequote =
        min::new_str_gen ( "\"" );
    min::number_sign =
        min::new_str_gen ( "#" );

    // Deprecated:
    //
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
	min::packed_vec_insptr<const char *> p =
	    min::const_char_ptr_packed_vec_type.new_stub
		( ::standard_special_names_length );
	::standard_special_names = p;
	min::standard_special_names = p;
	min::push ( p, ::standard_special_names_length,
		       ::standard_special_names_value );
    }

    {
	min::packed_vec_insptr<const char *> p =
	    min::const_char_ptr_packed_vec_type.new_stub
		( ::standard_attr_flag_names_length );
	::standard_attr_flag_names = p;
	min::standard_attr_flag_names = p;

	min::push ( p,
	            ::standard_attr_flag_names_length,
		    ::standard_attr_flag_names_value );
    }

    init_standard_char_flags();
    init_pgen_formats();

    for ( min::initializer * i = MINT::last_initializer;
          i != NULL; i = i->previous )
        i->init();
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
    MIN_ASSERT ( is_name ( v1 ) );
    MIN_ASSERT ( is_name ( v2 ) );

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

// Process Management
// ------- ----------

bool MINT::thread_interrupt_needed = false;
void MINT::thread_interrupt ( void ) {}  // TBD

// Allocator/Collector/Compactor 
// -----------------------------

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
//
#define MIN_SCAVENGE_S2(FAIL) \
    min::uns64 c = MUP::control_of ( s2 ); \
    int type = MUP::type_of_control ( c ); \
    \
    if ( ( c & sc.stub_flag ) == 0 ) \
	; /* do nothing */ \
    else if ( type < 0 \
	      || \
	      ! MINT::is_scavengable ( type ) ) \
	MUP::clear_flags_of \
	    ( s2, sc.stub_flag ); \
    else if (    sc.to_be_scavenged \
	      >= sc.to_be_scavenged_limit ) \
    { \
	sc.stub_flag_accumulator = accumulator; \
	FAIL; \
    } \
    else \
    { \
	* sc.to_be_scavenged ++ = s2; \
	MUP::clear_flags_of \
	    ( s2, sc.stub_flag ); \
    } \
    \
    accumulator |= c; \
    ++ sc.stub_count;

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
    min::uns32 subtype = * ( min::uns32 *) beginp;
    subtype &= MINT::PACKED_CONTROL_SUBTYPE_MASK;
    MIN_ASSERT ( subtype < MINT::packed_subtype_count);
    MINT::packed_struct_descriptor * psd =
        (MINT::packed_struct_descriptor *)
        (*MINT::packed_subtypes)[subtype];

    assert ( sc.state < (1ull << 32 ) );
    min::uns32 i = (min::uns32) sc.state >> 2;
    min::uns64 accumulator = sc.stub_flag_accumulator;

    if (    ( sc.state & 1 ) == 0
         && psd->gen_disp != NULL )
	while ( true )
    {
	assert ( i < (1<<30) );

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
	assert ( i < (1<<30) );

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
    min::uns32 subtype = * ( min::uns32 *) beginp;
    subtype &= MINT::PACKED_CONTROL_SUBTYPE_MASK;
    MIN_ASSERT ( subtype < MINT::packed_subtype_count);
    MINT::packed_vec_descriptor * pvd =
        (MINT::packed_vec_descriptor *)
        (*MINT::packed_subtypes)[subtype];

    min::uns32 length = * ( min::uns32 *)
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
	    assert ( i < (1<<30) );

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
	    assert ( i < (1<<30) );

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
	    sc.stub_flag_accumulator = \
		accumulator; \
	    if ( obj_aux_scavenge ( sc, s2 ) ) \
	    { \
		FAIL; \
	    } \
	    accumulator = \
		sc.stub_flag_accumulator; \
	} \
	else \
	{ \
	    if ( ( c & sc.stub_flag ) == 0 ) \
	        /* Do nothing */ ; \
	    else if ( ! MINT::is_scavengable \
			    ( type ) ) \
		MUP::clear_flags_of \
		    ( s2, sc.stub_flag ); \
	    else if (    sc.to_be_scavenged \
		      >= sc.to_be_scavenged_limit ) \
	    { \
		sc.stub_flag_accumulator = \
		    accumulator; \
		FAIL; \
	    } \
	    else \
	    { \
		* sc.to_be_scavenged ++ = s2; \
		MUP::clear_flags_of \
		    ( s2, sc.stub_flag ); \
	    } \
	    ++ sc.stub_count; \
	    accumulator |= c; \
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
	    if ( c & MUP::STUB_PTR )
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
    min::obj_vec_ptr vp ( sc.s1 );

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
	    assert ( len <= 7 );
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
	    flags = min::IS_MIDDLING
		  + min::QUOTE_SUPPRESS;
	else if ( cat[0] == 'P' )
	{
	    switch ( cat[1] )
	    {
	    case 'i':
	    case 's':
		flags = min::IS_LEADING
		      + min::IS_MARK;
		break;

	    case 'f':
	    case 'e':
		flags = min::IS_TRAILING
		      + min::IS_MARK;
		break;

	    case 'c':
	    case 'd':
	        flags = min::IS_MIDDLING
		      + min::CONDITIONAL_BREAK
		      + min::IS_MARK;
		break;

	    default:
		flags = min::IS_MIDDLING
		      + min::IS_MARK;
	    }
	}
	else if ( cat[0] == 'S' )
	    flags = min::IS_MIDDLING
		  + min::IS_MARK;
	else if ( strcmp ( cat, "Mn" ) == 0 )
	    flags = min::IS_MIDDLING
	          + min::IS_NON_SPACING
		  + min::QUOTE_SUPPRESS;
	else if ( cat[0] == 'M' )
	    flags = min::IS_MIDDLING
		  + min::QUOTE_SUPPRESS;
	else if ( cat[0] == 'N' )
	    flags = min::IS_MIDDLING;
	else if ( strcmp ( cat, "Zs" ) == 0 )
	    flags = min::IS_OTHER_HSPACE;
	else if ( cat[0] == 'Z' )
	    flags = min::IS_OTHER_CONTROL
		  + min::IS_NON_SPACING;
	else if ( cat[0] == 'C' )
	    flags = min::IS_OTHER_CONTROL
		  + min::IS_NON_SPACING;
	else
	    flags = min::IS_UNSUPPORTED;

	// Fixup flags in special cases.
	//
	switch ( i )
	{
	case ' ':	flags = min::IS_SP; break;	
	case '\t':	flags = min::IS_OTHER_HSPACE;
		    break;
	case 0xA0:	flags = min::IS_NB_HSPACE;
		    break;  // NBSP
	case '\f':
	case '\v':
	case '\r': flags = min::IS_VSPACE
			 + min::IS_NON_SPACING;
		   break;

	case '`':
	case 0xA1:  // Inverted !
	case 0xBF:	// Inverted ?
		    flags = min::IS_LEADING
			  + min::IS_MARK;
		    break;
	
	case '\'':
	case ',':
	case ';':
	case ':':
	case '!':
	case '?':
		    flags = min::IS_TRAILING
			  + min::IS_MARK;
		    break;

	case '.':	flags = min::IS_TRAILING
			  + min::QUOTE_SKIP
			  + min::IS_MARK;
		    break;

	case '-':
	case '_':
	case '%':	flags = min::IS_MIDDLING
			  + min::CONDITIONAL_BREAK
			  + min::IS_MARK;
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
	MIN_ASSERT ( type_of ( s ) == LONG_STR );
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
	MIN_ASSERT ( type_of ( s ) == LONG_STR );
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
	MIN_ASSERT ( type_of ( s ) == LONG_STR );
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
	MIN_ASSERT ( type_of ( s ) == LONG_STR );
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
	MIN_ASSERT ( type_of ( s ) == LONG_STR );
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
	MIN_ASSERT ( type_of ( s ) == LONG_STR );
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
    uns32 hash = strnhash ( ! p, n );
    uns32 h = hash & MINT::str_hash_mask;
    const char * q;

    min::stub * s = MINT::str_acc_hash[h];
    while ( s != MINT::null_stub )
    {
	uns64 c = MUP::control_of ( s );

        if (    n <= 8
	     && type_of ( s ) == SHORT_STR
	     && ::strncmp ( ! p, s->v.c8, n ) == 0
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
		       ( ! p, q = MUP::str_of (
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
	     && ::strncmp ( ! p, s2->v.c8, n ) == 0
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
		       ( ! p, q = MUP::str_of (
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
	::strncpy ( s2->v.c8, ! p, n );
    }
    else
    {
	MUP::set_type_of ( s2, LONG_STR );
	MUP::new_body
	    ( s2, sizeof ( MUP::long_str ) + n + 1 );

	MUP::long_str * ls = MUP::long_str_of ( s2 );
	ls->length = n;
	ls->hash = hash;

	// Be sure string is NUL padded to a multiple
	// of 8 bytes.
	//
	* (min::uns64 *)
	  ( MUP::str_of(ls) + n - n % 8 ) = 0;

	::strncpy
	    ( (char *) MUP::str_of ( ls ), ! p, n );
    }

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
		  const min::str_ptr sp, int & i,
		  int base )
{
    const char * beginp =
        ! ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    min::int64 v = strtol ( beginp, & endp, base );
    if ( errno != 0 ) return false;
    if ( endp == beginp ) return false;
    if ( v < INT_MIN || v > INT_MAX ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::int64 & value,
		  const min::str_ptr sp, int & i,
		  int base )
{
    const char * beginp =
        ! ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    min::int64 v = strtoll ( beginp, & endp, base );
    if ( errno != 0 ) return false;
    if ( endp == beginp ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::uns32 & value,
		  const min::str_ptr sp, int & i,
		  int base )
{
    const char * beginp =
        ! ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    min::uns64 v = strtoul ( beginp, & endp, base );
    if ( errno != 0 ) return false;
    if ( endp == beginp ) return false;
    if ( v > UINT_MAX ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::uns64 & value,
		  const min::str_ptr sp, int & i,
		  int base )
{
    const char * beginp =
        ! ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    min::uns64 v = strtoull ( beginp, & endp, base );
    if ( errno != 0 ) return false;
    if ( endp == beginp ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::float32 & value,
		  const min::str_ptr sp, int & i )
{
    const char * beginp =
        ! ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    min::float32 v = strtof ( beginp, & endp );
    if ( errno != 0 ) return false;
    if ( endp == beginp ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto ( min::float64 & value,
		  const min::str_ptr sp, int & i )
{
    const char * beginp =
        ! ( min::begin_ptr_of ( sp ) + i );
    char * endp;
    errno = 0;
    min::float64 v = strtod ( beginp, & endp );
    if ( errno != 0 ) return false;
    if ( endp == beginp ) return false;
    value = v;
    i += endp - beginp;
    return true;
}

bool min::strto
	( min::int32 & value, min::gen g, int base )
{
    min::str_ptr sp ( g );
    if ( ! sp ) return false;
    int i = 0;
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
    int i = 0;
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
    int i = 0;
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
    int i = 0;
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
    int i = 0;
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
    int i = 0;
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
        MIN_ASSERT ( is_name ( * p ) );
        hash = labhash ( hash, min::hash ( * p ++ ) );
    }
    return hash;
}

min::gen min::new_lab_gen
	( min::ptr<const min::gen> p, min::uns32 n )
{
    uns32 hash = labhash ( ! p, n );
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
    memcpy ( lh + 1, ! p, n * sizeof ( min::gen ) );

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
    memset ( tp, 0, psd->size );
    * tp = psd->subtype;
    MUP::set_type_of ( s, PACKED_STRUCT );
    return s;
}

// Files
// -----

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

void min::init_line_display
	( min::ref<min::file> file,
	  min::uns32 line_display )
{
    init ( file );
    file->line_display = line_display;
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
	  min::uns32 line_display,
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

    file->line_display = line_display;
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
	  min::uns32 line_display,
	  min::uns32 spool_lines )
{
    init_input ( file, line_display, spool_lines );
    file->istream = & istream;
}

void min::init_input_file
	( min::ref<min::file> file,
	  min::file ifile,
	  min::uns32 line_display,
	  min::uns32 spool_lines )
{
    init_input ( file, line_display, spool_lines );
    ifile_ref(file) = ifile;
}

void min::init_input_string
	( min::ref<min::file> file,
	  min::ptr<const char> string,
	  min::uns32 line_display,
	  min::uns32 spool_lines )
{
    init_input ( file, line_display, spool_lines );
    load_string ( file, string );
    complete_file ( file );
}

void min::load_string
	( min::file file,
	  min::ptr<const char> string )
{
    MIN_ASSERT ( ! min::file_is_complete ( file ) );

    uns64 length = ::strlen ( ! string );
    uns32 offset = file->buffer->length;
    assert ( length <= ( 1ull << 32 ) - 1 - offset );

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
	  min::uns32 line_display,
	  min::uns32 spool_lines )
{
    init_input ( file, line_display, spool_lines );
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
    MIN_ASSERT ( ! min::file_is_complete ( file ) );

    min::str_ptr fname ( file_name );
    min::uns32 offset = file->buffer->length;

    // Use OS independent min::os::file_size.
    //
    char error_buffer[512];
    uns64 file_size;
    if ( ! min::os::file_size
               ( file_size,
	         ! min::begin_ptr_of ( fname ),
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
        fopen ( ! min::begin_ptr_of ( fname ), "r" );

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
        fread ( ! ( file->buffer + offset ), 1,
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
	    assert ( file->ifile == NULL_STUB );

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
		    ( ! ( ifile->buffer + ioffset ) );
	    min::push ( file->buffer, length,
	                ifile->buffer + ioffset );
	}
	else
	    return min::NO_LINE;

	min::end_line ( file );
    }

    file->next_offset +=
        1 + ::strlen
	        ( ! ( file->buffer + line_offset ) );
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

min::uns32 min::print_line
	( min::printer printer,
	  min::uns32 line_display,
	  min::file file,
	  min::uns32 line_number,
	  const char * blank_line,
	  const char * end_of_file,
	  const char * unavailable_line )
{
    const min::uns32 * char_flags =
	printer->print_format.char_flags;
    uns32 offset = min::line ( file, line_number );
    uns32 length;
    bool eof = false;
    
    if ( offset == min::NO_LINE )
    {
	const char * message = NULL;
	if ( line_number == file->file_lines )
	{
	    length = partial_length ( file );
	    if ( length == 0 )
		message = end_of_file;
	    else
	    {
		offset = partial_offset ( file );
		eof = true;
	    }
	}
	else
	    message = unavailable_line;

	if ( offset == min::NO_LINE )
	{
	    if ( message == NULL ) return min::NO_LINE;

	    printer << message << min::eol;
	    return 0;
	}
    }
    else
        length =
	    ::strlen ( ! ( file->buffer + offset ) );

    // Move line to stack so that (1) it will not be
    // relocatable when printer is called, and (2) it
    // will end with NUL even if it is a partial line.
    //
    char buffer[length+1];
    memcpy ( buffer, ! ( file->buffer + offset ),
             length );
    buffer[length] = 0;

    // Blank line check.
    //
    if ( blank_line == NULL )
        ; // do nothing
    else if ( eof ? end_of_file != NULL :
                  (   line_display
                    & min::DISPLAY_EOL ) )
        ; // do nothing
    else if ( buffer[0] == 0 )
    {
	printer << blank_line << min::eol;
	return 0;
    }
    else
    {
	const char * p = & buffer[0];
	min::uns32 name_or_picture_flags =
	    ! printer->print_format.display_control
				   .display_char
	    &
	    ! printer->print_format.display_control
				   .display_suppress;
	if (   printer->print_format.op_flags
	     & min::DISPLAY_NON_GRAPHIC )
	    name_or_picture_flags |=
	        min::IS_NON_GRAPHIC;

	while ( * p && (unsigned) * p < 256 )
	{
	    min::uns32 cflags =
		 char_flags[(unsigned char) *p];
	    if ( cflags & min::IS_GRAPHIC )
	        break;
	    if ( name_or_picture_flags & cflags )
	        break;
	    ++ p;
	}

	if ( * p == 0 )
	{
	    printer << blank_line << min::eol;
	    return 0;
	}
    }

    printer
        << min::save_print_format
	<< min::set_line_display ( line_display )
	<< buffer;
    uns32 width = printer->column;
    if ( eof )
    {
	printer << min::restore_print_format;
        if ( end_of_file ) printer << end_of_file;
	printer << min::eol;
    }
    else
    {
	printer << min::eol
	        << min::restore_print_format;
	if ( line_display & min::DISPLAY_EOL )
	{
	    min::uns32 prefix_columns =
		min::ustring_columns
		    ( printer->print_format
		              .char_name_format
			     ->char_name_prefix );
	    assert ( prefix_columns > 0 );
	    min::uns32 postfix_columns =
		min::ustring_columns
		    ( printer->print_format
		              .char_name_format
			     ->char_name_postfix );
	    assert ( postfix_columns > 0 );
	    width +=
	        (   line_display
		  & min::DISPLAY_PICTURE ?  1 :
		  prefix_columns + 2
		                 + postfix_columns );
		      // 2 is width of "NL"
        }
    }
    return width;
}

min::uns32 min::print_line_column
	( min::file file,
	  const min::position & position,
	  min::uns32 line_display,
	  const min::print_format & print_format )
{
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
	    ::strlen ( ! ( file->buffer + offset ) );

    min::print_format pf = print_format;
    min::uns32 flags = min::DISPLAY_EOL
                     + min::DISPLAY_PICTURE
		     + min::DISPLAY_NON_GRAPHIC;
    pf.op_flags &= ~ flags;
    pf.op_flags |= ( flags & line_display);
    pf.op_flags |= min::EXPAND_HT;
    pf.display_control =
        ( line_display & min::DISPLAY_NON_GRAPHIC ?
	  min::graphic_only_display_control :
	  min::graphic_and_space_display_control );
    pf.break_control = min::no_auto_break_break_control;
    min::pwidth ( column, ! ( file->buffer + offset ),
    		  position.offset <= length ?
		      position.offset : length, pf );
    return column;
}

void min::print_phrase_lines
	( min::printer printer,
	  min::uns32 line_display,
	  min::file file,
	  const min::phrase_position & position,
	  char mark,
	  const char * blank_line,
	  const char * end_of_file,
	  const char * unavailable_line )
{
    assert ( position.end.line >= position.begin.line );

    min::position begin = position.begin;
    min::position end   = position.end;
    uns32 begin_column =
        print_line_column
	    ( file, begin, line_display,
	                   printer->print_format );
    uns32 end_column =
        print_line_column
	    ( file, end, line_display,
	                 printer->print_format );

    uns32 line = begin.line;
    uns32 first_column = begin_column;

    uns32 width = min::print_line
	( printer, line_display, file, line,
	  blank_line, end_of_file, unavailable_line );

    while ( true )
    {
        if ( mark != 0 )
	{
	    for ( uns32 i = 0; i < first_column; ++ i )
		printer << ' ';

	    uns32 next_column =
		end.line == line ? end_column : width;
	    if ( next_column <= first_column )
		next_column = first_column + 1;

	    for ( uns32 i = first_column;
		  i < next_column; ++ i )
		printer << mark;
	    printer << min::eol;
	}

	if ( line == end.line ) return;

	++ line;

	if ( line == end.line && end_column == 0 )
	    return;

	first_column = 0;
	width = min::print_line
	    ( printer, line_display, file, line,
	      blank_line, end_of_file,
	      unavailable_line );
    }
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

void min::flush_line
	( min::file file, min::uns32 offset )
{
    assert ( offset < file->end_offset );

    if ( file->ostream != NULL )
        * file->ostream << ! ( file->buffer + offset )
	                << std::endl;

    if ( file->ofile != NULL_STUB )
    {
	min::file ofile = file->ofile;
        uns32 length =
	    ::strlen ( ! ( file->buffer + offset ) );
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
        * file->ostream << ! ( file->buffer + offset );
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
        assert (    line_number
	         <= file->next_line_number );

    assert (    file->next_line_number
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
    assert (   lines_to_delete
             < file->line_index->length );

    uns32 buffer_offset =
	file->line_index[lines_to_delete];
    if ( buffer_offset < file->buffer->length )
	memmove ( ! ( file->buffer + 0 ) ,
		  ! ( file->buffer + buffer_offset ),
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
	assert ( file->line_index != NULL_STUB );
	assert ( line_number < file->next_line_number );
	min::uns32 lines_to_back_up =
	    file->next_line_number - line_number;
	assert (    file->line_index->length
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
    {
	MIN_ABORT ( "bad rewind line_number" );
    }
}

std::ostream & operator <<
	( std::ostream & out, min::file file )
{
    while ( true )
    {
        min::uns32 offset = min::next_line ( file );
	if ( offset == min::NO_LINE ) break;
	out << ! ( file->buffer + offset ) << std::endl;
    }

    if ( file->next_offset < file->buffer->length )
    {
	min::push(file->buffer) = 0;
        out << ! ( file->buffer + file->next_offset );
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
	    ::strlen ( ! ( ifile->buffer + offset ) );
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

static min::uns32 id_map_stub_disp[2] =
    { min::DISP ( & min::id_map_header<min::uns32>
                       ::hash_table ),
      min::DISP_END };

static min::uns32 stub_element_stub_disp[2] =
    { 0, min::DISP_END };

static min::packed_vec
	< const min::stub *,
	  min::id_map_header<min::uns32> >
    id_map_type
    ( "id_map_type",
      NULL, stub_element_stub_disp,
      NULL, id_map_stub_disp );

template < typename L >
inline L hash
	( min::packed_vec_ptr
	      < const min::stub *,
		min::id_map_header<L> > map,
	  const min::stub * s )
{
    min::packed_vec_insptr<min::uns32> hash_table =
        map->hash_table;
    assert ( hash_table != min::NULL_STUB );
    min::uns64 h0 = ( s - ( const min::stub * ) 0 );
    h0 &= map->hash_mask;
    h0 *= map->hash_multiplier;
    h0 %= hash_table->length;
    return (L) h0;
}

template < typename L >
static void new_hash_table
	( min::packed_vec_ptr
	      < const min::stub *,
		min::id_map_header<L> > map )
{
    typedef min::packed_vec_insptr
    		< const min::stub *,
		  min::id_map_header<L> > id_map_insptr;

    id_map_insptr map_insptr = (id_map_insptr) map;

    L length = map->occupied;
    assert (   length
	     < ( (L) 1 << ( 8 * sizeof ( L ) - 2 ) ) );
    length *= 4;
    if ( length < 128) length = 128;

    hash_table_ref ( map ) =
	min::uns32_packed_vec_type.new_stub ( length );
    min::packed_vec_insptr<min::uns32> hash_table =
        map->hash_table;
    min::push ( hash_table, length );

    map_insptr->hash_mask = (min::uns32) -1;
    map_insptr->hash_multiplier = 1103515245;
        // Used by gcc for linear congrential random
	// number generator with modulus 2^32.
    map_insptr->hash_max_offset = 0;
    for ( L id = 0; id < map->length; ++ id )
    {
	const min::stub * s = map[id];
	L h = ::hash ( map, s );
	L offset = 0;
	while ( hash_table[h] != 0 )
	{
	    h = ( h + 1 ) % hash_table->length;
	    ++ offset;
	}
	hash_table[h] = id;
	if ( map->hash_max_offset < offset )
	    map_insptr->hash_max_offset = offset;
    }
}

template < typename L >
inline min::packed_vec_ptr
	< const min::stub *,
	  min::id_map_header<L>, L > init
	( min::ref<min::packed_vec_ptr
		       < const min::stub *,
			 min::id_map_header<L>, L > >
	      map )
{
    typedef min::packed_vec_insptr
    		< const min::stub *,
		  min::id_map_header<L>, L >
	    id_map_insptr;

    id_map_insptr map_insptr =
        (id_map_insptr) (min::id_map) map;
    if ( map_insptr == min::NULL_STUB )
    {
	map_insptr = ::id_map_type.new_stub ( 16 );
	map = map_insptr;
    }
    else
    {
        hash_table_ref((min::id_map) map) =
	    min::NULL_STUB;
	min::pop ( map_insptr,
	           (min::unsptr) map->length );
	min::resize ( map_insptr, 16 );
    }
    min::push ( map_insptr ) = min::NULL_STUB;
    map_insptr->occupied = 0;
    map_insptr->next = 1;
    return map;
}

template < typename L >
inline min::uns32 find
	( min::packed_vec_ptr
	      < const min::stub *,
		min::id_map_header<L>, L > map,
	  const min::stub * s )
{
    if ( s == min::NULL_STUB ) return 0;

    if ( map->hash_table == min::NULL_STUB )
	::new_hash_table ( map );
    min::packed_vec_insptr<min::uns32> hash_table =
        map->hash_table;
    L h = ::hash ( map, s );
    L offset = 0;
    while ( true )
    {
        L id = hash_table[h];
	if ( id == 0 ) return 0;
	if ( map[id] == s ) return id;
	if ( offset >= map->hash_max_offset ) return 0;
	h = ( h + 1 ) % hash_table->length;
	++ offset;
    }
}

template < typename L >
inline min::uns32 find_or_add
	( min::packed_vec_ptr
	      < const min::stub *,
		min::id_map_header<L>, L > map,
	  const min::stub * s )
{
    typedef min::packed_vec_insptr
    		< const min::stub *,
		  min::id_map_header<L>, L >
	id_map_insptr;

    if ( s == min::NULL_STUB ) return 0;

    if ( map->hash_table == min::NULL_STUB )
	::new_hash_table ( map );
    min::packed_vec_insptr<min::uns32> hash_table =
        map->hash_table;
    L h = ::hash ( map, s );
    L offset = 0;
    while ( true )
    {
        L id = hash_table[h];
	if ( id == 0 ) break;
	if ( map[id] == s ) return id;
	h = ( h + 1 ) % hash_table->length;
	++ offset;
    }
    L id = map->length;
    id_map_insptr map_insptr =
        (id_map_insptr) (min::id_map) map;
    min::push ( map_insptr ) = s;
    hash_table[h] = id;

    if ( map->hash_max_offset < offset )
        map_insptr->hash_max_offset = offset;

    ++ map_insptr->occupied;
    if ( hash_table->length < 2 * map->occupied )
        hash_table_ref ( map ) = min::NULL_STUB;

    return id;
}

template < typename L >
inline void insert
	( min::packed_vec_ptr
	      < const min::stub *,
		min::id_map_header<L> > map,
	  const min::stub * s,
	  L id )
{
    typedef min::packed_vec_insptr
    		< const min::stub *,
		  min::id_map_header<L>, L >
        id_map_insptr;

    id_map_insptr map_insptr = (id_map_insptr) map;

    assert ( id != 0 );
    assert ( s != min::NULL_STUB );

    if ( id >= map->length )
        min::push ( map_insptr, id + 1 - map->length );
    map_insptr[id] = s;
    ++ map_insptr->occupied;

    min::packed_vec_insptr<min::uns32> hash_table =
        map->hash_table;

    if ( hash_table == min::NULL_STUB ) return;
    else if ( hash_table->length < 2 * map->occupied )
    {
        hash_table_ref ( map ) = min::NULL_STUB;
	    // Defer creation of a new hash table
	    // to the next find or find_or_add.
	return;
    }
        
    L h = ::hash ( map, s );
    L offset = 0;
    while ( true )
    {
        if ( hash_table[h] == 0 ) break;
	h = ( h + 1 ) % hash_table->length;
	++ offset;
    }
    hash_table[h] = id;
    if ( map->hash_max_offset < offset )
        map_insptr->hash_max_offset = offset;
}

min::id_map min::init
	( min::ref<min::id_map> map )
{
    return ::init ( map );
}

min::uns32 min::find
	( min::id_map map,
	  const min::stub * s )
{
    return ::find ( map, s );
}

min::uns32 min::find_or_add
	( min::id_map map,
	  const min::stub * s )
{
    return ::find_or_add ( map, s );
}

void min::insert
	( min::id_map map,
	  const min::stub * s,
	  min::uns32 id )
{
    ::insert ( map, s, id );
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
// min::gen units.
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


min::gen min::new_obj_gen
	    ( min::unsptr unused_size,
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

    min::stub * s = MUP::new_acc_stub();
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
    min::gen * & newb = * ( min::gen **) &
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
    memset ( & newb[to], 0,
             unused_size * sizeof ( min::gen ) );
    to += unused_size;

    // Copy auxiliary area.
    //
    from_end = from + min::aux_size_of ( vp );
    while ( from < from_end )
        newb[to++] = oldb[from++];

    MIN_ASSERT ( from == vp.total_size );
    MIN_ASSERT ( to == total_size );

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
    MIN_ASSERT ( n > 0 );

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
	           ( type, last, MUP::STUB_PTR ) );
	type = min::LIST_AUX;
	previous = last;
    }
    MUP::set_control_of
        ( last, MUP::renew_control_type ( end, type ) );
}

# endif // MIN_USE_OBJ_AUX_STUBS

// Remove a list.  Free any aux stubs used and
// set any auxiliary area elements use to min::NONE().
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
		if ( c & MUP::STUB_PTR)
		    s = MUP::stub_of_control ( c );
		else
		{
		    unsptr vc =
			MUP::value_of_control ( c );
		    if ( vc == 0 ) return;
		    index = total_size - c;
		    s = NULL;
		}
	    }
	    else
#	endif
	{
	    MIN_ASSERT ( index != 0 );
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
		        s = s2, index = 0;
		}
		else
#	    endif
	    if ( min::is_list_aux ( v ) )
	    {
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

    MIN_ASSERT ( lp.reserved_insertions >= 1 );
    MIN_ASSERT ( lp.reserved_elements >= n );

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
	MUP::acc_write_update
		( MUP::stub_of ( lp.vecp ), * p );
	lp.current = lp.base[lp.current_index] = * p;
	return;
    }

    unsptr unused_offset =
        MUP::unused_offset_of ( lp.vecp );
    unsptr aux_offset =
        MUP::aux_offset_of ( lp.vecp );
    unsptr total_size = lp.total_size;
    MIN_ASSERT (    total_size
                 == min::total_size_of ( lp.vecp ) );

    MUP::acc_write_update
            ( MUP::stub_of ( lp.vecp ), p, n );

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
				    MUP::STUB_PTR )
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
				  MUP::STUB_PTR ) );
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
		     <= aux_offset );

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

	MUP::aux_offset_of ( lp.vecp ) = aux_offset;
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
		     MUP::STUB_PTR );
		MIN_ASSERT ( previous );
	    }
	    else if ( ! previous )
	    {
	        s = MUP::new_aux_stub();
		MINT::set_aux_flag_of ( lp.vecp );
		MUP::set_gen_of ( s, lp.current );
		end = MUP::new_control_with_type
		    ( 0, s, MUP::STUB_PTR );
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
		    next = total_size - next;
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
				MUP::STUB_PTR ) );
		}
	    }
	    else
	    {
	        MIN_ASSERT ( lp.current_index != 0 );

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
		 <= aux_offset );

    unsptr first = aux_offset - 1;
    unsptr aux_first = total_size - first;

    while ( n -- )
	lp.base[-- aux_offset] = * p ++;

#   if MIN_USE_OBJ_AUX_STUBS
	if ( lp.current_stub != NULL )
	{
	    MIN_ASSERT ( previous );
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
	if ( next != 0 ) next = total_size - next;
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
	MIN_ASSERT ( lp.current_index != 0 );
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

    MUP::aux_offset_of ( lp.vecp ) = aux_offset;
}

void min::insert_after
	( min::list_insptr & lp,
	  const min::gen * p, min::unsptr n )
{
    if ( n == 0 ) return;

    unsptr unused_offset =
        MUP::unused_offset_of ( lp.vecp );
    unsptr aux_offset =
        MUP::aux_offset_of ( lp.vecp );
    unsptr total_size = lp.total_size;
    MIN_ASSERT (    total_size
                 == min::total_size_of ( lp.vecp ) );

    MIN_ASSERT ( lp.reserved_insertions >= 1 );
    MIN_ASSERT ( lp.reserved_elements >= n );
    MIN_ASSERT ( lp.current != min::LIST_END() );

    lp.reserved_insertions -= 1;
    lp.reserved_elements -= n;

    MUP::acc_write_update
	    ( MUP::stub_of ( lp.vecp ), p, n );

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
			   MUP::STUB_PTR ) );
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
			        MUP::STUB_PTR );
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
			        MUP::STUB_PTR ) );
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
		MIN_ASSERT ( lp.current_index != 0 );

		MUP::set_control_of
		    ( s,
		      MUP::new_control_with_type
		          ( min::LIST_AUX, first,
			    MUP::STUB_PTR ) );
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
		 <= aux_offset );

    unsptr first = aux_offset - 1;

    if ( lp.current_index != 0 )
	lp.base[-- aux_offset] = lp.current;

    // If previous, we can copy the last new element to
    // the old current element.

    // Copy all the new elements BUT the last new
    // element.
    //
    while ( -- n )
	lp.base[-- aux_offset] = * p ++;

#   if MIN_USE_OBJ_AUX_STUBS
    if ( lp.current_stub != NULL )
    {
	MIN_ASSERT ( previous );
	lp.base[-- aux_offset] = * p ++;
	uns64 c =
	    MUP::control_of ( lp.current_stub );
	if ( c & MUP::STUB_PTR )
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
    }
    else
#   endif
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
	// element as pointer to copied of current
	// element.
	//
	MIN_ASSERT ( lp.current_index != 0 );

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

    MUP::aux_offset_of ( lp.vecp ) = aux_offset;
}

min::unsptr min::remove
	( min::list_insptr & lp,
	  min::unsptr n )
{
    if ( n == 0 || lp.current == min::LIST_END() )
        return 0;

    unsptr total_size = lp.total_size;
    MIN_ASSERT (    total_size
                 == min::total_size_of ( lp.vecp ) );

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
	    MIN_ASSERT ( lp.current_index != 0 );
	    lp.base[lp.current_index] = min::NONE();
	    lp.current =
		lp.base[-- lp.current_index];
	    if ( min::is_list_aux ( lp.current ) )
	    {
		if ( lp.current == min::LIST_END() )
		    break;
		lp.base[lp.current_index] = min::NONE();
		lp.current_index =
		      total_size
		    - min::list_aux_of ( lp.current );
		lp.current = lp.base[lp.current_index];
	    }
	    else if ( min::is_stub ( lp.current ) )
		lp.forward ( lp.current_index );
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
			        MUP::STUB_PTR ) );
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
	        MIN_ASSERT ( lp.current_index != 0 );

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
	    MIN_ASSERT ( lp.current_index != 0 );

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
	// have a previous pointer.

	MIN_ASSERT ( current_index != 0 );

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
	        ( current_index != lp.head_index );
	    lp.base[current_index] =
		min::new_list_aux_gen
		    ( total_size - lp.current_index );
	    lp.previous_index = current_index;
	}

	lp.previous_is_sublist_head = false;
    }

    return count;
}

bool MINT::insert_reserve
	( min::list_insptr & lp,
	  min::unsptr insertions,
	  min::unsptr elements,
	  bool use_obj_aux_stubs )
{
    bool result = false;

#   if MIN_USE_OBJ_AUX_STUBS
	if ( use_obj_aux_stubs )
	    MINT::acc_expand_stub_free_list
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
		if ( c & MUP::STUB_PTR ) \
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
		v = vp[i]; \
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
	    v = vp[i]; \
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
        min::gen v = vp[index++];

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
        min::gen v = vp[index];
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
		    ( MUP::type_of ( s ) == LIST_AUX );
	    }
	    else
#       endif
	MIN_ASSERT ( is_list_aux ( v ) );
	    
	bool label_is_next = true;
	FORLIST(vp,index2,s,v2,false)
	    if ( label_is_next )
	    {
	        ++ count;
		label_is_next = false;
	    }
	    else label_is_next = true;
	ENDFORLIST
	MIN_ASSERT ( label_is_next );
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

// Copy elements vp[index .. end_index-1] to work as per
// pass1.
//
inline void copy_to_work
	( min::obj_vec_ptr & vp,
	  min::unsptr index,
	  min::unsptr end_index,
	  min::gen * & work_low,
	  min::gen * & work_high,
	  min::gen *   work_end )
{
    while ( index < end_index )
    {
	min::gen v = vp[index++];
	* work_low ++ = v;
#           if MIN_USE_OBJ_AUX_STUBS
	    const min::stub * s = NULL;
#           endif
	min::unsptr index2;
	if ( min::is_sublist_aux ( v )
	     ||
	     min::is_list_aux ( v ) )
	{
	    index2 = MUP::aux_of ( v );
	    if ( index2 == 0 ) continue;
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
    memset ( hash_count, 0, sizeof ( hash_count ) );
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
	MIN_ASSERT ( label_is_next );
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
	    memset ( work_high + 1, 0,
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
	MIN_ASSERT ( label_is_next );
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
                     + hash_size
                     + attr_size_of ( vp );
#   if MIN_USE_OBJ_AUX_STUBS
	if ( MINT::aux_flag_of ( vp ) )
	    work_size += min::list_element_count ( vp );
	else
#   endif
	work_size += aux_size_of ( vp );
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
	  vp.total_size_flags,
	  vp.unused_offset_flags,
	  vp.aux_offset_flags );

    memcpy ( (min::gen *) newb + header_size,
             ! & var ( vp, 0 ),
	       ( unused_offset - header_size )
	     * sizeof ( min::gen ) );
    memset ( (min::gen *) newb + unused_offset,
             0, unused_size * sizeof ( min::gen ) );
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
	    MIN_ASSERT ( len > 0 );
	}
	else
	{
	    MIN_ASSERT
		( is_str ( name ) || is_num ( name ) );
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
	    start_vector ( ap.dlp, i );
	    ap.flags = ap_type::IN_VECTOR;
	    ap.index = i;
	    c = current ( ap.dlp );
	}
	else
	{
	    ap.index = min::hash ( element[0] );
	    ap.flags = 0;

	    start_hash ( ap.dlp, ap.index );

	    for ( c = current ( ap.dlp );
		  ! is_list_end ( c );
	          next ( ap.dlp),
		  c = next ( ap.dlp ) )
	    {
		if ( c == element[0] )
		{
		    c = next ( ap.dlp );
		    MIN_ASSERT ( ! is_list_end ( c ) );
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
	          next ( ap.lp),
		  c = next ( ap.lp ) )
	    {
		if ( c == element[ap.length] )
		{
		    c = next ( ap.lp );
		    MIN_ASSERT ( ! is_list_end ( c ) );
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

	MIN_ASSERT ( ap.length > 0 );

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
		  next ( ap.locate_dlp),
		  c = next ( ap.locate_dlp ) )
	    {
		if ( c == element[0] )
		{
		    c = next ( ap.locate_dlp );
		    MIN_ASSERT ( ! is_list_end ( c ) );
		    break;
		}
	    }
	}

	MIN_ASSERT ( ap.length <= len );
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
		  next ( ap.dlp),
		  c = next ( ap.dlp ) )
	    {
		if ( c == element[ap.length] )
		{
		    c = next ( ap.dlp );
		    MIN_ASSERT ( ! is_list_end ( c ) );
		    break;
		}
	    }
	    if ( is_list_end ( c ) ) break;

	    start_copy ( ap.locate_dlp, ap.dlp );
	    ++ length;
	}

	MIN_ASSERT ( length == ap.length );
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
	    ( ap.state == ap_type::LOCATE_FAIL );

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

	MIN_ASSERT ( ap.length < len );

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
		start_vector
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
		( is_str ( name ) || is_num ( name ) );

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
		start_vector ( ap.locate_dlp, i );
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
	      next ( ap.locate_dlp),
	      c = next ( ap.locate_dlp ) )
	{
	    if ( c == name )
	    {
		c = next ( ap.locate_dlp );
		MIN_ASSERT ( ! is_list_end ( c ) );
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
	typedef MUP::attr_ptr_type<vecpt> ap_type;

	for ( min::gen c = current ( ap.locate_dlp );
	      ! is_list_end ( c );
	      next ( ap.locate_dlp),
	      c = next ( ap.locate_dlp ) )
	{
	    if ( c == ap.attr_name )
	    {
		c = next ( ap.locate_dlp );
		MIN_ASSERT ( ! is_list_end ( c ) );
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
	    ( ap.state == ap_type::LOCATE_FAIL );

	if ( ap.flags & ap_type::IN_VECTOR )
	{
	    start_vector ( ap.locate_dlp, ap.index );
	    min::gen c = current ( ap.locate_dlp );
	    MIN_ASSERT ( is_list_end ( c ) );
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
	( ap.state == ap_type::REVERSE_LOCATE_FAIL );

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
	  next ( ap.dlp), c = next ( ap.dlp ) )
    {
	if ( c == reverse_name )
	{
	    c = next ( ap.dlp );
	    MIN_ASSERT ( ! is_list_end ( c ) );
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
        start_vector ( ap.locate_dlp, ap.index );
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
	  next ( ap.dlp), c = next ( ap.dlp ) )
    {
	if ( c == ap.reverse_attr_name )
	{
	    c = next ( ap.dlp );
	    MIN_ASSERT ( ! is_list_end ( c ) );
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

// Helper functions for min::get_attrs.

// Called with current(lp) being start of
// double-arrow-sublist.  Return number of reverse
// attribute names with non-empty value sets.
//
static min::unsptr count_reverse_attrs
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

// Called with current(lp) equal to attribute-/node-
// descriptor for attribute.  Compute counts for
// attribute in info, and also the value and flags
// members.  Does NOT set info.name.  Return true if
// some count is non-zero and false if all counts are
// zero.
//
static bool compute_counts
	( min::list_ptr & lp,
	  min::attr_info & info )
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
	min::unsptr flag_count = 0;
	const min::gen zero_cc =
	    min::new_control_code_gen ( 0 );
	for ( c = min::current ( lpv );
	      ! min::is_list_end ( c );
	      c = min::next ( lpv ) )
	{
#   	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		if ( min::is_sublist ( c ) )
		{
		    c = min::next ( lpv );
		    if ( min::is_list_end ( c ) )
			break;
		}
#   	    endif
	    if ( min::is_sublist ( c ) )
	        info.reverse_attr_count =
		    count_reverse_attrs ( lpv );
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
	  min::unsptr & m )
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
	    if ( compute_counts ( lpv, info ) )
	    {
		info.name = new_label =
		    min::new_lab_gen
			  ( labvec, depth + 1 );
		if ( m < n ) out[m] = info;
		++ m;
	    }
	    compute_children
	        ( lpv, labvec, depth + 1, out, n, m );
	}
    }
# endif

template < class vecpt >
min::unsptr min::get_attrs
	( min::attr_info * out, min::unsptr n,
	  MUP::attr_ptr_type < vecpt > & ap,
	  bool include_attr_vec )
{
    min::unsptr m = 0;  // Return value.
    attr_info info;

    vecpt & vp = obj_vec_ptr_of ( ap.locate_dlp );
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
	    if ( compute_counts ( lp, info ) )
	    {
		if ( m < n ) out[m] = info;
		++ m;
	    }
#	    if MIN_ALLOW_PARTIAL_ATTR_LABELS
		compute_children
		    ( lp, & c, 1, out, n, m );
#	    endif
	}
    }

    if ( include_attr_vec )
    for ( unsptr i = 0;
          i < attr_size_of ( vp );
	  ++ i )
    {
	start_vector ( lp, i );
	if ( is_list_end ( current ( lp ) ) )
	    continue;

	info.name = new_num_gen ( i );
	if ( compute_counts ( lp, info ) )
	{
	    if ( m < n ) out[m] = info;
	    ++ m;
	}
#	if MIN_ALLOW_PARTIAL_ATTR_LABELS
	    compute_children
	        ( lp, & info.name, 1, out, n, m );
#	endif
    }

    return m;
}
template min::unsptr min::get_attrs
	( min::attr_info * out, min::unsptr n,
	  min::attr_ptr & ap,
	  bool include_attr_vec );
template min::unsptr min::get_attrs
	( min::attr_info * out, min::unsptr n,
	  min::attr_updptr & ap,
	  bool include_attr_vec );
template min::unsptr min::get_attrs
	( min::attr_info * out, min::unsptr n,
	  min::attr_insptr & ap,
	  bool include_attr_vec );

template < class vecpt >
min::unsptr min::get_reverse_attrs
	( min::reverse_attr_info * out, min::unsptr n,
	  MUP::attr_ptr_type < vecpt > & ap )
{
    typedef MUP::attr_ptr_type<vecpt> ap_type;
    min::unsptr m = 0;  // Return value

    switch ( ap.state )
    {
    case ap_type::INIT:
	MIN_ABORT
	    ( "min::get_reverse_attrs called before"
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
template min::unsptr min::get_reverse_attrs
	( min::reverse_attr_info * out, min::unsptr n,
	  min::attr_ptr & ap );
template min::unsptr min::get_reverse_attrs
	( min::reverse_attr_info * out, min::unsptr n,
	  min::attr_updptr & ap );
template min::unsptr min::get_reverse_attrs
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
	  == ap_type::REVERSE_LOCATE_SUCCEED );

    min::gen c = current ( rap.dlp );
    if ( ! is_sublist ( c ) )
    {
        MIN_ASSERT ( c == v );
	min::update ( rap, min::EMPTY_SUBLIST() );
	return;
    }
    start_sublist ( rap.dlp );
    for ( c = current ( rap.dlp );
          c != v && ! is_list_end ( c );
	  c = next ( rap.dlp ) );

    MIN_ASSERT ( c == v );
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
	  == ap_type::REVERSE_LOCATE_SUCCEED );

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
	    MINT::attr_create ( ap, * in );
	    return;
	}
        else
	    MINT::attr_create
	        ( ap, min::EMPTY_SUBLIST() );

	if ( ap.reverse_attr_name == min::NONE() )
	    break;

    case ap_type::REVERSE_LOCATE_FAIL:
	if ( n == 0 )
	    return;
        else if ( n == 1 )
	{
	    MINT::reverse_attr_create ( ap, * in );
	    MINT::add_reverse_attr_value ( ap, * in );
	    return;
	}
        else
	    MINT::reverse_attr_create
	        ( ap, min::EMPTY_SUBLIST() );
	break;
    }

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
		for ( c = current ( ap.lp );
		         ! is_list_end ( c )
		      && ! is_sublist ( c )
		      && ! is_control_code ( c );
		    )
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
	    MINT::attr_create ( ap, * in );
	    return;
	}
        else
	    MINT::attr_create
	        ( ap, min::EMPTY_SUBLIST() );

	if ( ap.reverse_attr_name == min::NONE() )
	    break;

    case ap_type::REVERSE_LOCATE_FAIL:
        if ( n == 1 )
	{
	    MINT::reverse_attr_create ( ap, * in );
	    MINT::add_reverse_attr_value ( ap, * in );
	    return;
	}
        else
	    MINT::reverse_attr_create
	        ( ap, min::EMPTY_SUBLIST() );
	break;
    }

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
        MIN_ASSERT ( is_control_code ( in[i] ) );

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
	MIN_ASSERT ( c == min::LIST_END() );

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
    typedef min::attr_insptr ap_type;

    for ( unsigned i = 0; i < n; ++ i )
        MIN_ASSERT ( is_control_code ( in[i] ) );

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

// Printers
// --------

min::locatable_var<min::printer> min::error_message;


const min::uns32 min::standard_op_flags =
    min::EXPAND_HT;

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
    0xFFFFFFFF, min::IS_UNSUPPORTED
};

const min::display_control
        min::display_all_display_control =
{
    0xFFFF, 0
};

const min::display_control
        min::graphic_and_space_display_control =
{
    min::IS_HSPACE + min::IS_GRAPHIC,
    0
};

const min::display_control
        min::graphic_only_display_control =
{
    min::IS_GRAPHIC,
    0
};

const min::display_control
        min::graphic_and_vspace_display_control =
{
      min::IS_GRAPHIC
    + min::IS_VSPACE
    + min::IS_HSPACE,
    0
};


const min::break_control
	min::no_auto_break_break_control =
{
    0, 0, 0, 0
};

const min::break_control
	min::break_after_space_break_control =
{
    min::IS_SP + min::IS_OTHER_HSPACE, 0, 0, 0
};

const min::break_control
	min::break_before_all_break_control =
{
    0, 0xFFFF, 0, 0
};

const min::break_control
   min::break_after_hyphens_break_control =
{
    min::IS_SP + min::IS_OTHER_HSPACE, 0,
    min::CONDITIONAL_BREAK, 4
};

static min::char_name_format standard_char_name_format =
{
    (const min::ustring *) "\x01\x01<",
    (const min::ustring *) "\x01\x01>"
};
const min::char_name_format *
	min::standard_char_name_format =
    & ::standard_char_name_format;

const min::line_break min::default_line_break =
{
    0, 0, 72, 4
};

// const min::print_format min::default_print_format
// defined below after top_gen_format.

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

    if ( file != NULL_STUB )
        file_ref(printer) = file;
    else
	init_input ( file_ref(printer) );

    printer->column = 0;
    printer->line_break = min::default_line_break;
    printer->print_format = min::default_print_format;
    printer->state = min::NON_GRAPHIC_STATE;

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
	  const min::ustring * str )
{
    min::uns32 length = min::ustring_length ( str );
    const char * p = min::ustring_chars ( str );
    min::push ( buffer, length, p );
}

static void end_line ( min::printer printer )
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
    if (   printer->print_format.op_flags
         & min::DISPLAY_EOL )
    {
        min::Uchar c = min::unicode::SOFTWARE_NL;
	assert ( c < min::unicode::index_size );
        min::uns16 cindex = min::unicode::index[c];
	assert
	    ( min::unicode::picture[cindex] != NULL );
	assert
	    ( min::unicode::name[cindex] != NULL );

	if ( printer->print_format.op_flags
	     &
	     min::DISPLAY_PICTURE )
	    ::push ( buffer,
	             min::unicode::picture [cindex] );
	else
	{
	    ::push ( buffer,
	             printer->print_format
		     	     .char_name_format
			    ->char_name_prefix );
	    ::push ( buffer,
	             min::unicode::name [cindex] );
	    ::push ( buffer,
	             printer->print_format
		     	     .char_name_format
			    ->char_name_postfix );
	}
    }

    min::end_line ( printer->file );

    printer->column = 0;
    printer->line_break.offset = buffer->length;
    printer->line_break.column = 0;
    printer->state = min::NON_GRAPHIC_STATE;
}

static bool insert_line_break
	( min::printer printer );

static min::printer flush_one_id
	( min::printer printer );

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
    case min::op::MAP_PGEN:
    {
        min::gen g = MUP::new_gen ( op.v1.g );
	const min::stub * s = min::stub_of ( g );
	if ( s != min::NULL_STUB )
	{
	    min::uns32 id =
		printer->id_map != min::NULL_STUB ?
		    min::find ( printer->id_map, s ) :
		    0;
	    if ( id != 0 )
		return printer << min::indent
		               << "@" << id << min::eol;
	    else if ( min::is_obj ( g ) )
	    {
		if ( printer->id_map == min::NULL_STUB )
		    min::init
		        ( min::id_map_ref ( printer ) );
	        min::find_or_add ( printer->id_map, s );
		return printer << min::flush_id_map;
	    }
	}
	return printer << min::indent << min::pgen ( g )
	               << min::eol;
    }

    case min::op::FLUSH_ONE_ID:
        return ::flush_one_id ( printer );
    case min::op::FLUSH_ID_MAP:
    {
        min::id_map id_map = printer->id_map;
	if ( id_map != min::NULL_STUB )
	while ( id_map->next < id_map->length )
	    ::flush_one_id ( printer );
	return printer;
    }

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
    case min::op::PUSTRING:
        return min::print_ustring
	         ( printer,
		   (const min::ustring *) op.v1.p );
    case min::op::PINT:
	sprintf ( buffer, (const char *) op.v2.p,
			  op.v1.i64 );
	return printer << buffer;
    case min::op::PUNS:
	sprintf ( buffer, (const char *) op.v2.p,
			  op.v1.u64 );
	return printer << buffer;
    case min::op::PFLOAT:
	sprintf ( buffer, (const char *) op.v2.p,
			  op.v1.f64 );
	return printer << buffer;
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
	printer->print_format.op_flags |= op.v1.u32;
	return printer;
    case min::op::CLEAR_PRINT_OP_FLAGS:
	printer->print_format.op_flags &= ~ op.v1.u32;
	return printer;
    case min::op::SET_LINE_DISPLAY:
    {
        min::uns32 flags = op.v1.u32;
	flags &= min::DISPLAY_EOL
	       + min::DISPLAY_PICTURE
	       + min::DISPLAY_NON_GRAPHIC;
	printer->print_format.op_flags &=
	    ~ (   min::DISPLAY_EOL
	        + min::DISPLAY_PICTURE
		+ min::DISPLAY_NON_GRAPHIC );
	printer->print_format.op_flags |= flags;
	printer->print_format.op_flags |=
	    min::EXPAND_HT;
	printer->print_format.display_control =
	    flags & min::DISPLAY_NON_GRAPHIC ?
	    min::graphic_only_display_control :
	    min::graphic_and_space_display_control;
	printer->print_format.break_control =
	    min::no_auto_break_break_control;
	return printer;
    }
    case min::op::SET_SUPPORT_CONTROL:
        printer->print_format.support_control =
	    * (const min::support_control *) op.v1.p;
	return printer;
    case min::op::SET_DISPLAY_CONTROL:
        printer->print_format.display_control =
	    * (const min::display_control *) op.v1.p;
	return printer;
    case min::op::SET_BREAK_CONTROL:
        printer->print_format.break_control =
	    * (const min::break_control *) op.v1.p;
	return printer;
    case min::op::VERBATIM:
	printer->print_format.op_flags &=
	    ~ min::EXPAND_HT;
	printer->print_format.support_control =
	    min::support_all_support_control;
	printer->print_format.display_control =
	    min::display_all_display_control;
	printer->print_format.break_control =
	    min::no_auto_break_break_control;
	return printer;
    case min::op::SPACE:
	return min::print_spaces ( printer, 1 );
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
	return printer;
    case min::op::BOM:
        min::push ( printer->print_format_stack ) =
	    printer->print_format;
	goto save_indent;
    case min::op::EOM:
	printer->line_break =
	    min::pop ( printer->line_break_stack );
        if ( printer->column != 0 )
	    ::end_line ( printer );
	if (   printer->print_format.op_flags
	     & min::FLUSH_ON_EOL )
	    min::flush_file ( printer->file );

	if (   printer->print_format.op_flags
	     & min::FLUSH_ID_MAP_ON_EOM )
	{
	    min::id_map id_map = printer->id_map;
	    if ( id_map != min::NULL_STUB )
	    while ( id_map->next < id_map->length )
		::flush_one_id ( printer );
	}

	goto restore_print_format;
    case min::op::EOL_IF_AFTER_INDENT:
        if (    printer->column
	     <= printer->line_break.indent )
	    return printer;
	else
	    goto eol;
    case min::op::BOL:
        if ( printer->column == 0 ) return printer;
	// Fall through to EOL.
    eol:
    case min::op::EOL:
	::end_line ( printer );
	if (   printer->print_format.op_flags
	     & min::FLUSH_ON_EOL )
	    min::flush_file ( printer->file );
	return printer;
    case min::op::FLUSH:
	min::flush_file ( printer->file );
	return printer;
    case min::op::SET_BREAK:
    set_break:
	printer->line_break.offset =
	    printer->file->buffer->length;
	printer->line_break.column = printer->column;
	return printer;
    case min::op::LEFT:
        if (   printer->column
	     <   printer->line_break.column
	       + op.v1.u32 )
	    min::print_spaces
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
		    ::insert_line_break ( printer );
		    offset = printer->line_break.offset;
		}
	    }

	    min::push ( buffer, n );
	    if ( len > 0 )
	        memmove ( ! & buffer[offset + n],
		          ! & buffer[offset],
			  len );

	    printer->column += n;
	    while ( n -- ) buffer[offset++] = ' ';
	}
	goto set_break;
    case min::op::RESERVE:
        if (    printer->column + op.v1.u32
	     <= printer->line_break.line_length )
	    return printer;
	// Fall through to INDENT.
    case min::op::INDENT:
        if (   printer->column
	     > printer->line_break.indent )
	    ::end_line ( printer );
    execute_indent:
	if (   printer->column
	     < printer->line_break.indent )
	    min::print_spaces
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
	    min::print_spaces ( printer, 1 );
	return printer;
    case min::op::LEADING:
        if ( printer->state & min::NON_GRAPHIC_STATE )
	    return printer;
	printer->state |= min::AFTER_LEADING
	                + min::FORCE_SPACE_OK;
	return printer;
    case min::op::TRAILING:
        if ( printer->state & min::NON_GRAPHIC_STATE )
	    return printer;
	printer->state |= min::AFTER_TRAILING
	                + min::FORCE_SPACE_OK;
	return printer;
    case min::op::LEADING_ALWAYS:
        if ( printer->state & min::NON_GRAPHIC_STATE )
	    return printer;
	printer->state |= min::AFTER_LEADING;
	return printer;
    case min::op::TRAILING_ALWAYS:
        if ( printer->state & min::NON_GRAPHIC_STATE )
	    return printer;
	printer->state |= min::AFTER_TRAILING;
	return printer;
    case min::op::PRINT_ASSERT:
        // For debugging only.
	{
	    static const min::uns32 * adr = NULL;
	    if ( adr == NULL )
	        adr = & printer->line_break.indent;
	    assert
	        ( adr == & printer->line_break.indent );
	}
	assert ( printer->line_break.indent < 1000 );
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

const min::op min::leading
    ( min::op::LEADING );
const min::op min::trailing
    ( min::op::TRAILING );
const min::op min::leading_always
    ( min::op::LEADING_ALWAYS );
const min::op min::trailing_always
    ( min::op::TRAILING_ALWAYS );

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

const min::op min::graphic_and_space
    ( min::op::SET_DISPLAY_CONTROL,
      & min::graphic_and_space_display_control );
const min::op min::graphic_only
    ( min::op::SET_DISPLAY_CONTROL,
      & min::graphic_only_display_control );
const min::op min::graphic_and_vspace
    ( min::op::SET_DISPLAY_CONTROL,
      & min::graphic_and_vspace_display_control );
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
// break and false if there MIGHT be an enabled line
// break.  The return value can be used to avoid
// repeated calls to check for break insertion if
// no break points are set between calls.
//
static bool insert_line_break ( min::printer printer )
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
	        ( ! & buffer[ begoff
		             +line_break.indent+1],
		  ! & buffer[endoff],
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
	        ( ! & buffer[begoff+line_break.indent
		                   +1],
		  ! & buffer[endoff],
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

min::printer MINT::print_unicode
	( min::printer printer,
	  min::unsptr & n,
	  min::ptr<const min::Uchar> & p,
	  min::uns32 & width,
	  const min::display_control * display_control,
	  const min::Uchar * substring,
	  min::unsptr substring_length,
	  const min::ustring * replacement )
{
    if ( n == 0 ) return printer;

    char temp[32];

    min::support_control sc =
        printer->print_format.support_control;
    min::display_control dc =
        display_control != NULL ? * display_control :
        printer->print_format.display_control;
    if (   printer->print_format.op_flags
         & min::DISPLAY_NON_GRAPHIC )
    {
        dc.display_char &= ~ min::IS_NON_GRAPHIC;
        dc.display_suppress &= ~ min::IS_NON_GRAPHIC;
    }
    min::break_control bc =
        printer->print_format.break_control;
    const min::uns32 * char_flags =
	printer->print_format.char_flags;

    min::uns32 line_length =
        printer->line_break.line_length;

    min::packed_vec_insptr<char> buffer =
        printer->file->buffer;
    min::uns32 expand_ht =
        printer->print_format.op_flags & min::EXPAND_HT;


    bool no_line_break_enabled = false;
        // This prevents repeated checks for an enabled
	// line break that does not exist.

    min::uns32 flags = printer->state
                     & (   min::AFTER_LEADING
		         + min::AFTER_TRAILING
			 + min::FORCE_SPACE_OK );
    if ( flags )
    {
        printer->state &= ~ flags;

        min::Uchar c = * p;
	min::uns16 cindex = min::Uindex ( c );
	min::uns32 cflags = char_flags[cindex];
	if ( ( cflags & sc.support_mask ) == 0 )
	    cflags = sc.unsupported_char_flags;

	if ( cflags & min::IS_NON_GRAPHIC )
	    flags = 0; // Do NOT print space.
	else if ( ( flags & (   min::AFTER_LEADING
	                      + min::AFTER_TRAILING ) )
		  == (   min::AFTER_LEADING
		       + min::AFTER_TRAILING ) )
	    /* Do nothing, i.e., print space. */;
	else if ( (   printer->print_format.op_flags
	            & min::FORCE_SPACE )
		  &&
		  ( flags & min::FORCE_SPACE_OK ) )
	    /* Do nothing, i.e., print space. */;
	else if ( flags & min::IS_LEADING
		  &&
		  ! (   printer->state
		      & min::LEADING_STATE ) )
	    /* Do nothing; i.e. print space. */;
	else if ( flags & min::IS_TRAILING
		  &&
		  ! (   printer->state
		      & min::TRAILING_STATE ) )
	    /* Do nothing; i.e. print space. */;
	else if ( flags & min::IS_LEADING
	          &&
	          cflags & min::IS_MIDDLING )
	    flags = 0; // Do NOT print space.
	else if ( flags & cflags )
	{
	    assert (    min::IS_LEADING
	             == min::AFTER_LEADING );
	    assert (    min::IS_TRAILING
	             == min::AFTER_TRAILING );

	    flags &= (   min::IS_LEADING
		       + min::IS_TRAILING );
	    min::uns32 sflags = flags;

	    min::unsptr m = 0;
	    while ( true )
	    {
		if ( cflags & min::IS_NON_GRAPHIC )
		    break;
		sflags &= cflags;

		if ( sflags == 0 || ++ m >= n ) break;

		c = p[m];
		cindex = min::Uindex ( c );
		cflags = char_flags[cindex];
		if ( ( cflags & sc.support_mask ) == 0 )
		    cflags = sc.unsupported_char_flags;
	    }
	    flags ^= sflags;
	}

	if ( flags ) min::print_spaces ( printer );

	if ( printer->state & min::AFTER_SAVE_INDENT )
	{
	    printer->state &= ! min::AFTER_SAVE_INDENT;
	    printer << min::save_indent;
	}
    }

    bool rep_is_space;
    printer->and_ed_char_flags = min::IS_LEADING
                               + min::IS_TRAILING;
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
	const min::ustring * prefix = NULL;
	const min::ustring * postfix = NULL;
	rep_is_space = false;

	if (    substring != NULL
	     && c == substring[0]
	     && n >= substring_length
	     &&    memcmp ( !p, substring,
	                        substring_length )
		== 0 )
	{
	    // Found substring; output replacement.
	    //
	    columns = min::ustring_columns
		         ( replacement );
	    assert ( columns > 0 );
	    length = 0;
	    prefix = replacement;
	    clength = substring_length;
	}
	else if ( cflags & dc.display_char )
	{
	    if ( c == '\t' )
	    {
		min::uns32 spaces =
		    8 - printer->column % 8;
		columns = spaces;
	        if ( expand_ht )
		{
		    ::strcpy ( temp, "        " );
		    temp[spaces] = 0;
		    length = spaces;
		}
		else
		{
		    temp[0] = (char) c;
		    temp[1] = 0;
		}
		rep_is_space = true;
	    }
	    else if ( 0 < c && c < 128 )
	    {
	        temp[0] = (char) c;
		temp[1] = 0;
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
	else if ( (   printer->print_format.op_flags
	            & min::DISPLAY_PICTURE )
	          &&
		     min::unicode::picture[cindex]
		  != NULL )
	{
	    const min::ustring * picture =
	        min::unicode::picture[cindex];
	    length = min::ustring_length ( picture );
	    columns = min::ustring_columns ( picture );
	    assert ( columns > 0 );
	    rep = min::ustring_chars ( picture );
	}
	else
	{
	    if ( min::unicode::name[cindex] != NULL )
	    {
		const min::ustring * name =
		    min::unicode::name[cindex];
		length = min::ustring_length ( name );
		columns = min::ustring_columns ( name );
		assert ( columns > 0 );
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
	    assert ( prefix_columns > 0 );
	    min::uns32 postfix_columns =
	        min::ustring_columns ( postfix );
	    assert ( postfix_columns > 0 );
	    columns += prefix_columns + postfix_columns;
	}

	if ( columns > width )
	    return printer;

	n -= clength;
	p = p + clength;
	printer->last_char_flags = cflags;
	if ( cflags & min::IS_GRAPHIC )
	    printer->and_ed_char_flags &= cflags;
	else
	{
	    printer->and_ed_char_flags =
	        min::IS_LEADING + min::IS_TRAILING;
	    printer->state |= min::NON_GRAPHIC_STATE;
	    printer->state &=
	        ~ (   min::LEADING_STATE
		    + min::TRAILING_STATE );
	}

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

	    if ( printer->column + columns > line_length
	         &&
		 ! rep_is_space
		 &&
		 ! no_line_break_enabled )
	    {
	        no_line_break_enabled =
		    ::insert_line_break ( printer );
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

    if ( ( printer->last_char_flags & min::IS_GRAPHIC )
         == 0 )
    {
	printer->state |= min::NON_GRAPHIC_STATE;
	printer->state &= ~ (   min::LEADING_STATE
	                      + min::TRAILING_STATE );
    }
    else if (   printer->last_char_flags
              & min::IS_MIDDLING )
    {
	printer->state &=
	    ~ (   min::LEADING_STATE
	        + min::NON_GRAPHIC_STATE );
	printer->state |= min::TRAILING_STATE;
    }
    else if (   printer->and_ed_char_flags
              & min::IS_LEADING )
    {
        if (   printer->state
             & min::NON_GRAPHIC_STATE )
	    printer->state |= min::LEADING_STATE;
	printer->state &=
	    ~ (   min::TRAILING_STATE
	        + min::NON_GRAPHIC_STATE );
    }
    else if (   printer->and_ed_char_flags
              & min::IS_TRAILING )
    {
        if (   printer->state
             & min::NON_GRAPHIC_STATE )
	    printer->state |= min::TRAILING_STATE;
	printer->state &=
	    ~ (   min::LEADING_STATE
	        + min::NON_GRAPHIC_STATE );
    }
    else
	printer->state &=
	    ~ (   min::LEADING_STATE
	        + min::TRAILING_STATE
	        + min::NON_GRAPHIC_STATE );

    return printer;
}

static min::printer print_quoted_unicode
	( min::printer printer,
	  min::unsptr length,
	  min::ptr<const min::Uchar> p,
	  const min::str_format * sf )
{
    min::bracket_format bf = sf->bracket_format;
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
        min::ustring_columns ( bf.str_prefix );
    assert ( prefix_columns > 0 );
    min::uns32 postfix_columns =
        min::ustring_columns ( bf.str_postfix );
    assert ( postfix_columns > 0 );

    min::uns32 reduced_width =
          printer->line_break.line_length
        - line_break.indent
	- prefix_columns
        - postfix_columns;
    if ( reduced_width < 10 ) reduced_width = 10;

    min::uns32 postfix_length =
        min::ustring_length ( bf.str_postfix );
    min::Uchar postfix_string[postfix_length+1];
    {
	const char * p =
	    min::ustring_chars ( bf.str_postfix );
	const char * endp = p + postfix_length;
	min::Uchar * q = postfix_string;
	while ( p != endp )
	    * q ++ = min::utf8_to_unicode ( p, endp );
	assert (    q - postfix_string
	         <= postfix_length + 1 );
	postfix_length = q - postfix_string;
    }

    printer << min::pustring ( bf.str_prefix );

    min::uns32 width = reduced_width;
    while ( length > 0 )
    {
	MINT::print_unicode
	    ( printer, length, p, width,
	               & sf->display_control,
	               postfix_string,
		       postfix_length,
		       bf.str_postfix_replacement );

	if ( length == 0 ) break;

	printer << min::pustring ( bf.str_postfix )
		<< " "
		<< min::set_break
		<< min::pustring ( bf.str_concatenator )
		<< " "
		<< min::pustring ( bf.str_prefix );

	min::uns32 cat_columns =
	      min::ustring_columns
		  ( bf.str_concatenator );
	assert ( cat_columns > 0 );
	width = reduced_width - 1 - cat_columns;
    }
    printer << min::pustring ( bf.str_postfix );

    return printer;
}

inline min::uns32 compute_flags
	( const min::print_format & print_format,
	  min::Uchar c )
{
    min::support_control sc =
        print_format.support_control;
    const min::uns32 * char_flags =
	print_format.char_flags;

    min::uns16 cindex = min::Uindex ( c );
    min::uns32 cflags = char_flags[cindex];
    if ( ( cflags & sc.support_mask ) == 0 )
	cflags = sc.unsupported_char_flags;
    return cflags;
}

bool min::in_str_class
	( const min::uns32 * char_flags,
	  min::support_control sc,
	  min::unsptr n,
	  min::ptr<const min::Uchar> p,
	  min::str_classifier strcl )
{
    if ( n == 0 ) return false;

    bool first = true;
    while ( n -- )
    {
        min::Uchar c = * p ++;
	min::uns32 cflags =
	    min::char_flags ( char_flags, sc, c );

	if ( ( cflags & strcl.in_class_if_all ) == 0 )
	    return false;
	if ( first
	     &&
	     (    ( cflags & strcl.skip_if_first ) == 0
	       || n == 0 ) )
	{
	    if ( ( cflags & strcl.in_class_if_first )
		 == 0 )
	        return false;
	    else if (    strcl.in_class_if_all
	              == min::ALL_CHARS )
	        return true;
	    else
		first = false;
	}
    }
    return true;
}

min::printer min::print_unicode
	( min::printer printer,
	  min::unsptr n,
	  min::ptr<const min::Uchar> p,
	  const min::str_format * sf )
{
    if ( sf != NULL
         &&
         ! min::in_str_class
	       ( printer->print_format.char_flags,
	         printer->print_format
		         .support_control,
	         n, p, sf->quote_control ) )
	return ::print_quoted_unicode
	    ( printer, n, p, sf );

    min::uns32 width = 0xFFFFFFFF;

    return MINT::print_unicode ( printer, n, p, width );
}



// NOTE: Importantly this function copies s to the
// stack BEFORE calling anything that might relocate
// memory.
//
min::printer min::print_chars
	( min::printer printer, const char * s,
	  const min::str_format * sf )
{
    // We translate this case to the punicode case
    // because we need to be able to perform look-ahead
    // for combining diacritics, and in the future for
    // other things.
    
    int length = ::strlen ( s );
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

min::printer MINT::print_ustring
	( min::printer printer,
	  const min::ustring * s )
{
    min::uns32 flags = ustring_flags ( s );

    if ( flags & USTRING_TRAILING )
        printer << min::trailing;
    else if ( flags & USTRING_TRAILING_ALWAYS )
        printer << min::trailing_always;

    min::print_chars
	( printer, ustring_chars ( s ) );

    if ( flags & USTRING_LEADING )
        printer << min::leading;
    else if ( flags & USTRING_LEADING_ALWAYS )
        printer << min::leading_always;

    return printer;
}
    

min::printer operator <<
	( min::printer printer, min::int64 i )
{
    char buffer[32];
    sprintf ( buffer, "%lld", i );
    return printer << buffer;
}

min::printer operator <<
	( min::printer printer, min::uns64 u )
{
    char buffer[32];
    sprintf ( buffer, "%llu", u );
    return printer << buffer;
}

min::printer operator <<
	( min::printer printer, min::float64 f )
{
    char buffer[64];
    sprintf ( buffer, "%.15g", f );
    return printer << buffer;
}

void min::pwidth ( min::uns32 & column,
                   const char * s, min::unsptr n,
		   const min::print_format &
		       print_format )
{
    min::support_control sc =
        print_format.support_control;
    min::display_control dc =
        print_format.display_control;
    if (   print_format.op_flags
         & min::DISPLAY_NON_GRAPHIC )
    {
        dc.display_char &= ~ min::IS_NON_GRAPHIC;
        dc.display_suppress &= ~ min::IS_NON_GRAPHIC;
    }
    const min::uns32 * char_flags =
	print_format.char_flags;

    const char * ends = s + n;
    char temp[32];

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
	else if ( (   print_format.op_flags
	            & min::DISPLAY_PICTURE )
		  &&
		  unicode::picture[cindex] != NULL )
	{
	    const ustring * picture =
	        unicode::picture[cindex];
	    min::uns32 picture_columns =
	        ustring_columns ( picture );
	    assert ( picture_columns > 0 );
	    column += picture_columns;
	}
	else
	{
	    if ( unicode::name[cindex] != NULL )
	    {
		const ustring * name =
		    unicode::name[cindex];
		min::uns32 name_columns =
		    ustring_columns ( name );
		assert ( name_columns > 0 );
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
	    assert ( prefix_columns > 0 );
	    min::uns32 postfix_columns =
		min::ustring_columns
		    ( print_format.char_name_format
				 ->char_name_postfix );
	    assert ( postfix_columns > 0 );

	    column += prefix_columns + postfix_columns;
	}
    }
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
	out << "[:";
	for ( unsigned i = 0; i < n; ++ i )
	    out << " " << lp[i];
	return out << " :]";
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
        nf = printer->print_format.gen_format
	            ->num_format;

    char buffer[128];
    if ( fabs ( value ) < nf->non_float_bound )
    {
	long long I = (long long) floor ( value );
	if ( I == value )
	{
	    sprintf ( buffer,
		      nf->int_printf_format,
		      value );
	    return printer << buffer;
	}

	if ( nf->fraction_divisors != NULL )
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
	        sprintf ( p, "%d/%d", N, D );
		return printer << buffer;
	    }
	}
    }

    sprintf ( buffer, nf->float_printf_format, value );
    return printer << buffer;
}

const min::str_classifier
	min::quote_all_control =
{
    0, 0, 0
};

const min::str_classifier
	min::quote_first_not_letter_control =
{
    min::QUOTE_SUPPRESS, min::QUOTE_SKIP,
    min::IS_GRAPHIC
};

const min::str_classifier
	min::quote_non_graphic_control =
{
    min::ALL_CHARS, 0, min::IS_GRAPHIC
};

const min::bracket_format min::quote_bracket_format =
{
    (const min::ustring *) "\x01\x01" "\"",
    (const min::ustring *) "\x01\x01" "\"",
    (const min::ustring *) "\x03\x03" "<Q>",
    (const min::ustring *) "\x01\x01" "#"
};

static min::str_format quote_all_str_format =
{
    min::quote_all_control,
    min::quote_bracket_format,
    min::graphic_only_display_control,
    0xFFFFFFFF
};
const min::str_format * min::quote_all_str_format =
    & ::quote_all_str_format;

static min::str_format
	quote_first_not_letter_str_format =
{
    min::quote_first_not_letter_control,
    min::quote_bracket_format,
    min::graphic_only_display_control,
    0xFFFFFFFF
};
const min::str_format *
	min::quote_first_not_letter_str_format =
    & ::quote_first_not_letter_str_format;

static min::str_format
	quote_non_graphic_str_format =
{
    min::quote_non_graphic_control,
    min::quote_bracket_format,
    min::graphic_only_display_control,
    0xFFFFFFFF
};
const min::str_format *
	min::quote_non_graphic_str_format =
    & ::quote_non_graphic_str_format;

static min::lab_format name_lab_format =
{
    NULL,
    (const min::ustring *) "\x01\x00" " ",
    NULL
};
const min::lab_format * min::name_lab_format =
    & ::name_lab_format;

static min::lab_format bracket_lab_format =
{
    (const min::ustring *) "\x03\x00" "[: ",
    (const min::ustring *) "\x01\x00" " ",
    (const min::ustring *) "\x03\x00" " :]"
};
const min::lab_format * min::bracket_lab_format =
    & ::bracket_lab_format;

static min::lab_format leading_always_lab_format =
{
    NULL,
    (const min::ustring *) "\x40\x00" "",
    NULL
};
const min::lab_format *
    min::leading_always_lab_format =
	& ::leading_always_lab_format;

static min::lab_format trailing_always_lab_format =
{
    NULL,
    (const min::ustring *) "\x80\x00" "",
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
    (const min::ustring *) "\x02\x02" "[$",
    (const min::ustring *) "\x02\x02" "$]",
    min::NULL_STUB	    // special_names*
};
const min::special_format *
	min::bracket_special_format =
    & ::bracket_special_format;

static min::obj_format compact_obj_format =
{
    min::ENABLE_COMPACT,    // obj_op_flags

    NULL,		    // element_format*
    NULL,		    // label_format*
    NULL,		    // value_format*

    NULL,		    // initiator_format*
    NULL,		    // separator_format*
    NULL,		    // terminator_format*

    // USTRING_LEADING  == 0x40
    // USTRING_TRAILING == 0x80

    (const min::ustring *)
        "\x02\x02" "{}",    // obj_empty

    (const min::ustring *)
        "\x01\x01" "{",     // obj_bra
    (const min::ustring *)
        "\x01\x01" "|",     // obj_braend
    (const min::ustring *)
        "\x01\x01" "|",     // obj_ketbegin
    (const min::ustring *)
        "\x01\x01" "}",     // obj_ket

    (const min::ustring *)
        "\x01\x00" " ",     // obj_sep

    (const min::ustring *)
        "\x82\x00" ": ",    // obj_attrbegin
    (const min::ustring *)
        "\x02\x00" ", ",    // obj_attrsep

    (const min::ustring *)
        "\x81\x01" ":",     // obj_attreol

    (const min::ustring *)
        "\x03\x00" "no ",   // obj_attrneg
    (const min::ustring *)
        "\x03\x00" " = ",   // obj_attreq

    (const min::ustring *)
        "\x03\x00" "{* ",   // obj_valbegin
    (const min::ustring *)
        "\x02\x00" ", ",    // obj_valsep
    (const min::ustring *)
        "\x03\x00" " *}",   // obj_valend
    (const min::ustring *)
        "\x04\x00" " <= ",  // obj_valreq

    min::quote_all_control, // marking_type

    min::NULL_STUB	    // attr_flag_names*
};
const min::obj_format * min::compact_obj_format =
    & ::compact_obj_format;

static min::obj_format isolated_line_obj_format =
{
    min::ISOLATED_LINE,	    // obj_op_flags

    NULL,		    // element_format*
    NULL,		    // label_format*
    NULL,		    // value_format*

    NULL,		    // initiator_format*
    NULL,		    // separator_format*
    NULL,		    // terminator_format*

    // USTRING_LEADING  == 0x40
    // USTRING_TRAILING == 0x80

    NULL,		    // obj_empty

    NULL,		    // obj_bra
    NULL,		    // obj_braend
    NULL,		    // obj_ketbegin
    NULL,		    // obj_ket

    (const min::ustring *)
        "\x01\x00" " ",     // obj_sep

    NULL,		    // obj_attrbegin
    NULL,		    // obj_attrsep

    (const min::ustring *)
        "\x81\x01" ":",     // obj_attreol

    (const min::ustring *)
        "\x03\x00" "no ",   // obj_attrneg
    (const min::ustring *)
        "\x03\x00" " = ",   // obj_attreq

    (const min::ustring *)
        "\x03\x00" "{* ",   // obj_valbegin
    (const min::ustring *)
        "\x02\x00" ", ",    // obj_valsep
    (const min::ustring *)
        "\x03\x00" " *}",   // obj_valend
    (const min::ustring *)
        "\x04\x00" " <= ",  // obj_valreq

    min::quote_all_control, // marking_type

    min::NULL_STUB	    // attr_flag_names*
};
const min::obj_format * min::isolated_line_obj_format =
    & ::isolated_line_obj_format;

static min::obj_format embedded_line_obj_format =
{
    min::EMBEDDED_LINE,	    // obj_op_flags

    NULL,		    // element_format*
    NULL,		    // label_format*
    NULL,		    // value_format*

    NULL,		    // initiator_format*
    NULL,		    // separator_format*
    NULL,		    // terminator_format*

    // USTRING_LEADING  == 0x40
    // USTRING_TRAILING == 0x80

    (const min::ustring *)
        "\x04\x04" "{||}",  // obj_empty

    (const min::ustring *)
        "\x01\x01" "{",     // obj_bra
    (const min::ustring *)
        "\x01\x01" "|",     // obj_braend
    (const min::ustring *)
        "\x01\x01" "|",     // obj_ketbegin
    (const min::ustring *)
        "\x01\x01" "}",     // obj_ket

    (const min::ustring *)
        "\x01\x00" " ",     // obj_sep

    NULL,		    // obj_attrbegin
    NULL,		    // obj_attrsep

    (const min::ustring *)
        "\x81\x01" ":",     // obj_attreol

    (const min::ustring *)
        "\x03\x00" "no ",   // obj_attrneg
    (const min::ustring *)
        "\x03\x00" " = ",   // obj_attreq

    (const min::ustring *)
        "\x03\x00" "{* ",   // obj_valbegin
    (const min::ustring *)
        "\x02\x00" ", ",    // obj_valsep
    (const min::ustring *)
        "\x03\x00" " *}",   // obj_valend
    (const min::ustring *)
        "\x04\x00" " <= ",  // obj_valreq

    min::quote_all_control, // marking_type

    min::NULL_STUB	    // attr_flag_names*
};
const min::obj_format * min::embedded_line_obj_format =
    & ::embedded_line_obj_format;

static min::obj_format id_obj_format =
{
    min::PRINT_ID,	    // obj_op_flags

    NULL,		    // element_format
    NULL,		    // label_format
    NULL,		    // value_format

    NULL,		    // initiator_format
    NULL,		    // separator_format
    NULL,		    // terminator_format

    NULL,		    // obj_empty

    NULL,		    // obj_bra
    NULL,		    // obj_braend
    NULL,		    // obj_ketbegin
    NULL,		    // obj_ket

    NULL,		    // obj_sep

    NULL,		    // obj_attrbegin
    NULL,		    // obj_attrsep

    NULL,		    // obj_attreol

    NULL,		    // obj_attrneg
    NULL,		    // obj_attreq

    NULL,		    // obj_valbegin
    NULL,		    // obj_valsep
    NULL,		    // obj_valend
    NULL,		    // obj_valreq

    {0,0,0},		    // marking_type

    min::NULL_STUB	    // attr_flag_names
};
const min::obj_format * min::id_obj_format =
    & ::id_obj_format;

static min::gen_format top_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_first_not_letter_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL,			    // id_map_format
};
const min::gen_format * min::top_gen_format =
    & ::top_gen_format;

static min::gen_format id_map_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_first_not_letter_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::isolated_line_obj_format,
    NULL,			    // id_map_format
};
const min::gen_format * min::id_map_gen_format =
    & ::id_map_gen_format;

static min::gen_format name_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_first_not_letter_str_format,
    & ::name_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL,			    // id_map_format
};
const min::gen_format * min::name_gen_format =
    & ::name_gen_format;

static min::gen_format leading_always_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    NULL,
    & ::leading_always_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL
};
const min::gen_format *
    min::leading_always_gen_format =
	& ::leading_always_gen_format;

static min::gen_format trailing_always_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    NULL,
    & ::trailing_always_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL
};
const min::gen_format *
    min::trailing_always_gen_format =
	& ::trailing_always_gen_format;

static min::gen_format value_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_first_not_letter_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL,			    // id_map_format
};
const min::gen_format * min::value_gen_format =
    & ::value_gen_format;

static min::gen_format element_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_first_not_letter_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL,			    // id_map_format
};
const min::gen_format * min::element_gen_format =
    & ::element_gen_format;

static min::gen_format id_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_first_not_letter_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::id_obj_format,
    NULL,			    // id_map_format
};
const min::gen_format * min::id_gen_format =
    & ::id_gen_format;

static min::gen_format always_quote_gen_format =
{
    & min::standard_pgen,
    & ::long_num_format,
    & ::quote_all_str_format,
    & ::bracket_lab_format,
    & ::bracket_special_format,
    & ::compact_obj_format,
    NULL,			    // id_map_format
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
    NULL,			    // id_map_format
};
const min::gen_format * min::never_quote_gen_format =
    & ::never_quote_gen_format;

const min::print_format min::default_print_format =
{
    min::EXPAND_HT,
    ::standard_char_flags,

    min::latin1_support_control,
    min::graphic_and_space_display_control,
    min::break_after_space_break_control,

    & ::standard_char_name_format,
    & ::top_gen_format
};

static void init_pgen_formats ( void )
{
    ::name_special_format.special_names =
        min::standard_special_names;

    ::bracket_special_format.special_names =
        min::standard_special_names;

    ::compact_obj_format.element_format =
        min::top_gen_format;
    ::compact_obj_format.label_format =
        min::name_gen_format;
    ::compact_obj_format.value_format =
        min::top_gen_format;
    ::compact_obj_format.initiator_format =
        min::leading_always_gen_format;
    ::compact_obj_format.separator_format =
        min::trailing_always_gen_format;
    ::compact_obj_format.terminator_format =
        min::trailing_always_gen_format;
    ::compact_obj_format.attr_flag_names =
        min::standard_attr_flag_names;

    ::isolated_line_obj_format.element_format =
        min::id_gen_format;
    ::isolated_line_obj_format.label_format =
        min::name_gen_format;
    ::isolated_line_obj_format.value_format =
        min::id_gen_format;
    ::isolated_line_obj_format.initiator_format =
        min::leading_always_gen_format;
    ::isolated_line_obj_format.separator_format =
        min::trailing_always_gen_format;
    ::isolated_line_obj_format.terminator_format =
        min::trailing_always_gen_format;
    ::isolated_line_obj_format.attr_flag_names =
        min::standard_attr_flag_names;

    ::embedded_line_obj_format.element_format =
        min::top_gen_format;
    ::embedded_line_obj_format.label_format =
        min::name_gen_format;
    ::embedded_line_obj_format.value_format =
        min::top_gen_format;
    ::embedded_line_obj_format.initiator_format =
        min::leading_always_gen_format;
    ::embedded_line_obj_format.separator_format =
        min::trailing_always_gen_format;
    ::embedded_line_obj_format.terminator_format =
        min::trailing_always_gen_format;
    ::embedded_line_obj_format.attr_flag_names =
        min::standard_attr_flag_names;

    ::top_gen_format.id_map_format =
        min::id_map_gen_format;
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

    min::uns32 id =
        min::find_or_add
	    ( printer->id_map, min::stub_of ( v ) );
    return printer << "@" << id;
}

// Return true if attributes printed and false if
// nothing printed.
//
static bool print_attributes
	( min::printer printer, 
	  const min::obj_format * objf,
	  min::obj_vec_ptr & vp,
	  min::attr_ptr & ap,
	  min::attr_info * info,
	  min::unsptr m,
	  bool line_format,
	  min::gen type = min::NONE() )
{
    bool first_attr = true;
    min::int32 adjust =
	  (min::int32) printer->line_break.line_length
	- (min::int32) printer->line_break.indent;
    adjust = ( adjust > 20 ? 4 : 2 );
    for ( min::unsptr i = 0; i < m; ++ i )
    {
	if ( info[i].name == min::dot_position )
	    continue;
	else if ( info[i].name == min::dot_type
		  &&
		  info[i].value_count == 1
		  &&
		  info[i].value == type )
	    continue;

	if ( first_attr )
	{
	    first_attr = false;
	    if ( line_format )
	    {
	        printer << min::pustring
			       ( objf->obj_attreol )
			<< min::eol << min::indent
			<< min::adjust_indent
			      ( adjust );
	    }
	    else
		printer << min::pustring
			       ( objf->obj_attrbegin )
		        << min::save_indent;
	}
	else
	{
	    if ( line_format )
		printer << min::adjust_indent
			      ( - adjust )
	                << min::indent
			<< min::adjust_indent
			      ( adjust );
	    else
	    {
		min::print_ustring
		    ( printer, objf->obj_attrsep );
		printer << min::set_break;
	    }
	}

	bool suppress_value =
	    (    info[i].value_count == 1
	      && info[i].flag_count == 0
	      && info[i].reverse_attr_count == 0 );
	if ( suppress_value )
	{
	    if ( info[i].value == min::FALSE )
		min::print_ustring
		    ( printer, objf->obj_attrneg );
	    else if ( info[i].value != min::TRUE )
		suppress_value = false;
	}

	min::print_gen
	    ( printer, info[i].name,
		       objf->label_format );

	if ( suppress_value ) continue;

 
	min::unsptr vc = info[i].value_count;
	min::unsptr rc = info[i].reverse_attr_count;
	min::unsptr fc = info[i].flag_count;
	min::gen value[vc];
	min::reverse_attr_info rinfo[rc];
	if ( vc > 1 || rc > 0 || fc * min::VSIZE > 64 )
		min::locate ( ap, info[i].name );
	if ( vc > 1 ) min::get ( value, vc, ap );
	else if ( vc == 1 ) value[0] = info[i].value;
	if ( rc > 0 )
	    min::get_reverse_attrs ( rinfo, rc, ap );

	if ( fc > 0 )
	{
	    min::packed_vec_ptr<const char *>
		names = objf->attr_flag_names;
	    min::unsptr length =
	        ( names == min::NULL_STUB ?
		  0 : names->length );
	    printer << "[";
	    if ( fc * min::VSIZE <= 64 && length >= 64 )
	    {
		min::uns64 f = info[i].flags;
		for ( unsigned j = 0; j < 64; ++ j )
		{
		    if ( f & 1 )
			printer << names[j];
		    f >>= 1;
		}
	    }
	    else
	    {
		min::gen flags[fc];
		min::get_flags ( flags, fc, ap );
		min::unsptr n = 0;
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
				printer << names[n];
			    else
				printer << "," << n;
			}
			flags2 >>= 1;
			++ n;
		    }
		}
	    }
	    printer << "]";
	}

	min::print_ustring
	    ( printer, objf->obj_attreq );
	printer << min::set_break;

	if ( vc + rc != 1 && ! line_format )
	    printer << min::pustring
			   ( objf->obj_valbegin )
		    << min::save_indent;

	bool first_value = true;
	for ( min::unsptr j = 0; j < vc; ++ j )
	{
	    if ( first_value ) first_value = false;
	    else
	        printer << min::pustring
			       ( objf->obj_valsep )
			<< min::set_break;
	    min::print_gen
		( printer, value[j],
			   objf->value_format );
	}
	for ( min::unsptr j = 0; j < rc; ++ j )
	{
	    if ( first_value ) first_value = false;
	    else
	        printer << min::pustring
			       ( objf->obj_valsep )
			<< min::set_break;
	    min::unsptr rvc = rinfo[j].value_count;
	    min::gen rvalue[rvc];
	    if ( rvc > 1 )
	    {
	        min::locate_reverse
		    ( ap, rinfo[j].name );
	        min::get ( rvalue, rvc, ap );
	    }
	    else
	    {
	        assert ( rvc == 1 );
		rvalue[0] = rinfo[j].value;
	    }

	    if ( rvc > 1 )
		printer << min::pustring
			       ( objf->obj_valbegin )
			<< min::save_indent;

	    bool first_rvalue = true;
	    for ( min::unsptr k = 0; k < rvc; ++ k )
	    {
		if ( first_rvalue )
		    first_rvalue = false;
		else
		    printer << min::pustring
				   ( objf->obj_valsep )
			    << min::set_break;
		min::print_id ( printer, rvalue[k] );
	    }

	    if ( rvc > 1 )
		printer << min::pustring
			       ( objf->obj_valend )
			<< min::restore_indent;
	    printer << min::set_break
	            << min::pustring
			   ( objf->obj_valreq );
	    min::print_gen
		( printer, rinfo[j].name,
		           objf->label_format );
	}

	if ( vc + rc != 1 && ! line_format )
	    printer << min::pustring
			   ( objf->obj_valend )
		    << min::restore_indent;
    }

    if ( ! line_format )
        min::print_ustring
	    ( printer, objf->obj_braend );

    if ( ! first_attr )
    {
	if ( line_format )
	    printer << min::adjust_indent ( - adjust );
	else
	    printer << min::restore_indent;
    }
    return ! first_attr;
}

min::printer min::print_obj
	( min::printer printer,
	  min::gen v,
	  const min::obj_format * objf,
	  min::uns32 obj_op_flags,
	  min::unsptr max_attrs )
{
    min::obj_vec_ptr vp ( v );
    min::attr_ptr ap ( vp );

    min::attr_info info[max_attrs];
    min::unsptr m =
        min::get_attrs ( info, max_attrs, ap );
    if ( m > max_attrs )
        return min::print_obj ( printer, v, objf, m );

    min::gen separator = min::NONE();
    min::gen initiator = min::NONE();
    min::gen terminator = min::NONE();
    min::gen type = min::NONE();

    // Figure out if type is needed for normal format
    // or embedded line format.
    //
    bool need_type =
        (    ( obj_op_flags
	       &
	       ( min::PRINT_ID + min::ISOLATED_LINE ) )
	  == 0 );

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
        if ( info[i].name == min::dot_position )
	    continue;

	if (    info[i].value_count != 1
	     || info[i].flag_count != 0
	     || info[i].reverse_attr_count != 0 )
	    compact_format = false;
	else if ( ! min::is_str ( info[i].value )
	          &&
		  ! min::is_lab ( info[i].value ) )
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
	    if ( terminator == min::NONE() )
	        compact_format = false;
	    else if ( type != min::NONE() )
	    {
	        min::lab_ptr labp ( type );
		if (    min::lablen ( labp ) != 2
		     || labp[0] != initiator
		     || labp[1] != terminator )
		    compact_format = false;
	    }
	}
	else if ( terminator != min::NONE() )
	    compact_format = false;
    }

    if (    ! compact_format
         && ( obj_op_flags & min::PRINT_ID ) )
        return print_id ( printer, v );

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

    min::gen marking_end_type = min::NONE();
    if ( compact_format )
    {
        if ( initiator != min::NONE() )
	    min::print_gen
	        ( printer, initiator,
	          objf->initiator_format );
	else if ( min::size_of ( vp ) == 0
		  &&
		  type == NONE() )
	    return
	        printer << min::pustring
		               ( objf->obj_empty )
			<< min::restore_print_format;
	else
	{
	    min::print_ustring
	        ( printer, objf->obj_bra );

	    const min::print_format & pf =
	        printer->print_format;
	    min::gen marking_begin_type = min::NONE();
	    if ( min::is_str ( type ) )
	    {
	        if ( min::in_str_class
			 ( pf.char_flags,
			   pf.support_control, 
			   type,
			   objf->marking_type ) )
		{
		    marking_begin_type = type;
		    marking_end_type = type;
		}
	    }
	    else if ( min::is_lab ( type ) )
	    {
	        min::lab_ptr lab = type;
		if ( lablen ( lab ) == 2
		     &&
		     min::is_str ( lab[0] )
		     &&
		     min::in_str_class
			 ( pf.char_flags,
			   pf.support_control, 
			   lab[0],
			   objf->marking_type )
		     &&
		     min::is_str ( lab[1] )
		     &&
		     min::in_str_class
			 ( pf.char_flags,
			   pf.support_control, 
			   lab[1],
			   objf->marking_type ) )
		{
		    marking_begin_type = lab[0];
		    marking_end_type = lab[1];
		}
	    }
	        

	    if ( marking_begin_type != min::NONE() )
		min::print_gen
		    ( printer, marking_begin_type,
		      objf->label_format );
	    else
	    {
	        if ( type != min::NONE() )
		    min::print_gen
			( printer, type,
			  objf->label_format );
		min::print_ustring
		    ( printer, objf->obj_braend );
	    }
	}
    }
    else if ( embedded_line_format )
    {
	printer << min::indent;
	min::print_ustring
	    ( printer, objf->obj_bra );
	if ( type != min::NONE() )
	    min::print_gen
		( printer, type, objf->label_format );
	min::print_ustring
	    ( printer, objf->obj_braend );
    }
    else if ( ! isolated_line_format )
    {
	min::print_ustring
	    ( printer, objf->obj_bra );
	min::print_gen
	    ( printer, type != min::NONE() ?
		       type : min::empty_string,
		       objf->label_format );
	::print_attributes
	    ( printer, objf, vp, ap, info, m,
	      false, type );
    }

    bool first = true;
    for ( min::unsptr i = 0; i < min::size_of ( vp );
                             ++ i )
    {
        if ( first ) 
	{
	    first = false;
	    if ( ! isolated_line_format )
	    {
		min::print_spaces ( printer, 1 );
		if ( ! compact_format )
		    printer << min::set_break;
	    }
	    printer << min::save_indent;
	}
	else
	{
	    if (    ! compact_format
	         || separator == min::NONE() )
		min::print_ustring
		    ( printer, objf->obj_sep );
	    else
	    {
		min::print_gen
		    ( printer, separator,
	              objf->separator_format );
		min::print_spaces ( printer, 1 );
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

	bool attributes_printed = ::print_attributes
		( printer, objf, vp, ap, info, m,
		  true, type );
	if ( attributes_printed )
	    printer << min::restore_indent
	            << min::indent;
	else
	    min::print_spaces ( printer, 1 );
	min::print_ustring
	    ( printer, objf->obj_ketbegin );
	if (    (   obj_op_flags
		  & min::NO_TRAILING_TYPE )
	     == 0
	     &&
	     type != min::NONE() )
	    min::print_gen
		( printer, type, objf->label_format );
	min::print_ustring
	    ( printer, objf->obj_ket );
	if ( ! attributes_printed )
	    printer << min::restore_indent;
	printer << min::eol;
    }
    else if ( isolated_line_format )
    {
	if ( ! first ) printer << min::restore_indent;

	printer << min::adjust_indent ( 4 );
        ::print_attributes
	    ( printer, objf, vp, ap, info, m, true );
	printer << min::adjust_indent ( -4 );
	printer << min::eol;
    }
    else
    {
	min::print_spaces ( printer, 1 );

	if (    compact_format
	     && terminator != min::NONE() )
	    min::print_gen
	        ( printer, terminator,
		  objf->terminator_format );
	else if ( marking_end_type != min::NONE() )
	{
	    min::print_gen
		( printer, marking_end_type,
		  objf->label_format );
	    min::print_ustring
		( printer, objf->obj_ket );
	}
	else
	{
	    min::print_ustring
		( printer, objf->obj_ketbegin );
	    if (    (   obj_op_flags
	              & min::NO_TRAILING_TYPE )
		 == 0
		 &&
		 type != min::NONE() )
		min::print_gen
		    ( printer, type,
		      objf->label_format );
	    min::print_ustring
		( printer, objf->obj_ket );
	}

	if ( ! first ) printer << min::restore_indent;
    }

    return printer << min::restore_print_format;
}


#ifdef NONE_SUCH

    if ( ! indent )
	printer << "{| " << min::save_indent;
    else
        printer << min::save_line_break;
    printer << min::no_auto_break;

    for ( min::unsptr i = 0;
	  i < min::size_of ( vp ); ++ i )
    {
	if ( i != 0 )
	    printer << " " << min::set_break;

        min::gen v = vp[i];
	if ( min::is_sublist ( v ) )
	{
	    printer << "**";
	}
	else
	    pgen ( printer,
	           (*context_gen_flags)
		       [min::PGEN_ELEMENT],
	           vp[i],
	           context_gen_flags, f );
    }

    if ( indent && m > 0 ) printer << " |:";

    for ( min::unsptr i = 0; i < m; ++ i )
    {
	bool do_values = ( info[i].value_count > 0 );
	min::unsptr rc = info[i].reverse_attr_count;
	min::reverse_attr_info rinfo[rc];
	if ( rc > 0 )
	{
	    min::locate ( ap, info[i].name );
	    min::get_reverse_attrs ( rinfo, rc, ap );
	}

	for ( min::unsptr j = 0;
	      j < do_values + rc; ++ j )
	{
	    if ( indent )
		printer << min::indent;
	    else
		printer << "; " << min::set_break;

	    pgen ( printer,
	           (*context_gen_flags)[min::PGEN_NAME],
	           info[i].name,
	           context_gen_flags, f );

	    min::unsptr fc = info[i].flag_count;
	    if ( fc > 0 )
	    {
		min::packed_vec_ptr<const char *>
		    names = f->attr_flag_names;
		printer << "[";
		if ( fc * min::VSIZE <= 64 )
		{
		    min::uns64 f = info[i].flags;
		    for ( unsigned j = 0; j < 64; ++ j )
		    {
			if ( f & 1 )
			    printer << names[j];
			f >>= 1;
		    }
		}
		else
		{
		    min::gen flags[fc];
		    min::locate ( ap, info[i].name );
		    min::get_flags ( flags, fc, ap );
		    min::unsptr n = 0;
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
				if ( n < names->length )
				    printer << names[n];
				else
				    printer << "," << n;
			    }
			    flags2 >>= 1;
			    ++ n;
			}
		    }
		}
		printer << "]";
	    }

	    min::unsptr vc = info[i].value_count;
	    if ( j == 0 && vc == 1 )
	    {
		printer << " = ";
		pgen ( printer,
		       (*context_gen_flags)
		           [min::PGEN_VALUE],
		       info[i].value,
		       context_gen_flags, f );
	    }
	    else if ( j == 0 && vc > 1 )
	    {
		printer << " = "
		    << "{: " << min::save_indent;
		min::gen value[vc];
		min::locate ( ap, info[i].name );
		min::get ( value, vc, ap );
		for ( min::unsptr k = 0; k < vc; ++ k )
		{
		    if ( k > 0 )
			printer << "; "
			        << min::set_break;
		    pgen ( printer,
		           (*context_gen_flags)
			       [min::PGEN_VALUE],
		           value[k],
		           context_gen_flags, f );
		}
		printer << " :}" << min::restore_indent;
	    }
	    else
	    {
		min::unsptr rvc =
		    rinfo[j-do_values].value_count;
		if ( rvc == 1 )
		{
		    printer << " = " << min::set_break;
		    ::pgen_id
		        ( printer,
		          rinfo[j-do_values].value );
		}
		else if ( rvc > 1 )
		{
		    printer << " = " << min::set_break
			<< "{: " << min::save_indent;
		    min::gen v[rvc];
		    min::locate_reverse
			( ap, rinfo[j-do_values].name );
		    min::get ( v, rvc, ap );
		    for ( min::unsptr k = 0;
			  k < rvc; ++ k )
		    {
			if ( k > 0 )
			    printer << "; "
				<< min::set_break;
			::pgen_id ( printer, v[k] );
		    }
		    printer << " :}"
		            << min::restore_indent;
		}
		printer << min::set_break << " = ";
		pgen ( printer,
		       (*context_gen_flags)
		           [min::PGEN_NAME],
		       rinfo[j-do_values].name,
		       context_gen_flags, f );
	    }
	}
    }

    if ( ! indent ) printer << " |}";
    
    return printer << min::restore_indent;
}

inline bool find
        ( min::gen name,
	  min::packed_vec_ptr<min::gen> v )
{
    for ( min::unsptr i = 0; i < v->length; ++ i )
    {
        if ( name == v[i] ) return true;
    }
    return false;
}

inline min::gen get
        ( min::gen name,
	  const min::attr_info * info,
	  min::unsptr info_length )
{
    while ( info_length -- )
    {
        if ( name == info->name )
	{
	    if ( info->value_count != 1 )
	        return min::NONE();
	    else return info->value;
	}
	++ info;
    }
    return min::NONE();
}

// Execute pgen (below) in the case an object is to be
// printed with the OBJ_EXP_FLAG.
//
static min::printer pgen_exp
	( min::printer printer,
	  min::uns32 gen_flags,
	  min::attr_ptr & ap,
	  const min::context_gen_flags *
	      context_gen_flags,
	  const min::gen_format * f,
	  min::printer ( * pgen )
	      ( min::printer printer,
	        min::uns32 gen_flags,
	        min::gen v,
	        const min::context_gen_flags *
		    context_gen_flags,
		const min::gen_format * f ),
	  min::attr_info * info,
	  min::unsptr info_length )
{
    min::obj_vec_ptr vp = min::obj_vec_ptr_of ( ap );

    min::gen initiator =
         ::get ( min::dot_initiator,
	          info, info_length );
    min::gen separator =
         ::get ( min::dot_separator,
	          info, info_length );
    min::gen terminator =
         ::get ( min::dot_terminator,
	          info, info_length );
    min::gen name =
         ::get ( min::dot_name,
	          info, info_length );

    bool use_suppressible_space =
	( gen_flags & min::SUPPRESS_EXP_SPACE_FLAG );

    if ( initiator == min::NONE()
         &&
	 terminator == min::NONE() )
    {
    	// Unbracketed Expression

	const char * prefix = "";
	const char * postfix = "";
        if ( ( gen_flags & min::BRACKET_IMPLICIT_FLAG )
	     &&
	     f->implicit_prefix != NULL
	     &&
	     f->implicit_postfix != NULL )
	{
	    prefix = f->implicit_prefix;
	    postfix = f->implicit_postfix;
	}
	printer << prefix
	        << min::save_indent
                << min::no_auto_break;
	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    if ( i != 0 )
	    {
	        if ( separator != min::NONE() )
		{
		    pgen ( printer,
		           (*context_gen_flags)
			       [min::PGEN_PUNCTUATION],
			   separator,
			   context_gen_flags, f );
		    printer << " ";
		}
		else if ( use_suppressible_space )
		    printer << min::suppressible_space;
		else
		    printer << " ";
		printer << min::set_break;
	    }
	    pgen ( printer,
	           (*context_gen_flags)
		       [min::PGEN_ELEMENT],
	           vp[i],
	           context_gen_flags, f );
	}
	if ( * postfix )
	    printer << min::spaces_if_before_indent
	            << postfix;
	return printer << min::restore_indent;
    }
    else if ( terminator == min::NONE() )
    {
        // Indentation Mark Expression

        if ( initiator == min::doublequote
	     &&
	     min::size_of ( vp ) == 1 )
	{
	    pgen ( printer,
	           gen_flags | min::BRACKET_STR_FLAG,
	           vp[0],
	           context_gen_flags, f );
	    return printer;
	}
        else if ( initiator == min::number_sign
	          &&
	          min::size_of ( vp ) == 1 )
	{
	    pgen ( printer,
	           gen_flags,
	           vp[0],
	           context_gen_flags, f );
	    return printer;
	}

	pgen ( printer,
	       (*context_gen_flags)
	           [min::PGEN_PUNCTUATION],
	       initiator,
	       context_gen_flags, f );

	printer << min::adjust_indent ( 4 )
	        << min::eol;
	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    printer << min::spaces_if_before_indent;
	    if ( i != 0 && separator != min::NONE() )
	    {
		pgen ( printer,
		       (*context_gen_flags)
			   [min::PGEN_PUNCTUATION],
		       separator,
		       context_gen_flags, f );
		printer << " ";
	    }
	    printer << min::set_break;
	    pgen ( printer,
	           (*context_gen_flags)
		       [min::PGEN_ELEMENT],
	           vp[i],
	           context_gen_flags, f );
	}
	return printer << min::adjust_indent ( -4 )
	               << min::eol_if_after_indent;
    }
    else if ( initiator == min::NONE() )
    {
        // Line In Indented Paragraph

	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    if ( i == 0 )
		printer << min::spaces_if_before_indent;
	    else
	    {
	        if ( separator != min::NONE() )
		{
		    pgen ( printer,
		           (*context_gen_flags)
			       [min::PGEN_PUNCTUATION],
			   separator,
			   context_gen_flags, f );
		    printer << " ";
		}
		else if ( use_suppressible_space )
		    printer << min::suppressible_space;
		else
		    printer << " ";
	    }
	    printer << min::set_break;
	    pgen ( printer,
	           (*context_gen_flags)
		       [min::PGEN_ELEMENT],
	           vp[i],
	           context_gen_flags, f );
	}
	if ( terminator == min::new_line )
		return printer << min::eol;
	else
	{
	    printer << min::spaces_if_before_indent
	            << min::suppressible_space;
	    pgen ( printer,
	           (*context_gen_flags)
		       [min::PGEN_PUNCTUATION],
	           terminator,
	           context_gen_flags, f );
	    return printer << " ";
	}
    }
    else if ( name == min::NONE() )
    {
        // Bracketed Expression.

	pgen ( printer,
	       (*context_gen_flags)
	           [min::PGEN_PUNCTUATION],
	       initiator,
	       context_gen_flags, f );
	printer << min::suppressible_space
	        << min::save_indent
                << min::no_auto_break;
	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    if ( i != 0 )
	    {
	        if ( separator != min::NONE() )
		{
		    pgen ( printer,
		           (*context_gen_flags)
			       [min::PGEN_PUNCTUATION],
			   separator,
			   context_gen_flags, f );
		    printer << " ";
		}
		else if ( use_suppressible_space )
		    printer << min::suppressible_space;
		else
		    printer << " ";
		printer << min::set_break;
	    }
	    pgen ( printer, 
		   (*context_gen_flags)
		       [min::PGEN_ELEMENT],
	           vp[i],
		   context_gen_flags, f );
	}

	printer << min::spaces_if_before_indent
	        << min::suppressible_space;
	pgen ( printer,
	       (*context_gen_flags)
	           [min::PGEN_PUNCTUATION],
	       terminator,
	       context_gen_flags, f );
	return printer << min::restore_indent;
    }
    else
    {
	min::gen middle =
	     ::get ( min::dot_middle,
		      info, info_length );
	min::gen arguments =
	     ::get ( min::dot_arguments,
		      info, info_length );
	min::gen keys =
	     ::get ( min::dot_keys,
		      info, info_length );

	printer << min::save_print_format
                << min::no_auto_break;
	pgen ( printer,
	       (*context_gen_flags)
	           [min::PGEN_PUNCTUATION],
	       initiator,
	       context_gen_flags, f );
	printer << min::suppressible_space
	        << min::save_indent;
	pgen ( printer,
	       (*context_gen_flags)[min::PGEN_NAME],
	       name,
	       context_gen_flags, f );
	if ( arguments != min::NONE()
	     ||
	     keys != min::NONE() )
	{
	    if ( arguments != min::NONE() )
	    {
		min::obj_vec_ptr argp ( arguments );
		for ( min::unsptr i = 0;
		      i < min::size_of ( argp ); ++ i )
		{
		    printer << " " << min::set_break;
		    min::uns32 gen_flags =
			(*context_gen_flags)
			    [min::PGEN_TOP];
		    gen_flags &= ~ min::OBJ_ID_FLAG;
		    gen_flags |= min::OBJ_EXP_FLAG;
		    pgen ( printer, gen_flags,
			   argp[i],
			   context_gen_flags, f );
		}
	    }
	    if ( keys != min::NONE() )
	    {
		min::obj_vec_ptr keyp ( keys );
		for ( min::unsptr i = 0;
		      i < min::size_of ( keyp ); ++ i )
		{
		    printer << " " << min::set_break
		            << "# ";
		    pgen ( printer,
			   (*context_gen_flags)
			       [min::PGEN_NAME],
			   keyp[i],
			   context_gen_flags, f );
		}
	    }

	    if ( middle == min::NONE() )
	    {
		printer << min::suppressible_space;
		pgen ( printer,
		       (*context_gen_flags)
		           [min::PGEN_PUNCTUATION],
		       terminator,
		       context_gen_flags, f );
		return printer
		           << min::restore_indent
			   << min::restore_print_format;
	    }
	    else
	    {
		printer << min::suppressible_space;
		pgen ( printer,
		       (*context_gen_flags)
		           [min::PGEN_PUNCTUATION],
		       middle,
		       context_gen_flags, f );
		printer << min::restore_indent;
	    }
	}
	else if ( middle == min::NONE() )
	{
	    printer << min::suppressible_space;
	    pgen ( printer,
		   (*context_gen_flags)
		       [min::PGEN_PUNCTUATION],
		   terminator,
		   context_gen_flags, f );
	    return printer << min::restore_indent
		           << min::restore_print_format;
	}
	else
	{
	    printer << min::suppressible_space;
	    pgen ( printer,
		   (*context_gen_flags)
		       [min::PGEN_PUNCTUATION],
		   middle,
		   context_gen_flags, f );
	    printer << min::restore_indent;
	}

	printer << " " << min::save_indent
	               << min::set_break;

	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    if ( i != 0 )
	    {
	        if ( separator != min::NONE() )
		{
		    pgen ( printer,
		           (*context_gen_flags)
			       [min::PGEN_PUNCTUATION],
			   separator,
			   context_gen_flags, f );
		    printer << " ";
		}
		else if ( use_suppressible_space )
		    printer << min::suppressible_space;
		else
		    printer << " ";
		printer << min::set_break;
	    }

	    pgen ( printer,
		   (*context_gen_flags)
		       [min::PGEN_ELEMENT],
	           vp[i],
		   context_gen_flags, f );
	}

	printer << " ";
	pgen ( printer,
	       (*context_gen_flags)
	           [min::PGEN_PUNCTUATION],
	       middle,
	       context_gen_flags, f );
	pgen ( printer,
	       (*context_gen_flags)
	           [min::PGEN_PUNCTUATION],
	       terminator,
	       context_gen_flags, f );
	return printer << min::restore_indent
		   << min::restore_print_format;
    }
}

#endif // NONE_SUCH

// Execute min::pgen.
//
min::printer min::standard_pgen
	( min::printer printer,
	  min::gen v,
	  const min::gen_format * f )
{
    if ( v == min::new_stub_gen ( MINT::null_stub ) )
    {
        // This must be first as MINT::null_stub
	// may not exist and therefore cannot have its
	// type tested.
	//
        return
	    printer
	        << "new_stub_gen ( MINT::null_stub )";
    }
    else if ( min::is_num ( v ) )
    {
        min::float64 value = MUP::float_of ( v );
	const min::num_format * nf = f->num_format;
	return min::print_num ( printer, value, nf );
    }
    else if ( min::is_str ( v ) )
    {
	return min::print_str
	    ( printer, v, f->str_format );
    }
    else if ( min::is_lab ( v ) )
    {
        const min::lab_format * lf = f->lab_format;
	if ( lf == NULL ) lf = min::name_lab_format;

	MUP::lab_ptr labp ( MUP::stub_of ( v ) );
        min::uns32 len = min::lablen ( labp );

	if ( lf ) min::print_ustring
	              ( printer, lf->lab_prefix );

	for ( min::unsptr i = 0; i < len; ++ i )
	{
	    if ( i != 0 )
		min::print_ustring
		    ( printer, lf->lab_separator );
	    (* f->pgen) ( printer, labp[i], f );
	}

	if ( lf ) min::print_ustring
	              ( printer, lf->lab_postfix );

	return printer;
    }
    else if ( min::is_special ( v ) )
    {
	const min::special_format * sf =
	    f->special_format;
        min::unsgen index = MUP::special_index_of ( v );
	min::packed_vec_ptr<const char *>
	        special_names =
	    sf ? sf->special_names
	       : (min::packed_vec_ptr<const char *>)
	         min::NULL_STUB;

	if ( sf ) min::print_ustring
		      ( printer, sf->special_prefix );
	if ( special_names != min::NULL_STUB
	     &&
	         0xFFFFFF
	       - special_names->length
	     < index
	     &&
	     index <= 0xFFFFFF )
	{
	    min::print_chars
	        ( printer,
		  special_names[0xFFFFFF - index] );
	}
	else
	{
	    char buffer[64];
	    sprintf ( buffer, "SPECIAL(0x%06llX)",
		              (min::uns64) index );
	    min::print_chars ( printer, buffer );
	}
	if ( sf ) min::print_ustring
		      ( printer, sf->special_postfix );
	return printer;
    }

    else if ( min::is_obj ( v ) )
        return min::print_obj
	    ( printer, v, f->obj_format );

# ifdef NONE_SUCH

    else if ( min::is_obj ( v ) )
    {
        if ( gen_flags & min::OBJ_EXP_FLAG )
	{
	    min::obj_vec_ptr vp ( v );
	    min::attr_ptr ap ( vp );
	    min::unsptr length =
	        f->exp_ok_attrs->length;
	    min::attr_info info[length];
	    min::unsptr c =
	        min::get_attrs ( info, length, ap );

	    bool ok = ( c <= length );
	    for ( min::unsptr i = 0; ok && i < c; ++ i )
	        ok = ::find ( info[i].name,
		              f->exp_ok_attrs )
		     &&
		     info[i].reverse_attr_count == 0;
	    if (    c == 0
	         && min::size_of ( vp ) == 0
		 &&    (   gen_flags
		         & min::BRACKET_IMPLICIT_FLAG )
		    == 0 )
	        ok = false;

	    if ( ok )
	    {
		::pgen_exp
		    ( printer, gen_flags, ap,
		      context_gen_flags, f,
		      pgen, &info[0], c );
		return printer;
	    }
	}

        if ( gen_flags & min::OBJ_ID_FLAG )
	    ::pgen_id ( printer, v );
	else
	    ::pgen_obj
	        ( printer, gen_flags, v,
		  context_gen_flags, f,
		  pgen );

	return printer;
    }

# endif // NONE_SUCH

    else if ( min::is_stub ( v ) )
    {
        const min::stub * s = MUP::stub_of ( v );
	int type = min::type_of ( s );
	const char * type_name = min::type_name[type];
	if ( type_name != NULL )
	    printer << type_name;
	else
	    printer << "TYPE(" << type << ")";
	return printer;
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

	if ( sf ) min::print_ustring
		      ( printer, sf->special_prefix );
	min::print_chars ( printer, buffer );
	if ( sf ) min::print_ustring
		      ( printer, sf->special_postfix );
	return printer;
    }
}

static min::printer flush_one_id
	( min::printer printer,
	  min::id_map id_map,
	  const min::gen_format * f )
{
    if ( id_map == min::NULL_STUB
         ||
	 id_map->next >= id_map->length )
        return printer;

    const min::gen_format * id_map_f =
        f->id_map_format;

    min::uns32 id = id_map->next;
    * ( min::uns32 * ) & id_map->next = id + 1;
    min::gen v = min::new_stub_gen ( id_map[id] );

    printer << min::save_indent << "@" << id << " = ";

    (* id_map_f->pgen) ( printer, v, id_map_f );
    return printer << min::eol << min::restore_indent;
}

static min::printer flush_one_id
	( min::printer printer )
{
    const min::gen_format * f =
	printer->print_format.gen_format;

    min::id_map id_map = printer->id_map;

    return ::flush_one_id ( printer, id_map, f );
}
