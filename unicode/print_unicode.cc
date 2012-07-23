// Program to Print Unicode Characters of a given class.
//
// File:	print_unicode.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Jul 22 07:16:41 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include "unicode_class.h"

# include <iostream>
# include <iomanip>
# include <cstdlib>
extern "C" {
# include <fontconfig/fontconfig.h>
}
using std::cout;
using std::endl;
using std::hex;

static struct class_info {
    char class_char;
    const char * class_name;
} classes[] = { UNICODE_CLASSES, { 0, NULL } };

// Return class name of given class.
//
const char * find ( char class_char )
{
    for ( class_info * p = classes;
          p->class_name != NULL; ++ p )
    {
        if ( p->class_char == class_char )
	    return p->class_name;
    }
    return "UNKNOWN CLASS";
}

const unsigned unicode_class_size =
    UNICODE_CLASS_SIZE;
char unicode_class[UNICODE_CLASS_SIZE+1] =
    UNICODE_CLASS;


int main ( int argc, const char ** argv )
{
    if (    argc == 2
         && strcmp ( argv[1], "classes" ) == 0 )
    {
        for ( class_info * p = classes;
	      p->class_name != NULL; ++ p )
	    cout << "    " << p->class_char
	         << "   " << p->class_name << endl;
	exit ( 0 );
    }
    else if ( argc != 3 && argc != 4 )
    {
        cout << "print_unicode FIRST-LAST [CLASS]\n"
	        "\n"
                "Prints all the characters whose"
		" character codes are\n"
                "in the range FIRST-LAST.  If CLASS"
		" is given, only characters\n"
                "with the given CLASS are printed. "
		" Here CLASS is the\n"
		"unicode class letter, and FIRST and"
		" LAST are hexadecimal integers.\n";
	exit ( 1 );
    }

    unsigned FIRST = strtol ( argv[1], NULL, 16 );
    unsigned LAST = strtol ( argv[2], NULL, 16 );
    char CLASS = (argc == 4 ? argv[1][0] : 0 );

    for ( unsigned c = FIRST; c <= LAST; ++ c )
    {
        if ( c >= unicode_class_size ) continue;
	char class_char = unicode_class[c];
	if ( CLASS != 0 && class_char != CLASS )
	    continue;

	unsigned char buffer[4000];
	unsigned char * p = buffer;
	p += FcUcs4ToUtf8 ( c, p );
	* p = 0;

	cout << hex << c << " " << buffer
	     << " " << class_char
	     << " " << find ( class_char )
	     << endl;
    }
    return 0;
}
