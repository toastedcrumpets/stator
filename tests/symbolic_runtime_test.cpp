/*
  Copyright (C) 2017 Marcus Bannerman <m.bannerman@gmail.com>

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
#define UNIT_TEST_SUITE_NAME Symbolic_Runtime
#define UNIT_TEST_GOOGLE
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

UNIT_TEST( symbolic_rt_constants_direct )
{
  auto l = ConstantRT<double>::create(2);
  auto r = ConstantRT<double>::create(2);
  UNIT_TEST_CHECK_EQUAL(*l, *r);
  UNIT_TEST_CHECK_EQUAL(*l, 2.0);
  UNIT_TEST_CHECK_EQUAL(*l, 2);
}

UNIT_TEST( symbolic_rt_simplify )
{
  UNIT_TEST_CHECK_EQUAL(Expr("0*2"), Expr("0"));
  Expr f = Expr("0") * Expr("2");
  f = simplify(f);
  UNIT_TEST_CHECK_EQUAL(f, Expr("0"));
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
  f = Expr(double(3.0)) + Expr(double(2));
  UNIT_TEST_CHECK_EQUAL(5.0, simplify(f).as<double>());

  f = Expr(double(3.0)) - Expr(double(2));
  UNIT_TEST_CHECK_EQUAL(1.0, simplify(f).as<double>());

  f = Expr(double(3.0)) * Expr(double(2));
  UNIT_TEST_CHECK_EQUAL(6.0, simplify(f).as<double>());

  f = Expr(double(3.0)) / Expr(double(2));
  UNIT_TEST_CHECK_EQUAL(3.0/2, simplify(f).as<double>());

  f = Expr(pow(Expr(3.0), Expr(2)));
  UNIT_TEST_CHECK_EQUAL(9.0, simplify(f).as<double>());
}

static constexpr char y_str[] = "y";

UNIT_TEST( symbolic_rt_variables )
{
  Var<> x;
  Var<y_str> y;
  Expr f;

  f = Expr(x);
  compare_expression(f, "x");

  f = Expr(x * x);
  compare_expression(f, x * x);

  f = sub(f, Expr(x = y));
  compare_expression(f, y * y);

  f = sub(f, Expr(y = 3.0));
  UNIT_TEST_CHECK_EQUAL(9.0, simplify(f).as<double>());
}

UNIT_TEST( symbolic_comparison_operator )
{
  Var< > x;
  UNIT_TEST_CHECK_EQUAL(Expr(2.0), Expr(2.0));
  UNIT_TEST_CHECK_EQUAL(Expr(2), Expr(2.0));
  UNIT_TEST_CHECK_EQUAL(Expr(2.0), Expr(2));
  UNIT_TEST_CHECK_EQUAL(Expr(x), Expr(x));  
  UNIT_TEST_CHECK_EQUAL(Expr(Expr(x)*Expr(x)), Expr(Expr(x)*Expr(x)));  
  UNIT_TEST_CHECK_EQUAL(Expr(Expr(2.0)+Expr(1.0)), Expr(Expr(2.0)+Expr(1.0)));
  UNIT_TEST_CHECK_EQUAL(Expr(sin(x)), Expr(sin(x)));
  UNIT_TEST_CHECK(Expr(sin(x)) != Expr(cos(x)));
  UNIT_TEST_CHECK(Expr(log(x)) != Expr(exp(x)));
}

namespace sym {
  template<class Var> auto tderivative(const UnaryOp<Expr, detail::Cosine>& f, const Var& x)
    -> STATOR_AUTORETURN(-derivative(f._arg, x) * sym::sin(f._arg));
}

UNIT_TEST( symbolic_rt_unary_ops )
{
  Var< > x;

  Expr f = Expr(sin(x));
  compare_expression(Expr(f), sin(x));
  Expr df = simplify(derivative(f, Expr(x)));
  compare_expression(df, cos(x));
  UNIT_TEST_CHECK_CLOSE(simplify(sub(f, Expr(x=1.2))).as<double>(), 0.9320390859672263, 0.000000001);
 
  f = Expr(exp(x));
  compare_expression(Expr(f), exp(x));
  df = simplify(derivative(f, Expr(x)));
  compare_expression(df, exp(x));

  f = Expr(cos(x));

  auto x_ptr = VarRT::create("x");
  auto& x_2 = *x_ptr;

  compare_expression(Expr(f), cos(x));
  auto ptr = cos(Expr(x));
  df = simplify(sym::derivative(*ptr, x_2));
  compare_expression(df, -1 * sin(x));
  UNIT_TEST_CHECK_CLOSE(simplify(sub(f, Expr(x=1.2))).as<double>(), 0.3623577544766736, 0.000000001);

  f = Expr(log(x));
  compare_expression(Expr(f), log(x));
  df = simplify(derivative(f, Expr(x)));
  compare_expression(df, 1/x);
  UNIT_TEST_CHECK_CLOSE(simplify(sub(f, Expr(x=1.2))).as<double>(), 0.1823215567939546, 0.000000001);


  f = Expr(exp(log(x)));
  df = derivative(f, Expr(x));
}

UNIT_TEST( symbolic_mixed_expr )
{
  Var< > x;
  
  auto f = Expr(x + x * Expr(x));
  auto g = sub(f, Expr(x=2));
  UNIT_TEST_CHECK_EQUAL(simplify(Expr(g)).as<double>(), 6.0);
}

UNIT_TEST( symbolic_runtime_derivative )
{
  auto x_ptr = VarRT::create("x");
  auto& x = *x_ptr;
  UNIT_TEST_CHECK_EQUAL(derivative(Expr("2.2"), x), Expr("0"));
  UNIT_TEST_CHECK_EQUAL(derivative(Expr("2.2"), x), Expr("0"));
  UNIT_TEST_CHECK_EQUAL(simplify(derivative(Expr("2*x"), x)), Expr("2"));
  UNIT_TEST_CHECK_EQUAL(simplify(derivative(Expr("x*x"), x)), Expr("x+x"));
  UNIT_TEST_CHECK_EQUAL(simplify(derivative(Expr("x+x"), x)), Expr("2"));
  UNIT_TEST_CHECK_EQUAL(simplify(derivative(Expr("ln(x)"), x)), Expr("1/x"));
  UNIT_TEST_CHECK_EQUAL(simplify(derivative(Expr("exp(x)"), x)), Expr("exp(x)"));
  UNIT_TEST_CHECK_EQUAL(simplify(derivative(Expr("x*exp(x)"), x)), Expr("exp(x)+x*exp(x)"));
}


UNIT_TEST( symbolic_dict_basic )
{
  auto v_ptr = Dict::create();
  auto& v = *v_ptr;

  auto x_ptr = VarRT::create("x");
  auto y_ptr = VarRT::create("y");
  auto& x = *x_ptr;
  auto& y = *y_ptr;
  v[x] = Expr(2);
  v[y] = Expr(3);
  UNIT_TEST_CHECK_EQUAL(sub(Expr("x"), v), Expr("2"));
  UNIT_TEST_CHECK_EQUAL(sub(Expr("y"), v), Expr("3"));
}

UNIT_TEST( symbolic_sub_general )
{
  UNIT_TEST_CHECK_EQUAL(sub(Expr("x"), Expr("x=2")), Expr("2"));
  UNIT_TEST_CHECK_EQUAL(sub(Expr("x"), Expr("{x:2, y:3}")), Expr("2"));
}
