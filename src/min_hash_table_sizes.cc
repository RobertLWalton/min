// MIN Language Computation of Possible Hash Table Sizes
//
// File:	min_hash_table_sizes.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Apr 29 08:08:27 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/04/29 15:18:43 $
//   $RCSfile: min_hash_table_sizes.cc,v $
//   $Revision: 1.6 $

# include <iostream>
# include <iomanip>
# include <cassert>
using std::cout;
using std::endl;
using std::setw;

// We compute and output the elements of the
// MINT::hash_table vector.

int main ( )
{

    unsigned count = 0;
    unsigned increment = 1;
    unsigned value = 0;

    for ( ; ; ++ count )
    {
	if ( count % 16 == 0 )
	    cout << "// [" << count
		 << " .. " << count + 15
		 << "]" << endl;

	if ( count == 1 )
	    increment = 2;
	else if ( count == 8 )
	    increment = 4;
	else if ( increment + 2 <= value/100 )
	{
	    increment = value/100;
	    if ( increment % 2 != 0 ) -- increment;
	}

	cout << setw(11) << value;

	if ( value == (unsigned) -1 ) break;

	unsigned next = value + increment;
	if ( next < value ) next = (unsigned) -1;

	cout << ",";

        if ( count % 4 == 3 )
	    cout << endl;


	value = next;
    }

    cout << endl;

    return 0;
}
