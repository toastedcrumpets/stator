/*
  Copyright (C) 2021 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#include <stator/symbolic/symbolic.hpp>

namespace sym
{
  template <class LHS, class RHS>
  auto units(const LHS &l, const RHS &r);
  namespace detail
  {
    struct Units;
  }

  template <class LHS, class RHS>
  using UnitsOp = BinaryOp<LHS, detail::Units, RHS>;

  namespace detail
  {
    struct Units
    {
      static constexpr int leftBindingPower = 60;
      static constexpr auto associativity = Associativity::LEFT;
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      static constexpr bool wrapped = true;
      typedef NoIdentity left_identity;
      typedef Unity right_identity;
      typedef Null left_zero;
      typedef NoIdentity right_zero;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "{"; }
      static inline std::string r_repr() { return "}"; }
      static inline std::string l_latex_repr() { return ""; }
      static inline std::string latex_repr() { return "\\left\\{"; }
      static inline std::string r_latex_repr() { return "\\right\\}"; }
      template <class L, class R>
      static auto apply(const L &l, const R &r)
      {
        return units(l, r);
      }
      static constexpr int type_index = 18;
    };

    template <class LHS, class RHS>
    auto units_impl(const LHS &l, const RHS &r, last_choice)
    {
      //When all else fails, just return the binary op, storing the units for later
      return UnitsOp<decltype(store(l)), decltype(store(r))>::create(l, r);
    }


    /*
    template <class ULHS, class URHS, class RHS>
    auto units_impl(const UnitsOp<ULHS, URHS> &l, const RHS &r, choice<1>)
    {
      //This is a unit conversion of l to the units of r.
      stator_throw() << "Conversion not yet supported!";
    }
    */
  }

  /*! \brief Actual application of units to an expression. */
  template <class LHS, class RHS>
  auto units(const LHS &l, const RHS &r)
  {
    return detail::units_impl(l, r, detail::select_overload{});
  }

  template<class LLHS, class LRHS, class RLHS, class RRHS>
  auto operator*(const UnitsOp<LLHS, LRHS>& l, const UnitsOp<RLHS, RRHS>& r)  {
    return units(l._l * r._l, l._r * r._r);
  }
}