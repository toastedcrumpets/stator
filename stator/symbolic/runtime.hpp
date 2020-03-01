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
#include <stator/repr.hpp>

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
}

namespace stator {
  template<class Config = DefaultReprConfig> std::string repr(const sym::RTBase&);
  template<class Config = DefaultReprConfig> std::string repr(const sym::Expr&);
}

namespace sym {
  template<class Op> struct UnaryOp<Expr, Op>;
  template<class Op> struct BinaryOp<Expr, Op, Expr>;
  template<class T>  class ConstantRT;
  typedef Var<Dynamic> VarRT;

  /*! \brief The generic holder/smart pointer for a runtime Abstract
    Syntax Tree (AST) (expression).
    
    This class is simply a smart pointer, with specialised
    constructors to allow it to convert compile-time expressions (and
    formula strings) into runtime forms. It also inherits from
    SymbolicOperator and can be used in compile-time expressions.
  */
  struct Expr : public shared_ptr<const RTBase>, public SymbolicOperator {
    typedef shared_ptr<const RTBase> Base;

    inline Expr() {}
    inline Expr(const std::shared_ptr<const RTBase>& p) : Base(p) {}

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

    inline
    Expr(const detail::NoIdentity&) { stator_throw() << "This should never be called as NoIdentity is not a usable type";}

    bool operator==(const Expr&) const;
    bool operator!=(const Expr& o) const { return !(*this == o); }
    bool operator==(const detail::NoIdentity&) const { return false; }
    explicit operator bool() const { return Base::operator bool(); }
        
    template<class T> T as() const;
  };
  
  namespace detail {
    template<class T>
    struct RT_type_index {
      static_assert(sizeof(T) == -1, "Missing index for runtime type");
    };

    template<> struct RT_type_index<ConstantRT<double>>               { static const int value = 0; };
    template<> struct RT_type_index<VarRT>                            { static const int value = 1; };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Sine>>      { static const int value = 2; };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Cosine>>    { static const int value = 3; };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Log>>       { static const int value = 4; };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Exp>>       { static const int value = 5; };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Absolute>>  { static const int value = 6; };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Arbsign>>   { static const int value = 7; };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Add, Expr>>      { static const int value = 8; };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Subtract, Expr>> { static const int value = 9; };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Multiply, Expr>> { static const int value = 10; };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Divide, Expr>>   { static const int value = 11; };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Power, Expr>>    { static const int value = 12; };
    
    /*! \brief Abstract interface class for the visitor programming
      pattern for Expr types.

      This class describes the visitor pattern interface used for all
      transformations (and evaluations) of runtime Expr (AST).
    */
    template<class RetType>
    struct VisitorInterface {
      virtual RetType visit(const double&) = 0;
      virtual RetType visit(const VarRT&) = 0;
      virtual RetType visit(const UnaryOp<Expr, detail::Sine>& ) = 0;
      virtual RetType visit(const UnaryOp<Expr, detail::Cosine>& ) = 0;
      virtual RetType visit(const UnaryOp<Expr, detail::Log>& ) = 0;
      virtual RetType visit(const UnaryOp<Expr, detail::Exp>& ) = 0;
      virtual RetType visit(const UnaryOp<Expr, detail::Absolute>& ) = 0;
      virtual RetType visit(const UnaryOp<Expr, detail::Arbsign>& ) = 0;
      virtual RetType visit(const BinaryOp<Expr, detail::Add, Expr>& ) = 0;
      virtual RetType visit(const BinaryOp<Expr, detail::Subtract, Expr>& ) = 0;
      virtual RetType visit(const BinaryOp<Expr, detail::Multiply, Expr>& ) = 0;
      virtual RetType visit(const BinaryOp<Expr, detail::Divide, Expr>& ) = 0;
      virtual RetType visit(const BinaryOp<Expr, detail::Power, Expr>& ) = 0;
    };


    /*! \brief A CRTP helper base class which transforms the visitor
        interface into a call to the derived classes apply function.

	This makes implementing visitors simpler, as a generic
	templated apply function can be used (i.e., all
	UnaryOp/BinaryOp can be treated with one imlementation each).
     */
    template<class Derived, class RetType = Expr>
    struct VisitorHelper: public VisitorInterface<RetType> {
      inline virtual RetType visit(const double& x) { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const VarRT& x) { return static_cast<Derived*>(this)->apply(x); }

      inline virtual RetType visit(const UnaryOp<Expr, detail::Sine>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const UnaryOp<Expr, detail::Cosine>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const UnaryOp<Expr, detail::Log>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const UnaryOp<Expr, detail::Exp>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const UnaryOp<Expr, detail::Absolute>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const UnaryOp<Expr, detail::Arbsign>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const BinaryOp<Expr, detail::Add, Expr>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const BinaryOp<Expr, detail::Subtract, Expr>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const BinaryOp<Expr, detail::Multiply, Expr>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const BinaryOp<Expr, detail::Divide, Expr>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const BinaryOp<Expr, detail::Power, Expr>& x)
      { return static_cast<Derived*>(this)->apply(x); }
    };
  }

  /*! \brief Abstract interface class for all runtime symbolic
      classes. 
      
      This class defines the interface for all classes/symbols which
      can be held by \ref Expr. Most actual functionality is
      implemented using the \ref VisitorInterface via \ref visit.
  */
  class RTBase : public SymbolicOperator, public std::enable_shared_from_this<RTBase> {
  public:
    inline RTBase(int idx): _type_idx(idx) {}
    
    inline virtual ~RTBase() {}

    virtual Expr clone() const = 0;

    virtual bool operator==(const Expr o) const = 0;

    const int _type_idx;

    template<class RetType>
    RetType visit(detail::VisitorInterface<RetType>& c) const;
  };
  
  /*! \brief CRTP helper base class which implements some of the
    common boilerplate code for runtime symbolic types.
  */
  template<class Derived>
  class RTBaseHelper : public RTBase {
  public:
    RTBaseHelper(): RTBase(detail::RT_type_index<Derived>::value) {}
    
    Expr clone() const {
      return Expr(std::make_shared<Derived>(static_cast<const Derived&>(*this)));
    }
    
    bool operator==(const Expr o) const {
      auto other = dynamic_pointer_cast<const Derived>(o);
      if (!other)
	return false;
      return *static_cast<const Derived*>(this) == *other;
    }
  };  

  /*! \brief Specialisation of Var for runtime variables.*/
  template<>
  class Var<Dynamic> : public RTBaseHelper<Var<Dynamic> >, public Dynamic {
  public:
    inline Var(const char v) : idx(v) {}
    
    template<typename ...Args>
    Var(const Var<Args...> v):
      idx(Var<Args...>::idx)
    {}

    inline bool operator==(const VarRT& o) const {
      return idx == o.idx;
    }

    inline Relation<VarRT, Expr> operator=(const Expr& f) const {
      return Relation<VarRT, Expr>(*this, f);
    }
        
    inline char getidx() const { return idx; } 
    
    char idx;
  };

  /*! \brief Determine the derivative of a variable by another variable.

    If the variable is NOT the variable in which a derivative is
    being taken, then this overload should be selected to return
    Null.
  */
  template<class ...Args1, class ...Args2,
	   typename = typename std::enable_if<std::is_base_of<Dynamic, Var<Args1...> >::value || std::is_base_of<Dynamic, Var<Args2...> >::value>::type >
  Expr derivative(Var<Args1...> v1, Var<Args2...> v2) {
    return Expr(v1.getidx() == v2.getidx());
  }

  
  template<typename T>
  class ConstantRT : public RTBaseHelper<ConstantRT<T> > {
  public:
    ConstantRT(const T& v): _val(v) {}
    
    bool operator==(const Expr o) const;
    
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
  struct UnaryOp<Expr,Op> : public RTBaseHelper<UnaryOp<Expr,Op> >, public Dynamic {
    UnaryOp(const Expr& arg): _arg(arg) {}

    Expr clone() const {
      return Expr(std::make_shared<UnaryOp>(_arg->clone()));
    }

    bool operator==(const UnaryOp<Expr,Op>& o) const {
      return (_arg == o._arg);
    }
    
    Expr getArg() const {
      return _arg;
    }

    Expr _arg;
  };

  /*! \brief Specialisation of BinaryOp for runtime arguments (Expr).
   */
  template<typename Op>
  struct BinaryOp<Expr, Op, Expr> : public RTBaseHelper<BinaryOp<Expr, Op, Expr> >, public Dynamic {
    BinaryOp(const Expr& lhs, const Expr& rhs): _l(lhs), _r(rhs) {}

    Expr clone() const {
      return Expr(std::make_shared<BinaryOp>(_l->clone(), _r->clone()));
    }
    
    bool operator==(const BinaryOp& o) const {
      return (_l == o._l) && (_r == o._r);
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

  inline Expr::Expr(const RTBase& v) : Base(v.clone()) {}

  inline Expr::Expr(const double& v) : Base(std::make_shared<ConstantRT<double> >(v)) {}
  
  template<std::intmax_t Num, std::intmax_t Denom>
  Expr::Expr(const C<Num, Denom>& c) : Base(std::make_shared<ConstantRT<double> >(double(Num) / Denom)) {}
  
  template<class Op, class Arg_t>
  Expr::Expr(const UnaryOp<Arg_t, Op>& op) : Base(std::make_shared<UnaryOp<Expr, Op> >(op._arg)) {}

  template<class LHS_t, class Op, class RHS_t>
  Expr::Expr(const BinaryOp<LHS_t, Op, RHS_t>& op) : Base(std::make_shared<BinaryOp<Expr, Op, Expr> >(op._l, op._r)) {}
  
  template<typename ...Args>
  Expr::Expr(const Var<Args...> v) : Base(std::make_shared<VarRT>(v)) {}

  template<class T>
  T Expr::as() const {
    auto* ptr = dynamic_cast<const ConstantRT<T>*>(Base::get());
    if (!ptr)
      stator_throw() << "Invalid as<>(), expression is as follows:" << *this;

    return ptr->get();
  }

  inline bool Expr::operator==(const Expr& e) const {
    return (*(*this)) == e;
  }

  namespace detail {
    template<typename Visitor, typename LHS_t, typename Op>
    struct DoubleDispatch2: public VisitorHelper<DoubleDispatch2<Visitor, LHS_t, Op> > {
      DoubleDispatch2(const LHS_t& LHS, Visitor& visitor) : _LHS(LHS), _visitor(visitor) {}
      
      template<class RHS_t>
      Expr apply(const RHS_t& RHS) { return _visitor.dd_visit(_LHS, RHS, Op()); }

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
      Expr _RHS;
      Visitor& _visitor;
    };

    struct SimplifyRT : VisitorHelper<SimplifyRT> {
      template<class Op>
      struct UnaryEval : VisitorHelper<UnaryEval<Op> > {
	Expr apply(const double& arg) { return Op::apply(arg); }
	template<class T> Expr apply(const T& arg) { return Expr(); }
      };

      //Direct evaluation of doubles
      template<class T1, class T2, class Op>
      Expr dd_visit(const T1& l, const T2& r, Op) {
	if (IsSymbolic<decltype(store(Op::apply(l, r)))>::value)
	  return Expr();
	return Expr(store(Op::apply(l, r)));
      }

      //Direct evaluation of doubles
      template<class T2>
      Expr dd_visit(const double& l, const T2& r, detail::Subtract) {
	if (l == 0)
	  return Expr(-r);
	
	if (IsSymbolic<decltype(store(detail::Subtract::apply(l, r)))>::value)
	  return Expr();
	return Expr(store(detail::Subtract::apply(l, r)));
      }

      //Direct evaluation of doubles
      Expr dd_visit(const double& l, const BinaryOp<Expr, detail::Multiply, Expr>& r, detail::Multiply) {
	if (dynamic_cast<const ConstantRT<double>*>(r.getLHS().get()))
	  return Expr(BinaryOp<Expr, detail::Multiply, Expr>(l * static_cast<const ConstantRT<double>*>(r.getLHS().get())->get(), r.getRHS()));

	if (dynamic_cast<const ConstantRT<double>*>(r.getRHS().get()))
	  return Expr(BinaryOp<Expr, detail::Multiply, Expr>(l * static_cast<const ConstantRT<double>*>(r.getRHS().get())->get(), r.getLHS()));
	return Expr();
      }

      Expr dd_visit(const BinaryOp<Expr, detail::Multiply, Expr>& r, const double& l, detail::Multiply) {
	if (dynamic_cast<const ConstantRT<double>*>(r.getLHS().get()))
	  return Expr(BinaryOp<Expr, detail::Multiply, Expr>(l * static_cast<const ConstantRT<double>*>(r.getLHS().get())->get(), r.getRHS()));

	if (dynamic_cast<const ConstantRT<double>*>(r.getRHS().get()))
	  return Expr(BinaryOp<Expr, detail::Multiply, Expr>(l * static_cast<const ConstantRT<double>*>(r.getRHS().get())->get(), r.getLHS()));
	return Expr();
      }
      
      //By default return the blank (use 
      template<class T>
      Expr apply(const T& v) {
	return Expr();
      }

      template<typename Op>
      Expr apply(const UnaryOp<Expr, Op>& op) {
	//Simplify the argument
	Expr arg = op.getArg()->visit(*this);
	//Try evaluating the unary expression
	UnaryEval<Op> visitor;
	Expr ret = (arg ? arg : op.getArg())->visit(visitor);
	if (ret)
	  return ret;	
	if (arg)
	  return Expr(std::make_shared<UnaryOp<Expr, Op> >(arg));
	return Expr();
      }

      //For binary operators, try to collapse them
      template<typename Op>
      Expr apply(const BinaryOp<Expr, Op, Expr>& op) {
	Expr l = op.getLHS()->visit(*this);
	bool lchanged = !!l;
	if (!l) l = op._l;

	Expr r = op.getRHS()->visit(*this);
	bool rchanged = !!r;
	if (!r) r = op._r;

	if (l == typename Op::left_zero())
	  return typename Op::left_zero();
	if (l == typename Op::left_identity())
	  return r;
	if (r == typename Op::right_zero())
	  return typename Op::right_zero();
	if (r == typename Op::right_identity())
	  return l;
	
	//Try direct simplification via application
	DoubleDispatch1<SimplifyRT, Op> visitor(r, *this);
	Expr ret = l->visit(visitor);
	if (ret)
	  return ret;

	if (lchanged || rchanged)
	  return Expr(std::make_shared<BinaryOp<Expr, Op, Expr> >(l, r));

	return Expr();
      }
    };
  }

  inline Expr simplify(const Expr& f) {
    detail::SimplifyRT visitor;
    Expr result = f->visit(visitor);
    return result ? result : f;
  }
  
  namespace detail {
    struct SubstituteRT : VisitorHelper<SubstituteRT> {
      SubstituteRT(VarRT var, Expr replacement): _var(var), _replacement(replacement) {}
      
      //By default, just return an empty Expr, and let the helper function return the original expression
      template<class T>
      Expr apply(const T& v)
      { return Expr(); }

      //Variable matching
      Expr apply(const VarRT& v) {
	if (v == _var)
	  return _replacement;
	return Expr();
      }
      
      template<typename Op>
      Expr apply(const UnaryOp<Expr, Op>& op) {
	Expr arg = op.getArg()->visit(*this);
	return arg ? Expr(Op::apply(arg)) : op.shared_from_this();
      }

      template<typename Op>
      Expr apply(const BinaryOp<Expr, Op, Expr>& op) {
	Expr l = op.getLHS()->visit(*this);
	Expr r = op.getRHS()->visit(*this);

	if (l || r)
	  return Expr(Op::apply(l ? l : op._l, r ? r : op._r));
	else
	  return Expr();
      }
      
      VarRT _var;
      Expr _replacement;
    };
  }
  
  template<class Var, class Arg>
  Expr sub(const Expr& f, const Relation<Var, Arg>& rel) {
    detail::SubstituteRT visitor(rel._var, rel._val);
    Expr result = f->visit(visitor);
    return (result) ?  result : f;
  }

  namespace detail {
    struct DerivativeRT : VisitorHelper<DerivativeRT> {
      DerivativeRT(VarRT var): _var(var) {}

      //The commented out code here is a double dispatch
      //implementation of derivatives; however, it has a huge compile
      //time cost.

      
      ///*! \brief Visitor to allow the compile time derivative
      //    implementation for unary operators. 
      //	  
      //	  This visitor is used to determine the type of the argument.
      //*/
      //template<class Op>
      //struct UnaryEval : VisitorHelper<UnaryEval<Op> > {
      //	UnaryEval(VarRT var): _var(var) {}
      //	template<class T> Expr apply(const T& arg) { return derivative(Op::apply(arg), _var); }
      //	VarRT _var;
      //};
      //
      ///*! \brief Handover to compile-time implementation for binary op derivatives. */
      //template<class Op, class LHS_t, class RHS_t>
      //Expr dd_visit(const LHS_t& l, const RHS_t& r) {
      //	auto e = Op::apply(l, r);
      //	return derivative(e, _var);
      //}
      
      template<class T, typename = typename std::enable_if<IsConstant<T>::value>::type>
      Expr apply(const T& v) {
	return ConstantRT<double>(0);
      }

      Expr apply(const VarRT& v) {
	return Expr((v == _var) ? 1 : 0);
      }

      template<class Op>
      Expr apply(const UnaryOp<Expr, Op>& v) {
	return derivative(v, _var);
	//UnaryEval<Op> visitor(_var);
	//return v.getArg()->visit(visitor);
      }

      template<typename Op>
      Expr apply(const BinaryOp<Expr, Op, Expr>& op) {
	//DoubleDispatch1<DerivativeRT, Op> visitor(op.getRHS(), *this);
	//return op.getLHS()->visit(visitor);
	return derivative(op,_var);
      }
      
      VarRT _var;
    };
  }
  
  template<class Var>
  Expr derivative(const Expr& f, const Var v) {
    detail::DerivativeRT visitor(v);
    return f->visit(visitor);
  }

  namespace detail {
    struct IsConstantVisitor: public VisitorHelper<IsConstantVisitor> {
      template<class T>
      Expr apply(const T& a) {
	_value = IsConstant<T>::value;
	return Expr();
      }

      template<class Op>
      Expr apply(const UnaryOp<Expr,Op>& a) {
	//If the argument is constant, then so is this
	a.getArg()->visit(*this);
	return Expr();
      }
      
      template<class Op>
      Expr apply(const BinaryOp<Expr,Op,Expr>& o) {
	//If the two arguments are constant, then so is this.
	o.getLHS()->visit(*this);
	//If the LHS is constant, then check the RHS
	if (_value) o.getRHS()->visit(*this);
	return Expr();
      }
      
      bool _value = false;
    };    
  }
  
  inline bool is_constant(const Expr& a) {
    detail::IsConstantVisitor visitor;
    a->visit(visitor);
    return visitor._value;
  }

  namespace detail {
    struct FastSubRT : VisitorHelper<FastSubRT> {
      FastSubRT(VarRT var, double replacement): _var(var), _replacement(replacement) {}
      
      //By default, throw an exception!
      template<class T>
      Expr apply(const T& v) { stator_throw() << "fast_sub cannot operate on this (" << stator::repr(v) << ") expression"; }

      Expr apply(const double& v) {
	_intermediate = v;
	return Expr();
      }
      
      //Variable matching
      Expr apply(const VarRT& v) {
	if (v == _var) {
	  _intermediate = _replacement;
	  return Expr();
	}
	stator_throw() << "Unexpected variable " << stator::repr(v) << " for fast_sub";
      }
      
      template<typename Op>
      auto apply(const UnaryOp<Expr, Op>& op) -> decltype(double(Op::apply(0.0)), Expr()) {
	op.getArg()->visit(*this);
	_intermediate = Op::apply(_intermediate);
	return Expr();
      }

      template<typename Op>
      Expr apply(const BinaryOp<Expr, Op, Expr>& op) {
	op.getLHS()->visit(*this);
	double lval = _intermediate;
	op.getRHS()->visit(*this);
	_intermediate = Op::apply(lval, _intermediate);
	return Expr();
      }
      
      VarRT _var;
      double _replacement;
      double _intermediate;
    };
  }
  
  template<class Var>
  double fast_sub(const Expr& f, const Relation<Var, double>& rel) {
    detail::FastSubRT visitor(rel._var, rel._val);
    f->visit(visitor);
    return visitor._intermediate;
  }

  template<class RetType>
  RetType RTBase::visit(detail::VisitorInterface<RetType>& c) const {
    switch (_type_idx) {
    case detail::RT_type_index<ConstantRT<double>>::value:                     return c.visit(static_cast<const ConstantRT<double>&>(*this).get());
    case detail::RT_type_index<VarRT>::value:                                  return c.visit(static_cast<const VarRT&>(*this));
    case detail::RT_type_index<UnaryOp<Expr, detail::Sine>>::value:            return c.visit(static_cast<const UnaryOp<Expr, detail::Sine>&>(*this));
    case detail::RT_type_index<UnaryOp<Expr, detail::Cosine>>::value:          return c.visit(static_cast<const UnaryOp<Expr, detail::Cosine>&>(*this));
    case detail::RT_type_index<UnaryOp<Expr, detail::Log>>::value:             return c.visit(static_cast<const UnaryOp<Expr, detail::Log>&>(*this));
    case detail::RT_type_index<UnaryOp<Expr, detail::Exp>>::value:             return c.visit(static_cast<const UnaryOp<Expr, detail::Exp>&>(*this));
    case detail::RT_type_index<UnaryOp<Expr, detail::Absolute>>::value:        return c.visit(static_cast<const UnaryOp<Expr, detail::Absolute>&>(*this));
    case detail::RT_type_index<UnaryOp<Expr, detail::Arbsign>>::value:         return c.visit(static_cast<const UnaryOp<Expr, detail::Arbsign>&>(*this));
    case detail::RT_type_index<BinaryOp<Expr, detail::Add, Expr>>::value:      return c.visit(static_cast<const BinaryOp<Expr, detail::Add, Expr>&>(*this));
    case detail::RT_type_index<BinaryOp<Expr, detail::Subtract, Expr>>::value: return c.visit(static_cast<const BinaryOp<Expr, detail::Subtract, Expr>&>(*this));
    case detail::RT_type_index<BinaryOp<Expr, detail::Multiply, Expr>>::value: return c.visit(static_cast<const BinaryOp<Expr, detail::Multiply, Expr>&>(*this));
    case detail::RT_type_index<BinaryOp<Expr, detail::Divide, Expr>>::value:   return c.visit(static_cast<const BinaryOp<Expr, detail::Divide, Expr>&>(*this));
    case detail::RT_type_index<BinaryOp<Expr, detail::Power, Expr>>::value:    return c.visit(static_cast<const BinaryOp<Expr, detail::Power, Expr>&>(*this));
    default: stator_throw() << "Unhandled type index for the visitor";
    }
  }
}
