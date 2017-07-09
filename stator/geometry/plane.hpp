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

namespace stator {
  namespace geometry {
    /*! \brief A planar half-space (a plane and the volume on one side
        of the plane).

      A half-space represents the volume on one side of a infinite
      planar surface. A plane is defined through a normal and a point
      on the plane (origin), and (by definition) the normal points
      away from the represented volume.
      
      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the HalfSpace.
    */
    template<typename Scalar, size_t D>
    class HalfSpace {
    public:
      HalfSpace(const Vector<Scalar, D>& center, const Vector<Scalar, D>& normal):
	center_(center), normal_(normal)
      {}
      
      const Vector<Scalar, D>& center() const { return center_; }
      const Vector<Scalar, D>& normal() const { return normal_; }

      protected:
      Vector<Scalar, D> center_;
      Vector<Scalar, D> normal_;
    };

    /*! \brief An infinite flat surface of fixed thickness.

      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the Plane.
    */
    template<typename Scalar, size_t D>
    class Plane {
    public:
      Plane(const Vector<Scalar, D>& center, const Vector<Scalar, D>& normal, const Scalar thickness = Scalar(0)):
	center_(center), normal_(normal), thickness_(thickness)
      {}
      
      const Vector<Scalar, D>& center() const { return center_; }
      const Vector<Scalar, D>& normal() const { return normal_; }
      const Vector<Scalar, D>& thickness() const { return thickness_; }

      protected:
      Vector<Scalar, D> center_;
      Vector<Scalar, D> normal_;
      Scalar thickness_;
    };

    
  }// namespace geometry
}// namespace stator
