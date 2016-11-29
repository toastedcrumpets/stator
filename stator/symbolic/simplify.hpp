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

namespace sym {
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

  struct expand_to_Polynomial : stator::orphan::basic_conf_t<detail::expand_to_Polynomial_ID> {};

  template <typename ...Args>
  struct SimplifyConfig {
    static constexpr const auto expand_to_Polynomial = stator::orphan::is_present<sym::expand_to_Polynomial, Args...>::value;
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
  template<class Config, class LHS, class Op>
  auto simplify_BinaryOp(const BinaryOp<LHS, typename Op::right_identity, Op>& op, detail::choice<0>) -> STATOR_AUTORETURN(try_simplify<Config>(op._l));

  template<class Config, class RHS, class Op,
	     typename = typename std::enable_if<!std::is_same<RHS, typename Op::right_identity>::value>::type>
    auto simplify_BinaryOp(const BinaryOp<typename Op::left_identity, RHS, Op>& op, detail::choice<0>) -> STATOR_AUTORETURN(try_simplify<Config>(op._r));

  //Special case for divide
  template<class Config, class ...VarArgs1, class ...VarArgs2,
	     typename = typename enable_if_var_in<Var<VarArgs1...>, Var<VarArgs2...> >::type>
  Unity simplify_BinaryOp(const BinaryOp<Var<VarArgs1...>, Var<VarArgs2...>, detail::Divide>& op, detail::choice<0>)
  { return {};}
  
  //Special case for the subtraction binary operator becoming the unary negation operator
  template<class Config, class RHS, typename = typename std::enable_if<!std::is_same<RHS, Null>::value>::type>
  RHS simplify_BinaryOp(const SubtractOp<Null, RHS>& r, detail::choice<0>) { return try_simplify<Config>(-r._r); }
  
  //Transformations to power-ops where possible
  template<class Config, class ...VarArgs1, class ...VarArgs2,
	     typename = typename enable_if_var_in<Var<VarArgs1...>, Var<VarArgs2...> >::type>
  auto simplify_BinaryOp(const MultiplyOp<Var<VarArgs1...>, Var<VarArgs2...> >&, detail::choice<0>)
    -> STATOR_AUTORETURN((PowerOp<typename variable_combine<Var<VarArgs1...>, Var<VarArgs2...> >::type, 2>()));
  
  template<class Config, class ...VarArgs1, class ...VarArgs2, size_t Order,
	     typename = typename enable_if_var_in<Var<VarArgs1...>, Var<VarArgs2...> >::type>
  auto simplify_BinaryOp(const MultiplyOp<PowerOp<Var<VarArgs1...>, Order>, Var<VarArgs2...> >&, detail::choice<0>)
    -> STATOR_AUTORETURN((PowerOp<typename variable_combine<Var<VarArgs1...>, Var<VarArgs2...> >::type, Order+1>()));
  
  template<class Config, class ...VarArgs1, class ...VarArgs2,  size_t Order,
	     typename = typename enable_if_var_in<Var<VarArgs1...>, Var<VarArgs2...> >::type>
  auto simplify_BinaryOp(const MultiplyOp<Var<VarArgs1...>, PowerOp<Var<VarArgs2...>, Order> >&, detail::choice<0>)
    -> STATOR_AUTORETURN((PowerOp<typename variable_combine<Var<VarArgs1...>, Var<VarArgs2...> >::type, Order+1>()));
      
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
  template<class ...VarArgs> struct PowerOpEnableExpansion<Var<VarArgs...> > { static const bool value = false; };

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
    Var. */
  template<class Config = DefaultSimplifyConfig, class PolyVar, class ...VarArgs, size_t Order, class Real,
	     typename = typename std::enable_if<Config::expand_to_Polynomial && variable_in<Var<VarArgs...>, PolyVar>::value>::type>
  Polynomial<Order+1, Real, PolyVar> simplify(MultiplyOp<Var<VarArgs...>, Polynomial<Order, Real, PolyVar> >& f)
  {
    Polynomial<Order+1, Real, PolyVar> retval;
    retval[0] = empty_sum(retval[0]);
    std::copy(f._r.begin(), f._r.end(), retval.begin() + 1);
    return retval;
  }
  
  /*! \brief Simplification of a Polynomial RHS multiplied by a
    Var. */
  template<class Config = DefaultSimplifyConfig, class PolyVar, class ...VarArgs, size_t Order, class Real,
	     typename = typename std::enable_if<(Config::expand_to_Polynomial && variable_in<Var<VarArgs...>, PolyVar>::value)>::type>
  Polynomial<Order+1, Real, typename variable_combine<PolyVar, Var<VarArgs...> >::type> simplify(const MultiplyOp<Polynomial<Order, Real, PolyVar>, Var<VarArgs...> >& f)
  {
    Polynomial<Order+1, Real, typename variable_combine<PolyVar, Var<VarArgs...> >::type> retval;
    retval[0] = empty_sum(retval[0]);
    std::copy(f._l.begin(), f._l.end(), retval.begin() + 1);
    return retval;
  }

  /*! \brief Conversion of a Var to a polynomial if polynomial expansion is enabled. */
  template<class Config = DefaultSimplifyConfig, class ...VarArgs,
	     typename = typename std::enable_if<Config::expand_to_Polynomial>::type>
  auto simplify(Var<VarArgs...>) -> STATOR_AUTORETURN((Polynomial<1, int, Var<VarArgs...> >{0, 1}));
  
  /*! \brief Conversion of a PowerOp to a Polynomial when Polynomial
      expansion is enabled.
   */
  template<class Config = DefaultSimplifyConfig, class ...VarArgs, size_t Power,
	     typename = typename std::enable_if<Config::expand_to_Polynomial>::type>
  Polynomial<Power, int, Var<VarArgs...>> simplify(const PowerOp<Var<VarArgs...>, Power>&) {
    Polynomial<Power, int, Var<VarArgs...> > retval;
    retval[Power] = 1;
    return retval;
  }

  /*! \brief A converter to arithmetic types
   */
  template<class T,
	     typename = typename std::enable_if<std::is_arithmetic<T>::value || std::is_base_of<Eigen::EigenBase<T>, T>::value>::type>
    auto toArithmetic(T val) -> STATOR_AUTORETURN_BYVALUE(val);

  template<class T,
	     typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    auto toArithmetic(std::complex<T> val) -> STATOR_AUTORETURN_BYVALUE(val);
  
  template<std::intmax_t n1, std::intmax_t d1,
  	     typename = typename std::enable_if<!(n1 % d1)>::type> 
  std::intmax_t toArithmetic(C<n1,d1> val) { return n1 / d1; }
  
  template<std::intmax_t n1, std::intmax_t d1, 
  	     typename = typename std::enable_if<n1 % d1>::type>
  double toArithmetic(C<n1,d1> val) { return double(n1) / double(d1); }

  /*! \brief Simplification of a Polynomial operating with a
    constant RHS. */
  template<class Config, class PolyVar, size_t Order, class Real, class Op, class Real2,
	     typename = typename std::enable_if<Config::expand_to_Polynomial && detail::IsConstant<Real2>::value>::type>
    auto simplify_BinaryOp(const BinaryOp<Polynomial<Order, Real, PolyVar>, Real2, Op>& f, detail::last_choice)
    -> STATOR_AUTORETURN(try_simplify<Config>(Op::apply(f._l, Polynomial<0, decltype(toArithmetic(f._r)), PolyVar>{toArithmetic(f._r)})));
  
  /*! \brief Simplification of a Polynomial operating with a
    constant LHS. */
  template<class Config, class PolyVar, size_t Order, class Real, class Op, class Real2,
	     typename = typename std::enable_if<Config::expand_to_Polynomial && detail::IsConstant<Real2>::value>::type>
    auto simplify_BinaryOp(const BinaryOp<Real2, Polynomial<Order, Real, PolyVar>, Op>& f, detail::last_choice)
    -> STATOR_AUTORETURN(try_simplify<Config>(Op::apply(Polynomial<0, decltype(toArithmetic(f._l)), PolyVar>{toArithmetic(f._l)}, f._r)));

  namespace detail {
    constexpr size_t max_order(size_t N, size_t M)
    { return N > M ? N : M; }
  }// namespace detail

  /*!\brief Addition operator for two Polynomial types. 
   */
  template<class Config = DefaultSimplifyConfig, class Real1, size_t N, class Real2, size_t M, class PolyVar1, class PolyVar2, class Op,
	     typename = typename std::enable_if<std::is_same<Op,detail::Add>::value || std::is_same<Op,detail::Subtract>::value>::type,
	     typename = typename enable_if_var_in<PolyVar1, PolyVar2>::type>
  auto simplify(const BinaryOp<Polynomial<N, Real1, PolyVar1>, Polynomial<M, Real2, PolyVar2>, Op> & f)
    -> Polynomial<detail::max_order(M, N), decltype(store(Op::apply(f._l[0],f._r[0]))), typename variable_combine<PolyVar1, PolyVar2>::type>
  {
    Polynomial<detail::max_order(M, N), decltype(store(Op::apply(f._l[0], f._r[0]))), typename variable_combine<PolyVar1, PolyVar2>::type> retval;
  
    for (size_t i(0); i <= std::min(N, M); ++i)
  	retval[i] = Op::apply(f._l[i], f._r[i]);
    
    for (size_t i(std::min(N, M)+1); i <= N; ++i)
  	retval[i] = Op::apply(f._l[i], empty_sum(f._l[i]));
  
    for (size_t i(std::min(N, M)+1); i <= M; ++i)
  	retval[i] = Op::apply(empty_sum(f._r[i]), f._r[i]);
    
    return retval;
  }
  
  /*! \brief Multiplication between two Polynomial types.
   */
  template<class Config = DefaultSimplifyConfig, class Real1, class Real2, size_t M, size_t N, class PolyVar1, class PolyVar2, class Op,
	     typename = typename std::enable_if<std::is_same<Op,detail::Multiply>::value || std::is_same<Op,detail::Dot>::value>::type>
  auto simplify(const BinaryOp<Polynomial<M, Real1, PolyVar1>, Polynomial<N, Real2, PolyVar2>, Op>& f)
    -> Polynomial<M + N, decltype(store(Op::apply(f._l[0], f._r[0]))), typename variable_combine<PolyVar1, PolyVar2>::type>
  {
    Polynomial<M + N, decltype(store(Op::apply(f._l[0], f._r[0]))), typename variable_combine<PolyVar1, PolyVar2>::type> retval;
    for (size_t i(0); i <= N+M; ++i)
  	for (size_t j(i>N?i-N:0); (j <= i) && (j <=M); ++j)
  	  retval[i] += Op::apply(f._l[j], f._r[i-j]);
    return retval;
  }

  /*! \brief Division between two Polynomial types.
   */
  template<class Config = DefaultSimplifyConfig, class Real1, class Real2, size_t M, class PolyVar1, class PolyVar2>
  auto simplify(const BinaryOp<Polynomial<M, Real1, PolyVar1>, Polynomial<0, Real2, PolyVar2>, detail::Divide>& f)
    -> Polynomial<M, decltype(store(f._l[0] / f._r[0])), typename variable_combine<PolyVar1, PolyVar2>::type>
  {
    Polynomial<M, decltype(store(f._l[0] / f._r[0])), typename variable_combine<PolyVar1, PolyVar2>::type> retval;
    for (size_t i(0); i <= M; ++i)
  	  retval[i] += f._l[i] / f._r[0];
    return retval;
  }
  
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

