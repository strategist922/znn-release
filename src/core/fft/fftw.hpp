//
// Copyright (C) 2014-present  Aleksandar Zlateski <zlateski@mit.edu>
//                             Kisuk Lee           <kisuklee@mit.edu>
// ------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef ZNN_CORE_FFT_FFTW_HPP_INCLUDED
#define ZNN_CORE_FFT_FFTW_HPP_INCLUDED

#include "fftw_plans.hpp"
#include "../types.hpp"
#include "../volume_pool.hpp"
#include "../volume_utils.hpp"
#include "../volume_operators.hpp"
#include "../utils.hpp"
#include "../measure.hpp"

#include <zi/utility/assert.hpp>
#include <zi/concurrency.hpp>
#include <zi/time.hpp>

namespace zi {
namespace znn {

class fftw_stats_impl
{
private:
    double         total_time_;
    std::size_t    total_     ;
    zi::mutex      m_         ;

public:
    fftw_stats_impl()
        : total_time_(0)
        , total_(0)
        , m_()
    { }

    // [kisuklee]
    std::size_t get_total_time() const
    {
        return total_time_;
    }

    // [kisuklee]
    void reset_total_time()
    {
        total_time_ = 0;
    }

    void add(double time)
    {
        zi::mutex::guard g(m_);
        ++total_;
        total_time_ += time;
    }
};

namespace {
fftw_stats_impl& fftw_stats = zi::singleton<fftw_stats_impl>::instance();
} // anonymous namespace

class fftw
{
public:
    static void forward( vol<double>& in,
                         vol<complex>& out )
    {
        ZI_ASSERT(in.shape()[0]==out.shape()[0]);
        ZI_ASSERT(in.shape()[1]==out.shape()[1]);
        ZI_ASSERT((in.shape()[2]/2+1)==out.shape()[2]);

        fftw_plan plan = fftw_plans.get_forward(
            vec3i(in.shape()[0],in.shape()[1],in.shape()[2]));

#ifdef MEASURE_FFT_RUNTIME
        zi::wall_timer wt;
#endif
        fftw_execute_dft_r2c(plan,
                             reinterpret_cast<double*>(in.data()),
                             reinterpret_cast<fftw_complex*>(out.data()));
#ifdef MEASURE_FFT_RUNTIME
        fftw_stats.add(wt.elapsed<double>());
#endif
    }

    static void backward( vol<complex>& in,
                          vol<double>& out )
    {
        ZI_ASSERT(in.shape()[0]==out.shape()[0]);
        ZI_ASSERT(in.shape()[1]==out.shape()[1]);
        ZI_ASSERT((out.shape()[2]/2+1)==in.shape()[2]);

        fftw_plan plan = fftw_plans.get_backward(
            vec3i(out.shape()[0],out.shape()[1],out.shape()[2]));

#ifdef MEASURE_FFT_RUNTIME
        zi::wall_timer wt;
#endif
        fftw_execute_dft_c2r(plan,
                             reinterpret_cast<fftw_complex*>(in.data()),
                             reinterpret_cast<double*>(out.data()));
#ifdef MEASURE_FFT_RUNTIME
        fftw_stats.add(wt.elapsed<double>());
#endif
    }

    static void forward( const vol_p<double>& in,
                         const vol_p<complex>& out )
    {
        forward(*in, *out);
    }

    static void backward( const vol_p<complex>& in,
                          const vol_p<double>& out )
    {
        backward(*in, *out);
    }

    static vol_p<complex> forward( const cvol_p<double>& in )
    {
        vol_p<double> in_copy = get_volume<double>(size(*in));
        *in_copy = *in;

        vol_p<complex> ret = get_fft_volume<complex>(size(*in));
        fftw::forward( *in_copy, *ret );
        return ret;
    }

    static vol_p<complex> forward( vol_p<double>&& in )
    {
        vol_p<complex> ret = get_fft_volume<complex>(size(*in));
        fftw::forward( *in, *ret );
        return ret;
    }

    static vol_p<double> backward( vol_p<complex>&& in, const vec3i& s )
    {
        vol_p<double> ret = get_volume<double>(s);
        fftw::backward( *in, *ret );
        return ret;
    }

    static vol_p<complex> forward_pad( const cvol_p<double>& in,
                                       const vec3i& pad )
    {
        if ( pad == size(*in) ) return fftw::forward(in);

        vol_p<double> pin = pad_zeros(*in, pad);
        return fftw::forward(std::move(pin));
    }

    // static vol_p<complex> forward( const cvol<double>& in )
    // {
    //     vol_p<complex> ret = get_volume<complex>(size(in));
    //     fftw::forward( in, *ret );
    //     return ret;
    // }

    // static vol_p<double> backward( const cvol<complex>& in,
    //                                const vec3i& s )
    // {
    //     vol_p<double> ret = get_volume<double>(s);
    //     fftw::backward( in, *ret );
    //     return ret;
    // }

    // static vol_p<complex> forward_pad( const cvol<double>& in,
    //                                    const vec3i& s )
    // {
    //     if ( s == size(in) )
    //     {
    //         return fftw::forward(in);
    //     }
    //     return fftw::forward(pad_zeros(in,s));
    // }


}; // class fftw


}} // namespace zi::znn

#endif // ZNN_CORE_FFT_FFTW_HPP_INCLUDED
