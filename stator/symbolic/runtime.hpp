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
#include <vector>
#include <unordered_map>
#include <functional>

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

/*
  Runtime philosophy.
  
  Compile-time types do not automatically convert to runtime
  expressions. This makes sure that the conversion to runtime is
  deliberate. This is not only for performance, its a key development
  choice too, as we can then provide specialised implementations
  without worrying they'll loop back to the generic runtime
  implmentation infinitely. Indeed, the runtime interface SHOULD
  simply resolve to the compile-time functions by determining types
  and calling the specialised versions.

  Runtime types MUST be held in a shared_ptr, as expressions may reuse
  parts of another for speed (for example the derivative of f*g can
  reuse f and g in its result). This means they cannot be constructed,
  only ::create()ed. This also allows some tricks regarding varying
  the returned type depending on arguments.

  Types generally have to be immutable then to allow expression reuse,
  so we might need to revist this for List and Dict types later.
  
  Everything is caught by reference for speed, but must eventually be
  returned inside an Expr to ensure lifetimes are long enough.
  
 */

namespace sym {
  class RTBase;
  struct Expr;

  template<class Config = DefaultReprConfig> std::string repr(const sym::RTBase&);
  template<class Config = DefaultReprConfig> std::string repr(const sym::Expr&);
  template<class Op> struct UnaryOp<Expr, Op>;
  template<class Op> struct BinaryOp<Expr, Op, Expr>;
  template<class T>  class ConstantRT;
  template<> struct Var<nullptr>;
  typedef Var<nullptr> VarRT;
  template<> class Array<Expr, RowMajorAddressing<-1u, -1u>>;

  class Dict;
  
  namespace detail {
    template<class RetType> struct VisitorInterface;
  }

  /*! \brief Abstract interface class for all runtime symbolic
      classes. 
      
      This class defines the interface for all classes/symbols which
      can be held by \ref Expr. Most actual functionality is
      implemented using the \ref VisitorInterface via \ref visit.
  */
  class RTBase : public std::enable_shared_from_this<RTBase> {
  public:
    inline RTBase(int idx): _type_idx(idx) {}
    
    inline virtual ~RTBase() {}

    /*! \brief Comparison operator against Expr. */
    template<class T>
    bool operator==(const T& rhs) const {
      return compare(Expr(rhs));
    }

    /*! \brief Starts a comparison.
      
      The only reason this exists is that the operator== is templated
      to stop implicit conversions, and templated functions cannot be
      virtual, so we handle the first dispatch here.
     */
    virtual bool compare(const Expr& rhs) const = 0;
    
    const int _type_idx;

    template<class RetType>
    RetType visit(detail::VisitorInterface<RetType>& c) const;
  };

  /*! \brief The generic holder/smart pointer for a runtime Abstract
    Syntax Tree (AST) (expression).
    
    This class is simply a smart pointer, with specialised
    constructors to allow it to convert compile-time expressions (and
    formula strings) into runtime forms. It also inherits from
    SymbolicOperator and can be used in compile-time expressions.
  */
  struct Expr : public shared_ptr<const RTBase>, public SymbolicOperator<Expr> {
    typedef shared_ptr<const RTBase> Base;

    inline Expr() {}
    inline Expr(const std::shared_ptr<RTBase>& p) : Base(p) {}

    template<class T, typename = typename std::enable_if<std::is_base_of<RTBase, T>::value>::type>
    inline Expr(const std::shared_ptr<T>& p) : Base(p) {}
    
    Expr(const char*);
    Expr(const std::string&);
    Expr(const RTBase&);

    explicit Expr(const double&);
    
    template<std::intmax_t Num, std::intmax_t Denom>
    explicit Expr(const C<Num, Denom>& c);

    template<conststr N>
    explicit Expr(const Var<N>& v);

    explicit Expr(const VarRT& v);
    
    template<class Op, class Arg_t>
    explicit Expr(const UnaryOp<Arg_t, Op>&);

    template<class LHS_t, class Op, class RHS_t>
    explicit Expr(const BinaryOp<LHS_t, Op, RHS_t>&);

    inline
    Expr(const detail::NoIdentity&) { stator_throw() << "This should never be called as NoIdentity is not a usable type";}

    explicit Expr(const ArrayRT&);
    explicit Expr(const Dict&);

    /*! \brief Expression comparison operator.
      
      Specialisation of comparison is done in the derived classes of
      RTBase.
     */
    template<class T>
    bool operator==(const T& rhs) const { return (*(*this)) == rhs; }

    
    /*! \brief Shortcut implementation for NoIdentity. */
    constexpr bool operator==(const detail::NoIdentity& ) { return false; }
    
    bool operator!=(const Expr& o) const { return !(*this == o); }
    
    explicit operator bool() const { return Base::operator bool(); }

    template<class T>
    Expr& operator=(const T& v) {
      Base::operator=(Expr(v));
      return *this;
    }
    
    template<class T> const T& as() const;
  };
}

namespace stator {
  template<class T, typename = typename std::enable_if<std::is_base_of<sym::RTBase, T>::value>::type>
  auto store(const std::shared_ptr<T>& val) {
    return sym::Expr(val);
  }

  template<class T, typename = typename std::enable_if<std::is_base_of<sym::RTBase, T>::value>::type>
  auto store(const T& val) {
    return sym::Expr(val);
  }
}

namespace sym {
  using stator::store;
  
  namespace detail {    
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
      virtual RetType visit(const BinaryOp<Expr, detail::Equality, Expr>& ) = 0;
      virtual RetType visit(const BinaryOp<Expr, detail::ArrayAccess, Expr>& ) = 0;
      virtual RetType visit(const BinaryOp<Expr, detail::Units, Expr>& ) = 0;
      virtual RetType visit(const ArrayRT& ) = 0;
      virtual RetType visit(const Dict& ) = 0;
      virtual RetType visit(const UnaryOp<Expr, detail::Negate>& ) = 0;
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
      inline virtual RetType visit(const BinaryOp<Expr, detail::Equality, Expr>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const BinaryOp<Expr, detail::ArrayAccess, Expr>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const BinaryOp<Expr, detail::Units, Expr>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const ArrayRT& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const Dict& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const UnaryOp<Expr, detail::Negate>& x)
      { return static_cast<Derived*>(this)->apply(x); }
    };
  }

  namespace detail {
    /*! \brief Comparison visitor.

      This visitor completes type determinations for operator==
      comparisons between RTBase derived types.
     */
    template<class LHS>
    struct ComparisonVisitor : public sym::detail::VisitorHelper<ComparisonVisitor<LHS>, bool> {
      ComparisonVisitor(const LHS& l): _l(l) {}
      
      template<class RHS> bool apply(const RHS& r) {
	return _l == r;
      }

      const LHS& _l;
    };
  }
  
  /*! \brief CRTP helper base class which implements some of the
    common boilerplate code for runtime symbolic types.
  */
  template<class Derived>
  class RTBaseHelper : public RTBase, public SymbolicOperator<Derived> {
  public:
    RTBaseHelper(): RTBase(detail::Type_index<Derived>::value) {}
        
    virtual bool compare(const Expr& rhs) const {
      const Derived& lhs(*static_cast<const Derived*>(this));
      detail::ComparisonVisitor<Derived> visitor(lhs);
      return rhs->visit(visitor);
    }
  };  

  namespace detail {
    /*! \brief Binding power visitor for sym::detail::BP(const Expr&). */
    struct BPVisitor : public sym::detail::VisitorHelper<BPVisitor, std::pair<int, int>> {
      template<class T> std::pair<int, int> apply(const T& rhs) {
	return BP(rhs);
      }
    };
  }
  
  /*! \brief Returns the binding powers (precedence) of operators
    (specialisation for Expr).
  */
  inline std::pair<int, int> BP(const Expr& v) {
    detail::BPVisitor vis;
     return v->visit(vis);
  }
}

#include <stator/symbolic/binary_ops_rt.hpp>
#include <stator/symbolic/variable_rt.hpp>

namespace sym {
  namespace detail {
    struct HashRT : VisitorHelper<HashRT, std::size_t> {
      HashRT() {}

      template<class T>
      std::size_t apply(const T& v) {
	std::hash<typename std::decay<T>::type> hasher;
	return hasher(v);
      }
    };
  }
}

namespace std
{
  template<> struct hash<sym::Expr>
  {
    std::size_t operator()(sym::Expr const& f) const noexcept
    {
      sym::detail::HashRT vis;
      return f->visit(vis);
    }
  };
}

#include <stator/symbolic/constants_rt.hpp>
#include <stator/symbolic/unary_ops_rt.hpp>
#include <stator/symbolic/array_rt.hpp>
#include <stator/symbolic/dict_rt.hpp>

namespace sym {

  inline Expr::Expr(const RTBase& v) : Base(v.shared_from_this()) {}

  inline Expr::Expr(const double& v) : Base(ConstantRT<double>::create(v)) {}
  
  template<std::intmax_t Num, std::intmax_t Denom>
  Expr::Expr(const C<Num, Denom>& c) : Base(ConstantRT<double>::create(double(Num) / Denom)) {}
  
  template<class Op, class Arg_t>
  Expr::Expr(const UnaryOp<Arg_t, Op>& op) : Base(UnaryOp<Expr, Op>::create(Expr(op._arg))) {}

  template<class LHS_t, class Op, class RHS_t>
  Expr::Expr(const BinaryOp<LHS_t, Op, RHS_t>& op) : Base(BinaryOp<Expr, Op, Expr>::create(Expr(op._l), Expr(op._r))) {}
  
  template<conststr N1>
  Expr::Expr(const Var<N1>& v) : Base(VarRT::create(v)) {}
  
  inline Expr::Expr(const VarRT& v) : Base(v.shared_from_this()) {}

  Expr::Expr(const ArrayRT& v) : Base(v.shared_from_this()) {}
  Expr::Expr(const Dict& v) : Base(v.shared_from_this()) {}
  
  template<class T>
  const T& Expr::as() const {
    const T* ptr = dynamic_cast<const T*>(Base::get());
    if (!ptr)
      stator_throw() << "Invalid as<>(), expression is as follows:" << *this;

    return *ptr;
  }

  
  template<>
  const double& Expr::as<double>() const {
    const ConstantRT<double>* ptr = dynamic_cast<const ConstantRT<double>*>(Base::get());
    if (!ptr)
      stator_throw() << "Invalid as<>(), expression is as follows:" << *this;

    return ptr->get();
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
      /*! 
	\brief Default action is to return the original expression. 
      */
      template<class T>
      Expr apply(const T& v) {
	return Expr();
      }

      Expr apply(const ArrayRT& v) {
	return simplify(v);
      }

      Expr apply(const Dict& v) {
	return simplify(v);
      }
      
      //////// Handling of BinaryOp simplification

      /*! \brief Simplify a BinaryOp expression. 
       */
      template<typename Op>
      Expr apply(const BinaryOp<Expr, Op, Expr>& op) {
	//First we try to simplify the LHS
	Expr l = op.getLHS()->visit(*this);
	bool lchanged = !!l;
	if (!l) l = op._l;

	//Now try to simplify the RHS
	Expr r = op.getRHS()->visit(*this);
	bool rchanged = !!r;
	if (!r) r = op._r;

	//Now check if either side is an identity
	if (l == typename Op::left_zero())
	  return Expr(typename Op::left_zero());
	if (l == typename Op::left_identity())
	  return r;
	if (r == typename Op::right_zero())
	  return Expr(typename Op::right_zero());
	if (r == typename Op::right_identity())
	  return Expr(l);
	
	//Try direct simplification via application
	DoubleDispatch1<SimplifyRT, Op> visitor(r, *this);
	Expr ret = l->visit(visitor);
	if (ret)
	  return ret;

	if (lchanged || rchanged)
	  return Expr(BinaryOp<Expr, Op, Expr>::create(l, r));

	return Expr();
      }
      
      template<class T1, class T2, class Op>
      Expr dd_visit(const T1& l, const T2& r, Op) {
	      if (IsSymbolic<decltype(store(Op::apply(l, r)))>::value)
	        return Expr();
	      return Expr(store(Op::apply(l, r)));
      }

      template<class T1, class T2>
      Expr dd_visit(const T1& l, const T2& r, detail::ArrayAccess) {
        stator_throw() << "No valid array operation here!";
      }

      Expr dd_visit(const ArrayRT& l, const double& r, detail::ArrayAccess) {
	      if (IsSymbolic<decltype(store(detail::ArrayAccess::apply(l, r)))>::value)
	        return Expr();
	      return Expr(store(detail::ArrayAccess::apply(l, r)));
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
	  return Expr(l * static_cast<const ConstantRT<double>*>(r.getLHS().get())->get() * r.getRHS());

	if (dynamic_cast<const ConstantRT<double>*>(r.getRHS().get()))
	  return Expr(l * static_cast<const ConstantRT<double>*>(r.getRHS().get())->get() * r.getLHS());
	
	return Expr();
      }

      Expr dd_visit(const BinaryOp<Expr, detail::Multiply, Expr>& r, const double& l, detail::Multiply) {
	if (dynamic_cast<const ConstantRT<double>*>(r.getLHS().get()))
	  return Expr(l * static_cast<const ConstantRT<double>*>(r.getLHS().get())->get() * r.getRHS());

	if (dynamic_cast<const ConstantRT<double>*>(r.getRHS().get()))
	  return Expr(l * static_cast<const ConstantRT<double>*>(r.getRHS().get())->get() * r.getLHS());
	
	return Expr();
      }
      
      //////// Handling of UnaryOp simplification
      template<class Op>
      struct UnaryEval : VisitorHelper<UnaryEval<Op> > {
	Expr apply(const double& arg) { return Expr(Op::apply(arg)); }
	template<class T> Expr apply(const T& arg) { return Expr(); }
      };

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
	  return Expr(UnaryOp<Expr, Op>::create(arg));
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
      SubstituteRT(const VarRT& var, Expr replacement): _var(var), _replacement(replacement) {}
      
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

      //Variable matching
      Expr apply(const ArrayRT& v) {
	auto ret_ptr = ArrayRT::create();
	auto& ret = *ret_ptr;
	bool _replaced = false; //We only start regenerating the list when needed

	auto iptr = v.begin();
	auto jptr = ret.begin();
	
	while (iptr != v.end()) {
	  Expr t = (*iptr)->visit(*this);
	  
	  if (bool(t) && (!_replaced)) {
	    //We're starting replacements, so copy everything skipped over so far
	    ret.resize(v.getDimensions());
	    jptr = ret.begin();
	    auto iiptr = v.begin();
	    while (iiptr != iptr) {
	      *jptr = *iiptr;
	      ++iiptr; ++jptr;
	    }
	    _replaced = true;
	  }

	  if (_replaced) {
	    //continue the copy where required
	    *jptr = (t) ? t : *iptr;
	    ++jptr;
	  }

	  ++iptr;
	}
	
	return (_replaced) ? ret_ptr : Expr();
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
      
      const VarRT& _var;
      Expr _replacement;
    };
  }
  
  namespace detail {
    struct SubstituteDictRT : VisitorHelper<SubstituteDictRT> {
      SubstituteDictRT(const Dict& replacement):
	_replacement(replacement)
      {}
      
      //By default, just return an empty Expr, and let the helper function return the original expression
      template<class T>
      Expr apply(const T& v)
      {
	return Expr();
      }

      //Variable matching
      Expr apply(const VarRT& v) {
	auto it = _replacement.find(v);
	if (it != _replacement.end()) {
	  return it->second;
	}
	return Expr();
      }

      //Variable matching
      Expr apply(const ArrayRT& v) {
	auto ret_ptr = ArrayRT::create();
	auto& ret = *ret_ptr;
	
	bool _replaced = false; //We only start regenerating the list when needed
	auto iptr = v.begin();
	auto jptr = ret.begin();
	
	while (iptr != v.end()) {
	  Expr t = (*iptr)->visit(*this);
	  
	  if (bool(t) && ! _replaced) {
	    //We're starting replacements, so copy everything skipped over so far
	    ret.resize(v.getDimensions());
	    jptr = ret.begin();
	    auto iiptr = v.begin();
	    while (iiptr != iptr) {
	      *jptr = *iiptr;
	      ++iiptr; ++jptr;
	    }
	    _replaced = true;
	  }

	  if (_replaced) {
	    //continue the copy where required
	    *jptr = (t) ? t : *iptr;
	    ++jptr;
	  }

	  ++iptr;
	}
	
	return (_replaced) ? ret_ptr : Expr();
      }
      
      template<typename Op>
      Expr apply(const UnaryOp<Expr, Op>& op) {
	Expr arg = op.getArg()->visit(*this);
	return arg ? Expr(Op::apply(arg)) : Expr();
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
      
      const Dict&  _replacement;
    };

  }

  template<conststr N, class T>
  Expr sub(const Expr& f, const BinaryOp<Var<N>, detail::Equality, T>& op) {
    auto v_ptr = VarRT::create(op._l);
    detail::SubstituteRT visitor(*v_ptr, Expr(op._r));
    Expr result = f->visit(visitor);
    return (result) ?  result : f;
  }
  
  Expr sub(const Expr& f, const Dict& rep) {
    detail::SubstituteDictRT visitor(rep);
    Expr result = f->visit(visitor);
    return (result) ?  result : f;
  }

  namespace detail {
    struct SubstituteExprRT : VisitorHelper<SubstituteExprRT> {
      SubstituteExprRT(const Expr& f): _f(f) {}
      
      //By default, just return an empty Expr, and let the helper function return the original expression
      template<class T>
      Expr apply(const T& v)
      {
	stator_throw() << "No substitution process available for " << v << "\n Needs to be a Equality or a Dict.";
      }

      Expr apply(const Dict& d) {
	return sub(_f, d);
      }

      Expr apply(const EqualityOp<Expr, Expr>& op) {
	const VarRT& v = op._l.as<VarRT>();
	detail::SubstituteRT visitor(v, op._r);
	Expr result = _f->visit(visitor);
	return (result) ?  result : _f;
      }
	
      const Expr& _f;
    };
  }
  
  Expr sub(const Expr& f, const Expr& rep) {
    detail::SubstituteExprRT visitor(f);
    Expr result = rep->visit(visitor);
    return (result) ?  result : f;
  }

  
  namespace detail {
    struct DerivativeRT : VisitorHelper<DerivativeRT> {
      DerivativeRT(const VarRT& var): _var(var) {}

      template<class Op>
      Expr apply(const Op& v) {
      	return Expr(derivative(v, _var));
      }

      const VarRT& _var;
    };
  }
  
  /*! \brief Runtime derivative where a Var<> type is known for the second argument.
   */
  Expr derivative(const Expr& f, const VarRT& v) {
    detail::DerivativeRT visitor(v);
    return f->visit(visitor);
  }

  template<conststr N>
  Expr derivative(const Expr& f, const Var<N>& v) {
    auto v_ptr = VarRT::create(v);
    return derivative(f, *v_ptr);
  }
  
  /*! \brief Runtime derivative where variable is an Expr.
    
    Confirm its a variable and hand it over to the templated implementation.
   */
  Expr derivative(const Expr& f, const Expr& v) {
    return derivative(f, v.as<VarRT>());
  }
  
  namespace detail {
    struct FastSubRT : VisitorHelper<FastSubRT> {
      FastSubRT(const VarRT& var, double replacement): _var(var), _replacement(replacement) {}
      
      //By default, throw an exception!
      template<class T>
      Expr apply(const T& v) { stator_throw() << "fast_sub cannot operate on this (" << repr(v) << ") expression"; }

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
	stator_throw() << "Unexpected variable " << repr(v) << " for fast_sub";
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

      Expr apply(const BinaryOp<Expr, detail::Equality, Expr>& op)
      { stator_throw() << "fast_sub cannot operate on this (" << repr(op) << ") expression"; }

      Expr apply(const BinaryOp<Expr, detail::ArrayAccess, Expr>& op)
      { stator_throw() << "fast_sub cannot operate on this (" << repr(op) << ") expression"; }

      Expr apply(const BinaryOp<Expr, detail::Units, Expr>& op)
      { stator_throw() << "fast_sub cannot operate on this (" << repr(op) << ") expression"; }
      
      const VarRT& _var;
      double _replacement;
      double _intermediate;
    };
  }
  
  template<class Var>
  double fast_sub(const Expr& f, const EqualityOp<Var, double>& rel) {
    detail::FastSubRT visitor(rel._var, rel._val);
    f->visit(visitor);
    return visitor._intermediate;
  }

  template<class RetType>
  RetType RTBase::visit(detail::VisitorInterface<RetType>& c) const {
    switch (_type_idx) {
    case detail::Type_index<ConstantRT<double>>::value:                     return c.visit(static_cast<const ConstantRT<double>&>(*this).get());
    case detail::Type_index<VarRT>::value:                                  return c.visit(static_cast<const VarRT&>(*this));
    case detail::Type_index<UnaryOp<Expr, detail::Sine>>::value:            return c.visit(static_cast<const UnaryOp<Expr, detail::Sine>&>(*this));
    case detail::Type_index<UnaryOp<Expr, detail::Cosine>>::value:          return c.visit(static_cast<const UnaryOp<Expr, detail::Cosine>&>(*this));
    case detail::Type_index<UnaryOp<Expr, detail::Log>>::value:             return c.visit(static_cast<const UnaryOp<Expr, detail::Log>&>(*this));
    case detail::Type_index<UnaryOp<Expr, detail::Exp>>::value:             return c.visit(static_cast<const UnaryOp<Expr, detail::Exp>&>(*this));
    case detail::Type_index<UnaryOp<Expr, detail::Absolute>>::value:        return c.visit(static_cast<const UnaryOp<Expr, detail::Absolute>&>(*this));
    case detail::Type_index<UnaryOp<Expr, detail::Arbsign>>::value:         return c.visit(static_cast<const UnaryOp<Expr, detail::Arbsign>&>(*this));
    case detail::Type_index<BinaryOp<Expr, detail::Add, Expr>>::value:      return c.visit(static_cast<const BinaryOp<Expr, detail::Add, Expr>&>(*this));
    case detail::Type_index<BinaryOp<Expr, detail::Subtract, Expr>>::value: return c.visit(static_cast<const BinaryOp<Expr, detail::Subtract, Expr>&>(*this));
    case detail::Type_index<BinaryOp<Expr, detail::Multiply, Expr>>::value: return c.visit(static_cast<const BinaryOp<Expr, detail::Multiply, Expr>&>(*this));
    case detail::Type_index<BinaryOp<Expr, detail::Divide, Expr>>::value:   return c.visit(static_cast<const BinaryOp<Expr, detail::Divide, Expr>&>(*this));
    case detail::Type_index<BinaryOp<Expr, detail::Power, Expr>>::value:    return c.visit(static_cast<const BinaryOp<Expr, detail::Power, Expr>&>(*this));
    case detail::Type_index<BinaryOp<Expr, detail::Equality, Expr>>::value: return c.visit(static_cast<const BinaryOp<Expr, detail::Equality, Expr>&>(*this));
    case detail::Type_index<BinaryOp<Expr, detail::ArrayAccess, Expr>>::value:    return c.visit(static_cast<const BinaryOp<Expr, detail::ArrayAccess, Expr>&>(*this));
    case detail::Type_index<BinaryOp<Expr, detail::Units, Expr>>::value:    return c.visit(static_cast<const BinaryOp<Expr, detail::Units, Expr>&>(*this));
    case detail::Type_index<ArrayRT>::value:                                return c.visit(static_cast<const ArrayRT&>(*this));
    case detail::Type_index<Dict>::value:                                   return c.visit(static_cast<const Dict&>(*this));
    case detail::Type_index<UnaryOp<Expr, detail::Negate>>::value:          return c.visit(static_cast<const UnaryOp<Expr, detail::Negate>&>(*this));
    default: stator_throw() << "Unhandled type index (" << _type_idx << ") for the visitor";
    }
  }
  
  template<class Config = DefaultReprConfig>
  inline std::string repr(const sym::ConstantRT<double>& f)
  {
    if (Config::Debug_output)
      return "ConstantRT<double>("+repr<Config>(f.get())+")";
    return repr<Config>(f.get());
  }
  
  template<class Config = DefaultReprConfig, class T, typename = typename std::enable_if<std::is_base_of<Eigen::EigenBase<T>, T>::value>::type>
  std::string repr(const T& val) {
    std::ostringstream os;
    if ((val.cols() == 1) && (val.rows()==1))
      os << repr<Config>(val(0,0));
    else if (val.cols() == 1) {
      os << "{ ";
      for (int i(0); i < val.rows(); ++i)
	os << repr<Config>(val(i, 0)) << " ";
      os << "}^T";
    } else {
      os << "{ ";
      for (int i(0); i < val.cols(); ++i) {
	os << "{ ";
	for (int j(0); j < val.rows(); ++j)
	  os << repr<Config>(val(i, j)) << " ";
	os << "} ";
      }
      os << "}";
    }
    return os.str();
  }
  
  /*! \} */

  namespace detail {
    template<class Config>
    struct ReprVisitor : public sym::detail::VisitorHelper<ReprVisitor<Config>, std::string> {
      template<class T> std::string apply(const T& rhs) {
	return repr<Config>(rhs);
      }
    };
  }

  template<class Config>
  std::string repr(const sym::RTBase& b) {
    detail::ReprVisitor<Config> visitor;
    return b.visit(visitor);
  }

  /*! \brief Give a representation of an Expr. 
    
    The visitor pattern is used to call the specialised repr implementations for each runtime type supported. 
   */
  template<class Config>
  std::string repr(const sym::Expr& b) {
    return repr<Config>(*b);
  }
}

#include <stator/symbolic/parser.hpp>
