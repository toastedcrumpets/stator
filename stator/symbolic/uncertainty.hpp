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

namespace sym {
   template<class LHS, class RHS> auto uncertainty(const LHS& l, const RHS& r);

   namespace detail {
    struct Uncertainty {
      static constexpr int leftBindingPower = 70;
      static constexpr auto associativity = Associativity::LEFT;
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      static constexpr bool wrapped = false;
      typedef NoIdentity left_identity;
      typedef Null right_identity;
      typedef NoIdentity left_zero;
      typedef NoIdentity right_zero;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "±"; }
      static inline std::string r_repr() { return ""; }
      static inline std::string l_latex_repr() { return ""; }
      static inline std::string latex_repr() { return "\\pm"; }
      static inline std::string r_latex_repr() { return ""; }
      template<class L, class R> static auto apply(const L& l, const R& r) {
        return uncertainty(l,r);
      }
      static constexpr int type_index = 19;
    };
  
    template<class LHS, class RHS>
    auto uncertainty_impl(const LHS& l, const RHS& r, last_choice) {
      return BinaryOp<decltype(store(l)), detail::Uncertainty, decltype(store(r))>::create(l, r);
    }
  }

  template<class LHS, class RHS>
  auto uncertainty(const LHS& l, const RHS& r) {
    return detail::uncertainty_impl(l, r, detail::select_overload{});
  }
}
