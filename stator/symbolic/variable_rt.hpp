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
  /*! \brief Specialisation of Var for runtime variables.*/
  template<>
  class Var<nullptr>: public RTBaseHelper<Var<nullptr> >, public Dynamic {
  protected:
    inline Var(const std::string name="x"):
      _name(name)
    {}

    Var(const Var& v) = delete;
    
    template<conststr N>
    Var(const Var<N>& v):
      _name(v.getName())
    {}

    
  public:
    static auto create(const std::string name="x") {
      return std::shared_ptr<VarRT>(new Var(name));
    }

    template<conststr N>
    static auto create(const Var<N>& v) {
      return std::shared_ptr<VarRT>(new Var(v));
    }

    
    inline bool operator==(const VarRT& o) const {
      //Shortcut comparison before proceeding with string comparison
      return (this == &o) || (_name == o._name);
    }

    template<class RHS>
    constexpr bool operator==(const RHS&) const {
      return false;
    }

    template<class Arg>
    auto operator=(const Arg& a) const {
      Expr lhs = Expr(*this);
      Expr rhs = Expr(a);
      return equality(lhs, rhs);
    }
    
    inline std::string getName() const { return _name; }
    
    std::string _name;
  };

  template<auto N, typename ...Args, typename Arg>
  Expr sub(const VarRT& f, const EqualityOp<Var<N, Args...>, Arg>& x)
  {
    if (f == x._l)
      return x._r;
    else
      return f;
  }

  /*! \brief Specialisation of Var for runtime variables.*/
  Expr derivative(const VarRT& v1, const VarRT& v2) {
    return Expr(v1 == v2);
  }
}
