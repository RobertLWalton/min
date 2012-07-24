// MIN Language Out-of-Line Code
//
// File:	min.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jul 24 03:35:08 EDT 2012
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
# include <cerrno>
# include <cctype>
# define MUP min::unprotected
# define MINT min::internal

# define ERR min::init ( min::error_message ) \
    << min::set_indent ( 7 ) << "ERROR: "

// For debugging.
//
# include <iostream>
# include <iomanip>
# include <cmath>

// Initialization
// --------------

min::initializer * MINT::last_initializer = NULL;
bool MINT::initialization_done = false;

# include "../unicode/unicode_class.h"

min::uns8 MINT::unicode_class[UNICODE_CLASS_SIZE+1] =
    UNICODE_CLASS;
min::uns32 MINT::unicode_class_size =
    UNICODE_CLASS_SIZE;

static char const * type_name_vector[256];
char const ** min::type_name = type_name_vector + 128;

min::locatable_gen min::dot_initiator;
min::locatable_gen min::dot_separator;
min::locatable_gen min::dot_terminator;
min::locatable_gen min::dot_middle;
min::locatable_gen min::dot_name;
min::locatable_gen min::dot_arguments;
min::locatable_gen min::dot_keys;
min::locatable_gen min::dot_operator;
min::locatable_gen min::dot_position;
min::locatable_gen min::new_line;
min::locatable_gen min::doublequote;
min::locatable_gen min::number_sign;

static min::packed_vec<const char *>
    const_char_ptr_packed_vec_type
        ( "min.cc::const_char_ptr_packed_vec_type" );

static min::uns32 gen_element_disp[2] =
    { 0, min::DISP_END };

static min::packed_vec<min::gen> gen_packed_vec_type
    ( "min.cc::gen_packed_vec_type",
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

min::packed_vec_ptr<min::gen>
    min::default_exp_ok_attrs;
static min::locatable_var
	< min::packed_vec_ptr<min::gen> >
    default_exp_ok_attrs;

static const unsigned default_flag_names_length = 256;
static char const * default_flag_names_value
		  [::default_flag_names_length] =
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
    min::default_flag_names;
static min::locatable_var
	< min::packed_vec_ptr<const char *> >
    default_flag_names;

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

static void init_default_suppress_matrix ( void );
void MINT::initialize ( void )
{
    MINT::initialization_done = true;

    for ( min::uns32 i = 0; i < 256; ++ i )
    {
        if ( MINT::unicode_class[i] == 'L' )
	    continue;
        if ( MINT::unicode_class[i] == 'U' )
	    continue;
	MINT::unicode_class[i] = i;
    }

    // Unicode class must be initialized before default
    // suppress matrix is initialized.

    init_default_suppress_matrix();

    PTR_CHECK ( min::packed_struct<int>::ptr );
    PTR_CHECK ( min::packed_struct<int>::updptr );
    PTR_CHECK ( min::packed_vec<int,int>::ptr );
    PTR_CHECK ( min::packed_vec<int,int>::updptr );
    PTR_CHECK ( min::packed_vec<int,int>::insptr );

    assert
        ( OFFSETOF ( & MINT::locatable_gen::previous )
	  == sizeof ( const min::stub * ) );
    assert
        ( OFFSETOF ( & MINT::locatable_var::previous )
	  == sizeof ( const min::stub * ) );

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

    min::dot_initiator =
        min::new_lab_gen ( ".", "initiator" );
    min::dot_separator =
        min::new_lab_gen ( ".", "separator" );
    min::dot_terminator =
        min::new_lab_gen ( ".", "terminator" );
    min::dot_middle =
        min::new_lab_gen ( ".", "middle" );
    min::dot_name =
        min::new_lab_gen ( ".", "name" );
    min::dot_arguments =
        min::new_lab_gen ( ".", "arguments" );
    min::dot_keys =
        min::new_lab_gen ( ".", "keys" );
    min::dot_operator =
        min::new_lab_gen ( ".", "operator" );
    min::dot_position =
        min::new_lab_gen ( ".", "position" );
    min::new_line =
        min::new_str_gen ( "\n" );
    min::doublequote =
        min::new_str_gen ( "\"" );
    min::number_sign =
        min::new_str_gen ( "#" );

    {
	min::packed_vec_insptr<const char *> p =
	    ::const_char_ptr_packed_vec_type.new_stub
		( ::standard_special_names_length );
	::standard_special_names = p;
	min::standard_special_names = p;
	min::push ( p, ::standard_special_names_length,
		       ::standard_special_names_value );
    }

    {
	min::packed_vec_insptr<min::gen> p =
	    ::gen_packed_vec_type.new_stub ( 9 );
	::default_exp_ok_attrs = p;
	min::default_exp_ok_attrs = p;
	* (const min::stub **)
	& min::default_gen_format.exp_ok_attrs = p;

	min::push ( p ) = min::dot_initiator;
	min::push ( p ) = min::dot_separator;
	min::push ( p ) = min::dot_terminator;
	min::push ( p ) = min::dot_middle;
	min::push ( p ) = min::dot_name;
	min::push ( p ) = min::dot_arguments;
	min::push ( p ) = min::dot_keys;
	min::push ( p ) = min::dot_operator;
	min::push ( p ) = min::dot_position;
    }

    {
	min::packed_vec_insptr<const char *> p =
	    ::const_char_ptr_packed_vec_type.new_stub
		( ::default_flag_names_length );
	::default_flag_names = p;
	min::default_flag_names = p;
	* (const min::stub **)
	& min::default_gen_format.flag_names = p;

	min::push ( p, ::default_flag_names_length,
		       ::default_flag_names_value );
    }

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
	unsptr len1 = length_of ( lp1 );
	unsptr len2 = length_of ( lp2 );
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

// Process Management
// ------- ----------

bool MINT::thread_interrupt_needed = false;
void MINT::thread_interrupt ( void ) {}  // TBD

// Allocator/Collector/Compactor 
// -----------------------------

static min::stub ZERO_STUB;
const min::stub * min::ZERO_STUB = & ::ZERO_STUB;

MINT::locatable_gen * MINT::locatable_gen_last;
MINT::locatable_var * MINT::locatable_var_last;

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
    while ( i < min::length_of ( labp ) )
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
	    MINT::locatable_var_last;
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
	min::gen v = sc.locatable_gen_last->value;
	if ( min::is_stub ( v ) )
	{
	    min::stub * s2 = MUP::stub_of ( v );
	    MIN_SCAVENGE_S2
	        ( sc.thread_state = 1; return );
	}
	++ sc.gen_count;
        sc.locatable_gen_last =
	    sc.locatable_gen_last->previous;
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
	    (min::stub *) sc.locatable_var_last->value;
	if ( s2 != NULL )
	{
	    MIN_SCAVENGE_S2
	        ( sc.thread_state = 1; return );
	}
	++ sc.gen_count;
	sc.locatable_var_last =
	    sc.locatable_var_last->previous;
    }

    sc.stub_flag_accumulator = accumulator;
    sc.thread_state = 0;
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
    uns32 hash = strnhash ( p, n );
    uns32 h = hash & MINT::str_hash_mask;
    const char * q;

    min::stub * s = MINT::str_acc_hash[h];
    while ( s != MINT::null_stub )
    {
	uns64 c = MUP::control_of ( s );

        if (    n <= 8
	     && type_of ( s ) == SHORT_STR
	     && ::strncmp ( p, s->v.c8, n ) == 0
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
		       ( p, q = MUP::str_of (
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
	     && ::strncmp ( p, s2->v.c8, n ) == 0
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
		       ( p, q = MUP::str_of (
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
	::strncpy ( s2->v.c8, p, n );
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

	::strncpy ( (char *) MUP::str_of ( ls ), p, n );
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
	( const min::uns32 * p, min::unsptr n )
{
    char buffer[8*n+1];
    char * q = buffer;
    unsptr m = 0;
    while ( n -- )
        m += min::unicode_to_utf8 ( q, * p ++ );
    return internal::new_str_gen ( buffer, m );
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
    uns32 hash = labhash ( p, n );
    uns32 h = hash & MINT::lab_hash_mask;

    // Search for existing label stub with given
    // elements.
    //
    min::stub * s = MINT::lab_acc_hash[h];
    while ( s != MINT::null_stub )
    {
	uns64 c = MUP::control_of ( s );

	lab_ptr labp ( s );

	if ( hash != hash_of ( labp ) ) continue;
	if ( n != length_of ( labp ) ) continue;

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

	if ( hash != hash_of ( labp ) ) continue;
	if ( n != length_of ( labp ) ) continue;

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
    memcpy ( lh + 1, p, n * sizeof ( min::gen ) );

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
    min::locatable_gen g1 ( min::new_str_gen ( s1 ) );
    min::locatable_gen g2 ( min::new_str_gen ( s2 ) );
    min::gen elements[2] = { g1, g2 };
    return min::new_lab_gen ( elements, 2 );
}

min::gen min::new_lab_gen
	( const char * s1,
	  const char * s2,
	  const char * s3 )
{
    min::locatable_gen g1 ( min::new_str_gen ( s1 ) );
    min::locatable_gen g2 ( min::new_str_gen ( s2 ) );
    min::locatable_gen g3 ( min::new_str_gen ( s3 ) );
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
    else
    {
	if ( file->line_index != NULL_STUB )
	{
	    min::pop ( file->line_index,
	               file->line_index->length );
	    min::resize ( file->line_index,
			  file_line_index_type
			      .initial_max_length );
	}
    }
    file->next_line_number = 0;
    file->next_offset = 0;
}

void min::init_print_flags
	( min::ref<min::file> file,
	  min::uns32 print_flags )
{
    init ( file );
    file->print_flags = print_flags;
}

inline void set_spool_lines
	( min::file file,
	  min::uns32 spool_lines )
{
    file->spool_lines = spool_lines;
    if ( spool_lines != 0 )
    {
        if ( file->line_index == min::NULL_STUB )
	    min::line_index_ref(file) =
		::file_line_index_type.new_stub();
    }
    else
    {
        if ( file->line_index != min::NULL_STUB )
	    min::line_index_ref(file) =
		min::NULL_STUB;
    }
}

void min::init_spool_lines
	( min::ref<min::file> file,
	  min::uns32 spool_lines )
{
    init ( file );
    ::set_spool_lines ( file, spool_lines );
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
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( file );
    min::pop ( file->buffer,
	       file->buffer->length );
    min::resize ( file->buffer,
	          file_buffer_type.initial_max_length );
    file->end_offset = 0;
    file->file_lines = min::NO_LINE;
    file->print_flags = print_flags;
    ::set_spool_lines ( file, spool_lines );
    file->istream = NULL;
    ifile_ref(file) = NULL_STUB;
    file_name_ref(file) = MISSING();
}

void min::init_input_stream
	( min::ref<min::file> file,
	  std::istream & istream,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init_input ( file );
    file->istream = & istream;
    ifile_ref(file) = NULL_STUB;
    file->print_flags = print_flags;
    ::set_spool_lines ( file, spool_lines );
}

void min::init_input_file
	( min::ref<min::file> file,
	  min::file ifile,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init_input ( file );
    file->istream = NULL;
    ifile_ref(file) = ifile;
    file->print_flags = print_flags;
    ::set_spool_lines ( file, spool_lines );
}

bool min::init_input_named_file
	( min::ref<min::file> file,
	  min::gen file_name,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init_input ( file );
    file->istream = NULL;
    ifile_ref(file) = NULL_STUB;
    file_name_ref(file) = file_name;
    file->print_flags = print_flags;
    ::set_spool_lines ( file, spool_lines );

    min::str_ptr fname ( file_name );

    // Use OS independent min::os::file_size.
    //
    char error_buffer[512];
    uns64 file_size;
    if ( ! min::os::file_size
               ( file_size, min::begin_ptr_of ( fname ),
	         error_buffer ) )
    {
	ERR << "During attempt to find the size of"
	       " file "
	    << fname << ": "
	    << min::reserve ( 20 )
	    << error_buffer << min::eol;
        return false;
    }

    if ( file_size >= ( 1ull << 32 ) - 1 )
    {
        ERR << "File "
	    << fname << ": "
	    << min::reserve ( 20 )
	    << "File too large ( size = " << file_size
	    << " bytes)" << min::eol;
	return false;
    }

    // We use FILE IO because it is standard for C
    // while open/read is OS dependent.

    FILE * in =
        fopen ( min::begin_ptr_of ( fname ), "r" );

    if ( in == NULL )
    {
        ERR << "Opening file "
	    << fname << ": "
	    << min::reserve ( 20 )
	    << strerror ( errno )
	    << min::eol;
	return false;
    }

    min::resize
	( file->buffer, (min::uns32) file_size + 1 );
    min::push ( file->buffer, file_size );

    errno = 0;
    uns64 bytes =
        fread ( & file->buffer[0], 1,
	        (size_t) file_size, in );
    if ( bytes != file_size )
    {
	if ( errno != 0 )
	    ERR << "Reading file "
		<< fname << ": "
	        << min::reserve ( 20 )
		<< strerror ( errno )
		<< min::eol;
	else
	    ERR << "Reading file "
		<< fname << ": "
	        << min::reserve ( 20 )
		<< " Only " << bytes
		<< " bytes out of " << file_size
		<< " read"
		<< min::eol;
	fclose ( in );
	return false;
    }

    if ( getc ( in ) != EOF )
    {
	ERR << "Reading file "
	    << fname << ": "
	    << min::reserve ( 20 )
	    << "File longer than expected (more than "
	    << file_size << " bytes were read)"
	    << min::eol;
	fclose ( in );
	return false;
    }

    fclose ( in );

    file->file_lines = 0;
    for ( uns32 i = 0; i < file->buffer->length; ++ i )
    {
	char c = file->buffer[i];
        if ( c == '\n' || c == 0 )
	{
	    min::end_line ( file, i );
	    ++ file->file_lines;
	}
    }

    return true;
}

void min::init_input_string
	( min::ref<min::file> file,
	  min::ptr<const char> data,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init_input ( file );
    file->istream = NULL;
    ifile_ref(file) = NULL_STUB;
    file->print_flags = print_flags;
    ::set_spool_lines ( file, spool_lines );

    uns64 length = ::strlen ( data );
    assert ( length < ( 1ull << 32 ) );

    min::resize ( file->buffer, length );
    min::push ( file->buffer, length, data );

    file->file_lines = 0;
    for ( uns32 i = 0; i < length; ++ i )
    {
        if ( file->buffer[i] == '\n' )
	{
	    min::end_line ( file, i );
	    ++ file->file_lines;
	}
    }
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
	        file->file_lines =
		    file->next_line_number;
	        return min::NO_LINE;
	    }
	}
	else if ( file->ifile != NULL_STUB )
	{
	    min::file ifile = file->ifile;
	    uns32 ioffset = min::next_line ( ifile );

	    if ( ioffset == min::NO_LINE )
	    {
		ioffset = ifile->next_offset;
		uns32 length = ifile->buffer->length
			     - ioffset;
	        if ( length > 0 )
		{
		    min::push
			( file->buffer, length,
			  & ifile->buffer[ioffset] );
		    ifile->next_offset =
		        ifile->buffer->length;
		}

	        file->file_lines =
		    file->next_line_number;
		return min::NO_LINE;
	    }

	    uns32 length =
		::strlen ( & ifile->buffer[ioffset] );
	    min::push ( file->buffer, length,
	                & ifile->buffer[ioffset] );
	}
	else
	{
	    file->file_lines = file->next_line_number;
	    return min::NO_LINE;
	}

	min::end_line ( file );
    }

    file->next_offset +=
        1 + ::strlen ( & file->buffer[line_offset] );
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
	  min::uns32 print_flags,
	  min::file file,
	  min::uns32 line_number,
	  const char * blank_line,
	  const char * end_of_file,
	  const char * unavailable_line )
{
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
        length = ::strlen ( & file->buffer[offset] );

    // Move line to stack so that (1) it will not be
    // relocatable when printer is called, and (2) it
    // will end with NUL even if it is a partial line.
    //
    char buffer[length+1];
    memcpy ( buffer, & file->buffer[offset], length );
    buffer[length] = 0;

    // Blank line check.
    //
    if ( blank_line == NULL )
        ; // do nothing
    else if ( ( print_flags & min::GRAPHIC_FLAGS )
              &&
	      buffer[0] != 0 )
        ; // do nothing
    else if ( eof ? end_of_file != NULL :
                  (   print_flags
                    & min::DISPLAY_EOL_FLAG ) )
        ; // do nothing
    else
    {
	const char * p = & buffer[0];
	while ( * p && ( * p <= ' ' || * p == 0x3F ) )
	    ++ p;
	if ( * p == 0 )
	{
	    printer << blank_line << min::eol;
	    return 0;
	}
    }

    printer << min::save_print_format
            << min::clear_print_flags
	            (   min::HBREAK_FLAG
		      + min::GBREAK_FLAG
		      + min::GRAPHIC_HSPACE_FLAG
		      + min::GRAPHIC_VSPACE_FLAG
		      + min::GRAPHIC_NSPACE_FLAG
		      + min::ALLOW_HSPACE_FLAG
		      + min::ALLOW_VSPACE_FLAG
		      + min::ALLOW_NSPACE_FLAG
		      + min::ASCII_FLAG
		      + min::DISPLAY_EOL_FLAG )
	    << min::set_print_flags
	    	    (   (   min::GRAPHIC_HSPACE_FLAG
			  + min::GRAPHIC_VSPACE_FLAG
			  + min::GRAPHIC_NSPACE_FLAG
			  + min::ALLOW_HSPACE_FLAG
			  + min::ALLOW_VSPACE_FLAG
			  + min::ALLOW_NSPACE_FLAG
		          + min::ASCII_FLAG
		          + min::DISPLAY_EOL_FLAG )
		      & print_flags )
	    << buffer;
    uns32 width = printer->column;
    if ( eof )
    {
        if ( end_of_file ) printer << end_of_file;
	printer << min::restore_print_format
	        << min::eol;
    }
    else
    {
	printer << min::eol
	        << min::restore_print_format;
	if ( print_flags & min::DISPLAY_EOL_FLAG )
	    width +=
	        ( print_flags & min::ASCII_FLAG ?
		  4 : 1 );
    }
    return width;
}

min::uns32 min::print_line_column
	( min::uns32 print_flags,
	  min::file file,
	  const min::position & position )
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
        length = ::strlen ( & file->buffer[offset] );

    min::pwidth ( column, & file->buffer[offset],
    		  position.offset <= length ?
		      position.offset : length,
                  print_flags );
    return column;
}

void min::print_phrase_lines
	( min::printer printer,
	  min::uns32 print_flags,
	  min::file file,
	  const min::phrase_position & position,
	  char mark,
	  const char * blank_line,
	  const char * end_of_file,
	  const char * unavailable_line )
{
    assert ( position.end.line >= position.begin.line );

    // Temporary recomputation of columns to see if
    // pwidth works.
    //
    min::position begin = position.begin;
    min::position end   = position.end;
    uns32 begin_column =
        print_line_column ( print_flags, file, begin );
    uns32 end_column =
        print_line_column ( print_flags, file, end );

    uns32 line = begin.line;
    uns32 first_column = begin_column;

    uns32 width = min::print_line
	( printer, print_flags, file, line,
	  blank_line, end_of_file, unavailable_line );

    while ( true )
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

	if ( line == end.line ) return;

	++ line;

	if ( line == end.line && end_column == 0 )
	    return;

	first_column = 0;
	width = min::print_line
	    ( printer, print_flags, file, line,
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
            << min::nohbreak;

    if ( pline_numbers.first == pline_numbers.last )
        printer << "line " << pline_numbers.first + 1;
    else
        printer << "lines " << pline_numbers.first + 1
	        << "-" << pline_numbers.last + 1;

    return printer << min::restore_print_format;
}

void min::flush_file ( min::file file )
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
}

void min::flush_line
	( min::file file, min::uns32 offset )
{
    assert ( offset < file->end_offset );

    if ( file->ostream != NULL )
        * file->ostream << & file->buffer[offset]
	                << std::endl;

    if ( file->ofile != NULL_STUB )
    {
        uns32 length =
	    ::strlen ( & file->buffer[offset] );
	min::push ( file->ofile->buffer, length,
	            & file->buffer[offset] );
	min::end_line ( file->ofile );
    }

    if ( file->printer != NULL_STUB )
        file->printer << min::save_print_format
		      << min::verbatim
		      << & file->buffer[offset]
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
        * file->ostream << & file->buffer[offset];
	std::flush ( * file->ostream );
    }

    if ( file->ofile != NULL_STUB )
	min::push ( file->ofile->buffer, length,
	            & file->buffer[offset] );

    if ( file->printer != NULL_STUB )
    {
        file->printer << min::save_print_format
		      << min::verbatim
		      << & file->buffer[offset]
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
    if ( first_spool_line_number + file->spool_lines
         >= line_number )
        return;

    uns32 lines_to_delete =
        line_number - first_spool_line_number;
    assert ( lines_to_delete > 0 );
    assert (   lines_to_delete
             < file->line_index->length );

    uns32 buffer_offset =
	file->line_index[lines_to_delete];
    if ( buffer_offset < file->buffer->length )
	memmove ( & file->buffer[0],
		  & file->buffer[buffer_offset],
		    file->buffer->length
		  - buffer_offset );
    min::pop ( file->buffer, buffer_offset );
    file->next_offset -= buffer_offset;
    if ( file->end_offset != 0 )
	file->end_offset -= buffer_offset;

    memmove ( & file->line_index[0],
	      & file->line_index[lines_to_delete],
		sizeof ( uns32 )
	      * (   file->line_index->length
	          - lines_to_delete ) );
    min::pop ( file->line_index,
	       lines_to_delete );
}

void min::rewind
	( min::file file, min::uns32 line_number )
{
    if ( file->next_line_number == line_number )
        return;

    if ( line_number == 0 )
    {
	if ( file->line_index != NULL_STUB )
	    min::pop ( file->line_index,
	               file->line_index->length );
	file->next_line_number = 0;
	file->next_offset = 0;
    }
    else
    {
	assert ( file->line_index != NULL_STUB );
	assert ( line_number < file->next_line_number );
	min::uns32 lines_to_back_up =
	    file->next_line_number - line_number;
	assert (    file->line_index->length
		 >= lines_to_back_up );
	file->next_offset =
	    file->line_index
	       [  file->line_index->length
		- lines_to_back_up ];
	min::pop ( file->line_index, lines_to_back_up );
	file->next_line_number = line_number;
    }
}

std::ostream & operator <<
	( std::ostream & out, min::file file )
{
    while ( true )
    {
        min::uns32 offset = min::next_line ( file );
	if ( offset == min::NO_LINE ) break;
	out << & file->buffer[offset] << std::endl;
    }

    if ( file->next_offset < file->buffer->length )
    {
	min::push(file->buffer) = 0;
        out << & file->buffer[file->next_offset];
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
	    ::strlen ( & ifile->buffer[offset] );
	min::push ( ofile->buffer, length,
	            & ifile->buffer[offset] );
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
	printer << & file->buffer[offset] << min::eol;
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

static min::packed_vec<min::uns32> hash_table_type
    ( "hash_table_type" );

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
	::hash_table_type.new_stub ( length );
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
             & var ( vp, 0 ),
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
	    lab_of ( element, len, name );
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
	    lab_of ( element, len, ap.attr_name );
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
	    lab_of ( element, len, ap.attr_name );
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
		lab_of ( & atom, 1, name );
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
	lab_of ( & atom, 1, reverse_name );
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
	        ( 1 << ( n - base ) );
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

static bool default_suppress_matrix[256][256];

min::suppress_matrix min::default_suppress_matrix =
    & ::default_suppress_matrix;


// Unicode class must be initialized before default
// suppress matrix is initialized.

static void init_default_suppress_matrix ( void )
{
    for ( unsigned i = 0; i < 256; ++ i )
    {
        ::default_suppress_matrix['('][i] = true;
        ::default_suppress_matrix['['][i] = true;
        ::default_suppress_matrix['{'][i] = true;
        ::default_suppress_matrix[0xAB][i] = true;
		// <<
        ::default_suppress_matrix['`'][i] = true;

        ::default_suppress_matrix[i][')'] = true;
        ::default_suppress_matrix[i][']'] = true;
        ::default_suppress_matrix[i]['}'] = true;
        ::default_suppress_matrix[i][0xB8] = true;
		// >>
        ::default_suppress_matrix[i]['\''] = true;
        ::default_suppress_matrix[i][','] = true;
        ::default_suppress_matrix[i][';'] = true;

	min::uns8 b = min::unicode_class ( i );

	if ( b == 'U' || b == 'L' )
	{
	    for ( unsigned j = 0; j < 256; ++ j )
	    {
		min::uns8 c = min::unicode_class ( j );
		if ( c == 'U' || c == 'L' ) continue;
		if ( c == '\'' ) continue;
		::default_suppress_matrix[i][j] = true;
		::default_suppress_matrix[j][i] = true;
	    }
	}

	if ( '0' <= b && b <= '9' )
	{
	    for ( unsigned j = 0; j < 256; ++ j )
	    {
		min::uns8 c = min::unicode_class ( j );
		if ( '0' <= c && c <= '9' ) continue;
		if ( c == ',' ) continue;
		if ( c == '.' ) continue;
		if ( c == ':' ) continue;
		if ( c == '/' ) continue;
		::default_suppress_matrix[i][j] = true;
		::default_suppress_matrix[j][i] = true;
	    }
	}
    }
}

const min::gen_format min::default_gen_format =
{
    min::default_pgen,
    "%.15g",
    "\"","\"","<Q>",
    "[: ", " ", " :]",
    "[$ ", " $]",
    "[. ", " .]",
    min::NULL_STUB,
    min::default_suppress_matrix,
    8,
    min::NULL_STUB, // Reset by initialization.
    min::NULL_STUB  // Reset by initialization.
};

const min::line_break min::default_line_break =
{
    0, 0, 72, 4
};

const min::print_format min::default_print_format =
{
    min::DEFAULT_PRINT_FLAGS,
    min::DEFAULT_GEN_FLAGS,
    min::DEFAULT_FLUSH_GEN_FLAGS,
    min::DEFAULT_FLUSH_SUBOBJ_GEN_FLAGS,
    & min::default_gen_format
};

min::print_format min::ostream_print_format =
{
    min::DEFAULT_PRINT_FLAGS,
    min::DEFAULT_GEN_FLAGS,
    min::DEFAULT_FLUSH_GEN_FLAGS,
    min::DEFAULT_FLUSH_SUBOBJ_GEN_FLAGS,
    & min::default_gen_format
};

static min::uns32 space[1] = { ' ' };

// Return true iff c is non-spacing combining mark.
//
inline bool is_non_spacing ( min::uns32 c )
{
    return min::unicode_class ( c ) == 'N';
}

// Representations for printing control characters in
// ASCII GRAPHIC mode.  utf8graphic[c] is the repre-
// sentation for character c if c <= 0x20, for DEL
// if c == DEL_REP, and for the illegal unicode
// character representative if c == ILL_REP.
//
static const min::uns32 DEL_REP = 0x21;
static const min::uns32 ILL_REP = 0x22;
static const char * asciigraphic[0x23] = {
    "<NUL>",
    "<SOH>",
    "<STX>",
    "<ETX>",
    "<EOT>",
    "<ENQ>",
    "<ACK>",
    "<BEL>",
    "<BS>",
    "<HT>",
    "<LF>",
    "<VT>",
    "<FF>",
    "<CR>",
    "<SO>",
    "<SI>",
    "<DLE>",
    "<DC1>",
    "<DC2>",
    "<DC3>",
    "<DC4>",
    "<NAK>",
    "<SYN>",
    "<ETB>",
    "<CAN>",
    "<EM>",
    "<SUB>",
    "<ESC>",
    "<FS>",
    "<GS>",
    "<RS>",
    "<US>",
    "<SP>",
    "<DEL>",
    "<ILL>",
};

// UTF8 Codes for printing control characters in UTF8
// GRAPHIC mode.  utf8graphic[c] is the representation
// for character c if c <= 0x20, for DEL if c == DEL_
// REP, and for the illegal unicode character represen-
// tative if c == ILL_REP.
//
// UTF8 codes are those of the UNICODE Control Pictures
// script, range 0x2400-0x243F, except for ILL_REP,
// which is min::ILLEGAL_UTF8.
//
static char utf8graphic[0x23][8];

// Initialize utf8control.  Called when first
// min::printer created.
//
static void init_utf8graphic ( void )
{
    // utf8graphic is initially all 0's, so we do not
    // have to insert string ending NUL's.
    //
    char * s;
    for ( min::uns32 c = 0; c < 0x20; ++ c )
        min::unicode_to_utf8
	    ( s = utf8graphic[c], 0x2400 + c );
    min::unicode_to_utf8
	    ( s = utf8graphic['\n'], 0x2424 );
    min::unicode_to_utf8
	    ( s = utf8graphic[' '], 0x2423 );
    min::unicode_to_utf8
	    ( s = utf8graphic[DEL_REP], 0x2421 );
    min::unicode_to_utf8
	    ( s = utf8graphic[ILL_REP],
	      min::ILLEGAL_UTF8 );
}

static char NUL_UTF8_ENCODING[3] = { 0xC0, 0x80, 0 };

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

static void init_utf8graphic ( void );
min::printer min::init
	( min::ref<min::printer> printer,
	  min::file file )
{
    if ( printer == NULL_STUB )
    {
        static bool first = true;
	if ( first )
	{
	    first = false;
	    ::init_utf8graphic();
	}

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

    return printer;
}

min::printer min::init_ostream
	( min::ref<min::printer> printer,
	  std::ostream & ostream )
{
    init ( printer );
    init_ostream ( file_ref(printer), ostream );
    return printer << min::eol_flush;
}

static void end_line ( min::printer printer )
{
    printer->suppress_matrix = NULL;

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
    min::uns32 flags = printer->print_format.flags;
    if ( flags & min::DISPLAY_EOL_FLAG )
    {
	const char * rep;
        if ( flags & min::ASCII_FLAG )
	    rep = asciigraphic['\n'];
	else
	    rep = utf8graphic['\n'];
	int len = ::strlen ( rep );
	min::push ( buffer, len, rep );
    }

    min::end_line ( printer->file );

    printer->column = 0;
    printer->line_break.offset = buffer->length;
    printer->line_break.column = 0;
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
    {
	const min::gen_format * f =
	    printer->print_format.gen_format;
	if  ( f == NULL )
	    f = & min::default_gen_format;

	return ( * f->pgen )
	    ( printer, MUP::new_gen ( op.v1.g ),
	      printer->print_format.gen_flags,
	      printer->print_format.gen_flags, f );
    }
    case min::op::PGEN_WITH_FLAGS:
    {
	const min::gen_format * f =
	    printer->print_format.gen_format;
	if  ( f == NULL )
	    f = & min::default_gen_format;

	return ( * f->pgen )
	    ( printer, MUP::new_gen ( op.v1.g ),
	      op.v2.u32, op.v3.u32, f );
    }
    case min::op::FLUSH_PGEN:
    {
	const min::gen_format * f =
	    printer->print_format.gen_format;
	if  ( f == NULL )
	    f = & min::default_gen_format;

	return ( * f->pgen )
	    ( printer, MUP::new_gen ( op.v1.g ),
	      printer->print_format.flush_gen_flags,
	      printer->print_format
	                   .flush_subobj_gen_flags, f );
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
	return MINT::print_unicode
	    ( printer, 1, & op.v1.u32 );
    case min::op::PUNICODE2:
	return MINT::print_unicode
	    ( printer, op.v1.uptr,
		(const min::uns32 *) op.v2.p );
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
    case min::op::SET_GEN_FLAGS:
	printer->print_format.gen_flags |= op.v1.u32;
	return printer;
    case min::op::CLEAR_GEN_FLAGS:
	printer->print_format.gen_flags &= ~ op.v1.u32;
	return printer;
    case min::op::SET_FLUSH_GEN_FLAGS:
	printer->print_format.flush_gen_flags |=
	    op.v1.u32;
	return printer;
    case min::op::CLEAR_FLUSH_GEN_FLAGS:
	printer->print_format.flush_gen_flags &=
	    ~ op.v1.u32;
	return printer;
    case min::op::SET_FLUSH_SUBOBJ_GEN_FLAGS:
	printer->print_format.flush_subobj_gen_flags |=
	    op.v1.u32;
	return printer;
    case min::op::CLEAR_FLUSH_SUBOBJ_GEN_FLAGS:
	printer->print_format.flush_subobj_gen_flags &=
	    ~ op.v1.u32;
	return printer;
    case min::op::SET_GEN_FORMAT:
	printer->print_format.gen_format =
	    (const min::gen_format *) op.v1.p;
	return printer;
    case min::op::SET_PRINT_FLAGS:
	printer->print_format.flags |= op.v1.u32;
	return printer;
    case min::op::CLEAR_PRINT_FLAGS:
	printer->print_format.flags &= ~ op.v1.u32;
	return printer;
    case min::op::VERBATIM:
	printer->print_format.flags |=
	      min::ALLOW_HSPACE_FLAG
	    + min::ALLOW_VSPACE_FLAG
	    + min::ALLOW_NSPACE_FLAG;
	printer->print_format.flags &=
	    ~ (   min::GRAPHIC_HSPACE_FLAG
	        + min::GRAPHIC_VSPACE_FLAG
	        + min::GRAPHIC_NSPACE_FLAG
	        + min::HBREAK_FLAG
	        + min::GBREAK_FLAG
	        + min::ASCII_FLAG );
	return printer;
    case min::op::SUPPRESSIBLE_SPACE:
	{
	    printer->suppress_matrix =
	        printer->print_format.gen_format
		       ->suppress_matrix;
	    if ( printer->suppress_matrix == NULL )
	    {
		MINT::print_unicode
		    ( printer, 1, ::space );
		return printer;
	    }
	    min::packed_vec_insptr<char> buffer =
	        printer->file->buffer;
	    min::unsptr length = buffer->length;
	    min::uns32 b = '\n';
	    if ( length > 0 )
	    {
	        // Code to set b to the last UNICODE
		// character in buffer.  This block of
		// code contains relocatable pointers.

	        const char * p =
		    min::end_ptr_of ( buffer );
		const char * q = p;
		if ( p[-1] != 0 ) while ( true )
		{
		    assert ( length > 0 );

		    -- q;
		    -- length;

		    if ( ( * q & 0x80 ) == 0 )
		    {
			b = min::utf8_to_unicode
			        ( q, p );
			assert ( q == p );
		        break;
		    }
		}
	    }

	    printer->previous_unicode_class =
		min::unicode_class ( b );
	    printer->previous_print_flags =
	        printer->print_format.flags;
		    
	    return printer;
	}
    case min::op::SPACE:
	MINT::print_unicode ( printer, 1, ::space );
	return printer;
    case min::op::RESTORE_INDENT:
	printer->line_break =
	    min::pop ( printer->line_break_stack );
	return printer;
    case min::op::SAVE_INDENT:
    save_indent:
        min::push ( printer->line_break_stack ) =
	    printer->line_break;
	printer->line_break.indent = printer->column;
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
	if (   printer->print_format.flags
	     & min::EOL_FLUSH_FLAG )
	    min::flush_file ( printer->file );
	if (   printer->print_format.flags
	     & min::FLUSH_ID_MAP_FLAG )
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
	if (   printer->print_format.flags
	     & min::EOL_FLUSH_FLAG )
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
        while (   printer->column
	        <   printer->line_break.column
		  + op.v1.u32 )
	{
	    ++ printer->column;
	    min::push(printer->file->buffer) = ' ';
	}
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
	        memmove ( & buffer[offset + n],
		          & buffer[offset],
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
	while (   printer->column
		< printer->line_break.indent )
	{
	    ++ printer->column;
	    min::push(printer->file->buffer) = ' ';
	}
	goto set_break;
    case min::op::SPACES_IF_BEFORE_INDENT:
	while (   printer->column
		< printer->line_break.indent )
	{
	    ++ printer->column;
	    min::push(printer->file->buffer) = ' ';
	}
	goto set_break;
    case min::op::SPACE_IF_AFTER_INDENT:
        if (   printer->column
	     > printer->line_break.indent )
	{
	    ++ printer->column;
	    min::push(printer->file->buffer) = ' ';
	}
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

static std::ostream & ostream_unicode
	( std::ostream & out, min::uns32 flags,
	  min::unsptr n, const min::uns32 * str );

static std::ostream & ostream_pgen
	( std::ostream & out,
	  min::gen v,
	  min::uns32 gen_flags,
	  min::uns32 subobj_gen_flags,
	  const min::gen_format * f );

static std::ostream & flush_one_id
	( std::ostream & out );

static min::locatable_var<min::id_map> ostream_id_map;

std::ostream & operator <<
	( std::ostream & out,
	  const min::op & op )
{
    char buffer[256];
    switch ( op.opcode )
    {
    case min::op::SET_GEN_FLAGS:
	min::ostream_print_format.gen_flags |=
	    op.v1.u32;
	return out;
    case min::op::CLEAR_GEN_FLAGS:
	min::ostream_print_format.gen_flags &=
	    ~ op.v1.u32;
	return out;
    case min::op::SET_FLUSH_GEN_FLAGS:
	min::ostream_print_format.flush_gen_flags |=
	    op.v1.u32;
	return out;
    case min::op::CLEAR_FLUSH_GEN_FLAGS:
	min::ostream_print_format.flush_gen_flags &=
	    ~ op.v1.u32;
	return out;
    case min::op::SET_FLUSH_SUBOBJ_GEN_FLAGS:
	min::ostream_print_format
	    .flush_subobj_gen_flags |= op.v1.u32;
	return out;
    case min::op::CLEAR_FLUSH_SUBOBJ_GEN_FLAGS:
	min::ostream_print_format
	    .flush_subobj_gen_flags &= ~ op.v1.u32;
	return out;
    case min::op::SET_GEN_FORMAT:
	min::ostream_print_format.gen_format =
	    (const min::gen_format *) op.v1.p;
	return out;
    case min::op::SET_PRINT_FLAGS:
	min::ostream_print_format.flags |= op.v1.u32;
	return out;
    case min::op::CLEAR_PRINT_FLAGS:
	min::ostream_print_format.flags &= ~ op.v1.u32;
	return out;
    case min::op::PGEN:
    {
	const min::gen_format * f =
	    min::ostream_print_format.gen_format;
	if  ( f == NULL )
	    f = & min::default_gen_format;

	return ::ostream_pgen
	    ( out, MUP::new_gen ( op.v1.g ),
	      min::ostream_print_format.gen_flags,
	      min::ostream_print_format.gen_flags, f );
    }
    case min::op::PGEN_WITH_FLAGS:
    {
	const min::gen_format * f =
	    min::ostream_print_format.gen_format;
	if  ( f == NULL )
	    f = & min::default_gen_format;

	return ::ostream_pgen
	    ( out, MUP::new_gen ( op.v1.g ),
	      op.v2.u32, op.v3.u32, f );
    }
    case min::op::FLUSH_PGEN:
    {
	const min::gen_format * f =
	    min::ostream_print_format.gen_format;
	if  ( f == NULL )
	    f = & min::default_gen_format;

	return ::ostream_pgen
	    ( out, MUP::new_gen ( op.v1.g ),
	      min::ostream_print_format
	          .flush_gen_flags,
	      min::ostream_print_format
	          .flush_subobj_gen_flags, f );
    }
    case min::op::FLUSH_ONE_ID:
        return ::flush_one_id ( out );
    case min::op::FLUSH_ID_MAP:
    {
        min::id_map id_map = ::ostream_id_map;
	if ( id_map != min::NULL_STUB )
	while ( id_map->next < id_map->length )
	    ::flush_one_id ( out );
	return out;
    }
    case min::op::PUNICODE1:
	return ::ostream_unicode
	    ( out, min::ostream_print_format.flags,
	      1, & op.v1.u32 );
    case min::op::PUNICODE2:
	return ::ostream_unicode
	    ( out, min::ostream_print_format.flags,
	      op.v1.uptr,
	      (const min::uns32 *) op.v2.p );
    case min::op::PINT:
	sprintf ( buffer, (const char *) op.v2.p,
			  op.v1.i64 );
	return out << buffer;
    case min::op::PUNS:
	sprintf ( buffer, (const char *) op.v2.p,
			  op.v1.u64 );
	return out << buffer;
    case min::op::PFLOAT:
	sprintf ( buffer, (const char *) op.v2.p,
			  op.v1.f64 );
	return out << buffer;
    case min::op::SUPPRESSIBLE_SPACE:
    case min::op::SPACE:
    case min::op::SPACE_IF_AFTER_INDENT:
	return out << " ";
    case min::op::EOM:
	out << std::endl;
	if (   min::ostream_print_format.flags
	     & min::FLUSH_ID_MAP_FLAG )
	{
	    min::id_map id_map = ::ostream_id_map;
	    if ( id_map != min::NULL_STUB )
	    while ( id_map->next < id_map->length )
		::flush_one_id ( out );
	}
	return out;
    case min::op::BOL:
    case min::op::EOL_IF_AFTER_INDENT:
    case min::op::EOL:
	return out << std::endl;
    case min::op::FLUSH:
	return out << std::flush;
    default:
        return out;
    }
}

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

const min::op min::ascii
    ( min::op::SET_PRINT_FLAGS, min::ASCII_FLAG );
const min::op min::noascii
    ( min::op::CLEAR_PRINT_FLAGS, min::ASCII_FLAG );

const min::op min::graphic_hspace
    ( min::op::SET_PRINT_FLAGS,
      min::GRAPHIC_HSPACE_FLAG );
const min::op min::nographic_hspace
    ( min::op::CLEAR_PRINT_FLAGS,
      min::GRAPHIC_HSPACE_FLAG );
const min::op min::graphic_vspace
    ( min::op::SET_PRINT_FLAGS,
      min::GRAPHIC_VSPACE_FLAG );
const min::op min::nographic_vspace
    ( min::op::CLEAR_PRINT_FLAGS,
      min::GRAPHIC_VSPACE_FLAG );
const min::op min::graphic_nspace
    ( min::op::SET_PRINT_FLAGS,
      min::GRAPHIC_NSPACE_FLAG );
const min::op min::nographic_nspace
    ( min::op::CLEAR_PRINT_FLAGS,
      min::GRAPHIC_NSPACE_FLAG );

const min::op min::allow
    ( min::op::SET_PRINT_FLAGS,
      min::ALLOW_FLAGS );
const min::op min::noallow
    ( min::op::CLEAR_PRINT_FLAGS,
      min::ALLOW_FLAGS );
const min::op min::allow_hspace
    ( min::op::SET_PRINT_FLAGS,
      min::ALLOW_HSPACE_FLAG );
const min::op min::noallow_hspace
    ( min::op::CLEAR_PRINT_FLAGS,
      min::ALLOW_HSPACE_FLAG );
const min::op min::allow_vspace
    ( min::op::SET_PRINT_FLAGS,
      min::ALLOW_VSPACE_FLAG );
const min::op min::noallow_vspace
    ( min::op::CLEAR_PRINT_FLAGS,
      min::ALLOW_VSPACE_FLAG );
const min::op min::allow_nspace
    ( min::op::SET_PRINT_FLAGS,
      min::ALLOW_NSPACE_FLAG );
const min::op min::noallow_nspace
    ( min::op::CLEAR_PRINT_FLAGS,
      min::ALLOW_NSPACE_FLAG );

const min::op min::display_eol
    ( min::op::SET_PRINT_FLAGS,
      min::DISPLAY_EOL_FLAG );
const min::op min::nodisplay_eol
    ( min::op::CLEAR_PRINT_FLAGS,
      min::DISPLAY_EOL_FLAG );

const min::op min::hbreak
    ( min::op::SET_PRINT_FLAGS, min::HBREAK_FLAG );
const min::op min::nohbreak
    ( min::op::CLEAR_PRINT_FLAGS, min::HBREAK_FLAG );
const min::op min::gbreak
    ( min::op::SET_PRINT_FLAGS, min::GBREAK_FLAG );
const min::op min::nogbreak
    ( min::op::CLEAR_PRINT_FLAGS, min::GBREAK_FLAG );

const min::op min::eol_flush
    ( min::op::SET_PRINT_FLAGS, min::EOL_FLUSH_FLAG );
const min::op min::noeol_flush
    ( min::op::CLEAR_PRINT_FLAGS,
      min::EOL_FLUSH_FLAG );

const min::op min::graphic
    ( min::op::SET_PRINT_FLAGS, min::GRAPHIC_FLAGS );
const min::op min::nographic
    ( min::op::CLEAR_PRINT_FLAGS, min::GRAPHIC_FLAGS );
const min::op min::verbatim
    ( min::op::VERBATIM );
const min::op min::suppressible_space
    ( min::op::SUPPRESSIBLE_SPACE );
const min::op min::space
    ( min::op::SPACE );

const min::op min::flush_one_id
    ( min::op::FLUSH_ONE_ID );
const min::op min::flush_id_map
    ( min::op::FLUSH_ID_MAP );

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
	// to the stack so we do not have to use a
	// min::ptr to access it.
    min::uns32 i;
        // i is also needed at end to copy line_break
	// back.
    for ( i = 0; i < line_break_stack->length; ++ i )
    {
        if ( line_break_stack[i].column
	     > line_break_stack[i].indent )
	{
	    line_break = line_break_stack[i];
	    break;
	}
    }
    if ( line_break.column <= line_break.indent )
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
	        ( & buffer[begoff+line_break.indent+1],
		  & buffer[endoff],
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
	        ( & buffer[begoff+line_break.indent+1],
		  & buffer[endoff],
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
	    line_break_stack[i].offset += offset_adj;
	    line_break_stack[i].column += column_adj;
	    line_break_stack[i].indent += column_adj;
	}
	printer->line_break.offset += offset_adj;
	printer->line_break.column += column_adj;
	printer->line_break.indent += column_adj;
    }
    else
        printer->line_break = line_break;

    return false;
}

min::printer operator <<
	( min::printer printer, const char * s )
{
    // We translate this case to the punicode case
    // because we need to be able to perform look-ahead
    // for combining diacritics, and in the future for
    // other things.
    
    int length = ::strlen ( s );
    const char * ends = s + length;

    min::uns32 buffer[length];
    min::uns32 i = 0;

    while ( s < ends )
    {
	min::uns32 c = (min::uns8) * s ++;
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

    return MINT::print_unicode ( printer, i, buffer );
}

min::printer operator <<
	( min::printer printer, min::ptr<const char> s )
{
    min::unsptr length = strlen ( s ) + 1;
    MIN_STACK_COPY ( char, buffer, length, s );
    return printer << buffer;
}

min::printer MINT::print_unicode
	( min::printer printer,
	  min::unsptr n, const min::uns32 * str )
{
    uns32 flags = printer->print_format.flags;
    uns32 line_length = printer->line_break.line_length;

    min::packed_vec_insptr<char> buffer =
        printer->file->buffer;

    if ( printer->suppress_matrix != NULL )
    {
        if ( n == 0 ) return printer;

	uns8 B = printer->previous_unicode_class;
	uns8 C = min::unicode_class ( str[0] );
	bool suppress =
	    ( * printer->suppress_matrix )[B][C];

        printer->suppress_matrix = NULL;
	if ( ! suppress )
	{
	    printer->print_format.flags =
	        printer->previous_print_flags;
	    MINT::print_unicode ( printer, 1, ::space );
	    printer->print_format.flags = flags;
	}
    }

    bool ascii = 
	( ( flags & min::ASCII_FLAG ) != 0 );
    bool gbreak =
	( ( flags & min::GBREAK_FLAG ) != 0 );
    bool hbreak =
	( ( flags & min::HBREAK_FLAG ) != 0 );

    char temp[32];

    char lastc = ( buffer->length == 0 ? 0 :
                   buffer[buffer->length - 1] );
    bool hspace_precedes =
	( lastc == ' ' || lastc == '\t' );
    bool no_line_break_enabled = false;
    while ( n -- )
    {
        uns32 c = * str ++;

	// Divide into cases.  For each, either process
	// and continue, or fall through to output
	// utf8graphic[c] or asciigraphic[c].
	//
	if ( 0x20 < c && c < 0x7F )
	{
	    /* Common case: ASCII graphic character. */

	    if ( gbreak
	         ||
		 ( hbreak && hspace_precedes ) )
	    {
		printer->line_break.offset =
		    buffer->length;
		printer->line_break.column =
		    printer->column;
		no_line_break_enabled = false;
	    }

	    if ( printer->column >= line_length
	         &&
		 ! no_line_break_enabled )
	        no_line_break_enabled =
		    ::insert_line_break ( printer );
	   
	    min::push(buffer) = (char) c;
	    ++ printer->column;
	    hspace_precedes = false;

	    continue;
	}
	else if ( c == ' ' )
	{
	    if ( flags & min::GRAPHIC_HSPACE_FLAG )
	        ; // Drop through
	    else
	    {
		if ( gbreak && ! hspace_precedes )
		{
		    printer->line_break.offset =
		        buffer->length;
		    printer->line_break.column =
		        printer->column;
		    no_line_break_enabled = false;
		}

		++ printer->column;
		min::push(buffer) = ' ';
		hspace_precedes = true;
		
		continue;
	    }
	}
	else if ( c == '\t' )
	{
	    if ( flags & min::GRAPHIC_HSPACE_FLAG )
	        ; // Drop through
	    else
	    {
		if ( gbreak && ! hspace_precedes )
		{
		    printer->line_break.offset =
		        buffer->length;
		    printer->line_break.column =
		        printer->column;
		    no_line_break_enabled = false;
		}

	        uns32 spaces = 8 - printer->column % 8;
		printer->column += spaces;

	        if ( flags & min::ALLOW_HSPACE_FLAG )
		    min::push(buffer) = '\t';
		else
		    min::push
			( buffer, spaces, "        " );
		hspace_precedes = true;

		continue;
	    }
	}
	else if ( c == '\f' || c == '\v' )
	{
	    if ( flags & min::GRAPHIC_VSPACE_FLAG )
	        ; // Drop through
	    else
	    {
	        if ( flags & min::ALLOW_VSPACE_FLAG )
		{
		    min::push(buffer) = (char) c;
		    hspace_precedes = false;
		}

		continue;
	    }
	}
	else if ( c < 0x20 || c == 0x7F )
	{
	    /* Non-Space Control Character */
	   
	    if ( flags & min::GRAPHIC_NSPACE_FLAG )
	        ; // Drop through
	    else
	    {
		if ( flags & min::ALLOW_NSPACE_FLAG )
		{
		    if ( c == 0 )
			min::push
			  ( buffer,
			    2, NUL_UTF8_ENCODING );
		    else
			min::push(buffer) = (char) c;
		    hspace_precedes = false;
		}

		continue;
	    }
	}
	else
	{
	    /* Non-ASCII UNICODE Character. */
	   
	    const char * rep;
	    int len;
	    int columns;
	    bool is_non_spacing =
	        ::is_non_spacing ( c );
	    if( ascii )
	    {
		if ( c == min::ILLEGAL_UTF8 )
		{
		    rep = asciigraphic[ILL_REP];
		    len = ::strlen ( rep );
		    columns = len;
		}
		else
		{
		    len = sprintf
			( temp+1, "<%X>", c );
		    char * p = temp + 1;
		    if ( p[1] < '0' || '9' < p[1] )
		    {
			* p -- = '0';
			* p = '<';
			++ len;
		    }
		    rep = p;
		    columns = len;
		}
	    }
	    else /* not ascii mode */
	    {
		columns = ! is_non_spacing;
		if ( c == min::ILLEGAL_UTF8 )
		{
		    rep = utf8graphic[ILL_REP];
		    len = ::strlen ( rep );
		}
		else
		{
		    rep = temp;
		    char * p = temp;
		    len = min::unicode_to_utf8
		               ( p, c );
		}
	    }

	    if ( columns > 0  // ASCII or non-diacritic
	         &&
		 ( gbreak
		   ||
		   ( hbreak && hspace_precedes ) ) )
	    {
		printer->line_break.offset =
		    buffer->length;
		printer->line_break.column =
		    printer->column;
		no_line_break_enabled = false;
	    }
	   
	    if ( printer->column + columns > line_length
	         &&
		 ! no_line_break_enabled )
	        no_line_break_enabled =
		    ::insert_line_break ( printer );
	   
	    min::push ( buffer, len, rep );
	    printer->column += columns;

	    hspace_precedes = false;

	    continue;
	}

	// Output c as utf8graphic[c] or
	// asciigraphic[c].

	// Recode DEL as DEL_REP.
	//
	if ( c == 0x7F ) c = DEL_REP;
       
	const char * rep;
	int len;
	int columns;
	if ( ascii )
	{
	    rep = asciigraphic[c];
	    len = ::strlen ( rep );
	    columns = len;
	}
	else
	{
	    rep = utf8graphic[c];
	    len = ::strlen ( rep );
	    columns = 1;
	}

	if ( gbreak || ( hbreak && hspace_precedes ) )
	{
	    printer->line_break.offset =
	        buffer->length;
	    printer->line_break.column =
	        printer->column;
	    no_line_break_enabled = false;
	}

	if ( printer->column + columns > line_length
	     &&
	     ! no_line_break_enabled )
	    no_line_break_enabled =
		::insert_line_break ( printer );
       
	min::push ( buffer, len, rep );
	printer->column += columns;
	hspace_precedes = false;
    }

    return printer;
}

static std::ostream & ostream_unicode
	( std::ostream & out, min::uns32 flags,
	  min::unsptr n, const min::uns32 * str )
{
    bool ascii = 
	( ( flags & min::ASCII_FLAG ) != 0 );

    char temp[32];

    while ( n -- )
    {
        min::uns32 c = * str ++;

	// Divide into cases.  For each, either process
	// and continue, or fall through to output
	// utf8graphic[c] or asciigraphic[c].
	//
	if ( 0x20 < c && c < 0x7F )
	{
	    /* Common case: ASCII graphic character. */

	    out << (char) c;

	    continue;
	}
	else if ( c == ' ' )
	{
	    if ( flags & min::GRAPHIC_HSPACE_FLAG )
	        ; // Drop through
	    else
	    {
		out << ' ';
		
		continue;
	    }
	}
	else if ( c == '\t' )
	{
	    if ( flags & min::GRAPHIC_HSPACE_FLAG )
	        ; // Drop through
	    else
	    {
	        out << '\t';

		continue;
	    }
	}
	else if ( c == '\f' || c == '\v' )
	{
	    if ( flags & min::GRAPHIC_VSPACE_FLAG )
	        ; // Drop through
	    else
	    {
	        if ( flags & min::ALLOW_VSPACE_FLAG )
		    out << (char) c;

		continue;
	    }
	}
	else if ( c < 0x20 || c == 0x7F )
	{
	    /* Non-Space Control Character */
	   
	    if ( flags & min::GRAPHIC_NSPACE_FLAG )
	        ; // Drop through
	    else
	    {
		if ( flags & min::ALLOW_NSPACE_FLAG )
		{
		    if ( c == 0 )
			out << NUL_UTF8_ENCODING;
		    else
			out << (char) c;
		}

		continue;
	    }
	}
	else
	{
	    /* Non-ASCII UNICODE Character. */
	   
	    const char * rep;
	    if( ascii )
	    {
		if ( c == min::ILLEGAL_UTF8 )
		    rep = asciigraphic[ILL_REP];
		else
		{
		    sprintf ( temp+1, "<%X>", c );
		    char * p = temp + 1;
		    if ( p[1] < '0' || '9' < p[1] )
		    {
			* p -- = '0';
			* p = '<';
		    }
		    rep = p;
		}
	    }
	    else /* not ascii mode */
	    {
		if ( c == min::ILLEGAL_UTF8 )
		    rep = utf8graphic[ILL_REP];
		else
		{
		    char * p = temp;
		    min::unsptr len =
		        min::unicode_to_utf8 ( p, c );
		    temp[len] = 0;
		    rep = temp;
		}
	    }

	   
	    out << rep;

	    continue;
	}

	// Output c as utf8graphic[c] or
	// asciigraphic[c].

	// Recode DEL as DEL_REP.
	//
	if ( c == 0x7F ) c = DEL_REP;
       
	const char * rep;
	if ( ascii )
	    rep = asciigraphic[c];
	else
	    rep = utf8graphic[c];

       
        out << rep;
    }

    return out;
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

void MINT::pwidth
    ( min::uns32 & column,
      min::uns32 c, min::uns32 print_flags )
{
    // Continuation of inline min::pwidth.

    // Divide into cases.  Update column and return, or
    // fall through to use asciigraphic[c] if ASCII or
    // just add 1 to column if not ASCII.
    //
    if ( c == '\f' || c == '\v' )
    {
        if ( print_flags & min::GRAPHIC_VSPACE_FLAG )
	    ; // Fall through
	else
	    return;
    }
    else if ( c < 0x20 || c == 0x7F )
    {
        if ( print_flags & min::GRAPHIC_NSPACE_FLAG )
	    ; // Fall through
	else
	    return;
    }
    else // c > 0x7F
    {
	if ( print_flags & min::ASCII_FLAG )
	{
	    if ( c == min::ILLEGAL_UTF8 ) \
	        column += 
		    ::strlen ( asciigraphic[ILL_REP] );
	    else
	    {
	        while ( c > 0xF )
		    c >>= 4, ++ column;
		if ( c > 9 ) ++ column;
		column += 3;
	    }
	}
	else if ( ! ::is_non_spacing ( c ) )
	    ++ column;

	return;
    }

    // We have fallen through.
    //
    if ( print_flags & min::ASCII_FLAG )
    {
	if ( c == 0x7F ) c = DEL_REP;
	column += ::strlen ( asciigraphic[c] );
    }
    else ++ column;
}

void min::pwidth ( min::uns32 & column,
                   const char * s, min::unsptr n,
		   min::uns32 print_flags )
{
    const char * ends = s + n;
    while ( s < ends )
        pwidth ( column, utf8_to_unicode ( s, ends ),
	         print_flags );
}


// Printing General values
// -------- ------- ------

// Execute pgen (below) in case OBJ_ID_FLAG is
// effective.
//
template < typename T >
static T pgen_id
	( T out,
	  min::gen v );
template <>
static min::printer pgen_id<min::printer>
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
template <>
static std::ostream & pgen_id<std::ostream &>
	( std::ostream & out,
	  min::gen v )
{
    if ( ::ostream_id_map == min::NULL_STUB )
    	init ( ::ostream_id_map );

    min::uns32 id =
        min::find_or_add
	    ( ::ostream_id_map, min::stub_of ( v ) );
    return out << "@" << id;
}

// Execute pgen (below) in the case an object is to be
// printed without an effective OBJ_EXP_FLAG or
// OBJ_ID_FLAG.
//
template < typename T >
static T pgen_obj
	( T out,
	  min::gen v,
	  min::uns32 gen_flags,
	  min::uns32 subobj_gen_flags,
	  const min::gen_format * f,
	  T ( * pgen )
	      ( T out,
	        min::gen v,
		min::uns32 gen_flags,
		min::uns32 subobj_gen_flags,
		const min::gen_format * f ) )
{
    bool indent = ( gen_flags & min::OBJ_INDENT_FLAG );

    min::obj_vec_ptr vp ( v );
    min::attr_ptr ap ( vp );

    min::attr_info first_info[1000];
    min::attr_info * info = first_info;
    min::unsptr m = min::get_attrs ( info, 1000, ap );

    min::attr_info second_info[m];
    if ( m > 1000 )
    {
	info = second_info;
        min::get_attrs ( info, m, ap );
    }

    if ( m == 0 && min::size_of ( vp ) == 0 )
    {
        if ( indent )
	    return out << "// <empty object>";
	else
	    return out << "{| |}";
    }

    min::uns32 name_flags = subobj_gen_flags;
    name_flags |= min::GRAPHIC_STR_FLAG;
    name_flags |= min::SUPPRESS_LAB_SPACE_FLAG;
    name_flags &= ~ min::BRACKET_STR_FLAG;
    name_flags &= ~ min::BRACKET_LAB_FLAG;

    if ( ! indent )
	out << "{| " << min::save_indent
	    << min::nohbreak;
    else
        out << min::save_indent
	    << min::nohbreak
	    << min::adjust_indent ( 4 );

    bool include_attr_vec = false;
    for ( min::unsptr i = 0;
	  i < min::size_of ( vp ); ++ i )
    {
	if ( i != 0 )
	    out << " " << min::set_break;

        min::gen v = vp[i];
	if ( min::is_sublist ( v ) )
	{
	    out << "**";
	    include_attr_vec = true;
	}
	else
	    pgen ( out, vp[i],
	           subobj_gen_flags, subobj_gen_flags,
		   f );
    }

    if ( indent )
        out << min::adjust_indent ( -4 );

    if ( indent && m > 0 ) out << " |:";

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
		out << min::indent;
	    else
		out << "; " << min::set_break;

	    pgen ( out, info[i].name,
	           name_flags, name_flags, f );

	    min::unsptr fc = info[i].flag_count;
	    if ( fc > 0 )
	    {
		min::packed_vec_ptr<const char *>
		    names = f->flag_names;
		out << "[";
		if ( fc * min::VSIZE <= 64 )
		{
		    min::uns64 f = info[i].flags;
		    for ( unsigned j = 0; j < 64; ++ j )
		    {
			if ( f & 1 ) out << names[j];
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
				    out << names[n];
				else
				    out << "," << n;
			    }
			    flags2 >>= 1;
			    ++ n;
			}
		    }
		}
		out << "]";
	    }

	    min::unsptr vc = info[i].value_count;
	    if ( j == 0 && vc == 1 )
	    {
		out << " = ";
		pgen ( out, info[i].value,
		       subobj_gen_flags,
		       subobj_gen_flags, f );
	    }
	    else if ( j == 0 && vc > 1 )
	    {
		out << " = "
		    << "{: " << min::save_indent;
		min::gen value[vc];
		min::locate ( ap, info[i].name );
		min::get ( value, vc, ap );
		for ( min::unsptr k = 0; k < vc; ++ k )
		{
		    if ( k > 0 )
			out << "; " << min::set_break;
		    pgen ( out, value[k],
		           subobj_gen_flags,
		           subobj_gen_flags, f );
		}
		out << " :}" << min::restore_indent;
	    }
	    else
	    {
		min::unsptr rvc =
		    rinfo[j-do_values].value_count;
		if ( rvc == 1 )
		{
		    out << " = " << min::set_break;
		    ::pgen_id<T>
		        ( out,
		          rinfo[j-do_values].value );
		}
		else if ( rvc > 1 )
		{
		    out << " = " << min::set_break
			<< "{: " << min::save_indent;
		    min::gen v[rvc];
		    min::locate_reverse
			( ap, rinfo[j-do_values].name );
		    min::get ( v, rvc, ap );
		    for ( min::unsptr k = 0;
			  k < rvc; ++ k )
		    {
			if ( k > 0 )
			    out << "; "
				<< min::set_break;
			::pgen_id<T> ( out, v[k] );
		    }
		    out << " :}" << min::restore_indent;
		}
		out << min::set_break << " = ";
		pgen ( out, rinfo[j-do_values].name,
		       name_flags, name_flags, f );
	    }
	}
    }

    if ( ! indent ) out << " |}";
    
    return out << min::restore_indent;
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
template < typename T >
static T pgen_exp
	( T out,
	  min::attr_ptr & ap,
	  min::uns32 gen_flags,
	  min::uns32 subobj_gen_flags,
	  const min::gen_format * f,
	  T ( * pgen )
	      ( T out,
	        min::gen v,
		min::uns32 gen_flags,
		min::uns32 subobj_gen_flags,
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

    min::uns32 name_flags = subobj_gen_flags;
    name_flags |= min::GRAPHIC_STR_FLAG;
    name_flags |= min::SUPPRESS_LAB_SPACE_FLAG;
    name_flags &= ~ min::BRACKET_STR_FLAG;
    name_flags &= ~ min::BRACKET_LAB_FLAG;

    const char * prefix = "";
    const char * postfix = "";
    if ( initiator == min::NONE()
         &&
	 terminator == min::NONE() )
    {
        if ( ( gen_flags & min::BRACKET_IMPLICIT_FLAG )
	     &&
	     f->implicit_prefix != NULL
	     &&
	     f->implicit_postfix != NULL )
	{
	    prefix = f->implicit_prefix;
	    postfix = f->implicit_postfix;
	}
	out << prefix << min::save_indent;
	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    if ( i != 0 )
	    {
	        if ( separator != min::NONE() )
		    out << min::pgen
			     ( separator, name_flags );
		out << " ";
		out << min::set_break;
	    }
	    pgen ( out, vp[i],
	           subobj_gen_flags, subobj_gen_flags,
		   f );
	}
	return out << postfix << min::restore_indent;
    }
    else if ( terminator == min::NONE() )
    {
        if ( initiator == min::doublequote
	     &&
	     min::size_of ( vp ) == 1 )
	{
	    subobj_gen_flags |= min::BRACKET_STR_FLAG;
	    pgen ( out, vp[0],
	           subobj_gen_flags, subobj_gen_flags,
		   f );
	    return out;
	}
        else if ( initiator == min::number_sign
	          &&
	          min::size_of ( vp ) == 1 )
	{
	    pgen ( out, vp[0],
	           subobj_gen_flags, subobj_gen_flags,
		   f );
	    return out;
	}

	out << min::pgen ( initiator, name_flags )
	    << min::adjust_indent ( 4 )
	    << min::eol;
	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    out << min::spaces_if_before_indent
	        << min::space_if_after_indent;
	    pgen ( out, vp[i],
	           subobj_gen_flags, subobj_gen_flags,
		   f );
	}
	return out << min::adjust_indent ( -4 )
	           << min::eol_if_after_indent;
    }
    else if ( initiator == min::NONE() )
    {
	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    out << min::spaces_if_before_indent
	        << min::space_if_after_indent;
	    pgen ( out, vp[i],
	           subobj_gen_flags, subobj_gen_flags,
		   f );
	}
	if ( terminator == min::new_line )
		return out << min::eol;
	else
	    return out << min::suppressible_space
		       << min::pgen
			    ( terminator, name_flags )
		       << " ";
    }
    else if ( name == min::NONE() )
    {
	out << min::pgen ( initiator, name_flags )
	    << min::suppressible_space
	    << min::save_indent;
	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    if ( i != 0 )
	    {
	        if ( separator != min::NONE() )
		    out << min::pgen
			     ( separator, name_flags );
	        out << " ";
		out << min::set_break;
	    }
	    pgen ( out, vp[i],
	           subobj_gen_flags, subobj_gen_flags,
		   f );
	}
	return out << min::suppressible_space
	           << min::pgen
		        ( terminator, name_flags )
	           << min::restore_indent;
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

	out << min::pgen ( initiator, name_flags )
	    << min::suppressible_space
	    << min::pgen ( name, name_flags );
	if ( arguments != min::NONE()
	     ||
	     keys != min::NONE() )
	{
	    out << " " << min::save_indent;
	    if ( arguments != min::NONE() )
	    {
	    }
	    if ( keys != min::NONE() )
	    {
	    }
	    if ( middle == min::NONE() )
	    {
		return out << min::suppressible_space
			   << min::pgen
				( terminator,
				  name_flags )
			   << min::restore_indent;
	    }
	    else
		out << min::suppressible_space
		    << min::pgen ( middle, name_flags )
		    << min::restore_indent;
	}
	else if ( middle == min::NONE() )
	{
	    return out << min::suppressible_space
		       << min::pgen
			    ( terminator,
			      name_flags );
	}
	else
	    out << min::suppressible_space
		<< min::pgen ( middle, name_flags );

	out << " " << min::save_indent;

	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    if ( i != 0 )
	    {
	        if ( separator != min::NONE() )
		    out << min::pgen
			     ( separator, name_flags );
	        out << " ";
		out << min::set_break;
	    }

	    pgen ( out, vp[i],
	           subobj_gen_flags, subobj_gen_flags,
		   f );
	}

	return out << " "
	           << min::pgen
		        ( middle, name_flags )
	           << min::pgen
		        ( terminator, name_flags )
	           << min::restore_indent;
    }
}

// Execute min::pgen for output type T.
//
template < typename T >
static T pgen
	( T out,
	  min::gen v,
	  min::uns32 gen_flags,
	  min::uns32 subobj_gen_flags,
	  const min::gen_format * f,
	  T ( * pgen )
	      ( T out,
	        min::gen v,
		min::uns32 gen_flags,
		min::uns32 subobj_gen_flags,
		const min::gen_format * f ) )
{
    if ( v == min::new_stub_gen ( MINT::null_stub ) )
    {
        return
	    out << "new_stub_gen ( MINT::null_stub )";
    }
    else if ( min::is_num ( v ) )
    {
        min::float64 vf = MUP::float_of ( v );
	char buffer[128];
	sprintf ( buffer, f->number_format, vf );
	return out << buffer;
    }
    else if ( min::is_str ( v ) )
    {
        min::uns32 graphic_flags =
	    ( gen_flags & min::GRAPHIC_STR_FLAG ?
	      min::GRAPHIC_FLAGS : 0 );
        MUP::str_ptr sp ( v );
	if ( ( gen_flags & min::BRACKET_STR_FLAG )
	     &&
	     f->str_prefix != NULL
	     &&
	     f->str_postfix != NULL
	     &&
	     f->str_quote != NULL
	    )
	{
	    char * first =
	        std::strstr ( & sp[0], f->str_postfix );
	    if ( first == NULL )
		return out << f->str_prefix
			   << min::save_print_format
			   << min::set_print_flags
			       ( graphic_flags )
			   << min::begin_ptr_of ( sp )
			   << min::restore_print_format
			   << f->str_postfix;
	    else
	    {
		min::uns32 length = min::strlen ( sp );

	        MIN_STACK_COPY
		    ( char, str, length + 1, & sp[0] );

		const char * p = str;
		length = std::strlen ( f->str_postfix );

	        out << f->str_prefix;
		while ( ( first = std::strstr
			    ( p, f->str_postfix ) ) )
		{
		    * first = 0;
		    out << min::save_print_format
			<< min::set_print_flags
			     ( graphic_flags )
		        << p
		        << min::restore_print_format;
		    out << f->str_quote;
		    p = first + length;
		}

		return out << p << f->str_postfix;
	    }
	}
	else
	    return out << min::save_print_format
		       << min::set_print_flags
		              ( graphic_flags )
		       << min::begin_ptr_of ( sp )
		       << min::restore_print_format;
    }
    else if ( min::is_lab ( v ) )
    {
	const char * prefix = "";
	const char * postfix = "";
	const char * separator =
	    ( gen_flags & min::SUPPRESS_LAB_SPACE_FLAG ?
	      NULL : " " );
	if ( ( gen_flags & min::BRACKET_LAB_FLAG )
	     &&
	     f->lab_prefix != NULL
	     &&
	     f->lab_separator != NULL
	     &&
	     f->lab_postfix != NULL
	    )
	{
	    prefix = f->lab_prefix;
	    separator = f->lab_separator;
	    postfix = f->lab_postfix;
	}

	MUP::lab_ptr labp ( MUP::stub_of ( v ) );
        min::uns32 len = min::length_of ( labp );
	out << prefix;
	for ( min::unsptr i = 0; i < len; ++ i )
	{
	    if ( i != 0 )
	    {
		if ( separator == NULL)
		    out << min::suppressible_space;
		else
		    out << separator;
	    }
	    pgen ( out, labp[i],
	           gen_flags, gen_flags, f );
	}
	return out << postfix;
    }
    else if ( min::is_special ( v ) )
    {
	const char * prefix = "";
	const char * postfix = "";
	if ( ( gen_flags & min::BRACKET_SPECIAL_FLAG )
	     &&
	     f->special_prefix != NULL
	     &&
	     f->special_postfix != NULL
	    )
	{
	    prefix = f->special_prefix;
	    postfix = f->special_postfix;
	}

        min::unsgen index = MUP::special_index_of ( v );
	bool allow_special_name =
	    (    (   gen_flags
	           & min::SUPPRESS_SPECIAL_NAME_FLAG )
	      == 0 );

	if ( allow_special_name
	     &&
	         0xFFFFFF
	       - min::standard_special_names->length
	     < index
	     &&
	     index <= 0xFFFFFF )
	    return out << prefix
	        << min::standard_special_names
		            [0xFFFFFF - index]
		<< postfix;
	else if ( allow_special_name
	          &&
		  f->special_names != min::NULL_STUB
	          &&
		  index < f->special_names->length )
	    return out << prefix
	        << f->special_names[index]
		<< postfix;
	else
	{
	    char buffer[64];
	    sprintf ( buffer, "SPECIAL(0x%llx)",
		              (min::uns64) index );
	    return out << prefix << buffer << postfix;
	}
    }
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
		::pgen_exp<T>
		    ( out, ap,
		      gen_flags, subobj_gen_flags, f,
		      pgen, &info[0], c );
		return out;
	    }
	}

        if ( gen_flags & min::OBJ_ID_FLAG )
	    ::pgen_id<T> ( out, v );
	else
	    ::pgen_obj<T>
	        ( out, v,
		  gen_flags, subobj_gen_flags, f,
		  pgen );

	return out;
    }
    else if ( min::is_stub ( v ) )
    {
        const min::stub * s = MUP::stub_of ( v );
	int type = min::type_of ( s );
	const char * type_name = min::type_name[type];
	if ( type_name != NULL )
	    out << type_name;
	else
	    out << "TYPE(" << type << ")";
	return out;
    }
    else if ( min::is_list_aux ( v ) )
        return out << "LIST_AUX("
	           << MUP::list_aux_of ( v ) << ")";
    else if ( min::is_sublist_aux ( v ) )
        return out << "SUBLIST_AUX("
	           << MUP::sublist_aux_of ( v ) << ")";
    else if ( min::is_indirect_aux ( v ) )
        return out << "INDIRECT_AUX("
	           << MUP::indirect_aux_of ( v )
		   << ")";
    else if ( min::is_index ( v ) )
        return out << "INDEX("
	           << MUP::index_of ( v ) << ")";
    else if ( min::is_control_code ( v ) )
    {
	char buffer[64];
	sprintf ( buffer, "CONTROL_CODE(0x%llx)",
		          (min::uns64)
		          MUP::control_code_of ( v ) );
	return out << buffer;
    }
    else
    {
	char buffer[64];
	sprintf ( buffer, "UNDEFINED_GEN(0x%llx)",
		  (min::uns64) MUP::value_of ( v ) );
	return out << buffer;
    }

}

template < typename T >
static T flush_one_id
	( T out,
	  min::uns32 gen_flags,
	  min::uns32 subobj_gen_flags,
	  const min::gen_format * f,
	  T ( * pgen )
	      ( T out,
	        min::gen v,
		min::uns32 gen_flags,
		min::uns32 subobj_gen_flags,
		const min::gen_format * f ),
	  min::id_map id_map )
{
    if ( id_map == min::NULL_STUB
         ||
	 id_map->next >= id_map->length )
        return out;

    min::uns32 id = id_map->next;
    * ( min::uns32 * ) & id_map->next = id + 1;
    min::gen v = min::new_stub_gen ( id_map[id] );

    out << "@" << id << " = ";
    ::pgen<T> ( out, v, gen_flags, subobj_gen_flags, f,
                pgen );
    return out << min::eol;
}

min::printer min::default_pgen
	( min::printer printer,
	  min::gen v,
	  min::uns32 gen_flags,
	  min::uns32 subobj_gen_flags,
	  const min::gen_format * f )
{
    return ::pgen<min::printer>
	    ( printer, v, gen_flags, subobj_gen_flags,
	      f, f->pgen );
}

static std::ostream & ostream_pgen
	( std::ostream & out,
	  min::gen v,
	  min::uns32 gen_flags,
	  min::uns32 subobj_gen_flags,
	  const min::gen_format * f )
{
    return ::pgen<std::ostream &>
	    ( out, v, gen_flags, subobj_gen_flags, f,
	      ::ostream_pgen );
}

static min::printer flush_one_id
	( min::printer printer )
{

    min::uns32 gen_flags =
        printer->print_format.flush_gen_flags;
    min::uns32 subobj_gen_flags =
        printer->print_format.flush_subobj_gen_flags;
    const min::gen_format * f =
	printer->print_format.gen_format;
    if  ( f == NULL )
	f = & min::default_gen_format;

    min::id_map id_map = printer->id_map;

    return ::flush_one_id<min::printer>
	( printer, gen_flags, subobj_gen_flags,
          f, f->pgen, id_map );
}

static std::ostream & flush_one_id
	( std::ostream & out )
{
    min::uns32 gen_flags =
	  min::ostream_print_format.flush_gen_flags;
    min::uns32 subobj_gen_flags =
	  min::ostream_print_format
	      .flush_subobj_gen_flags;
    const min::gen_format * f =
	min::ostream_print_format.gen_format;
    if  ( f == NULL )
	f = & min::default_gen_format;

    return ::flush_one_id<std::ostream &>
	    ( out, gen_flags, subobj_gen_flags,
	      f, ::ostream_pgen, ::ostream_id_map );
}
