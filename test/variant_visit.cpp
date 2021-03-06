
// Copyright 2017 Peter Dimov.
//
// Distributed under the Boost Software License, Version 1.0.
//
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <boost/variant2/variant.hpp>
#include <boost/mp11.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/lightweight_test_trait.hpp>
#include <type_traits>
#include <utility>
#include <string>
#include <cstdio>

using namespace boost::variant2;
using boost::mp11::mp_size_t;

struct X
{
};

struct F
{
    mp_size_t<1> operator()( X& ) const;
    mp_size_t<2> operator()( X const& ) const;
    mp_size_t<3> operator()( X&& ) const;
    mp_size_t<4> operator()( X const&& ) const;
};

int main()
{
    {
        variant<int> v( 1 );

        BOOST_TEST_EQ( (visit( []( auto x ){ return x; }, v )), 1 );

        visit( []( auto x ){ BOOST_TEST_EQ( x, 1 ); }, v );
        visit( []( auto x ){ BOOST_TEST_EQ( x, 1 ); }, std::move(v) );
    }

    {
        variant<int> const v( 2 );

        BOOST_TEST_EQ( (visit( []( auto x ){ return x; }, v )), 2 );

        visit( []( auto x ){ BOOST_TEST_EQ( x, 2 ); }, v );
        visit( []( auto x ){ BOOST_TEST_EQ( x, 2 ); }, std::move(v) );
    }

    {
        variant<int const> v( 3 );

        BOOST_TEST_EQ( (visit( []( auto x ){ return x; }, v )), 3 );

        visit( []( auto x ){ BOOST_TEST_EQ( x, 3 ); }, v );
        visit( []( auto x ){ BOOST_TEST_EQ( x, 3 ); }, std::move(v) );
    }

    {
        variant<int const> const v( 4 );

        BOOST_TEST_EQ( (visit( []( auto x ){ return x; }, v )), 4 );

        visit( []( auto x ){ BOOST_TEST_EQ( x, 4 ); }, v );
        visit( []( auto x ){ BOOST_TEST_EQ( x, 4 ); }, std::move(v) );
    }

    {
        variant<int, float> v1( 1 );
        variant<int, float> const v2( 3.14f );

        BOOST_TEST_EQ( (visit( []( auto x1, auto x2 ){ return (int)(x1 * 1000) + (int)(x2 * 100); }, v1, v2 )), 1314 );

        visit( []( auto x1, auto x2 ){ BOOST_TEST_EQ( x1, 1 ); BOOST_TEST_EQ( x2, 3.14f ); }, v1, v2 );
        visit( []( auto x1, auto x2 ){ BOOST_TEST_EQ( x1, 1 ); BOOST_TEST_EQ( x2, 3.14f ); }, std::move(v1), std::move(v2) );
    }

    {
        variant<int, float, double> v1( 1 );
        variant<int, float, double> const v2( 3.14f );
        variant<int, float, double> v3( 6.28 );

        BOOST_TEST_EQ( (visit( []( auto x1, auto x2, auto x3 ){ return (int)(x1 * 100) * 1000000 + (int)(x2 * 100) * 1000 + (int)(x3 * 100); }, v1, v2, v3 )), 100314628 );

        visit( []( auto x1, auto x2, auto x3 ){ BOOST_TEST_EQ( x1, 1 ); BOOST_TEST_EQ( x2, 3.14f ); BOOST_TEST_EQ( x3, 6.28 ); }, v1, v2, v3 );
        visit( []( auto x1, auto x2, auto x3 ){ BOOST_TEST_EQ( x1, 1 ); BOOST_TEST_EQ( x2, 3.14f ); BOOST_TEST_EQ( x3, 6.28 ); }, std::move(v1), std::move(v2), std::move(v3) );
    }

    {
        variant<int, float, double, char> v1( 1 );
        variant<int, float, double, char> const v2( 3.14f );
        variant<int, float, double, char> v3( 6.28 );
        variant<int, float, double, char> const v4( 'A' );

        BOOST_TEST_EQ( (visit( []( auto x1, auto x2, auto x3, auto x4 ){ return (long long)(x1 * 100) * 100000000 + (long long)(x2 * 100) * 100000 + (long long)(x3 * 10000) + (int)x4; }, v1, v2, v3, v4 )), 10031462800 + 'A' );

        visit( []( auto x1, auto x2, auto x3, auto x4 ){ BOOST_TEST_EQ( x1, 1 ); BOOST_TEST_EQ( x2, 3.14f ); BOOST_TEST_EQ( x3, 6.28 ); BOOST_TEST_EQ( x4, 'A' ); }, v1, v2, v3, v4 );
        visit( []( auto x1, auto x2, auto x3, auto x4 ){ BOOST_TEST_EQ( x1, 1 ); BOOST_TEST_EQ( x2, 3.14f ); BOOST_TEST_EQ( x3, 6.28 ); BOOST_TEST_EQ( x4, 'A' ); }, std::move(v1), std::move(v2), std::move(v3), std::move(v4) );
    }

    {
        variant<X> v;
        variant<X> const cv;

        BOOST_TEST_EQ( decltype(visit(F{}, v))::value, 1 );
        BOOST_TEST_EQ( decltype(visit(F{}, cv))::value, 2 );
        BOOST_TEST_EQ( decltype(visit(F{}, std::move(v)))::value, 3 );
        BOOST_TEST_EQ( decltype(visit(F{}, std::move(cv)))::value, 4 );
    }

    return boost::report_errors();
}
