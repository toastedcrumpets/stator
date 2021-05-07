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

namespace sym {
  class RTBase;
  struct Expr;

  template<class Config = DefaultReprConfig> std::string repr(const sym::RTBase&);
  template<class Config = DefaultReprConfig> std::string repr(const sym::Expr&);
  template<class Op> struct UnaryOp<Expr, Op>;
  template<class Op> struct BinaryOp<Expr, Op, Expr>;
  template<class T>  class ConstantRT;
  typedef Var<nullptr> VarRT;
  class List;
  class Dict;

  namespace detail {
    template<class T>
    struct RT_type_index {
      static_assert(sizeof(T) == -1, "Missing index for runtime type");
    };

    template<> struct RT_type_index<ConstantRT<double>>                     { static const int value = 0;  };
    template<> struct RT_type_index<VarRT>                                  { static const int value = 1;  };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Sine>>            { static const int value = 2;  };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Cosine>>          { static const int value = 3;  };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Log>>             { static const int value = 4;  };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Exp>>             { static const int value = 5;  };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Absolute>>        { static const int value = 6;  };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Arbsign>>         { static const int value = 7;  };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Add, Expr>>      { static const int value = 8;  };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Subtract, Expr>> { static const int value = 9;  };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Multiply, Expr>> { static const int value = 10; };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Divide, Expr>>   { static const int value = 11; };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Power, Expr>>    { static const int value = 12; };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Equality, Expr>> { static const int value = 13; };
    template<> struct RT_type_index<BinaryOp<Expr, detail::Array, Expr>>    { static const int value = 14; };
    template<> struct RT_type_index<List>                                   { static const int value = 15; };
    template<> struct RT_type_index<Dict>                                   { static const int value = 16; };
    template<> struct RT_type_index<UnaryOp<Expr, detail::Negate>>          { static const int value = 17;  };
    
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
      virtual RetType visit(const BinaryOp<Expr, detail::Array, Expr>& ) = 0;
      virtual RetType visit(const List& ) = 0;
      virtual RetType visit(const Dict& ) = 0;
      virtual RetType visit(const UnaryOp<Expr, detail::Negate>& ) = 0;
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

    /*! \brief Comparison operator against Expr
      
      The template wizzardry is to make sure this operator is only
      used for comparison against Expr types WITHOUT implicit
      conversion. This is because derived types should implement all
      other comparison operators which will be called by the visitor
      inside compare.
     */
    template<class T, typename = typename std::enable_if<std::is_same<T, Expr>::value>::type>
    bool operator==(const T& rhs) const {
      return compare(rhs);
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

    template<conststr N>
    Expr(const Var<N> v);
    
    template<class Op, class Arg_t>
    Expr(const UnaryOp<Arg_t, Op>&);

    template<class LHS_t, class Op, class RHS_t>
    Expr(const BinaryOp<LHS_t, Op, RHS_t>&);

    inline
    Expr(const detail::NoIdentity&) { stator_throw() << "This should never be called as NoIdentity is not a usable type";}

    Expr(const List&);
    Expr(const Dict&);

    /*! \brief Expression comparison operator.
      
      This converts all types to an Expr ready for
      comparison. Specialisation of comparison is done in the derived
      classes of RTBase.
     */
    bool operator==(const Expr& rhs) const { return (*(*this)) == rhs; }

    /*! \brief Shortcut implementation for NoIdentity. */
    constexpr bool operator==(const detail::NoIdentity& ) { return false; }
    
    bool operator!=(const Expr& o) const { return !(*this == o); }
    
    explicit operator bool() const { return Base::operator bool(); }
        
    template<class T> const T& as() const;
  };
  
  namespace detail { 

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
      inline virtual RetType visit(const BinaryOp<Expr, detail::Array, Expr>& x)
      { return static_cast<Derived*>(this)->apply(x); }
      inline virtual RetType visit(const List& x)
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
    struct ComparisonVisitor : public sym::detail::VisitorHelper<ComparisonVisitor<LHS> > {
      ComparisonVisitor(const LHS& l): _l(l) {}
      
      template<class RHS> sym::Expr apply(const RHS& r) {
	_result = _l == r;
	return Expr();
      }

      const LHS& _l;
      bool _result;
    };
  }
  
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
    
    virtual bool compare(const Expr& rhs) const {
      const Derived& lhs(*static_cast<const Derived*>(this));
      detail::ComparisonVisitor<Derived> visitor(lhs);
      rhs->visit(visitor);
      return visitor._result;
    }
  };  

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
  }

  namespace detail {
    /*! \brief Binding power visitor for sym::detail::BP(const Expr&). */
    struct BPVisitor : public sym::detail::VisitorHelper<BPVisitor> {
      template<class T> sym::Expr apply(const T& rhs) {
	_BP = BP(rhs);
	return Expr();
      }
      
      std::pair<int, int> _BP;
    };
  }
  
  /*! \brief Returns the binding powers (precedence) of operators
    (specialisation for Expr).
  */
  inline std::pair<int, int> BP(const Expr& v) {
    detail::BPVisitor vis;
    v->visit(vis);
    return vis._BP;
  }

  
  /*! \brief Specialisation of Var for runtime variables.*/
  template<>
  class Var<nullptr>: public RTBaseHelper<Var<nullptr> >, public Dynamic {
  public:
    inline Var(const std::string name="x") :
      _name(name)
    {}

    template<conststr N>
    Var(const Var<N> v):
      _name(v.getName())
    {}

    inline bool operator==(const VarRT& o) const {
      return _name == o._name;
    }

    template<class RHS>
    constexpr bool operator==(const RHS&) const {
      return false;
    }

    template<class Arg>
    auto operator=(const Arg& a) const -> STATOR_AUTORETURN(equal(*this, a));
    
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
  
}

namespace std
{
  template<> struct hash<sym::VarRT>
  {
    std::size_t operator()(sym::VarRT const& v) const noexcept
    {
      return std::hash<std::string>{}(v.getName());
    }
  };
}
  
namespace sym { 
  
  /*! \brief Specialisation of Var for runtime variables.*/
  
  /*! \brief Determine the derivative of a variable by another variable.

    If the variable is NOT the variable in which a derivative is
    being taken, then this overload should be selected to return
    Null.
  */
  Expr derivative(VarRT v1, VarRT v2) {
    return Expr(v1 == v2);
  }

  
  template<typename T>
  class ConstantRT : public RTBaseHelper<ConstantRT<T> > {
  public:
    ConstantRT(const T& v): _val(v) {}

    template<typename T2>
    bool operator==(const ConstantRT<T2>& o) const {
      return _val == o._val;
    }

    template<class RHS>
    bool operator==(const RHS& o) const {
      if constexpr(detail::IsConstant<RHS>::value)
	 return _val == o;
      else
	 return false;
    }
    
    const T& get() const { return _val; }
    
  private:
    T _val;
  };
  

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

    template<class RHS>
    constexpr bool operator==(const RHS&) const {
      return false;
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

    template<class RHS>
    constexpr bool operator==(const RHS&) const {
      return false;
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

  class List: public RTBaseHelper<List>  {
    typedef std::vector<Expr> Store;

    public:
    List() {}
    
    typedef typename Store::iterator iterator;
    typedef typename Store::const_iterator const_iterator;

    iterator begin() {return _store.begin();}
    const_iterator begin() const {return _store.begin();}
    const_iterator cbegin() const {return _store.cbegin();}
    iterator end() {return _store.end();}
    const_iterator end() const {return _store.end();}
    const_iterator cend() const {return _store.cend();}

    typedef Store::reference reference;
    typedef Store::const_reference const_reference;
    reference& operator[](Store::size_type pos) { return _store[pos]; }
    const_reference operator[](Store::size_type pos) const { return _store[pos]; }

    void reserve(Store::size_type cap) { _store.reserve(cap); }
    void resize(Store::size_type cap) { _store.resize(cap); }
    void push_back(const Expr& value) { _store.push_back(value); }
    Store::size_type size() const noexcept { return _store.size(); } 
    bool empty() const noexcept { return _store.empty(); }

    bool operator==(const List& o) const {
      return _store == o._store;
    }

    template<class RHS>
    constexpr bool operator==(const RHS&) const {
      return false;
    }
    
  private:
    Store _store;
  };

  template<typename Var>
  List derivative(const List& in, Var x) {
    List out;
    out.resize(in.size());
    for (size_t idx(0); idx < in.size(); ++idx)
      out[idx] = derivative(in[idx], x);
    return out;
  }
  
  List operator+(const List& l, const List& r) {
    if (l.size() != r.size())
      stator_throw() << "Mismatched list size for: \n" << l << "\n and\n" << r;
    List out;
    out.resize(l.size());
    
    for (size_t idx(0); idx < l.size(); ++idx)
      out[idx] = l[idx] + r[idx];
    return out;
  }

  class Dict: public RTBaseHelper<Dict> {
    typedef std::unordered_map<VarRT, Expr> Store;

  public:
    Dict() {}
    
    typedef Store::key_type key_type;
    typedef Store::reference reference;
    typedef Store::const_reference const_reference;
    typedef typename Store::iterator iterator;
    typedef typename Store::const_iterator const_iterator;

    iterator begin() {return _store.begin();}
    const_iterator begin() const {return _store.begin();}
    const_iterator cbegin() const {return _store.cbegin();}
    iterator end() {return _store.end();}
    const_iterator end() const {return _store.end();}
    const_iterator cend() const {return _store.cend();}

    iterator find( const key_type& key ) { return _store.find(key); }
    const_iterator find( const key_type& key ) const { return _store.find(key); }

    Expr& operator[](const VarRT& k) { return _store[k]; }
    Expr& at(const VarRT& k) { return _store.at(k); }
    const Expr& at(const VarRT& k) const { return _store.at(k); }

    Store::size_type size() const noexcept { return _store.size(); } 
    bool empty() const noexcept { return _store.empty(); }

    bool operator==(const Dict& o) const {
      return _store == o._store;
    }
    
    template<class RHS>
    constexpr bool operator==(const RHS&) const {
      return false;
    }
    
  private:
    Store _store;
  };

  template<typename Var>
  Dict derivative(const Dict& in, Var x) {
    stator_throw() << "Cannot take derivatives of dictionaries";
  }

  inline Expr::Expr(const RTBase& v) : Base(v.clone()) {}

  inline Expr::Expr(const double& v) : Base(std::make_shared<ConstantRT<double> >(v)) {}
  
  template<std::intmax_t Num, std::intmax_t Denom>
  Expr::Expr(const C<Num, Denom>& c) : Base(std::make_shared<ConstantRT<double> >(double(Num) / Denom)) {}
  
  template<class Op, class Arg_t>
  Expr::Expr(const UnaryOp<Arg_t, Op>& op) : Base(std::make_shared<UnaryOp<Expr, Op> >(op._arg)) {}

  template<class LHS_t, class Op, class RHS_t>
  Expr::Expr(const BinaryOp<LHS_t, Op, RHS_t>& op) : Base(std::make_shared<BinaryOp<Expr, Op, Expr> >(op._l, op._r)) {}
  
  template<conststr N1>
  Expr::Expr(const Var<N1> v) : Base(std::make_shared<VarRT>(v)) {}

  Expr::Expr(const List& v) : Base(std::make_shared<List>(v)) {}

  Expr::Expr(const Dict& v) : Base(std::make_shared<Dict>(v)) {}
  
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

  //Need to forward declare this as its used in the SimplifyRT visitor
  //Can't declare it here though, as it needs the SimplfiyRT implementation.
  List simplify(const List&);
  Dict simplify(const Dict&);
  
  namespace detail {
    struct SimplifyRT : VisitorHelper<SimplifyRT> {      
      /*! 
	\brief Default action is to return the original expression. 
      */
      template<class T>
      Expr apply(const T& v) {
	return Expr();
      }

      Expr apply(const List& v) {
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
      
      //////// Handling of UnaryOp simplification
      template<class Op>
      struct UnaryEval : VisitorHelper<UnaryEval<Op> > {
	Expr apply(const double& arg) { return Op::apply(arg); }
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
	  return Expr(std::make_shared<UnaryOp<Expr, Op> >(arg));
	return Expr();
      }
    };
  }

  inline Expr simplify(const Expr& f) {
    detail::SimplifyRT visitor;
    Expr result = f->visit(visitor);
    return result ? result : f;
  }

  List simplify(const List& in) {
    List out;
    out.resize(in.size());
    for (size_t idx(0); idx < in.size(); ++idx)
      out[idx] = simplify(in[idx]);
    return out;
  }  

  Dict simplify(const Dict& in) {
    Dict out;
    for (const auto& p : in) 
      out[p.first] = simplify(p.second);
    return out;
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

      //Variable matching
      Expr apply(const List& v) {
	List ret;
	
	bool _replaced = false; //We only start regenerating the list when needed
	for (size_t i(0); i < v.size(); ++i) {
	  Expr t = v[i]->visit(*this);
	  
	  if (bool(t) && (!_replaced)) {
	    //We're starting replacements, so copy everything skipped over so far
	    ret.resize(v.size());
	    for (size_t j(0); j < i; ++j)
	      ret[j] = v[j];
	    _replaced = true;
	  }

	  if (_replaced) //continue the copy where required
	    ret[i] = (t) ? t : v[i];
	}
	
	return (_replaced) ? ret : Expr();
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
  Expr sub(const Expr& f, const EqualityOp<Var, Arg>& rel) {
    detail::SubstituteRT visitor(rel._l, rel._r);
    Expr result = f->visit(visitor);
    return (result) ?  result : f;
  }
  
  namespace detail {
    struct SubstituteDictRT : VisitorHelper<SubstituteRT> {
      SubstituteDictRT(const Dict& replacement):  _replacement(replacement) {}
      
      //By default, just return an empty Expr, and let the helper function return the original expression
      template<class T>
      Expr apply(const T& v)
      { return Expr(); }

      //Variable matching
      Expr apply(const VarRT& v) {
	std::cout << "!! Found " << v << std::endl;
	auto it = _replacement.find(v);
	if (it != _replacement.end()) {
	  std::cout << "!! Replacing it with " << it->second << std::endl;
	  return it->second;
	}
	std::cout << "!! Not replacing it " << v << std::endl;
	return Expr();
      }

      //Variable matching
      Expr apply(const List& v) {
	List ret;
	
	bool _replaced = false; //We only start regenerating the list when needed
	for (size_t i(0); i < v.size(); ++i) {
	  Expr t = v[i]->visit(*this);
	  
	  if (bool(t) && ! _replaced) {
	    //We're starting replacements, so copy everything skipped over so far
	    ret.resize(v.size());
	    for (size_t j(0); j < i; ++j)
	      ret[j] = v[j];
	    _replaced = true;
	  }

	  if (_replaced) //continue the copy where required
	    ret[i] = (t) ? t : v[i];
	}
	
	return (_replaced) ? ret : Expr();
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

  Expr sub(const Expr& f, const Dict& rep) {
    detail::SubstituteDictRT visitor(rep);
    Expr result = f->visit(visitor);
    return (result) ?  result : f;
  }

  namespace detail {
    struct DerivativeRT : VisitorHelper<DerivativeRT> {
      DerivativeRT(VarRT var): _var(var) {}

      template<class Op>
      Expr apply(const Op& v) {
      	return derivative(v, _var);
      }

      Expr apply(const Expr& v) {
	stator_throw() << "No specialised compile-time implementation available for " << v;
      }
      
      VarRT _var;
    };
  }
  
  /*! \brief Runtime derivative where a Var<> type is known for the second argument.
   */
  template<auto N>
  Expr derivative(const Expr& f, const Var<N>& v) {
    detail::DerivativeRT visitor(v);
    return f->visit(visitor);
  }

  /*! \brief Runtime derivative where variable is an Expr.
    
    Confirm its a variable and hand it over to the templated implementation.
   */
  Expr derivative(const Expr& f, const Expr& v) {
    return derivative(f, v.as<VarRT>());
  }
  
  namespace detail {
    struct FastSubRT : VisitorHelper<FastSubRT> {
      FastSubRT(VarRT var, double replacement): _var(var), _replacement(replacement) {}
      
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

      Expr apply(const BinaryOp<Expr, detail::Array, Expr>& op)
      { stator_throw() << "fast_sub cannot operate on this (" << repr(op) << ") expression"; }
      
      VarRT _var;
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
    case detail::RT_type_index<BinaryOp<Expr, detail::Equality, Expr>>::value: return c.visit(static_cast<const BinaryOp<Expr, detail::Equality, Expr>&>(*this));
    case detail::RT_type_index<BinaryOp<Expr, detail::Array, Expr>>::value:    return c.visit(static_cast<const BinaryOp<Expr, detail::Array, Expr>&>(*this));
    case detail::RT_type_index<List>::value:                                   return c.visit(static_cast<const List&>(*this));
    case detail::RT_type_index<Dict>::value:                                   return c.visit(static_cast<const Dict&>(*this));
    case detail::RT_type_index<UnaryOp<Expr, detail::Negate>>::value:          return c.visit(static_cast<const UnaryOp<Expr, detail::Negate>&>(*this));
    default: stator_throw() << "Unhandled type index (" << _type_idx << ") for the visitor";
    }
  }
  
  template<class Config = DefaultReprConfig>
  inline std::string repr(const sym::List& f)
  {
    std::string out = std::string((Config::Latex_output) ? "\\left[" : "[");
    const std::string end = std::string((Config::Latex_output) ? "\\right]" : "]");
    if (f.empty())
      return out+end;
    
    for (const auto& term : f)
      out += repr<Config>(term) + ", ";
    
    return out.substr(0, out.size() - 2) + end;
  }

  template<class Config = DefaultReprConfig>
  inline std::string repr(const sym::Dict& f)
  {
    std::string out = std::string((Config::Latex_output) ? "\\left\\{" : "{");
    const std::string end = std::string((Config::Latex_output) ? "\\right\\}" : "}");
    if (f.empty())
      return out+end;

    std::vector<std::pair<std::string, const Dict::key_type*> > keys;
    for (const auto& term : f)
      keys.emplace_back(repr<Config>(term.first), &term.first);

    sort(keys.begin(), keys.end(), [](const auto& l, const auto& r){ return l.first < r.first; });

    for (const auto& k : keys)
      out += k.first + ":" + repr<Config>(f.at(*k.second)) + ", ";
    
    return out.substr(0, out.size() - 2) + end;
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
    struct ReprVisitor : public sym::detail::VisitorHelper<ReprVisitor<Config> > {
      template<class T> sym::Expr apply(const T& rhs) {
	_repr = repr<Config>(rhs);
	return sym::Expr();
      }

      std::string _repr;
    };
  }

  template<class Config>
  std::string repr(const sym::RTBase& b) {
    detail::ReprVisitor<Config> visitor;
    b.visit(visitor);
    return visitor._repr;
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
