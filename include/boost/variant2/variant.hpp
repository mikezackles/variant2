#ifndef BOOST_VARIANT2_VARIANT_HPP_INCLUDED
#define BOOST_VARIANT2_VARIANT_HPP_INCLUDED

//  Copyright 2017 Peter Dimov.
//
//  Distributed under the Boost Software License, Version 1.0.
//
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_MP11_HPP_INCLUDED
#include <boost/mp11.hpp>
#endif
#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <cstddef>
#include <type_traits>
#include <exception>
#include <cassert>
#include <initializer_list>
#include <utility>

//

namespace boost
{
namespace variant2
{

using namespace boost::mp11;

// bad_variant_access

class bad_variant_access: public std::exception
{
public:

    bad_variant_access() noexcept
    {
    }

    char const * what() const noexcept
    {
        return "bad_variant_access";
    }
};

// monostate

struct monostate
{
};

constexpr bool operator<(monostate, monostate) noexcept { return false; }
constexpr bool operator>(monostate, monostate) noexcept { return false; }
constexpr bool operator<=(monostate, monostate) noexcept { return true; }
constexpr bool operator>=(monostate, monostate) noexcept { return true; }
constexpr bool operator==(monostate, monostate) noexcept { return true; }
constexpr bool operator!=(monostate, monostate) noexcept { return false; }

// valueless

struct valueless
{
};

constexpr bool operator<(valueless, valueless) noexcept { return false; }
constexpr bool operator>(valueless, valueless) noexcept { return false; }
constexpr bool operator<=(valueless, valueless) noexcept { return true; }
constexpr bool operator>=(valueless, valueless) noexcept { return true; }
constexpr bool operator==(valueless, valueless) noexcept { return true; }
constexpr bool operator!=(valueless, valueless) noexcept { return false; }

// variant forward declaration

template<class... T> class variant;

// variant_size

template<class T> struct variant_size
{
};

template<class T> struct variant_size<T const>: variant_size<T>
{
};

template<class T> struct variant_size<T volatile>: variant_size<T>
{
};

template<class T> struct variant_size<T const volatile>: variant_size<T>
{
};

template <class T> /*inline*/ constexpr std::size_t variant_size_v = variant_size<T>::value;

template <class... T> struct variant_size<variant<T...>>: mp_size<variant<T...>>
{
};

// variant_alternative

template<std::size_t I, class T> struct variant_alternative;

template<std::size_t I, class T> using variant_alternative_t = typename variant_alternative<I, T>::type;

namespace detail
{
    template<class I, class T, class Q> using var_alt_impl = mp_invoke<Q, variant_alternative_t<I::value, T>>;
} // namespace detail

template<std::size_t I, class T> struct variant_alternative
{
};

template<std::size_t I, class T> struct variant_alternative<I, T const>: mp_defer< variant2::detail::var_alt_impl, mp_size_t<I>, T, mp_quote<std::add_const_t>>
{
};

template<std::size_t I, class T> struct variant_alternative<I, T volatile>: mp_defer< variant2::detail::var_alt_impl, mp_size_t<I>, T, mp_quote<std::add_volatile_t>>
{
};

template<std::size_t I, class T> struct variant_alternative<I, T const volatile>: mp_defer< variant2::detail::var_alt_impl, mp_size_t<I>, T, mp_quote<std::add_cv_t>>
{
};

template<std::size_t I, class... T> struct variant_alternative<I, variant<T...>>: mp_defer<mp_at, variant<T...>, mp_size_t<I>>
{
};

// holds_alternative

template<class U, class... T> constexpr bool holds_alternative( variant<T...> const& v ) noexcept
{
    static_assert( mp_count<variant<T...>, U>::value == 1, "The type must occur exactly once in the list of variant alternatives" );
    return v.index() == mp_find<variant<T...>, U>::value;
}

// get (index)

template<std::size_t I, class... T> constexpr variant_alternative_t<I, variant<T...>>& get(variant<T...>& v)
{
    static_assert( I < sizeof...(T), "Index out of bounds" );

#if BOOST_WORKAROUND( BOOST_GCC, < 60000 )

    return (void)( v.index() != I? throw bad_variant_access(): 0 ), v._get_impl( mp_size_t<I>() );

#else

    if( v.index() != I ) throw bad_variant_access();
    return v._get_impl( mp_size_t<I>() );

#endif
}

template<std::size_t I, class... T> constexpr variant_alternative_t<I, variant<T...>>&& get(variant<T...>&& v)
{
    static_assert( I < sizeof...(T), "Index out of bounds" );

#if BOOST_WORKAROUND( BOOST_GCC, < 60000 )

    return (void)( v.index() != I? throw bad_variant_access(): 0 ), std::move( v._get_impl( mp_size_t<I>() ) );

#else

    if( v.index() != I ) throw bad_variant_access();
    return std::move( v._get_impl( mp_size_t<I>() ) );

#endif
}

template<std::size_t I, class... T> constexpr variant_alternative_t<I, variant<T...>> const& get(variant<T...> const& v)
{
    static_assert( I < sizeof...(T), "Index out of bounds" );

#if BOOST_WORKAROUND( BOOST_GCC, < 60000 )

    return (void)( v.index() != I? throw bad_variant_access(): 0 ), v._get_impl( mp_size_t<I>() );

#else

    if( v.index() != I ) throw bad_variant_access();
    return v._get_impl( mp_size_t<I>() );

#endif
}

template<std::size_t I, class... T> constexpr variant_alternative_t<I, variant<T...>> const&& get(variant<T...> const&& v)
{
    static_assert( I < sizeof...(T), "Index out of bounds" );

#if BOOST_WORKAROUND( BOOST_GCC, < 60000 )

    return (void)( v.index() != I? throw bad_variant_access(): 0 ), std::move( v._get_impl( mp_size_t<I>() ) );

#else

    if( v.index() != I ) throw bad_variant_access();
    return std::move( v._get_impl( mp_size_t<I>() ) );

#endif
}

// get (type)

template<class U, class... T> constexpr U& get(variant<T...>& v)
{
    static_assert( mp_count<variant<T...>, U>::value == 1, "The type must occur exactly once in the list of variant alternatives" );
    constexpr auto I = mp_find<variant<T...>, U>::value;

#if BOOST_WORKAROUND( BOOST_GCC, < 60000 )

    return (void)( v.index() != I? throw bad_variant_access(): 0 ), v._get_impl( mp_size_t<I>() );

#else

    if( v.index() != I ) throw bad_variant_access();
    return v._get_impl( mp_size_t<I>() );

#endif
}

template<class U, class... T> constexpr U&& get(variant<T...>&& v)
{
    static_assert( mp_count<variant<T...>, U>::value == 1, "The type must occur exactly once in the list of variant alternatives" );
    constexpr auto I = mp_find<variant<T...>, U>::value;

#if BOOST_WORKAROUND( BOOST_GCC, < 60000 )

    return (void)( v.index() != I? throw bad_variant_access(): 0 ), std::move( v._get_impl( mp_size_t<I>() ) );

#else

    if( v.index() != I ) throw bad_variant_access();
    return std::move( v._get_impl( mp_size_t<I>() ) );

#endif
}

template<class U, class... T> constexpr U const& get(variant<T...> const& v)
{
    static_assert( mp_count<variant<T...>, U>::value == 1, "The type must occur exactly once in the list of variant alternatives" );
    constexpr auto I = mp_find<variant<T...>, U>::value;

#if BOOST_WORKAROUND( BOOST_GCC, < 60000 )

    return (void)( v.index() != I? throw bad_variant_access(): 0 ), v._get_impl( mp_size_t<I>() );

#else

    if( v.index() != I ) throw bad_variant_access();
    return v._get_impl( mp_size_t<I>() );

#endif
}

template<class U, class... T> constexpr U const&& get(variant<T...> const&& v)
{
    static_assert( mp_count<variant<T...>, U>::value == 1, "The type must occur exactly once in the list of variant alternatives" );
    constexpr auto I = mp_find<variant<T...>, U>::value;

#if BOOST_WORKAROUND( BOOST_GCC, < 60000 )

    return (void)( v.index() != I? throw bad_variant_access(): 0 ), std::move( v._get_impl( mp_size_t<I>() ) );

#else

    if( v.index() != I ) throw bad_variant_access();
    return std::move( v._get_impl( mp_size_t<I>() ) );

#endif
}

// get_if

template<std::size_t I, class... T> constexpr std::add_pointer_t<variant_alternative_t<I, variant<T...>>> get_if(variant<T...>* v) noexcept
{
    static_assert( I < sizeof...(T), "Index out of bounds" );
    return v && v->index() == I? &v->_get_impl( mp_size_t<I>() ): 0;
}

template<std::size_t I, class... T> constexpr std::add_pointer_t<const variant_alternative_t<I, variant<T...>>> get_if(variant<T...> const * v) noexcept
{
    static_assert( I < sizeof...(T), "Index out of bounds" );
    return v && v->index() == I? &v->_get_impl( mp_size_t<I>() ): 0;
}

template<class U, class... T> constexpr std::add_pointer_t<U> get_if(variant<T...>* v) noexcept
{
    static_assert( mp_count<variant<T...>, U>::value == 1, "The type must occur exactly once in the list of variant alternatives" );
    constexpr auto I = mp_find<variant<T...>, U>::value;

    return v && v->index() == I? &v->_get_impl( mp_size_t<I>() ): 0;
}

template<class U, class... T> constexpr std::add_pointer_t<U const> get_if(variant<T...> const * v) noexcept
{
    static_assert( mp_count<variant<T...>, U>::value == 1, "The type must occur exactly once in the list of variant alternatives" );
    constexpr auto I = mp_find<variant<T...>, U>::value;

    return v && v->index() == I? &v->_get_impl( mp_size_t<I>() ): 0;
}

//

namespace detail
{

// trivially_*

#if defined( BOOST_LIBSTDCXX_VERSION ) && BOOST_LIBSTDCXX_VERSION < 50000

template<class T> struct is_trivially_copy_constructible: mp_bool<std::is_copy_constructible<T>::value && std::has_trivial_copy_constructor<T>::value>
{
};

template<class T> struct is_trivially_copy_assignable: mp_bool<std::is_copy_assignable<T>::value && std::has_trivial_copy_assign<T>::value>
{
};

template<class T> struct is_trivially_move_constructible: mp_bool<std::is_move_constructible<T>::value && std::is_trivial<T>::value>
{
};

template<class T> struct is_trivially_move_assignable: mp_bool<std::is_move_assignable<T>::value && std::is_trivial<T>::value>
{
};

#else

using std::is_trivially_copy_constructible;
using std::is_trivially_copy_assignable;
using std::is_trivially_move_constructible;
using std::is_trivially_move_assignable;

#endif

// variant_storage

template<class D, class... T> union variant_storage_impl;

template<class... T> using variant_storage = variant_storage_impl<mp_all<std::is_trivially_destructible<T>...>, T...>;

template<class D> union variant_storage_impl<D>
{
};

// not all trivially destructible
template<class T1, class... T> union variant_storage_impl<mp_false, T1, T...>
{
    T1 first_;
    variant_storage<T...> rest_;

    template<class... A> constexpr explicit variant_storage_impl( mp_size_t<0>, A&&... a ): first_( std::forward<A>(a)... )
    {
    }

    template<std::size_t I, class... A> constexpr explicit variant_storage_impl( mp_size_t<I>, A&&... a ): rest_( mp_size_t<I-1>(), std::forward<A>(a)... )
    {
    }

    ~variant_storage_impl()
    {
    }

    template<class... A> void emplace( mp_size_t<0>, A&&... a )
    {
        ::new( &first_ ) T1( std::forward<A>(a)... );
    }

    template<std::size_t I, class... A> void emplace( mp_size_t<I>, A&&... a )
    {
        rest_.emplace( mp_size_t<I-1>(), std::forward<A>(a)... );
    }

    constexpr T1& get( mp_size_t<0> ) noexcept { return first_; }
    constexpr T1 const& get( mp_size_t<0> ) const noexcept { return first_; }

    template<std::size_t I> constexpr mp_at_c<mp_list<T...>, I-1>& get( mp_size_t<I> ) noexcept { return rest_.get( mp_size_t<I-1>() ); }
    template<std::size_t I> constexpr mp_at_c<mp_list<T...>, I-1> const& get( mp_size_t<I> ) const noexcept { return rest_.get( mp_size_t<I-1>() ); }
};

// all trivially destructible
template<class T1, class... T> union variant_storage_impl<mp_true, T1, T...>
{
    T1 first_;
    variant_storage<T...> rest_;

    template<class... A> constexpr explicit variant_storage_impl( mp_size_t<0>, A&&... a ): first_( std::forward<A>(a)... )
    {
    }

    template<std::size_t I, class... A> constexpr explicit variant_storage_impl( mp_size_t<I>, A&&... a ): rest_( mp_size_t<I-1>(), std::forward<A>(a)... )
    {
    }

    template<class... A> void emplace_impl( mp_false, mp_size_t<0>, A&&... a )
    {
        ::new( &first_ ) T1( std::forward<A>(a)... );
    }

    template<std::size_t I, class... A> constexpr void emplace_impl( mp_false, mp_size_t<I>, A&&... a )
    {
        rest_.emplace( mp_size_t<I-1>(), std::forward<A>(a)... );
    }

    template<std::size_t I, class... A> constexpr void emplace_impl( mp_true, mp_size_t<I>, A&&... a )
    {
        *this = variant_storage_impl( mp_size_t<I>(), std::forward<A>(a)... );
    }

    template<std::size_t I, class... A> constexpr void emplace( mp_size_t<I>, A&&... a )
    {
        this->emplace_impl( mp_all<variant2::detail::is_trivially_move_assignable<T1>, variant2::detail::is_trivially_move_assignable<T>...>(), mp_size_t<I>(), std::forward<A>(a)... );
    }

    constexpr T1& get( mp_size_t<0> ) noexcept { return first_; }
    constexpr T1 const& get( mp_size_t<0> ) const noexcept { return first_; }

    template<std::size_t I> constexpr mp_at_c<mp_list<T...>, I-1>& get( mp_size_t<I> ) noexcept { return rest_.get( mp_size_t<I-1>() ); }
    template<std::size_t I> constexpr mp_at_c<mp_list<T...>, I-1> const& get( mp_size_t<I> ) const noexcept { return rest_.get( mp_size_t<I-1>() ); }
};

// resolve_overload_*

template<class... T> struct overload;

template<> struct overload<>
{
    void operator()() const;
};

template<class T1, class... T> struct overload<T1, T...>: overload<T...>
{
    using overload<T...>::operator();
    mp_identity<T1> operator()(T1) const;
};

#if BOOST_WORKAROUND( BOOST_MSVC, <= 1910 )

template<class U, class... T> using resolve_overload_type_ = decltype( overload<T...>()(std::declval<U>()) );

template<class U, class... T> struct resolve_overload_type_impl: mp_defer< resolve_overload_type_, U, T... >
{
};

template<class U, class... T> using resolve_overload_type = typename resolve_overload_type_impl<U, T...>::type::type;

#else

template<class U, class... T> using resolve_overload_type = typename decltype( overload<T...>()(std::declval<U>()) )::type;

#endif

template<class U, class... T> using resolve_overload_index = mp_find<mp_list<T...>, resolve_overload_type<U, T...>>;

// variant_base

template<class... T> using can_be_valueless = std::is_same<mp_first<mp_list<T...>>, valueless>;

template<bool is_trivially_destructible, bool is_single_buffered, class... T> struct variant_base_impl; // trivially destructible, single buffered
template<class... T> using variant_base = variant_base_impl<mp_all<std::is_trivially_destructible<T>...>::value, mp_any<mp_all<std::is_nothrow_move_constructible<T>...>, can_be_valueless<T...>>::value, T...>;

struct none {};

// trivially destructible, single buffered
template<class... T> struct variant_base_impl<true, true, T...>
{
    int ix_;
    variant_storage<none, T...> st1_;

    constexpr variant_base_impl(): ix_( 0 ), st1_( mp_size_t<0>() )
    {
    }

    template<class I, class... A> constexpr explicit variant_base_impl( I, A&&... a ): ix_( I::value + 1 ), st1_( mp_size_t<I::value + 1>(), std::forward<A>(a)... )
    {
    }

    constexpr std::size_t index() const noexcept
    {
        return ix_ - 1;
    }

    template<std::size_t I> constexpr mp_at_c<variant<T...>, I>& _get_impl( mp_size_t<I> ) noexcept
    {
        size_t const J = I+1;

        assert( ix_ == J );

        return st1_.get( mp_size_t<J>() );
    }

    template<std::size_t I> constexpr mp_at_c<variant<T...>, I> const& _get_impl( mp_size_t<I> ) const noexcept
    {
        size_t const J = I+1;

        assert( ix_ == J );

        return st1_.get( mp_size_t<J>() );
    }

    template<std::size_t J, class U, bool B, class... A> constexpr void emplace_impl( mp_true, mp_bool<B>, A&&... a )
    {
        st1_.emplace( mp_size_t<J>(), std::forward<A>(a)... );
        ix_ = J;
    }

    template<std::size_t J, class U, class... A> constexpr void emplace_impl( mp_false, mp_true, A&&... a )
    {
        U tmp( std::forward<A>(a)... );

        st1_.emplace( mp_size_t<J>(), std::move(tmp) );
        ix_ = J;
    }

    template<std::size_t J, class U, class... A> void emplace_impl( mp_false, mp_false, A&&... a )
    {
        if( can_be_valueless<T...>::value ) // T0 == valueless
        {
            std::size_t const K = 0;

            try
            {
                st1_.emplace( mp_size_t<J>(), std::forward<A>(a)... );
                ix_ = J;
            }
            catch( ... )
            {
                st1_.emplace( mp_size_t<K+1>() );
                ix_ = K+1;

                throw;
            }
        }
        else
        {
            assert( std::is_nothrow_move_constructible<U>::value );

            U tmp( std::forward<A>(a)... );

            st1_.emplace( mp_size_t<J>(), std::move(tmp) );
            ix_ = J;
        }
    }

    template<std::size_t I, class... A> constexpr void emplace( A&&... a )
    {
        std::size_t const J = I+1;
        using U = mp_at_c<variant<T...>, I>;

        this->emplace_impl<J, U>( std::is_nothrow_constructible<U, A&&...>(), mp_all<variant2::detail::is_trivially_move_constructible<U>, variant2::detail::is_trivially_move_assignable<T>...>(), std::forward<A>(a)... );
    }
};

// trivially destructible, double buffered
template<class... T> struct variant_base_impl<true, false, T...>
{
    int ix_;
    variant_storage<none, T...> st1_;
    variant_storage<none, T...> st2_;

    constexpr variant_base_impl(): ix_( 0 ), st1_( mp_size_t<0>() ), st2_( mp_size_t<0>() )
    {
    }

    template<class I, class... A> constexpr explicit variant_base_impl( I, A&&... a ): ix_( I::value + 1 ), st1_( mp_size_t<I::value + 1>(), std::forward<A>(a)... ), st2_( mp_size_t<0>() )
    {
    }

    constexpr std::size_t index() const noexcept
    {
        return ix_ >= 0? ix_ - 1: -ix_ - 1;
    }

    template<std::size_t I> constexpr mp_at_c<variant<T...>, I>& _get_impl( mp_size_t<I> ) noexcept
    {
        size_t const J = I+1;

        assert( ix_ == J || -ix_ == J );

        constexpr mp_size_t<J> j{};
        return ix_ >= 0? st1_.get( j ): st2_.get( j );
    }

    template<std::size_t I> constexpr mp_at_c<variant<T...>, I> const& _get_impl( mp_size_t<I> ) const noexcept
    {
        size_t const J = I+1;

        assert( ix_ == J || -ix_ == J );

        constexpr mp_size_t<J> j{};
        return ix_ >= 0? st1_.get( j ): st2_.get( j );
    }

    template<std::size_t I, class... A> constexpr void emplace( A&&... a )
    {
        size_t const J = I+1;

        if( ix_ >= 0 )
        {
            st2_.emplace( mp_size_t<J>(), std::forward<A>(a)... );
            ix_ = -static_cast<int>( J );
        }
        else
        {
            st1_.emplace( mp_size_t<J>(), std::forward<A>(a)... );
            ix_ = J;
        }
    }
};

// not trivially destructible, single buffered
template<class... T> struct variant_base_impl<false, true, T...>
{
    int ix_;
    variant_storage<none, T...> st1_;

    constexpr variant_base_impl(): ix_( 0 ), st1_( mp_size_t<0>() )
    {
    }

    template<class I, class... A> constexpr explicit variant_base_impl( I, A&&... a ): ix_( I::value + 1 ), st1_( mp_size_t<I::value + 1>(), std::forward<A>(a)... )
    {
    }

    void _destroy() noexcept
    {
        if( ix_ > 0 )
        {
            mp_with_index<1 + sizeof...(T)>( ix_, [&]( auto I ){

                using U = mp_at_c<mp_list<none, T...>, I>;
                st1_.get( I ).~U();

            });
        }
    }

    ~variant_base_impl() noexcept
    {
        _destroy();
    }

    constexpr std::size_t index() const noexcept
    {
        return ix_ - 1;
    }

    template<std::size_t I> constexpr mp_at_c<variant<T...>, I>& _get_impl( mp_size_t<I> ) noexcept
    {
        size_t const J = I+1;

        assert( ix_ == J );

        return st1_.get( mp_size_t<J>() );
    }

    template<std::size_t I> constexpr mp_at_c<variant<T...>, I> const& _get_impl( mp_size_t<I> ) const noexcept
    {
        size_t const J = I+1;

        assert( ix_ == J );

        return st1_.get( mp_size_t<J>() );
    }

    template<std::size_t I, class... A> void emplace( A&&... a )
    {
        size_t const J = I+1;

        using U = mp_at_c<variant<T...>, I>;

        if( std::is_nothrow_constructible<U, A...>::value )
        {
            _destroy();

            st1_.emplace( mp_size_t<J>(), std::forward<A>(a)... );
            ix_ = J;
        }
        else if( can_be_valueless<T...>::value ) // T0 == valueless
        {
            std::size_t const K = 0;

            _destroy();

            try
            {
                st1_.emplace( mp_size_t<J>(), std::forward<A>(a)... );
                ix_ = J;
            }
            catch( ... )
            {
                st1_.emplace( mp_size_t<K+1>() );
                ix_ = K+1;

                throw;
            }
        }
        else
        {
            assert( std::is_nothrow_move_constructible<U>::value );

            U tmp( std::forward<A>(a)... );

            _destroy();

            st1_.emplace( mp_size_t<J>(), std::move(tmp) );
            ix_ = J;
        }
    }
};

// not trivially destructible, double buffered
template<class... T> struct variant_base_impl<false, false, T...>
{
    int ix_;
    variant_storage<none, T...> st1_;
    variant_storage<none, T...> st2_;

    constexpr variant_base_impl(): ix_( 0 ), st1_( mp_size_t<0>() ), st2_( mp_size_t<0>() )
    {
    }

    template<class I, class... A> constexpr explicit variant_base_impl( I, A&&... a ): ix_( I::value + 1 ), st1_( mp_size_t<I::value + 1>(), std::forward<A>(a)... ), st2_( mp_size_t<0>() )
    {
    }

    void _destroy() noexcept
    {
        if( ix_ > 0 )
        {
            mp_with_index<1 + sizeof...(T)>( ix_, [&]( auto I ){

                using U = mp_at_c<mp_list<none, T...>, I>;
                st1_.get( I ).~U();

            });
        }
        else if( ix_ < 0 )
        {
            mp_with_index<1 + sizeof...(T)>( -ix_, [&]( auto I ){

                using U = mp_at_c<mp_list<none, T...>, I>;
                st2_.get( I ).~U();

            });
        }
    }

    ~variant_base_impl() noexcept
    {
        _destroy();
    }

    constexpr std::size_t index() const noexcept
    {
        return ix_ >= 0? ix_ - 1: -ix_ - 1;
    }

    template<std::size_t I> constexpr mp_at_c<variant<T...>, I>& _get_impl( mp_size_t<I> ) noexcept
    {
        size_t const J = I+1;

        assert( ix_ == J || -ix_ == J );

        constexpr mp_size_t<J> j{};
        return ix_ >= 0? st1_.get( j ): st2_.get( j );
    }

    template<std::size_t I> constexpr mp_at_c<variant<T...>, I> const& _get_impl( mp_size_t<I> ) const noexcept
    {
        size_t const J = I+1;

        assert( ix_ == J || -ix_ == J );

        constexpr mp_size_t<J> j{};
        return ix_ >= 0? st1_.get( j ): st2_.get( j );
    }

    template<std::size_t I, class... A> void emplace( A&&... a )
    {
        size_t const J = I+1;

        if( ix_ >= 0 )
        {
            st2_.emplace( mp_size_t<J>(), std::forward<A>(a)... );
            _destroy();

            ix_ = -static_cast<int>( J );
        }
        else
        {
            st1_.emplace( mp_size_t<J>(), std::forward<A>(a)... );
            _destroy();

            ix_ = J;
        }
    }
};

} // namespace detail

// in_place_type_t

template<class T> struct in_place_type_t
{
};

template<class T> constexpr in_place_type_t<T> in_place_type{};

namespace detail
{

template<class T> struct is_in_place_type: std::false_type {};
template<class T> struct is_in_place_type<in_place_type_t<T>>: std::true_type {};

} // namespace detail

// in_place_index_t

template<std::size_t I> struct in_place_index_t
{
};

template<std::size_t I> constexpr in_place_index_t<I> in_place_index{};

namespace detail
{

template<class T> struct is_in_place_index: std::false_type {};
template<std::size_t I> struct is_in_place_index<in_place_index_t<I>>: std::true_type {};

} // namespace detail

// is_nothrow_swappable

namespace detail
{

namespace det2
{

using std::swap;

template<class T> using is_swappable_impl = decltype(swap(std::declval<T&>(), std::declval<T&>()));

#if BOOST_WORKAROUND( BOOST_MSVC, <= 1910 )

template<class T> struct is_nothrow_swappable_impl_
{
    static constexpr bool value = noexcept(swap(std::declval<T&>(), std::declval<T&>()));
};

template<class T> using is_nothrow_swappable_impl = mp_bool< is_nothrow_swappable_impl_<T>::value >;

#else

template<class T> using is_nothrow_swappable_impl = std::enable_if_t<noexcept(swap(std::declval<T&>(), std::declval<T&>()))>;

#endif

} // namespace det2

template<class T> struct is_swappable: mp_valid<det2::is_swappable_impl, T>
{
};

#if BOOST_WORKAROUND( BOOST_MSVC, <= 1910 )

template<class T> struct is_nothrow_swappable: mp_eval_if<mp_not<is_swappable<T>>, mp_false, det2::is_nothrow_swappable_impl, T>
{
};

#else

template<class T> struct is_nothrow_swappable: mp_valid<det2::is_nothrow_swappable_impl, T>
{
};

#endif

} // namespace detail

// variant

template<class... T> class variant: private variant2::detail::variant_base<T...>
{
private:

    using variant_base = variant2::detail::variant_base<T...>;

private:

    variant( variant const volatile& r ) = delete;
    variant& operator=( variant const volatile& r ) = delete;

public:

    // constructors

    template<class E1 = void, class E2 = mp_if<std::is_default_constructible< mp_first<variant<T...>> >, E1>>
    constexpr variant()
        noexcept( std::is_nothrow_default_constructible< mp_first<variant<T...>> >::value )
        : variant_base( mp_size_t<0>() )
    {
    }

    template<class E1 = void,
        class E2 = mp_if<mp_all<variant2::detail::is_trivially_copy_constructible<T>...>, E1>
    >
    constexpr variant( variant const& r ) noexcept
        : variant_base( static_cast<variant_base const&>(r) )
    {
    }

    template<class E1 = void,
        class E2 = mp_if<mp_not<mp_all<variant2::detail::is_trivially_copy_constructible<T>...>>, E1>,
        class E3 = mp_if<mp_all<std::is_copy_constructible<T>...>, E1>
    >
    variant( variant const& r )
        noexcept( mp_all<std::is_nothrow_copy_constructible<T>...>::value )
    {
        mp_with_index<sizeof...(T)>( r.index(), [&]( auto I ){

            ::new( static_cast<variant_base*>(this) ) variant_base( I, r._get_impl( I ) );

        });
    }

    template<class E1 = void,
        class E2 = mp_if<mp_all<variant2::detail::is_trivially_move_constructible<T>...>, E1>
    >
    constexpr variant( variant && r ) noexcept
        : variant_base( static_cast<variant_base&&>(r) )
    {
    }

    template<class E1 = void,
        class E2 = mp_if<mp_not<mp_all<variant2::detail::is_trivially_move_constructible<T>...>>, E1>,
        class E3 = mp_if<mp_all<std::is_move_constructible<T>...>, E1>
    >
    variant( variant && r )
        noexcept( mp_all<std::is_nothrow_move_constructible<T>...>::value )
    {
        mp_with_index<sizeof...(T)>( r.index(), [&]( auto I ){

            ::new( static_cast<variant_base*>(this) ) variant_base( I, std::move( r._get_impl( I ) ) );

        });
    }

    template<class U,
        class Ud = std::decay_t<U>,
        class E1 = std::enable_if_t< !std::is_same<Ud, variant>::value && !variant2::detail::is_in_place_index<Ud>::value && !variant2::detail::is_in_place_type<Ud>::value >,
        class V = variant2::detail::resolve_overload_type<U&&, T...>,
        class E2 = std::enable_if_t<std::is_constructible<V, U>::value>
        >
    constexpr variant( U&& u )
        noexcept( std::is_nothrow_constructible<V, U>::value )
        : variant_base( variant2::detail::resolve_overload_index<U&&, T...>(), std::forward<U>(u) )
    {
    }

    template<class U, class... A, class I = mp_find<variant<T...>, U>, class E = std::enable_if_t<std::is_constructible<U, A...>::value>>
    constexpr explicit variant( in_place_type_t<U>, A&&... a ): variant_base( I(), std::forward<A>(a)... )
    {
    }

    template<class U, class V, class... A, class I = mp_find<variant<T...>, U>, class E = std::enable_if_t<std::is_constructible<U, std::initializer_list<V>&, A...>::value>>
    constexpr explicit variant( in_place_type_t<U>, std::initializer_list<V> il, A&&... a ): variant_base( I(), il, std::forward<A>(a)... )
    {
    }

    template<std::size_t I, class... A, class E = std::enable_if_t<std::is_constructible<mp_at_c<variant<T...>, I>, A...>::value>>
    constexpr explicit variant( in_place_index_t<I>, A&&... a ): variant_base( mp_size_t<I>(), std::forward<A>(a)... )
    {
    }

    template<std::size_t I, class V, class... A, class E = std::enable_if_t<std::is_constructible<mp_at_c<variant<T...>, I>, std::initializer_list<V>&, A...>::value>>
    constexpr explicit variant( in_place_index_t<I>, std::initializer_list<V> il, A&&... a ): variant_base( mp_size_t<I>(), il, std::forward<A>(a)... )
    {
    }

    // assignment
    template<class E1 = void,
        class E2 = mp_if<mp_all<std::is_trivially_destructible<T>..., variant2::detail::is_trivially_copy_assignable<T>...>, E1>
    >
    constexpr variant& operator=( variant const & r ) noexcept
    {
        static_cast<variant_base&>( *this ) = static_cast<variant_base const&>( r );
        return *this;
    }

    template<class E1 = void,
        class E2 = mp_if<mp_not<mp_all<std::is_trivially_destructible<T>..., variant2::detail::is_trivially_copy_assignable<T>...>>, E1>,
        class E3 = mp_if<mp_all<std::is_copy_constructible<T>..., std::is_copy_assignable<T>...>, E1>
    >
    constexpr variant& operator=( variant const & r )
        noexcept( mp_all<std::is_nothrow_copy_constructible<T>..., std::is_nothrow_copy_assignable<T>...>::value )
    {
        mp_with_index<sizeof...(T)>( r.index(), [&]( auto I ){

            if( this->index() == I )
            {
                this->_get_impl( I ) = r._get_impl( I );
            }
            else
            {
                this->variant_base::template emplace<I>( r._get_impl( I ) );
            }

        });

        return *this;
    }

    template<class E1 = void,
        class E2 = mp_if<mp_all<std::is_trivially_destructible<T>..., variant2::detail::is_trivially_move_assignable<T>...>, E1>
    >
    constexpr variant& operator=( variant && r ) noexcept
    {
        static_cast<variant_base&>( *this ) = static_cast<variant_base&&>( r );
        return *this;
    }

    template<class E1 = void,
        class E2 = mp_if<mp_not<mp_all<std::is_trivially_destructible<T>..., variant2::detail::is_trivially_move_assignable<T>...>>, E1>,
        class E3 = mp_if<mp_all<std::is_move_constructible<T>..., std::is_move_assignable<T>...>, E1>
    >
    variant& operator=( variant && r )
        noexcept( mp_all<std::is_nothrow_move_constructible<T>..., std::is_nothrow_move_assignable<T>...>::value )
    {
        mp_with_index<sizeof...(T)>( r.index(), [&]( auto I ){

            if( this->index() == I )
            {
                this->_get_impl( I ) = std::move( r._get_impl( I ) );
            }
            else
            {
                this->variant_base::template emplace<I>( std::move( r._get_impl( I ) ) );
            }

        });

        return *this;
    }

    template<class U,
        class E1 = std::enable_if_t<!std::is_same<std::decay_t<U>, variant>::value>,
        class V = variant2::detail::resolve_overload_type<U, T...>,
        class E2 = std::enable_if_t<std::is_assignable<V&, U>::value && std::is_constructible<V, U>::value>
    >
    constexpr variant& operator=( U&& u )
        noexcept( std::is_nothrow_assignable<V&, U>::value && std::is_nothrow_constructible<V, U>::value )
    {
        std::size_t const I = variant2::detail::resolve_overload_index<U, T...>::value;

        if( index() == I )
        {
            _get_impl( mp_size_t<I>() ) = std::forward<U>(u);
        }
        else
        {
            this->template emplace<I>( std::forward<U>(u) );
        }

        return *this;
    }

    // modifiers

    template<class U, class... A, class I = mp_find<variant<T...>, U>, class E = std::enable_if_t<std::is_constructible<U, A...>::value>>
    constexpr U& emplace( A&&... a )
    {
        variant_base::template emplace<I::value>( std::forward<A>(a)... );
        return _get_impl( I() );
    }

    template<class U, class V, class... A, class I = mp_find<variant<T...>, U>, class E = std::enable_if_t<std::is_constructible<U, std::initializer_list<V>&, A...>::value>>
    constexpr U& emplace( std::initializer_list<V> il, A&&... a )
    {
        variant_base::template emplace<I::value>( il, std::forward<A>(a)... );
        return _get_impl( I() );
    }

    template<std::size_t I, class... A, class E = std::enable_if_t<std::is_constructible<mp_at_c<variant<T...>, I>, A...>::value>>
    constexpr variant_alternative_t<I, variant<T...>>& emplace( A&&... a )
    {
        variant_base::template emplace<I>( std::forward<A>(a)... );
        return _get_impl( mp_size_t<I>() );
    }

    template<std::size_t I, class V, class... A, class E = std::enable_if_t<std::is_constructible<mp_at_c<variant<T...>, I>, std::initializer_list<V>&, A...>::value>>
    constexpr variant_alternative_t<I, variant<T...>>& emplace( std::initializer_list<V> il, A&&... a )
    {
        variant_base::template emplace<I>( il, std::forward<A>(a)... );
        return _get_impl( mp_size_t<I>() );
    }

    // value status

    using variant_base::index;

    // swap

    void swap( variant& r ) noexcept( mp_all<std::is_nothrow_move_constructible<T>..., variant2::detail::is_nothrow_swappable<T>...>::value )
    {
        if( index() == r.index() )
        {
            mp_with_index<sizeof...(T)>( index(), [&]( auto I ){

                using std::swap;
                swap( this->_get_impl( I ), r._get_impl( I ) );

            });
        }
        else
        {
            variant tmp( std::move(*this) );
            *this = std::move( r );
            r = std::move( tmp );
        }
    }

    // private accessors

    constexpr int _real_index() const noexcept
    {
        return this->ix_;
    }

    using variant_base::_get_impl;

    // converting constructors (extension)

    template<class... U,
        class E2 = mp_if<mp_all<std::is_copy_constructible<U>..., mp_contains<mp_list<T...>, U>...>, void> >
    variant( variant<U...> const& r )
        noexcept( mp_all<std::is_nothrow_copy_constructible<U>...>::value )
    {
        mp_with_index<sizeof...(U)>( r.index(), [&]( auto I ){

            using J = mp_find<mp_list<T...>, mp_at_c<mp_list<U...>, I>>;

            ::new( static_cast<variant_base*>(this) ) variant_base( J{}, r._get_impl( I ) );

        });
    }

    template<class... U,
        class E2 = mp_if<mp_all<std::is_move_constructible<U>..., mp_contains<mp_list<T...>, U>...>, void> >
    variant( variant<U...> && r )
        noexcept( mp_all<std::is_nothrow_move_constructible<U>...>::value )
    {
        mp_with_index<sizeof...(U)>( r.index(), [&]( auto I ){

            using J = mp_find<mp_list<T...>, mp_at_c<mp_list<U...>, I>>;

            ::new( static_cast<variant_base*>(this) ) variant_base( J{}, std::move( r._get_impl( I ) ) );

        });
    }

    // subset (extension)

private:

    template<class... U, class V, std::size_t J, class E = std::enable_if_t<J != sizeof...(U)>> static constexpr variant<U...> _subset_impl( mp_size_t<J>, V && v )
    {
        return variant<U...>( in_place_index<J>, std::forward<V>(v) );
    }

    template<class... U, class V> static variant<U...> _subset_impl( mp_size_t<sizeof...(U)>, V && /*v*/ )
    {
        throw bad_variant_access();
    }

public:

    template<class... U,
        class E2 = mp_if<mp_all<std::is_copy_constructible<U>..., mp_contains<mp_list<T...>, U>...>, void> >
    constexpr variant<U...> subset() &
    {
        return mp_with_index<sizeof...(T)>( index(), [&]( auto I ){

            using J = mp_find<mp_list<U...>, mp_at_c<mp_list<T...>, I>>;

            return this->_subset_impl<U...>( J{}, this->_get_impl( I ) );

        });
    }

    template<class... U,
        class E2 = mp_if<mp_all<std::is_copy_constructible<U>..., mp_contains<mp_list<T...>, U>...>, void> >
    constexpr variant<U...> subset() const&
    {
        return mp_with_index<sizeof...(T)>( index(), [&]( auto I ){

            using J = mp_find<mp_list<U...>, mp_at_c<mp_list<T...>, I>>;

            return this->_subset_impl<U...>( J{}, this->_get_impl( I ) );

        });
    }

    template<class... U,
        class E2 = mp_if<mp_all<std::is_copy_constructible<U>..., mp_contains<mp_list<T...>, U>...>, void> >
    constexpr variant<U...> subset() &&
    {
        return mp_with_index<sizeof...(T)>( index(), [&]( auto I ){

            using J = mp_find<mp_list<U...>, mp_at_c<mp_list<T...>, I>>;

            return this->_subset_impl<U...>( J{}, std::move( this->_get_impl( I ) ) );

        });
    }

    template<class... U,
        class E2 = mp_if<mp_all<std::is_copy_constructible<U>..., mp_contains<mp_list<T...>, U>...>, void> >
    constexpr variant<U...> subset() const&&
    {
        return mp_with_index<sizeof...(T)>( index(), [&]( auto I ){

            using J = mp_find<mp_list<U...>, mp_at_c<mp_list<T...>, I>>;

            return this->_subset_impl<U...>( J{}, std::move( this->_get_impl( I ) ) );

        });
    }
};

// relational operators
template<class... T> constexpr bool operator==( variant<T...> const & v, variant<T...> const & w )
{
    if( v.index() != w.index() ) return false;

    return mp_with_index<sizeof...(T)>( v.index(), [&]( auto I ){

        return v._get_impl( I ) == w._get_impl( I );

    });
}

template<class... T> constexpr bool operator!=( variant<T...> const & v, variant<T...> const & w )
{
    if( v.index() != w.index() ) return true;

    return mp_with_index<sizeof...(T)>( v.index(), [&]( auto I ){

        return v._get_impl( I ) != w._get_impl( I );

    });
}

template<class... T> constexpr bool operator<( variant<T...> const & v, variant<T...> const & w )
{
    if( v.index() < w.index() ) return true;
    if( v.index() > w.index() ) return false;

    return mp_with_index<sizeof...(T)>( v.index(), [&]( auto I ){

        return v._get_impl( I ) < w._get_impl( I );

    });
}

template<class... T> constexpr bool operator>(  variant<T...> const & v, variant<T...> const & w )
{
    if( v.index() > w.index() ) return true;
    if( v.index() < w.index() ) return false;

    return mp_with_index<sizeof...(T)>( v.index(), [&]( auto I ){

        return v._get_impl( I ) > w._get_impl( I );

    });
}

template<class... T> constexpr bool operator<=( variant<T...> const & v, variant<T...> const & w )
{
    if( v.index() < w.index() ) return true;
    if( v.index() > w.index() ) return false;

    return mp_with_index<sizeof...(T)>( v.index(), [&]( auto I ){

        return v._get_impl( I ) <= w._get_impl( I );

    });
}

template<class... T> constexpr bool operator>=( variant<T...> const & v, variant<T...> const & w )
{
    if( v.index() > w.index() ) return true;
    if( v.index() < w.index() ) return false;

    return mp_with_index<sizeof...(T)>( v.index(), [&]( auto I ){

        return v._get_impl( I ) >= w._get_impl( I );

    });
}

// visitation
namespace detail
{

template<class F> struct Qret
{
    template<class... T> using fn = decltype( std::declval<F>()( std::declval<T>()... ) );
};

template<class L> using front_if_same = mp_if<mp_apply<mp_same, L>, mp_front<L>>;

template<class V> using var_size = variant_size<std::remove_reference_t<V>>;

template<class V, class N, class T = variant_alternative_t<N::value, std::remove_reference_t<V>>> using apply_cv_ref_ = mp_if<std::is_reference<V>, T&, T>;

#if BOOST_WORKAROUND( BOOST_MSVC, <= 1910 )

template<class V> struct apply_cv_ref_impl
{
    template<class T> using _f = apply_cv_ref_<V, T>;

    using L = mp_iota<var_size<V>>;

    using type = mp_transform<_f, L>;
};

template<class V> using apply_cv_ref = typename apply_cv_ref_impl<V>::type;

#else

template<class V> using apply_cv_ref = mp_transform_q<mp_bind_front<apply_cv_ref_, V>, mp_iota<var_size<V>>>;

#endif

template<class F, class... V> using Vret = front_if_same<mp_product_q<Qret<F>, apply_cv_ref<V>...>>;

} // namespace detail

template<class F> constexpr auto visit( F&& f ) -> decltype(std::forward<F>(f)())
{
    return std::forward<F>(f)();
}

template<class F, class V1> constexpr auto visit( F&& f, V1&& v1 ) -> variant2::detail::Vret<F, V1>
{
    return mp_with_index<variant2::detail::var_size<V1>>( v1.index(), [&]( auto I ){

        return std::forward<F>(f)( get<I>( std::forward<V1>(v1) ) );

    });
}

#if BOOST_WORKAROUND( BOOST_MSVC, <= 1910 )

template<class F, class V1, class V2> constexpr auto visit( F&& f, V1&& v1, V2&& v2 ) -> variant2::detail::Vret<F, V1, V2>
{
    return mp_with_index<variant2::detail::var_size<V1>>( v1.index(), [&]( auto I ){

        auto f2 = [&]( auto&&... a ){ return std::forward<F>(f)( get<I.value>( std::forward<V1>(v1) ), std::forward<decltype(a)>(a)... ); };
        return visit( f2, std::forward<V2>(v2) );

    });
}

template<class F, class V1, class V2, class V3> constexpr auto visit( F&& f, V1&& v1, V2&& v2, V3&& v3 ) -> variant2::detail::Vret<F, V1, V2, V3>
{
    return mp_with_index<variant2::detail::var_size<V1>>( v1.index(), [&]( auto I ){

        auto f2 = [&]( auto&&... a ){ return std::forward<F>(f)( get<I.value>( std::forward<V1>(v1) ), std::forward<decltype(a)>(a)... ); };
        return visit( f2, std::forward<V2>(v2), std::forward<V3>(v3) );

    });
}

template<class F, class V1, class V2, class V3, class V4> constexpr auto visit( F&& f, V1&& v1, V2&& v2, V3&& v3, V4&& v4 ) -> variant2::detail::Vret<F, V1, V2, V3, V4>
{
    return mp_with_index<variant2::detail::var_size<V1>>( v1.index(), [&]( auto I ){

        auto f2 = [&]( auto&&... a ){ return std::forward<F>(f)( get<I.value>( std::forward<V1>(v1) ), std::forward<decltype(a)>(a)... ); };
        return visit( f2, std::forward<V2>(v2), std::forward<V3>(v3), std::forward<V4>(v4) );

    });
}

#else

template<class F, class V1, class V2, class... V> constexpr auto visit( F&& f, V1&& v1, V2&& v2, V&&... v ) -> variant2::detail::Vret<F, V1, V2, V...>
{
    return mp_with_index<variant2::detail::var_size<V1>>( v1.index(), [&]( auto I ){

        auto f2 = [&]( auto&&... a ){ return std::forward<F>(f)( get<I.value>( std::forward<V1>(v1) ), std::forward<decltype(a)>(a)... ); };
        return visit( f2, std::forward<V2>(v2), std::forward<V>(v)... );

    });
}

#endif

// specialized algorithms
template<class... T,
    class E = std::enable_if_t<mp_all<std::is_move_constructible<T>..., variant2::detail::is_swappable<T>...>::value>>
void swap( variant<T...> & v, variant<T...> & w )
    noexcept( noexcept(v.swap(w)) )
{
    v.swap( w );
}

} // namespace variant2
} // namespace boost

#endif // #ifndef BOOST_VARIANT2_VARIANT_HPP_INCLUDED
