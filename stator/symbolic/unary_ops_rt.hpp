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
  /*! \brief Specialisation of unary operator for runtime arguments
      and use in runtime expressions (Expr).
   */
  template<typename Op>
  struct UnaryOp<Expr,Op> : public RTBaseHelper<UnaryOp<Expr,Op> >, public Dynamic {
  protected:
    UnaryOp(const Expr& arg): _arg(arg) {}
    UnaryOp(const UnaryOp& e) = default;
    
  public:
    static auto create(const Expr& arg) {
      return std::shared_ptr<UnaryOp>(new UnaryOp(arg));
    }

    bool operator==(const UnaryOp& o) const {
      //Shortcut comparison before proceeding with item by item
      return (this == &o) || (_arg == o._arg);
    }

    template<class RHS>
    constexpr bool operator==(const RHS&) const {
      return false;
    }
    
    Expr getArg() const {
      return _arg;
    }

    Expr _arg;
  };

  //Need to forward declare this for the Dict and List simplifies to find it
  inline Expr simplify(const Expr& f);

}
