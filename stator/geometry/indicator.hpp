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
#include "stator/symbolic/symbolic.hpp"

namespace stator {
  namespace geometry {
    using namespace stator::symbolic;
    
    template<class Scalar, size_t D, class VijFunc>
    auto indicator(const Ball<Scalar, D>& bi,  const Point<Scalar, D>& bj, const VijFunc& vij) 
      -> STORETYPE(pow<2>(try_simplify(vij * Variable<'t'>() + bi.center() - bj.center())) - pow<2>(bi.radius())) {
      return pow<2>(try_simplify(vij * Variable<'t'>() + bi.center() - bj.center())) - bi.radius()*bi.radius();
    }

    template<class Scalar, size_t D, class VijFunc>
    auto indicator(const Ball<Scalar, D>& bi,  const Ball<Scalar, D>& bj, const VijFunc& vij)
      -> STORETYPE(pow<2>(try_simplify(vij * Variable<'t'>() + bi.center() - bj.center())) - pow<2>(bi.radius() + bj.radius())) {
      return pow<2>(try_simplify(vij * Variable<'t'>() + bi.center() - bj.center())) - pow<2>(bi.radius() + bj.radius());
    }

  } // namespace geometry
} // namespace stator
