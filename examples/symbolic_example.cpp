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

  //Constants, but note the value is
  //actually the type:
  C<1, 2> half;
  C<2> two;
  auto three = half + half + two;
  //three is of type "C<3>"
  
  //Fractional constants are possible as well as C inherits from std::ratio
  C<1, 4> quarter;
  Var<0> var0;
  Var<2> var2;
  Var<'x'> x;
  Var<'y'> y;
  assert((std::is_same<decltype(sin(x) * C<0>()), C<0> >::value));
  assert((std::is_same<decltype(C<1>() * Variable<'x'>()), Variable<'x'> >::value));
  auto f = x * x * (1.5 + 2 * x) - 3 * x;
  std::cout << f << std::endl;
  //output: (((x)^2) × ((1.5) + ((2) × (x)))) - ((3) × (x))
  auto simplef = simplify(f);
  std::cout << simplef << std::endl;
  //output: P(2 × x³ + 1.5 × x² + -3 × x)
  std::cout << solve_real_roots(simplef) << std::endl;
  //output: StackVector{ -1.65587 0 0.905869 }
  //Evaluation/substitution at a point x=2
  std::cout << substitution(simplef, x == 2) << std::endl;
  //output: 16
  //A symbolic substitution, replacing x with x+2:
  std::cout << simplify(substitution(simplef, x == x + 2)) << std::endl;
  //output: 2 × x³ + 13.5 × x² + 27 × x + 16
  auto f2 = 4 * x * cos(2*x+2);
  std::cout << simplify(derivative(f2, x)) << std::endl;
  //output: ((4) × (cos(2 × x + 2))) + ((-8 × x) × (sin(2 × x + 2)))
  //5th order Taylor expansion about zero in x
  std::cout << taylor_series<5>(4 * x * cos(2*x+2), 0.0, x) << std::endl;
  //output: -1.10972 × x^5 + 4.84959 × x^4 + 3.32917 × x³ + -7.27438 × x² + -1.66459 × x
  stator::Vector<double, 3> r{1.0, 2.0, 3.0};
  stator::Vector<double, 3> v{1.0, 0.5, 0.1};
  auto fvec = r + x * v;
  std::cout << substitution(fvec, x == 12) << std::endl;
  //output: 13 8 4.2
  //The following will not compile as fvec is a column vector:
  //fvec * fvec;
  auto polyfvec = simplify(fvec);
  std::cout << pow<2>(polyfvec) << std::endl;
  //output: P(1.26 × x² + 4.6 × x + 14)
}
