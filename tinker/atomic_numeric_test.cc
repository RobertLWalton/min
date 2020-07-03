// Atomic Numeric Test Program
//
// File:	atomic_numeric_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Nov 28 06:15:58 EST 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// A character string X is called a representative of an
// IEEE 64 bit floating point number x if strtod(3)
// converts X to x.
//
// A minimal representative is a representative with a
// minimal number of characters.
//
// This program computes minimal representatives.
// The standard input consists of lines of one of the
// forms
//
// 	x =
// 	x + y =
// 	x - y =
// 	x * y =
// 	x / y =
// 	x % y =
//
// Here x and y are floating point numbers.
//
// For each input line, this program outputs one line
// that is a copy of the input line followed by the
// minimum representative of the computed number.
//
// E.g.,
//
//     0.01 = 0.01
//     0.01 + 0.20 = 0.21
//     0.01 - 0.20 = -0.19
//     0.01 * 0.20 = 0.002
//     0.01 / 0.20 = 0.05
//     0.25 % 0.04 = 0.01
//
// Compile with g++ -c atomic_numeric_test.cc

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::ws;
using std::istringstream;
using std::fpclassify;

const char * precise_sprint
	( char * buffer, double v )
{
    assert ( sizeof ( v ) == 8 );

    int fclass = fpclassify ( v );
    if ( fclass != FP_NORMAL
         &&
	 fclass != FP_SUBNORMAL )
    {
	sprintf ( buffer, "%.2g", v );
	return buffer;
    }

    int sign = +1;
    if ( v < 0 ) v = - v, sign = - sign;

    int exp;
    double fv = frexp ( v, & exp );
    long long iv = (long long) ldexp ( fv, 60 );
    exp -= 60;

    // IEEE 64 bit floating point has max exponent of
    // 308.
    //
    char big_buffer[1000];
    char * first = big_buffer + 500;

    // Put iv in big_buffer as an integer.
    //
    sprintf ( first, "%ld", iv );
    char * last = first + strlen ( first );
    char * units = last - 1;

    // Multiply number in big_buffer by 2**exp
    // if exp > 0.
    //
    while ( exp > 0 )
    {
	char * p = last;
	int d = 0;
	while ( first < p )
	{
	    d += 2 * ( ( * -- p ) - '0' );
	    * p = '0' + d % 10;
	    d /= 10;
	}
	if ( d != 0 ) * -- first = '0' + d;
	-- exp;
	while ( first < last && last[-1] == '0' )
	    -- last;
    }

    // Divide number in big_buffer by 2**(-exp)
    // if exp < 0.
    //
    while ( exp < 0 )
    {
	char * p = first;
	int d = 0;
	while ( p < last )
	{
	    d += ( * p ) - '0';
	    * p ++ = ( d / 2 ) + '0';
	    d = ( d % 2 ) * 10;
	}
	if ( d != 0 ) * last ++ = ( d / 2 ) + '0';
	++ exp;
	while ( first < last && * first == '0' )
	    ++ first;
    }
    assert ( first < last );

    // Keep at most 18 significant digits.
    //
    if ( last > first + 18 ) last = first + 18;
    * last = 0;

    buffer[0] = ( sign == -1 ? '-' : '+' );
    buffer[1] =  * first;
    char * p = buffer + 2;
    if ( first + 1 < last )
    {
        * p ++ = '.';
	strcpy ( p, first + 1 );
	p += last - first - 1;
    }
    else * p = 0;

    int dexp = units - first;

    if ( dexp != 0 )
        sprintf ( p, "E%+d", dexp );

    return buffer;
}

// Input Data
//
string line;
double x, y, z;
char op, equal;

int main ( int argc, const char * argv[] )
{
    while ( getline ( cin, line ) )
    {
	istringstream in ( line );
	in >> x >> ws >> op >> ws;
	if ( op != '=' )
	    in >> y >> ws >> equal >> ws;
	else if ( in.eof() )
	    equal = '=';
	else
	    equal = 0;

	if ( ! in.eof()
	     ||
	     equal != '=' )
	{
	    cout << line << " BADLY FORMATTED" << endl;
	    continue;
	}

	switch ( op )
	{
	case '=':	z = x; break;
	case '+':	z = x + y; break;
	case '-':	z = x - y; break;
	case '*':	z = x * y; break;
	case '/':	z = x / y; break;
	case '%':	z = fmod ( x, y ); break;
	default:
	    cout << line << " HAS BAD OPERATOR: "
	         << op << endl;
	    continue;
	}

	char buffer[100];
	precise_sprint ( buffer, z );

	cout << line << ' ' << buffer;

	sprintf ( buffer, "%.15g", z );
	cout << " = " << buffer << endl;

	// Convert %.15g representation back to a
	// double using strtod and check if it matches
	// original z.
	//
	char * p = buffer;
	double z2 = strtod ( buffer, & p );
	assert ( * p == 0 );
	if ( z != z2 )
	{
	    precise_sprint ( buffer, z2 );
	    cout << "    ~~  " << buffer;

	    sprintf ( buffer, "%.15g", z2 );
	    cout << " = " << buffer << endl;
	}
    }
    
    return 0;
}
