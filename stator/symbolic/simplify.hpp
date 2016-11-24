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

namespace stator {
  namespace symbolic {
    ////////////////////// Simplify! ////////////////////////////
    //
    // Rules of the game:
    //
    // (1) A simplfy implementation is only available if there is a
    // simplification! This is required to ensure recursive
    // simplification terminates.
    //
    // (2) Cancellation is the highest priority to keep the AST as
    // small as possible.
    //
    // (3) Adding new rules is difficult, some simplify rules are
    // recursive, thus you must avoid infinite recursion AND ambiguity.
    //
    // (4) ...?

    ////////////////////// Simplify configuration //////////////
    namespace detail {
      struct expand_to_Polynomial_ID;
    };

    struct expand_to_Polynomial : orphan::basic_conf_t<detail::expand_to_Polynomial_ID> {};

    template <typename ...Args>
    struct SimplifyConfig {
      static constexpr const auto expand_to_Polynomial = orphan::is_present<stator::symbolic::expand_to_Polynomial, Args...>::value;
    };

    using DefaultSimplifyConfig = SimplifyConfig<>;


    ////////////////////// Simplify generic implementations //////////////
    /*! \brief The simplify base implementation.

      By default, there is no generic simplify implemented. There
      should be a compiler error if no simplify is available.
     */
    template<class Config = DefaultSimplifyConfig> void simplify();

    namespace detail {
      template<class Config, class T>
      auto try_simplify_imp(const T& a, detail::choice<0>) -> STATOR_AUTORETURN(simplify<Config>(a));
      
      template<class Config, class T>
      T try_simplify_imp(const T& a, detail::choice<1>) {
	return a;
      }
    }

    /*! \brief A method to apply simplification only if it is
        available.
     */
    template<class Config = DefaultSimplifyConfig, class T>
    auto try_simplify(const T& a) -> decltype(detail::try_simplify_imp<Config>(a, detail::select_overload{})) {
      return detail::try_simplify_imp<Config>(a, detail::select_overload{});
    }

    typedef SimplifyConfig<expand_to_Polynomial> ExpandConfig;
    
    /*! \brief A variant of simplify that expands into Polynomial
        types aggressively.
     */
    template<class T> auto expand(const T& a)
      -> STATOR_AUTORETURN(simplify<ExpandConfig>(a));

    /*! \brief A variant of try_simplify that expands into Polynomial
        types aggressively.
     */
    template<class T> auto try_expand(const T& a)
      -> STATOR_AUTORETURN(try_simplify<ExpandConfig>(a));
    
    /////////////////// BINARY OPERATORS /////////////////////////////
    // 
    // Here we perform different simplifications with different
    // priority. We must attempt to prune the expression tree as fast
    // as possible to minimise template depth. Then we try
    // permutations.
    
    // Prune expressions which do not require the BinaryOp framework
    // as their are higher precedent operator implementations// .
    template<class Config, class LHS, class RHS, class Op>
    auto simplify_BinaryOp(const BinaryOp<LHS, RHS, Op>& f, detail::choice<0>)
      -> typename std::enable_if<!std::is_same<decltype(Op::apply(f._l, f._r)), BinaryOp<LHS, RHS, Op> >::value, decltype(Op::apply(f._l, f._r))>::type
    { return Op::apply(f._l, f._r); };
    
    // Convert any zero operations into the zero
    template<class Config, class LHS, class RHS, class Op,
	     typename = typename std::enable_if<std::is_same<RHS, typename Op::right_zero>::value >::type>
    typename Op::right_zero simplify_BinaryOp(const BinaryOp<LHS, RHS, Op>&, detail::choice<0>) { return {}; };
    
    template<class Config, class LHS, class RHS, class Op,
	     typename = typename std::enable_if<std::is_same<LHS, typename Op::left_zero>::value && !std::is_same<RHS, typename Op::right_zero>::value>::type>
      typename Op::left_zero simplify_BinaryOp(const BinaryOp<LHS, RHS, Op>&, detail::choice<0>) { return {}; };

    // Eliminate any identity operations
    template<class Config, class LHS, class RHS, class Op,
	     typename = typename std::enable_if<std::is_same<RHS, typename Op::right_identity>::value >::type>
    LHS simplify_BinaryOp(const BinaryOp<LHS, RHS, Op>& op, detail::choice<0>) { return try_simplify<Config>(op._l); }

    template<class Config, class LHS, class RHS, class Op,
	     typename = typename std::enable_if<std::is_same<LHS, typename Op::left_identity>::value && !std::is_same<RHS, typename Op::right_identity>::value>::type>
      RHS simplify_BinaryOp(const BinaryOp<LHS, RHS, Op>& op, detail::choice<0>) { return try_simplify<Config>(op._r); }

    //Special case for divide
    template<class Config, char Letter>
    Unity simplify_BinaryOp(const BinaryOp<Variable<Letter>, Variable<Letter>, detail::Divide>& op, detail::choice<0>)
    { return {};}
    
    //Special case for the subtraction binary operator becoming the unary negation operator
    template<class Config, class RHS, typename = typename std::enable_if<!std::is_same<RHS, Null>::value>::type>
    RHS simplify_BinaryOp(const SubtractOp<Null, RHS>& r, detail::choice<0>) { return try_simplify<Config>(-r._r); }
    
    //Transformations to power-ops where possible
    template<class Config, char Letter>
    auto simplify_BinaryOp(const MultiplyOp<Variable<Letter>, Variable<Letter> >&, detail::choice<0>)
      -> STATOR_AUTORETURN((PowerOp<Variable<Letter>, 2>()));
    
    template<class Config, char Letter, size_t Order>
    auto simplify_BinaryOp(const MultiplyOp<PowerOp<Variable<Letter>, Order>, Variable<Letter> >&, detail::choice<0>)
      -> STATOR_AUTORETURN((PowerOp<Variable<Letter>, Order+1>()));
    
    template<class Config, char Letter, size_t Order>
    auto simplify_BinaryOp(const MultiplyOp<Variable<Letter>, PowerOp<Variable<Letter>, Order> >&, detail::choice<0>)
      -> STATOR_AUTORETURN((PowerOp<Variable<Letter>, Order+1>()));
        
    // Simplification of both arguments available
    template<class Config, class LHS, class RHS, class Derived>
    auto simplify_BinaryOp(const BinaryOp<LHS, RHS, Derived>& f, detail::choice<1>)
      -> STATOR_AUTORETURN(try_simplify<Config>(Derived::apply(simplify<Config>(f._l), simplify<Config>(f._r))));
    
    // Simplification of only one argument available
    template<class Config, class LHS, class RHS, class Derived>
    auto simplify_BinaryOp(const BinaryOp<LHS, RHS, Derived>& f, detail::choice<2>)
      -> STATOR_AUTORETURN(try_simplify<Config>(Derived::apply(simplify<Config>(f._l), f._r)));
    
    template<class Config, class LHS, class RHS, class Derived>
    auto simplify_BinaryOp(const BinaryOp<LHS, RHS, Derived>& f, detail::choice<2>)
      -> STATOR_AUTORETURN(try_simplify<Config>(Derived::apply(f._l, simplify<Config>(f._r))));
    
    // Reorder to find further simplifications
    //
    // If you're trying to check if there is a simplification for a
    // reordering, you must create the new BinaryOp's yourself and not
    // use Op::apply, as the arguments may have their own ::operatorX
    // methods which are simpler but do not simplify(). To detect this
    // we rely on the first BinaryOp simplification rule which detects
    // arguments having their own specialised operators.
    
    // For associative operators, try (A*B)*C as A*(B*C) (but only do this if B*C has a simplification)
    template<class Config, class Arg1, class Arg2, class Arg3, class Op,
	     typename = typename std::enable_if<Op::associative>::type>
    auto simplify_BinaryOp(const BinaryOp<BinaryOp<Arg1, Arg2, Op>, Arg3, Op>& f, detail::choice<4>)
      -> STATOR_AUTORETURN(try_simplify<Config>(Op::apply(f._l._l, simplify<Config>(BinaryOp<Arg2, Arg3, Op>(f._l._r, f._r)))));
    
    // For associative operators, try A*(B*C) as (A*B)*C (but only do this if A*B has a simplification)
    template<class Config, class Arg1, class Arg2, class Arg3, class Op,
	     typename = typename std::enable_if<Op::associative>::type>
    auto simplify_BinaryOp(const BinaryOp<Arg1, BinaryOp<Arg2, Arg3, Op>, Op>& f, detail::choice<4>)
      -> STATOR_AUTORETURN(try_simplify<Config>(Op::apply(simplify<Config>(BinaryOp<Arg1, Arg2, Op>(f._l, f._r._l)), f._r._r)));

    // For associative and commutative operators, try (A*B)*C as (A*C)*B (but only do this if A*C has a simplification)
    template<class Config, class Arg1, class Arg2, class Arg3, class Op,
	     typename = typename std::enable_if<Op::associative>::type>
    auto simplify_BinaryOp(const BinaryOp<BinaryOp<Arg1, Arg2, Op>, Arg3, Op>& f, detail::choice<5>)
      -> STATOR_AUTORETURN(try_simplify<Config>(Op::apply(simplify<Config>(BinaryOp<Arg1, Arg3, Op>(f._l._l, f._r)), f._l._r)));
    
    // For associative and commutative operators, try A*(B*C) as (A*C)*B (but only do this if A*C has a simplification)
    template<class Config, class Arg1, class Arg2, class Arg3, class Op,
	     typename = typename std::enable_if<Op::associative>::type>
    auto simplify_BinaryOp(const BinaryOp<Arg1, BinaryOp<Arg2, Arg3, Op>, Op>& f, detail::choice<5>)
      -> STATOR_AUTORETURN(try_simplify<Config>(Op::apply(simplify<Config>(BinaryOp<Arg1, Arg3, Op>(f._l, f._r._r)), f._r._l)));


    // Finally, the gateway to the above series of simplifications for BinaryOps!
    template<class Config = DefaultSimplifyConfig, class T, typename = typename std::enable_if<std::is_base_of<BinaryOpBase, T>::value >::type >
    auto simplify(T t) -> STATOR_AUTORETURN(simplify_BinaryOp<Config>(t, detail::select_overload{}));
    

    /////////////////// PowerOp OPERATORS /////////////////////////////
    // 
    template<class Config, class Arg, size_t Power>
    auto simplify_powerop_impl(const PowerOp<Arg, Power>& f, detail::choice<0>)
      -> STATOR_AUTORETURN(try_simplify<Config>(PowerOpSubstitution<Power>::eval(simplify<Config>(f._arg))));
        
    template<class Config, class Arg, size_t Power>
    auto simplify_powerop_impl(const PowerOp<Arg, Power>& f, detail::choice<1>)
      -> STATOR_AUTORETURN(simplify<Config>(PowerOpSubstitution<Power>::eval(f._arg)));

    //Disable expansion of PowerOps of variables (otherwise we will recurse to death)
    template<class T> struct PowerOpEnableExpansion { static const bool value = true; };
    template<char Letter> struct PowerOpEnableExpansion<Variable<Letter> > { static const bool value = false; };

    /*! \brief Expansion operator for PowerOp types. 
    
      This implementation only works if the argument has an simplify
      function defined for its argument.
     */
    template<class Config = DefaultSimplifyConfig, class Arg, size_t Power,
	     typename = typename std::enable_if<PowerOpEnableExpansion<Arg>::value>::type>
    auto simplify(const PowerOp<Arg, Power>& f) -> STATOR_AUTORETURN(simplify_powerop_impl<Config>(f, detail::select_overload{}));


    /*! \relates Polynomial 
	
 	\brief Type trait which determines if an operation
 	(multiplication, addition) can be distributed over the
 	coefficients of a polynomial.

 	This general form allows all operations between all constants.
      */

    /*! \brief Simplification of a Polynomial LHS multiplied by a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, typename = typename std::enable_if<Config::expand_to_Polynomial>::type>
    Polynomial<Order+1, Real, Letter> simplify(MultiplyOp<Variable<Letter>, Polynomial<Order, Real, Letter> >& f)
    {
      Polynomial<Order+1, Real, Letter> retval;
      retval[0] = 0;
      std::copy(f._r.begin(), f._r.end(), retval.begin() + 1);
      return retval;
    }
    
    /*! \brief Simplification of a Polynomial RHS multiplied by a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, typename = typename std::enable_if<Config::expand_to_Polynomial>::type>
    Polynomial<Order+1, Real, Letter> simplify(const MultiplyOp<Polynomial<Order, Real, Letter>, Variable<Letter>>& f)
    {
      Polynomial<Order+1, Real, Letter> retval;
      retval[0] = empty_sum(retval[0]);
      std::copy(f._l.begin(), f._l.end(), retval.begin() + 1);
      return retval;
    }

    /*! \brief Simplification of a Polynomial LHS added to a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, typename = typename std::enable_if<Config::expand_to_Polynomial>::type>
    Polynomial<Order+1, Real, Letter> simplify(const AddOp<Polynomial<Order, Real, Letter>, Variable<Letter>>& f)
    { 
      Polynomial<(Order > 0) ? Order : 1, Real, Letter> retval(f._l);
      retval[1] += 1;
      return retval;
    }
    
    /*! \brief Simplification of a Polynomial RHS added to a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, typename = typename std::enable_if<Config::expand_to_Polynomial>::type>
    Polynomial<Order+1, Real, Letter> simplify(const MultiplyOp<Variable<Letter>, Polynomial<Order, Real, Letter> >& f)
    {
      Polynomial<(Order > 0) ? Order : 1, Real, Letter> retval(f._r);
      retval[1] += 1;
      return retval;
    }
    
    /*! \brief Simplification of a Polynomial LHS subtracted by a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, typename = typename std::enable_if<Config::expand_to_Polynomial>::type>
    Polynomial<Order+1, Real, Letter> simplify(const SubtractOp<Polynomial<Order, Real, Letter>, Variable<Letter>>& f)
    {
      Polynomial<(Order > 0) ? Order : 1, Real, Letter> retval(f._l);
      retval[1] -= 1;
      return retval;
    }
    
    /*! \brief Simplification of a Polynomial RHS subtracted by a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, typename = typename std::enable_if<Config::expand_to_Polynomial>::type>
    Polynomial<Order+1, Real, Letter> simplify(const SubtractOp<Variable<Letter>, Polynomial<Order, Real, Letter>>& f)
    {
      Polynomial<(Order > 0) ? Order : 1, Real, Letter> retval(-f._r);
      retval[1] += 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial operating with a
      constant RHS. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, class Op, class Real2,
	     typename = typename std::enable_if<Config::expand_to_Polynomial && detail::IsConstant<Real2>::value>::type>
      auto simplify_BinaryOp(const BinaryOp<Polynomial<Order, Real, Letter>, Real2, Op>& f, detail::last_choice)
      -> STATOR_AUTORETURN(try_simplify<Config>(Op::apply(f._l, Polynomial<0, Real2, Letter>{f._r})));
    
    /*! \brief Simplification of a Polynomial operating with a
      constant LHS. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, class Op, class Real2,
	     typename = typename std::enable_if<Config::expand_to_Polynomial && detail::IsConstant<Real2>::value>::type>
      auto simplify_BinaryOp(const BinaryOp<Real2, Polynomial<Order, Real, Letter>, Op>& f, detail::last_choice)
      -> STATOR_AUTORETURN(try_simplify<Config>(Op::apply(Polynomial<0, Real2, Letter>{f._l}, f._r)));
    
    //      template<class OpType, class PolyType>
    //      struct distribute_poly {
    // 	static const bool value = (detail::IsConstant<OpType>::value && detail::IsConstant<PolyType>::value);
    //      };
    //
    //      /*! \relates Polynomial 
    //	
    // 	\brief Type trait enabling use of std::complex as a Polynomial
    // 	coefficient with arithmetic types.
    //      */
    //      template<class OpType, class T>
    //      struct distribute_poly<OpType, std::complex<T> > {
    // 	static const bool value = std::is_arithmetic<OpType>::value;
    //      };
    //
    //      /*! \relates Polynomial 
    //	
    // 	\brief Type trait enabling use of std::complex as an operation
    // 	with Polynomials with arithmetic type coefficients.
    //      */
    //      template<class T, class PolyType>
    //      struct distribute_poly<std::complex<T>, PolyType> {
    // 	static const bool value = std::is_arithmetic<PolyType>::value;
    //      };
    //
    //      /*! \relates Polynomial 
    //	
    // 	\brief Type trait enabling use of std::complex as an operation
    // 	with Polynomials.
    //      */
    //      template<class T>
    //      struct distribute_poly<std::complex<T>, std::complex<T> > {
    // 	static const bool value = true;
    //      };    
    //
    //
    ///*! \brief Simplification of a Polynomial LHS added to a
    //  PowerOp of the Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    //Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const AddOp<Polynomial<Order, Real, Letter>, PowerOp<Variable<Letter>, POrder> > & f)
    //{
    //  Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(f._l);
    //  retval[POrder] += 1;
    //  return retval;
    //}
    //
    ///*! \brief Simplification of a Polynomial RHS added to a
    //  PowerOp of the Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    //Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const AddOp<PowerOp<Variable<Letter>, POrder>, Polynomial<Order, Real, Letter> > & f)
    //{
    //  Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(f._r);
    //  retval[POrder] += 1;
    //  return retval;
    //}
    //
    ///*! \brief Simplification of a Polynomial LHS subtracted from a
    //  PowerOp of the Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    //Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const SubtractOp<Polynomial<Order, Real, Letter>, PowerOp<Variable<Letter>, POrder> >& f)
    //{
    //  Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(f._r);
    //  retval[POrder] -= 1;
    //  return retval;
    //}
    //
    ///*! \brief Simplification of a Polynomial RHS subtracted from a
    //  PowerOp of the Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    //Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const SubtractOp<PowerOp<Variable<Letter>, POrder>, Polynomial<Order, Real, Letter> >& f)
    //{
    //  Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(-f._r);
    //  retval[POrder] += Real(1);
    //  return retval;
    //}
    //
    ///*! \brief Simplification of a Polynomial LHS multiplied by a
    //  PowerOp of a Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    //Polynomial<Order+POrder, Real, Letter> simplify(const MultiplyOp<PowerOp<Variable<Letter>, POrder>, Polynomial<Order, Real, Letter> >& f)
    //{
    //  Polynomial<Order+POrder, Real, Letter> retval;
    //  std::copy(f._r.begin(), f._r.end(), retval.begin() + POrder);
    //  return retval;
    //}
    //
    ///*! \brief Simplification of a Polynomial RHS multiplied by a
    //  PowerOp of a Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    //Polynomial<Order+POrder, Real, Letter> simplify(const MultiplyOp<Polynomial<Order, Real, Letter>, PowerOp<Variable<Letter>, POrder> >& f)
    //{
    //  Polynomial<Order+POrder, Real, Letter> retval;
    //  std::copy(f._l.begin(), f._l.end(), retval.begin() + POrder);
    //  return retval;
    //}
    //
    ///*! \brief Simplification of a Polynomial LHS added to a
    //  PowerOp of the Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    //Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const AddOp<Polynomial<Order, Real, Letter>, Unity> & f)
    //{
    //  Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(f._l);
    //  retval[0] += 1;
    //  return retval;
    //}
    //
    ///*! \brief Simplification of a Polynomial RHS added to a
    //  PowerOp of the Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    //Polynomial<Order, Real, Letter> simplify(const AddOp<Unity, Polynomial<Order, Real, Letter> > & f)
    //{
    //  Polynomial<Order, Real, Letter> retval(f._r);
    //  retval[0] += 1;
    //  return retval;
    //}
    //
    ///*! \brief Simplification of a Polynomial LHS subtracted by a
    //  PowerOp of the Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    //Polynomial<Order, Real, Letter> simplify(const SubtractOp<Polynomial<Order, Real, Letter>, Unity> & f)
    //{
    //  Polynomial<Order, Real, Letter> retval(f._l);
    //  retval[0] -= 1;
    //  return retval;
    //}
    //
    ///*! \brief Simplification of a Polynomial RHS subtracted by a
    //  PowerOp of the Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    //Polynomial<Order, Real, Letter> simplify(const SubtractOp<Unity, Polynomial<Order, Real, Letter> > & f)
    //{
    //  Polynomial<Order, Real, Letter> retval(-f._r);
    //  retval[0] += 1;
    //  return retval;
    //}
    //
    ///*! \brief Conversion of PowerOp RHS multiplied by a constant to a
    //  Polynomial. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, Real, Letter> >::type 
    //simplify(const MultiplyOp<PowerOp<Variable<Letter>, Order>, Real>& f)
    //{
    //  Polynomial<Order, Real, Letter> retval;
    //  retval[Order] = f._r;
    //  return retval;
    //}
    //
    ///*! \brief Conversion of PowerOp LHS multiplied by a constant to a
    //  Polynomial. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, Real, Letter> >::type 
    //simplify(const MultiplyOp<Real, PowerOp<Variable<Letter>, Order> >& f)
    //{
    //  Polynomial<Order, Real, Letter> retval;
    //  retval[Order] = f._l;
    //  return retval;
    //}
    //
    ///*! \brief Conversion of Variable RHS multiplied by a constant to a
    //  Polynomial. */
    //template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type
    //simplify(const MultiplyOp<Variable<Letter>, Real> & f)
    //{ return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{toArithmetic(empty_sum(f._r)), toArithmetic(f._r)}; }
    //
    ///*! \brief Conversion of Variable LHS multiplied by a constant to a
    //  Polynomial. */
    //template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value && !std::is_same<Real, Unity>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type 
    //simplify(const MultiplyOp<Real, Variable<Letter> > & f)
    //{ return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{empty_sum(f._l), f._l}; }
    //
    ///*! \brief Conversion of PowerOp LHS added with a constant to a
    //  Polynomial. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> >::type 
    //simplify(const AddOp<PowerOp<Variable<Letter>, Order>, Real>& f)
    //{ 
    //  Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> retval;
    //  retval[Order] = 1;
    //  retval[0] = toArithmetic(f._r);
    //  return retval;
    //}
    //
    ///*! \brief Conversion of PowerOp LHS added with a constant to a
    //  Polynomial. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> >::type 
    //simplify(const AddOp<Real, PowerOp<Variable<Letter>, Order> >& f)
    //{ 
    //  Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> retval;
    //  retval[Order] = 1;
    //  retval[0] = toArithmetic(f._r);
    //  return retval;
    //}
    //
    ///*! \brief Conversion of PowerOp LHS subtracted with a constant to a
    //  Polynomial. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> >::type 
    //simplify(const SubtractOp<PowerOp<Variable<Letter>, Order>, Real>& f)
    //{
    //  Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> retval;
    //  retval[Order] = 1;
    //  retval[0] = -toArithmetic(f._r);
    //  return retval;
    //}
    //
    ///*! \brief Conversion of PowerOp LHS subtracted with a constant to a
    //  Polynomial. */
    //template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> >::type 
    //simplify(const SubtractOp<Real, PowerOp<Variable<Letter>, Order> >& f)
    //{
    //  Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> retval;
    //  retval[Order] = -1;
    //  retval[0] = toArithmetic(f._l);
    //  return retval;
    //}
    //
    ///*! \brief Conversion of a Variable RHS added with a constant. */
    //template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type
    //simplify(const AddOp<Variable<Letter>, Real>& f)
    //{ return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{toArithmetic(f._r), Real(1)}; }
    //
    ///*! \brief Conversion of a Variable LHS added with a constant. */
    //template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type 
    //simplify(const AddOp<Real, Variable<Letter> >& f)
    //{ return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{toArithmetic(f._r), Real(1)}; }
    //
    //
    ///*! \brief Conversion of a Variable added with a Variable. */
    //template<class Config = DefaultSimplifyConfig, char Letter>
    //Polynomial<1, int, Letter>
    //simplify(const AddOp<Variable<Letter>, Variable<Letter> >& f)
    //{ return Polynomial<1, int, Letter>{0, 2}; }
    //
    //
    ///*! \brief Conversion of a Variable RHS subtracted with a constant. */
    //template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type 
    //simplify(const SubtractOp<Variable<Letter>, Real>& f)
    //{ return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{-toArithmetic(f._r), Real(1)}; }
    //
    ///*! \brief Conversion of a Variable LHS subtracted with a constant. */
    //template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    //typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type
    //simplify(const SubtractOp<Real, Variable<Letter> >& f)
    //{ return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{toArithmetic(f._l), Real(-1)}; }
    //
    ///*! \brief Right-handed addition operation on a Polynomial.
    //  
    //  This operator is only enabled if the type of the Polynomial
    //  coefficients and the type being added is marked as compatitble
    //  for distribution over the Polnomial coefficients. This is tested
    //  using detail::distribute_poly.
    //*/
    //template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
    //	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    //auto simplify(const AddOp<Real1, Polynomial<Order, Real, Letter> >& f) -> Polynomial<Order, STORETYPE(f._l + f._r[0]), Letter>
    //{
    //  Polynomial<Order, STORETYPE(f._l + f._r[0]), Letter> retval(f._r);
    //  retval[0] += f._l;
    //  return retval;
    //}
    //
    //
    ///*!\brief Left-handed addition operator for Polynomials 
    //
    //  This operator is only enabled if the type of the Polynomial
    //  coefficients and the type being added is marked as compatitble
    //  for distribution over the Polnomial coefficients. This is tested
    //  using detail::distribute_poly.
    //*/
    //template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
    //	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    //auto simplify(const AddOp<Polynomial<Order, Real, Letter>, Real1>& f) -> Polynomial<Order, STORETYPE(f._l[0] + f._r), Letter>
    //{
    //  Polynomial<Order, STORETYPE(f._l[0] + f._r), Letter> retval(f._l);
    //  retval[0] += f._r;
    //  return retval;
    //}
    //
    //
    ///*! \brief Right-handed multiplication operation on a Polynomial.
    //  
    //  This operator is only enabled if the type of the Polynomial
    //  coefficients and the type being added is marked as compatitble
    //  for distribution over the Polnomial coefficients. This is tested
    //  using detail::distribute_poly.
    //*/
    //template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
    //	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    //auto simplify(const MultiplyOp<Real1, Polynomial<Order, Real, Letter> >& f) -> Polynomial<Order, STORETYPE(f._l * f._r[0]), Letter>
    //{
    //  Polynomial<Order, STORETYPE(f._l * f._r[0]), Letter> retval;
    //
    //  for (size_t i(0); i <= Order; ++i)
    //	retval[i] = f._l * f._r[i];
    //
    //  return retval;
    //}
    //
    ///*! \brief Left-handed multiplication on a Polynomial.
    //
    //  This operator is only enabled if the type of the Polynomial
    //  coefficients and the type being added is marked as compatitble
    //  for distribution over the Polnomial coefficients. This is tested
    //  using detail::distribute_poly.
    //*/
    //template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
    //	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    //auto simplify(const MultiplyOp<Polynomial<Order, Real, Letter>, Real1>& f) -> Polynomial<Order, STORETYPE(f._l[0] * f._r), Letter>
    //{
    //  Polynomial<Order, STORETYPE(f._l[0] * f._r), Letter> retval;
    //  for (size_t i(0); i <= Order; ++i)
    //	retval[i] = f._l[i] * f._r;
    //  return retval;
    //}
    //
    ///*! \brief Specialisation for squares of matrix expressions. */
    //template<class Config = DefaultSimplifyConfig, size_t Power, class Matrix, size_t N, char Letter,
    //         typename = typename std::enable_if<(Power==2) && std::is_base_of<Eigen::EigenBase<Matrix>, Matrix>::value>::type>
    //auto simplify(const PowerOp<Polynomial<N, Matrix, Letter>, Power>& f)
    //  -> Polynomial<2 * N, STORETYPE(f._arg[0].dot(f.arg[0])), Letter>
    //{ 
    //  Polynomial<2 * N, STORETYPE(f._arg[0].dot(f._arg[0])), Letter> retval;
    //  for (size_t i(0); i <= 2 * N; ++i)
    //	for (size_t j(i>N?i-N:0); (j <= i) && (j <=N); ++j)
    //	  retval[i] += f._arg[j].dot(f._arg[i-j]);
    //  return retval;
    //}
    //
    ///*! \brief Division of a Polynomial by a constant. */
    //template<class Config = DefaultSimplifyConfig, class Real1, class Real2, size_t N, char Letter,
    //	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real2>::value>::type>
    //auto simplify(const DivideOp<Polynomial<N, Real1, Letter>, Real2> & f) -> Polynomial<N, STORETYPE(f._l[0] / f._r), Letter>
    //{
    //  Polynomial<N, STORETYPE(f._l[0] / f._r), Letter> retval;
    //  for (size_t i(0); i <= N; ++i)
    //	retval[i] = f._l[i] / f._r;
    //  return retval;
    //}

    //                      FUNCTION SIMPLIFICATION
    template<class Config = DefaultSimplifyConfig, class Arg, size_t FuncID>
    auto simplify(const Function<Arg, FuncID>& f) -> decltype(Function<decltype(simplify<Config>(f._arg)), FuncID>(simplify<Config>(f._arg)))
    { return Function<decltype(simplify<Config>(f._arg)), FuncID>(simplify<Config>(f._arg)); }

    template<class Config = DefaultSimplifyConfig, class LHS, class Arg>
    auto simplify(const MultiplyOp<LHS, arbsignF<Arg> >& f) 
      -> decltype(arbsign(try_simplify<Config>(f._l * f._r._arg)))
    { return arbsign(try_simplify<Config>(f._l * f._r._arg)); }

    template<class Config = DefaultSimplifyConfig, class RHS, class Arg>
    auto simplify(const MultiplyOp<arbsignF<Arg>, RHS>& f)
      -> decltype(arbsign(try_simplify<Config>(f._l._arg * f._r)))
    { return arbsign(try_simplify<Config>(f._l._arg * f._r)); }

    template<class Config = DefaultSimplifyConfig, class Arg1, class Arg2>
    auto simplify(const MultiplyOp<arbsignF<Arg1>, arbsignF<Arg2> >& f)
      -> decltype(arbsign(try_simplify<Config>(f._l._arg * f._r._arg)))
    { return arbsign(try_simplify<Config>(f._l._arg * f._r._arg)); }

    template<class Config = DefaultSimplifyConfig, class LHS, class Arg>
    auto simplify(const DivideOp<LHS, arbsignF<Arg> >& f) 
      -> decltype(arbsign(try_simplify<Config>(f._l / f._r._arg)))
    { return arbsign(try_simplify<Config>(f._l / f._r._arg)); }

    template<class Config = DefaultSimplifyConfig, class RHS, class Arg>
    auto simplify(const DivideOp<arbsignF<Arg>, RHS>& f)
      -> decltype(arbsign(try_simplify<Config>(f._l._arg / f._r)))
    { return arbsign(try_simplify<Config>(f._l._arg / f._r)); }

    template<class Config = DefaultSimplifyConfig, class Arg1, class Arg2>
    auto simplify(const DivideOp<arbsignF<Arg1>, arbsignF<Arg2> >& f)
      -> decltype(arbsign(try_simplify<Config>(f._l._arg / f._r._arg)))
    { return arbsign(try_simplify<Config>(f._l._arg / f._r._arg)); }

    template<class Config = DefaultSimplifyConfig, class Arg>
    auto simplify(const arbsignF<arbsignF<Arg> >& f)
      -> decltype(arbsign(try_simplify<Config>(f._arg._arg)))
    { return arbsign(try_simplify<Config>(f._arg._arg)); }

    //For even powers, remove the sign term
    template<class Config = DefaultSimplifyConfig, class Arg, size_t Power>
    auto simplify(const PowerOp<arbsignF<Arg>,Power>& f)
      -> typename std::enable_if<!(Power % 2), decltype(pow<Power>(f._arg._arg))>::type
    { return pow<Power>(f._arg._arg); }

    //For odd powers, move the sign term outside
    template<class Config = DefaultSimplifyConfig, class Arg, size_t Power>
    auto simplify(const PowerOp<arbsignF<Arg>,Power>& f)
      -> typename std::enable_if<Power % 2, decltype(arbsign(pow<Power>(f._arg._arg)))>::type
    { return arbsign(pow<Power>(f._arg._arg)); }    
  }
}
