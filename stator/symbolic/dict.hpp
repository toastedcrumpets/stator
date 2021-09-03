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
    
    typedef typename Store::key_type key_type;
    typedef typename Store::value_type value_type;
    typedef typename Store::reference reference;
    typedef typename Store::const_reference const_reference;
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

    reference& operator[](const key_type& k) { return _store[k]; }
    reference& at(const key_type& k) { return _store.at(k); }
    const const_reference& at(const key_type& k) const { return _store.at(k); }

    typename Store::size_type size() const noexcept { return _store.size(); } 
    bool empty() const noexcept { return _store.empty(); }

    template<typename...Args>
    bool operator==(const DictBase<Args...>& o) const {
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
    
    std::pair<iterator,bool> insert(value_type&& value ) {
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
}