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

#include <stator/symbolic/symbolic.hpp>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

namespace sym {
  namespace detail {
    template<class T, size_t D>
    struct LinearStore {
      typedef std::array<T, D> type;
    };

    template<class T>
    struct LinearStore<T, -1u> {
      typedef std::vector<T> type;
    };
  }

  template<class T, size_t StoreSize>
  class AddressingBase {
  public:
    typedef typename detail::LinearStore<T, StoreSize>::type Store;
    typedef typename Store::value_type value_type;
    typedef typename Store::reference reference;
    typedef typename Store::const_reference const_reference;
    
    Store& getStore() { return _store; }
    const Store& getStore() const { return _store; }

    size_t store_size() const {
      return _store.size();
    }
    
  protected:
    Store _store;
  };

  template<class T, class AddressingType>
  class Addressing;

  template<size_t StoreSize>
  struct LinearAddressing {};
    
  template<class T, size_t StoreSize>
  class Addressing<T, LinearAddressing<StoreSize>> : public AddressingBase<T, StoreSize> {
  public:
    typedef AddressingBase<T, StoreSize> Base;
    typedef size_t Coords;
    size_t _dimension;
    
    Addressing(): _dimension(0) {}

    Addressing(const Coords d) { resize(d); }

    Addressing(std::initializer_list<T> vals)
    {
      Base::_store = Base::Store(vals);
      _dimension = Base::_store.size();
    }

    Coords getDimensions() const { return _dimension; }
    
    const auto& operator[](const Coords& d) const {
      return Base::_store[d];
    }

    auto& operator[](const Coords& d) {
      return Base::_store[d];
    }

    auto begin() const { return Base::_store.begin(); }
    auto begin(){ return Base::_store.begin(); }
    auto end() const { return Base::_store.end(); }
    auto end() { return Base::_store.end(); }
    
    void resize(const Coords& d) {
      if constexpr (StoreSize == -1u) {
	  _dimension = d;
	  Base::_store.resize(d);
      } else {
	if (d > Base::_store.size())
	  stator_throw() << "Cannot resize fixed-length array!";
	_dimension = d;
      }
    }
    
    size_t size() const { return _dimension; }
    bool empty() const { return _dimension == 0; }
  };

  template<size_t StoreSize, size_t D>
  struct RowMajorAddressing {};
  
  
  template<class T, size_t StoreSize, size_t D>
  class Addressing<T, RowMajorAddressing<StoreSize, D> > : public AddressingBase<T, StoreSize> {
  public:
    typedef AddressingBase<T, StoreSize> Base;
    typedef typename detail::LinearStore<size_t, D>::type Coords;
    Coords _dimensions;

    Addressing(): _dimensions() {} //This zero initialises the dimensions
    Addressing(const Coords& d) { resize(d); }

    Addressing(const Coords& d, std::initializer_list<T> vals)
    {
      resize(d);
      Base::_store = typename Base::Store(vals);
    }
    
    auto begin() const { return Base::_store.begin(); }
    auto begin(){ return Base::_store.begin(); }
    auto end() const { return Base::_store.end(); }
    auto end() { return Base::_store.end(); }

    Coords getDimensions() const { return _dimensions; }

    size_t coords_to_index(const Coords& d) const {
      size_t address = 0;
      for (size_t i(0); i < _dimensions.size(); ++i) {
	address *= _dimensions[i];
	address += d[i];
      }
      return address;
    }
    
    const auto& operator[](const Coords& d) const {
      return Base::_store[coords_to_index(d)];
    }
    auto&       operator[](const Coords& d) {
      return Base::_store[coords_to_index(d)];
    }
    
    void resize(const Coords& d) {
      _dimensions = d;
      if constexpr (StoreSize == -1u) {
	  Base::_store.resize(store_size());
	} else
	if (store_size() > StoreSize)
	  stator_throw() << "StoreSize is too small for this dimensionality";
    }

    size_t size() const {
      return store_size();
    }
    
    bool empty() const { return store_size() == 0; }
    
    size_t store_size() const {
      size_t s = _dimensions.size() > 0;
      for (size_t i(0); i < _dimensions.size(); ++i)
	s *= _dimensions[i];
      return s;
    }

    template<size_t Dim, class Array>
    struct ArrayAccessor {
      Array& _array;
      typename Array::Coords _coords;
      
      ArrayAccessor(Array& a): _array(a) {}

      ArrayAccessor(ArrayAccessor<Dim+1, Array>& a): _array(a._array), _coords(a._coords) {}
      
      auto operator[](size_t idx) {
	_coords[_coords.size() - Dim] = idx;
	return ArrayAccessor<Dim-1, Array>(*this);
      }      
    };

    template<class Array>
    struct ArrayAccessor<1, Array> {
      Array& _array;
      typename Array::Coords _coords;
      
      ArrayAccessor(Array& a): _array(a) {}
      ArrayAccessor(const ArrayAccessor<2, Array>& a): _array(a._array), _coords(a._coords) {}

      auto& operator[](size_t idx) {
	_coords[_coords.size() - 1] = idx;
	return _array[_coords];
      }
    };

    template<class Array>
    struct ArrayAccessor<-1u, Array> {
      Array& _array;
      typename Array::Coords _coords;
      size_t _idx;
      
      ArrayAccessor(Array& a):
	_array(a),
	_coords(a._dimensions.size()),
	_idx(0)
      {}

      template<class RHS>
      auto& operator=(const RHS& rhs) {
	if (_idx != _coords.size())
	  stator_throw() << "Address was not fully specified, dimensionality is " << _coords.size();
	
	return _array[_coords] = rhs;
      }

      template<class RHS>
      bool operator==(const RHS& rhs) const {
	if (_idx != _coords.size())
	  stator_throw() << "Address was not fully specified, dimensionality is " << _coords.size();
	
	return _array[_coords] == rhs;
      }
      
      operator typename std::conditional<std::is_const<Array>::value,
					 typename Array::const_reference,
					 typename Array::reference>::type () {
	if (_idx != _coords.size())
	  stator_throw() << "Address was not fully specified, dimensionality is " << _coords.size();
	
	return _array[_coords];
      }
      
      ArrayAccessor operator[](size_t v) {
	if (_idx == _coords.size())
	  stator_throw() << "Too many array operations, dimensionality is " << _coords.size();
	
	_coords[_idx++] = v;
	return *this;
      }
    };

    //These accessors need to use the -> decltype form, as they return
    //ArrayAccessors by value, unless D=1, then we return array
    //elements by (possibly const) reference. 
    auto operator[](const size_t& c) -> decltype(ArrayAccessor<D, Addressing>(*this)[c]) {
      return ArrayAccessor<D, Addressing>(*this)[c];
    }

    auto operator[](const size_t& c) const -> decltype(ArrayAccessor<D, const Addressing>(*this)[c]) {
      return ArrayAccessor<D, const Addressing>(*this)[c];
    }
  };

  template<class T, class Addressing_t = RowMajorAddressing<-1u, -1u> >
  class Array : public Addressing<T, Addressing_t> {
  public:
    typedef Addressing<T, Addressing_t> Base;
    using Base::Base;

    template<class ...Args>
    auto Create(Args&&...args) {
      return Array(args...);
    }
  };

  template<class Config = DefaultReprConfig, typename ...Args>
  inline std::string repr(const Array<Args...>& f)
  {
    std::string out = std::string((Config::Latex_output) ? "\\left[" : "[");
    const std::string end = std::string((Config::Latex_output) ? "\\right]" : "]");
    if (f.empty())
      return out+end;
    
    for (const auto& term : f)
      out += repr<Config>(term) + ", ";
    
    return out.substr(0, out.size() - 2) + end;
  }

  namespace detail {
    template<class T>
    auto& unwrap(std::shared_ptr<T> ptr) {
      return *ptr;
    }

    template<class T>
    auto& unwrap(T& ptr) {
      return ptr;
    }
  }
    
  template<typename Var, typename T, typename ...Args>
  auto derivative(const Array<T, Args...>& in, const Var& x) {
    auto out_ptr = Array<decltype(store(derivative(in[0], x))), Args...>::create();
    auto& out = detail::unwrap(out_ptr);
    out.resize(in.getDimensions());
    for (size_t idx(0); idx < in.size(); ++idx)
      out[idx] = derivative(in[idx], x);
    return out;
  }

}
