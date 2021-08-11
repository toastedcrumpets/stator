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
  class Dict;
  typedef std::shared_ptr<Dict> DictPtr;
  
  class Dict: public RTBaseHelper<Dict> {
    typedef std::unordered_map<Expr, Expr> Store;

    Dict() {}
    Dict(const Dict& l) = default;
  public:
    static auto create() {
      return DictPtr(new Dict());
    }
    
    typedef Store::key_type key_type;
    typedef Store::value_type value_type;
    typedef Store::reference reference;
    typedef Store::const_reference const_reference;
    typedef typename Store::iterator iterator;
    typedef typename Store::const_iterator const_iterator;

    iterator begin() {return _store.begin();}
    const_iterator begin() const {return _store.begin();}
    const_iterator cbegin() const {return _store.cbegin();}
    iterator end() {return _store.end();}
    const_iterator end() const {return _store.end();}
    const_iterator cend() const {return _store.cend();}

    iterator find( const key_type& key ) { return _store.find(key); }
    const_iterator find( const key_type& key ) const { return _store.find(key); }

    Expr& operator[](const key_type& k) { return _store[k]; }
    Expr& at(const key_type& k) { return _store.at(k); }
    const Expr& at(const key_type& k) const { return _store.at(k); }

    Store::size_type size() const noexcept { return _store.size(); } 
    bool empty() const noexcept { return _store.empty(); }

    bool operator==(const Dict& o) const {
      //Shortcut comparison before proceeding with item by item
      return (this == &o) || (_store == o._store);
    }

    template<class RHS>
    constexpr bool operator==(const RHS&) const {
      return false;
    }
    
    std::pair<iterator,bool> insert( const value_type& value ) {
      return _store.insert(value);
    }
    
    std::pair<iterator,bool> insert( value_type&& value ) {
      return _store.insert(std::move(value));
    }
    
  private:
    Store _store;
  };

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
}
