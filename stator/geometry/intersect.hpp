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

namespace stator {
  namespace geometry {
    /*! \brief Test if two Ball volumes intersect.
      
      This is a simple distance test:
      
      \f[
      \left| \bm{r}_{ij}\right| < \frac{1}{2}(\sigma_i + \sigma_j)
      \f]

      As both sides are always positive, we can avoid taking the square
      root and just square both sides:

      \f[
      r_{ij}^2 < \frac{1}{4}(\sigma_i + \sigma_j)^2
      \f]
    */
    template<typename Scalar, size_t D>
    bool intersects(const Ball<Scalar, D>& b1, const Ball<Scalar, D>& b2) {
      return (b1.center() - b2.center()).squaredNorm() <= std::pow((b1.radius() + b2.radius()), 2);
    }

  } // namespace geometry
} // namespace stator
