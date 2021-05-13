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

namespace sym {
  namespace detail {
    struct Units {
      static constexpr int leftBindingPower = 60;
      static constexpr auto associativity = Associativity::LEFT;
      static constexpr bool commutative = true;
      static constexpr bool associative = true;
      typedef NoIdentity left_identity;
      typedef Unity right_identity;
      typedef Null left_zero;
      typedef NoIdentity right_zero;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "{"; }
      static inline std::string r_repr() { return "}"; }
      static inline std::string l_latex_repr() { return ""; }
      static inline std::string latex_repr() { return "\left\{"; }
      static inline std::string r_latex_repr() { return "\right\}"; }
      template<class L, class R> static auto apply(const L& l, const R& r) {
	return BinaryOp<decltype(store(l)), detail::Units, decltype(store(r))>::create(l, r);
      }
      static constexpr int type_index = 18;
    };
  }
  
}
