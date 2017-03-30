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
  /////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////        Standard functions         /////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////

  template<class Arg, typename Op>
  struct UnaryOp: SymbolicOperator {
    Arg _arg;
    UnaryOp(Arg a): _arg(a) {}
  };
  
  float sin(float a) { return std::sin(a); }
  double sin(double a) { return std::sin(a); }
  long double sin(long double a) { return std::sin(a); }
  template<class T> std::complex<T> sin(std::complex<T> a) { return std::sin(a); }
  
  float cos(float a) { return std::cos(a); }
  double cos(double a) { return std::cos(a); }
  long double cos(long double a) { return std::cos(a); }
  template<class T> std::complex<T> cos(std::complex<T> a) { return std::cos(a); }

  float exp(float a) { return std::exp(a); }
  double exp(double a) { return std::exp(a); }
  long double exp(long double a) { return std::exp(a); }
  template<class T> std::complex<T> exp(std::complex<T> a) { return std::exp(a); }

  float log(float a) { return std::log(a); }
  double log(double a) { return std::log(a); }
  long double log(long double a) { return std::log(a); }
  template<class T> std::complex<T> log(std::complex<T> a) { return std::log(a); }
  
  int abs(int a) { return std::abs(a); }
  long abs(long a) { return std::abs(a); }
  long long abs(long long a) { return std::abs(a); }
  float abs(float a) { return std::abs(a); }
  double abs(double a) { return std::abs(a); }
  long double abs(long double a) { return std::abs(a); }
  template<class T> T abs(std::complex<T> a) { return std::abs(a); }

  namespace detail {
    struct Sine {
      static constexpr const char* _str_left = "sin(";
      static constexpr const char* _str_right = ")";
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(sin(a));
    };

    struct Cosine {
      static constexpr const char* _str_left = "cos(";
      static constexpr const char* _str_right = ")";
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(cos(a));
    };

    struct Exp {
      static constexpr const char* _str_left = "exp(";
      static constexpr const char* _str_right = ")";
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(exp(a));
    };

    struct Log {
      static constexpr const char* _str_left = "ln(";
      static constexpr const char* _str_right = ")";
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(log(a));
    };

    struct Absolute {
      static constexpr const char* _str_left = "|";
      static constexpr const char* _str_right = "|";
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN(abs(a));
    };

    struct Arbsign {
      static constexpr const char* _str_left = "±|";
      static constexpr const char* _str_right = "|";
      template<class Arg> static auto apply(const Arg& a) -> STATOR_AUTORETURN((UnaryOp<decltype(store(a)), Arbsign>(a)));
    };
  }

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

  template<class Var, class Arg1, class Arg2, class Op>
  auto sub(const UnaryOp<Arg1, Op>& f, const Relation<Var, Arg2>& x)
    -> STATOR_AUTORETURN(Op::apply(sub(f._arg, x)));
  
  template<class Arg, class Op>
  inline std::ostream& operator<<(std::ostream& os, const UnaryOp<Arg, Op>& f)
  { return os << Op::_str_left << f._arg << Op::_str_right; }  
}
