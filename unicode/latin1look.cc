// Program to convert hex character codes to/from
// latin1 characters.
//
// File:	latin1look.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jul 23 06:47:08 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

#include <iostream>
#include <iomanip>
#include <cstdlib>
using std::cout;
using std::endl;
using std::hex;

int main ( int argc, const char ** argv )
{
    if ( argc == 1 )
    {
        cout << "latin1look HEXCODE ...\n"
	        "    Outputs a character string"
		" containing the Latin-1 encoded"
		" versions\n"
		"    of the characters whose"
		" hexadecimal codes are given.\n"
		"\n"
		"latin1look STRING\n"
		"    Outputs the hexadecimal codes"
		" of the Latin-1 characters in STRING."
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
    	// Latin1 to hex.
	//
	const unsigned char * p =
	    (const unsigned char *) argv[1];
	bool first = true;
	while ( * p != 0 )
        {
	    unsigned c = * p ++;

	    if ( first ) first = false;
	    else cout << " ";

	    cout << hex << c;
	}
	cout << endl;
    }
    else
    {
    	// Hex to Latin-1.

	unsigned char buffer[4000];
	unsigned char * p = buffer;
	while ( argc > 1 )
	{
	    unsigned c = strtol ( argv[1], NULL, 16 );
	    * p ++ = c;
	    -- argc;
	    ++ argv;
	}
	* p = 0;
	cout << buffer << endl;
    }

    return 0;
}
