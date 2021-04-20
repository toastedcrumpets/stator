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

// stator
#include <stator/config.hpp>
#include <stator/constants.hpp>
#include <stator/orphan/stack_vector.hpp>

// Eigen
#include <Eigen/Dense>

// C++
#include <complex>
#include <ratio>
#include <type_traits>

namespace sym {
  using stator::orphan::StackVector;
  using stator::detail::store;

  namespace detail {
    using stator::detail::choice;
    using stator::detail::last_choice;
    using stator::detail::select_overload;
  } // namespace detail

  /*! \brief A type trait to denote symbolic terms (i.e., one that
      is not yet immediately evaluable to a "normal" type)*/
  struct SymbolicOperator {};
}

#include <stator/symbolic/constants.hpp>
#include <stator/orphan/template_config.hpp>

namespace sym {
  /*! \brief Template argument for dynamic types, as well as their
      base class.

      This class is used to provide specialisations of compile time
      types, such as UnaryOp, BinaryOp, for the run-time CAS
      implementation. These types also inherit from this class, to
      allow their identification.
  */
  struct Dynamic {};

  
  template<class T>
  struct IsSymbolic {
    static constexpr bool value = std::is_base_of<SymbolicOperator, T>::value;
  };

  namespace detail {
    struct vidx_ID;
  };

  template<char idx>
  struct vidx : stator::orphan::value_conf_t<detail::vidx_ID, char, idx> {};

  /*!\brief Symbolic representation of a variable.

    This class is used to denote a variable.
  */
  template<typename ...Args> struct Var : SymbolicOperator {
    static constexpr const auto idx = stator::orphan::get_value<vidx<'x'>, Args...>::value;

    template<class Arg>
    auto operator=(const Arg& a) const -> STATOR_AUTORETURN(equal(*this, a));

    template<typename ...Args2>
    bool operator==(const Var<Args2...>& a) const { return Var::idx == Var<Args2...>::idx; }
    
    std::string getID() const { return std::string(1, Var::idx); }
  };

  template<typename Var1, typename Var2>
  struct variable_in {
    static constexpr const bool value = Var1::idx == Var2::idx;
  };

  template<typename Var1, typename Var2>
  struct enable_if_var_in : std::enable_if<variable_in<Var1, Var2>::value> {
  };
  
  template<typename Var1, typename Var2>
  struct enable_if_var_not_in : std::enable_if<!variable_in<Var1, Var2>::value> {
  };

  template<typename Var1, typename Var2>
  struct variable_combine {
    typedef Var<vidx<Var1::idx> > type;
  };
  
  
  namespace detail {
    /*!\brief Type trait to determine if a certain type is a constant.

	This is used to enable the derivative operation to convert
	these types to Null types. It is also to apply a
	specialised functions to these types.
    */
    template<class T>
    struct IsConstant : std::conditional<std::is_arithmetic<T>::value || is_C<T>::value || std::is_base_of<Eigen::EigenBase<T>, T>::value, std::true_type, std::false_type>::type {};

    template<class T>
    struct IsConstant<std::complex<T> > : IsConstant<T> {};
    
  }// namespace detail

  template<class T>
  constexpr typename std::enable_if<detail::IsConstant<T>::value, bool>::type is_constant(const T&) { return true;}

  template<class T>
  constexpr typename std::enable_if<!detail::IsConstant<T>::value, bool>::type is_constant(const T&) { return false;}
  
  /*! \brief Returns the empty sum of a type.
    
    The empty sum is a term whose additive (and typically its
    subtractive) action is null (can be ignored). This definition
    only applies for selected types and assumes that their default
    constructors create the empty sum.
  */
  template<class T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
  T empty_sum(const T&) { return T(); }

  template<class T, typename = typename std::enable_if<std::is_base_of<Eigen::EigenBase<T>, T>::value>::type>
  auto empty_sum(const T&) -> STATOR_AUTORETURN_BYVALUE(T::Zero());

  template<class T>
  T empty_sum(const std::complex<T>&) { return T(); }
    
  /*! \brief Determine the derivative of a symbolic expression.
    
    This default implementation gives all consants
    derivative of zero.
  */
  template<class T, class ...Args,
	     typename = typename std::enable_if<detail::IsConstant<T>::value>::type>
  Null derivative(const T&, Var<Args...>) { return Null(); }

  /*! \brief Determine the derivative of a variable.

    If the variable is the variable in which a derivative is being
    taken, then this overload should be selected to return
    Unity.
  */
  template<class ...Args1, class ...Args2,
	   typename = typename std::enable_if<!std::is_base_of<Dynamic, Var<Args1...> >::value && !std::is_base_of<Dynamic, Var<Args2...> >::value>::type>
  auto derivative(Var<Args1...>, Var<Args2...>) -> typename std::enable_if<Var<Args1...>::idx == Var<Args2...>::idx, Unity>::type
  { return Unity(); } 

  inline StackVector<double, 0> solve_real_roots(Null f) {
    return StackVector<double, 0>();
  }

  /*! \brief Determine the derivative of a variable by another variable.

    If the variable is NOT the variable in which a derivative is
    being taken, then this overload should be selected to return
    Null.
  */
  template<class ...Args1, class ...Args2,
	   typename = typename std::enable_if<!std::is_base_of<Dynamic, Var<Args1...> >::value && !std::is_base_of<Dynamic, Var<Args2...> >::value>::type>
    auto derivative(Var<Args1...>, Var<Args2...>) -> typename std::enable_if<Var<Args1...>::idx != Var<Args2...>::idx, Null>::type
  { return Null(); }

  
  template<size_t Order, class Real = double, class PolyVar = Var<>> class Polynomial;
}

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
