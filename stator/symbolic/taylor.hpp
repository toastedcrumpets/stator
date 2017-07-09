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

// stator
#include <stator/symbolic/symbolic.hpp>

namespace sym {
  namespace detail {
    template<size_t State, size_t max_Order, class Var>
    struct TaylorSeriesWorker {
	template<class Real>
	static Null eval(const Null& f, const Real& a)
	{ return Null(); }
      
	template<class F, class Real>
	static auto eval(const F& f, const Real& a) 
        -> STATOR_AUTORETURN((typename InvFactorial<State>::value() * sub(f, Var() = a) + (Var() - a) * TaylorSeriesWorker<State+1, max_Order, Var>::eval(derivative(f, Var()), a)))
    };

    template<size_t max_Order, class Var>
    struct TaylorSeriesWorker<max_Order,max_Order, Var> {
	template<class F, class Real>
	static auto eval(const F& f, const Real& a)
        -> STATOR_AUTORETURN((typename InvFactorial<max_Order>::value() * sub(f, Var() = a)));
      
	template<class Real>
	static Null eval(const Null& f, const Real& a)
	{ return Null(); }
    };
  }

  /*! \brief Generate a Taylor series representation of a Symbolic
    expression.
  */
  template<size_t Order, class Var, class F, class Real>
  auto taylor_series(const F& f, Real a, Var)
    -> STATOR_AUTORETURN(try_simplify(detail::TaylorSeriesWorker<0, Order, Var>::eval(f, a)));
} // namespace symbolic
