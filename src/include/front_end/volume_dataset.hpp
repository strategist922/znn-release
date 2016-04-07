//
// Copyright (C)      2016  Kisuk Lee           <kisuklee@mit.edu>
// ---------------------------------------------------------------
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
#pragma once

#include "dataset.hpp"
#include "volume_data.hpp"
#include "box.hpp"

namespace znn { namespace v4 {

template<typename T>
class volume_dataset: public dataset<T>
{
private:
    typedef std::map<std::string, std::pair<vec3i,size_t>>  layers_spec_t;
    typedef std::map<std::string, vec3i>                    sample_spec_t;
    typedef std::unique_ptr<tensor_data<T>>                 tensor_data_p;

private:
    std::map<std::string, tensor_data_p>    dataset_     ;
    sample_spec_t                           sample_spec_ ;
    box                                     sample_range_;

public:
    box const & range() const { return range_; }


public:
    sample<T> random_sample() override
    {
        ZI_ASSERT(!sample_spec_.empty());
        return get_sample(random_location());
    }

    sample<T> next_sample() override
    {
        ZI_ASSERT(!sample_spec_.empty());
        // TODO(lee): temporary implementation
        return random_sample();
    }

public:
    sample<T> get_sample( vec3i const & loc )
    {
        ZI_ASSERT(!sample_spec_.empty());

        sample<T> ret;

        for ( auto& s: sample_spec_ )
        {
            auto const & name = s.first;
            ZI_ASSERT(dataset_.count(name)!=0);
            ret[name] = dataset_[name]->get_patch(loc);
        }

        return ret;
    }

private:
    vec3i random_location() const
    {
        vec3i size = range_.size();

        // TODO(lee): seeding issue
        auto z = std::rand() % size[0];
        auto y = std::rand() % size[1];
        auto x = std::rand() % size[2];

        return range_.min() + vec3i(z,y,x);
    }


public:
    void set_spec( layers_spec_t const & spec )
    {
        sample_spec_t ss;
        for ( auto& s: spec )
            ss[s.first] = s.second.first;
        set_spec(ss);
    }

    void set_spec( sample_spec_t const & spec )
    {
        sample_spec_ = spec;
        update_range();
    }

private:
    void update_range()
    {
        range_ = box(); // empty box

        for ( auto& layer: sample_spec_ )
        {
            auto const & name = layer.first;
            auto const & dim  = layer.second;

            ZI_ASSERT(dataset_.count(name)!=0);
            ZI_ASSERT(dataset_[name]->size()!=0);

            // update patch size
            dataset_[name]->set_fov(dim);

            // update valid range
            box const & range = dataset_[name]->range();
            range_ = range_.empty() ? range : range_.intersect(range);
        }
    }


public:
    // cube
    void add_data( std::string const & name,
                   cube_p<T> const & data,
                   vec3i const & offset = vec3i::zero )
    {
        tensor<T> t = {data};
        add_data(name, t, offset);
    }

    // tensor
    void add_data( std::string const & name,
                   tensor<T> const & data,
                   vec3i const & offset = vec3i::zero )
    {
        dataset_[name] =
            std::make_unique<tensor_data<T>>(data, vec3i::zero, offset);
    }


public:
             volume_dataset() {}
    virtual ~volume_dataset() {}

}; // class volume_dataset

}} // namespace znn::v4