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
  
  template<size_t D>
  class RowMajor {
  public:
    typedef typename detail::LinearStore<size_t, D>::type Coords;
    Coords _dimensions;
    
    RowMajor(): _dimensions() {} //This zero initialises the dimensions
    RowMajor(const Coords& d): _dimensions(d) {} //This zero initialises the dimensions
    
    size_t operator[](const Coords& d) const {
      size_t address = 0;

      for (size_t i(0); i < _dimensions.size(); ++i) {
	address *= _dimensions[i];
	address += d[i];
      }
      
      return address;
    }
    
    void resize(const Coords& d) {
      _dimensions = d;
    }
    
    size_t size() const {
      size_t s = _dimensions.size() > 0;
      for (size_t i(0); i < _dimensions.size(); ++i)
	s *= _dimensions[i];
      return s;
    }

    size_t store_size() const {
      return size();
    }
  };

  namespace detail {
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
	_coords(a.getAddressing()._dimensions.size()),
	_idx(0)
      {}

      template<class T>
      auto& operator=(const T& rhs) {
	if (_idx != _coords.size())
	  stator_throw() << "Address was not fully specified, dimensionality is " << _coords.size();
	
	return _array[_coords] = rhs;
      }

      template<class T>
      bool operator==(const T& rhs) const {
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
  }
  
  template<class T, size_t D, class Addressing = RowMajor<D>>
  class Array {
    Addressing _addressing;
    
    typedef std::vector<T> Store;
    Store _store;
    
  public:
    typedef typename Addressing::Coords Coords;
    typedef typename Store::value_type value_type;
    typedef typename Store::reference reference;
    typedef typename Store::const_reference const_reference;
    
    Store& getStore() { return _store; }
    const Store& getStore() const { return _store; }
    Addressing& getAddressing() { return _addressing; }
    const Addressing& getAddressing() const { return _addressing; }

    
    Array(const Addressing& a = Addressing()): _addressing(a) {
      _store.resize(a.store_size());
    }

    T& operator[](const Coords& c) {
      return _store[_addressing[c]];
    }

    const T& operator[](const Coords& c) const {
      return _store[_addressing[c]];
    }

    //These accessors need to use the -> decltype form, as they return
    //ArrayAccessors by value, unless D=1, then we return array
    //elements by (possibly const) reference. 
    auto operator[](const size_t& c) -> decltype(detail::ArrayAccessor<D, Array>(*this)[c]) {
      return detail::ArrayAccessor<D, Array>(*this)[c];
    }

    auto operator[](const size_t& c) const -> decltype(detail::ArrayAccessor<D, const Array>(*this)[c]) {
      return detail::ArrayAccessor<D, const Array>(*this)[c];
    }
    
    size_t size() const { return _addressing.size(); }
    bool empty() const { return _addressing.size() == 0; }
    
    void resize(const Coords& d) {
      _addressing.resize(d);
      _store.resize(_addressing.store_size());
    }
  };
}
