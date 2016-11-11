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
    //The implementations below perform basic simplification of expressions
    //
    //These simplify expressions dramatically, so they have the highest priority
    template<class RHS>
    typename std::enable_if<!is_C<RHS>::value, Null>::type 
    multiply(const Null&, const RHS&, detail::choice<0>) { return Null(); }
    
    template<class LHS>
    typename std::enable_if<!is_C<LHS>::value, Null>::type 
    multiply(const LHS&, const Null&, detail::choice<0>) { return Null(); }

    template<class LHS>
    typename std::enable_if<!is_C<LHS>::value, LHS>::type 
    multiply(const LHS& l, const Unity& r, detail::choice<0>) { return l; }
    template<class RHS> 
    typename std::enable_if<!is_C<RHS>::value, RHS>::type 
    multiply(const Unity& l, const RHS& r, detail::choice<0>) { return r; }
    

    template<class LHS> 
    typename std::enable_if<!is_C<LHS>::value, LHS>::type 
    add(const LHS& l, const Null& r, detail::choice<0>) { return l; }
    template<class RHS> 
    typename std::enable_if<!is_C<RHS>::value, RHS>::type
    add(const Null& l, const RHS& r, detail::choice<0>) { return r; }

    template<class LHS> 
    typename std::enable_if<!is_C<LHS>::value, LHS>::type 
    subtract(const LHS& l, const Null&, detail::choice<0>) { return l; }
    template<class RHS> 
    auto subtract(const Null&, const RHS& r, detail::choice<0>) -> typename std::enable_if<!is_C<RHS>::value, decltype(-r)>::type { return -r; }
    
    template<class LHS> LHS divide(const LHS& l, const Unity&, detail::choice<0>) { return l; }
    template<char Letter> Unity divide(const Variable<Letter>& l, const Variable<Letter>&, detail::choice<0>) { return Unity(); }

    template<char Letter>
    PowerOp<Variable<Letter>, 2> multiply(const Variable<Letter>&, const Variable<Letter>&, detail::choice<0>)
    { return PowerOp<Variable<Letter>, 2>(Variable<Letter>()); }
    
    template<char Letter, size_t Order>
    PowerOp<Variable<Letter>, Order+1> multiply(const PowerOp<Variable<Letter>, Order>&, const Variable<Letter>&, detail::choice<0>)
    { return PowerOp<Variable<Letter>, Order+1>(Variable<Letter>()); }
    
    template<char Letter, size_t Order>
    PowerOp<Variable<Letter>, Order+1> multiply(const Variable<Letter>&, const PowerOp<Variable<Letter>, Order>&, detail::choice<0>)
    { return PowerOp<Variable<Letter>, Order+1>(Variable<Letter>()); }

    template<class C_arg, class factor, class offset = std::ratio<0> >
    struct is_whole_factor {
      static const bool value = (std::ratio_divide<std::ratio_subtract<C_arg, offset>, factor>::den == 1);
    };

    //Specialisations of sine cosine for whole multiples of pi/2
    template<std::intmax_t num, std::intmax_t den,
	     typename = typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi>::value>::type>
    constexpr Null sin(const C<num, den>&) { return Null(); }

    template<std::intmax_t num, std::intmax_t den,
	     typename = typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi, decltype(pi()/C<2>())>::value>::type>
    constexpr Unity sin(const C<num, den>&) { return Unity(); }

    template<std::intmax_t num, std::intmax_t den,
	     typename = typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi, decltype(pi()/C<2>())>::value>::type>
    constexpr Null cos(const C<num, den>&) { return Null(); }

    template<std::intmax_t num, std::intmax_t den,
	     typename = typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi >::value>::type>
    constexpr Unity cos(const C<num, den>&) { return Unity(); }

    //Removal of sign via abs on compile-time constants!
    template<std::intmax_t num, std::intmax_t den>
    constexpr C<((num >= 0) ? num : -num), den> abs(const C<num, den>&) { return C<((num >= 0) ? num : -num), den>(); }
    
    namespace detail {
      template<class Config, class T>
      auto try_simplify_imp(const T& a, int) -> decltype(simplify(a)) {
	return simplify<DefaultSimplifyConfig>(a);
      }
      
      template<class Config, class T>
      T try_simplify_imp(const T& a, long) {
	return a;
      }
    }

    template<class Config = DefaultSimplifyConfig, class T>
    auto try_simplify(const T& a) -> decltype(detail::try_simplify_imp<Config>(a,0)) {
      return detail::try_simplify_imp<Config>(a, 0);
    }

    template<class Config, class Arg, size_t Power>
    auto simplify_powerop_impl(const PowerOp<Arg, Power>& f, detail::choice<0>) -> decltype(simplify<Config>(PowerOpSubstitution<Power>::eval(simplify<Config>(f._arg))))
    { return simplify<Config>(PowerOpSubstitution<Power>::eval(simplify<Config>(f._arg))); }
    
    template<class Config, class Arg, size_t Power>
    auto simplify_powerop_impl(const PowerOp<Arg, Power>& f, detail::choice<1>) -> decltype(PowerOpSubstitution<Power>::eval(simplify<Config>(f._arg)))
    { return PowerOpSubstitution<Power>::eval(simplify<Config>(f._arg)); }
    
    template<class Config, class Arg, size_t Power>
    auto simplify_powerop_impl(const PowerOp<Arg, Power>& f, detail::choice<2>) -> decltype(simplify<Config>(PowerOpSubstitution<Power>::eval(f._arg)))
    { return simplify<Config>(PowerOpSubstitution<Power>::eval(f._arg)); }

    template<class Config, class Arg, size_t Power>
    auto simplify_powerop_impl(const PowerOp<Arg, Power>& f, detail::choice<3>) -> decltype(PowerOpSubstitution<Power>::eval(f._arg))
    { return PowerOpSubstitution<Power>::eval(f._arg); }

    template<class T> struct PowerOpEnableExpansion { static const bool value = true; };
    template<char Letter> struct PowerOpEnableExpansion<Variable<Letter> > { static const bool value = false; };

    /*! \brief Expansion operator for PowerOp types. 
    
      This implementation only works if the argument has an simplify
      function defined for its argument.
     */
    template<class Config = DefaultSimplifyConfig, class Arg, size_t Power,
	     typename = typename std::enable_if<PowerOpEnableExpansion<Arg>::value>::type>
    auto simplify(const PowerOp<Arg, Power>& f) -> decltype(simplify_powerop_impl<Config>(f, detail::select_overload{}))
    { return simplify_powerop_impl<Config>(f, detail::select_overload{}); }

    /*! \brief Simplification of a Polynomial LHS multiplied by a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    Polynomial<Order+1, Real, Letter> simplify(const MultiplyOp<Variable<Letter>, Polynomial<Order, Real, Letter> >& f)
    { 
      Polynomial<Order+1, Real, Letter> retval;
      retval[0] = 0;
      std::copy(f._r.begin(), f._r.end(), retval.begin() + 1);
      return retval;
    }

    /*! \brief Simplification of a Polynomial RHS multiplied by a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    Polynomial<Order+1, Real, Letter> simplify(const MultiplyOp<Polynomial<Order, Real, Letter>, Variable<Letter> >& f)
    {
      Polynomial<Order+1, Real, Letter> retval;
      retval[0] = empty_sum(retval[0]);
      std::copy(f._l.begin(), f._l.end(), retval.begin() + 1);
      return retval;
    }

    /*! \brief Simplification of a Polynomial LHS added to a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    Polynomial<Order+1, Real, Letter> simplify(const AddOp<Polynomial<Order, Real, Letter>, Variable<Letter> >& f)
    { 
      Polynomial<(Order > 0) ? Order : 1, Real, Letter> retval(f._l);
      retval[1] += 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial RHS added to a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    Polynomial<Order+1, Real, Letter> simplify(const AddOp<Variable<Letter>, Polynomial<Order, Real, Letter> >& f)
    {
      Polynomial<(Order > 0) ? Order : 1, Real, Letter> retval(f._r);
      retval[1] += 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial LHS subtracted by a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    Polynomial<Order+1, Real, Letter> simplify(const SubtractOp<Polynomial<Order, Real, Letter>, Variable<Letter> >& f)
    {
      Polynomial<(Order > 0) ? Order : 1, Real, Letter> retval(f._l);
      retval[1] -= 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial RHS subtracted by a
      Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    Polynomial<Order+1, Real, Letter> simplify(const SubtractOp<Variable<Letter>, Polynomial<Order, Real, Letter> >& f)
    { 
      Polynomial<(Order > 0) ? Order : 1, Real, Letter> retval(-f._r);
      retval[1] += 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial LHS added to a
      PowerOp of the Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const AddOp<Polynomial<Order, Real, Letter>, PowerOp<Variable<Letter>, POrder> > & f)
    {
      Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(f._l);
      retval[POrder] += 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial RHS added to a
      PowerOp of the Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const AddOp<PowerOp<Variable<Letter>, POrder>, Polynomial<Order, Real, Letter> > & f)
    {
      Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(f._r);
      retval[POrder] += 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial LHS subtracted from a
      PowerOp of the Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const SubtractOp<Polynomial<Order, Real, Letter>, PowerOp<Variable<Letter>, POrder> >& f)
    {
      Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(f._r);
      retval[POrder] -= 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial RHS subtracted from a
      PowerOp of the Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const SubtractOp<PowerOp<Variable<Letter>, POrder>, Polynomial<Order, Real, Letter> >& f)
    {
      Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(-f._r);
      retval[POrder] += Real(1);
      return retval;
    }

    /*! \brief Simplification of a Polynomial LHS multiplied by a
      PowerOp of a Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    Polynomial<Order+POrder, Real, Letter> simplify(const MultiplyOp<PowerOp<Variable<Letter>, POrder>, Polynomial<Order, Real, Letter> >& f)
    {
      Polynomial<Order+POrder, Real, Letter> retval;
      std::copy(f._r.begin(), f._r.end(), retval.begin() + POrder);
      return retval;
    }

    /*! \brief Simplification of a Polynomial RHS multiplied by a
      PowerOp of a Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    Polynomial<Order+POrder, Real, Letter> simplify(const MultiplyOp<Polynomial<Order, Real, Letter>, PowerOp<Variable<Letter>, POrder> >& f)
    {
      Polynomial<Order+POrder, Real, Letter> retval;
      std::copy(f._l.begin(), f._l.end(), retval.begin() + POrder);
      return retval;
    }

    /*! \brief Simplification of a Polynomial LHS added to a
      PowerOp of the Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real, size_t POrder>
    Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> simplify(const AddOp<Polynomial<Order, Real, Letter>, Unity> & f)
    {
      Polynomial<((Order > POrder) ? Order : POrder), Real, Letter> retval(f._l);
      retval[0] += 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial RHS added to a
      PowerOp of the Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    Polynomial<Order, Real, Letter> simplify(const AddOp<Unity, Polynomial<Order, Real, Letter> > & f)
    {
      Polynomial<Order, Real, Letter> retval(f._r);
      retval[0] += 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial LHS subtracted by a
      PowerOp of the Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    Polynomial<Order, Real, Letter> simplify(const SubtractOp<Polynomial<Order, Real, Letter>, Unity> & f)
    {
      Polynomial<Order, Real, Letter> retval(f._l);
      retval[0] -= 1;
      return retval;
    }

    /*! \brief Simplification of a Polynomial RHS subtracted by a
      PowerOp of the Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    Polynomial<Order, Real, Letter> simplify(const SubtractOp<Unity, Polynomial<Order, Real, Letter> > & f)
    {
      Polynomial<Order, Real, Letter> retval(-f._r);
      retval[0] += 1;
      return retval;
    }

    /*! \brief Conversion of PowerOp RHS multiplied by a constant to a
      Polynomial. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, Real, Letter> >::type 
    simplify(const MultiplyOp<PowerOp<Variable<Letter>, Order>, Real>& f)
    {
      Polynomial<Order, Real, Letter> retval;
      retval[Order] = f._r;
      return retval;
    }

    /*! \brief Conversion of PowerOp LHS multiplied by a constant to a
      Polynomial. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, Real, Letter> >::type 
    simplify(const MultiplyOp<Real, PowerOp<Variable<Letter>, Order> >& f)
    {
      Polynomial<Order, Real, Letter> retval;
      retval[Order] = f._l;
      return retval;
    }

    /*! \brief Conversion of Variable RHS multiplied by a constant to a
      Polynomial. */
    template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type
    simplify(const MultiplyOp<Variable<Letter>, Real> & f)
    { return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{toArithmetic(empty_sum(f._r)), toArithmetic(f._r)}; }

    /*! \brief Conversion of Variable LHS multiplied by a constant to a
      Polynomial. */
    template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type 
    simplify(const MultiplyOp<Real, Variable<Letter> > & f)
    { return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{empty_sum(f._l), f._l}; }

    /*! \brief Conversion of PowerOp LHS added with a constant to a
      Polynomial. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> >::type 
    simplify(const AddOp<PowerOp<Variable<Letter>, Order>, Real>& f)
    { 
      Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> retval;
      retval[Order] = 1;
      retval[0] = toArithmetic(f._r);
      return retval;
    }

    /*! \brief Conversion of PowerOp LHS added with a constant to a
      Polynomial. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> >::type 
    simplify(const AddOp<Real, PowerOp<Variable<Letter>, Order> >& f)
    { 
      Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> retval;
      retval[Order] = 1;
      retval[0] = toArithmetic(f._r);
      return retval;
    }

    /*! \brief Conversion of PowerOp LHS subtracted with a constant to a
      Polynomial. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> >::type 
    simplify(const SubtractOp<PowerOp<Variable<Letter>, Order>, Real>& f)
    {
      Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> retval;
      retval[Order] = 1;
      retval[0] = -toArithmetic(f._r);
      return retval;
    }

    /*! \brief Conversion of PowerOp LHS subtracted with a constant to a
      Polynomial. */
    template<class Config = DefaultSimplifyConfig, char Letter, size_t Order, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> >::type 
    simplify(const SubtractOp<Real, PowerOp<Variable<Letter>, Order> >& f)
    {
      Polynomial<Order, STORETYPE(toArithmetic(Real())), Letter> retval;
      retval[Order] = -1;
      retval[0] = toArithmetic(f._l);
      return retval;
    }

    /*! \brief Conversion of a Variable RHS added with a constant. */
    template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type
    simplify(const AddOp<Variable<Letter>, Real>& f)
    { return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{toArithmetic(f._r), Real(1)}; }

    /*! \brief Conversion of a Variable LHS added with a constant. */
    template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type 
    simplify(const AddOp<Real, Variable<Letter> >& f)
    { return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{toArithmetic(f._r), Real(1)}; }


    /*! \brief Conversion of a Variable added with a Variable. */
    template<class Config = DefaultSimplifyConfig, char Letter>
    Polynomial<1, int, Letter>
    simplify(const AddOp<Variable<Letter>, Variable<Letter> >& f)
    { return Polynomial<1, int, Letter>{0, 2}; }


    /*! \brief Conversion of a Variable RHS subtracted with a constant. */
    template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type 
    simplify(const SubtractOp<Variable<Letter>, Real>& f)
    { return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{-toArithmetic(f._r), Real(1)}; }

    /*! \brief Conversion of a Variable LHS subtracted with a constant. */
    template<class Config = DefaultSimplifyConfig, char Letter, class Real>
    typename std::enable_if<detail::IsConstant<Real>::value, Polynomial<1, STORETYPE(toArithmetic(Real())), Letter> >::type
    simplify(const SubtractOp<Real, Variable<Letter> >& f)
    { return Polynomial<1, STORETYPE(toArithmetic(Real())), Letter>{toArithmetic(f._l), Real(-1)}; }

    /*! \brief Right-handed addition operation on a Polynomial.
      
      This operator is only enabled if the type of the Polynomial
      coefficients and the type being added is marked as compatitble
      for distribution over the Polnomial coefficients. This is tested
      using detail::distribute_poly.
    */
    template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    auto simplify(const AddOp<Real1, Polynomial<Order, Real, Letter> >& f) -> Polynomial<Order, STORETYPE(f._l + f._r[0]), Letter>
    {
      Polynomial<Order, STORETYPE(f._l + f._r[0]), Letter> retval(f._r);
      retval[0] += f._l;
      return retval;
    }
    

    /*!\brief Left-handed addition operator for Polynomials 

      This operator is only enabled if the type of the Polynomial
      coefficients and the type being added is marked as compatitble
      for distribution over the Polnomial coefficients. This is tested
      using detail::distribute_poly.
    */
    template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    auto simplify(const AddOp<Polynomial<Order, Real, Letter>, Real1>& f) -> Polynomial<Order, STORETYPE(f._l[0] + f._r), Letter>
    {
      Polynomial<Order, STORETYPE(f._l[0] + f._r), Letter> retval(f._l);
      retval[0] += f._r;
      return retval;
    }

    /*!\brief Addition operator for two Polynomial types. 
     */
    template<class Config = DefaultSimplifyConfig, class Real1, size_t M, class Real2, size_t N, char Letter>
    auto simplify(const AddOp<Polynomial<M, Real1, Letter>, Polynomial<N, Real2, Letter> >& f) -> Polynomial<detail::max_order(M, N), STORETYPE(f._l[0] + f._r[0]), Letter>
    {
      Polynomial<detail::max_order(M, N), STORETYPE(f._l[0] + f._r[0]), Letter> retval;

      for (size_t i(0); i <= std::min(N, M); ++i)
	retval[i] = f._l[i] + f._r[i];
      
      for (size_t i(std::min(N, M)+1); i <= N; ++i)
	retval[i] = f._l[i];

      for (size_t i(std::min(N, M)+1); i <= M; ++i)
	retval[i] = f._r[i];
      
      return retval;
    }

    /*! \brief Right-handed subtraction operator for Polynomial types.
     
      This will reorder and convert the operation to a unary negation
      operator with an addition if the left-handed addition form
      exists.
    */
    template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    auto simplify(const SubtractOp<Real1, Polynomial<Order, Real, Letter> >& f)
      -> STATOR_AUTORETURN(simplify<Config>(f._l + (-f._r)));
    
    /*! \brief Left-handed subtraction from a Polynomial type.
      
      This will convert the operation to a unary negation operator
      with an addition if the left-handed form exists.
    */
    template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    auto simplify(const SubtractOp<Polynomial<Order, Real, Letter>, Real1>& f)
      -> STATOR_AUTORETURN(simplify<Config>(f._l + (-f._r)));

    template<class Real1, class Real2, size_t N, char Letter,
	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real2>::value>::type >
    auto operator-(const Polynomial<N,Real1,Letter>& poly, const Real2& r) 
      -> STATOR_AUTORETURN(poly + (-r))

    /*! \brief Subtraction between two Polynomial types. 
     */
    template<class Config = DefaultSimplifyConfig, class Real1, size_t M, class Real2, size_t N, char Letter>
    auto simplify(const SubtractOp<Polynomial<M, Real1, Letter>, Polynomial<N, Real2, Letter> >& f) -> Polynomial<detail::max_order(M, N), STORETYPE(f._l[0] - f._r[0]), Letter>
    {
      Polynomial<detail::max_order(M, N), STORETYPE(f._l[0] - f._r[0]), Letter> retval;
      for (size_t i(0); i <= std::min(N, M); ++i)
	retval[i] = f._l[i] - f._r[i];
      
      for (size_t i(std::min(N, M)+1); i <= N; ++i)
	retval[i] = f._l[i];

      for (size_t i(std::min(N, M)+1); i <= M; ++i)
	retval[i] = -f._r[i];
      
      return retval;
    }

    /*! \brief Right-handed multiplication operation on a Polynomial.
      
      This operator is only enabled if the type of the Polynomial
      coefficients and the type being added is marked as compatitble
      for distribution over the Polnomial coefficients. This is tested
      using detail::distribute_poly.
    */
    template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    auto simplify(const MultiplyOp<Real1, Polynomial<Order, Real, Letter> >& f) -> Polynomial<Order, STORETYPE(f._l * f._r[0]), Letter>
    {
      Polynomial<Order, STORETYPE(f._l * f._r[0]), Letter> retval;

      for (size_t i(0); i <= Order; ++i)
	retval[i] = f._l * f._r[i];

      return retval;
    }

    /*! \brief Left-handed multiplication on a Polynomial.

      This operator is only enabled if the type of the Polynomial
      coefficients and the type being added is marked as compatitble
      for distribution over the Polnomial coefficients. This is tested
      using detail::distribute_poly.
    */
    template<class Config = DefaultSimplifyConfig, class Real1, size_t Order, class Real, char Letter,
	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real>::value>::type>
    auto simplify(const MultiplyOp<Polynomial<Order, Real, Letter>, Real1>& f) -> Polynomial<Order, STORETYPE(f._l[0] * f._r), Letter>
    {
      Polynomial<Order, STORETYPE(f._l[0] * f._r), Letter> retval;
      for (size_t i(0); i <= Order; ++i)
	retval[i] = f._l[i] * f._r;
      return retval;
    }

    /*! \brief Multiplication between two Polynomial types.
     */
    template<class Config = DefaultSimplifyConfig, class Real1, class Real2, size_t M, size_t N, char Letter>
    auto simplify(const MultiplyOp<Polynomial<M, Real1, Letter>, Polynomial<N, Real2, Letter> >& f)
      -> Polynomial<M + N, STORETYPE(f._l[0] * f._r[0]), Letter>
    {
      Polynomial<M + N, STORETYPE(f._l[0] * f._r[0]), Letter> retval;
      for (size_t i(0); i <= N+M; ++i)
	for (size_t j(i>N?i-N:0); (j <= i) && (j <=M); ++j)
	  retval[i] += f._l[j] * f._r[i-j];
      return retval;
    }

    /*! \brief Specialisation for squares of matrix expressions. */
    template<class Config = DefaultSimplifyConfig, size_t Power, class Matrix, size_t N, char Letter,
             typename = typename std::enable_if<(Power==2) && std::is_base_of<Eigen::EigenBase<Matrix>, Matrix>::value>::type>
    auto simplify(const PowerOp<Polynomial<N, Matrix, Letter>, Power>& f)
      -> Polynomial<2 * N, STORETYPE(f._arg[0].dot(f.arg[0])), Letter>
    { 
      Polynomial<2 * N, STORETYPE(f._arg[0].dot(f._arg[0])), Letter> retval;
      for (size_t i(0); i <= 2 * N; ++i)
	for (size_t j(i>N?i-N:0); (j <= i) && (j <=N); ++j)
	  retval[i] += f._arg[j].dot(f._arg[i-j]);
      return retval;
    }

    /*! \brief Division of a Polynomial by a constant. */
    template<class Config = DefaultSimplifyConfig, class Real1, class Real2, size_t N, char Letter,
	     typename = typename std::enable_if<detail::distribute_poly<Real1, Real2>::value>::type>
    auto simplify(const DivideOp<Polynomial<N, Real1, Letter>, Real2> & f) -> Polynomial<N, STORETYPE(f._l[0] / f._r), Letter>
    {
      Polynomial<N, STORETYPE(f._l[0] / f._r), Letter> retval;
      for (size_t i(0); i <= N; ++i)
	retval[i] = f._l[i] / f._r;
      return retval;
    }

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
