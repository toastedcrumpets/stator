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
  class Array<Expr, LinearAddressing<-1u>> : public RTBaseHelper<ArrayRT>, public Addressing<Expr, LinearAddressing<-1u>>, public detail::ArrayBase  {
    private:
    typedef Addressing<Expr, LinearAddressing<-1u>> Base;

    Array(): Base(0) {}

    Array(const Base::Coords d):Base(d) {}

    Array(const std::initializer_list<Expr>& vals): Base(vals) {}

    typedef std::shared_ptr<Array> ArrayPtr;
  public:
    typedef Expr Value;
    
    static auto create(Base::Coords d = 0) {
      return ArrayPtr(new Array(d));
    }

    static auto create(const std::initializer_list<Expr>& vals) {
      return ArrayPtr(new Array(vals));
    }

    //We need to force the use of the Addressing operator[], not the RTBaseHelper::operator[], same for ==
    using Base::operator[];
    using Base::operator==;

    template<class...Args>
    bool operator==(const Array<Args...>& ad) const {
      return (ad.getDimensions() == getDimensions()) && (ad._store == Base::_store);
    }
  };
  
  namespace detail {
    template<> struct Type_index<ArrayRT> { static const int value = 15; };
  }
}