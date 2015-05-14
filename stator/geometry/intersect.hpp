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
#include "stator/geometry/indicator.hpp"

namespace stator {
  namespace geometry {
    template<typename Obj1, typename Obj2>
    bool intersects(const Obj1& b1, const Obj2& b2) {
      return indicator(b1, b2, stator::symbolic::NullSymbol()) < 0;
    }
  } // namespace geometry
} // namespace stator
