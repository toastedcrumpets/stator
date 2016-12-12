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

namespace sym {
  struct BinaryOpBase {};
  
  /*! \brief Symbolic representation of a binary symbolic operation. 
   */
  template<class LHS, typename Op, class RHS>
  struct BinaryOp: BinaryOpBase, SymbolicOperator {
    LHS _l;
    RHS _r;
    BinaryOp(const LHS& l, const RHS& r): _l(l), _r(r) {}
  };
  
  template<class LHS, class RHS, class Op>
  inline std::ostream& operator<<(std::ostream& os, const BinaryOp<LHS, Op, RHS>& op) {
    os << "(" << op._l << " " << Op::_str <<  " " << op._r << ")";
    return os;
  }

  template<class LHS, class RHS, class Op, class Var, class Arg> 
  auto sub(BinaryOp<LHS, Op, RHS> f, VarSub<Var, Arg> x)
    -> STATOR_AUTORETURN_BYVALUE(Op::apply(sub(f._l, x), sub(f._r, x)));

  template<class LHS, class RHS,
	   typename = typename std::enable_if<(std::is_arithmetic<LHS>::value && std::is_arithmetic<RHS>::value)>::type>
  auto pow(const LHS& l, const RHS& r) -> STATOR_AUTORETURN(std::pow(l, r));

  template<class LHS, std::intmax_t num, std::intmax_t den,
	   typename = typename std::enable_if<std::is_arithmetic<LHS>::value>::type>
  auto pow(const LHS& l, const C<num, den>& r) -> STATOR_AUTORETURN(std::pow(l, decltype(r+l)(r)));
  
  template<class LHS,
	   typename = typename std::enable_if<std::is_arithmetic<LHS>::value>::type>
  auto pow(const LHS& l, const C<1,3>& r) -> STATOR_AUTORETURN(std::cbrt(l));

  template<class LHS,
	   typename = typename std::enable_if<std::is_arithmetic<LHS>::value>::type>
  auto pow(const LHS& l, const C<1,2>& r) -> STATOR_AUTORETURN(std::sqrt(l));

  template<class LHS,
	   typename = typename std::enable_if<std::is_base_of<Eigen::EigenBase<LHS>, LHS>::value>::type>
  auto pow(const LHS& l, const C<2,1>& r) -> STATOR_AUTORETURN_BYVALUE(l.squaredNorm());
  
  template<class LHS>
  auto pow(const LHS& l, const C<1,1>& r) -> STATOR_AUTORETURN(l);

  namespace {
    /*! \brief Generic implementation of the eval routine for PowerOp.
	
      As the types of non-arithmetic arguments to PowerOp might
      change with each round of multiplication, we must be careful
      to accommodate this using templated looping. This class
      achieves this.
    */
    template<size_t Power>
    struct PowerOpSub {
      template<class Arg_t>
      static auto eval(Arg_t x)
        -> STATOR_AUTORETURN(PowerOpSub<Power-1>::eval(x) * x)
	};

    template<>
    struct PowerOpSub<1> {
      template<class Arg_t> static Arg_t eval(Arg_t x) { return x; }
    };

    template<>
    struct PowerOpSub<0> {
      template<class Arg_t> static Unity eval(Arg_t x) { return Unity(); }
    };
  }
  
  template<std::intmax_t num1, std::intmax_t den1, std::intmax_t num2>
  auto pow(const C<num1, den1>& l, const C<num2,1>& r)
    -> STATOR_AUTORETURN(PowerOpSub<num2>::eval(sub(l, num2)));

  namespace detail {
    struct Add {
      static constexpr bool commutative = true;
      static constexpr bool associative = true;
      typedef Null left_identity;
      typedef Null right_identity;
      static constexpr const char* _str = "+";
      //Apply has to accept by const ref, as returned objs may reference/alias the arguments, so everything needs at least the parent scope
      template<class L, class R> static auto apply(const L& l, const R& r) -> STATOR_AUTORETURN(l + r);
    };

    struct Subtract {
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      typedef Null right_identity;
      static constexpr const char* _str = "-";
      template<class L, class R> static auto apply(const L& l, const R& r) -> STATOR_AUTORETURN(l - r);
    };

    struct Multiply {
      static constexpr bool commutative = true;
      static constexpr bool associative = true;
      typedef Unity left_identity;
      typedef Unity right_identity;
      typedef Null left_zero;
      typedef Null right_zero;
      static constexpr const char* _str = "×";
      template<class L, class R> static auto apply(const L& l, const R& r) -> STATOR_AUTORETURN(l * r);
    };

    struct Divide {
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      typedef Unity right_identity;
      typedef Null left_zero;
      static constexpr const char* _str = "÷";
      template<class L, class R> static auto apply(const L& l, const R& r) -> STATOR_AUTORETURN(l / r);
    };

    struct Dot {
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      static constexpr const char* _str = "•";
      template<class L, class R> static auto apply(const L& l, const R& r) -> STATOR_AUTORETURN(l.dot(r));
    };

    struct Power {
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      static constexpr const char* _str = "^";
      typedef Unity right_identity;
      typedef Unity left_zero;
      template<class L, class R> static auto apply(const L& l, const R& r) -> STATOR_AUTORETURN(sym::pow(l, r));
    };
  }

  template<class LHS, class RHS> using AddOp      = BinaryOp<LHS, detail::Add, RHS>;
  template<class LHS, class RHS> using SubtractOp = BinaryOp<LHS, detail::Subtract, RHS>;    
  template<class LHS, class RHS> using MultiplyOp = BinaryOp<LHS, detail::Multiply, RHS>;
  template<class LHS, class RHS> using DivideOp   = BinaryOp<LHS, detail::Divide, RHS>;
  template<class LHS, class RHS> using DotOp      = BinaryOp<LHS, detail::Dot, RHS>;
  template<class LHS, class RHS> using PowerOp   = BinaryOp<LHS, detail::Power, RHS>;

  template <class Op, class OverOp>
  struct left_distributive : std::false_type {};

  template <class Op, class OverOp>
  struct right_distributive { static constexpr bool value = Op::commutative && left_distributive<Op,OverOp>::value; };
  
  template <class Op, class OverOp>
  struct distributive { static constexpr bool value = right_distributive<Op,OverOp>::value && left_distributive<Op,OverOp>::value; };

  template<> struct left_distributive<detail::Multiply, detail::Add> : std::true_type {};
  
  template<> struct left_distributive<detail::Dot, detail::Add> : std::true_type {};
  template<> struct right_distributive<detail::Dot, detail::Add> : std::true_type {};
  
  template<> struct right_distributive<detail::Divide, detail::Add> : std::true_type {};

  template<> struct right_distributive<detail::Power, detail::Multiply> : std::true_type {};
  
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
    -> STATOR_AUTORETURN((AddOp<decltype(store(l)), decltype(store(r))>(l, r)))

  /*! \brief Symbolic multiplication operator. */
    template<class LHS, class RHS,
	     typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
    auto operator*(const LHS& l, const RHS& r) 
    -> STATOR_AUTORETURN((MultiplyOp<decltype(store(l)), decltype(store(r))>(l, r)))

  /*! \brief Symbolic subtraction operator. */
  template<class LHS, class RHS,
	   typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
    auto operator-(const LHS& l, const RHS& r) 
  -> STATOR_AUTORETURN((SubtractOp<decltype(store(l)), decltype(store(r))>(l, r)))

  /*! \brief Symbolic divide operator. */
  template<class LHS, class RHS,
	   typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
    auto operator/(const LHS& l, const RHS& r) 
  -> STATOR_AUTORETURN((DivideOp<decltype(store(l)), decltype(store(r))>(l, r)))

  /*! \brief Symbolic dot operator. */
    template<class LHS, class RHS,
	     typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
    auto dot(const LHS& l, const RHS& r) 
  -> STATOR_AUTORETURN((DotOp<decltype(store(l)), decltype(store(r))>(l, r)));

  template<class LHS, class RHS,
	   typename = typename std::enable_if<!ApplySymbolicOps<LHS, RHS>::value>::type>
  auto dot(const LHS& l, const RHS& r) 
    -> STATOR_AUTORETURN(detail::Dot::apply(l, r));
  
  /*! \brief Symbolic power operator. */
  template<class LHS, class RHS,
	   typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
  auto pow(const LHS& l, const RHS& r) 
    -> STATOR_AUTORETURN((PowerOp<decltype(store(l)), decltype(store(r))>(l, r)));

  /*! \brief Derivatives of AddOp operations.
   */
  template<class Var, class LHS, class RHS>
  auto derivative(const AddOp<LHS, RHS>& f, Var)
    -> STATOR_AUTORETURN(derivative(f._l, Var()) + derivative(f._r, Var()))

  /*! \brief Derivatives of SubtractOp operations.
   */
    template<class Var, class LHS, class RHS>
    auto derivative(const SubtractOp<LHS, RHS>& f, Var) 
    -> STATOR_AUTORETURN(derivative(f._l, Var()) - derivative(f._r, Var()))

  /*! \brief Derivatives of MultiplyOp operations.
   */
    template<class Var, class LHS, class RHS>
    auto derivative(const MultiplyOp<LHS, RHS>& f, Var) 
    -> STATOR_AUTORETURN(derivative(f._l, Var()) * f._r + f._l * derivative(f._r, Var()))

  /*! \} */

  /*! \brief Derivatives of PowerOp operations.
   */
  template<class Var, class Arg, std::intmax_t num, std::intmax_t den>
  auto derivative(const PowerOp<Arg, C<num, den> >& f, Var)
    -> STATOR_AUTORETURN((C<num, den>() * derivative(f._l, Var()) * pow(f._l, C<num, den>()-C<1>())));
  /*! \}*/
}

    
