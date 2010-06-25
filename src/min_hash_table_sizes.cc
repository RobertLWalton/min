// MIN Language Computation of Possible Hash Table Sizes
//
// File:	min_hash_table_sizes.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan 26 00:51:43 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/06/25 12:36:12 $
//   $RCSfile: min_hash_table_sizes.cc,v $
//   $Revision: 1.8 $

# include <iostream>
# include <iomanip>
# include <cmath>
# include <cassert>
using std::cout;
using std::endl;
using std::setw;
using std::setprecision;
using std::ios;

// We compute and output the elements of the
// MINT::hash_table vector.
//
//	hash_table[0] = 0
//	hash_table[C] = max ( hash_table[C-1] + 1,
//			      round ( 2**(C/100))
//		        if C > 0
//
// 0 <= C < HSIZE where
//
//	round ( 2**(HSIZE/100 ) >= 2^32
//
// i.e., the hash_table is as large as possible
// while still having its elements fit into uns32
// values.

int main ( )
{

    cout << setiosflags ( ios::fixed );
    cout << setprecision (0);

    double C;
    double previous_value;
    double ln2 = log ( 2.0 );

    // Test that all powers of 2 are covered.
    // K is max integer such that all values
    // 2**I for 0 <= I <= K are present.
    //
    int K = -1;
    double two_to_the_next_K = 1;

    for ( C = 0; ; ++ C )
    {
	double value;
	if ( C == 0 ) value = 0;
	else
	{
	    value = round ( exp ( ln2 * C / 100 ) );
	    if ( value < previous_value + 1 )
	        value = previous_value + 1;
	}
	previous_value = value;

	if ( value > 0xFFFFFFFFu ) break;

	if ( value == two_to_the_next_K )
	    ++ K, two_to_the_next_K *= 2;

	if ( C != 0 )
	{
	    cout << ",";
	    if ( fmod ( C, 4 ) == 0 )
	        cout << endl;
	}

	if ( fmod ( C, 16 ) == 0 )
	    cout << "// [" << C
		 << " .. " << C + 15
		 << "]" << endl;

	cout << setw(11) << value;
    }

    cout << endl;

    assert ( K >= 31 );

    return 0;
}
