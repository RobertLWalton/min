// MIN OS Interface Test Program
//
// File:	min_os_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Thu Jun  4 20:35:56 EDT 2009
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2009/06/05 07:26:59 $
//   $RCSfile: min_os_test.cc,v $
//   $Revision: 1.1 $

// Table of Contents:
//
//	Setup
//	Memory Management Tests

// Setup
// -----

// Note: we include min_os.cc because we want to use
// its static functions and data.
//
# include "../src/min_os.cc"

MINT::initializer::initializer ( void ) {}

int main ( )
{
    cout << "Start min_os Test" << endl;

// Memory Management Tests
// ------ ---------- -----

    read_used_pools();
    dump_used_pools();


// Finish
// ------

    cout << "Finish min_os Test" << endl;
    return 0;
}
