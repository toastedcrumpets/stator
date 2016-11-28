/*! \file integrate.hpp
  \brief Integration support for the stator::symbolic library
*/
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

//stator
#include "stator/exception.hpp"
#include "stator/config.hpp"


namespace stator {
  namespace symbolic {
    
#define IS_CONSTANT(f, Variable) std::is_same<decltype(store(derivative(f, Variable()))), Null>::value

    /*! \brief Integration of any constant/independent expression.
      
      We test if an expression is independant by checking that its
      derivative is nullary zero.
    */
    template<class T, class Variable>
    auto integrate(const T& a, Variable)
      -> typename std::enable_if<IS_CONSTANT(a, Variable), decltype(store(a * Variable()))>::type
    { return a * Variable(); }

    /*! \brief Distributive integration over addition. */
    template<class ...VarArgs, class LHS, class RHS>
    auto integrate(const AddOp<LHS, RHS>& a, Variable<VarArgs...> x)
      -> STATOR_AUTORETURN(integrate(a._l, x) + integrate(a._r, x));

    /*! \brief Distributive integration over subtraction. */
    template<class ...VarArgs, class LHS, class RHS>
    auto integrate(const SubtractOp<LHS, RHS>& a, Variable<VarArgs...> x)
      -> STATOR_AUTORETURN(integrate(a._l, x) - integrate(a._r, x));

    /*! \brief distribute integration through LHS constant multiplication. */
    template<class ...VarArgs, class LHS, class RHS>
    auto integrate(const MultiplyOp<LHS, RHS>& a, Variable<VarArgs...> x)
      -> typename std::enable_if<IS_CONSTANT(a._l, Variable<VarArgs...>), decltype(store(a._l * integrate(a._r, x)))>::type
    { return a._l * integrate(a._r, x); }

    /*! \brief distribute integration through RHS constant multiplication. */
    template<class ...VarArgs, class LHS, class RHS>
    auto integrate(const MultiplyOp<LHS, RHS>& a, Variable<VarArgs...> x)
      -> typename std::enable_if<IS_CONSTANT(a._r, Variable<VarArgs...>), decltype(store(integrate(a._l, x) * a._r))>::type
    { return integrate(a._l, x) * a._r; }
    
    /*! \brief Integration of \$x\$ by \$x\$.*/
    template<class ...VarArgs1, class ...VarArgs2,
	     typename = typename enable_if_var_in<Variable<VarArgs1...>, Variable<VarArgs2...> >::type>
    auto integrate(Variable<VarArgs1...>, Variable<VarArgs2...>)
      -> STATOR_AUTORETURN((C<1,2>() * pow<2>(typename variable_combine<Variable<VarArgs1...>, Variable<VarArgs2...> >::type())));
    
    /*! \brief Integration of \$x^n\$ by \$x\$.*/
    template<class ...VarArgs1, class ...VarArgs2, size_t Power,
	     typename = typename enable_if_var_in<Variable<VarArgs1...>, Variable<VarArgs2...> >::type>
    auto integrate(const PowerOp<Variable<VarArgs1...>, Power>& a, Variable<VarArgs2...>)
      -> STATOR_AUTORETURN((C<1, Power+1>() * PowerOp<typename variable_combine<Variable<VarArgs1...>, Variable<VarArgs2...> >::type(), Power+1>()));
    
  } // namespace symbolic
} // namespace stator
