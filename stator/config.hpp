/*! \file config.hpp
  \brief Fundamental typedef's and macros for stator.
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
#include <Eigen/Dense>


/*! \brief A convenience Macro for defining auto return type functions.

  A fully generic addition function is written like so

  \code{.cpp}
  template<typename T1, typename T2>
  auto add(const T1& a, const T2& b) -> decltype(a+b)
  { return a+b; }
  \endcode

  This requires repetiting the actual expression twice, once in the
  decltype to determine the resulting type of the expression and once
  again in the return statement to actually calculate it. This macro
  permits a simpler definition:

  \code{.cpp}
  template<typename T1, typename T2>
  auto add(const T1& a, const T2& b) -> STATOR_AUTORETURN(a+b)
  \endcode  

  This reduces the amount of code that has to be written and ensures
  consistency between the two definitions. It is redundant in later
  C++ standards.
*/
#define STATOR_AUTORETURN(EXPR) decltype(EXPR) { return EXPR; }

namespace stator {
  /*! \brief A convenience typedef for a non-aligned Eigen Matrix.*/
  template<typename Scalar, size_t D1, size_t D2> using Matrix = Eigen::Matrix<Scalar, D1, D2, Eigen::DontAlign>;
  
  /*! \brief A convenience typedef for a non-aligned Eigen Vector.*/
  template<typename Scalar, size_t D> using Vector = Matrix<Scalar, D, 1>;
  
  namespace detail {

    /*! \class choice
      \brief A class which recursively inherits from itself to allow ambiguous function definition ordering.
     */
    template<unsigned I> struct choice : choice<I+1> {};
    
    /*! \brief The lowest-priority overload level.*/
    static const int LAST_OVERLOAD_LVL = 10;

    /*! \brief The lowest-priority overload level (closing the recursion)*/
    template<> struct choice<LAST_OVERLOAD_LVL> {};
    
    typedef choice<LAST_OVERLOAD_LVL> last_choice;

    /*! \brief A class used to start the ambiguous function definition ordering calculation. */
    struct select_overload : choice<0>{};

    template<class T> auto store_impl(T val, choice<0>) -> STATOR_AUTORETURN(typename std::decay<decltype(std::declval<T>().eval())>::type(val));
    template<class T> auto store_impl(T val, choice<1>) -> STATOR_AUTORETURN(typename std::decay<T>::type(val));

    /*!  Determine the type which can permanently store the passed
      Type (an extension of std::decay).
      
      This should determine the underlying type which is capable of
      storing a given type. Generally, this is just std::decay;
      however, libraries (such as Eigen) uses delayed evaluation thus
      we need to determine the resulting type to avoid aliasing
      issues.
      
      \code{.cpp}
      StoreType<decltype(A + B)>::type val = A + B;
      \endcode
    */
    template<class T> auto store(const T& val) -> STATOR_AUTORETURN(store_impl(val, select_overload{}));

    template<class T> struct dependent_false: std::false_type {};
    template<class T> struct dependent_true: std::true_type {};
  } // namespace detail
} // namespace stator

/*! \brief A convenience Macro for defining auto by-value return type functions.

  This macro is identical to STATOR_AUTORETURN, except it ensures the
  return values are by reference. It does this by using the STORETYPE
  macro to calculate the return type.
*/
#define STATOR_AUTORETURN_BYVALUE(EXPR) STATOR_AUTORETURN(store(EXPR))

