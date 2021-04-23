/*! \file symbolic.hpp
  \brief Main header for the stator::symbolic library.
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

#include <stator/config.hpp>
#include <stator/constants.hpp>
#include <stator/orphan/stack_vector.hpp>
#include <Eigen/Dense>
#include <complex>
#include <ratio>
#include <type_traits>
#include <stator/orphan/template_config.hpp>

namespace sym {
  using stator::orphan::StackVector;
  using stator::detail::store;

  namespace detail {
    using stator::detail::choice;
    using stator::detail::last_choice;
    using stator::detail::select_overload;
  } // namespace detail

  /*! \brief A type trait to denote symbolic terms (i.e., one that
      is not yet immediately evaluable to a "normal" type).
      
      Inheriting from this will enable symbolic operators (and
      construction of expressions) for that class.
  */
  struct SymbolicOperator {
    //template<class T>
    //auto operator[](const T& key) const -> STATOR_AUTORETURN()
  };

  /*! \brief A test if a class is symbolic. 
    
    This is mainly used to determine if the symbolic operators should
    be applied to the class.
   */
  template<class T>
  struct IsSymbolic {
    static constexpr bool value = std::is_base_of<SymbolicOperator, T>::value;
  };

  /*! \brief Template argument for dynamic/run-time types, as well as
      their base class.

      This class is used in template specialisations of compile time
      types, such as UnaryOp, BinaryOp, for the run-time CAS
      implementation. These types also inherit from this class, to
      allow their identification.
  */
  struct Dynamic {};

  /*! \brief A test if a class is symbolic. 
    
    This is mainly used to determine if the symbolic operators should
    be applied to the class.
   */
  template<class T>
  struct IsDynamic {
    static constexpr bool value = std::is_base_of<Dynamic, T>::value;
  };
}

#include <stator/symbolic/constants.hpp>
#include <stator/symbolic/variable.hpp>
#include <stator/symbolic/empty_sum.hpp>
#include <stator/symbolic/binary_ops.hpp>
#include <stator/symbolic/unary_ops.hpp>
#include <stator/symbolic/sub.hpp>
#include <stator/symbolic/polynomial.hpp>
#include <stator/symbolic/simplify.hpp>
#include <stator/symbolic/integrate.hpp>
#include <stator/symbolic/taylor.hpp>
#include <stator/symbolic/runtime.hpp>
#include <stator/symbolic/parser.hpp>
#include <stator/symbolic/print.hpp>
