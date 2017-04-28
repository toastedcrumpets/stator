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

namespace stator {
  template<class Config = DefaultReprConfig, std::intmax_t Num, std::intmax_t Denom>
  inline std::string repr(const sym::C<Num, Denom>)
  { return "C<" + repr(Num) + ", " + repr (Denom) + ">"; }

  template<class Config = DefaultReprConfig, class ...Args>
  inline std::string repr(const sym::Var<Args...>& v) {
    return std::string(1, v.getidx());
  }

  template<class Config = DefaultReprConfig, class Var, class Arg>
  inline std::string repr(const sym::Relation<Var, Arg>& sub) {
    return repr<Config>(sub._var) << "=" << repr<Config>(sub._val);
  }
  
  template<class Config = DefaultReprConfig, class Arg, class Op>
  inline std::string repr(const sym::UnaryOp<Arg, Op>& f)
  { return std::string(Op::_repr_left)+repr<Config>(f._arg) + Op::_repr_right; }

  template<class Config = DefaultReprConfig, class T, typename = typename std::enable_if<std::is_base_of<Eigen::EigenBase<T>, T>::value>::type>
  std::string repr(const T& val) {
    std::ostringstream os;
    if ((val.cols() == 1) && (val.rows()==1))
      os << val;
    else if (val.cols() == 1) {
      os << "{ ";
      for (int i(0); i < val.rows(); ++i)
	os << val(i, 0) << " ";
      os << "}^T";
    } else {
      os << "{ ";
      for (int i(0); i < val.cols(); ++i) {
	os << "{ ";
	for (int j(0); j < val.rows(); ++j)
	  os << val(i, j) << " ";
	os << "} ";
      }
      os << "}";
    }
    return os.str();
  }
  
  /*! \relates Polynomial 
    \name Polynomial input/output operations
    \{
  */
  /*! \brief Returns a human-readable representation of the Polynomial. */
  template<class Config = DefaultReprConfig, class Coeff_t, size_t N, class PolyVar>
  inline std::string repr(const sym::Polynomial<N, Coeff_t, PolyVar>& poly) {
    std::ostringstream oss;
    size_t terms = 0;
    oss << "P(";
    for (size_t i(N); i != 0; --i) {
      if (poly[i] == sym::empty_sum(poly[i])) continue;
	if (terms != 0)
	  oss << " + ";
	++terms;
	oss << repr<Config>(poly[i]) << "*" << PolyVar::idx;
	if (i > 1)
	  oss << "^" << i;
    }
    if ((poly[0] != sym::empty_sum(poly[0])) || (terms == 0)) {
	if (terms != 0)
	  oss << " + ";
	++terms;
	oss << repr<Config>(poly[0]);
    }
    oss << ")";
    return oss.str();
  }
  /*! \} */

  
  namespace detail {
    /*! \brief Returns the binding powers (precedence) of binary operators.
      
      As unary operators/tokens have no arguments which can be bound
      by other operators, we return a large binding power (which
      should exclude them from any binding power calculations).
     */
    template<class T>
    std::pair<int, int> BP (const T& v)
    { return std::make_pair(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()); }

    /*! \brief Returns the binding powers (precedence) of binary
        operators (specialisation for binary ops).
     */
    template<class LHS, class Op, class RHS>
    std::pair<int, int> BP (const sym::BinaryOp<LHS, Op, RHS>& v) {
      const int L = Op::leftBindingPower;
      const int R = sym::detail::RBP<Op>();
      return std::make_pair(L, R);
    }

    /*! \brief Binding power visitor for sym::detail::BP(const Expr&). */
    struct BPVisitor : public sym::detail::VisitorHelper<BPVisitor> {
      
      template<class T> sym::Expr apply(const T& rhs) { return sym::Expr(); }

      template<class Op> sym::Expr apply(const sym::BinaryOp<sym::Expr, Op, sym::Expr>& op) {
	LBP = Op::leftBindingPower;
	RBP = sym::detail::RBP<Op>();
	return sym::Expr();
      }
      
      int LBP = std::numeric_limits<int>::max();
      int RBP = std::numeric_limits<int>::max();
    };

    /*! \brief Returns the binding powers (precedence) of binary
        operators (specialisation for Expr).
     */
    std::pair<int, int> BP (const sym::Expr& v) {
      BPVisitor vis;
      v->visit(vis);
      return std::make_pair(vis.LBP, vis.RBP);
    }
  }

  template<class Config = DefaultReprConfig, class LHS, class RHS, class Op>
  inline std::string repr(const sym::BinaryOp<LHS, Op, RHS>& op) {
    const auto this_BP = detail::BP(op);
    const auto LHS_BP = detail::BP(op._l);
    const auto RHS_BP = detail::BP(op._r);

    std::string retval;
    
    bool parenL = LHS_BP.second < this_BP.first;
    if (parenL) retval = "(";
    retval = retval + repr<Config>(op._l);
    if (parenL) retval = retval + ")";

    retval = retval + Op::repr();
    
    bool parenR = this_BP.second > RHS_BP.first;
    if (parenR) retval = retval + "(";
    retval = retval + repr<Config>(op._r);
    if (parenR) retval = retval + ")";
    
    return retval;
  }


  namespace detail {
    template<class Config>
    struct ReprVisitor : public sym::detail::VisitorHelper<ReprVisitor<Config> > {
      template<class T> sym::Expr apply(const T& rhs) {
	_repr = repr<Config>(rhs);
	return sym::Expr();
      }

      std::string _repr;
    };
  }

  template<class Config>
  std::string repr(const sym::RTBase& b) {
    detail::ReprVisitor<Config> visitor;
    b.visit(visitor);
    return visitor._repr;
  }

  template<class Config>
  std::string repr(const sym::Expr& b) {
    return repr<Config>(*b);
  }
}

namespace sym {
  template<class T, typename = typename std::enable_if<sym::IsSymbolic<T>::value>::type>
  std::ostream& operator<<(std::ostream& os, const T& v) {
    return os << stator::repr(v);
  }
}
