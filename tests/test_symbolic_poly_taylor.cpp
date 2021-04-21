/*
  Copyright (C) 2015 Marcus Bannerman <m.bannerman@gmail.com>

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


//C++
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>

//stator
#include <stator/symbolic/symbolic.hpp>
#include <stator/unit_test.hpp>

#include <random>

std::mt19937 RNG;
std::normal_distribution<double> normal_dist(0, 1);
std::uniform_real_distribution<double> angle_dist(0, std::atan(1)*4);

typedef Eigen::Matrix<double, 3, 1, Eigen::DontAlign> Vector;

Vector random_unit_vec() {
  Vector vec{normal_dist(RNG), normal_dist(RNG), normal_dist(RNG)};
  return vec.normalized();
}

bool err(double val, double expected)
{
  return std::abs(val / expected - 1) < 0.0001;
}

template<class T1, class T2>
bool compare_expression(const T1& f, const T2& g) {
  using namespace sym;
  std::ostringstream os;
  os << f;
  std::string f_str = os.str();
  os.str(""); os.clear();
  os << g;
  std::string g_str = os.str();
  if (!(f_str == g_str))
    std::cerr << f << " != " << g << std::endl;
  return f_str == g_str;
}

UNIT_TEST( poly_taylor )
{ 
  using namespace sym;
  Var<vidx<'y'> > y;

  UNIT_TEST_CHECK(compare_expression(taylor_series<3>(y*y*y, Null(), Var<vidx<'x'> >()), simplify(y*y*y)));
  
  UNIT_TEST_CHECK(compare_expression(taylor_series<3>(y*y*y, Null(), y), simplify(y*y*y)));

  //Test truncation of PowerOp expressions when the original order is too high
  UNIT_TEST_CHECK(compare_expression(taylor_series<2>(y*y*y, Null(), y), Null()));
  
  //Test simple Taylor expansion of sine 
  UNIT_TEST_CHECK(compare_expression(taylor_series<6>(sin(y), Null(), y), y * (C<1>()+(pow(y, C<2>())*(C<-1,6>()+(pow(y, C<2>())*C<1,120>()))))));

  //MSVC only supports limited symbol names lengths, thus deep
  //templating is not possible, and these tests fail on compilation
#if !(defined(_MSC_VER) && !defined(__INTEL_COMPILER))
  //00:24
  //UNIT_TEST_CHECK(compare_expression(taylor_series<8>(sin(y*y), Null(), y), pow(y, C<2>()) * (C<1>()+(pow(y, C<4>())*C<-1,6>()))));

  //0:54
  //Test Taylor expansion of a complex expression at zero
  //Var<vidx<'x'> > x;
  //auto f = sin(cos(x)+2*x*x - x + 3);
  //auto ffinal = simplify((3.0 * std::sin(4.0)/2.0 + (std::cos(4.0)/6.0)) * x*x*x + (3*std::cos(4.0)/2.0 - std::sin(4.0)/2.0) * x*x - std::cos(4.0) * x + std::sin(4.0));

  //1:23
  //We compare the expressions as Polynomials using the expand
  //auto ffinal_expanded = expand(ffinal);
  //UNIT_TEST_CHECK(compare_expression(expand(taylor_series<3>(f, Null(), x)), ffinal_expanded));

  //2:20
  //Test Taylor expansion again at a non-zero location
  //  UNIT_TEST_CHECK(compare_expression(expand(taylor_series<3>(sin(cos(x)+2*x*x - x + 3), 3.0, x)), expand(82.779086708666071 * x*x*x - 688.83303789847946 * x*x + 1895.0795438013947 * x - 1721.7407344541725)));

  //3:30

  //Partially truncate a Polynomial through expansion
  //UNIT_TEST_CHECK(compare_expression(expand(taylor_series<2>(Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4}, Null(), y)), Polynomial<2,int,Var<vidx<'y'> > >{1,2,3}));
  //
  ////Keep the order the same
  //UNIT_TEST_CHECK(compare_expression(expand(taylor_series<3>(Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4}, Null(), y)), Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4}));
  //
  ////Taylor simplify at a higher order
  //UNIT_TEST_CHECK(compare_expression(expand(taylor_series<4>(Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4}, Null(), y)), Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4})); 
#endif
}
