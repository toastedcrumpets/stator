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

// C++
#include <array>
#include <ostream>

namespace stator {
  namespace orphan {
    /*! \brief Stack-allocated equivalent of std::vector.
      
      This class impersonates a STL vector, but its storage is
      allocated on the stack. this allows the simplicity of a vector
      without the overhead of dynamic memory allocation. It is usually
      used to return small arrays of values (e.g., a list of roots)
      where a maximum number of elements can be calculated at compile
      time.

      \tparam T The stored data type.

      \tparam Nmax The maximum number of elements to be stored in the container.

      
    */
    template<class T, size_t Nmax>
    class StackVector: public std::array<T, Nmax> {
      typedef std::array<T, Nmax> Base;
    public:
      
      /*! \brief Copy constructor which allows construction from
          smaller StackVector types. */
      template<size_t Nmax2,
               typename = typename std::enable_if<(Nmax2 <= Nmax)>::type>
      StackVector(const StackVector<T, Nmax2>& vec):
        _size(vec.size())
      { std::copy(vec.begin(), vec.end(), Base::begin()); }

      /*! \brief Default constructor. */
      StackVector(): Base(), _size(0) {}
      
      /*! \brief Array initialisation constructor. */
      StackVector(std::initializer_list<T> _list):
	Base(),
	_size(0)
      {
	auto it = _list.begin();
	for (size_t i(0); (i < Nmax) && (it != _list.end()); ++i, ++it)
	  push_back(*it);
      }
            
      /*! \brief The number of elements currently stored. */
      constexpr typename Base::size_type size() const { return _size; }
      
      /*! \brief Test if the container is empty. */
      constexpr bool empty() const { return size() == 0; }

      /*! \brief Returns an iterator pointing to the end of the container. */
      typename Base::iterator end() { return Base::begin() + _size; }

      /*! \brief Returns an iterator pointing to the end of the container. */
      typename Base::const_iterator end() const { return Base::begin() + _size; }

      /*! \brief Returns a const iterator pointing to the end of the container. */
      typename Base::const_iterator cend() const { return Base::begin()+ _size; }

      /*! \brief Returns a reverse iterator pointing to the start of
          the container. */
      typename Base::reverse_iterator rbegin() { return typename Base::reverse_iterator(this->end()); }

      /*! \brief Returns a reverse iterator pointing to the start of
          the container. */
      typename Base::const_reverse_iterator rbegin() const { return typename Base::const_reverse_iterator(this->end()); }

      /*! \brief Returns a const reverse iterator pointing to the start of
          the container. */
      typename Base::const_reverse_iterator crbegin() const { return typename Base::const_reverse_iterator(this->end()); }

      /*! \brief Returns a reverse iterator pointing to the end of
          the container. */
      typename Base::reverse_iterator rend() { return typename Base::reverse_iterator(this->begin()); }

      /*! \brief Returns a reverse iterator pointing to the end of
          the container. */
      typename Base::const_reverse_iterator rend() const { return typename Base::const_reverse_iterator(this->begin()); }

      /*! \brief Returns a const reverse iterator pointing to the end
          of the container. */
      typename Base::const_reverse_iterator crend() const { return typename Base::const_reverse_iterator(this->begin()); }
      
      /*! \brief Returns a reference to the last element stored in the container. */
      typename Base::reference back() { return _size ? *(this->end() - 1) : *this->end(); }

      /*! \brief Returns a reference to the last element stored in the container. */
      typename Base::const_reference back() const { return _size ? *(this->end() - 1) : *this->end(); }

      /*! \brief Add an element to the end of the container. */
      void push_back(const T& val) {
#ifdef STATOR_DEBUG
	if (_size+1 > Nmax)
	  stator_throw() << "Cannot push elements to a filled StackVector " << *this;
#endif
	Base::operator[](_size) = val;
	++_size;
      }

      /*! \brief Removes and returns the last element of the container. */
      T pop_back() {
#ifdef STATOR_DEBUG
	if (empty())
	  stator_throw() << "Cannot pop elements from an empty StackVector " << *this;
#endif
	return Base::operator[](--_size);
      }

      /*! \brief Merge the contents of another StackVector into this one.*/
      template<size_t Nmax2>
      void extend(const StackVector<T,Nmax2>& ovec) {
	for (const T& a: ovec)
	  push_back(a);
      }
      
    private:
      /*! The number of elements stored in the StackVector. */
      size_t _size;
    };

    /*! Output operator for pretty printing StackVector classes. */
    template<class T, size_t Nmax>
    std::ostream& operator<<(std::ostream& os, const StackVector<T,Nmax>&s) {
      os << "StackVector{ ";
      for (const auto& val : s)
	os << val << " ";
      os << "}";
      return os;
    }

    /*! Output operator for pretty printing StackVector classes containing pairs. */
    template<class T1, class T2, size_t Nmax>
    std::ostream& operator<<(std::ostream& os, const StackVector<std::pair<T1,T2>,Nmax>&s) {
      os << "StackVector{ ";
      for (const auto& val : s)
	os << "[" << val.first << ", " << val.second << "] ";
      os << "}";
      return os;
    }

    namespace detail {
      template<std::size_t I = 0, typename... Tp>
      inline typename std::enable_if<I == sizeof...(Tp), void>::type
      tuple_print(const std::tuple<Tp...>& t, std::ostream& os)
      { }

      template<std::size_t I = 0, typename... Tp>
      inline typename std::enable_if<I < sizeof...(Tp), void>::type
      tuple_print(const std::tuple<Tp...>& t, std::ostream& os)
      {
        os << std::get<I>(t) << " ";
        tuple_print<I + 1, Tp...>(t, os);
      }
    }// namespace detail

    /*! Output operator for pretty printing StackVector classes containing tuples. */
    template<size_t Nmax, typename... Tp>
    std::ostream& operator<<(std::ostream& os, const StackVector<std::tuple<Tp...>,Nmax>&s) 
    {
      os << "StackVector{ ";
      for (const auto& val : s) {
	os << "[";
        detail::tuple_print(val, os);
	os << "] ";
      }
      os << "}";
      return os;
    }
  }// namespace orphan
}// namespace stator
