// MIN_ASSERT code.
//
// File:	min_assert.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jan  8 07:40:43 EST 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Setup
//	Assert

// Setup
// -----

# include <min_parameters.h>
# include <cstdlib>
# include <cstdio>


// Assert
// ------

bool min::assert_print = true;
bool min::assert_throw = false;
bool min::assert_abort = false;

void min::standard_assert
    ( bool value,
      const char * expression,
      const char * file_name, unsigned line_number,
      const char * function_name,
      const char * message_format, ... )
{
    if (    min::assert_print
         || ( ! min::assert_throw
	      &&
	      ( ! value || expression == NULL ) ) )
    {
	printf ( "%s:%d", file_name, line_number );
	if ( expression == NULL )
	    printf ( " abort\n" );
	else
	    printf ( " assert:\n" );
	    printf ( "    %s %s.\n",
	             expression,
	             ( value ? "true" : "false" ) );
    }

    if ( ! value || expression == NULL )
    {
         if ( min::assert_throw )
	     throw ( new min::assert_exception );
	 if ( min::assert_abort || expression == NULL )
	     abort();
    }
}

void ( * min::assert_hook )
    ( bool value,
      const char * expression,
      const char * file_name, unsigned line_number,
      const char * function_name,
      const char * message_format, ... )
   = min::standard_assert;
