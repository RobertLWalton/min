// MIN_ASSERT code.
//
// File:	min_assert.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu May 28 21:49:21 EDT 2015
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
# include <cstdarg>


// Assert
// ------

bool min::assert_print = false;
bool min::assert_throw = false;
bool min::assert_abort = true;

void min::standard_assert
    ( bool value,
      const char * expression,
      const char * file_name, unsigned line_number,
      const char * function_name,
      const char * message_format, ... )
{
    if (    min::assert_print
         || ( ! min::assert_throw && ! value ) )
    {
	printf ( "%s:%d:\n", file_name, line_number );
	if ( function_name != NULL )
	    printf ( "    in %s:\n", function_name );
	if ( expression == NULL )
	    printf ( "     abort\n" );
	else
	    printf ( "    %s => %s\n",
	             expression,
	             ( value ? "true" : "false" ) );
	if ( message_format != NULL )
	{
	    va_list ap;
	    va_start ( ap, message_format );
	    printf ( "    " );
	    vprintf ( message_format, ap );
	    printf ( "\n" );
	}
    }

    if ( ! value )
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
