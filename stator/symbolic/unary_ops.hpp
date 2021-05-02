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
  /////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////        Standard functions         /////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////

  /*! \brief Symbolic representation of a unary operator (i.e., sin(x)).
   */
  template<class Arg, typename Op>
  struct UnaryOp: SymbolicOperator {
    Arg _arg;
    UnaryOp(Arg a): _arg(a) {}
  };
  
  using std::sin;
  using std::cos;
  using std::exp;
  using std::log;
  using std::abs;
  
  namespace detail {
    struct Sine {
      static constexpr int BP = std::numeric_limits<int>::max();
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(sin(a));
      static inline std::string l_repr()       { return "sin "; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "\\sin "; }
      static inline std::string r_latex_repr() { return ""; }
    };

    struct Cosine {
      static constexpr int BP = std::numeric_limits<int>::max();
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(cos(a));
      static inline std::string l_repr()       { return "cos "; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "\\cos "; }
      static inline std::string r_latex_repr() { return ""; }
    };

    struct Exp {
      static constexpr int BP = std::numeric_limits<int>::max();
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(exp(a));
      static inline std::string l_repr()       { return "exp "; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "\\mathrm{e}^{"; }
      static inline std::string r_latex_repr() { return "}"; }
    };

    struct Log {
      static constexpr int BP = std::numeric_limits<int>::max();
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(log(a));
      static inline std::string l_repr()       { return "ln "; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "\\ln "; }
      static inline std::string r_latex_repr() { return "}"; }
    };

    struct Absolute {
      static constexpr int BP = 0; //Binding power is zero, as it wraps its arguments, no need to fight for them.
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(abs(a));
      static inline std::string l_repr()       { return "|"; }
      static inline std::string r_repr()       { return "|"; }
      static inline std::string l_latex_repr() { return "\\left|"; }
      static inline std::string r_latex_repr() { return "\\right|"; }
    };

    struct Arbsign {
      static constexpr int BP = 0; //Binding power is zero, as it wraps its arguments, no need to fight for them.
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN((UnaryOp<decltype(store(a)), Arbsign>(a)));
      static inline std::string l_repr()       { return "±|"; }
      static inline std::string r_repr()       { return "|"; }
      static inline std::string l_latex_repr() { return "\\pm\\left|"; }
      static inline std::string r_latex_repr() { return "\\right|"; }
    };

    struct Negate {
      //The binding power of negation is equal to binary addition's
      //RBP as its equivalent. for example, exponents and
      //multiplication should be more powerful.
      static constexpr int BP = 21;
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(-a);
      static inline std::string l_repr()       { return "-"; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "-"; }
      static inline std::string r_latex_repr() { return ""; }
    };
  }

  /*! \brief Symbolic unary positive operator. */
  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  Arg operator+(const Arg& l) { return l; }

  /*! \brief Symbolic unary negation operator. */
  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto operator-(const Arg& l)  -> STATOR_AUTORETURN((UnaryOp<decltype(store(l)), detail::Negate>(l)));
  
  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto sin(const Arg& arg) -> STATOR_AUTORETURN((UnaryOp<decltype(store(arg)), detail::Sine>(arg)));

  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto cos(const Arg& arg) -> STATOR_AUTORETURN((UnaryOp<decltype(store(arg)), detail::Cosine>(arg)));

  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto exp(const Arg& arg) -> STATOR_AUTORETURN((UnaryOp<decltype(store(arg)), detail::Exp>(arg)));

  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto log(const Arg& arg) -> STATOR_AUTORETURN((UnaryOp<decltype(store(arg)), detail::Log>(arg)));
  
  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto abs(const Arg& arg) -> STATOR_AUTORETURN((UnaryOp<decltype(store(arg)), detail::Absolute>(arg)));
  
  template<class Arg>
  auto arbsign(const Arg& arg) -> STATOR_AUTORETURN((UnaryOp<decltype(store(arg)), detail::Arbsign>(arg)));
  
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Sine>& f, Var x)
    -> STATOR_AUTORETURN(derivative(f._arg, x) * sym::cos(f._arg));
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Cosine>& f, Var x)
    -> STATOR_AUTORETURN(-derivative(f._arg, x) * sym::sin(f._arg));
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Exp>& f, Var x)
    -> STATOR_AUTORETURN(derivative(f._arg, x) * f);
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Log>& f, Var x)
    -> STATOR_AUTORETURN(derivative(f._arg, x) / f._arg);
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Absolute>& f, Var x)
    -> STATOR_AUTORETURN(derivative(f._arg, x) * sym::abs(f._arg) / f._arg);
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Arbsign>& f, Var x)
    -> STATOR_AUTORETURN(derivative(f._arg, x) * sym::arbsign(Unity()));
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Negate>& f, Var x)
    -> STATOR_AUTORETURN(-derivative(f._arg, x));

  template<class Var, class Arg1, class Arg2, class Op>
  auto sub(const UnaryOp<Arg1, Op>& f, const EqualityOp<Var, Arg2>& x)
    -> STATOR_AUTORETURN(Op::apply(sub(f._arg, x)));  


  /*! \brief A function allowing you to see the binding power of any
    unary operation.
  */
  template<class Op, class Arg>
  std::pair<int, int> BP(const sym::UnaryOp<Arg, Op>& v)
  { return std::make_pair(0, Op::BP); }
  
  template<class Config = DefaultReprConfig, class Arg, class Op>
  inline std::string repr(const sym::UnaryOp<Arg, Op>& f)
  {
    std::string arg_repr = repr<Config>(f._arg);

    const auto this_BP = BP(f);
    const auto arg_BP = BP(f._arg);
    
    if ((arg_BP.first < this_BP.second) || Config::Force_parenthesis)
      arg_repr = detail::paren_wrap<Config>(arg_repr);
    
    return std::string((Config::Latex_output) ? Op::l_latex_repr() : Op::l_repr())
      + arg_repr
      + std::string((Config::Latex_output) ? Op::r_latex_repr() : Op::r_repr())
      ;
  }
}
