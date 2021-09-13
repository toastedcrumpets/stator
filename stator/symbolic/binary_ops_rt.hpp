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
/*! \brief Specialisation of BinaryOp for runtime arguments (Expr).
   */
  template<typename Op>
  struct BinaryOp<Expr, Op, Expr> : public RTBaseHelper<BinaryOp<Expr, Op, Expr> >, public Dynamic {
  protected:
    BinaryOp(const Expr& lhs, const Expr& rhs): _l(lhs), _r(rhs) {}
    BinaryOp(const BinaryOp& e) = default;
  public:

    static auto create(const Expr& lhs, const Expr& rhs) {
      return std::shared_ptr<BinaryOp>(new BinaryOp(lhs, rhs));
    }
    
    bool operator==(const BinaryOp& o) const {
      //Shortcut comparison before proceeding with item by item
      return (this == &o) || ((_l == o._l) && (_r == o._r));
    }
    
    Expr getLHS() const {
      return _l;
    }
    
    Expr getRHS() const {
      return _r;
    }

    Expr _l;
    Expr _r;    
  };
}
