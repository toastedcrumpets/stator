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

#include <stator/symbolic/symbolic.hpp>
#include <memory>
#include <sstream>

namespace sym {
  using std::shared_ptr;
  using std::make_shared;
  using std::dynamic_pointer_cast;
}

namespace sym {
  class RTBase;

  /*! \brief The holder for a runtime expression or Abstract Syntax
      Tree (AST).

      This class is simply a smart pointer, with specialised
      constructors to allow it to convert compile-time expressions
      into runtime forms. It also inherits from SymbolicOperator to
      allow it to be used in compile-time expressions.
   */
  struct Expr : public shared_ptr<RTBase>, public SymbolicOperator {
    Expr() {}

    //Allow the standard shared_ptr interface
    typedef shared_ptr<RTBase> Base;
    using Base::Base;

    Expr(const RTBase&);

    //Type conversion constructors from compile-time objects
    Expr(const int&);
    Expr(const long&);
    Expr(const double&);
    Expr(const std::complex<double>&);
    
    template<std::intmax_t Num, std::intmax_t Denom>
    Expr(const C<Num, Denom>& c);

    template<typename ...Args>
    Expr(const Var<Args...> v);
    
    template<class LHS_t, class Op, class RHS_t>
    Expr(const BinaryOp<LHS_t, Op, RHS_t>&);

    template<class T>
    T as() const;
  };
  
  class VarRT;
  template<typename Op> class BinaryOpRT;
  
  namespace detail {
    /*! \brief Base interface for the visitor programming pattern.

      This class is used to propogate and evaluate transformations of
      the AST using the visitor pattern.
    */
    struct VisitorInterface {
      virtual void visit(const int&) = 0;
      virtual void visit(const long&) = 0;
      virtual void visit(const double&) = 0;
      virtual void visit(const std::complex<double>&) = 0;
      virtual void visit(const VarRT&) = 0;
      virtual void visit(const BinaryOpRT<detail::Add>& ) = 0;
      virtual void visit(const BinaryOpRT<detail::Subtract>& ) = 0;
      virtual void visit(const BinaryOpRT<detail::Multiply>& ) = 0;
      virtual void visit(const BinaryOpRT<detail::Divide>& ) = 0;
      virtual void visit(const BinaryOpRT<detail::Power>& ) = 0;
      virtual void visit(const Expr&) = 0;
    };


    /*! \brief A CRTP helper to facilitate all the required
        specialisations for the visitor interface.
     */
    template<typename Derived>
    struct VisitorHelper: public VisitorInterface {
      virtual void visit(const int& x) { static_cast<Derived*>(this)->apply(x); }
      virtual void visit(const long& x) { static_cast<Derived*>(this)->apply(x); }
      virtual void visit(const double& x) { static_cast<Derived*>(this)->apply(x); }
      virtual void visit(const std::complex<double>& x) { static_cast<Derived*>(this)->apply(x); }
      virtual void visit(const VarRT& x) { static_cast<Derived*>(this)->apply(x); }
      virtual void visit(const Expr& x) { static_cast<Derived*>(this)->apply(x); }

      virtual void visit(const BinaryOpRT<detail::Add>& x)
      { static_cast<Derived*>(this)->apply(x); }
      virtual void visit(const BinaryOpRT<detail::Subtract>& x)
      { static_cast<Derived*>(this)->apply(x); }
      virtual void visit(const BinaryOpRT<detail::Multiply>& x)
      { static_cast<Derived*>(this)->apply(x); }
      virtual void visit(const BinaryOpRT<detail::Divide>& x)
      { static_cast<Derived*>(this)->apply(x); }
      virtual void visit(const BinaryOpRT<detail::Power>& x)
      { static_cast<Derived*>(this)->apply(x); }
    };
  }

  /*! \brief Interface for runtime symbolic types. */
  class RTBase : public std::enable_shared_from_this<RTBase>, public SymbolicOperator {
  public:
    virtual ~RTBase() {}

    virtual Expr clone() const = 0;

    virtual bool operator==(const Expr o) const = 0;

    virtual void visit(detail::VisitorInterface& c) = 0;

    virtual std::string str() const = 0;

    virtual void throw_self() const {
      stator_throw() << "The expression (" << str() << ") does not resolve to a constant type.";
    }
  };

  std::ostream& operator<<(std::ostream& os, const Expr& e) {
    return os << e->str();
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
    
    virtual std::string str() const {
      std::ostringstream os;
      os << idx;
      return os.str();
    }

    void visit(detail::VisitorInterface& c) {
      c.visit(*this);
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
    
    std::string str() const {
      std::ostringstream os;
      os << _val;
      return os.str();
    }

    void visit(detail::VisitorInterface& c) {
      c.visit(_val);
    }

    void throw_self() const {
      throw _val;
    }
    
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
    
    virtual std::string str() const {
      std::ostringstream os;
      os << "(" << _LHS->str() << " " << Op::str() <<  " " << _RHS->str() << ")";
      return os.str();
    }

    Expr getLHS() const {
      return _LHS;
    }
    
    Expr getRHS() const {
      return _RHS;
    }

    void visit(detail::VisitorInterface& c) {
      c.visit(*this);
    }
    
  private:
    Expr _LHS;
    Expr _RHS;
  };

  Expr::Expr(const RTBase& v) : Expr(v.clone()) {}

  Expr::Expr(const int& v) : Expr(new ConstantRT<int>(v)) {}
  Expr::Expr(const long& v) : Expr(new ConstantRT<long>(v)) {}
  Expr::Expr(const double& v) : Expr(new ConstantRT<double>(v)) {}
  Expr::Expr(const std::complex<double>& v) : Expr(new ConstantRT<std::complex<double> >(v)) {}
  
  template<std::intmax_t Num, std::intmax_t Denom>
  Expr::Expr(const C<Num, Denom>& c) : Expr(new ConstantRT<double>(double(Num) / Denom)) {}
  
  template<class LHS_t, class Op, class RHS_t>
  Expr::Expr(const BinaryOp<LHS_t, Op, RHS_t>& op) : Expr(new BinaryOpRT<Op>(op._l, op._r)) {}
  
  template<typename ...Args>
  Expr::Expr(const Var<Args...> v) : Expr(new VarRT(v)) {}

  template<class T>
  T Expr::as() const {
    try {
      (*this)->throw_self();
    } catch(const T& v) {
      return v;
    } catch(const stator::Exception& e) {
      throw e;
    } catch(...) {}

    stator_throw() << "Uncaught error! Check implementation of throw_self in expression (" << (*this)->str() << ")";
  }

  namespace detail {
    template<typename Visitor, typename LHS_t, typename Op>
    struct DoubleDispatch2: public VisitorHelper<DoubleDispatch2<Visitor, LHS_t, Op> > {
      DoubleDispatch2(const LHS_t& LHS, Visitor& visitor) : _LHS(LHS), _visitor(visitor) {}
      
      template<class RHS_t>
      void apply(const RHS_t& RHS) { _visitor.template dd_visit<Op>(_LHS, RHS); }

    private:
      const LHS_t& _LHS;
      Visitor& _visitor;
    };

    template<typename Visitor, typename Op>
    struct DoubleDispatch1: public VisitorHelper<DoubleDispatch1<Visitor, Op> > {
      DoubleDispatch1(const Expr& RHS, Visitor& visitor):
	_RHS(RHS), _visitor(visitor) {}
      
      template<class LHS_t>
      void apply(const LHS_t& lhs) {
	DoubleDispatch2<Visitor, LHS_t, Op> visitor(lhs, _visitor);
	_RHS->visit(visitor);
      }

    private:
      const Expr& _RHS;
      Visitor& _visitor;
    };

    struct SimplifyRT : VisitorHelper<SimplifyRT> {

      //This uses SFINAE to exclude itself when the operator does not apply to those types.
      template<class Op, class LHS_t, class RHS_t>
      auto dd_visit(const LHS_t& l, const RHS_t& r) -> decltype((void)Op::apply(l, r)) {
	_result = Expr(store(Op::apply(l, r)));
      }

      //As everything is specialised, this requires type conversions to work, thus it should be chosen last
      template<class Op>
      void dd_visit(const Expr& l, const Expr& r) {
	//Do nothing, the existing value has been copied into the result
	//anyway.
      }

      //By default, just copy the item
      template<class T>
      void apply(const T& v) {
	_result = Expr(v);
      }

      //For binary operators, try to collapse them
      template<typename Op>
      void apply(const BinaryOpRT<Op>& op) {
	Expr l = run_simplify(op.getLHS());
	Expr r = run_simplify(op.getRHS());

	//Incase of failed match, just return the original item
	_result = op.clone();
	  
	DoubleDispatch1<SimplifyRT, Op> visitor(r, *this);
	l->visit(visitor);
      }
      
      Expr _result;

      static Expr run_simplify(const Expr& f) {
	SimplifyRT visitor;
	f->visit(visitor);
	return visitor._result;
      }
    };
  }

  Expr simplify(const Expr& f) {
    return detail::SimplifyRT::run_simplify(f);
  }

}
