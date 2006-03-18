// MIN Language Computation of Possible Hash Table Sizes
//
// File:	min_hash_table_sizes.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Sat Mar 18 12:41:34 EST 2006
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2006/03/18 17:48:06 $
//   $RCSfile: min_hash_table_sizes_prime.cc,v $
//   $Revision: 1.1 $

# include <iostream>
# include <iomanip>
using std::cout;
using std::endl;
using std::setw;

// We compute and output the settings of the
// MUP::hash_table_size vector.

const int limit = 20000000;
bool sieve[limit];

int main ( )
{
    cout << "// These sizes are primes chosen so that"
	 << endl;
    cout << "// none is greater than 105% of the"
         << endl;
    cout << "// previous one, where possible."
         << endl;
    cout << "//"
         << endl;

    int last = 0;
    int previous = 0;
    int count = 0;
    for ( int i = 2; i < limit && count < 256; ++ i )
    {
        if ( sieve[i] ) continue;
	
	for ( int j = i; j < limit; j += i )
	    sieve[j] = true;

	if ( i > 21 * last / 20 && previous != 0 )
	{
	    if ( count > 0 ) cout << ",";
	    if ( count % 4 == 0 ) cout << endl;
	    cout << setw(9) << previous;
	    last = previous;
	    ++ count;
	}
	previous = i;
    }
    cout << endl;

    return 0;
}
