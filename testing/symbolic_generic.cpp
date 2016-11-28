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
#include <stator/symbolic/symbolic.hpp>

//boost
#define BOOST_TEST_MODULE Symbolic_math_test
#include <boost/test/included/unit_test.hpp>

#include <random>

std::mt19937 RNG;
std::normal_distribution<double> normal_dist(0, 1);
std::uniform_real_distribution<double> angle_dist(0, std::atan(1)*4);

using namespace stator::symbolic;
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


BOOST_AUTO_TEST_CASE( symbolic_C )
{
  Variable<> x;
  Check_Type<decltype(Unity() * Unity()), Unity>();
  Check_Type<decltype(Unity() + Unity()), C<2>>();
  Check_Type<decltype(Unity() + Null()), Unity>();
  Check_Type<decltype(Null() + Unity()), Unity>();

  Check_Type<std::decay<decltype(substitution(Unity(), x == 100))>::type, Unity>();

  Check_Type<decltype(Unity() * Null()), Null>();

  Check_Type<std::decay<decltype(substitution(C<5>(), x == 2))>::type, C<5>>();
  Check_Type<decltype((C<5>() - C<3>() - C<2>()) * x), Null>();

  BOOST_CHECK(compare_expression(pi()*pi()/pi(), "π"));
  BOOST_CHECK(compare_expression(e(), "e"));

  Check_Type<decltype(sin(Null())), Null>();
  Check_Type<decltype(sin(pi())), Null>();
  Check_Type<decltype(sin(C<12>()*pi())), Null>();
  Check_Type<decltype(sin(C<5,2>()*pi())), Unity>();

  Check_Type<decltype(cos(Null())), Unity>();
  Check_Type<decltype(cos(pi())), Unity>();
  Check_Type<decltype(cos(C<8>()*pi())), Unity>();
  Check_Type<decltype(cos(C<5,2>()*pi())), Null>();

  Variable<vidx<'y'>> y;
  Check_Type<std::decay<decltype(substitution(Null(), y==100))>::type, Null>();
  Check_Type<decltype(derivative(Null(), x)), Null>();
  Check_Type<decltype(derivative(Unity(),x)), Null>();
  Check_Type<decltype(substitution(Null(), y==100)), Null>();
  //Check some substitutions
  Check_Type<std::decay<decltype(substitution(y*y*y, y == Null()))>::type, Null>();
  Check_Type<decltype(derivative(2, x)), Null>();
  Check_Type<decltype(derivative(3.141, x)), Null>();

  Check_Type<decltype((Vector{1,2,3} * Null())), Null>();
  BOOST_CHECK(compare_expression(Vector{1,2,3} + Null(), Vector{1,2,3}));
}

BOOST_AUTO_TEST_CASE( Unity_tests )
{
  //Check that Null symbols have zero derivative and value

  BOOST_CHECK(compare_expression(Unity()+ 1.1, 2.1));


  //Check derivatives of Unity
  BOOST_CHECK(compare_expression(derivative(Unity(), Variable<>()), Null()));

  Variable<> x;
  //Check simplification of multiplication with Unity
  BOOST_CHECK(compare_expression(Unity() * Unity(), Unity()));
  BOOST_CHECK(compare_expression(Unity() * 2, 2));
  BOOST_CHECK(compare_expression(simplify<>(Unity() * x), x));

  BOOST_CHECK(compare_expression(2 * Unity(), 2));
  BOOST_CHECK(compare_expression(simplify<>(x * Unity() * x), simplify(x * x)));
}

BOOST_AUTO_TEST_CASE( Substitution_of_variables )
{
  Variable<> x;
  Variable<vidx<'y'>> y;

  Check_Type<std::decay<decltype(substitution(x, x==y))>::type, Variable<vidx<'y'>>>();
}

BOOST_AUTO_TEST_CASE( function_basic )
{
  Variable<> x;
  //Check basic Function operation
  BOOST_CHECK_CLOSE(substitution(2 * x, x==0.5), 1.0, 1e-10);
    
  BOOST_CHECK_CLOSE(substitution(sin(x), x==0.5), std::sin(0.5), 1e-10);
  BOOST_CHECK_CLOSE(substitution(cos(x), x==0.5), std::cos(0.5), 1e-10);

  //Test BinaryOP Addition and subtraction
  BOOST_CHECK_CLOSE(substitution(x * sin(x) + x, x==0.5), 0.5 * std::sin(0.5) + 0.5, 1e-10);
  BOOST_CHECK_CLOSE(substitution(x * sin(x) - x, x==0.5), 0.5 * std::sin(0.5) - 0.5, 1e-10);
}

BOOST_AUTO_TEST_CASE( function_multiplication )
{
  Variable<> x;
  //Check function and Polynomial multiplication
  auto poly1 = sin(x + x) * x;
  BOOST_CHECK_CLOSE(substitution(poly1, x==0.5), std::sin(2 * 0.5) * 0.5, 1e-10);
  auto poly2 = x * sin(x + x);
  BOOST_CHECK_CLOSE(substitution(poly2, x==0.5), std::sin(2 * 0.5) * 0.5, 1e-10);
}

BOOST_AUTO_TEST_CASE( function_derivatives )
{
  Variable<> x;
  //Check function and Polynomial derivatives
  auto f1 = derivative(x * sin(x), x);
  BOOST_CHECK_CLOSE(substitution(f1, x==0.5), std::sin(0.5) + 0.5 * std::cos(0.5), 1e-10);
  auto f2 = derivative(x * cos(x), x);
  BOOST_CHECK_CLOSE(substitution(f2, x==0.5), -0.5 * std::sin(0.5) + std::cos(0.5), 1e-10);
}

BOOST_AUTO_TEST_CASE( power_basic )
{
  Variable<> x;
  BOOST_CHECK(pow<3>(3) == 27);
  BOOST_CHECK(pow<2>(Vector{0,1,2}) == 5);

  BOOST_CHECK_CLOSE(substitution(pow<3>(x), x==4.0), 4.0*4.0*4.0, 1e-10);
  BOOST_CHECK_CLOSE(substitution(pow<3>(x), x==0.75), std::pow(0.75, 3), 1e-10);

  //Test PowerOp algebraic operations
  BOOST_CHECK_CLOSE(substitution(pow<3>(x) - x, x==0.75), std::pow(0.75, 3) - 0.75, 1e-10);
  BOOST_CHECK_CLOSE(substitution(pow<3>(x) + x, x==0.75), std::pow(0.75, 3) + 0.75, 1e-10);
  BOOST_CHECK_CLOSE(substitution(pow<3>(x) * x, x==0.75), std::pow(0.75, 3) * 0.75, 1e-10);

  //Check special case derivatives
  Check_Type<decltype(derivative(pow<1>(x), Variable<>())), Unity>();
}

BOOST_AUTO_TEST_CASE( Var_tests )
{
  Variable<> x;
  Variable<vidx<'y'>> y;

  BOOST_CHECK(compare_expression(x, "x")); 
  BOOST_CHECK(compare_expression(y, "y"));
  BOOST_CHECK(compare_expression(derivative(x, Variable<>()), Unity()));
  BOOST_CHECK(compare_expression(derivative(y, Variable<>()), Null()));
  BOOST_CHECK(compare_expression(derivative(y, Variable<vidx<'y'>>()), Unity()));
  BOOST_CHECK(compare_expression(substitution(x, x == 3.14159265), 3.14159265));

  //Check that substitutions in the wrong variable do nothing
  BOOST_CHECK(compare_expression(substitution(y, x == 3.14159265), "y"));

  //Check default substitution is for x
  BOOST_CHECK(compare_expression(substitution(y, x == 3.14159265), "y"));

  //Check that Var derivatives are correct
  BOOST_CHECK(compare_expression(simplify(derivative(sin(x), Variable<>())), cos(x)));

  //Check derivatives of Unity
  BOOST_CHECK(compare_expression(derivative(Unity(), Variable<>()), Null()));
  BOOST_CHECK(compare_expression(derivative(x, Variable<>()), Unity()));
  BOOST_CHECK(compare_expression(simplify(derivative(x * sin(x), Variable<>())), sin(x) + x * cos(x)));
}

BOOST_AUTO_TEST_CASE( reorder_operations )
{
  Variable<> x;
  //Check the specialised multiply operators are consistently
  //simplifying statements.

  //Again, check that negative matches are correctly determined by
  //compare_expression
  BOOST_CHECK(!compare_expression(x, sin(x), false));

  //Here we're looking for the two Polynomial terms to be reordered
  BOOST_CHECK(compare_expression(simplify((sin(2*x) * x) * x), sin(2*x) * pow<2>(x)));
  BOOST_CHECK(compare_expression(simplify((x * sin(2*x)) * x), pow<2>(x) * sin(2*x)));
  BOOST_CHECK(compare_expression(simplify(x * (sin(2*x) * x)), pow<2>(x) * sin(2*x)));
  BOOST_CHECK(compare_expression(simplify(x * (x * sin(2*x))), pow<2>(x) * sin(2*x)));

  //Here we check that constants (such as 2) will become Null
  //types when the derivative is taken, causing their terms to be
  //eliminated.
  BOOST_CHECK(compare_expression(simplify(derivative(C<2>() * cos(x), Variable<>())), C<-2>() * sin(x)));
  BOOST_CHECK(compare_expression(simplify(derivative(2 * sin(x), Variable<>())), 2 * cos(x)));
}

BOOST_AUTO_TEST_CASE( Factorial_test )
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


BOOST_AUTO_TEST_CASE( vector_symbolic )
{
  static_assert(stator::symbolic::detail::IsConstant<Vector>::value, "Vectors are not considered constant!");
  
  BOOST_CHECK(compare_expression(derivative(Vector{1,2,3}, Variable<>()), Null()));
  BOOST_CHECK(compare_expression(Unity() * Vector{1,2,3}, Vector{1,2,3}));
  BOOST_CHECK(compare_expression(Vector{1,2,3} * Unity(), Vector{1,2,3}));

  Variable<> x;

  const size_t testcount = 100;
  const double errlvl = 1e-10;

  Vector test1 = substitution(Vector{0,1,2} * x, x == 2);
  
  BOOST_CHECK(test1[0] == 0);
  BOOST_CHECK(test1[1] == 2);
  BOOST_CHECK(test1[2] == 4);

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
      Vector err = end - substitution(f, x==angle);
      
      BOOST_CHECK(std::abs(err[0]) < errlvl);
      BOOST_CHECK(std::abs(err[1]) < errlvl);
      BOOST_CHECK(std::abs(err[2]) < errlvl);
    }

  BOOST_CHECK(toArithmetic(Vector{1,2,3}) == (Vector{1,2,3}));
  BOOST_CHECK(dot(Vector{1,2,3} , Vector{4,5,6}) == 32);
}

BOOST_AUTO_TEST_CASE( symbolic_abs_arbsign )
{
  Variable<> x;

  BOOST_CHECK(compare_expression(abs(x), "|x|"));
  BOOST_CHECK_EQUAL(substitution(abs(x*x - 5*x), x==2), 6);
  BOOST_CHECK_EQUAL(C<6/-2>::num, -3);
  BOOST_CHECK_EQUAL(C<-8/-4>::num, 2);
  BOOST_CHECK(compare_expression(abs(Unity()), Unity()));
  BOOST_CHECK(compare_expression(abs(Null()), Null()));
  BOOST_CHECK(compare_expression(abs(Null()), Null()));
  BOOST_CHECK(compare_expression(simplify(derivative(arbsign(x), x)), arbsign(Unity())));
  BOOST_CHECK(compare_expression(derivative(arbsign(x), Variable<vidx<'y'>>()), Null()));

  BOOST_CHECK(compare_expression(simplify(x * arbsign(x)), arbsign(pow<2>(x))));
  BOOST_CHECK(compare_expression(simplify(arbsign(x) * x), arbsign(pow<2>(x))));
  BOOST_CHECK(compare_expression(simplify(arbsign(x) * arbsign(x)), arbsign(pow<2>(x))));

  BOOST_CHECK(compare_expression(simplify(arbsign(x) / x), arbsign(Unity())));
  BOOST_CHECK(compare_expression(simplify(arbsign(x) / arbsign(x)), arbsign(Unity())));
  BOOST_CHECK(compare_expression(simplify(arbsign(arbsign(x))), arbsign(x)));
  BOOST_CHECK(compare_expression(simplify(pow<5>(arbsign(x))), arbsign(pow<5>(x))));
  BOOST_CHECK(compare_expression(simplify(pow<6>(arbsign(x))), pow<6>(x)));
}

BOOST_AUTO_TEST_CASE( derivative_addition )
{
  using namespace stator::symbolic;

  //Test Polynomial derivatives on addition Operation types
  Variable<> x;

  BOOST_CHECK(compare_expression(simplify(derivative(simplify(C<2>() * x * x + x), x)), C<4>() * x + C<1>()));
}
