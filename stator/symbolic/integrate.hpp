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
    
    /*! \brief Integration of \$C\$.*/
    template<char letter, class T,
             typename = typename std::enable_if<detail::IsConstant<T>::value>::type>
    auto integrate(const T& a, Variable<letter>)
      -> STATOR_AUTORETURN(a * Variable<letter>());
    
    /*! \brief Integration of \$x\$ by \$x\$.*/
    template<char letter>
    auto integrate(Variable<letter>, Variable<letter>)
      -> STATOR_AUTORETURN((C<1,2>() * pow<2>(Variable<letter>())));
    
    /*! \brief Integration of \$x\$ by \$y\$.*/
    template<char letter1, char letter2,
             typename = typename std::enable_if<letter1!= letter2>::type>
    auto integrate(Variable<letter1>, Variable<letter2>)
      -> STATOR_AUTORETURN(Variable<letter1>() * Variable<letter2>());

    /*! \brief Integration of \$x^n\$ by \$x\$.*/
    template<char letter, size_t power>
    auto integrate(const PowerOp<Variable<letter>, power>& a, Variable<letter>)
      -> STATOR_AUTORETURN(C<1, power+1>() * PowerOp<Variable<letter>, power+1>());
    
    
  } // namespace symbolic
} // namespace stator
