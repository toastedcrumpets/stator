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

#include <ratio>

namespace sym {
  /*! \brief A class representing a compile-time rational constant (i.e., std::ratio).

    Fundamentally this is a std::ratio which has been extended to
    support standard "run-time" ops, such as multiplication.
  */
  template<std::intmax_t Num, std::intmax_t Denom = 1>
  struct C: std::ratio<Num, Denom> {
    operator double() const {
	return double(Num) / double(Denom);
    }
  };

  namespace detail {
    /*! \brief Conversion operator from std::ratio to C.*/
    template<class stdratio>
    struct C_wrap {
      typedef C<stdratio::num, stdratio::den> type;
    };
  }
  
  /*! \brief Compile time type-test for compile-time constants \ref C.*/
  template<class T> struct is_C { static const bool value = false; };

  template<std::intmax_t N, std::intmax_t D> struct is_C<C<N,D> > { static const bool value = true; };

  /*! \brief A symbolic representation of zero. */
  typedef C<0> Null;
  /*! \brief A symbolic representation of one. */
  typedef C<1> Unity;
  
  /*! \brief A symbolic/compile-time rational approximation of \f$\pi\f$. */
  typedef detail::C_wrap<stator::constant_ratio::pi>::type pi;

  /*! \brief A symbolic/compile-time rational approximation of \f$\mathrm{e}\f$. */
  typedef detail::C_wrap<stator::constant_ratio::e>::type e;

  /*! \brief Output operator for compile-time constant (\ref C types).*/
  template<std::intmax_t Num, std::intmax_t Denom>
  inline std::ostream& operator<<(std::ostream& os, const C<Num, Denom>) {
    os << "C<" << Num;
    if (Denom != 1)
	os << "," << Denom;
    return os << ">()";
  }
  
  /*! \brief Output operator specialised for one quarter.*/
  inline std::ostream& operator<<(std::ostream& os, const C<1,4>) {
    return (os << "©¼");
    
  }

  /*! \brief Output operator specialised for one half.*/
  inline std::ostream& operator<<(std::ostream& os, const C<1,2>) {
    return (os << "©½");
  }

  /*! \brief Output operator specialised for three quarters.*/
  inline std::ostream& operator<<(std::ostream& os, const C<3,4>) {
    return (os << "©¾");
  }
  
  /*! \brief Specialized output operator for \f$\pi\f$.*/
  inline std::ostream& operator<<(std::ostream& os, const pi) { os << "π"; return os; }
  /*! \brief Specialized output operator for \f$\mathrm{e}\f$.*/
  inline std::ostream& operator<<(std::ostream& os, const e) { os << "e"; return os; }

  /*! \brief Symbolic Factorial function.
   
    This template implementation returns Unity for 0! and 1!,
    allowing simplification of symbolic expressions.
  */
  template<size_t i> struct Factorial {
    typedef C<i * Factorial<i - 1>::value::num, 1> value;
  };
  
  template<> struct Factorial<1> {
    typedef C<1, 1> value;
  };

  template<> struct Factorial<0> {
    typedef C<1, 1> value;
  };

  template<std::intmax_t n1, std::intmax_t d1, std::intmax_t n2, std::intmax_t d2>
  constexpr auto operator+(C<n1, d1>, C<n2, d2>) -> STATOR_AUTORETURN((typename detail::C_wrap<std::ratio_add<std::ratio<n1,d1>, std::ratio<n2,d2> > >::type()));

  template<std::intmax_t n1, std::intmax_t d1, std::intmax_t n2, std::intmax_t d2>
  constexpr auto operator-(C<n1, d1>, C<n2, d2>) -> STATOR_AUTORETURN((typename detail::C_wrap<std::ratio_subtract<std::ratio<n1,d1>, std::ratio<n2,d2> > >::type()));

  template<std::intmax_t n1, std::intmax_t d1, std::intmax_t n2, std::intmax_t d2>
  constexpr auto operator*(C<n1, d1>, C<n2, d2>) -> STATOR_AUTORETURN((typename detail::C_wrap<std::ratio_multiply<std::ratio<n1,d1>, std::ratio<n2,d2> > >::type()));

  template<std::intmax_t n1, std::intmax_t d1, std::intmax_t n2, std::intmax_t d2>
  constexpr auto operator/(C<n1, d1>, C<n2, d2>) -> STATOR_AUTORETURN((typename detail::C_wrap<std::ratio_divide<std::ratio<n1,d1>, std::ratio<n2,d2> > >::type()));

  template<std::intmax_t n1, std::intmax_t d1, std::intmax_t n2, std::intmax_t d2>
  constexpr bool operator==(const C<n1, d1>&, const C<n2, d2>&)
  { return std::ratio_equal<std::ratio<n1, d1>, std::ratio<n2, d2> >::value; }
  
  template<class T, typename = typename std::enable_if<!is_C<T>::value>::type>
  T operator+(const T& l, Null) { return l; }
  template<class T, typename = typename std::enable_if<!is_C<T>::value>::type>
  T operator+(Null, const T& r) { return r; }
  template<class T, typename = typename std::enable_if<!is_C<T>::value>::type>
  T operator-(const T& l, Null) { return l; }
  template<class T, typename = typename std::enable_if<!is_C<T>::value>::type>
  auto operator-(Null, const T& r) -> STATOR_AUTORETURN(-r);
  template<class T, typename = typename std::enable_if<!is_C<T>::value>::type>
  Null operator*(const T&, Null) { return Null(); }
  template<class T, typename = typename std::enable_if<!is_C<T>::value>::type>
  Null operator*(Null, const T&) { return Null(); }
  
  template<class T, typename = typename std::enable_if<!is_C<T>::value>::type>
  auto operator*(const T& a, Unity) -> STATOR_AUTORETURN(a);
  template<class T, typename = typename std::enable_if<!is_C<T>::value>::type>
  auto operator*(Unity, const T& a) -> STATOR_AUTORETURN(a);
  
  /*! \brief Symbolic Inverse factorial function.
   
    This template implementation returns Unity for 1/0! and 1/1!,
    allowing simplification of symbolic expressions.
  */
  template<size_t i> struct InvFactorial {
    typedef C<Factorial<i>::value::den, Factorial<i>::value::num> value;
  };
  
  template<class T>
  inline std::ostream& operator<<(std::ostream& os, const std::complex<T>& c) {
    if (c.imag() == 0)
	return (os << c.real());
    if (c.imag() < 0)
	return (os << "(" <<c.real() << " - " << std::abs(c.imag()) << "i)");
    return (os << "(" <<c.real() << " + " << c.imag() << "i)");
  }

  template<class C_arg, class factor, class offset = std::ratio<0> >
  struct is_whole_factor {
    static const bool value = (std::ratio_divide<std::ratio_subtract<std::ratio<C_arg::num, C_arg::den>, std::ratio<offset::num, offset::den> >, std::ratio<factor::num, factor::den> >::den == 1);
  };

  //simplify sin(i * pi)
  template<std::intmax_t num, std::intmax_t den,
	   typename = typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi>::value>::type>
  constexpr Null sin_Cimpl(const C<num, den>& a, detail::choice<0>) { return {}; }

  template<std::intmax_t num, std::intmax_t den,
	   typename = typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi, decltype(pi() / C<2>())>::value>::type>
  constexpr Unity sin_Cimpl(const C<num, den>& a, detail::choice<0>) { return {}; }

  template<std::intmax_t num, std::intmax_t den,
	   typename = typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi>::value>::type>
  constexpr Unity cos_Cimpl(const C<num, den>& a, detail::choice<0>) { return {}; }

  template<std::intmax_t num, std::intmax_t den,
	   typename = typename std::enable_if<is_whole_factor<std::ratio<num, den>, pi, decltype(pi() / C<2>())>::value>::type>
  constexpr Null cos_Cimpl(const C<num, den>& a, detail::choice<0>) { return {}; }

  template<std::intmax_t num, std::intmax_t den>
  auto sin(const C<num, den>& a) -> STATOR_AUTORETURN(sin_Cimpl(a, detail::select_overload{}));  

  template<std::intmax_t num, std::intmax_t den>
  auto cos(const C<num, den>& a) -> STATOR_AUTORETURN(cos_Cimpl(a, detail::select_overload{}));

  template<std::intmax_t num, std::intmax_t den>
  constexpr C<(1 - 2 * (num < 0)) * num, (1 - 2 * (den < 0)) * den> abs(const C<num, den>& a) { return {}; }
}
