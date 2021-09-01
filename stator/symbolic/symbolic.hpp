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
#include <stator/repr.hpp>
#include <stator/orphan/stack_vector.hpp>
#include <Eigen/Dense>
#include <complex>
#include <ratio>
#include <type_traits>
#include <stator/orphan/template_config.hpp>

namespace sym {
  namespace detail {
    using stator::detail::choice;
    using stator::detail::last_choice;
    using stator::detail::select_overload;
  } // namespace detail
  
  using stator::orphan::StackVector;
  using stator::detail::store;
  using stator::repr;
  using stator::DefaultReprConfig;
  
  /*! \brief A type trait to denote symbolic terms (i.e., one that
      is not yet immediately evaluable to a "normal" type).
      
      Inheriting from this will enable symbolic operators (and
      construction of expressions) for that class.
  */
  struct SymbolicOperatorBase {};

  template<class Derived>
  struct SymbolicOperator : public SymbolicOperatorBase {
    template<class RHS> auto operator[](const RHS & rhs) const;
  };

  /*! \brief A test if a class is symbolic. 
    
    This is mainly used to determine if the symbolic operators should
    be applied to the class.
   */
  template<class T>
  struct IsSymbolic {
    static constexpr bool value = std::is_base_of<SymbolicOperatorBase, T>::value;
  };

  template<class T>
  constexpr bool is_symbolic(const T&) { return std::is_base_of<SymbolicOperatorBase, T>::value; }
  
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

  template<class T>
  constexpr bool is_dynamic(const T&) { return std::is_base_of<Dynamic, T>::value; }

  template<class T, typename = typename std::enable_if<sym::IsSymbolic<T>::value>::type>
  std::ostream& operator<<(std::ostream& os, const T& v) {
    return os << repr(v);
  }

  namespace detail {
    template<class T>
    struct Type_index {
      static_assert(sizeof(T) == -1, "Missing index for runtime type");
    };
  }
}


#include <stator/symbolic/constants.hpp>
#include <stator/symbolic/variable.hpp>
#include <stator/symbolic/empty_sum.hpp>
#include <stator/symbolic/binary_ops.hpp>
#include <stator/symbolic/sub.hpp>
#include <stator/symbolic/unary_ops.hpp>
#include <stator/symbolic/polynomial.hpp>
#include <stator/symbolic/simplify.hpp>
#include <stator/symbolic/integrate.hpp>
#include <stator/symbolic/array.hpp>
#include <stator/symbolic/taylor.hpp>
#include <stator/symbolic/units.hpp>

// If you need runtime expressions and parsing, please include
// <stator/symbolic/runtime.hpp> instead of this file.

