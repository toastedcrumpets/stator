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

namespace sym {

  template<typename Key, typename Value>
  class DictBase {
    typedef std::unordered_map<Key, Value> Store;

  public:
    DictBase() {}
    DictBase(const DictBase& l) = default;
    
    typedef typename Store::iterator iterator;
    typedef typename Store::const_iterator const_iterator;

    iterator begin() {return _store.begin();}
    const_iterator begin() const {return _store.begin();}
    const_iterator cbegin() const {return _store.cbegin();}
    iterator end() {return _store.end();}
    const_iterator end() const {return _store.end();}
    const_iterator cend() const {return _store.cend();}

    iterator find( const Key& key ) { return _store.find(key); }
    const_iterator find( const Key& key ) const { return _store.find(key); }

    Value& operator[](const Key& k) { return _store[k]; }
    Value& at(const Key& k) { return _store.at(k); }
    const Value& at(const Key& k) const { return _store.at(k); }

    typename Store::size_type size() const noexcept { return _store.size(); } 
    bool empty() const noexcept { return _store.empty(); }

    template<typename...Args>
    bool operator==(const DictBase<Args...>& o) const {
      //Shortcut comparison before proceeding with item by item
      return (this == &o) || (_store == o._store);
    }
    
    std::pair<iterator,bool> insert( const typename Store::value_type& value ) {
      return _store.insert(value);
    }
    
    std::pair<iterator,bool> insert(typename Store::value_type&& value ) {
      return _store.insert(std::move(value));
    }
    
  private:
    Store _store;
  };

  template<typename Key, typename Value>
  class Dict: public DictBase<Key, Value> {
      typedef DictBase<Key, Value> Base;
      public:
      using Base::Base;
  };

  struct Expr;
  template<> class Dict<Expr, Expr>;
  typedef Dict<Expr, Expr> DictRT;

  template<typename Key, typename Value>
  auto simplify(const Dict<Key, Value>& in) {
    auto out_ptr = Dict<decltype(store(simplify(in.begin()->first))), decltype(store(simplify(in.begin()->second)))>::create();
    auto& out = sym::detail::unwrap(out_ptr);
    
    for (const auto& p : in)
      out[p.first] = simplify(p.second);
    return out_ptr;
  }

  template<class Config = DefaultReprConfig, typename ...Args>
  inline std::string repr(const Dict<Args...>& f)
  {
    std::string out = std::string((Config::Latex_output) ? "\\left\\{" : "{");
    const std::string end = std::string((Config::Latex_output) ? "\\right\\}" : "}");
    if (f.empty())
      return out+end;
    
    for (const auto& term : f)
      out += repr<Config>(term.first) + ":" +  repr<Config>(term.second) + ", " ;
    
    return out.substr(0, out.size() - 2) + end;
  }
}


namespace std
{
  template<class Key, class Value> struct hash<sym::Dict<Key, Value>>
  {
    std::size_t operator()(sym::Dict<Key, Value> const& v) const noexcept
    {
      std::size_t seed = 16;
      for (const auto& item : v) {
	stator::hash_combine(seed, std::hash<typename std::decay<decltype(item.first)>::type>{}(item.first));
	stator::hash_combine(seed, std::hash<typename std::decay<decltype(item.second)>::type>{}(item.second));
      }
      return seed;
    }
  };
}