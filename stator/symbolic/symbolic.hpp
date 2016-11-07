/*! \file symbolic.hpp
  \brief Main header for the stator::symbolic library.
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

// stator
#include "stator/config.hpp"
#include "stator/constants.hpp"
#include "stator/orphan/stack_vector.hpp"

// Eigen
#include "Eigen/Dense"

// C++
#include <complex>
#include <ratio>

namespace stator {
  namespace symbolic {
    using stator::orphan::StackVector;
    
    namespace detail {
      using stator::detail::choice;
      using stator::detail::last_choice;
      using stator::detail::select_overload;
    } // namespace detail
    
    /*! \brief A class representing a compile-time constant.

      These are implemented using std::ratio. The only reason this
      type exists as a separate type from std::ratio (and inherits
      from from std::ratio) is to ensure operator lookups consider
      this namespace. Many additional options are enabled (such as
      ratio-float multipliation).
     */
    template<std::intmax_t Num, std::intmax_t Denom = 1>
    struct C: std::ratio<Num, Denom> {};

    /*! \brief Conversion operator from std::ratio to C.*/
    template<class stdratio>
    struct C_wrap {
      typedef C<stdratio::num, stdratio::den> type;
    };
    
    /*! \brief Compile time type-test for compile-time constants \ref C.*/
    template<class T> struct is_C { static const bool value = false; };

    template<std::intmax_t N, std::intmax_t D> struct is_C<C<N,D> > { static const bool value = true; };

    /*! \brief A symbolic representation of zero. */
    typedef C<0> Null;
    /*! \brief A symbolic representation of one. */
    typedef C<1> Unity;
    
    /*! \brief A symbolic/compile-time rational approximation of \f$\pi\f$. */
    typedef C_wrap<constant_ratio::pi>::type pi;

    /*! \brief A symbolic/compile-time rational approximation of \f$\mathrm{e}\f$. */
    typedef C_wrap<constant_ratio::e>::type e;

    /*! \brief Output operator for compile-time constant (\ref C types).*/
    template<std::intmax_t Num, std::intmax_t Denom>
    inline std::ostream& operator<<(std::ostream& os, const C<Num, Denom>) {
      os << "C<" << Num;
      if (Denom != 1)
	os << ", " << Denom;
      return os << ">()";
    }
    
    /*! \brief Specialized output operator for \f$\pi\f$.*/
    inline std::ostream& operator<<(std::ostream& os, const pi) { os << "π"; return os; }
    /*! \brief Specialized output operator for \f$\mathrm{e}\f$.*/
    inline std::ostream& operator<<(std::ostream& os, const e) { os << "e"; return os; }

    template<class T>
    inline std::ostream& operator<<(std::ostream& os, const std::complex<T>& c) {
      if (c.imag() == 0)
	return (os << c.real());
      if (c.imag() < 0)
	return (os << "(" <<c.real() << " - " << std::abs(c.imag()) << "i)");
      return (os << "(" <<c.real() << " + " << c.imag() << "i)");
    }
    
    template<class T,
	     typename = typename std::enable_if<std::is_arithmetic<T>::value || std::is_base_of<Eigen::EigenBase<T>, T>::value>::type>
    const T& toArithmetic(const T& val) { return val; }
    
    template<std::intmax_t n1, std::intmax_t d1,
    	     typename = typename std::enable_if<!(n1 % d1)>::type> 
    std::intmax_t toArithmetic(C<n1,d1> val) { return n1 / d1; }
    
    template<std::intmax_t n1, std::intmax_t d1, 
    	     typename = typename std::enable_if<n1 % d1>::type>
    double toArithmetic(C<n1,d1> val) { return double(n1) / double(d1); }

    /*!\brief Compile-time symbolic representation of a variable
      substitution.
    */
    template<char Letter, class Arg> struct VariableSubstitution {
      VariableSubstitution(const Arg& val): _val(val) {}
      const Arg& _val;
    };

    /*!\brief Symbolic representation of a variable.

      This class is used to denote a variable. The template argument
      is a single ASCII character which represents this variable and
      is used to identify it during symbolic actions and output.
    */
    template<char Letter> struct Variable {
      template<class Arg>
      VariableSubstitution<Letter, Arg> operator==(const Arg& a) const {
	return VariableSubstitution<Letter, Arg>(a);
      }
    };

    namespace detail {
      /*!\brief Type trait to determine if a certain type is a constant.

	This is used to enable the derivative operation to convert
	these types to Null types. It is also to apply a
	specialised functions to these types.
      */
      template<class T>
      struct IsConstant {
	static const bool value = std::is_arithmetic<T>::value || is_C<T>::value || std::is_base_of<Eigen::EigenBase<T>, T>::value;
      };

      template<class T>
      struct IsConstant<std::complex<T> > {
	static const bool value = IsConstant<T>::value;
      };
    }// namespace detail

    /*! \brief Returns the empty sum of a type.
      
      The empty sum is a term whose additive (and typically its
      subtractive) action is null (can be ignored). This definition
      only applies for selected types and assumes that their default
      constructors create the empty sum.
    */
    template<class T>
    typename std::enable_if<detail::IsConstant<T>::value && !std::is_base_of<Eigen::EigenBase<T>, T>::value, T>::type empty_sum(const T&) {
      return T();
    }

    template<class T>
    typename std::enable_if<std::is_base_of<Eigen::EigenBase<T>, T>::value, T>::type empty_sum(const T&) {
      return T::Zero();
    }

    /*! \brief Evaluates a symbolic expression by substituting a
      variable for another expression.
	
      If a arithmetic type is substituted, this will likely cause a
      numerical evaluation of the expression. This "helper"
      implementation converts to a substitution for the variable
      'x'.
    */
    template<class T, class VarArg>
    auto eval(const T& f, const VarArg& xval) 
      -> STATOR_AUTORETURN(substitution(f, Variable<'x'>() == xval))

    /*! \brief Evaluates a symbolic expression using a substitution.
      
      This is just a synonym for substitution.
    */
    template<class T, char Letter, class Arg>
    auto eval(const T& f, const VariableSubstitution<Letter, Arg>& x)
      -> STATOR_AUTORETURN(substitution(f, x))
    
    /*! \brief Default implementation of substitution of a symbolic
      expression at a given point.
      
      This implementation only applies if the term is a constant term.

      We deliberately return by const reference as, if this is an
      Eigen expression, the Eigen library may take an internal
      reference to this object to allow delayed evaluation. By
      returning the original object we can try to ensure its lifetime
      is at least longer than the current expression.
    */
    template<class T, char Letter, class Arg,
	     typename = typename std::enable_if<detail::IsConstant<T>::value>::type >
    const T& substitution(const T& f, const VariableSubstitution<Letter, Arg>&)
    { return f; }

    /*! \brief Evaluates a symbolic Variable at a given point.
      
      This is only used if the Variable is the correct letter for the
      substitution.
    */
    template<char Letter, class Arg>
    const Arg& substitution(const Variable<Letter>& f, const VariableSubstitution<Letter, Arg>& x)
    { return x._val; }

    /*! \brief Evaluates a symbolic Variable at a given point.
      
      This is only used if the Variable is not the correct letter for the
      substitution.
    */
    template<char Letter1, class Arg, char Letter2,
	     typename = typename std::enable_if<Letter1 != Letter2>::type>
    Variable<Letter1> substitution(const Variable<Letter1>& f, const VariableSubstitution<Letter2, Arg>& x)
    { return f; }
    
    /*! \brief Output operator for Variable types. */
    template<char Letter>
    inline std::ostream& operator<<(std::ostream& os, const Variable<Letter>&) {
      os << Letter;
      return os;
    }

    /*! \brief Output operator for VariableSubstitution types. */
    template<char Letter, class Arg>
    inline std::ostream& operator<<(std::ostream& os, const VariableSubstitution<Letter, Arg>& sub) {
      os << Letter << " <- " << sub._val;
      return os;
    }
    
    /*! \brief Determine the derivative of a symbolic expression.
      
      This default implementation gives all consants
      derivative of zero.
    */
    template<class T, char Letter,
	     typename = typename std::enable_if<detail::IsConstant<T>::value>::type>
    Null derivative(const T&, Variable<Letter>) { return Null(); }

    /*! \brief Determine the derivative of a variable.

      If the variable is the variable in which a derivative is being
      taken, then this overload should be selected to return
      Unity.
    */
    template<char Letter1, char Letter2,
	     typename = typename std::enable_if<Letter1 == Letter2>::type>
    Unity derivative(Variable<Letter1>, Variable<Letter2>) { return Unity(); }

    inline StackVector<double, 0> solve_real_roots(Null f) {
      return StackVector<double, 0>();
    }

    /*! \brief Determine the derivative of a variable by another variable.

      If the variable is NOT the variable in which a derivative is
      being taken, then this overload should be selected to return
      Null.
    */
    template<char Letter1, char Letter2,
	     typename = typename std::enable_if<Letter1 != Letter2>::type>
    Null derivative(Variable<Letter1>, Variable<Letter2>) { return Null(); }
 
    /*! \brief Shift a function forward. It returns \f$g(x)=f(x+a)\f$

      For constant terms, these remain the same so this generic
      implementation does nothing.
    */
    template<class F, class Real,
	     typename = typename std::enable_if<detail::IsConstant<F>::value>::type>
    inline F shift_function(const F& f, const Real t) {
      return f;
    }
    
    /*! \brief Calculate the next real root of a symbolic function.

      For constant terms, +inf is returned to indicate no root was
      found.
    */
    template<class F,
	     typename = typename std::enable_if<detail::IsConstant<F>::value>::type>
    inline double next_root(const F& f) {
      return HUGE_VAL;
    }

    /*! \brief Estimate the error in evaluating a function at a given time.
     */
    template<class F, class Real,
	     typename = typename std::enable_if<detail::IsConstant<F>::value>::type>
    inline double precision(const F& f, const Real) {
      return 0.0;
    }

    template<size_t Order, class Real = double, char Letter = 'x'> class Polynomial;

    /*! \brief Symbolic Factorial function.
     
      This template implementation returns Unity for 0! and 1!,
      allowing simplification of symbolic expressions.
     */
    template<size_t i> struct Factorial {
      typedef C<i * Factorial<i - 1>::value::num, 1> value;
    };
    
    template<> struct Factorial<1> {
      typedef C<1, 1> value;
    };

    template<> struct Factorial<0> {
      typedef C<1, 1> value;
    };

    /*! \brief Symbolic Inverse factorial function.
     
      This template implementation returns Unity for 1/0! and 1/1!,
      allowing simplification of symbolic expressions.
     */
    template<size_t i> struct InvFactorial {
      typedef C<Factorial<i>::value::den, Factorial<i>::value::num> value;
    };
  }
}

#include "stator/symbolic/operators.hpp"
#include "stator/symbolic/functions.hpp"
#include "stator/symbolic/polynomial.hpp"
#include "stator/symbolic/simplify.hpp"
#include "stator/symbolic/integrate.hpp"

namespace stator {
  namespace symbolic {
    namespace detail {
      template<size_t State, size_t max_Order, char Letter>
      struct TaylorSeriesWorker {
	template<class Real>
	static Null eval(const Null& f, const Real& a)
	{ return Null(); }
        
	template<class F, class Real>
	static auto eval(const F& f, const Real& a) 
          -> STATOR_AUTORETURN((typename InvFactorial<State>::value() * substitution(f, Variable<Letter>() == a) + (Variable<Letter>() - a) * TaylorSeriesWorker<State+1, max_Order, Letter>::eval(derivative(f, Variable<Letter>()), a)))
      };

      template<size_t max_Order, char Letter>
      struct TaylorSeriesWorker<max_Order,max_Order,Letter> {
	template<class F, class Real>
	static auto eval(const F& f, const Real& a)
          -> STATOR_AUTORETURN((typename InvFactorial<max_Order>::value() * substitution(f, Variable<Letter>() == a)));
        
	template<class Real>
	static Null eval(const Null& f, const Real& a)
	{ return Null(); }
      };
    }

    /*! \function taylor_series
      \brief Generate a Taylor series representation of a Symbolic
      expression.
    */
    template<size_t Order, char Letter, class F, class Real>
    auto taylor_series(const F& f, Real a, Variable<Letter>) 
      -> STATOR_AUTORETURN((try_simplify(detail::TaylorSeriesWorker<0, Order, Letter>::eval(f, a))));
  } // namespace symbolic
}// namespace stator
