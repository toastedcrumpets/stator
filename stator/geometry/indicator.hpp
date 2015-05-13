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
    namespace sym = stator::symbolic;

    template<class Scalar, size_t D, class VijFunc>
    auto indicator(const Ball<Scalar, D>& bi,  const Point<Scalar, D>& bj, const VijFunc& vij) -> decltype(sym::simplify(sym::pow(vij + bi.center() - bj.center(), 2) - sym::pow(bi.radius(), 2))) {
      return sym::simplify(sym::pow(vij + bi.center() - bj.center(), 2) - sym::pow(bi.radius(), 2));
    }

    template<class Scalar, size_t D, class VijFunc>
    auto indicator(const Ball<Scalar, D>& bi,  const Ball<Scalar, D>& bj, const VijFunc& vij) -> decltype(sym::simplify(sym::pow<2>(vij + bi.center() - bj.center()) - sym::pow<2>(bi.radius() + bj.radius()))) {
      return sym::simplify(sym::pow<2>(vij + bi.center() - bj.center()) - sym::pow<2>(bi.radius() + bj.radius()));
    }

  } // namespace geometry
} // namespace stator
