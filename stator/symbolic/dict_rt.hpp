/*
  Copyright (C) 2021 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#include <stator/symbolic/array.hpp>
#include <stator/symbolic/runtime.hpp>

namespace sym {
  template<>
  class Dict<Expr, Expr>: public RTBaseHelper<DictRT>, public DictBase<Expr, Expr> {
      typedef DictBase<Expr, Expr> Base;

      Dict() {}

      public:

      typedef std::shared_ptr<Dict> DictPtr;

      static auto create() {
        return DictPtr(new Dict());
      }

      using Base::operator==;
  };

  typedef Dict<Expr, Expr> DictRT;

  namespace detail {
    template<> struct Type_index<DictRT> { static const int value = 16; };
  }

/*
  auto operator+(const Dict& l, const Dict& r)  {
    auto out_ptr = Dict::create();
    auto& out = *out_ptr;
    
    for (const auto& item : l)
      out.insert(item);

    for (const auto& item : r) {
      auto it = out.find(item.first);
      if (it != out.end())
	out[item.first] = out[item.first] + item.second;
      else
	out.insert(item);
    }
    
    return out_ptr;
  }
  
  auto operator-(const Dict& l, const Dict& r)  {
    auto out_ptr = Dict::create();
    auto& out = *out_ptr;
    
    for (const auto& item : l)
      out.insert(item);

    for (const auto& item : r) {
      auto it = out.find(item.first);
      if (it != out.end())
	out[item.first] = out[item.first] - item.second;
      else
	out[item.first] = -item.second;
    }
    
    return out_ptr;
  }

  auto operator*(const Dict& l, const Dict& r)  {
    auto out_ptr = Dict::create();
    auto& out = *out_ptr;
    
    for (const auto& item : l) {
      auto it = r.find(item.first);
      if (it != r.end())
	out[item.first] = item.second * it->second;
    }
    
    return out_ptr;
  }
    
  namespace detail {
    template<> struct Type_index<Dict> { static const int value = 16; };
  }

  template<typename Var>
  Dict derivative(const Dict& in, const Var& x) {
    stator_throw() << "Cannot take derivatives of dictionaries";
  }

  Expr simplify(const Dict& in) {
    auto out_ptr =  Dict::create();
    auto& out = *out_ptr;
    
    for (const auto& p : in) 
      out[p.first] = simplify(p.second);
    
    return out;
  }

  std::pair<int, int> BP(const Dict& v)
  { return std::make_pair(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()); }
  
  template<class Config = DefaultReprConfig>
  inline std::string repr(const sym::Dict& f)
  {
    std::string out = std::string((Config::Latex_output) ? "\\left\\{" : "{");
    const std::string end = std::string((Config::Latex_output) ? "\\right\\}" : "}");
    if (f.empty())
      return out+end;

    std::vector<std::pair<std::string, const Dict::key_type*> > keys;
    for (const auto& term : f)
      keys.emplace_back(repr<Config>(term.first), &term.first);

    sort(keys.begin(), keys.end(), [](const auto& l, const auto& r){ return l.first < r.first; });

    for (const auto& k : keys)
      out += k.first + ":" + repr<Config>(f.at(*k.second)) + ", ";
    
    return out.substr(0, out.size() - 2) + end;
  }
}

namespace std
{
  template<> struct hash<sym::Dict>
  {
    std::size_t operator()(sym::Dict const& v) const noexcept
    {
      std::size_t seed = sym::detail::Type_index<sym::Dict>::value;
      for (const auto& item : v) {
	stator::hash_combine(seed, std::hash<typename std::decay<decltype(item.first)>::type>{}(item.first));
	stator::hash_combine(seed, std::hash<typename std::decay<decltype(item.second)>::type>{}(item.second));
      }
      return seed;
    }
  };
  */
}
