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

    /*! \brief The preferred implementation of try_eval().
    
      This takes a higher precedence to the try_eval implementation
      below due to not requiring a conversion for the second
      argument (when called as try_eval_imp(a, 0)).
    */
    template<class T>
    auto try_eval_imp(const T& a, choice<0>) -> decltype(a.eval()) {
      return a.eval();
    }

    /*! \brief The backup implementation of try_eval().
        
      This takes a lower precedence to the above try_eval due to the
      implicit conversion from int->long for the second argument.
    */
    template<class T>
    const T& try_eval_imp(const T& a, choice<1>) {
      return a;
    }

    /*!\brief Returns the result of calling the eval() member function
      (if available) on the passed argument, or the unmodified
      argument (if not).
    */
    template<class T>
    auto try_eval(const T& a) -> decltype(detail::try_eval_imp(a, select_overload{})) {
      return detail::try_eval_imp(a, select_overload{});
    }
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
#define STORETYPE(A) typename std::decay<decltype(stator::detail::try_eval(A))>::type


#define STATOR_AUTORETURN(EXPR) decltype(EXPR) { return EXPR; }
#define STATOR_AUTORETURN_BYVALUE(EXPR) STORETYPE(EXPR) { return EXPR; }
