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

namespace stator {
  namespace symbolic {
    struct BinaryOpBase {};
    
    /*! \brief Symbolic representation of a binary symbolic operation. 
     */
    template<class LHStype, class RHStype, typename Derived>
    struct BinaryOp: BinaryOpBase, SymbolicOperator {
      LHStype _l;
      RHStype _r;
      BinaryOp(const LHStype& l, const RHStype& r): _l(l), _r(r) {}
    };
    
    template<class LHS, class RHS, class Derived>
    inline std::ostream& operator<<(std::ostream& os, const BinaryOp<LHS, RHS, Derived>& op) {
      os << "(" << op._l << ") " << Derived::_str <<  " (" << op._r << ")";
      return os;
    }

    template<class LHS, class RHS, class Derived, char Letter, class Arg> 
    auto substitution(BinaryOp<LHS, RHS, Derived> f, VariableSubstitution<Letter, Arg> x) 
      -> STATOR_AUTORETURN(Derived::apply(substitution(f._l, x), substitution(f._r, x)));
    
    /*! \brief Type trait which denotes if operations should be
        reordered to bring these types together. 

	This is true for all arithmetic types, as operations on these
	generally can be precalculate by the compiler into a single
	term. All symbolic constants are also reordered.
    */
    template<class T1, class T2> struct Reorder {
      static const bool value = std::is_arithmetic<T1>::value && std::is_arithmetic<T2>::value;
    };

    template<char Letter1, char Letter2> 
    struct Reorder<Variable<Letter1>, Variable<Letter2> > {
      static const bool value = Letter1 == Letter2;
    };

    template<std::intmax_t n1, std::intmax_t d1, std::intmax_t n2, std::intmax_t d2> 
    struct Reorder<C<n1,d1>, C<n2,d2> > {
      static const bool value = true;
    };
    
#define CREATE_BINARY_OP(HELPERNAME, CLASSNAME, OP, PRINTFORM)		\
    template<class LHStype, class RHStype>				\
    struct CLASSNAME : public BinaryOp<LHStype, RHStype, CLASSNAME<LHStype, RHStype> > { \
      typedef BinaryOp<LHStype, RHStype, CLASSNAME<LHStype, RHStype> > Base;			\
      CLASSNAME(const LHStype& l, const RHStype& r): Base(l, r) {}	\
      static constexpr const char* _str = PRINTFORM;			\
      template<class L, class R>					\
      static auto apply(L l, R r) -> STATOR_AUTORETURN((l) OP (r));	\
    };
									    
    CREATE_BINARY_OP(add, AddOp, +, "+")
    CREATE_BINARY_OP(subtract, SubtractOp, -, "-")
    CREATE_BINARY_OP(multiply, MultiplyOp, *, "×")
    CREATE_BINARY_OP(divide, DivideOp, /, "÷")
    CREATE_BINARY_OP(dot, DotOp, .dot , "•")

    /*! \name Symbolic algebra
      \{
    */

    /*! \brief Simple combination rule to enable Symbolic operations,
        but avoid redundantly specifying where two SymbolicOperator
        classes are operated on. */
    template<class LHS, class RHS> 
    struct ApplySymbolicOps {
      static const bool value = IsSymbolic<LHS>::value || (!IsSymbolic<LHS>::value && IsSymbolic<RHS>::value);
    };

    /*! \brief Symbolic unary positive operator. */
    template<class Arg,
	     typename = typename std::enable_if<IsSymbolic<SymbolicOperator>::value>::type>
    Arg operator+(const Arg& l) { return l; }

    /*! \brief Symbolic unary negation operator. */
    template<class Arg,
	     typename = typename std::enable_if<IsSymbolic<SymbolicOperator>::value>::type>
    auto operator-(const Arg& l) -> STATOR_AUTORETURN(C<-1>() * l)

    /*! \brief Symbolic addition operator. */
    template<class LHS, class RHS,
	     typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
    auto operator+(const LHS& l, const RHS& r) 
      -> STATOR_AUTORETURN((AddOp<LHS, RHS>(l, r)))

    /*! \brief Symbolic multiplication operator. */
    template<class LHS, class RHS,
	     typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
    auto operator*(const LHS& l, const RHS& r) 
      -> STATOR_AUTORETURN((MultiplyOp<LHS, RHS>(l, r)))

    /*! \brief Symbolic subtraction operator. */
    template<class LHS, class RHS,
	     typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
    auto operator-(const LHS& l, const RHS& r) 
    -> STATOR_AUTORETURN((SubtractOp<LHS, RHS>(l, r)))

    /*! \brief Symbolic divide operator. */
    template<class LHS, class RHS,
	     typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
    auto operator/(const LHS& l, const RHS& r) 
    -> STATOR_AUTORETURN((DivideOp<LHS, RHS>(l, r)))

    /*! \brief Derivatives of AddOp operations.
     */
    template<char dVariable, class LHS, class RHS>
    auto derivative(const AddOp<LHS, RHS>& f, Variable<dVariable>)
      -> STATOR_AUTORETURN(derivative(f._l, Variable<dVariable>()) + derivative(f._r, Variable<dVariable>()))

    /*! \brief Derivatives of SubtractOp operations.
     */
    template<char dVariable, class LHS, class RHS>
    auto derivative(const SubtractOp<LHS, RHS>& f, Variable<dVariable>) 
      -> STATOR_AUTORETURN(derivative(f._l, Variable<dVariable>()) - derivative(f._r, Variable<dVariable>()))

    /*! \brief Derivatives of MultiplyOp operations.
     */
    template<char dVariable, class LHS, class RHS>
    auto derivative(const MultiplyOp<LHS, RHS>& f, Variable<dVariable>) 
    -> STATOR_AUTORETURN(derivative(f._l, Variable<dVariable>()) * f._r + f._l * derivative(f._r, Variable<dVariable>()))

    /*! \} */

    namespace {
      /*! \brief Generic implementation of the eval routine for PowerOp.
	
	As the types of non-arithmetic arguments to PowerOp might
	change with each round of multiplication, we must be careful
	to accommodate this using templated looping. This class
	achieves this.
      */
      template<size_t Power>
      struct PowerOpSubstitution {
	template<class Arg_t>
	static auto eval(Arg_t x)
          -> STATOR_AUTORETURN(PowerOpSubstitution<Power-1>::eval(x) * x)
      };

      template<>
      struct PowerOpSubstitution<1> {
	template<class Arg_t> static Arg_t eval(Arg_t x) { return x; }
      };

      template<>
      struct PowerOpSubstitution<0> {
	template<class Arg_t> static Unity eval(Arg_t x) { return Unity(); }
      };
    }

    /*! \brief Symbolic representation of a (positive) power operator.
     */
    template<class Arg, size_t Power>
    struct PowerOp: SymbolicOperator {
      Arg _arg;
      PowerOp(Arg a): _arg(a) {}
      PowerOp(): _arg() {}
    };
    
    /*! \brief Evaluate PowerOp symbol at a value of x.
      
      This operator is only used if the result of evaluating the
      argument is an arithmetic type. If this is the case, the
      evaluation is passed to std::pow.
    */
    template<class Arg1, size_t Power, class Arg2, char Letter>
    constexpr auto substitution(const PowerOp<Arg1, Power>& f, const VariableSubstitution<Letter, Arg2>& x)
      -> typename std::enable_if<std::is_arithmetic<decltype(substitution(f._arg, x))>::value,
				 decltype(std::pow(substitution(f._arg, x), Power))>::type
    { return std::pow(substitution(f._arg, x), Power); }

    /*! \brief Evaluate PowerOp symbol at a value of x.
      
      This is the general implementation for PowerOp.
    */
    template<class Arg1, size_t Power, class Arg2, char Letter>
    auto substitution(const PowerOp<Arg1, Power>& f, const VariableSubstitution<Letter, Arg2>& x)
      -> typename std::enable_if<!std::is_arithmetic<decltype(substitution(f._arg, x))>::value,
				 decltype(PowerOpSubstitution<Power>::eval(substitution(f._arg, x)))>::type
    { return PowerOpSubstitution<Power>::eval(substitution(f._arg, x)); }


    /*! \relates PowerOp
      \name PowerOp helper functions.
    */
    /*! \brief Helper function for creating PowerOp types. */
    template<size_t Power, class Arg>
    typename std::enable_if<!detail::IsConstant<Arg>::value, PowerOp<Arg, Power> >::type
    pow(const Arg& f)
    { return PowerOp<Arg, Power>(f); }

    /*! \brief Helper function for immediately evaluating powers of constants. */
    template<size_t Power, class Arg,
             typename = typename std::enable_if<detail::IsConstant<Arg>::value && !std::is_base_of<Eigen::EigenBase<Arg>, Arg>::value>::type>
    auto pow(const Arg& f) 
      -> STATOR_AUTORETURN(PowerOpSubstitution<Power>::eval(f))

    /*! \brief Specialisation for squares of matrix expressions. */
    template<size_t Power, class Arg,
             typename = typename std::enable_if<(Power==2) && std::is_base_of<Eigen::EigenBase<Arg>, Arg>::value>::type>
      auto pow(const Arg& f) 
      -> STATOR_AUTORETURN_BYVALUE(f.dot(f))

    /*! \} */

    /*! \relates PowerOp
      \name PowerOp input/output operators.
    */
    /*! \brief Writes a human-readable representation of the BinaryOp to the output stream. */
    template<class Arg, size_t Power>
    inline std::ostream& operator<<(std::ostream& os, const PowerOp<Arg, Power>& p) {
      os << "(" << p._arg << ")^" << Power;
      return os;
    }
    /*! \} */

    
    /*! \relates PowerOp
      \name PowerOp operations
      \{
    */

    /*! \brief Derivatives of PowerOp operations.
     */
    template<char dVariable, class Arg, size_t Power>
    auto derivative(const PowerOp<Arg, Power>& f, Variable<dVariable>) 
      -> STATOR_AUTORETURN((C<Power>() * derivative(f._arg, Variable<dVariable>()) * PowerOp<Arg, Power-1>(f._arg)))

    template<char dVariable, class Arg>
    Null derivative(const PowerOp<Arg, 0>& f, Variable<dVariable>)
    { return Null(); }

    template<char dVariable, class Arg>
    auto derivative(const PowerOp<Arg, 1>& f, Variable<dVariable>) 
      -> STATOR_AUTORETURN(derivative(f._arg, Variable<dVariable>()))

    template<char dVariable, class Arg>
    auto derivative(const PowerOp<Arg, 2>& f, Variable<dVariable>) 
      -> STATOR_AUTORETURN(C<2>() * derivative(f._arg, Variable<dVariable>()) * f._arg)
    /*! \}*/
  }
}
    
