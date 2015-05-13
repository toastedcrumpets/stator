/*
  Copyright (C) 2015 Marcus Bannerman <m.bannerman@gmail.com>

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
#include "stator/geometry/sphere.hpp"
#include "stator/geometry/point.hpp"

namespace stator {
  namespace geometry {
    /*! \brief Test if a point intersects with a ball volume.*/
    template<typename Scalar, size_t D>
    bool intersects(const Ball<Scalar, D>& b1, const Point<Scalar, D>& b2) {
      return (b1.center() - b2.center()).squaredNorm() <= std::pow(b1.radius(), 2);
    }

    /*! \brief Test if a point intersects with a ball volume.*/
    template<typename Scalar, size_t D>
    bool intersects(const Point<Scalar, D>& b1, const Ball<Scalar, D>& b2) {
      return intersects(b2, b1);
    }

    /*! \brief Test if two ball volumes intersect.*/
    template<typename Scalar, size_t D>
    bool intersects(const Ball<Scalar, D>& b1, const Ball<Scalar, D>& b2) {
      return (b1.center() - b2.center()).squaredNorm() <= std::pow(b1.radius() + b2.radius(), 2);
    }
    

  } // namespace geometry
} // namespace stator
