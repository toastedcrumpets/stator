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
  class Dict<Expr, Expr>: public RTBaseHelper<DictRT>, public DictBase<Expr, Expr> {
      typedef DictBase<Expr, Expr> Base;

      Dict() {}

      public:

      typedef std::shared_ptr<Dict> DictPtr;

      static auto create() {
        return DictPtr(new Dict());
      }

      using Base::operator==;
      using Base::operator[];
  };

  typedef Dict<Expr, Expr> DictRT;


  namespace detail {
    template<> struct Type_index<DictRT> { static const int value = 16; };
  }
}
