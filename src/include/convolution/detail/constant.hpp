#pragma once

#include "../../cube/cube.hpp"
#include "../../types.hpp"
#include "../../meta.hpp"

namespace znn { namespace v4 { namespace detail {

template< typename T >
inline void convolve_constant_add( cube<T> const & a,
                                   identity_t<T> b,
                                   cube<T> & r) noexcept
{
    ZI_ASSERT(size(a)==size(r));

    T const * ap = a.data();
    T       * rp = r.data();

    for ( size_t i = 0; i < r.num_elements(); ++i )
    {
        rp[i] += ap[i] * b;
    }
}

template< typename T >
inline void convolve_constant( cube<T> const & a,
                               identity_t<T> b,
                               cube<T> & r) noexcept
{
    ZI_ASSERT(size(a)==size(r));

    T const * ap = a.data();
    T       * rp = r.data();

    for ( size_t i = 0; i < r.num_elements(); ++i )
    {
        rp[i] = ap[i] * b;
    }
}

template< typename T >
inline T convolve_constant_flipped( cube<T> const & a,
                                    cube<T> const & b ) noexcept
{
    ZI_ASSERT(size(a)==size(b));

    T r = 0;
    T const * ap = a.data();
    T const * bp = b.data();

    for ( size_t i = 0; i < a.num_elements(); ++i )
    {
        r += ap[i] * bp[i];
    }

    return r;
}

template< typename T >
inline void convolve_constant_inverse_add( cube<T> const & a,
                                           identity_t<T> b,
                                           cube<T> & r) noexcept
{
    znn::v4::detail::convolve_constant_add(a,b,r);
}

template< typename T >
inline void convolve_constant_inverse( cube<T> const & a,
                                       identity_t<T> b,
                                       cube<T> & r) noexcept
{
    znn::v4::detail::convolve_constant(a,b,r);
}


}}} // namespace znn::v4::detail
