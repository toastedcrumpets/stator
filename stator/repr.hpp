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

#include <stator/string.hpp>
#include <stator/orphan/template_config.hpp>
#include <limits>
#include <cstdio>

namespace stator {  
  namespace detail {
    struct Latex_output_ID;
    struct Debug_output_ID;
    struct Force_parens_ID;
    struct Rounding_digits_ID {};
  };

  struct Latex_output : stator::orphan::basic_conf_t<detail::Latex_output_ID> {};
  struct Debug_output : stator::orphan::basic_conf_t<detail::Debug_output_ID> {};
  struct Force_parenthesis : stator::orphan::basic_conf_t<detail::Force_parens_ID> {};

  template<int digits>
  struct Rounding_digits : stator::orphan::value_conf_t<detail::Rounding_digits_ID, int, digits> {};
  
  template <typename ...Args>
  struct ReprConfig {
    static constexpr const auto Force_parenthesis = stator::orphan::is_present<stator::Force_parenthesis, Args...>::value;
    static constexpr const auto Latex_output = stator::orphan::is_present<stator::Latex_output, Args...>::value;
    static constexpr const auto Rounding_digits = stator::orphan::get_value<stator::Rounding_digits<0>, Args...>::value;
    static constexpr const auto Debug_output = stator::orphan::is_present<stator::Debug_output, Args...>::value;
  };

  using DefaultReprConfig = ReprConfig<>;
  
  template<class Config = DefaultReprConfig, typename T>
  typename std::enable_if<std::is_integral<T>::value, std::string>::type
  repr(T a) { return std::to_string(a); }
  
  template<class Config = DefaultReprConfig, class Float>
  inline
  typename std::enable_if<std::is_floating_point<Float>::value, std::string>::type
  repr(Float a) {
    std::string basic_output = stator::string_format("%.*g", std::numeric_limits<Float>::max_digits10 - Config::Rounding_digits, a);
    if (Config::Latex_output) {
      //Strip the unneeded exponent leading plus sign if present
      auto fin = search_replace(basic_output, "e+", "\\times10^{");
      //If the number is in exponential notation, the replacement
      //should have succeeded, so close the brackets around the
      //exponent
      if (fin.second) return fin.first + "}";

      //Repeat, but for the case where the exponent is negative
      fin = search_replace(basic_output, "e", "\\times10^{");
      if (fin.second) return fin.first + "}";

    }
    return basic_output;
  }
}
