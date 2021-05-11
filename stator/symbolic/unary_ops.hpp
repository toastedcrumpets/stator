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
  /*! \brief Symbolic representation of a unary operator (i.e., sin(x)).
   */
  template<class Arg, typename Op>
  struct UnaryOp: SymbolicOperator {
  protected:
     UnaryOp(const Arg& a): _arg(a) {}    
  public:
    static auto create(const Arg& a) { return UnaryOp(a); }
    Arg _arg;
  };
  
  using std::sin;
  using std::cos;
  using std::exp;
  using std::log;
  using std::abs;
  
  namespace detail {
    template<typename Arg, typename Op>
    struct Type_index<UnaryOp<Arg, Op>> { static const int value = Op::type_index;  };
    
    struct Sine {
      static constexpr int BP = std::numeric_limits<int>::max();
      template<class Arg> static auto apply(const Arg& a) {return sin(a); }
      static inline std::string l_repr()       { return "sin "; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "\\sin "; }
      static inline std::string r_latex_repr() { return ""; }
      static constexpr int type_index = 2;
    };

    struct Cosine {
      static constexpr int BP = std::numeric_limits<int>::max();
      template<class Arg> static auto apply(const Arg& a) { return cos(a); }
      static inline std::string l_repr()       { return "cos "; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "\\cos "; }
      static inline std::string r_latex_repr() { return ""; }
      static constexpr int type_index = 3;
    };

    struct Exp {
      static constexpr int BP = std::numeric_limits<int>::max();
      template<class Arg> static auto apply(const Arg& a) { return exp(a); }
      static inline std::string l_repr()       { return "exp "; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "\\mathrm{e}^{"; }
      static inline std::string r_latex_repr() { return "}"; }
      static constexpr int type_index = 5;
    };

    struct Log {
      static constexpr int BP = std::numeric_limits<int>::max();
      template<class Arg> static auto apply(const Arg& a) { return log(a); }
      static inline std::string l_repr()       { return "ln "; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "\\ln "; }
      static inline std::string r_latex_repr() { return "}"; }
      static constexpr int type_index = 4;
    };

    struct Absolute {
      static constexpr int BP = 0; //Binding power is zero, as it wraps its arguments, no need to fight for them.
      template<class Arg> static auto apply(const Arg& a) { return abs(a); }
      static inline std::string l_repr()       { return "|"; }
      static inline std::string r_repr()       { return "|"; }
      static inline std::string l_latex_repr() { return "\\left|"; }
      static inline std::string r_latex_repr() { return "\\right|"; }
      static constexpr int type_index = 6;
    };

    struct Arbsign {
      static constexpr int BP = 0; //Binding power is zero, as it wraps its arguments, no need to fight for them.
      template<class Arg> static auto apply(const Arg& a) { return store(UnaryOp<decltype(store(a)), Arbsign>::create(a)); }
      static inline std::string l_repr()       { return "±|"; }
      static inline std::string r_repr()       { return "|"; }
      static inline std::string l_latex_repr() { return "\\pm\\left|"; }
      static inline std::string r_latex_repr() { return "\\right|"; }
      static constexpr int type_index = 7;
    };

    struct Negate {
      //The binding power of negation is equal to binary addition's
      //RBP as its equivalent. for example, exponents and
      //multiplication should be more powerful.
      static constexpr int BP = 21;
      template<class Arg> static auto apply(const Arg& a) { return -a; }
      static inline std::string l_repr()       { return "-"; }
      static inline std::string r_repr()       { return ""; }
      static inline std::string l_latex_repr() { return "-"; }
      static inline std::string r_latex_repr() { return ""; }
      static constexpr int type_index = 17;
    };
  }

  /*! \brief Symbolic unary positive operator. */
  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto operator+(const Arg& l) { return store(l); }

  /*! \brief Symbolic unary negation operator. */
  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto operator-(const Arg& arg) {
    return store(UnaryOp<decltype(store(arg)), detail::Negate>::create(arg));
  }
  
  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto sin(const Arg& arg) {
    return store(UnaryOp<decltype(store(arg)), detail::Sine>::create(arg));
  }

  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto cos(const Arg& arg) {
    return store(UnaryOp<decltype(store(arg)), detail::Cosine>::create(arg));
  }

  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto exp(const Arg& arg) {
    return store(UnaryOp<decltype(store(arg)), detail::Exp>::create(arg));
  }

  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto log(const Arg& arg) {
    return store(UnaryOp<decltype(store(arg)), detail::Log>::create(arg));
  }
  
  template<class Arg,
	   typename = typename std::enable_if<IsSymbolic<Arg>::value>::type>
  auto abs(const Arg& arg) {
    return store(UnaryOp<decltype(store(arg)), detail::Absolute>::create(arg));
  }
  
  template<class Arg>
  auto arbsign(const Arg& arg) {
    return store(UnaryOp<decltype(store(arg)), detail::Arbsign>::create(arg));
  }
  
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Sine>& f, const Var& x)
  { return store(derivative(f._arg, x) * sym::cos(f._arg)); }
  
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Cosine>& f, const Var& x)
  { return store(-derivative(f._arg, x) * sym::sin(f._arg)); }
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Exp>& f, const Var& x)
  { return store(derivative(f._arg, x) * f); }
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Log>& f, const Var& x)
  { return store(derivative(f._arg, x) / f._arg); }
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Absolute>& f, const Var& x)
  { return store(derivative(f._arg, x) * sym::abs(f._arg) / f._arg); }
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Arbsign>& f, const Var& x)
  { return store(derivative(f._arg, x) * sym::arbsign(Unity())); }
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Negate>& f, const Var& x)
  { return store(-derivative(f._arg, x)); }

  template<class Var, class Arg1, class Arg2, class Op>
  auto sub(const UnaryOp<Arg1, Op>& f, const EqualityOp<Var, Arg2>& x)
  { return store(Op::apply(sub(f._arg, x)));  }


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

namespace std
{
  template<typename Arg, typename Op> struct hash<sym::UnaryOp<Arg, Op> >
  {
    std::size_t operator()(sym::UnaryOp<Arg, Op> const& v) const noexcept
    {
      std::size_t seed = Op::type_index;
      stator::hash_combine(seed, std::hash<Arg>{}(v._arg));
      return seed;
    }
  };
}
