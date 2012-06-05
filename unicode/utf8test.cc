// Compile with:
//	g++ -o utf8test utf8test.cc -lfontconfig

#include <iostream>
#include <iomanip>
#include <cstdlib>
extern "C" {
#include <fontconfig/fontconfig.h>
}
using std::cout;
using std::endl;
using std::hex;

int main ( int argc, const char ** argv )
{
    unsigned char buffer[4000];
    unsigned char * p = buffer;
    while ( argc > 1 )
    {
	unsigned c = strtol ( argv[1], NULL, 16 );
	cout << hex << c << " ";
	p += FcUcs4ToUtf8 ( c, p );
	-- argc;
	++ argv;
    }
    * p = 0;
    cout << buffer << endl;
    return 0;
}
