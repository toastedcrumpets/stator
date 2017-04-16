/*
  Copyright (C) 2015 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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
#include <stator/symbolic/symbolic.hpp>

namespace sym {
  template<class ...Args>
  inline std::ostream& operator<<(std::ostream& os, const Var<Args...>&) {
    os << Var<Args...>::idx;
    return os;
  }

  template<class Var, class Arg>
  inline std::ostream& operator<<(std::ostream& os, const Relation<Var, Arg>& sub) {
    os << Var::idx << " <- " << sub._val;
    return os;
  }
  
  template<class Arg, class Op>
  inline std::ostream& operator<<(std::ostream& os, const UnaryOp<Arg, Op>& f)
  { return os << Op::_str_left << f._arg << Op::_str_right; }  

  template<class LHS, class RHS, class Op>
  inline std::ostream& operator<<(std::ostream& os, const BinaryOp<LHS, Op, RHS>& op) {
    os << "(" << op._l << " " << Op::str() <<  " " << op._r << ")";
    return os;
  }

  /*! \relates Polynomial 
    \name Polynomial input/output operations
    \{
  */
  /*! \brief Writes a human-readable representation of the Polynomial to the output stream. */
  template<class Coeff_t, size_t N, class PolyVar>
  inline std::ostream& operator<<(std::ostream& os, const Polynomial<N, Coeff_t, PolyVar>& poly) {
    std::ostringstream oss;
    oss.precision(os.precision());
    size_t terms = 0;
    oss << "P(";
    for (size_t i(N); i != 0; --i) {
	if (poly[i] == empty_sum(poly[i])) continue;
	if (terms != 0)
	  oss << " + ";
	++terms;
      detail::print_coeff(oss, poly[i]);
	oss << "*" << PolyVar::idx;
	if (i > 1)
	  oss << "^" << i;
    }
    if ((poly[0] != empty_sum(poly[0])) || (terms == 0)) {
	if (terms != 0)
	  oss << " + ";
	++terms;
	detail::print_coeff(oss, poly[0]);
    }
    os << oss.str() << ")";
    return os;
  }
  /*! \} */

  namespace detail {
    template<size_t Order, class Coeff_t, class PolyVar>
    std::ostream& operator<<(std::ostream& os, const SturmChain<Order, Coeff_t, PolyVar>& c) {
      os << "SturmChain{p_0=" << c._p_n;
	c._p_nminus1.output_helper(os, Order);
	os << "}";
	return os;
    }
  }
}
