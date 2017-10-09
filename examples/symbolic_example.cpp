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

#include <iostream>
#include <stator/symbolic/symbolic.hpp>

int main(int argc, char **argv) {
  using namespace sym;
  //The default variable is the letter x
  Var<> x;
  
  //But you can specify your own variables by letter:
  Var<vidx<'y'> > y;
  
  //Or by index:
  Var<vidx<42>> v42;
  //We have to catch symbolic expressions using "auto" as their type is very complex
  auto f1 = x * x + sin(y);
  
  std::cout << f1 << std::endl;
  //Output: ((x × x) + sin(y))
  auto f1_xsub = sub(f1, x = y + 2);
  
  std::cout << f1_xsub << std::endl;
  //Output: (((y + 2) × (y + 2)) + sin(y))
  std::cout << sub(f1_xsub, y = 3.14159265359) << std::endl;
  //Output: 26.436...
  auto f2 = (3-3) * x;
  std::cout << f2 << std::endl;
  //Output: (0 × x)
  C<1, 2> half;
  C<2> two;
  
  //Compile-time rational arithmetic!
  auto three = half + half + two;

  //The variable three is of type "C<3>", thus it must be computed at compile time.
  assert((std::is_same<decltype(three), C<3> >::value)); //does not fail
  
  std::cout << three << std::endl;
  //Output: C<3>()
  auto f3 = (C<1>() - C<1>()) * sin(x);

  std::cout << f3 << std::endl;
  //Output: C<0>()
  auto f4 = C<1>() * sin(x);
  std::cout << f4 << std::endl;
  //Output: sin(x)
  auto f5 = cos(2 * x);
  auto df5_dx = derivative(f5, x);

  std::cout << df5_dx << std::endl;
  //Output: (-2 × sin((2 × x)))
  //A taylor series of sin(2x) around zero
  std::cout << taylor_series<3>(sin(C<2>() * x), C<0>(), x) << std::endl;
  //Output: (x × (C<2>() + ((x ^ C<2>()) × C<-4,3>())))
  std::cout << x * x * x * x * x << std::endl;
  //Output: ((((x × x) × x) × x) × x)

  std::cout << simplify(x * x * x * x * x) << std::endl;
  //Output: (x ^ C<5>())         (which is equivalent to "pow(x, C<5>())")
  
  auto f6 = taylor_series<3>(sin(C<2>() * x), C<0>(), x);
  std::cout << f6 << std::endl;
  //Output: (x × (C<2>() + ((x ^ C<2>()) × C<-4,3>())))

  auto f6_poly = expand(f6);
  std::cout << f6_poly << std::endl;
  //Output: P(-1.33333 × x³ + 2 × x)

  std::cout << f6_poly[0] << std::endl;
  //Output: 0
  std::cout << f6_poly[3] << std::endl;
  //Output: -1.33333
  //Then we can analyse its real roots!
  auto f6_roots = solve_real_roots(f6_poly);

  std::cout << f6_roots << std::endl;
  //Output: StackVector{ -1.22474 0 1.22474 }

  //Extracting the number of roots
  std::cout << f6_roots.size() << std::endl;
  //Output: 3
  
  //Extracting a value of a root
  std::cout << f6_roots[2] << std::endl;
  //Output: 1.22474
  Eigen::Matrix<double, 1, 3> r{1.0, 2.0, 3.0};
  Eigen::Matrix<double, 1, 3> v{1.0, 0.5, 0.1};
  auto fvec = r + x * v;
  std::cout << sub(fvec, x = 12) << std::endl;
  //Output:  13   8 4.2

  {//Quick example
    
    Var<vidx<'x'> > x; //Create a variable "x"
    Var<vidx<'y'> > y; //Create a variable "y"
    
    auto f = sin(x) + 2 * cos(x); //Define a function f(x)

    //Perform a substitution
    auto g = sub(f, x = y*y); //g(y)=f(y^2)

    //Print the function to screen
    std::cout << g << std::endl;
    //Output: (sin((y × y)) + (2 × cos((y × y))))

    //Evaluate the function
    double a = sub(g, y = 2.3); // a = g(2.3)
    std::cout << a << std::endl;
    //Output: 0.254279
    auto dg_dy = derivative(g, y);  // dg/dy
    std::cout << dg_dy << std::endl;
    //Output: (((y + y) × cos((y × y))) + (2 × ((C<-1>() × (y + y)) × sin((y × y)))))
    //Take a 5th order taylor series of the derivative around y=0
    auto poly = expand(taylor_series<5>(dg_dy, C<0>(), y));
    std::cout << poly << std::endl;
    //Output: P(-1 × y^5 + -4 × y³ + 2 × y)

    //Calculate the real roots of this 5th order polynomial
    std::cout << solve_real_roots(poly) << std::endl;
    //Output: StackVector{ -0.67044 0.67044 }
    Expr f_rt = "sin(x) + 2 * cos(x)";
    Expr g_rt = sub(f_rt, x=y*y);
    a = simplify(sub(g_rt, y = 2.3)).as<double>(); // a = g(2.3)
    Expr dg_dy_rt = derivative(g_rt, y);
  }//End of the example
}
