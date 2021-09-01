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
  class Array<Expr, LinearAddressing<-1u>> : public RTBaseHelper<ArrayRT>, public Addressing<Expr, LinearAddressing<-1u>>  {
    private:
    typedef Addressing<Expr, LinearAddressing<-1u>> Base;
    
    Array(): Base(0) {}

    Array(const Base::Coords d):Base(d) {}

    Array(const std::initializer_list<Expr>& vals): Base(vals) {}

    typedef std::shared_ptr<Array> ArrayPtr;
  public:
    static auto create(Base::Coords d = 0) {
      return ArrayPtr(new Array(d));
    }

    static auto create(const std::initializer_list<Expr>& vals) {
      return ArrayPtr(new Array(vals));
    }

    //We need to force the use of the Addressing operator[], not the RTBaseHelper::operator[]
    using Base::operator[];
  };
  
  namespace detail {
    template<> struct Type_index<ArrayRT> { static const int value = 15; };
  }
  
//
//  auto operator-(const List& l, const List& r) {
//    if (l.size() != r.size())
//      stator_throw() << "Mismatched list size for: \n" << l << "\n and\n" << r;
//    
//    auto out = List::create();
//    out->resize(l.size());
//    
//    for (size_t idx(0); idx < l.size(); ++idx)
//      (*out)[idx] = l[idx] - r[idx];
//    
//    return out;
//  }
//
//  auto operator*(const List& l, const List& r) {
//    if (l.size() != r.size())
//      stator_throw() << "Mismatched list size for: \n" << l << "\n and\n" << r;
//    
//    auto out = List::create();
//    out->resize(l.size());
//    
//    for (size_t idx(0); idx < l.size(); ++idx)
//      (*out)[idx] = l[idx] * r[idx];
//    
//    return out;
//  }
//
//  auto operator/(const List& l, const List& r) {
//    if (l.size() != r.size())
//      stator_throw() << "Mismatched list size for: \n" << l << "\n and\n" << r;
//    
//    auto out = List::create();
//    out->resize(l.size());
//    
//    for (size_t idx(0); idx < l.size(); ++idx)
//      (*out)[idx] = l[idx] / r[idx];
//    
//    return out;
//  }
//  
//  Expr simplify(const List& in) {
//    auto out_ptr =  List::create();
//    auto& out = *out_ptr;
//    
//    out.resize(in.size());
//    for (size_t idx(0); idx < in.size(); ++idx) {
//      auto s = simplify(in[idx]);
//      out[idx] = s;
//    }
//    return out_ptr;
//  }
//
//  std::pair<int, int> BP(const List& v)
//  { return std::make_pair(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()); }
//
//  
//  template<class Config = DefaultReprConfig>
//  inline std::string repr(const List& f)
//  {
//    std::string out = std::string((Config::Latex_output) ? "\\left[" : "[");
//    const std::string end = std::string((Config::Latex_output) ? "\\right]" : "]");
//    if (f.empty())
//      return out+end;
//    
//    for (const auto& term : f)
//      out += repr<Config>(term) + ", ";
//    
//    return out.substr(0, out.size() - 2) + end;
//  }
}

namespace std
{
  template<> struct hash<sym::ArrayRT>
  {
    std::size_t operator()(sym::ArrayRT const& v) const noexcept
    {
      std::size_t seed = sym::detail::Type_index<sym::ArrayRT>::value;
      for (const auto& item : v) {
	std::hash<typename std::decay<decltype(item)>::type> h;
	stator::hash_combine(seed, h(item));
      }
      return seed;
    }
  };
}
