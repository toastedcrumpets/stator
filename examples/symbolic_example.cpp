/*! \file symbolic_example.cpp
  \brief An example of how to use the stator::symbolic library.

  More details are given in the \ref symbolic_guide "guide".
*/
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

#include <stator/symbolic/symbolic.hpp>
#include <iostream>

int main(int argc, char **argv) {
  using namespace sym;

  //Sym extends std::ratio into its own constant class, C.
  C<1, 2> half;
  C<2> two;
  //This supports standard arithmetic operators
  auto three = half + half + two;
  //three is of type "C<3>", which is computed at compile time.
  assert((std::is_same<decltype(three), C<3> >::value));
  std::cout << three << std::endl;
  //Output: ©3
  // The copyright symbol is used to indicate its a compile time constant.
  
  //The default variable is the letter x
  Var<> x;
  //Other variables can be declared as numbers:
  Var<vidx<2>> var2;
  //or as letters
  Var<vidx<'y'> > y;

  //These allow symbolic expressions
  auto f1 = x * x + sin(y);
  std::cout << f1 << std::endl;
  //Output: ((x × x) + sin(y))

  //Compile-time constants allow compile-time cancellation
  auto f2 = (C<1>() - C<1>()) * sin(x);
  assert((std::is_same<decltype(f2), C<0> >::value));
  std::cout << f2 << std::endl;
  //Output: ©0

  //And basic simplification
  auto f3 = C<1>() * sin(x);
  std::cout << f3 << std::endl;
  assert((std::is_same<decltype(f3), decltype(sin(x))>::value));
  
  //Standard arithmetic terms can also be used, but they only have limited symbolic support
  auto f4 = x * x * (1.5 + 2 * x) - 3 * x;
  std::cout << f4 << std::endl;
  //Output: (((x × x) × (1.5 + (2 × x))) - (3 × x))

  //To perform analysis functions there are some standard forms which
  //are needed.  One example is the polynomial class which is obtained
  //using expansion:
  auto f5 = expand(f4);
  std::cout << f5 << std::endl;
  //Output: P(2 × x³ + 1.5 × x² + -3 × x)

  //Then we can analyse its real roots!
  auto f5_roots = solve_real_roots(f5);
  std::cout << f5_roots << std::endl;
  //Output: StackVector{ -1.65587 0 0.905869 }

  //Roots are always returned from smallest to largest in a
  //StackVector type, which is a stack allocated std::vector clone.

  
//  //Evaluation/substitution at a point x=2
//  std::cout << substitution(simplef, x == 2) << std::endl;
//  //Output: 16
//  //A symbolic substitution, replacing x with x+2:
//  std::cout << simplify(substitution(simplef, x == x + 2)) << std::endl;
//  //Output: 2 × x³ + 13.5 × x² + 27 × x + 16
//  auto f2 = 4 * x * cos(2*x+2);
//  std::cout << simplify(derivative(f2, x)) << std::endl;
//  //Output: ((4) × (cos(2 × x + 2))) + ((-8 × x) × (sin(2 × x + 2)))
//  //5th order Taylor expansion about zero in x
//  std::cout << taylor_series<5>(4 * x * cos(2*x+2), 0.0, x) << std::endl;
//  //Output: -1.10972 × x^5 + 4.84959 × x^4 + 3.32917 × x³ + -7.27438 × x² + -1.66459 × x
//  stator::Vector<double, 3> r{1.0, 2.0, 3.0};
//  stator::Vector<double, 3> v{1.0, 0.5, 0.1};
//  auto fvec = r + x * v;
//  std::cout << substitution(fvec, x == 12) << std::endl;
//  //Output: 13 8 4.2
//  //The following will not compile as fvec is a column vector:
//  //fvec * fvec;
//  auto polyfvec = simplify(fvec);
//  std::cout << pow<2>(polyfvec) << std::endl;
//  //Output: P(1.26 × x² + 4.6 × x + 14)
}
