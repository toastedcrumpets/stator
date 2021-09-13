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
  template<typename T>
  class ConstantRT : public RTBaseHelper<ConstantRT<T> > {
    ConstantRT(const T& v): _val(v) {}
    
  public:
    static auto create(const T& val) {
      return std::shared_ptr<ConstantRT>(new ConstantRT(val));
    }
    
    template<typename T2>
    bool operator==(const ConstantRT<T2>& o) const {
      return _val == o._val;
    }

    template<class RHS, typename = typename std::enable_if<detail::IsConstant<RHS>::value>::type>
    bool operator==(const RHS& o) const {
	    return _val == o;
    }
    
    const T& get() const { return _val; }
    
  private:
    T _val;
  };

  namespace detail {
    template<class T>
    struct Type_index<ConstantRT<T>> { static const int value = 0;  };
  }
}
