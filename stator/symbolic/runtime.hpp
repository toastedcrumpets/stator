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

#include <stator/symbolic/symbolic.hpp>
#include <memory>
#include <sstream>

#ifdef STATOR_USE_BOOST_SHARED_PTR
# include <boost/shared_ptr.hpp>
namespace sym {
  using boost::shared_ptr;
  using boost::make_shared;
  using boost::dynamic_pointer_cast;
}
#else
namespace sym {
  using std::shared_ptr;
  using std::make_shared;
  using std::dynamic_pointer_cast;
}
#endif

namespace sym {
  class RTBase;
  struct Expr;
  template<class Op> struct UnaryOp<Expr, Op>;
  template<typename Op> using UnaryOpRT = UnaryOp<Expr, Op>;
  template<class Op> struct BinaryOp<Expr, Op, Expr>;
  template<typename Op> using BinaryOpRT = BinaryOp<Expr, Op, Expr>;

  /*! \brief Type used when template arguments correspond to the runtime specialisation. */
  struct Dynamic {};
  typedef Var<Dynamic> VarRT;

  /*! \brief The generic holder/smart pointer for a runtime Abstract
    Syntax Tree (AST) (expression).
    
    This class is simply a smart pointer, with specialised
    constructors to allow it to convert compile-time expressions (and
    formula strings) into runtime forms. It also inherits from
    SymbolicOperator and can be used in compile-time expressions.
  */
  struct Expr : public shared_ptr<RTBase>, public SymbolicOperator {
    Expr() {}

    //Allow the standard shared_ptr interface
    typedef shared_ptr<RTBase> Base;
    using Base::Base;

    Expr(const char*);
    Expr(const std::string&);

    Expr(const RTBase&);

    //Type conversion constructors from compile-time objects
    Expr(const double&);
    
    template<std::intmax_t Num, std::intmax_t Denom>
    Expr(const C<Num, Denom>& c);

    template<typename ...Args>
    Expr(const Var<Args...> v);
    
    template<class Op, class Arg_t>
    Expr(const UnaryOp<Arg_t, Op>&);

    template<class LHS_t, class Op, class RHS_t>
    Expr(const BinaryOp<LHS_t, Op, RHS_t>&);

    Expr(const detail::NoIdentity&) { stator_throw() << "This should never be called as NoIdentity is not a usable type";}

    bool operator==(const Expr&) const;
    bool operator!=(const Expr& o) const { return !(*this == o); }
    bool operator==(const detail::NoIdentity&) const { return false; }
    explicit operator bool() const { return shared_ptr<RTBase>::operator bool(); }
        
    template<class T> T as() const;
  };
    
  namespace detail {
    /*! \brief Abstract interface class for the visitor programming
      pattern for Expr types.

      This class describes the visitor pattern interface used for all
      transformations (and evaluations) of runtime Expr (AST).
    */
    struct VisitorInterface {
      virtual Expr visit(const double&) = 0;
      virtual Expr visit(const VarRT&) = 0;
      virtual Expr visit(const UnaryOpRT<detail::Sine>& ) = 0;
      virtual Expr visit(const UnaryOpRT<detail::Cosine>& ) = 0;
      virtual Expr visit(const UnaryOpRT<detail::Log>& ) = 0;
      virtual Expr visit(const UnaryOpRT<detail::Exp>& ) = 0;
      virtual Expr visit(const UnaryOpRT<detail::Absolute>& ) = 0;
      virtual Expr visit(const UnaryOpRT<detail::Arbsign>& ) = 0;
      virtual Expr visit(const BinaryOpRT<detail::Add>& ) = 0;
      virtual Expr visit(const BinaryOpRT<detail::Subtract>& ) = 0;
      virtual Expr visit(const BinaryOpRT<detail::Multiply>& ) = 0;
      virtual Expr visit(const BinaryOpRT<detail::Divide>& ) = 0;
      virtual Expr visit(const BinaryOpRT<detail::Power>& ) = 0;
    };


    /*! \brief A CRTP helper base class which transforms the visitor
        interface into a call to the derived classes apply function.

	This makes implementing visitors simpler, as a generic
	templated apply function can be used (i.e., all
	UnaryOp/BinaryOp can be treated with one imlementation each).
     */
    template<typename Derived>
    struct VisitorHelper: public VisitorInterface {
      virtual Expr visit(const double& x) { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const VarRT& x) { return static_cast<Derived*>(this)->apply(x); }

      virtual Expr visit(const UnaryOpRT<detail::Sine>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const UnaryOpRT<detail::Cosine>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const UnaryOpRT<detail::Log>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const UnaryOpRT<detail::Exp>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const UnaryOpRT<detail::Absolute>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const UnaryOpRT<detail::Arbsign>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const BinaryOpRT<detail::Add>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const BinaryOpRT<detail::Subtract>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const BinaryOpRT<detail::Multiply>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const BinaryOpRT<detail::Divide>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      virtual Expr visit(const BinaryOpRT<detail::Power>& x)
      { return static_cast<Derived*>(this)->apply(x); }
    };
  }

  /*! \brief Abstract interface class for all runtime symbolic
      classes. 
      
      This class defines the interface for all classes/symbols which
      can be held by \ref Expr. Most actual functionality is
      implemented using the \ref VisitorInterface via \ref visit.
  */
  class RTBase : public std::enable_shared_from_this<RTBase>, public SymbolicOperator {
  public:
    virtual ~RTBase() {}

    virtual Expr clone() const = 0;

    virtual bool operator==(const Expr o) const = 0;

    virtual Expr visit(detail::VisitorInterface& c) = 0;

    virtual std::string str() const = 0;

    virtual void throw_self() const {
      stator_throw() << "The expression (" << str() << ") does not resolve to a constant type.";
    }
  };

  /*! \brief String output operator for Expr classes.
   */
  std::ostream& operator<<(std::ostream& os, const Expr& e) {
    return os << e->str();
  }

  
  /*! \brief CRTP helper base class which implements some of the
    common boilerplate code for runtime symbolic types.
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

  /*! \brief Specialisation of Var for runtime variables.*/
  template<>
  class Var<Dynamic> : public RTBaseHelper<Var<Dynamic> > {
  public:
    Var(const char v) : idx(v) {}
    
    template<typename ...Args>
    Var(const Var<Args...> v):
      idx(Var<Args...>::idx)
    {}

    bool operator==(const VarRT& o) const {
      return idx == o.idx;
    }

    Relation<VarRT, Expr> operator=(const Expr& f) const {
      return Relation<VarRT, Expr>(*this, f);
    }
    
    virtual std::string str() const {
      std::ostringstream os;
      os << idx;
      return os.str();
    }

    Expr visit(detail::VisitorInterface& c) {
      return c.visit(*this);
    }

    char idx;
  };

  template<typename T>
  class ConstantRT : public RTBaseHelper<ConstantRT<T> > {
  public:
    ConstantRT(const T& v): _val(v) {}
    
    bool operator==(const Expr o) const;
    
    std::string str() const {
      std::ostringstream os;
      os << _val;
      return os.str();
    }

    Expr visit(detail::VisitorInterface& c) {
      return c.visit(_val);
    }

    void throw_self() const {
      throw _val;
    }

    const T& get() const { return _val; }
    
  private:
    T _val;
  };

  namespace detail {
    template<class LHS_t>
    class CompareConstantsVisitor : public VisitorHelper<CompareConstantsVisitor<LHS_t> > {
    public:
      CompareConstantsVisitor(const LHS_t& l) : _l(l), _value(false) {}
      
      template<class T> Expr apply(const T& rhs) {
	return worker(rhs, detail::select_overload{});
      }

      //This implementation is available whenever the two types have a comparison operator defined
      template<class T>
      auto worker(const T& rhs, detail::choice<0>) -> decltype(LHS_t() == rhs, Expr())
      {
	_value = _l == rhs;
	return Expr();
      }
      
      //If they don't have a comparison operator defined, we assume
      //they are not comparable
      template<class T>
      Expr worker(const T& rhs, detail::choice<1>)
      { return Expr(); }
	
      const LHS_t& _l;
      bool _value;
    };
  }

  template<class T>
  bool ConstantRT<T>::operator==(const Expr o) const {
    detail::CompareConstantsVisitor<T> visitor(_val);
    o->visit(visitor);
    return visitor._value;
  }
  

  /*! \brief Specialisation of unary operator for runtime arguments
      and use in runtime expressions (Expr).
   */
  template<typename Op>
  struct UnaryOp<Expr,Op> : public RTBaseHelper<UnaryOp<Expr,Op> > {
    UnaryOp(const Expr& arg): _arg(arg) {}

    Expr clone() const {
      return Expr(new UnaryOp<Expr,Op>(_arg->clone()));
    }

    bool operator==(const UnaryOp<Expr,Op>& o) const {
      return (_arg == o._arg);
    }
    
    virtual std::string str() const {
      std::ostringstream os;
      os << Op::_str_left << _arg << Op::_str_right ;
      return os.str();
    }

    Expr getArg() const {
      return _arg;
    }

    Expr visit(detail::VisitorInterface& c) {
      return c.visit(*this);
    }
    
    Expr _arg;
  };

  /*! \brief Specialisation of BinaryOp for runtime arguments (Expr).
   */
  template<typename Op>
  struct BinaryOp<Expr, Op, Expr> : public RTBaseHelper<BinaryOp<Expr, Op, Expr> > {
    BinaryOp(const Expr& lhs, const Expr& rhs): _l(lhs), _r(rhs) {}

    Expr clone() const {
      return Expr(new BinaryOp(_l->clone(), _r->clone()));
    }
    
    bool operator==(const BinaryOp& o) const {
      return (_l == o._l) && (_r == o._r);
    }
    
    virtual std::string str() const {
      std::ostringstream os;
      os << "(" << _l->str() << " " << Op::str() <<  " " << _r->str() << ")";
      return os.str();
    }

    Expr getLHS() const {
      return _l;
    }
    
    Expr getRHS() const {
      return _r;
    }

    Expr visit(detail::VisitorInterface& c) {
      return c.visit(*this);
    }

    Expr _l;
    Expr _r;
  };

  Expr::Expr(const RTBase& v) : Expr(v.clone()) {}

  Expr::Expr(const double& v) : Expr(new ConstantRT<double>(v)) {}
  
  template<std::intmax_t Num, std::intmax_t Denom>
  Expr::Expr(const C<Num, Denom>& c) : Expr(new ConstantRT<double>(double(Num) / Denom)) {}
  
  template<class Op, class Arg_t>
  Expr::Expr(const UnaryOp<Arg_t, Op>& op) : Expr(new UnaryOpRT<Op>(op._arg)) {}

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

  bool Expr::operator==(const Expr& e) const {
    return (*(*this)) == e;
  }

  namespace detail {
    template<typename Visitor, typename LHS_t, typename Op>
    struct DoubleDispatch2: public VisitorHelper<DoubleDispatch2<Visitor, LHS_t, Op> > {
      DoubleDispatch2(const LHS_t& LHS, Visitor& visitor) : _LHS(LHS), _visitor(visitor) {}
      
      template<class RHS_t>
      Expr apply(const RHS_t& RHS) { return _visitor.template dd_visit<Op>(_LHS, RHS); }

    private:
      const LHS_t& _LHS;
      Visitor& _visitor;
    };

    template<typename Visitor, typename Op>
    struct DoubleDispatch1: public VisitorHelper<DoubleDispatch1<Visitor, Op> > {
      DoubleDispatch1(const Expr& RHS, Visitor& visitor):
	_RHS(RHS), _visitor(visitor) {}
      
      template<class LHS_t>
      Expr apply(const LHS_t& lhs) {
	DoubleDispatch2<Visitor, LHS_t, Op> visitor(lhs, _visitor);
	return _RHS->visit(visitor);
      }

    private:
      const Expr& _RHS;
      Visitor& _visitor;
    };

    struct SimplifyRT : VisitorHelper<SimplifyRT> {
      template<class Op>
      struct UnaryEval : VisitorHelper<UnaryEval<Op> > {
	template<class T> Expr apply(const T& arg) { return Op::apply(arg); }
      };
      
      
      //This uses SFINAE to exclude itself when the operator does not apply to those types.
      template<class Op, class LHS_t, class RHS_t>
      auto dd_visit(const LHS_t& l, const RHS_t& r) -> STATOR_AUTORETURN(Expr(store(Op::apply(l, r))))

      //As everything is specialised, this requires type conversions
      //to work, thus it should be chosen last
      template<class Op>
      Expr dd_visit(const Expr& l, const Expr& r) {
	return Op::apply(l, r);
      }

      //By default, just copy the item
      template<class T>
      Expr apply(const T& v) {
	return Expr(v);
      }

      template<typename Op>
      Expr apply(const UnaryOpRT<Op>& op) {
	//Simplify the argument
	Expr arg = op.getArg()->visit(*this);
	//Try evaluating the unary expression
	UnaryEval<Op> visitor;
	return arg->visit(visitor);
      }

      //For binary operators, try to collapse them
      template<typename Op>
      Expr apply(const BinaryOpRT<Op>& op) {
	Expr l = op.getLHS()->visit(*this);
	Expr r = op.getRHS()->visit(*this);

	if (r == typename Op::right_zero())
	  return typename Op::right_zero();
	
	if (l == typename Op::left_zero())
	  return typename Op::left_zero();

	if (r == typename Op::right_identity())
	  return l;
	
	if (l == typename Op::left_identity())
	  return r;
	
	DoubleDispatch1<SimplifyRT, Op> visitor(r, *this);
	return l->visit(visitor);
      }
    };
  }

  Expr simplify(const Expr& f) {
    detail::SimplifyRT visitor;
    return f->visit(visitor);
  }
  
  namespace detail {
    struct SubstituteRT : VisitorHelper<SubstituteRT> {
      SubstituteRT(VarRT var, Expr replacement): _var(var), _replacement(replacement) {}
      
      //By default, just copy the item
      template<class T>
      Expr apply(const T& v) { return Expr(v); }

      //Variable matching
      Expr apply(const VarRT& v) {
	if (v == _var)
	  //Need to ensure a copy is performed here
	  return Expr(_replacement);

	return Expr(v);
      }
      
      template<typename Op>
      Expr apply(const UnaryOpRT<Op>& op) {
	return Op::apply(op.getArg()->visit(*this));
      }

      template<typename Op>
      Expr apply(const BinaryOpRT<Op>& op) {
	return Op::apply(op.getLHS()->visit(*this), op.getRHS()->visit(*this));
      }
      
      VarRT _var;
      Expr _replacement;
    };
  }
  
  template<class Var, class Arg>
  Expr sub(const Expr& f, const Relation<Var, Arg>& rel) {
    detail::SubstituteRT visitor(rel._var, rel._val);
    return f->visit(visitor);
  }

  namespace detail {
    struct DerivativeRT : VisitorHelper<DerivativeRT> {
      DerivativeRT(VarRT var): _var(var) {}

      template<class T, typename = typename std::enable_if<IsConstant<T>::value>::type>
      Expr apply(const T& v) {
	return ConstantRT<double>(0);
      }
      
      Expr apply(const VarRT& v) {
	return Expr((v == _var) ? new ConstantRT<double>(1) : new ConstantRT<double>(0));
      }

      template<class Op>
      Expr apply(const UnaryOpRT<Op>& v) {
	//Hand over to the compile time implementation
	return derivative(Op::apply(v.getArg()), _var);
      }

      template<class Op>
      Expr apply(const BinaryOpRT<Op>& v) {
	//Hand over to the compile time implementation
	return derivative(Op::apply(v.getLHS(), v.getRHS()), _var);
      }
      
      VarRT _var;
    };
  }
  
  template<class Var>
  Expr derivative(const Expr& f, const Var v) {
    detail::DerivativeRT visitor(v);
    return f->visit(visitor);
  }
}
