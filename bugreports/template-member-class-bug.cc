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

// These are OK:

    template < typename T >
    void funcAvoid ( A<T> * a );

    template < typename T >
    int funcAint ( A<T> * a );

    template < typename T >
    void funcBvoid ( B<T> * b );

    template < typename T >
    int funcBint ( B<T> * b );

// These incorrectly fail:

    template < typename T >
    void funcCvoid ( B<T>::C * c );

    template < typename T >
    int funcCint ( B<T>::C * c );

    template < typename T >
    void funcDvoid ( B<T>::D * d );

    template < typename T >
    int funcDint ( B<T>::D * d );

// The following checks the non-template version:

class E;
class F
{
public:
    class G;
};

// These non-template versions are OK:

    void funcGvoid ( F::G * g );
    int funcGint ( F::G * g );

int main ()
{
    cout << "HELLO" << endl;
}
