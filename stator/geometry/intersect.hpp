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

    /*! \brief Convenience overload for \ref intersects(const Sphere<Scalar, D>& b1, const Ball<Scalar,D>& b2).
     */
    template<typename Scalar, size_t D>
    bool intersects(const Ball<Scalar, D>& b1, const Sphere<Scalar, D>& b2) {
      return intersects(b2, b1);
    }

    /*! \brief Test if a Spherical surface and a Ball volume intersect.
      
      A spherical surface and a ball only intersect if they are at
      least within touching distance:

      \f[
      \left|\bm{r}_{ij}\right| < \frac{1}{2}(\sigma_i + \sigma_j)
      \f]
      
      However, if the spherical shell (i) has a larger diameter than
      the ball (\f$\sigma_i > \sigma_j\f$), then it may surround the
      ball without intersecting it. This adds the additional constraint:

      \f{align*}{
      \left|\bm{r}_{ij}\right| &> \frac{1}{2}(\sigma_i - \sigma_j)
      \f}

      Provided we test that the spherical shell is indeed larger than
      the ball before performing the second test we can avoid taking a
      square root.
     */
    template<typename Scalar, size_t D>
    bool intersects(const Sphere<Scalar, D>& b1, const Ball<Scalar, D>& b2) {
      Scalar sqdist = (b1.center() - b2.center()).squaredNorm();
      return (sqdist < std::pow((b1.radius() + b2.radius()), 2)) 
        && ((b2.radius() > b1.radius()) 
            || (sqdist >= std::pow(b1.radius() - b2.radius(), 2)));
    }
  } // namespace geometry
} // namespace stator
