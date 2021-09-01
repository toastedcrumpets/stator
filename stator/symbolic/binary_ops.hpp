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

namespace sym {
  /*! \brief Symbolic representation of a binary symbolic operation. 
   */
  template<class LHS, typename Op, class RHS>
  struct BinaryOp: SymbolicOperator {
  protected:
    BinaryOp(const LHS& l, const RHS& r): _l(l), _r(r) {}
  public:
    static BinaryOp create(const LHS& l, const RHS& r) { return BinaryOp(l, r); }
    
    const LHS _l;
    const RHS _r;
  };
  
  namespace detail {
    template<typename LHS, typename Op, typename RHS>
    struct Type_index<BinaryOp<LHS, Op, RHS>> { static const int value = Op::type_index;  };

    struct NoIdentity {
      template<class T>
      constexpr bool operator==(const T&) const { return false; }
    };

    enum class Associativity { LEFT, RIGHT, NONE };

    template<class Op>
    constexpr int RBP() {
      return Op::leftBindingPower + (Op::associativity == Associativity::LEFT) + (Op::associativity == Associativity::NONE);
    }
    
    template<class Op>
    constexpr int NBP() {
      return Op::leftBindingPower - (Op::associativity == Associativity::RIGHT) - (Op::associativity == Associativity::NONE);
    }

    struct Add {
      static constexpr int leftBindingPower = 20;
      static constexpr auto associativity = Associativity::LEFT;
      static constexpr bool commutative = true;
      static constexpr bool associative = true;
      typedef Null left_identity;
      typedef Null right_identity;
      typedef NoIdentity left_zero;
      typedef NoIdentity right_zero;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "+"; }
      static inline std::string r_repr() { return ""; }
      static inline std::string l_latex_repr() { return ""; }
      static inline std::string latex_repr() { return "+"; }
      static inline std::string r_latex_repr() { return ""; }
      //Apply has to accept by const ref, as returned objs may reference/alias the arguments, so everything needs at least the parent scope
      template<class L, class R> static auto apply(const L& l, const R& r) { return l + r; }
      static constexpr int type_index = 8;
    };

    struct Subtract {
      static constexpr int leftBindingPower = 20;
      static constexpr auto associativity = Associativity::LEFT;
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      typedef NoIdentity left_identity;
      typedef Null right_identity;
      typedef NoIdentity left_zero;
      typedef NoIdentity right_zero;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "-"; }
      static inline std::string r_repr() { return ""; }
      static inline std::string l_latex_repr() { return ""; }
      static inline std::string latex_repr() { return "-"; }
      static inline std::string r_latex_repr() { return ""; }
      template<class L, class R> static auto apply(const L& l, const R& r) { return l - r; }
      static constexpr int type_index = 9;
    };

    struct Multiply {
      static constexpr int leftBindingPower = 30;
      static constexpr auto associativity = Associativity::LEFT;
      static constexpr bool commutative = true;
      static constexpr bool associative = true;
      typedef Unity left_identity;
      typedef Unity right_identity;
      typedef Null left_zero;
      typedef Null right_zero;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "*"; }
      static inline std::string r_repr() { return ""; }
      static inline std::string l_latex_repr() { return ""; }
      static inline std::string latex_repr() { return "\\times "; }
      static inline std::string r_latex_repr() { return ""; }
      template<class L, class R> static auto apply(const L& l, const R& r) { return l * r; }
      static constexpr int type_index = 10;
    };

    struct Divide {
      static constexpr int leftBindingPower = 30;
      static constexpr auto associativity = Associativity::LEFT;
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      typedef NoIdentity left_identity;
      typedef Unity right_identity;
      typedef Null left_zero;
      typedef NoIdentity right_zero;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "/"; }
      static inline std::string r_repr() { return ""; }
      static inline std::string l_latex_repr() { return "\\frac{"; }
      static inline std::string latex_repr() { return "}{"; }
      static inline std::string r_latex_repr() { return "}"; }
      template<class L, class R> static auto apply(const L& l, const R& r) { return l / r; }
      static constexpr int type_index = 11;
    };

    struct Power {
      static constexpr int leftBindingPower = 40;
      static constexpr auto associativity = Associativity::RIGHT;
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "^"; }
      static inline std::string r_repr() { return ""; }
      static inline std::string l_latex_repr() { return ""; }
      static inline std::string latex_repr() { return "^{"; }
      static inline std::string r_latex_repr() { return "}"; }
      typedef NoIdentity left_identity;
      typedef Unity right_identity;
      typedef NoIdentity right_zero;
      typedef Unity left_zero;
      //We have to prevent silly powers (i.e. matrix powers) otherwise
      //the MSVSC compiler gets confused
      template<class L, class R,
	       typename = typename std::enable_if<!std::is_base_of<Eigen::EigenBase<R>, R>::value>::type>
      static auto apply(const L& l, const R& r) { return pow(l, r); }
      static constexpr int type_index = 12;
    };

    struct Equality {
      static constexpr int leftBindingPower = 10;
      static constexpr auto associativity = Associativity::RIGHT;
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      typedef NoIdentity left_identity;
      typedef NoIdentity right_identity;
      typedef NoIdentity left_zero;
      typedef NoIdentity right_zero;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "="; }
      static inline std::string r_repr() { return ""; }
      static inline std::string l_latex_repr() { return ""; }
      static inline std::string latex_repr() { return "="; }
      static inline std::string r_latex_repr() { return ""; }
      template<class L, class R> static auto apply(const L& l, const R& r) {
	return BinaryOp<decltype(store(l)), detail::Equality, decltype(store(r))>::create(l, r);
      }
      static constexpr int type_index = 13;
    };

    struct ArrayAccess {
      static constexpr int leftBindingPower = 50;
      static constexpr auto associativity = Associativity::LEFT;
      static constexpr bool commutative = false;
      static constexpr bool associative = false;
      static inline std::string l_repr() { return ""; }
      static inline std::string repr() { return "["; }
      static inline std::string r_repr() { return "]"; }
      static inline std::string l_latex_repr() { return ""; }
      static inline std::string latex_repr() { return "\\left["; }
      static inline std::string r_latex_repr() { return "\\right]"; }
      typedef NoIdentity left_identity;
      typedef NoIdentity right_identity;
      typedef NoIdentity right_zero;
      typedef NoIdentity left_zero;
      template<class L, class R>
      static auto apply(const L& l, const R& r) {
        return l[r];
      }
      static constexpr int type_index = 14;
    };
  }

  template<class LHS, class RHS> using AddOp      = BinaryOp<LHS, detail::Add,         RHS>;
  template<class LHS, class RHS> using SubtractOp = BinaryOp<LHS, detail::Subtract,    RHS>;    
  template<class LHS, class RHS> using MultiplyOp = BinaryOp<LHS, detail::Multiply,    RHS>;
  template<class LHS, class RHS> using DivideOp   = BinaryOp<LHS, detail::Divide,      RHS>;
  template<class LHS, class RHS> using PowerOp    = BinaryOp<LHS, detail::Power,       RHS>;
  template<class LHS, class RHS> using EqualityOp = BinaryOp<LHS, detail::Equality,    RHS>;
  template<class LHS, class RHS> using ArrayOp    = BinaryOp<LHS, detail::ArrayAccess, RHS>;

  template <class Op, class OverOp>
  struct left_distributive : std::false_type {};

  template <class Op, class OverOp>
  struct right_distributive { static constexpr bool value = Op::commutative && left_distributive<Op,OverOp>::value; };
  
  template <class Op, class OverOp>
  struct distributive { static constexpr bool value = right_distributive<Op,OverOp>::value && left_distributive<Op,OverOp>::value; };

  template<> struct left_distributive<detail::Multiply, detail::Add> : std::true_type {};
  
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

  /*! \brief Symbolic addition operator. */
  template<class LHS, class RHS,
	   typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
  auto operator+(const LHS& l, const RHS& r)  {
    return store(AddOp<decltype(store(l)), decltype(store(r))>::create(l, r));
  }

  /*! \brief Symbolic multiplication operator. */
  template<class LHS, class RHS,
	   typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
  auto operator*(const LHS& l, const RHS& r) {
    return store(MultiplyOp<decltype(store(l)), decltype(store(r))>::create(l, r));
  }

  /*! \brief Symbolic subtraction operator. */
  template<class LHS, class RHS,
	   typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
  auto operator-(const LHS& l, const RHS& r) {
    return store(SubtractOp<decltype(store(l)), decltype(store(r))>::create(l, r));
  }

  /*! \brief Symbolic divide operator. */
  template<class LHS, class RHS,
	   typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
  auto operator/(const LHS& l, const RHS& r) {
    return store(DivideOp<decltype(store(l)), decltype(store(r))>::create(l, r));
  }
  
  /*! \brief Symbolic power operator. */
  template<class LHS, class RHS,
	   typename = typename std::enable_if<ApplySymbolicOps<LHS, RHS>::value>::type>
  auto pow(const LHS& l, const RHS& r) {
    return store(PowerOp<decltype(store(l)), decltype(store(r))>::create(l, r));
  }

  template<class LHS, class RHS,
	   typename std::enable_if<(std::is_arithmetic<LHS>::value && std::is_arithmetic<RHS>::value), bool>::type = true>
  auto pow(const LHS& l, const RHS& r) { return std::pow(l, r); }

  template<class LHS, std::intmax_t num, std::intmax_t den,
	   typename std::enable_if<std::is_arithmetic<LHS>::value, bool>::type = true>
  auto pow(const LHS& l, const C<num, den>& r) { return std::pow(l, double(r)); } 
  
  template<class LHS,
	   typename = typename std::enable_if<std::is_arithmetic<LHS>::value>::type>
  auto pow(const LHS& l, const C<1,3>& r) { return std::cbrt(l); }

  template<class LHS,
	   typename = typename std::enable_if<std::is_arithmetic<LHS>::value>::type>
  auto pow(const LHS& l, const C<1,2>& r) { return std::sqrt(l); }

  template<class LHS,
	   typename = typename std::enable_if<std::is_base_of<Eigen::EigenBase<LHS>, LHS>::value>::type>
  auto pow(const LHS& l, const C<2,1>& r) { return store(l.squaredNorm()); }
  
  template<class LHS>
  auto pow(const LHS& l, const C<1,1>& r) { return l; }
  
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
      static auto eval(Arg_t x) {
        return PowerOpSub<Power-1>::eval(x) * x;
      }
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
  auto pow(const C<num1, den1>& l, const C<num2,1>& r) {
    return PowerOpSub<num2>::eval(sub(l, num2));
  }
  
  /*! \brief Symbolic equality operator. 

    This operator is always symbolic (we don't try to do assignment,
    it would be a mess of const correctness), so it only has this
    definition.
   */
  template<class LHS, class RHS>
  auto equality(const LHS& l, const RHS& r) {
    return store(EqualityOp<decltype(store(l)), decltype(store(r))>::create(l, r));
  }

  /*! \brief Symbolic array access operator. */
  template<class LHS, class RHS>
  auto array_access(const LHS& l, const RHS& r) {
    return store(ArrayOp<decltype(store(l)), decltype(store(r))>::create(l, r));
  }
  
  /*! \brief Derivatives of AddOp operations.
   */
  template<class Var, class LHS, class RHS>
  auto derivative(const AddOp<LHS, RHS>& f, const Var& v) {
    return derivative(f._l, v) + derivative(f._r, v);
  }

  /*! \brief Derivatives of SubtractOp operations.
   */
  template<class Var, class LHS, class RHS>
  auto derivative(const SubtractOp<LHS, RHS>& f, const Var& v) {
    return derivative(f._l, v) - derivative(f._r, v);
  }

  /*! \brief Derivatives of MultiplyOp operations.
   */
    template<class Var, class LHS, class RHS>
    auto derivative(const MultiplyOp<LHS, RHS>& f, const Var& v) {
      return derivative(f._l, v) * f._r + f._l * derivative(f._r, v);
    }

  /*! \brief Derivatives of DivideOp operations.

    This is the quotient rule
   */
    template<class Var, class LHS, class RHS>
    auto derivative(const DivideOp<LHS, RHS>& f, const Var& v) {
      return (derivative(f._l, v) * f._r - f._l * derivative(f._r, v)) / pow(f._r, C<2>());
    }

  /*! \brief Derivatives of PowerOp operation specialised for constant powers.
   */
  template<class Var, class Arg, std::intmax_t num, std::intmax_t den>
  auto derivative(const PowerOp<Arg, C<num, den> >& f, const Var& v) {
    return C<num, den>() * derivative(f._l, v) * pow(f._l, C<num, den>()-C<1>());
  }

  /*! \brief Derivative of a PowerOp operation.
   */
  template<class Var, class Arg, class Power>
  auto derivative(const PowerOp<Arg, Power>& f, const Var& v) {
    return f._r * derivative(f._l, v) * pow(f._l, f._r - C<1>()) + derivative(f._r, v) * log(f._l) * f;
  }

  /*! \brief Derivative of a EqualityOp operation.
   */
  template<class Var, class LHS, class RHS>
  auto derivative(const EqualityOp<LHS, RHS>& f, const Var& v) {
    return equality(derivative(f._l, v), derivative(f._r, v));
  }
  /*! \}*/

  /*! \brief Derivative of an array operation.

    For now, we just return the variable.
   */
  template<class Var, class LHS, class RHS>
  auto derivative(const ArrayOp<LHS, RHS>& f, const Var& v) {
    return store(f);
  }
  /*! \}*/

  template<class LHS, class RHS, class Op, class Var, class Arg> 
  auto sub(const BinaryOp<LHS, Op, RHS>& f, const EqualityOp<Var, Arg>& x) {
    return Op::apply(sub(f._l, x), sub(f._r, x));
  }

  namespace detail {
    /*! \brief Wrap the passed string in parenthesis.
     */
    template<class Config>
    std::string paren_wrap(std::string arg) {
      return ((Config::Latex_output) ? "\\left(" : "(") + arg + ((Config::Latex_output) ? "\\right)" : ")");
    }
  }

  /*! \brief Returns the binding powers (precedence) of binary
    operators (specialisation for binary ops).
  */
  template<class LHS, class Op, class RHS>
  std::pair<int, int> BP(const sym::BinaryOp<LHS, Op, RHS>& v) {
    const int L = Op::leftBindingPower;
    const int R = sym::detail::RBP<Op>();
    return std::make_pair(L, R);
  }

  /*! \brief String representation of binary operations.
   */
  template<class Config = DefaultReprConfig, class LHS, class RHS, class Op>
  inline std::string repr(const sym::BinaryOp<LHS, Op, RHS>& op) {
    const auto this_BP = BP(op);
    const auto LHS_BP  = BP(op._l);
    const auto RHS_BP  = BP(op._r);

    std::string LHS_repr = repr<Config>(op._l);
    if (LHS_BP.second < this_BP.first || Config::Force_parenthesis)
      LHS_repr = detail::paren_wrap<Config>(LHS_repr);

    std::string RHS_repr = repr<Config>(op._r);
    if (this_BP.second > RHS_BP.first || Config::Force_parenthesis)
      RHS_repr = detail::paren_wrap<Config>(RHS_repr);

    return (Config::Latex_output ? Op::l_latex_repr() : Op::l_repr())
      +  LHS_repr
      + (Config::Latex_output ? Op::latex_repr() : Op::repr())
      + RHS_repr
      + (Config::Latex_output ? Op::r_latex_repr() : Op::r_repr());
  }
}

namespace std
{
  template<typename LHS, typename Op, typename RHS> struct hash<sym::BinaryOp<LHS, Op, RHS> >
  {
    std::size_t operator()(sym::BinaryOp<LHS, Op, RHS> const& v) const noexcept
    {
      std::size_t seed = Op::type_index;
      stator::hash_combine(seed, std::hash<LHS>{}(v._l));
      stator::hash_combine(seed, std::hash<RHS>{}(v._r));
      return seed;
    }
  };
}
