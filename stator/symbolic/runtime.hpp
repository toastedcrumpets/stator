/*
  Copyright (C) 2015 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#ifdef STATOR_BOOST_SHARED
# include <boost/shared_ptr.hpp>
# include <boost/make_shared.hpp>
namespace sym {
  using boost::shared_ptr;
  using boost::make_shared;
  using boost::dynamic_pointer_cast;
}
#else
# include <memory>
namespace sym {
  using std::shared_ptr;
  using std::make_shared;
  using std::dynamic_pointer_cast;
}
#endif

#include <stator/symbolic/symbolic.hpp>

namespace sym {

  class RTBase;
  typedef shared_ptr<RTBase> Expr;
  class VarRT;
  
  /*! \brief Interface for runtime symbolic types. */
  class RTBase {
  public:
    virtual ~RTBase() {}

    virtual Expr clone() const = 0;

    virtual bool operator==(const Expr o) const = 0;

    virtual Expr sub(const VarRT&, Expr) const = 0;
  };

  /*! \brief CRTP helper base class which implements the boilerplate
    code for runtime symbolic types.
  */
  template<class Derived>
  class RTBaseHelper : public RTBase {
  public:
    virtual Expr clone() const {
      return Expr(new Derived(static_cast<const Derived&>(*this)));
    }

    virtual bool operator==(const Expr o) const {
      auto other = dynamic_pointer_cast<Derived>(o);
      if (!other)
	return false;
      return *static_cast<const Derived*>(this) == *other;
    }
  };
  
  class VarRT : public RTBaseHelper<VarRT> {
  public:
    template<typename ...Args>
    VarRT(const Var<Args...> v):
      idx(Var<Args...>::idx)
    {}

    bool operator==(const VarRT& o) const {
      return idx == o.idx;
    }

    Expr sub(const VarRT& var, Expr exp) const {
      if (*this == var)
	return exp->clone();
      else
	return this->clone();
    }
    
    char idx;
  };

  template<typename T>
  class ConstantRT : public RTBaseHelper<ConstantRT<T> > {
  public:
    ConstantRT(const T& v): _val(v) {}

    
    bool operator==(const ConstantRT& o) const {
      return _val == o._val;
    }
    
    /*! \brief Default substitution operation
     */
    virtual Expr sub(const VarRT& var, Expr exp) const {
      return this->clone();
    }

    void throw_self() const {
      throw _val;
    };

  private:
    T _val;
  };

  template<typename Op>
  class BinaryOpRT : public RTBaseHelper<BinaryOpRT<Op> > {
  public:
    BinaryOpRT(const Expr& lhs, const Expr& rhs): _LHS(lhs), _RHS(rhs) {}
    
    bool operator==(const BinaryOpRT<Op>& o) const {
      return (_LHS == o._LHS) && (_RHS == o._RHS);
    }
    
    virtual Expr sub(const VarRT& var, Expr exp) const {
      return Expr(new BinaryOpRT(_LHS->sub(var, exp), _RHS->sub(var, exp)));
    }
    
    Expr apply(Expr LHS, Expr RHS) {
      throw 0;
    }

  private:
    Expr _LHS;
    Expr _RHS;
  };

}
