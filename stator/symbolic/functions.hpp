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
  
  template<class C_arg, class factor, class offset = std::ratio<0> >
  struct is_whole_factor {
    static const bool value = (std::ratio_divide<std::ratio_subtract<std::ratio<C_arg::num, C_arg::den>, std::ratio<offset::num, offset::den> >, std::ratio<factor::num, factor::den> >::den == 1);
  };

  namespace detail {
    struct Sine {
      static constexpr const char* _str_left = "sin(";
      static constexpr const char* _str_right = ")";

	  template<std::intmax_t num, std::intmax_t den>
	  static constexpr typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi>::value, Null>::type apply(const C<num, den>& a, detail::choice<0>) { return{}; }

      template<std::intmax_t num, std::intmax_t den>
      static constexpr typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi, decltype(pi() / C<2>())>::value, Unity>::type apply(const C<num, den>& a, detail::choice<0>) { return{}; }

	  template<class Arg> static auto apply(const Arg& a, detail::choice<1>) -> STATOR_AUTORETURN(std::sin(a));

	  template<class Arg> static auto apply(const Arg& a, detail::last_choice) -> STATOR_AUTORETURN((UnaryOp<decltype(store(a)), Sine>(a)));
    };

    struct Cosine {
      static constexpr const char* _str_left = "cos(";
      static constexpr const char* _str_right = ")";

	  template<std::intmax_t num, std::intmax_t den>
	  static constexpr typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi>::value, Unity>::type apply(const C<num, den>& a, detail::choice<0>) { return{}; }

	  template<std::intmax_t num, std::intmax_t den>
	  static constexpr typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi, decltype(pi() / C<2>())>::value, Null>::type apply(const C<num, den>& a, detail::choice<0>) { return{} };

	  template<class Arg> static auto apply(const Arg& a, detail::choice<1>) -> STATOR_AUTORETURN(std::cos(a));
      template<class Arg> static auto apply(const Arg& a, detail::last_choice) -> STATOR_AUTORETURN((UnaryOp<decltype(store(a)), Cosine>(a)));
    };

    struct Absolute {
      static constexpr const char* _str_left = "|";
      static constexpr const char* _str_right = "|";

      template<std::intmax_t num, std::intmax_t den> static
      constexpr C<(1 - 2 *(num < 0)) * num, den> apply(const C<num, den>& a, detail::choice<0>) { return {}; }

	  template<class Arg> static auto apply(const Arg& a, detail::choice<1>) -> STATOR_AUTORETURN(std::abs(a));

	  template<class Arg> static auto apply(const Arg& a, detail::last_choice) -> STATOR_AUTORETURN((UnaryOp<decltype(store(a)), Absolute>(a)));
    };

    struct Arbsign {
      static constexpr const char* _str_left = "±|";
      static constexpr const char* _str_right = "|";
      template<class Arg> static auto apply(const Arg& a, detail::last_choice) -> STATOR_AUTORETURN((UnaryOp<decltype(store(a)), Arbsign>(a)));
    };
  }
  
  template <class Arg> auto sin(const Arg& a) -> STATOR_AUTORETURN(detail::Sine::apply(a, detail::select_overload{}));
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Sine>& f, Var x)
    -> STATOR_AUTORETURN(derivative(f._arg, x) * detail::Cosine::apply(f._arg, detail::select_overload{}));

  template <class Arg> auto cos(const Arg& a) -> STATOR_AUTORETURN(detail::Cosine::apply(a, detail::select_overload{}));
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Cosine>& f, Var x)
    -> STATOR_AUTORETURN(-derivative(f._arg, x) * detail::Sine::apply(f._arg, detail::select_overload{}));

  template <class Arg> auto abs(const Arg& a) -> STATOR_AUTORETURN(detail::Absolute::apply(a, detail::select_overload{}));
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Absolute>& f, Var x)
    -> STATOR_AUTORETURN(derivative(f._arg, x) * detail::Absolute::apply(f._arg, detail::select_overload{}) / f._arg);

  template <class Arg> auto arbsign(const Arg& a) -> STATOR_AUTORETURN(detail::Arbsign::apply(a, detail::select_overload{}));
  template<class Var, class Arg> auto derivative(const UnaryOp<Arg, detail::Arbsign>& f, Var x)
    -> STATOR_AUTORETURN((derivative(f._arg, x) * UnaryOp<Unity, detail::Arbsign>(Unity())));

  template<class Var, class Arg1, class Arg2, class Op>
  auto substitution(const UnaryOp<Arg1, Op>& f, const VarSub<Var, Arg2>& x)
    -> STATOR_AUTORETURN(Op::apply(substitution(f._arg, x), detail::select_overload{}));
  
  template<class Arg, class Op>
  inline std::ostream& operator<<(std::ostream& os, const UnaryOp<Arg, Op>& f)
  { return os << Op::_str_left << f._arg << Op::_str_right; }
  
  /////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////         Complex functions         /////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////    
}

