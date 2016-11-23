/*! \file config.hpp
  \brief Fundamental typedef's and macros for stator.
*/
/*
  Copyright (C) 2015 Marcus Bannerman <m.bannerman@gmail.com>

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
  consistency between the two definitions.
*/
#define STATOR_AUTORETURN(EXPR) decltype(EXPR) { return EXPR; }

namespace stator {
  /*! \brief A convenience typedef for a non-aligned Eigen Matrix.*/
  template<typename Scalar, size_t D1, size_t D2> using Matrix = Eigen::Matrix<Scalar, D1, D2, Eigen::DontAlign>;
  
  /*! \brief A convenience typedef for a non-aligned Eigen Vector.*/
  template<typename Scalar, size_t D> using Vector = Matrix<Scalar, D, 1>;
  
  namespace detail {

    /*! \brief A class which recursively inherits from itself to allow ambiguous function definition ordering.*/
    template<unsigned I> struct choice : choice<I+1>{};
    
    /*! \brief The lowest-priority overload level.*/
    static const int LAST_OVERLOAD_LVL = 10;

    /*! \brief The lowest-priority overload level (closing the recursion)*/
    template<> struct choice<LAST_OVERLOAD_LVL> {};

    typedef choice<LAST_OVERLOAD_LVL> last_choice;

    /*! \brief A class used to start the ambiguous function definition ordering calculation. */
    struct select_overload : choice<0>{};

    template<class T, class = void >
    struct StoreType {
      typedef typename std::decay<T>::type type;
    };

    template<class T>
    struct StoreType<T, typename std::result_of<decltype(&T::eval)()>::type> {
      typedef typename std::decay<typename std::result_of<decltype(&T::eval)()>::type>::type type;
    };
      
    
    /*! \brief The preferred implementation of try_eval().
    
      This takes a higher precedence to the try_eval implementation
      below due to not requiring a conversion for the second
      argument (when called as try_eval_imp(a, 0)).
    */
    template<class T>
    auto try_Eigen_eval_imp(const T& a, choice<0>) -> STATOR_AUTORETURN(a.eval());

    /*! \brief The backup implementation of try_eval().
        
      This takes a lower precedence to the above try_eval due to the
      implicit conversion from int->long for the second argument.
    */
    template<class T>
    T try_Eigen_eval_imp(T a, choice<1>) { return a; }

    /*!\brief Returns the result of calling the eval() member function
      (if available) on the passed argument, or the unmodified
      argument (if not).
    */
    template<class T>
    auto try_Eigen_eval(const T& a) -> STATOR_AUTORETURN(detail::try_Eigen_eval_imp(a, select_overload{}));
  } // namespace detail
} // namespace stator


/*! Determine the type used to store the result of an expression.
  
  This Macro handles everything, including Eigen expressions, and
  returns an appropriate type to store the results of the expression
  A. Example usage is:
  
  \code{.cpp}
  STORETYPE(A.dot(B) + C) val = A.dot(B) + C;
  \endcode

  This macro is often used to determine the coefficient type of a
  Polynomial class.
*/
#define STORETYPE(A) typename std::decay<decltype(stator::detail::try_Eigen_eval(A))>::type

#define AS_STORETYPE(A) STORETYPE(A)(A)

/*! \brief A convenience Macro for defining auto by-value return type functions.

  This macro is identical to STATOR_AUTORETURN, except it ensures the
  return values are by reference. It does this by using the STORETYPE
  macro to calculate the return type.
*/
#define STATOR_AUTORETURN_BYVALUE(EXPR) STORETYPE(EXPR) { return EXPR; }

