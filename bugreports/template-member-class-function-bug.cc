# include <iostream>
using namespace std;

template < typename T >
class B
{
public:
    class C;
};

template < typename T >
class B<T>::C { int i; };

// The following definition is accepted BUT then
// appears to be `forgotten' so it cannot be used.
//
template < typename T >
int funcC ( typename::B<T>::C * c ) { return 0; }

// Inclusion of the following makes no difference in
// the complier error message.
//
template int funcC<int> ( B<int>::C * c);

template < typename T >
int funcB ( B<T> * b ) { return 0; }

// Check that the non-template version works.
//
class F
{
public:
    class G;
};

class F::G { int j; };

void funcG ( F::G * g );

// Workaround for my case.

template < typename T >
class C2 { int k; };

template < typename T >
class B2
{
public:
    typedef C2<T> C;
};
template < typename T >
int funcC2 ( C2<T> * c ) { return 0; }

int main ()
{
    B<int>::C x;	// OK
    funcC ( & x );	// COMPILER ERROR!
    F::G y;		// OK
    funcG ( & y );	// OK
    B<int> z;		// OK
    funcB ( & z );	// OK
    B2<int>::C w;	// OK
    funcC2 ( & w );	// OK
}
