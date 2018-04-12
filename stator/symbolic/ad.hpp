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
	result[k] -= result[i] * r[k-i];
      result[k] /= r[0];
    }
    return result;
  }

}
