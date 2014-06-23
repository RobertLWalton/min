// Program to convert hex character codes to/from
// unicode characters.
//
// File:	utf8look.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun 16 15:23:45 EDT 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Compile with:
//	g++ -o utf8look utf8look.cc -lfontconfig

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
extern "C" {
#include <fontconfig/fontconfig.h>
}
using std::cout;
using std::endl;
using std::hex;

int main ( int argc, const char ** argv )
{
    if ( argc == 1 )
    {
        cout << "utf8look HEXCODE ...\n"
	        "    Outputs a character string"
		" containing the UTF-8 encoded"
		" versions\n"
		"    of the characters whose"
		" hexadecimal codes are given.\n"
		"\n"
		"utf8look STRING\n"
		"    Outputs the hexadecimal codes"
		" of the UTF-8 characters in STRING."
	     << endl;
	exit ( 1 );
    }

    // Test if there is just one argument and it is NOT
    // a hexadecimal number.
    //
    char * p = NULL;
    strtol ( argv[1], & p, 16 );
    if ( argc == 2 && * p != 0 )
    {
    	// UTF-8 to hex.
	//
	const unsigned char * p =
	    (const unsigned char *) argv[1];
	unsigned length = strlen ( argv[1] );
	bool first = true;
	while ( * p != 0 )
        {
	    unsigned c;
	    unsigned clength =
	        FcUtf8ToUcs4 ( p, & c, length );

	    if ( first ) first = false;
	    else cout << " ";

	    cout << hex << c;

	    length -= clength;
	    p += clength;
	}
	cout << endl;
    }
    else
    {
    	// Hex to UTF-8.

	unsigned char buffer[4000];
	unsigned char * p = buffer;
	while ( argc > 1 )
	{
	    unsigned c = strtol ( argv[1], NULL, 16 );
	    p += FcUcs4ToUtf8 ( c, p );
	    -- argc;
	    ++ argv;
	}
	* p = 0;
	cout << buffer << endl;
    }

    return 0;
}
