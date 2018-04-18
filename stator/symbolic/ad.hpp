/*! \file ad.hpp
  \brief Automatic differentiation header
*/
/*
  Copyright (C) 2018 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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
  template<size_t Nd, typename T, typename Var, typename Arg,
	   typename = typename std::enable_if<detail::IsConstant<T>::value>::type>
  Eigen::Matrix<double, Nd+1,1> ad(const T& v, const Relation<Var, Arg>&) {
    Eigen::Matrix<double, Nd+1,1> r = Eigen::Matrix<double, Nd+1,1>::Zero();
    r[0] = v;
    return r;
  }

  template<size_t Nd, typename ...Args, typename Var_t, typename Arg_t>
  Eigen::Matrix<double, Nd+1,1> ad(const Var<Args...>& v, const Relation<Var_t, Arg_t>& r) {

    Eigen::Matrix<double, Nd+1,1> result = Eigen::Matrix<double, Nd+1,1>::Zero();    
    if ((v.getidx() == r._var.getidx())) {
      result[0] = r._val;
      if (Nd > 0) result[1] = 1.0;
    } else
      result[0] = std::numeric_limits<double>::quiet_NaN();
    
    return result;
  }

  template<size_t Nd, typename LHS_t, typename RHS_t, typename Var_t, typename Arg_t>
  Eigen::Matrix<double, Nd+1,1> ad(const BinaryOp<LHS_t, detail::Add, RHS_t>& op, const Relation<Var_t, Arg_t>& r) {
    return ad<Nd>(op._l, r) + ad<Nd>(op._r, r);
  }

  template<size_t Nd, typename LHS_t, typename RHS_t, typename Var_t, typename Arg_t>
  Eigen::Matrix<double, Nd+1,1> ad(const BinaryOp<LHS_t, detail::Subtract, RHS_t>& op, const Relation<Var_t, Arg_t>& r) {
    return ad<Nd>(op._l, r) - ad<Nd>(op._r, r);
  }
  
  template<size_t Nd, typename LHS_t, typename RHS_t, typename Var_t, typename Arg_t>
  Eigen::Matrix<double, Nd+1,1> ad(const BinaryOp<LHS_t, detail::Multiply, RHS_t>& op, const Relation<Var_t, Arg_t>& sub) {
    Eigen::Matrix<double, Nd+1,1> l = ad<Nd>(op._l, sub);
    Eigen::Matrix<double, Nd+1,1> r = ad<Nd>(op._r, sub);
    Eigen::Matrix<double, Nd+1,1> result = Eigen::Matrix<double, Nd+1,1>::Zero();

    result[0] = l[0] * r[0];
    for (size_t k(1); k < Nd+1; ++k)
      for (size_t i(0); i <= k; ++i)
	result[k] += l[i] * r[k-i];
    
    return result;
  }

  template<size_t Nd, typename LHS_t, typename RHS_t, typename Var_t, typename Arg_t>
  Eigen::Matrix<double, Nd+1,1> ad(const BinaryOp<LHS_t, detail::Divide, RHS_t>& op, const Relation<Var_t, Arg_t>& sub) {
    Eigen::Matrix<double, Nd+1,1> l = ad<Nd>(op._l, sub);
    Eigen::Matrix<double, Nd+1,1> r = ad<Nd>(op._r, sub);
    Eigen::Matrix<double, Nd+1,1> result = Eigen::Matrix<double, Nd+1,1>::Zero();

    result[0] = l[0] / r[0];
    
    for (size_t k(1); k < Nd+1; ++k) {
      result[k] = l[k];
      for (size_t i(0); i < k; ++i)
	result[k] -= result[i] * r[k - i];
      result[k] /= r[0];
    }
    return result;
  }

  template<size_t Nd, typename LHS_t, typename RHS_t, typename Var_t, typename Arg_t,
	   typename = typename std::enable_if<detail::IsConstant<RHS_t>::value>::type>
  Eigen::Matrix<double, Nd+1,1> ad(const BinaryOp<LHS_t, detail::Power, RHS_t>& op, const Relation<Var_t, Arg_t>& sub) {
    Eigen::Matrix<double, Nd+1,1> g = ad<Nd>(op._l, sub);
    double a = op._r;
    Eigen::Matrix<double, Nd+1,1> result = Eigen::Matrix<double, Nd+1,1>::Zero();

    result[0] = std::pow(g[0], a);
    
    for (size_t k(1); k < Nd+1; ++k) {
      for (size_t i(1); i <= k; ++i)
	result[k] += ((a + 1) * i / k - 1) * g[i] * result[k - i];
      result[k] /= g[0];
    }
    return result;
  }
  
  template<size_t Nd, typename Arg, typename Var_t, typename Arg_t>
  Eigen::Matrix<double, Nd+1,1> ad(const UnaryOp<Arg, detail::Exp>& op, const Relation<Var_t, Arg_t>& sub) {
    Eigen::Matrix<double, Nd+1,1> g = ad<Nd>(op._arg, sub);

    Eigen::Matrix<double, Nd+1,1> result = Eigen::Matrix<double, Nd+1,1>::Zero();    
    result[0] = sym::exp(g[0]);
    
    for (size_t k(1); k < Nd+1; ++k) {
      for (size_t i(1); i <= k; ++i)
	result[k] += i * g[i] * result[k-i];
      result[k] /= k;
    }
    return result;
  }

  template<size_t Nd, typename Arg, typename Var_t, typename Arg_t>
  Eigen::Matrix<double, Nd+1,1> ad(const UnaryOp<Arg, detail::Log>& op, const Relation<Var_t, Arg_t>& sub) {
    Eigen::Matrix<double, Nd+1,1> g = ad<Nd>(op._arg, sub);

    Eigen::Matrix<double, Nd+1,1> result = Eigen::Matrix<double, Nd+1,1>::Zero();
    result[0] = sym::log(g[0]);
    
    for (size_t k(1); k < Nd+1; ++k) {
      for (size_t i(1); i < k; ++i)
	result[k] += i * result[i] * g[k - i];
      
      result[k] = (g[k] - result[k] / k) / g[0];
    }
    return result;
  }

  template<size_t Nd, typename Arg, typename Var_t, typename Arg_t>
  Eigen::Matrix<double, Nd+1,1> ad(const UnaryOp<Arg, detail::Sine>& op, const Relation<Var_t, Arg_t>& sub) {
    Eigen::Matrix<double, Nd+1,1> g = ad<Nd>(op._arg, sub);

    Eigen::Matrix<double, Nd+1,1> cos = Eigen::Matrix<double, Nd+1,1>::Zero();
    Eigen::Matrix<double, Nd+1,1> sin = Eigen::Matrix<double, Nd+1,1>::Zero();
    cos[0] = sym::cos(g[0]);
    sin[0] = sym::sin(g[0]);
    
    for (size_t k(1); k < Nd+1; ++k) {
      for (size_t i(1); i <= k; ++i) {
	sin[k] += i * g[i] * cos[k - i];
	cos[k] += i * g[i] * sin[k - i];
      }
      sin[k] /= k;
      cos[k] /= -double(k); //Promotion is needed before the minus sign, otherwise the unsigned int becomes large
    }
    
    return sin;
  }

  template<size_t Nd, typename Arg, typename Var_t, typename Arg_t>
  Eigen::Matrix<double, Nd+1,1> ad(const UnaryOp<Arg, detail::Cosine>& op, const Relation<Var_t, Arg_t>& sub) {
    Eigen::Matrix<double, Nd+1,1> g = ad<Nd>(op._arg, sub);

    Eigen::Matrix<double, Nd+1,1> cos = Eigen::Matrix<double, Nd+1,1>::Zero();
    Eigen::Matrix<double, Nd+1,1> sin = Eigen::Matrix<double, Nd+1,1>::Zero();
    cos[0] = sym::cos(g[0]);
    sin[0] = sym::sin(g[0]);
    
    for (size_t k(1); k < Nd+1; ++k) {
      for (size_t i(1); i <= k; ++i) {
	cos[k] += i * g[i] * sin[k - i];
	sin[k] += i * g[i] * cos[k - i];
      }
      sin[k] /= k;
      cos[k] /= -double(k); //Promotion is needed before the minus sign, otherwise the unsigned int becomes large
    }
    
    return cos;
  }

//  template<size_t Nd, class Var_t, class Arg_t>
//  Eigen::Matrix<double, Nd+1,1> ad(const Expr& f, const Relation<Var_t, Arg_t>& v) {
//    Expr RTBase::visit(detail::VisitorInterface& c) const {
//    switch (_type_idx) {
//    case detail::RT_type_index<ConstantRT<double>>::value:                     return c.visit(static_cast<const ConstantRT<double>&>(*this).get());
//    case detail::RT_type_index<VarRT>::value:                                  return c.visit(static_cast<const VarRT&>(*this));
//    case detail::RT_type_index<UnaryOp<Expr, detail::Sine>>::value:            return c.visit(static_cast<const UnaryOp<Expr, detail::Sine>&>(*this));
//    case detail::RT_type_index<UnaryOp<Expr, detail::Cosine>>::value:          return c.visit(static_cast<const UnaryOp<Expr, detail::Cosine>&>(*this));
//    case detail::RT_type_index<UnaryOp<Expr, detail::Log>>::value:             return c.visit(static_cast<const UnaryOp<Expr, detail::Log>&>(*this));
//    case detail::RT_type_index<UnaryOp<Expr, detail::Exp>>::value:             return c.visit(static_cast<const UnaryOp<Expr, detail::Exp>&>(*this));
//    case detail::RT_type_index<UnaryOp<Expr, detail::Absolute>>::value:        return c.visit(static_cast<const UnaryOp<Expr, detail::Absolute>&>(*this));
//    case detail::RT_type_index<UnaryOp<Expr, detail::Arbsign>>::value:         return c.visit(static_cast<const UnaryOp<Expr, detail::Arbsign>&>(*this));
//    case detail::RT_type_index<BinaryOp<Expr, detail::Add, Expr>>::value:      return c.visit(static_cast<const BinaryOp<Expr, detail::Add, Expr>&>(*this));
//    case detail::RT_type_index<BinaryOp<Expr, detail::Subtract, Expr>>::value: return c.visit(static_cast<const BinaryOp<Expr, detail::Subtract, Expr>&>(*this));
//    case detail::RT_type_index<BinaryOp<Expr, detail::Multiply, Expr>>::value: return c.visit(static_cast<const BinaryOp<Expr, detail::Multiply, Expr>&>(*this));
//    case detail::RT_type_index<BinaryOp<Expr, detail::Divide, Expr>>::value:   return c.visit(static_cast<const BinaryOp<Expr, detail::Divide, Expr>&>(*this));
//    case detail::RT_type_index<BinaryOp<Expr, detail::Power, Expr>>::value:    return c.visit(static_cast<const BinaryOp<Expr, detail::Power, Expr>&>(*this));
//    default: stator_throw() << "Unhandled type index for the visitor";
//    }
//  }

//    detail::ADRT<Nd, Relation<Var_t, Arg_t> > visitor(v);
//    return f->visit(visitor);
//  }
}
