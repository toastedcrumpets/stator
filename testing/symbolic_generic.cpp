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

//boost
#define BOOST_TEST_MODULE Symbolic_math_test
#include <boost/test/included/unit_test.hpp>

std::mt19937 RNG;
std::normal_distribution<double> normal_dist(0, 1);
std::uniform_real_distribution<double> angle_dist(0, std::atan(1)*4);

using namespace stator::symbolic;
typedef Eigen::Matrix<double, 3, 1, Eigen::DontAlign> Vector;
const Polynomial<1> x{0, 1};

Vector random_unit_vec() {
  Vector vec{normal_dist(RNG), normal_dist(RNG), normal_dist(RNG)};
  return vec.normalized();
}


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


BOOST_AUTO_TEST_CASE( symbolic_ratio )
{
  BOOST_CHECK_EQUAL(UnitySymbol() * UnitySymbol(), UnitySymbol());
  BOOST_CHECK_EQUAL(UnitySymbol() + UnitySymbol(), ratio<2>());
  BOOST_CHECK_EQUAL(UnitySymbol() + NullSymbol(), UnitySymbol());
  BOOST_CHECK_EQUAL(multiply(UnitySymbol() , NullSymbol(), detail::select_overload{}), NullSymbol());
  Variable<'x'> x;
  BOOST_CHECK_EQUAL(substitution(ratio<1,2>(), x == 2), (ratio<1,2>()));
  BOOST_CHECK_EQUAL((ratio<5>() - ratio<3>() - ratio<2>()) * x, NullSymbol());

  BOOST_CHECK(compare_expression(pi()*pi()/pi(), "π"));
  BOOST_CHECK(compare_expression(e(), "e"));

  BOOST_CHECK(compare_expression(sin(NullSymbol()), NullSymbol()));
  BOOST_CHECK(compare_expression(sin(pi()), NullSymbol()));
  BOOST_CHECK(compare_expression(sin(ratio<12>()*pi()), NullSymbol()));
  BOOST_CHECK(compare_expression(sin(ratio<5,2>()*pi()), UnitySymbol()));

  BOOST_CHECK(compare_expression(cos(NullSymbol()), UnitySymbol()));
  BOOST_CHECK(compare_expression(cos(pi()), UnitySymbol()));
  BOOST_CHECK(compare_expression(cos(ratio<8>()*pi()), UnitySymbol()));
  BOOST_CHECK(compare_expression(cos(ratio<5,2>()*pi()), NullSymbol()));
}


BOOST_AUTO_TEST_CASE( Substitution_of_variables )
{
  Variable<'x'> x;
  Variable<'y'> y;
  
  BOOST_CHECK(compare_expression(substitution(x, x==y), "y"));
}

BOOST_AUTO_TEST_CASE( simplify_tests )
{
  //Test that simplify does nothing when it has nothing to do
  auto poly1 = try_simplify(2 * x * x);
  BOOST_CHECK_EQUAL(poly1[0], 0);
  BOOST_CHECK_EQUAL(poly1[1], 0);
  BOOST_CHECK_EQUAL(poly1[2], 2);

  Variable<'y'> y;

  //Check that expansions exist for these functions
  simplify(y+y);
  simplify(y+y+y);
  //simplify(y*y*y);
  simplify(y*y*2);
}
  
BOOST_AUTO_TEST_CASE( simplify_polynomials )
{
  //Test addition and simplification of Polynomials
  auto poly1 = 2 * x * x + x;
  //This should become a Polynomial class, with its coefficients
  //accessible by the array operator.
  BOOST_CHECK_EQUAL(poly1[0], 0);
  BOOST_CHECK_EQUAL(poly1[1], 1);
  BOOST_CHECK_EQUAL(poly1[2], 2);
}
  
BOOST_AUTO_TEST_CASE( polynomials_derivative_addition )
{
  //Test Polynomial derivatives on addition Operation types
  auto poly1 = derivative(add(2 * x * x,  x, detail::select_overload{}), Variable<'x'>());
  //derivative will automatically combine polynomials
  BOOST_CHECK_EQUAL(poly1[0], 1);
  BOOST_CHECK_EQUAL(poly1[1], 4);
}

BOOST_AUTO_TEST_CASE( polynomials_derivative_subtraction )
{
  //Test Polynomial derivatives on subtraction Operation types
  auto poly1 = derivative(subtract(2 * x * x,  x, detail::select_overload{}), Variable<'x'>());
  //derivative will automatically combine polynomials
  BOOST_CHECK_EQUAL(poly1[0], -1);
  BOOST_CHECK_EQUAL(poly1[1], 4);
}

BOOST_AUTO_TEST_CASE( polynomials_multiply_expansion )
{
  //Test Polynomial simplification on multiplication Operation types
  auto poly1 = (x + 1) *  (x + 3);

  //derivative will automatically combine polynomials
  BOOST_CHECK_EQUAL(poly1[0], 3);
  BOOST_CHECK_EQUAL(poly1[1], 4);
  BOOST_CHECK_EQUAL(poly1[2], 1);
}

BOOST_AUTO_TEST_CASE( function_basic )
{
  //Check basic Function operation
  BOOST_CHECK_CLOSE(eval(stator::symbolic::sin(x), 0.5), std::sin(0.5), 1e-10);
  BOOST_CHECK_CLOSE(eval(stator::symbolic::cos(x), 0.5), std::cos(0.5), 1e-10);

  //Test BinaryOP Addition and subtraction
  BOOST_CHECK_CLOSE(eval(x * sin(x) + x, 0.5), 0.5 * std::sin(0.5) + 0.5, 1e-10);
  BOOST_CHECK_CLOSE(eval(x * sin(x) - x, 0.5), 0.5 * std::sin(0.5) - 0.5, 1e-10);
}

BOOST_AUTO_TEST_CASE( function_poly_multiplication )
{
  //Check function and Polynomial multiplication
  auto poly1 = sin(x + x) * x;
  BOOST_CHECK_CLOSE(eval(poly1, 0.5), std::sin(2 * 0.5) * 0.5, 1e-10);
  auto poly2 = x * sin(x + x);
  BOOST_CHECK_CLOSE(eval(poly2, 0.5), std::sin(2 * 0.5) * 0.5, 1e-10);
}

BOOST_AUTO_TEST_CASE( function_poly_derivatives )
{
  //Check function and Polynomial derivatives
  auto f1 = derivative(x * sin(x), Variable<'x'>());
  BOOST_CHECK_CLOSE(eval(f1, 0.5), std::sin(0.5) + 0.5 * std::cos(0.5), 1e-10);
  auto f2 = derivative(x * cos(x), Variable<'x'>());
  BOOST_CHECK_CLOSE(eval(f2, 0.5), -0.5 * std::sin(0.5) + std::cos(0.5), 1e-10);
}

BOOST_AUTO_TEST_CASE( function_poly_derivatives_special )
{ //Check special case derivatives of Functions with constant
  //arguments.
  BOOST_CHECK(compare_expression(derivative(sin(Polynomial<0>{1}), Variable<'x'>()), NullSymbol()));
  BOOST_CHECK(compare_expression(derivative(cos(Polynomial<0>{1}), Variable<'x'>()), NullSymbol()));
}

BOOST_AUTO_TEST_CASE( power_basic )
{
  BOOST_CHECK(pow<3>(3) == 27);
  BOOST_CHECK(pow<2>(Vector{0,1,2}) == 5);

  BOOST_CHECK_CLOSE(substitution(pow<3>(x), Variable<'x'>()==4.0), 4.0*4.0*4.0, 1e-10);
  BOOST_CHECK_CLOSE(eval(pow<3>(x), 0.75), std::pow(0.75, 3), 1e-10);

  //Test PowerOp algebraic operations
  BOOST_CHECK_CLOSE(eval(pow<3>(x) - x, 0.75), std::pow(0.75, 3) - 0.75, 1e-10);
  BOOST_CHECK_CLOSE(eval(pow<3>(x) + x, 0.75), std::pow(0.75, 3) + 0.75, 1e-10);
  BOOST_CHECK_CLOSE(eval(pow<3>(x) * x, 0.75), std::pow(0.75, 3) * 0.75, 1e-10);

  //Check special case derivatives
  BOOST_CHECK(compare_expression(derivative(pow<1>(x), Variable<'x'>()), 1));

  BOOST_CHECK(compare_expression(derivative(pow<2>(x), Variable<'x'>()), Polynomial<1>{0,1} * ratio<2>()));

  //Check expansion
  BOOST_CHECK(compare_expression(simplify_powerop_impl(pow<3>(x+2), detail::select_overload{}), (x+2) * (x+2) * (x+2)));;
}

BOOST_AUTO_TEST_CASE( Null_tests )
{
  Variable<'y'> y;
  BOOST_CHECK(substitution(NullSymbol(), y==100) == NullSymbol());
  
  //Check that Null symbols have zero derivative and value
  BOOST_CHECK(compare_expression(NullSymbol(), NullSymbol()));
  BOOST_CHECK(compare_expression(derivative(NullSymbol(), Variable<'x'>()), NullSymbol()));
  BOOST_CHECK_EQUAL(substitution(NullSymbol(), y==100), NullSymbol());
  
  //Check some substitutions
  BOOST_CHECK(compare_expression(substitution(y*y*y, y == NullSymbol()), NullSymbol()));
  
  //Check derivatives of constants becomes Null
  BOOST_CHECK(compare_expression(derivative(2, Variable<'x'>()), NullSymbol()));
  BOOST_CHECK(compare_expression(derivative(3.141, Variable<'x'>()), NullSymbol()));

  BOOST_CHECK(compare_expression(Vector{1,2,3} * NullSymbol(), NullSymbol()));
  BOOST_CHECK(compare_expression(Vector{1,2,3} + NullSymbol(), Vector{1,2,3}));
}

BOOST_AUTO_TEST_CASE( Unity_tests )
{
  //Check that Null symbols have zero derivative and value
  BOOST_CHECK(compare_expression(UnitySymbol(), UnitySymbol()));
  BOOST_CHECK(compare_expression(UnitySymbol() + UnitySymbol(), 2));
  BOOST_CHECK(compare_expression(derivative(UnitySymbol(),Variable<'x'>()), NullSymbol()));
  BOOST_CHECK_EQUAL(eval(UnitySymbol(), 100), UnitySymbol());

  BOOST_CHECK(compare_expression(UnitySymbol() + UnitySymbol(), 2));
  BOOST_CHECK(compare_expression(add(UnitySymbol(), 1.1, detail::select_overload()), 2.1));

  BOOST_CHECK(compare_expression(UnitySymbol() + NullSymbol(), UnitySymbol()));
  BOOST_CHECK(compare_expression(NullSymbol() + UnitySymbol(), UnitySymbol()));

  //Check derivatives of Unity
  BOOST_CHECK(compare_expression(derivative(UnitySymbol(), Variable<'x'>()), NullSymbol()));

  //Check simplification of multiplication with Unity
  BOOST_CHECK(compare_expression(UnitySymbol() * UnitySymbol(), UnitySymbol()));
  BOOST_CHECK(compare_expression(UnitySymbol() * 2, 2));
  BOOST_CHECK(compare_expression(UnitySymbol() * x, x));

  BOOST_CHECK(compare_expression(2 * UnitySymbol(), 2));
  BOOST_CHECK(compare_expression(x * UnitySymbol() * x, x * x));
}

BOOST_AUTO_TEST_CASE( Var_tests )
{
  Variable<'x'> x;
  Variable<'y'> y;

  BOOST_CHECK(compare_expression(x, "x")); 
  BOOST_CHECK(compare_expression(y, "y"));
  BOOST_CHECK(compare_expression(derivative(x, Variable<'x'>()), UnitySymbol()));
  BOOST_CHECK(compare_expression(derivative(y, Variable<'x'>()), NullSymbol()));
  BOOST_CHECK(compare_expression(derivative(y, Variable<'y'>()), UnitySymbol()));
  BOOST_CHECK(compare_expression(substitution(x, x == 3.14159265), 3.14159265));

  //Check that substitutions in the wrong variable do nothing
  BOOST_CHECK(compare_expression(substitution(y, x == 3.14159265), "y"));

  //Check default substitution is for x
  BOOST_CHECK(compare_expression(eval(y, 3.14159265), "y"));

  //Check that Var derivatives are correct
  BOOST_CHECK(compare_expression(derivative(sin(x), Variable<'x'>()), cos(x)));

  //Check derivatives of Unity
  BOOST_CHECK(compare_expression(derivative(UnitySymbol(), Variable<'x'>()), NullSymbol()));
  BOOST_CHECK(compare_expression(derivative(x, Variable<'x'>()), UnitySymbol()));
  BOOST_CHECK(compare_expression(derivative(x * sin(x), Variable<'x'>()), sin(x) + x * cos(x)));
}

BOOST_AUTO_TEST_CASE( reorder_operations )
{
  //Check the specialised multiply operators are consistently
  //simplifying statements.

  //Again, check that negative matches are correctly determined by
  //compare_expression
  BOOST_CHECK(!compare_expression(x, sin(x), false));

  //Here we're looking for the two Polynomial terms to be reordered 
  BOOST_CHECK(compare_expression((sin(2*x) * x) * x, x * x * sin(2*x)));
  BOOST_CHECK(compare_expression((x * sin(2*x)) * x, x * x * sin(2*x)));
  BOOST_CHECK(compare_expression(x * (sin(2*x) * x), x * x * sin(2*x)));
  BOOST_CHECK(compare_expression(x * (x * sin(2*x)), x * x * sin(2*x)));

  //Here we check that constants (such as 2) will become NullSymbol
  //types when the derivative is taken, causing their terms to be
  //eliminated.
  BOOST_CHECK(compare_expression(derivative(2 * cos(x), Variable<'x'>()), -2 * sin(x)));
  BOOST_CHECK(compare_expression(derivative(2 * sin(x), Variable<'x'>()), 2 * cos(x)));
}

BOOST_AUTO_TEST_CASE( polynomial_substitution_function )
{
  //Test substitution and expansion of a Polynomial 
  BOOST_CHECK(compare_expression(eval(x * x - 3 * x + 2, Variable<'x'>() == x+1), x * x - x));
}

BOOST_AUTO_TEST_CASE( taylor_series_test )
{
  Variable<'y'> y;

  //Simplifying in the wrong variable
  BOOST_CHECK(compare_expression(taylor_series<3>(y*y*y, NullSymbol(), Variable<'x'>()), y*y*y));
  
  ////Simplifying PowerOp expressions into Polynomial
  BOOST_CHECK(compare_expression(taylor_series<3>(y*y*y, NullSymbol(), y), Polynomial<3,int,'y'>{0,0,0,1}));
  
  //Test truncation of PowerOp expressions when the order is too high
  BOOST_CHECK(compare_expression(taylor_series<2>(y*y*y, NullSymbol(), y), NullSymbol()));
  
  //Partially truncate a Polynomial through expansion
  BOOST_CHECK(compare_expression(taylor_series<2>(Polynomial<3,int,'y'>{1,2,3,4}, NullSymbol(), y), Polynomial<2,int,'y'>{1,2,3}));
  
  //Keep the order the same
  BOOST_CHECK(compare_expression(taylor_series<3>(Polynomial<3,int,'y'>{1,2,3,4}, NullSymbol(), y), Polynomial<3,int,'y'>{1,2,3,4}));
  
  //Taylor simplify at a higher order
  BOOST_CHECK(compare_expression(taylor_series<4>(Polynomial<3,int,'y'>{1,2,3,4}, NullSymbol(), y), Polynomial<3,int,'y'>{1,2,3,4}));
  
  //Test simple Taylor expansion of sine 
  BOOST_CHECK(compare_expression(taylor_series<6>(sin(y), NullSymbol(), y), simplify((1.0/120) * y*y*y*y*y - (1.0/6) * y*y*y + y)));
  BOOST_CHECK(compare_expression(taylor_series<8>(sin(y*y), NullSymbol(), y), simplify(- (1.0/6) * y*y*y*y*y*y + y*y)));
  
  //Test Taylor expansion of a complex expression at zero
  BOOST_CHECK(compare_expression(taylor_series<3>(sin(cos(x)+2*x*x - x + 3), NullSymbol(), Variable<'x'>()), (3.0 * std::sin(4.0)/2.0 + (std::cos(4.0)/6.0)) * x*x*x + (3*std::cos(4.0)/2.0 - std::sin(4.0)/2.0) * x*x - std::cos(4.0) * x + std::sin(4.0)));
  //Test Taylor expansion again at a non-zero location
  BOOST_CHECK(compare_expression(taylor_series<3>(sin(cos(x)+2*x*x - x + 3), 3.0, Variable<'x'>()), 82.77908670866608 * x*x*x - 688.8330378984795 * x*x + 1895.079543801394 * x - 1721.740734454172));
}

BOOST_AUTO_TEST_CASE( Vector_symbolic )
{
  static_assert(stator::symbolic::detail::IsConstant<Vector>::value, "Vectors are not considered constant!");
  
  BOOST_CHECK(compare_expression(derivative(Vector{1,2,3}, Variable<'x'>()), NullSymbol()));
  BOOST_CHECK(compare_expression(UnitySymbol() * Vector{1,2,3}, Vector{1,2,3}));
  BOOST_CHECK(compare_expression(Vector{1,2,3} * UnitySymbol(), Vector{1,2,3}));

  const Polynomial<1> x{0, 1};

  const size_t testcount = 100;
  const double errlvl = 1e-10;

  Vector test1 = substitution(Vector{0,1,2} * Variable<'x'>(), Variable<'x'>() == 2);
  BOOST_CHECK(test1[0] == 0);
  BOOST_CHECK(test1[1] == 2);
  BOOST_CHECK(test1[2] == 4);

  //A tough test is to implement the Rodriugues formula symbolically.
  RNG.seed();
  for (size_t i(0); i < testcount; ++i)
    {
      double angle = angle_dist(RNG);
      Vector axis = random_unit_vec().normalized();
      Vector start = random_unit_vec();
      Vector end = Eigen::AngleAxis<double>(angle, axis) * start;
      
      Vector r = axis * axis.dot(start);
      auto f = (start - r) * cos(x) + axis.cross(start) * sin(x) + r;
      Vector err = end - eval(f, angle);
      
      BOOST_CHECK(std::abs(err[0]) < errlvl);
      BOOST_CHECK(std::abs(err[1]) < errlvl);
      BOOST_CHECK(std::abs(err[2]) < errlvl);
    }

  BOOST_CHECK(toArithmetic(Vector{1,2,3}) == (Vector{1,2,3}));
  BOOST_CHECK(dot(Vector{1,2,3} , Vector{4,5,6}, stator::symbolic::detail::select_overload{}) == 32);

  //std::cout << simplify(Vector{1,2,3} + Variable<'x'>() * Vector{1,2,3}) << std::endl;
  //std::cout << Polynomial<1, Vector>{{1,2,3}, Vector{2,2,3}} << std::endl;
}

BOOST_AUTO_TEST_CASE( symbolic_abs_arbsign )
{
  Variable<'x'> x;

  BOOST_CHECK(compare_expression(abs(x), "|x|"));
  BOOST_CHECK_EQUAL(substitution(abs(x*x - 5*x), x==2), 6);
  BOOST_CHECK_EQUAL(ratio<6/-2>::num, -3);
  BOOST_CHECK_EQUAL(ratio<-8/-4>::num, 2);
  BOOST_CHECK(compare_expression(abs(UnitySymbol()), UnitySymbol()));
  BOOST_CHECK(compare_expression(abs(NullSymbol()), NullSymbol()));
  BOOST_CHECK(compare_expression(abs(NullSymbol()), NullSymbol()));
  BOOST_CHECK(compare_expression(derivative(arbsign(x), x), arbsign(UnitySymbol())));
  BOOST_CHECK(compare_expression(derivative(arbsign(x), Variable<'y'>()), NullSymbol()));

  BOOST_CHECK(compare_expression(simplify(x * arbsign(x)), arbsign(pow<2>(x))));
  BOOST_CHECK(compare_expression(simplify(arbsign(x) * x), arbsign(pow<2>(x))));
  BOOST_CHECK(compare_expression(simplify(arbsign(x) * arbsign(x)), arbsign(pow<2>(x))));

  BOOST_CHECK(compare_expression(simplify(x / arbsign(x)), arbsign(UnitySymbol())));
  BOOST_CHECK(compare_expression(simplify(arbsign(x) / x), arbsign(UnitySymbol())));
  BOOST_CHECK(compare_expression(simplify(arbsign(x) / arbsign(x)), arbsign(UnitySymbol())));
  BOOST_CHECK(compare_expression(simplify(arbsign(arbsign(x))), arbsign(x)));
  BOOST_CHECK(compare_expression(simplify(pow<5>(arbsign(x))), arbsign(pow<5>(x))));
  BOOST_CHECK(compare_expression(simplify(pow<6>(arbsign(x))), pow<6>(x)));
}
