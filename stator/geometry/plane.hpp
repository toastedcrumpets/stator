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
#include "stator/geometry/object.hpp"

namespace stator {
  namespace geometry {
    /*! \brief A half-space.

      A half-space is the volume on one side of a infinite planar
      surface. A plane is defined through a normal and a point on the
      plane (origin), and by definition the normal points away from
      the volume.
      
      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the ball.
    */
    template<typename Scalar, size_t D>
    class HalfSpace {
    public:
      HalfSpace(const Vector<Scalar, D>& origin, const Vector<Scalar, D>& normal):
	origin_(origin), normal_(normal)
      {}
      
      const Vector<Scalar, D>& origin() const { return origin_; }
      const Vector<Scalar, D>& normal() const { return normal_; }

      protected:
      Vector<Scalar, D> origin_;
      Vector<Scalar, D> normal_;
    };
  }// namespace geometry
}// namespace stator