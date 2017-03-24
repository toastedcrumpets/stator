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
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> RTMatrix;

  class RTBase;

  struct Expr : public shared_ptr<RTBase>, public SymbolicOperator {
    Expr() {}
    typedef shared_ptr<RTBase> Base;
    using Base::Base;
    Expr(const long&);
    Expr(const double&);
    Expr(const RTMatrix&);
    Expr(const std::complex<double>&);
    template<class LHS_t, class Op, class RHS_t>
    Expr(const BinaryOp<LHS_t, Op, RHS_t>&);
  };
  
  class VarRT;

  namespace detail {
    /*! \brief Base interface for runtime symbolic math to allow the
      visitor programming pattern.
    */
    struct VisitorInterface {
      virtual void visit(const double&) = 0;
      virtual void visit(const RTMatrix&) = 0;
      virtual void visit(const std::complex<double>&) = 0;
      virtual void visit(Expr) = 0;
    };
  }

  /*! \brief Interface for runtime symbolic types. */
  class RTBase : public std::enable_shared_from_this<RTBase> {
  public:
    virtual ~RTBase() {}

    virtual Expr clone() const = 0;

    virtual bool operator==(const Expr o) const = 0;

    //! \brief default visitor pattern interface
    virtual void visit(detail::VisitorInterface& c) {
      auto selfref = this->shared_from_this();
      c.visit(selfref);
    }

    virtual std::ostream& output(std::ostream&) const = 0;

    virtual Expr try_collapse() const {
      return clone();
    }
  };

  std::ostream& operator<<(std::ostream& os, const Expr& e) {
    return e->output(os);
  }

  namespace detail {
    template<typename Op, typename LHS_t>
    struct SecondDispatch: public VisitorInterface {
      SecondDispatch(const LHS_t& LHS, Op& op) : _LHS(LHS), _op(op) {}
      
      virtual void visit(const double& RHS) { run(RHS); }
      
      virtual void visit(const RTMatrix& RHS) { run(RHS); }
      
      virtual void visit(const std::complex<double>& RHS) { run(RHS); }
      
      virtual void visit(Expr RHS) { run(RHS); }

      const LHS_t& _LHS;
      Op& _op;
      
    private:
      template<class T>
      void run(const T& RHS) { _op.template apply<LHS_t, T>(_LHS, RHS); }
    };

    template<typename Op>
    struct FirstDispatch: public VisitorInterface {
      FirstDispatch(Expr RHS, Op& op):
	_RHS(RHS), _op(op) {}
      
      virtual void visit(const double& LHS) { run(LHS); }
      virtual void visit(const RTMatrix& LHS) { run(LHS); }
      virtual void visit(const std::complex<double>& LHS) { run(LHS); }
      virtual void visit(Expr LHS) { run(LHS); }
      
      Expr _RHS;
      Op& _op;

    private:
      template<class T>
      void run(const T& lhs) {
	SecondDispatch<Op, T> visitor(lhs, _op);
	_RHS->visit(visitor);
      }
    };
  }
  
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
    
    virtual std::ostream& output(std::ostream& os) const {
      return os << idx;
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
    
    void throw_self() const {
      throw _val;
    };

    virtual std::ostream& output(std::ostream& os) const {
      return os << _val;
    }

    virtual void visit(detail::VisitorInterface& c) {
      c.visit(_val);
    }

  private:
    T _val;
  };

  Expr::Expr(const long& v) : Expr(new ConstantRT<long>(v)) {}
  Expr::Expr(const double& v) : Expr(new ConstantRT<double>(v)) {}
  Expr::Expr(const RTMatrix& v) : Expr(new ConstantRT<RTMatrix>(v)) {}
  Expr::Expr(const std::complex<double>& v) : Expr(new ConstantRT<std::complex<double> >(v)) {}
  
  template<typename Op>
  class BinaryOpRT : public RTBaseHelper<BinaryOpRT<Op> > {
  public:
    BinaryOpRT(const Expr& lhs, const Expr& rhs): _LHS(lhs), _RHS(rhs) {}
    
    bool operator==(const BinaryOpRT<Op>& o) const {
      return (_LHS == o._LHS) && (_RHS == o._RHS);
    }
        
    Expr try_collapse() const {
      OpVisitor result;
      detail::FirstDispatch<OpVisitor> visitor(_RHS, result);
      _LHS->visit(visitor);
      return result._value;
    }
    
    virtual std::ostream& output(std::ostream& os) const {
      os << "(";
      _LHS->output(os);
      os << " " << Op::_str <<  " ";
      _RHS->output(os);
      os << ")";
      return os;
    }
    
  private:
    Expr _LHS;
    Expr _RHS;

    struct OpVisitor {
      template<class LHS, class RHS>
      void apply(const LHS& l, const RHS& r) {
	check_avail(l, r, detail::select_overload{});
      }

      template<class LHS, class RHS>
      auto check_avail(const LHS& l, const RHS& r, detail::choice<0>) -> decltype(Op::apply(l, r), void())
      { _value = store(Op::apply(l, r)); }

      template<class LHS, class RHS>
      void check_avail(const LHS& l, const RHS& r, detail::choice<1>)
      { stator_throw() << "No operator defined for " << l << Op::_str << r; }
      
      Expr _value;
    };
    
  };

  template<class LHS_t, class Op, class RHS_t>
  Expr::Expr(const BinaryOp<LHS_t, Op, RHS_t>& op) : Expr(new BinaryOpRT<Op>(op._l, op._r))
  {}
}
