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

#include <iostream>
//stator
#include <stator/symbolic/runtime.hpp>
#include <stator/unit_test.hpp>

#include <random>

std::mt19937 RNG;
std::normal_distribution<double> normal_dist(0, 1);
std::uniform_real_distribution<double> angle_dist(0, std::atan(1)*4);

using namespace sym;
typedef Eigen::Matrix<double, 3, 1, Eigen::DontAlign> Vector;

Vector random_unit_vec() {
  Vector vec{normal_dist(RNG), normal_dist(RNG), normal_dist(RNG)};
  return vec.normalized();
}

template<class A, class B>
struct Check_Type {
  static_assert(std::is_same<typename std::decay<A>::type, B>::value, "Type comparison failed");
};

#define CHECK_TYPE(EXPRESSION, TYPE) static_assert(std::is_same<decltype(EXPRESSION), TYPE>::value, "Type is wrong")


template<class T1, class T2>
void compare_expression(const T1& f, const T2& g, bool output_error=true) {
  std::ostringstream os;
  os << f;
  std::string f_str = os.str();
  os.str(""); os.clear();
  os << g;
  std::string g_str = os.str();
  UNIT_TEST_CHECK_EQUAL(f_str, g_str);
}


UNIT_TEST( symbolic_rt_constants )
{
  //Check the Expr operators are working
  Expr f = Expr(1.0) + Expr(2.0);
  Expr g = simplify(f);
  UNIT_TEST_CHECK_EQUAL(3.0, g.as<double>());

  f = Expr(1.0) - Expr(2.0);
  g = simplify(f);
  UNIT_TEST_CHECK_EQUAL(-1, g.as<double>());

  f = Expr(2.0) * Expr(3.0);
  g = simplify(f);
  UNIT_TEST_CHECK_EQUAL(6, g.as<double>());

  f = Expr(2.0) / Expr(3.0);
  g = simplify(f);
  UNIT_TEST_CHECK_EQUAL(2.0/3.0, g.as<double>());
  
  f = pow(Expr(2.0), Expr(3.0));
  g = simplify(f);
  UNIT_TEST_CHECK_EQUAL(8, g.as<double>());


  //Check that the particular type operators are working
  f = ConstantRT<double>(3.0) + ConstantRT<double>(2);
  UNIT_TEST_CHECK_EQUAL(5.0, simplify(f).as<double>());

  f = ConstantRT<double>(3.0) - ConstantRT<double>(2);
  UNIT_TEST_CHECK_EQUAL(1.0, simplify(f).as<double>());

  f = ConstantRT<double>(3.0) * ConstantRT<double>(2);
  UNIT_TEST_CHECK_EQUAL(6.0, simplify(f).as<double>());

  f = ConstantRT<double>(3.0) / ConstantRT<double>(2);
  UNIT_TEST_CHECK_EQUAL(3.0/2, simplify(f).as<double>());

  f = pow(ConstantRT<double>(3.0),ConstantRT<double>(2));
  UNIT_TEST_CHECK_EQUAL(9.0, simplify(f).as<double>());
}

UNIT_TEST( symbolic_rt_variables )
{
  Var<vidx<'x'> > x;
  Var<vidx<'y'> > y;
  Expr f;

  f = x;  
  compare_expression(f, "x");

  f = x * x;
  compare_expression(f, x * x);

  f = sub(f, x = y);
  compare_expression(f, y * y);

  f = sub(f, y = 3.0);
  UNIT_TEST_CHECK_EQUAL(9.0, simplify(f).as<double>());
}

UNIT_TEST( symbolic_comparison_operator )
{
  Var<vidx<'x'> > x;
  UNIT_TEST_CHECK_EQUAL(Expr(2.0), Expr(2.0));
  UNIT_TEST_CHECK_EQUAL(Expr(2), Expr(2.0));
  UNIT_TEST_CHECK_EQUAL(Expr(2.0), Expr(2));
  UNIT_TEST_CHECK_EQUAL(Expr(x), Expr(x));  
  UNIT_TEST_CHECK_EQUAL(Expr(Expr(x)*Expr(x)), Expr(Expr(x)*Expr(x)));  
  UNIT_TEST_CHECK_EQUAL(Expr(Expr(2.0)+Expr(1.0)), Expr(Expr(2.0)+Expr(1.0)));
}

UNIT_TEST( symbolic_rt_unary_ops )
{
  Var<vidx<'x'> > x;
  //As f is Expr, assignment forces runtime conversion
  Expr f;

  f = sin(x);
  compare_expression(f, sin(x));
  Expr df = simplify(derivative(f, x));
  compare_expression(df, cos(x));
}
