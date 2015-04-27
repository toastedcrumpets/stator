/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.dynamomd.org
    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include "Eigen/Dense"
#include <complex>
#include <ratio>

namespace stator {
  namespace symbolic {
    /*! \brief Stack allocated std::vector.
      
      This class impersonates a STL vector, but its storage is
      allocated on the stack.
    */
    template<class T, size_t Nmax>
    class StackVector: public std::array<T, Nmax> {
      typedef std::array<T, Nmax> Base;
    public:
      template<size_t Nmax2>
      StackVector(const StackVector<T, Nmax2>& vec):
	Base()
      {
	static_assert(Nmax2 <= Nmax, "Can only convert to larger StackVector containers");
	_size = vec.size();
	std::copy(vec.begin(), vec.end(), Base::begin());
      }

      StackVector(): Base(), _size(0) {}
      
      StackVector(std::initializer_list<T> _list):
	Base(),
	_size(0)
      {
	auto it = _list.begin();
	for (size_t i(0); (i < Nmax) && (it != _list.end()); ++i, ++it)
	  push_back(*it);
      }
            
      constexpr typename Base::size_type size() const { return _size; }
      constexpr bool empty() const { return size() == 0; }

      typename Base::iterator end() { return typename Base::iterator(Base::data() + _size); }
      typename Base::const_iterator end() const { return typename Base::iterator(Base::data() + _size); }
      typename Base::const_iterator cend() const { return typename Base::iterator(Base::data() + _size); }

      typename Base::reverse_iterator rbegin() { return typename Base::reverse_iterator(Base::end()); }
      typename Base::const_reverse_iterator rbegin() const { return typename Base::const_reverse_iterator(Base::end()); }
      typename Base::const_reverse_iterator crbegin() const { return typename Base::const_reverse_iterator(Base::end()); }
      typename Base::reverse_iterator rend() { return typename Base::reverse_iterator(Base::begin()); }
      typename Base::const_reverse_iterator rend() const { return typename Base::const_reverse_iterator(Base::begin()); }
      typename Base::const_reverse_iterator crend() const { return typename Base::const_reverse_iterator(Base::begin()); }
      
      typename Base::reference back() { return _size ? *(Base::end() - 1) : *Base::end(); }
      typename Base::const_reference back() const { return _size ? *(Base::end() - 1) : *Base::end(); }

      void push_back(const T& val) {
#ifdef STATOR_DEBUG
	if (_size+1 > Nmax)
	  stator_throw() << "Cannot push elements to a filled StackVector " << *this;
#endif
	Base::operator[](_size) = val;
	++_size;
      }

      T pop_back() {
#ifdef STATOR_DEBUG
	if (empty())
	  stator_throw() << "Cannot pop elements from an emptry StackVector " << *this;
#endif
	return Base::operator[](--_size);
      }

      template<size_t Nmax2>
      void extend(const StackVector<T,Nmax2>& ovec) {
	for (const T& a: ovec)
	  push_back(a);
      }
      
    private:
      size_t _size;
    };

    template<class T, size_t Nmax>
    std::ostream& operator<<(std::ostream& os, const StackVector<T,Nmax>&s) {
      os << "StackVector{ ";
      for (const auto& val : s)
	os << val << " ";
      os << "}";
      return os;
    }

    template<class T1, class T2, size_t Nmax>
    std::ostream& operator<<(std::ostream& os, const StackVector<std::pair<T1,T2>,Nmax>&s) {
      os << "StackVector{ ";
      for (const auto& val : s)
	os << "[" << val.first << ", " << val.second << "] ";
      os << "}";
      return os;
    }

    template<std::size_t I = 0, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type
    tuple_print(const std::tuple<Tp...>& t, std::ostream& os)
    { }
    
    template<std::size_t I = 0, typename... Tp>
    inline typename std::enable_if<I < sizeof...(Tp), void>::type
    tuple_print(const std::tuple<Tp...>& t, std::ostream& os)
    {
      os << std::get<I>(t) << " ";
      tuple_print<I + 1, Tp...>(t, os);
    }
      
    template<size_t Nmax, typename... Tp>
    std::ostream& operator<<(std::ostream& os, const StackVector<std::tuple<Tp...>,Nmax>&s) 
    {
      os << "StackVector{ ";
      for (const auto& val : s) {
	os << "[";
	tuple_print(val, os);
	os << "] ";
      }
      os << "}";
      return os;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////    Compile time constants         /////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    //These are implemented using std::ratio. We use inheritance to create
    //this as a new ratio type in the stator::symbolic namespace to ensure
    //operator lookups consider this namespace.
    template<std::intmax_t Num, std::intmax_t Denom = 1>
    struct ratio : std::ratio<Num, Denom> {};

    //Compile time constants with special meaning
    typedef ratio<0> NullSymbol;
    typedef ratio<1> UnitySymbol;
    //C++11 requires implementations support long long (64bits), so
    //ratio's may have up to 2^32 in numerator or denominator. We
    //choose rational approximations close to the limits of double
    //precision. Higher precision approximations require significantly
    //more digits.
    typedef ratio<47627751, 15160384> pi; //approximation of pi with an error < 1.60e-14
    typedef ratio<28245729, 10391023> e; //approximation of e with an error < 8.89e-16

    /*! \brief Output operator for ratio types */
    template<std::intmax_t Num, std::intmax_t Denom>
    inline std::ostream& operator<<(std::ostream& os, const std::ratio<Num, Denom>) {
      os << Num;
      if (Denom != 1)
	os << "/" << Denom ;
      return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const pi) { os << "π"; return os; }
    inline std::ostream& operator<<(std::ostream& os, const e) { os << "e"; return os; }

    template<class T>
    inline std::ostream& operator<<(std::ostream& os, const std::complex<T>& c) {
      if (c.imag() == 0)
	return (os << c.real());
      if (c.imag() < 0)
	return (os << "(" <<c.real() << " - " << std::abs(c.imag()) << "i)");

      return (os << "(" <<c.real() << " + " << c.imag() << "i)");
    }

    namespace detail {
      /*!\brief Type trait to determine if a certain type is a
	symbolic representation of a constant.
	
	This is used to enable the derivative operation to convert
	these types to NullSymbol types. It is also to apply a
	specialised functions to these types.
      */
      template <class T>
      struct IsSymbolicConstant {
	static const bool value = false;
      };

      template<std::intmax_t Num, std::intmax_t Denom>
      struct IsSymbolicConstant<ratio<Num, Denom> > {
	static const bool value = true;
      };
    }
    
    template<class T,
	     typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    T toArithmetic(T val) { return val; }
    
    template<std::intmax_t n1, std::intmax_t d1, 
    	     typename = typename std::enable_if<!(n1 % d1)>::type> 
    std::intmax_t toArithmetic(ratio<n1,d1> val) { return n1 / d1; }
    
    template<std::intmax_t n1, std::intmax_t d1, 
    	     typename = typename std::enable_if<n1 % d1>::type>
    double toArithmetic(ratio<n1,d1> val) { return double(n1) / double(d1); }

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
	these types to NullSymbol types. It is also to apply a
	specialised functions to these types.
      */
      template<class T>
      struct IsConstant {
	static const bool value = std::is_arithmetic<T>::value || IsSymbolicConstant<T>::value || std::is_base_of<Eigen::EigenBase<T>, T>::value;
      };

      template<class Derived>
      struct IsConstant<Eigen::MatrixBase<Derived> > {
	static const bool value = true;
      };

      template<class T>
      struct IsConstant<std::complex<T> > {
	static const bool value = IsConstant<T>::value;
      };
    }

    /*! \brief Returns the empty sum of a type.
      
      The empty sum is a term whose additive (and typically its
      subtractive) action is null (can be ignored). This definition
      only applies for selected types and assumes that their default
      constructors create the empty sum.
    */
    template<class T>
    constexpr auto empty_sum(const T&) -> typename std::enable_if<detail::IsConstant<T>::value, T>::type { 
      return T();
    }

    /*! \brief Evaluates a symbolic expression by substituting a
      variable for another expression.
	
      If a arithmetic type is substituted, this will likely cause a
      numerical evaluation of the expression. This "helper"
      implementation converts to a substitution for the variable
      'x'.
    */
    template<class T, class VarArg>
    auto eval(const T& f, const VarArg& xval) -> decltype(substitution(f, Variable<'x'>() == xval))
    { return substitution(f, Variable<'x'>() == xval); }

    /*! \brief Evaluates a symbolic expression using a substitution.
      
      This is just a synonym for substitution.
    */
    template<class T, char Letter, class Arg>
    auto eval(const T& f, const VariableSubstitution<Letter, Arg>& x)  -> decltype(substitution(f, x))
    { return substitution(f,x); }
    
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
    
    /*! \} */
  
    /*! \brief Determine the derivative of a symbolic expression.
      
      This default implementation gives all consants
      derivative of zero.
    */
    template<class T, char Letter,
	     typename = typename std::enable_if<detail::IsConstant<T>::value>::type>
    NullSymbol derivative(const T&, Variable<Letter>) { return NullSymbol(); }

    /*! \brief Determine the derivative of a variable.

      If the variable is the variable in which a derivative is being
      taken, then this overload should be selected to return
      UnitySymbol.
    */
    template<char Letter1, char Letter2,
	     typename = typename std::enable_if<Letter1 == Letter2>::type>
    UnitySymbol derivative(Variable<Letter1>, Variable<Letter2>) { return UnitySymbol(); }

    inline StackVector<double, 0> solve_real_roots(NullSymbol f) {
      return StackVector<double, 0>();
    }

    /*! \brief Determine the derivative of a variable by another variable.

      If the variable is NOT the variable in which a derivative is
      being taken, then this overload should be selected to return
      NullSymbol.
    */
    template<char Letter1, char Letter2,
	     typename = typename std::enable_if<Letter1 != Letter2>::type>
    NullSymbol derivative(Variable<Letter1>, Variable<Letter2>) { return NullSymbol(); }
 
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
     
      This template implementation returns UnitySymbol for 0! and 1!,
      allowing simplification of symbolic expressions.
     */
    template<size_t i> struct Factorial {
      static size_t eval() { return i * Factorial<i-1>::eval(); }
    };
    
    template<> struct Factorial<1> {
      static UnitySymbol eval() { return UnitySymbol(); }
    };

    template<> struct Factorial<0> {
      static UnitySymbol eval() { return UnitySymbol(); }
    };

    /*! \brief Symbolic Inverse factorial function.
     
      This template implementation returns UnitySymbol for 1/0! and 1/1!,
      allowing simplification of symbolic expressions.
     */
    template<size_t i> struct InvFactorial {
      static double eval() { return 1.0 / Factorial<i>::eval(); }
    };
    
    template<> struct InvFactorial<1> {
      static UnitySymbol eval() { return UnitySymbol(); }
    };

    template<> struct InvFactorial<0> {
      static UnitySymbol eval() { return UnitySymbol(); }
    };
  }
}

#include "stator/symbolic/operators.hpp"
#include "stator/symbolic/functions.hpp"
#include "stator/symbolic/polynomial.hpp"
#include "stator/symbolic/simplify.hpp"

namespace stator {
  namespace symbolic {
    namespace detail {
      template<size_t State, size_t max_Order, char Letter>
      struct TaylorSeriesWorker {
	template<class Real>
	static NullSymbol eval(const NullSymbol& f, const Real& a)
	{ return NullSymbol(); }
        
	template<class F, class Real>
	static auto eval(const F& f, const Real& a) -> decltype(InvFactorial<State>::eval() * substitution(f, Variable<Letter>() == a) + (Variable<Letter>() - a) * TaylorSeriesWorker<State+1, max_Order, Letter>::eval(derivative(f, Variable<Letter>()), a))
	{
	  return InvFactorial<State>::eval() * substitution(f, Variable<Letter>() == a) + (Variable<Letter>()-a) * TaylorSeriesWorker<State+1, max_Order, Letter>::eval(derivative(f, Variable<Letter>()), a);
	}
      };

      template<size_t max_Order, char Letter>
      struct TaylorSeriesWorker<max_Order,max_Order,Letter> {
	template<class F, class Real>
	static auto eval(const F& f, const Real& a) -> decltype(InvFactorial<max_Order>::eval() * substitution(f, Variable<Letter>() == a))
	{ return InvFactorial<max_Order>::eval() * substitution(f, Variable<Letter>() == a); }

	template<class Real>
	static NullSymbol eval(const NullSymbol& f, const Real& a)
	{ return NullSymbol(); }
      };
    }

    /*! \brief Generate a Taylor series representation of a Symbolic
        expression.
     */
    template<size_t Order, char Letter, class F, class Real>
    auto taylor_series(const F& f, Real a) -> decltype(try_simplify(detail::TaylorSeriesWorker<0, Order, Letter>::eval(f, a)))
    { return try_simplify(detail::TaylorSeriesWorker<0, Order, Letter>::eval(f, a)); }
  }
}
