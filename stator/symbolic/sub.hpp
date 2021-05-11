/*
  Copyright (C) 2021 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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
  /*! \brief Default implementation of substitution of a symbolic
    expression at a given point.
    
    This implementation only applies if the expression is a constant term.

    We deliberately return by const reference as, if this is an
    Eigen expression, the Eigen library may take an internal
    reference to this object to allow delayed evaluation. By
    returning the original object we can try to ensure its lifetime
    is at least longer than the current expression.
  */
  template<class T, class Var, class Arg,
	   typename = typename std::enable_if<detail::IsConstant<T>::value>::type >
  auto sub(const T& f, const EqualityOp<Var, Arg>&) { return store(f); }
  
  /*! \brief Evaluates a symbolic Var at a given point.
    
    This is only used if the Var is correct 
    substitution.
  */
  template<conststr N1, conststr N2, typename ...Args1, typename ...Args2, class Arg,
	   typename = typename enable_if_var_eq<Var<N1, Args1...>, Var<N2, Args2...> >::type>
  auto sub(const Var<N1, Args1...>& f, const EqualityOp<Var<N2, Args2...>, Arg>& x)
  { return store(x._r); }

  /*! \brief Evaluates a symbolic Var at a given point.
    
    This is only used if the Var is not the correct letter for the
    substitution.
  */
  template<conststr N1, class ...Args1, class Arg, class Var2,
	   typename = typename enable_if_var_not_eq<Var<N1, Args1...>, Var2>::type>
  Var<N1, Args1...> sub(const Var<N1, Args1...>& f, const EqualityOp<Var2, Arg>& x)
  { return f; }
}
