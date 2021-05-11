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
  class List: public RTBaseHelper<List>  {
    typedef std::vector<Expr> Store;

    List() {}
    List(const List& l) = default;
    
    public:
    static auto create() {
      return std::shared_ptr<List>(new List());
    }
    
    typedef typename Store::iterator iterator;
    typedef typename Store::const_iterator const_iterator;

    iterator begin() {return _store.begin();}
    const_iterator begin() const {return _store.begin();}
    const_iterator cbegin() const {return _store.cbegin();}
    iterator end() {return _store.end();}
    const_iterator end() const {return _store.end();}
    const_iterator cend() const {return _store.cend();}

    typedef Store::reference reference;
    typedef Store::const_reference const_reference;
    reference& operator[](Store::size_type pos) { return _store[pos]; }
    const_reference operator[](Store::size_type pos) const { return _store[pos]; }

    void reserve(Store::size_type cap) { _store.reserve(cap); }
    void resize(Store::size_type cap) { _store.resize(cap); }
    void push_back(const Expr& value) { _store.push_back(value); }
    Store::size_type size() const noexcept { return _store.size(); } 
    bool empty() const noexcept { return _store.empty(); }

    bool operator==(const List& o) const {
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
    template<> struct Type_index<List> { static const int value = 15; };
  }
  
  template<typename Var>
  Expr derivative(const List& in, const Var& x) {
    auto out_ptr = List::create();
    auto& out = *out_ptr;
    out.resize(in.size());
    for (size_t idx(0); idx < in.size(); ++idx)
      out[idx] = derivative(in[idx], x);
    return out;
  }
  
  Expr operator+(const List& l, const List& r) {
    if (l.size() != r.size())
      stator_throw() << "Mismatched list size for: \n" << l << "\n and\n" << r;
    
    auto out = List::create();
    out->resize(l.size());
    
    for (size_t idx(0); idx < l.size(); ++idx)
      (*out)[idx] = l[idx] + r[idx];
    
    return out;
  }
}

namespace std
{
  template<> struct hash<sym::List>
  {
    std::size_t operator()(sym::List const& v) const noexcept
    {
      std::size_t seed = sym::detail::Type_index<sym::List>::value;
      for (const auto& item : v) {
	std::hash<typename std::decay<decltype(item)>::type> h;
	stator::hash_combine(seed, h(item));
      }
      return seed;
    }
  };
}
