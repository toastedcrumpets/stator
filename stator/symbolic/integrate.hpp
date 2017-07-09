/*! \file integrate.hpp
  \brief Integration support for the stator::symbolic library
*/
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

//stator
#include "stator/exception.hpp"
#include "stator/config.hpp"


namespace sym {  
#define IS_CONSTANT(f, Var) std::is_same<decltype(store(derivative(f, Var()))), Null>::value

  /*! \brief Integration of any constant/independent expression.
    
    We test if an expression is independant by checking that its
    derivative is nullary zero.
  */
  template<class T, class Var>
  auto integrate(const T& a, Var)
    -> typename std::enable_if<IS_CONSTANT(a, Var), decltype(store(a * Var()))>::type
  { return a * Var(); }

  /*! \brief Distributive integration over addition. */
  template<class ...VarArgs, class LHS, class RHS>
  auto integrate(const AddOp<LHS, RHS>& a, Var<VarArgs...> x)
    -> STATOR_AUTORETURN(integrate(a._l, x) + integrate(a._r, x));

  /*! \brief Distributive integration over subtraction. */
  template<class ...VarArgs, class LHS, class RHS>
  auto integrate(const SubtractOp<LHS, RHS>& a, Var<VarArgs...> x)
    -> STATOR_AUTORETURN(integrate(a._l, x) - integrate(a._r, x));

  /*! \brief distribute integration through LHS constant multiplication. */
  template<class ...VarArgs, class LHS, class RHS>
  auto integrate(const MultiplyOp<LHS, RHS>& a, Var<VarArgs...> x)
    -> typename std::enable_if<IS_CONSTANT(a._l, Var<VarArgs...>), decltype(store(a._l * integrate(a._r, x)))>::type
  { return a._l * integrate(a._r, x); }

  /*! \brief distribute integration through RHS constant multiplication. */
  template<class ...VarArgs, class LHS, class RHS>
  auto integrate(const MultiplyOp<LHS, RHS>& a, Var<VarArgs...> x)
    -> typename std::enable_if<IS_CONSTANT(a._r, Var<VarArgs...>), decltype(store(integrate(a._l, x) * a._r))>::type
  { return integrate(a._l, x) * a._r; }
  
  /*! \brief Integration of \$x\$ by \$x\$.*/
  template<class ...VarArgs1, class ...VarArgs2,
	     typename = typename enable_if_var_in<Var<VarArgs1...>, Var<VarArgs2...> >::type>
  auto integrate(Var<VarArgs1...>, Var<VarArgs2...>)
    -> STATOR_AUTORETURN((C<1,2>() * pow<2>(typename variable_combine<Var<VarArgs1...>, Var<VarArgs2...> >::type())));
  
  /*! \brief Integration of \$x^n\$ by \$x\$.*/
  template<class ...VarArgs1, class ...VarArgs2, std::intmax_t Power,
	     typename = typename enable_if_var_in<Var<VarArgs1...>, Var<VarArgs2...> >::type>
  auto integrate(const PowerOp<Var<VarArgs1...>, C<Power> >& a, Var<VarArgs2...>)
    -> STATOR_AUTORETURN((C<1, Power+1>() * pow(typename variable_combine<Var<VarArgs1...>, Var<VarArgs2...> >::type(), C<Power+1>())));
  
} // namespace symbolic
