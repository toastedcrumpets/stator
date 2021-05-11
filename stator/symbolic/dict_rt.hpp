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
  class Dict: public RTBaseHelper<Dict> {
    typedef std::unordered_map<Expr, Expr> Store;

    Dict() {}
    Dict(const Dict& l) = default;
  public:
    static auto create() {
      return std::shared_ptr<Dict>(new Dict());
    }
    
    typedef Store::key_type key_type;
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
      return _store == o._store;
    }
    
    template<class RHS>
    constexpr bool operator==(const RHS&) const {
      return false;
    }
    
  private:
    Store _store;
  };

  namespace detail {
    template<> struct Type_index<Dict> { static const int value = 16; };
  }

  template<typename Var>
  Dict derivative(const Dict& in, const Var& x) {
    stator_throw() << "Cannot take derivatives of dictionaries";
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
