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

//boost
#define BOOST_TEST_MODULE Polynomial_test
#include <boost/test/included/unit_test.hpp>
#include <boost/mpl/assert.hpp>

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

#define CHECK_TYPE(EXPRESSION, TYPE) \
  BOOST_MPL_ASSERT_MSG((std::is_same<decltype(EXPRESSION), TYPE>::value), TYPE_COMPARE, (decltype(EXPRESSION), TYPE));

BOOST_AUTO_TEST_CASE( poly_taylor )
{ 
  using namespace sym;
  Var<vidx<'y'> > y;

  BOOST_CHECK(compare_expression(taylor_series<3>(y*y*y, Null(), Var<vidx<'x'> >()), simplify(y*y*y)));
  
  BOOST_CHECK(compare_expression(taylor_series<3>(y*y*y, Null(), y), simplify(y*y*y)));

  //Test truncation of PowerOp expressions when the original order is too high
  CHECK_TYPE(taylor_series<2>(y*y*y, Null(), y), Null);
  
  //Test simple Taylor expansion of sine 
  BOOST_CHECK(compare_expression(taylor_series<6>(sin(y), Null(), y), y * (C<1>()+(pow(y, C<2>())*(C<-1,6>()+(pow(y, C<2>())*C<1,120>()))))));

  //MSVC only supports limited symbol names lengths, thus deep
  //templating is not possible, and these tests fail on compilation
#if !(defined(_MSC_VER) && !defined(__INTEL_COMPILER))
  BOOST_CHECK(compare_expression(taylor_series<8>(sin(y*y), Null(), y), pow(y, C<2>()) * (C<1>()+(pow(y, C<4>())*C<-1,6>()))));
  
  //Test Taylor expansion of a complex expression at zero
  Var<vidx<'x'> > x;
  auto f = sin(cos(x)+2*x*x - x + 3);
  auto ffinal = simplify((3.0 * std::sin(4.0)/2.0 + (std::cos(4.0)/6.0)) * x*x*x + (3*std::cos(4.0)/2.0 - std::sin(4.0)/2.0) * x*x - std::cos(4.0) * x + std::sin(4.0));

  //We compare the expressions as Polynomials using the expand
  auto ffinal_expanded = expand(ffinal);
  BOOST_CHECK(compare_expression(expand(taylor_series<3>(f, Null(), x)), ffinal_expanded));

  //Test Taylor expansion again at a non-zero location
  BOOST_CHECK(compare_expression(expand(taylor_series<3>(sin(cos(x)+2*x*x - x + 3), 3.0, x)), expand(82.77908670866608 * x*x*x - 688.8330378984795 * x*x + 1895.079543801394 * x - 1721.740734454172)));

  //Partially truncate a Polynomial through expansion
  BOOST_CHECK(compare_expression(expand(taylor_series<2>(Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4}, Null(), y)), Polynomial<2,int,Var<vidx<'y'> > >{1,2,3}));
  
  //Keep the order the same
  BOOST_CHECK(compare_expression(expand(taylor_series<3>(Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4}, Null(), y)), Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4}));
  
  //Taylor simplify at a higher order
  BOOST_CHECK(compare_expression(expand(taylor_series<4>(Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4}, Null(), y)), Polynomial<3,int,Var<vidx<'y'> > >{1,2,3,4})); 
#endif
}

