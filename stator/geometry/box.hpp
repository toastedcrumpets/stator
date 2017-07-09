/*
  Copyright (C) 2017 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

  This file is part of stator.

  stator is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  stator is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with stator. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// stator
#include "stator/constants.hpp"
#include "stator/config.hpp"

namespace stator {
  namespace geometry {
    /*! \brief An axis-aligned box/block (including its interior volume).
      
      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the ball.
     */
    template<typename Scalar, size_t D>
    class AABox {
    public:
      AABox(const Vector<Scalar, D>& max, const Vector<Scalar, D>& min):
        max_(max), min_(min)
      {}
      
      const Vector<Scalar, D>& max() const { return max_; }
      const Vector<Scalar, D>& min() const { return min_; }
      
      Vector<Scalar, D> dimensions() const { return max_ - min_; }
      
    protected:
      Vector<Scalar, D> max_;
      Vector<Scalar, D> min_;
    };

    /*! \brief Calculate the volume of an Axis-Aligned Box.*/
    template<typename Scalar, size_t D>
    Scalar volume(const AABox<Scalar, D>& bb) {
      auto extent = bb.dimensions();
      Scalar measure = extent[0];
      for (size_t i(1); i < D; ++i) 
        measure *= extent[i];
      return measure;
    }

    /*! \brief Calculate the surface area of an Axis-Aligned Box.*/
    template<typename Scalar, size_t D>
    Scalar area(const AABox<Scalar, D>& bb) {
      auto extent = bb.dimensions();
      Scalar measure = 0;
      for (size_t i(0); i < D; ++i)
        for (size_t j(i+1); j < D; ++j)
          measure += extent[i] * extent[j];
      return 2 * measure;
    }
  } // namespace geometry
} //namespace stator
