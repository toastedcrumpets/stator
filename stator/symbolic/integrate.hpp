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
    
#define IS_CONSTANT(f, letter) std::is_same<STORETYPE(derivative(f, Variable<letter>())), Null>::value

    /*! \brief Integration of any constant/independent expression.
      
      We test if an expression is independant by checking that its
      derivative is nullary zero.
    */
    template<char letter, class T>
    auto integrate(const T& a, Variable<letter>)
      -> typename std::enable_if<IS_CONSTANT(a, letter), STORETYPE(a * Variable<letter>())>::type
    { return a * Variable<letter>(); }

    /*! \brief Distributive integration over addition. */
    template<char Letter, class LHS, class RHS>
    auto integrate(const AddOp<LHS, RHS>& a, Variable<Letter> x)
      -> STATOR_AUTORETURN(integrate(a._l, x) + integrate(a._r, x));

    /*! \brief Distributive integration over subtraction. */
    template<char Letter, class LHS, class RHS>
    auto integrate(const SubtractOp<LHS, RHS>& a, Variable<Letter> x)
      -> STATOR_AUTORETURN(integrate(a._l, x) - integrate(a._r, x));

    /*! \brief distribute integration through LHS constant multiplication. */
    template<char Letter, class LHS, class RHS>
    auto integrate(const MultiplyOp<LHS, RHS>& a, Variable<Letter> x)
      -> typename std::enable_if<IS_CONSTANT(a._l, Letter), STORETYPE(a._l * integrate(a._r, x))>::type
    { return a._l * integrate(a._r, x); }

    /*! \brief distribute integration through RHS constant multiplication. */
    template<char Letter, class LHS, class RHS>
    auto integrate(const MultiplyOp<LHS, RHS>& a, Variable<Letter> x)
      -> typename std::enable_if<IS_CONSTANT(a._r, Letter), STORETYPE(integrate(a._l, x) * a._r)>::type
    { return integrate(a._l, x) * a._r; }
    
    /*! \brief Integration of \$x\$ by \$x\$.*/
    template<char letter>
    auto integrate(Variable<letter>, Variable<letter>)
      -> STATOR_AUTORETURN((C<1,2>() * pow<2>(Variable<letter>())));
    
    /*! \brief Integration of \$x^n\$ by \$x\$.*/
    template<char letter, size_t power>
    auto integrate(const PowerOp<Variable<letter>, power>& a, Variable<letter>)
      -> STATOR_AUTORETURN((C<1, power+1>() * PowerOp<Variable<letter>, power+1>()));
    
  } // namespace symbolic
} // namespace stator
