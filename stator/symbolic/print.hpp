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
  {
    if (Config::Debug_output)
      return "C<" + repr(Num) + ((Denom!=1) ? (std::string(", ") + repr(Denom) + ">") : std::string(">"));
    else
      return (Denom!=1) ? ("("+repr(Num)+"/"+repr(Denom)+")") : repr(Num);
      
  }

  template<class Config = DefaultReprConfig, class ...Args>
  inline std::string repr(const sym::Var<Args...>& v) {
    return std::string(1, v.getidx());
  }

  template<class Config = DefaultReprConfig, class Var, class Arg>
  inline std::string repr(const sym::Relation<Var, Arg>& sub) {
    return repr<Config>(sub._var) << "=" << repr<Config>(sub._val);
  }
    
  template<class Config = DefaultReprConfig, class Arg>
  inline std::string repr(const sym::UnaryOp<Arg, sym::detail::Sine>& f)
  {
    return std::string((Config::Latex_output) ? "\\sin\\left(" : "sin(")
      + repr<Config>(f._arg)
      + std::string((Config::Latex_output) ? "\\right)" : ")")
      ;
  }

  template<class Config = DefaultReprConfig, class Arg>
  inline std::string repr(const sym::UnaryOp<Arg, sym::detail::Cosine>& f)
  {
    return std::string((Config::Latex_output) ? "\\cos\\left(" : "cos(")
      + repr<Config>(f._arg)
      + std::string((Config::Latex_output) ? "\\right)" : ")")
      ;
  }

  template<class Config = DefaultReprConfig, class Arg>
  inline std::string repr(const sym::UnaryOp<Arg, sym::detail::Exp>& f)
  {
    return
      std::string((Config::Latex_output) ? "\\mathrm{e}^{" : "exp(")
      + repr<Config>(f._arg)
      + std::string((Config::Latex_output) ? "}" : ")")
      ;
  }

  template<class Config = DefaultReprConfig, class Arg>
  inline std::string repr(const sym::UnaryOp<Arg, sym::detail::Log>& f)
  {
    return
      std::string((Config::Latex_output) ? "\\ln\\left(" : "ln(")
      + repr<Config>(f._arg)
      + std::string((Config::Latex_output) ? "\\right)" : ")")
      ;
  }

  template<class Config = DefaultReprConfig, class Arg>
  inline std::string repr(const sym::UnaryOp<Arg, sym::detail::Absolute>& f)
  {
    return
      std::string((Config::Latex_output) ? "\\left|" : "|")
      + repr<Config>(f._arg)
      + std::string((Config::Latex_output) ? "\\right|" : "|")
      ;
  }
  
  template<class Config = DefaultReprConfig, class Arg>
  inline std::string repr(const sym::UnaryOp<Arg, sym::detail::Arbsign>& f)
  {
    return
      std::string((Config::Latex_output) ? "\\pm\\left|" : "±|")
      + repr<Config>(f._arg)
      + std::string((Config::Latex_output) ? "\\right|" : "|")
      ;
  }

  template<class Config = DefaultReprConfig, class T, typename = typename std::enable_if<std::is_base_of<Eigen::EigenBase<T>, T>::value>::type>
  std::string repr(const T& val) {
    std::ostringstream os;
    if ((val.cols() == 1) && (val.rows()==1))
      os << repr<Config>(val(0,0));
    else if (val.cols() == 1) {
      os << "{ ";
      for (int i(0); i < val.rows(); ++i)
	os << repr<Config>(val(i, 0)) << " ";
      os << "}^T";
    } else {
      os << "{ ";
      for (int i(0); i < val.cols(); ++i) {
	os << "{ ";
	for (int j(0); j < val.rows(); ++j)
	  os << repr<Config>(val(i, j)) << " ";
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
    inline std::pair<int, int> BP (const sym::Expr& v) {
      BPVisitor vis;
      v->visit(vis);
      return std::make_pair(vis.LBP, vis.RBP);
    }

    template<class Config>
    std::string paren_wrap(std::string arg) {
      return ((Config::Latex_output) ? "\\left(" : "(") + arg + ((Config::Latex_output) ? "\\right)" : ")");
    }
  }

  template<class Config = DefaultReprConfig>
  inline std::string repr(const sym::detail::Multiply& op) {
    return (Config::Latex_output) ? "*" : "\\times ";
  }
  
  template<class Config = DefaultReprConfig, class LHS, class RHS, class Op>
  inline std::string repr(const sym::BinaryOp<LHS, Op, RHS>& op) {
    const auto this_BP = detail::BP(op);
    const auto LHS_BP = detail::BP(op._l);
    const auto RHS_BP = detail::BP(op._r);

    std::string LHS_repr = repr<Config>(op._l);
    if (LHS_BP.second < this_BP.first || Config::Force_parenthesis) LHS_repr = detail::paren_wrap<Config>(LHS_repr);

    std::string RHS_repr = repr<Config>(op._r);
    if (this_BP.second > RHS_BP.first || Config::Force_parenthesis) RHS_repr = detail::paren_wrap<Config>(RHS_repr);

    return (Config::Latex_output ? Op::l_latex_repr() : Op::l_repr()) +  LHS_repr + (Config::Latex_output ? Op::latex_repr() : Op::repr()) + RHS_repr + (Config::Latex_output ? Op::r_latex_repr() : Op::r_repr());
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
