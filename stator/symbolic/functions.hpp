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
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////        Standard functions         /////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////

    /*! \brief Function types are symbolic representation of
        non-polynomial functions.
    */
    template<class Arg, size_t FUNC_ID>
    class Function {
    public:
      /*! \brief The symbolic expression which is the argument of the
          Function. 
      */
      Arg _arg;
      
      /*! \brief Construct a Function with its argument. */
      Function(const Arg& a): _arg(a) {}
    };

    /*! \brief Enabling of symbolic operators for Function types 
     */
    template<class Arg, size_t FuncID>
    struct SymbolicOperators<Function<Arg, FuncID> > {
      static const bool value = true;
    };

#define CREATE_FUNCTION(HELPERNAME, NUMERIC_IMPL, ARG_DERIVATIVE, TEXT_REPRESENTATION, ID) \
    template <class Arg> using HELPERNAME##F = Function<Arg, ID>;	\
    template<class A,							\
	     typename = typename std::enable_if<!std::is_arithmetic<A>::value>::type> \
    HELPERNAME##F<A> HELPERNAME(const A& a) { return a; } \
									\
    template<class A,							\
	     typename = typename std::enable_if<std::is_arithmetic<A>::value>::type> \
    auto HELPERNAME(const A& x) -> decltype(NUMERIC_IMPL)		\
    { return NUMERIC_IMPL; }						\
									\
    template<char Letter, class Arg1, class Arg2>			\
    auto substitution(const Function<Arg1, ID>& f, const VariableSubstitution<Letter, Arg2>& x) \
      -> STATOR_AUTORETURN(HELPERNAME(substitution(f._arg, x)))         \
									\
    template<class A>							\
    inline std::ostream& operator<<(std::ostream& os, const Function<A, ID>& f) \
    { return os << TEXT_REPRESENTATION; }			\
									\
    template<char dVariable, class A>					\
    auto derivative(const HELPERNAME##F<A>& f, Variable<dVariable>)	\
      -> STATOR_AUTORETURN(derivative(f._arg, Variable<dVariable>()) * (ARG_DERIVATIVE)) \
    
    CREATE_FUNCTION(sin, std::sin(x), cos(f._arg), "sin(" << f._arg << ")", 0)
    CREATE_FUNCTION(cos, std::cos(x), -sin(f._arg), "cos(" << f._arg << ")", 1)
    CREATE_FUNCTION(abs, std::abs(x), f._arg / f, "|" << f._arg << "|", 2)
    CREATE_FUNCTION(arbsign, arbsign(std::abs(x)), arbsign(Unity()), "±|" << f._arg << "|", 3)

    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////         Complex functions         /////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////    
  }
}
