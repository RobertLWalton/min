// MIN Language Computation of Possible Hash Table Sizes
//
// File:	min_hash_table_sizes.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Apr 29 06:44:00 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/04/29 12:06:14 $
//   $RCSfile: min_hash_table_sizes.cc,v $
//   $Revision: 1.4 $

# include <iostream>
# include <iomanip>
# include <cassert>
using std::cout;
using std::endl;
using std::setw;

// We compute and output the settings of the
// MUP::hash_table_size vector.

// Maximum size of hash table + 1.  Must be <= 2**31.
//
const unsigned SIZE = ( 1 << 31 );

// Number of bits in hash code.
//
const unsigned BITS = 10;

// We compute a prime sieve for numbers up to SIZE - 1.
//
unsigned char sieve[SIZE >> 3];
inline bool is_prime ( unsigned i )
{
    return ( sieve[i>>3] & ( 1<<(i&7) ) ) == 0;
}
inline bool set_non_prime ( unsigned i )
{
    sieve[i>>3] |= ( 1<<(i&7) );
}

int main ( )
{
    cout << "// Except for the first 2 values, these"
         << endl
         << "// sizes are primes chosen so that none"
	 << endl
         << "// is greater than 105% of the previous"
         << endl
         << "// one, where possible."
         << endl
         << "//"
         << endl;

    cout << "// [" << 0
	 << " .. " << 15
	 << "]" << endl;
    cout << setw(11) << 0;

    unsigned count = 1;	   // Number of numbers output.
    unsigned last = 0;	   // Last number output.
    unsigned previous = 1; // Previous number that might
    			   // be output (prime or == 1).
    unsigned current = 2;  // Current number that might
    			   // be output if it is prime.
    //
    for ( ; current < SIZE; ++ current )
    {
	if ( ! is_prime ( current ) ) continue;

	if ( current < ( 1 << 16 ) )
	    for ( unsigned i = current * current;
	          i < SIZE;
		  i += current )
	        set_non_prime ( i );

	if ( current > last + last / 20 )
	{
	    if ( count > 0 ) cout << ",";
	    if ( count % 4 == 0 ) cout << endl;
	    if ( count % 16 == 0 )
	        cout << "// [" << count
		     << " .. " << count + 15
		     << "]" << endl;
	    cout << setw(11) << previous;
	    last = previous;
	    ++ count;
	}
	previous = current;
    }

    cout << endl;

    return 0;
}
