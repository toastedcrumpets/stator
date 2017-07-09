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
#include "stator/geometry/sphere.hpp"
#include "stator/geometry/point.hpp"
#include "stator/geometry/plane.hpp"
#include "stator/symbolic/symbolic.hpp"

namespace stator {
  namespace geometry {
    using namespace stator::symbolic;
    
    /*! \brief Ball-Point indicator function.*/
    template<class Scalar, size_t D, class DeltaRijFunc>
    auto indicator(const Ball<Scalar, D>& bi,  const Point<Scalar, D>& bj, const DeltaRijFunc& deltarij) 
      -> STATOR_AUTORETURN_BYVALUE(pow<2>(try_simplify(deltarij + bi.center() - bj.center())) - pow<2>(bi.radius()));
    
    /*! \brief Point-Ball indicator function.*/
    template<class Scalar, size_t D, class DeltaRijFunc>
    auto indicator(const Point<Scalar, D>& bi, const Ball<Scalar, D>& bj, const DeltaRijFunc& deltarij) 
      -> STATOR_AUTORETURN_BYVALUE(indicator(bj, bi, -deltarij));

    /*! \brief Ball-Ball indicator function.*/
    template<class Scalar, size_t D, class DeltaRijFunc>
    auto indicator(const Ball<Scalar, D>& bi,  const Ball<Scalar, D>& bj, const DeltaRijFunc& deltarij)
      -> STATOR_AUTORETURN_BYVALUE(pow<2>(try_simplify(deltarij + bi.center() - bj.center())) - pow<2>(bi.radius() + bj.radius()));

    /*! \brief Ball-HalfSpace indicator function.*/
    template<class Scalar, size_t D, class DeltaRijFunc>
    auto indicator(const Ball<Scalar, D>& bi, const HalfSpace<Scalar, D>& bj, const DeltaRijFunc& deltarij)
      -> STATOR_AUTORETURN_BYVALUE(try_simplify(dot(bj.normal(), deltarij + bi.center() - bj.center()) - bi.radius()));

    /*! \brief HalfSpace-Ball indicator function.*/
    template<class Scalar, size_t D, class DeltaRijFunc>
    auto indicator(const HalfSpace<Scalar, D>& bi, const Ball<Scalar, D>& bj, const DeltaRijFunc& deltarij)
      -> STATOR_AUTORETURN_BYVALUE(indicator(bj, bi, -deltarij));

    /*! \brief Generic implementation of an intersection test for
     shapes with indicator functions defined.
     */
    template<typename Obj1, typename Obj2>
    auto intersects(const Obj1& b1, const Obj2& b2)
    -> STATOR_AUTORETURN(indicator(b1, b2, Null()) < 0)

  } // namespace geometry
} // namespace stator
