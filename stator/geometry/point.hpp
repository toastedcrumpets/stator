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
#include "stator/config.hpp"
#include "stator/constants.hpp"

namespace stator {
  namespace geometry {
    /*! \brief A point.
      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the ball.
    */
    template<typename Scalar, size_t D>
    class Point {
    public:
      /*! \brief Default constructor.
        
        This constructor deliberately leaves the object uninitialised
        to allow convenient stack-based construction without
        comprimising the detection of uninitialised accesses by tools
        such as valgrind.
      */
      Point() {}
      
      /*! \brief RAII constructor. */
      Point(const Vector<Scalar, D>& center = Vector<Scalar, D>::Zero().eval()): center_(center) {}
      
      /*! \brief Get function for the point's location. */
      const Vector<Scalar, D>& center() const { return center_; }

    protected:
      /*! \brief Center of the ball. */
      Vector<Scalar, D> center_;
    };

    template<typename Scalar, size_t D>
    constexpr Scalar volume(const Point<Scalar, D>& ball) { return Scalar(0); }

    template<typename Scalar, size_t D>
    Scalar area(const Point<Scalar, D>& sphere) { return Scalar(0); }

  } // namespace geometry
} // namespace stator

