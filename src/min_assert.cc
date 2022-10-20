// MIN_ASSERT code.
//
// File:	min_assert.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Oct 20 02:56:03 EDT 2022
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
extern "C" {
# include <sys/prctl.h>
}


// Assert
// ------

bool min::assert_print = false;
bool min::assert_throw = false;
bool min::assert_abort = true;
FILE * min::assert_err = stderr;

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
	fprintf ( assert_err, "%s: %s:%d:\n",
                  expression == NULL ? "ABORT" :
		  value ? "ASSERT SUCCEEDED" :
		          "ASSERT FAILED",
	          file_name, line_number );
	if ( function_name != NULL )
	    fprintf ( assert_err, "    in %s:\n",
	              function_name );
	if ( expression != NULL )
	    fprintf ( assert_err, "    %s => %s\n",
	             expression,
	             ( value ? "true" : "false" ) );
	if ( message_format != NULL )
	{
	    va_list ap;
	    va_start ( ap, message_format );
	    fprintf ( assert_err, "    " );
	    vfprintf ( assert_err, message_format, ap );
	    fprintf ( assert_err, "\n" );
	}
    }

    if ( ! value )
    {
        if ( min::assert_throw )
	    throw ( new min::assert_exception );
	if ( min::assert_abort || expression == NULL )
	{
	    // prctl ( PR_SET_DUMPABLE, 0, 0, 0 );
	        // Added because even though corefile
		// size set to 0, large core file was
		// being piped to abrt-hook on Fedora.
		//
		// Removed because abort() no longer
		// keeps core for gdb.
	    fprintf ( assert_err, "    ABORTING"
	              " (core dump disabled)\n" );
	    abort();
	}
    }
}

void ( * min::assert_hook )
    ( bool value,
      const char * expression,
      const char * file_name, unsigned line_number,
      const char * function_name,
      const char * message_format, ... )
   = min::standard_assert;
