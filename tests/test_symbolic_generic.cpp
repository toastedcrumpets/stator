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

//stator
#include <stator/symbolic/symbolic.hpp>
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
bool compare_expression(const T1& f, const T2& g, bool output_error=true) {
  std::ostringstream os;
  os << f;
  std::string f_str = os.str();
  os.str(""); os.clear();
  os << g;
  std::string g_str = os.str();
  if (!(f_str == g_str) && output_error)
    std::cerr << f << " != " << g << std::endl;
  return f_str == g_str;
}


UNIT_TEST( symbolic_C )
{
  Var<> x;
  Check_Type<decltype(Unity() * Unity()), Unity>();
  Check_Type<decltype(Unity() + Unity()), C<2>>();
  Check_Type<decltype(Unity() + Null()), Unity>();
  Check_Type<decltype(Null() + Unity()), Unity>();

  Check_Type<std::decay<decltype(sub(Unity(), x = 100))>::type, Unity>();

  Check_Type<decltype(Unity() * Null()), Null>();

  Check_Type<std::decay<decltype(sub(C<5>(), x = 2))>::type, C<5>>();
  Check_Type<decltype((C<5>() - C<3>() - C<2>()) * x), Null>();

  UNIT_TEST_CHECK(compare_expression(pi()*pi()/pi(), "π"));
  UNIT_TEST_CHECK(compare_expression(e(), "e"));

  Check_Type<decltype(sin(Null())), Null>();
  Check_Type<decltype(sin(pi())), Null>();
  Check_Type<decltype(sin(C<12>()*pi())), Null>();
  Check_Type<decltype(sin(C<5,2>()*pi())), Unity>();

  Check_Type<decltype(cos(Null())), Unity>();
  Check_Type<decltype(cos(pi())), Unity>();
  Check_Type<decltype(cos(C<8>()*pi())), Unity>();
  Check_Type<decltype(cos(C<5,2>()*pi())), Null>();

  Var<vidx<'y'>> y;
  Check_Type<std::decay<decltype(sub(Null(), y=100))>::type, Null>();
  Check_Type<decltype(derivative(Null(), x)), Null>();
  Check_Type<decltype(derivative(Unity(),x)), Null>();
  Check_Type<decltype(sub(Null(), y=100)), Null>();
  //Check some subs
  Check_Type<std::decay<decltype(sub(y*y*y, y = Null()))>::type, Null>();
  Check_Type<decltype(derivative(2, x)), Null>();
  Check_Type<decltype(derivative(3.141, x)), Null>();

  Check_Type<decltype((Vector{1,2,3} * Null())), Null>();
  UNIT_TEST_CHECK(compare_expression(Vector{1,2,3} + Null(), Vector{1,2,3}));
}

UNIT_TEST( Unity_tests )
{
  //Check that Null symbols have zero derivative and value

  UNIT_TEST_CHECK(compare_expression(Unity()+ 1.1, 2.1));


  //Check derivatives of Unity
  UNIT_TEST_CHECK(compare_expression(derivative(Unity(), Var<>()), Null()));

  Var<> x;
  //Check simplification of multiplication with Unity
  UNIT_TEST_CHECK(compare_expression(Unity() * Unity(), Unity()));
  UNIT_TEST_CHECK(compare_expression(Unity() * 2, 2));
  UNIT_TEST_CHECK(compare_expression(Unity() * x, x));

  UNIT_TEST_CHECK(compare_expression(2 * Unity(), 2));
  UNIT_TEST_CHECK(compare_expression(x * Unity() * x, x * x));
}

UNIT_TEST( Sub_of_variables )
{
  Var<> x;
  Var<vidx<'y'>> y;

  Check_Type<std::decay<decltype(sub(x, x=y))>::type, Var<vidx<'y'>>>();
}

UNIT_TEST( function_basic )
{
  Var<> x;
  //Check basic Function operation
  UNIT_TEST_CHECK_CLOSE(sub(2 * x, x=0.5), 1.0, 1e-10);
    
  UNIT_TEST_CHECK_CLOSE(sub(sin(x), x=0.5), std::sin(0.5), 1e-10);
  UNIT_TEST_CHECK_CLOSE(sub(cos(x), x=0.5), std::cos(0.5), 1e-10);

  //Test BinaryOP Addition and subtraction
  UNIT_TEST_CHECK_CLOSE(sub(x * sin(x) + x, x=0.5), 0.5 * std::sin(0.5) + 0.5, 1e-10);
  UNIT_TEST_CHECK_CLOSE(sub(x * sin(x) - x, x=0.5), 0.5 * std::sin(0.5) - 0.5, 1e-10);
}

UNIT_TEST( function_multiplication )
{
  Var<> x;
  //Check function and Polynomial multiplication
  auto poly1 = sin(x + x) * x;
  UNIT_TEST_CHECK_CLOSE(sub(poly1, x=0.5), std::sin(2 * 0.5) * 0.5, 1e-10);
  auto poly2 = x * sin(x + x);
  UNIT_TEST_CHECK_CLOSE(sub(poly2, x=0.5), std::sin(2 * 0.5) * 0.5, 1e-10);
}

UNIT_TEST( function_derivatives )
{
  Var<> x;
  //Check function and Polynomial derivatives
  auto f1 = derivative(x * sin(x), x);
  UNIT_TEST_CHECK_CLOSE(sub(f1, x=0.5), std::sin(0.5) + 0.5 * std::cos(0.5), 1e-10);
  auto f2 = derivative(x * cos(x), x);
  UNIT_TEST_CHECK_CLOSE(sub(f2, x=0.5), -0.5 * std::sin(0.5) + std::cos(0.5), 1e-10);
}

UNIT_TEST( power_basic )
{
  Var<> x;
  UNIT_TEST_CHECK(sym::pow(3, C<3>()) == 27);
  UNIT_TEST_CHECK(sym::pow(Vector{0,1,2}, C<2>()) == 5);

  UNIT_TEST_CHECK_CLOSE(sub(pow(x, C<3>()), x=4.0), 4.0*4.0*4.0, 1e-10);
  UNIT_TEST_CHECK_CLOSE(sub(pow(x, C<3>()), x=0.75), std::pow(0.75, 3), 1e-10);

  //Test PowerOp algebraic operations
  UNIT_TEST_CHECK_CLOSE(sub(pow(x, C<3>()) - x, x=0.75), std::pow(0.75, 3) - 0.75, 1e-10);
  UNIT_TEST_CHECK_CLOSE(sub(pow(x, C<3>()) + x, x=0.75), std::pow(0.75, 3) + 0.75, 1e-10);
  UNIT_TEST_CHECK_CLOSE(sub(pow(x, C<3>()) * x, x=0.75), std::pow(0.75, 3) * 0.75, 1e-10);

  //Check special case derivatives
  Check_Type<decltype(derivative(pow(x, C<1>()), Var<>())), Unity>();
}

UNIT_TEST( Var_tests )
{
  Var<> x;
  Var<vidx<'y'>> y;

  UNIT_TEST_CHECK(compare_expression(x, "x")); 
  UNIT_TEST_CHECK(compare_expression(y, "y"));
  UNIT_TEST_CHECK(compare_expression(derivative(x, Var<>()), Unity()));
  UNIT_TEST_CHECK(compare_expression(derivative(y, Var<>()), Null()));
  UNIT_TEST_CHECK(compare_expression(derivative(y, Var<vidx<'y'>>()), Unity()));
  UNIT_TEST_CHECK(compare_expression(sub(x, x = 3.14159265), 3.14159265));

  //Check that subs in the wrong variable do nothing
  UNIT_TEST_CHECK(compare_expression(sub(y, x = 3.14159265), "y"));

  //Check default sub is for x
  UNIT_TEST_CHECK(compare_expression(sub(y, x = 3.14159265), "y"));

  //Check that Var derivatives are correct
  UNIT_TEST_CHECK(compare_expression(derivative(sin(x), Var<>()), cos(x)));

  //Check derivatives of Unity
  UNIT_TEST_CHECK(compare_expression(derivative(Unity(), Var<>()), Null()));
  UNIT_TEST_CHECK(compare_expression(derivative(x, Var<>()), Unity()));
  UNIT_TEST_CHECK(compare_expression(derivative(x * sin(x), Var<>()), sin(x) + x * cos(x)));
}

UNIT_TEST( reorder_operations )
{
  Var<> x;
  //Check the specialised multiply operators are consistently
  //simplifying statements.

  //Again, check that negative matches are correctly determined by
  //compare_expression
  UNIT_TEST_CHECK(!compare_expression(x, sin(x), false));

  //Here we're looking for the two Polynomial terms to be reordered
  UNIT_TEST_CHECK(compare_expression(simplify((sin(2*x) * x) * x), sin(2*x) * pow(x, C<2>())));
  UNIT_TEST_CHECK(compare_expression(simplify((x * sin(2*x)) * x), pow(x, C<2>()) * sin(2*x)));
  UNIT_TEST_CHECK(compare_expression(simplify(x * (sin(2*x) * x)), pow(x, C<2>()) * sin(2*x)));
  UNIT_TEST_CHECK(compare_expression(simplify(x * (x * sin(2*x))), pow(x, C<2>()) * sin(2*x)));

  //Here we check that constants (such as 2) will become Null
  //types when the derivative is taken, causing their terms to be
  //eliminated.
  UNIT_TEST_CHECK(compare_expression(simplify(derivative(C<2>() * cos(x), Var<>())), C<-2>() * sin(x)));
  UNIT_TEST_CHECK(compare_expression(derivative(2 * sin(x), Var<>()), 2 * cos(x)));
}

UNIT_TEST( Factorial_test )
{
  static_assert(Factorial<0>::value::num == 1, "0! != 1");
  static_assert(Factorial<1>::value::num == 1, "1! != 1");
  static_assert(Factorial<3>::value::num == 6, "3! != 6");
  static_assert(Factorial<3>::value::den == 1, "Base isn't 1!");

  static_assert(InvFactorial<0>::value::den == 1, "0! != 1");
  static_assert(InvFactorial<1>::value::den == 1, "1! != 1");
  static_assert(InvFactorial<3>::value::den == 6, "3! != 6");
  static_assert(InvFactorial<3>::value::num == 1, "Num isn't 1!");
}


UNIT_TEST( vector_symbolic )
{
  static_assert(sym::detail::IsConstant<Vector>::value, "Vectors are not considered constant!");
  
  UNIT_TEST_CHECK(compare_expression(derivative(Vector{1,2,3}, Var<>()), Null()));
  UNIT_TEST_CHECK(compare_expression(Unity() * Vector{1,2,3}, Vector{1,2,3}));
  UNIT_TEST_CHECK(compare_expression(Vector{1,2,3} * Unity(), Vector{1,2,3}));

  Var<> x;

  const size_t testcount = 100;
  const double errlvl = 1e-10;

  Vector test1 = sub(Vector{0,1,2} * x, x = 2);
  
  UNIT_TEST_CHECK(test1[0] == 0);
  UNIT_TEST_CHECK(test1[1] == 2);
  UNIT_TEST_CHECK(test1[2] == 4);

  //Implementation of the Rodriugues formula symbolically.
  RNG.seed();
  for (size_t i(0); i < testcount; ++i)
    {
      double angle = angle_dist(RNG);
      Vector axis = random_unit_vec().normalized();
      Vector start = random_unit_vec();
      Vector end = Eigen::AngleAxis<double>(angle, axis) * start;
      
      Vector r = axis * axis.dot(start);
      auto f = (start - r) * cos(x) + axis.cross(start) * sin(x) + r;
      Vector err = end - sub(f, x=angle);
      
      UNIT_TEST_CHECK(std::abs(err[0]) < errlvl);
      UNIT_TEST_CHECK(std::abs(err[1]) < errlvl);
      UNIT_TEST_CHECK(std::abs(err[2]) < errlvl);
    }

  UNIT_TEST_CHECK(toArithmetic(Vector{1,2,3}) == (Vector{1,2,3}));
}

UNIT_TEST( symbolic_abs_arbsign )
{
  Var<> x;

  UNIT_TEST_CHECK(compare_expression(abs(x), "|x|"));
  UNIT_TEST_CHECK_EQUAL(sub(abs(x*x - 5*x), x=2), 6);
  UNIT_TEST_CHECK_EQUAL(C<6/-2>::num, -3);  
  UNIT_TEST_CHECK_EQUAL(C<-8/-4>::num, 2);
  UNIT_TEST_CHECK(compare_expression(abs(Unity()), Unity()));
  UNIT_TEST_CHECK(compare_expression(abs(Null()), Null()));
  UNIT_TEST_CHECK(compare_expression(abs(Null()), Null()));
  UNIT_TEST_CHECK(compare_expression(derivative(arbsign(x), x), arbsign(Unity())));
  UNIT_TEST_CHECK(compare_expression(derivative(arbsign(x), Var<vidx<'y'>>()), Null()));

  UNIT_TEST_CHECK(compare_expression(simplify(x * arbsign(x)), arbsign(pow(x, C<2>()))));
  UNIT_TEST_CHECK(compare_expression(simplify(arbsign(x) * x), arbsign(pow(x, C<2>()))));
  UNIT_TEST_CHECK(compare_expression(simplify(arbsign(x) * arbsign(x)), arbsign(pow(x, C<2>()))));

  UNIT_TEST_CHECK(compare_expression(simplify(arbsign(x) / x), arbsign(Unity())));
  UNIT_TEST_CHECK(compare_expression(simplify(arbsign(x) / arbsign(x)), arbsign(Unity())));
  UNIT_TEST_CHECK(compare_expression(simplify(arbsign(arbsign(x))), arbsign(x)));
  simplify(pow(arbsign(x), C<5>()));
  UNIT_TEST_CHECK(compare_expression(simplify(pow(arbsign(x), C<5>())), arbsign(pow(x, C<5>()))));
  UNIT_TEST_CHECK(compare_expression(simplify(pow(arbsign(x), C<6>())), pow(x, C<6>())));
}

UNIT_TEST( derivative_addition )
{
  using namespace sym;

  //Test Polynomial derivatives on addition Operation types
  Var<> x;

  UNIT_TEST_CHECK(compare_expression(simplify(derivative(simplify(C<2>() * x * x + x), x)), C<4>() * x + C<1>()));
}
