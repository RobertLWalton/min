# include <iostream>
using namespace std;

template < typename T >
class A;

template < typename T >
class B
{
public:

    class C;

    typedef A<T> D;
    friend class A<T>;
};


template < typename T >
void funcAvoid ( A<T> & c );

template < typename T >
int funcAint ( A<T> & c );

template < typename T >
void funcBvoid ( B<T> & c );

template < typename T >
int funcBint ( B<T> & c );

template < typename T >
void funcCvoid ( B<T>::C * c );

template < typename T >
int funcCint ( B<T>::C * c );

template < typename T >
void funcDvoid ( B<T>::D * d );

template < typename T >
int funcDint ( B<T>::D * d );

int main ()
{
    cout << "HELLO" << endl;
}
