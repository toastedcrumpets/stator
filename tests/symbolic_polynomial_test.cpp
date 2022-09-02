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
#define UNIT_TEST_SUITE_NAME Symbolic_Poly
#define UNIT_TEST_GOOGLE
#include <stator/unit_test.hpp>

//C++
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <complex>
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

struct RootData {
  RootData() : multiplicity(0) {}
  std::vector<double> matched_roots;
  size_t multiplicity;
};

template<class Real1, class Real2, size_t N, class PolyVar>
sym::Polynomial<N, Real1, PolyVar> convert_poly(const sym::Polynomial<N, Real2, PolyVar>& p) {
  sym::Polynomial<N, Real1, PolyVar> f;
  for (size_t i(0); i <= N; ++i)
    f[i] = Real1(p[i]);
  return f;
}

template<class T1, class T2, class Func>
void compare_roots(T1 roots, T2 actual_roots, Func f){
  std::sort(roots.begin(), roots.end());
  std::sort(actual_roots.begin(), actual_roots.end());

  //Push all real roots on to a map
  std::map<double, RootData> root_data;
  for (auto root: actual_roots)
    root_data[root].multiplicity += 1;

  bool extra_root = false;
  for (auto root: roots) {
    bool found = false;
    for (auto& test_root: root_data) {
      const double root_error = std::abs((root - test_root.first) / (test_root.first + (test_root.first == 0)));
      if (root_error < 0.00124) {
	found = true;
	test_root.second.matched_roots.push_back(root);
	break;
      }
    }
    if (!found) extra_root = true;
  }

  //Check all roots of odd multiplicity have been detected
  bool missed_root = false;
  for (auto test_root: root_data)
    if (test_root.second.multiplicity % 2)
      if (test_root.second.matched_roots.empty()) {
	missed_root = true;
	break;
      }

  if (extra_root || missed_root) {
    std::ostringstream os;
    os.precision(10);
    os << "\n";
    if (extra_root)
      os << "Unmatched root, ";
    if (missed_root)
      os << "Missing root";

    os << "\n f     = " << f;
    os << "\n actual_roots = " << actual_roots;
    os << "\n detected_roots = " << roots;
    os << "\n Matching data:";
    for (auto test_root: root_data)
      os << "\n   " << test_root.first << ", multiplicity=" << test_root.second.multiplicity << ", matches=" <<  test_root.second.matched_roots.size();
    UNIT_TEST_ERROR(os.str());
  }
}

static constexpr char y_str[] = "y";

UNIT_TEST( poly_variables )
{
  using namespace sym;
  Polynomial<1> x{0, 1};
  Polynomial<1,double, Var<y_str > > y{0, 1};

  UNIT_TEST_CHECK(compare_expression(sub(y, Var<y_str >()=Var<>()), "P(1*x)"));
  UNIT_TEST_CHECK(compare_expression(expand(x * x * x), "P(1*x^3)"));
  UNIT_TEST_CHECK(compare_expression(expand(y * y * y), "P(1*y^3)"));
  UNIT_TEST_CHECK(compare_expression(sub(expand(y * y * y), Var<y_str >()=Var<>()), "P(1*x^3)"));
}

UNIT_TEST( poly_addition )
{
  using namespace sym;
  Polynomial<1> x{0, 2.5};
  Polynomial<0> C{0.3};
  auto poly1 = expand(x+C);
  UNIT_TEST_CHECK_EQUAL(poly1[0], 0.3);
  UNIT_TEST_CHECK_EQUAL(poly1[1], 2.5);

  poly1 = expand(C+x);
  UNIT_TEST_CHECK_EQUAL(poly1[0], 0.3);
  UNIT_TEST_CHECK_EQUAL(poly1[1], 2.5);

  auto poly2 = expand(x + 0.3);
  UNIT_TEST_CHECK_EQUAL(poly2[0], 0.3);
  UNIT_TEST_CHECK_EQUAL(poly2[1], 2.5);

  poly2 = expand(0.3 + x);
  UNIT_TEST_CHECK_EQUAL(poly2[0], 0.3);
  UNIT_TEST_CHECK_EQUAL(poly2[1], 2.5);
}

UNIT_TEST( poly_multiplication )
{
  using namespace sym;
  Polynomial<1> x{0, 1};
  auto poly1 = -2.0;
  auto poly2 = 2.0 - x + x * x;
  auto poly3 = expand(poly2 * poly1);
  UNIT_TEST_CHECK_EQUAL(poly3[0], -4);
  UNIT_TEST_CHECK_EQUAL(poly3[1], +2);
  UNIT_TEST_CHECK_EQUAL(poly3[2], -2);

  UNIT_TEST_CHECK(compare_expression((Null() * Polynomial<2>{1,2,3}), Null()));
  UNIT_TEST_CHECK(compare_expression((Polynomial<2>{1,2,3}) * Null(), Null()));

  static_assert(std::is_same<decltype(Null() * Polynomial<2>{1,2,3}), Null>::value, "Null multiply is not cancelling the polynomial");
  static_assert(std::is_same<decltype(Polynomial<2>{1,2,3} * Null()), Null>::value, "Null multiply is not cancelling the polynomial");
  
  UNIT_TEST_CHECK(compare_expression((Unity() * Polynomial<2>{1,2,3}), Polynomial<2>{1,2,3}));
  UNIT_TEST_CHECK(compare_expression((Polynomial<2>{1,2,3} * Unity()), Polynomial<2>{1,2,3}));
}

UNIT_TEST( poly_division )
{
  using namespace sym;
  Polynomial<1> x{0, 1};
  auto poly1 = 2.0 - x + x * x;
  auto poly2 = expand(poly1 / 0.5);
  UNIT_TEST_CHECK_EQUAL(poly2[0], 4);
  UNIT_TEST_CHECK_EQUAL(poly2[1], -2);
  UNIT_TEST_CHECK_EQUAL(poly2[2], 2);

  static_assert(std::is_same<decltype(simplify(Polynomial<2>{1,2,3} / Unity())), Polynomial<2> >::value, "Unity division is not returning the polynomial");
}

UNIT_TEST( poly_vector )
{
  using namespace sym;
  Polynomial<1, Vector> x{Vector{0,0,0}, Vector{1,2,3}};
  Polynomial<0, Vector> C{Vector{3,2,1}};
  auto poly1 = expand(x+C);
  UNIT_TEST_CHECK_EQUAL(poly1[0], (Vector{3,2,1}));
  UNIT_TEST_CHECK_EQUAL(poly1[1], (Vector{1,2,3}));
}

UNIT_TEST( poly_lower_order )
{
  using namespace sym;
  Polynomial<1> x{0, 1};
  Polynomial<2> poly2 = expand(2.0 - x + x * x);
  Polynomial<3> poly3 = expand(poly2 + 0 * x * x * x);
  //Try to cast down one level as the highest order coefficient is zero
  UNIT_TEST_CHECK(poly3[3] == 0);
  Polynomial<2> poly4(change_order<2>(poly3));

  UNIT_TEST_CHECK_EQUAL(poly4[0], 2);
  UNIT_TEST_CHECK_EQUAL(poly4[1], -1);
  UNIT_TEST_CHECK_EQUAL(poly4[2], 1);
  UNIT_TEST_CHECK_EQUAL(sub(poly3, Var<>()=123), sub(poly4, Var<>()=123));
}

UNIT_TEST( poly_simplify )
{
  using namespace sym;
  Var<> x;
  //Test that simplify creates polynomials from Vars
  auto poly1 = expand(2 * x * x);
  UNIT_TEST_CHECK_EQUAL(poly1[0], 0);
  UNIT_TEST_CHECK_EQUAL(poly1[1], 0);
  UNIT_TEST_CHECK_EQUAL(poly1[2], 2);

  Var<y_str> y;
  //Check that Polynomial simplifications exist for these functions
  expand(y+y);
  expand(y+y+y);
  expand(y*y*2);
  expand(y*y*2+1+2*x+2-12*x*x);

  //Check expansion
  {
    Polynomial<1> x{0,1};
    UNIT_TEST_CHECK(compare_expression(expand(pow(x+2, C<3>())), expand((x+2) * (x+2) * (x+2))));;
  }
}

UNIT_TEST( poly_eval_limits )
{
  using namespace sym;
  Polynomial<1> x{0, 1};

  {//Check even positive polynomials
    auto f = expand(x * x - x + 3);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=0), 3);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=+HUGE_VAL), +HUGE_VAL);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=-HUGE_VAL), +HUGE_VAL);
  }

  {//Check even negative polynomials
    auto f = expand(-x * x + x + 3);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=0), 3);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=+HUGE_VAL), -HUGE_VAL);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=-HUGE_VAL), -HUGE_VAL);
  }

  {//Check odd positive polynomials
    auto f = expand(x * x * x + x + 3);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=+HUGE_VAL), +HUGE_VAL);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=-HUGE_VAL), -HUGE_VAL);
  }

  {//Check odd negative polynomials
    auto f = expand(-x * x * x + x + 3);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=+HUGE_VAL), -HUGE_VAL);
    UNIT_TEST_CHECK_EQUAL(sub(f, Var<>()=-HUGE_VAL), +HUGE_VAL);
  }
}

UNIT_TEST( poly_derivative )
{
  using namespace sym;
  Polynomial<1> x{0, 1};

  auto poly1 = expand(x + x*x + x*x*x + x*x*x*x);
  auto poly2 = derivative(poly1, Var<>());
  UNIT_TEST_CHECK_EQUAL(poly2[0], 1);
  UNIT_TEST_CHECK_EQUAL(poly2[1], 2);
  UNIT_TEST_CHECK_EQUAL(poly2[2], 3);  
  UNIT_TEST_CHECK_EQUAL(poly2[3], 4);  

  UNIT_TEST_CHECK_CLOSE(sub(poly2, Var<>()=3.14159), eval_derivatives<1>(poly1, 3.14159)[1], 1e-10);

  auto poly3 = expand(2.0 - x + 2 * x * x);
  auto poly4 = derivative(poly3, Var<>());
  UNIT_TEST_CHECK_EQUAL(poly4[0], -1);
  UNIT_TEST_CHECK_EQUAL(poly4[1], 4);
  UNIT_TEST_CHECK_EQUAL(sub(poly4, Var<>()=0), -1);
  UNIT_TEST_CHECK_EQUAL(sub(poly4, Var<>()=1), 3);

  C<2>() * derivative(x, Var<>()) * pow(x, C<1>());
  //derivative(pow(x, C<2>), Var<>());
  
  UNIT_TEST_CHECK(compare_expression(simplify(derivative(pow(x, C<2>()), Var<>())), C<2>()* Polynomial<1>{0,1}));
}

UNIT_TEST( poly_zero_derivative)
{
  using namespace sym;
  const Polynomial<1> x{0, 1};
  const auto poly1 = derivative(x, Var<>());
  UNIT_TEST_CHECK_EQUAL(poly1[0], 1);

  const auto poly2 = derivative(poly1, Var<>());
  UNIT_TEST_CHECK(compare_expression(poly2, Null()));
}

UNIT_TEST( poly_deflation)
{
  using namespace sym;
  Polynomial<1> x{0, 1};

  const double roots[] = {-1e3, 4e3, 0, 3.14159265, -3.14159265};
  for (double root1: roots)
    for (double root2: roots)
      for (double root3: roots) {
	auto poly = expand((x - root1) * (x - root2) * (x-root3));
	
	auto deflated = deflate_polynomial(poly, root1);
	auto exact = expand((x - root2) * (x-root3));
	for (size_t i(0); i < 3; ++i)
	  if (exact[i] != 0) {
	    UNIT_TEST_CHECK_CLOSE(deflated[i], exact[i], std::abs(exact[i]) * 1e-10);
	  } else {
	    UNIT_TEST_CHECK_SMALL(deflated[i], 1e-10);
	  }
	
	deflated = deflate_polynomial(poly, root2);
	exact = expand((x - root1) * (x-root3));
	for (size_t i(0); i < 3; ++i)
	  if (exact[i] != 0) {
	    UNIT_TEST_CHECK_CLOSE(deflated[i], exact[i], std::abs(exact[i]) * 1e-10);
	  } else {
	    UNIT_TEST_CHECK_SMALL(deflated[i], 1e-10);
	  }

	deflated = deflate_polynomial(poly, root3);
	exact = expand((x - root1) * (x-root2));
	for (size_t i(0); i < 3; ++i)
	  if (exact[i] != 0) {
	    UNIT_TEST_CHECK_CLOSE(deflated[i], exact[i], std::abs(exact[i]) * 1e-10);
	  } else {
	    UNIT_TEST_CHECK_SMALL(deflated[i], 1e-10);
	  }
      }
}

UNIT_TEST( poly_shift)
{
  using namespace sym;
  Polynomial<1> x{0, 1};

  const double roots[] = {-1e3, 4e3, 0, 3.14159265, -3.14159265};
  for (double root1: roots)
    for (double root2: roots)
      for (double root3: roots) {
	auto f = expand((x - root1) * (x - root2) * (x-root3));

	for (double shift : {-1.0, 2.0, 1e3, 3.14159265, -1e5}) {
	  auto g = shift_function(f, shift);
	  
	  UNIT_TEST_CHECK_CLOSE(sub(g, Var<>() = 0), sub(f, Var<>() = shift), std::abs(sub(f, Var<>() = shift)) * 1e-10);
	  UNIT_TEST_CHECK_CLOSE(sub(g, Var<>() = 1e3), sub(f, Var<>() = 1e3 + shift), std::abs(sub(f, Var<>() = 1e3 + shift)) * 1e-10);
	}
      }
}


UNIT_TEST( poly_gcd )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  { //Standard division with remainder
    auto q = expand(x * x * x + 3 * x - 2);
    auto g = expand(x * x - 2 * x);
    auto r = expand(4 * x - 2);
    auto f = expand(q * g + r);
    auto euclid = gcd(f, g);  
    UNIT_TEST_CHECK(compare_expression(q, std::get<0>(euclid)));
    UNIT_TEST_CHECK(compare_expression(r, std::get<1>(euclid)));
  }
  {//Standard division without remainder
    auto q = expand(x * x * x + 3 * x - 2);
    auto g = expand(x * x - 2 * x);
    auto r = Polynomial<0>{0};
    auto f = simplify(q * g + r);
    auto euclid = gcd(f, g);
    
    UNIT_TEST_CHECK(compare_expression(q, std::get<0>(euclid)));
    UNIT_TEST_CHECK(compare_expression(r, std::get<1>(euclid)));
  }

  {//Division with a zero leading order coefficient
    auto q = expand(x * x * x + 3 * x - 2);
    auto g = expand(0 * x*x*x + x*x - 2 * x);
    auto r = Polynomial<0>{0};
    auto f = expand(q * g + r);
    auto euclid = gcd(f, g);
    
    UNIT_TEST_CHECK(compare_expression(q, std::get<0>(euclid)));
    UNIT_TEST_CHECK(compare_expression(r, std::get<1>(euclid)));
  }

  {//Division by a constant
    auto q = expand(x * x * x + 3 * x - 2);
    auto g = Polynomial<0>{0.5};
    auto r = Polynomial<0>{0};
    auto f = expand(q * g + r);
    auto euclid = gcd(f, g);
    
    UNIT_TEST_CHECK(compare_expression(q, std::get<0>(euclid)));
    UNIT_TEST_CHECK(compare_expression(r, std::get<1>(euclid)));
  }

  {//Division by a high-order Polynomial which is actually a constant
    auto q = expand(x * x * x + 3 * x - 2);
    auto g = Polynomial<3>{0.25};
    auto r = Polynomial<0>{0};
    auto f = expand(q * g + r);
    auto euclid = gcd(f, g);
    
    UNIT_TEST_CHECK(compare_expression(q, std::get<0>(euclid)));
    UNIT_TEST_CHECK(compare_expression(r, std::get<1>(euclid)));
  }
}

// Broken in the massive refactor where I changed store(), to properly
// implement runtime types, commit on May 11th 2021 at ~15:00
//
UNIT_TEST( Poly_Vector_symbolic )
{
  using namespace sym;

  static_assert(sym::detail::IsConstant<Vector>::value, "Vectors are not considered constant!");
  
  UNIT_TEST_CHECK(compare_expression(derivative(Vector{1,2,3}, Var<>()), Null()));
  UNIT_TEST_CHECK(compare_expression(Unity() * Vector{1,2,3}, Vector{1,2,3}));
  UNIT_TEST_CHECK(compare_expression(Vector{1,2,3} * Unity(), Vector{1,2,3}));

  const Polynomial<1> x{0, 1};

  Vector a{0,1,2};
  auto f = a * Var<>();
  auto g = Var<>() = 2;
  Vector test1 = sub(f, g);
  UNIT_TEST_CHECK(test1[0] == 0);
  UNIT_TEST_CHECK(test1[1] == 2);
  UNIT_TEST_CHECK(test1[2] == 4);

  //A tough test is to implement the Rodriugues formula symbolically.
  RNG.seed();
  const size_t testcount = 100;
  const double errlvl = 1e-10;

  for (size_t i(0); i < testcount; ++i)
    {
      double angle = angle_dist(RNG);
      Vector axis = random_unit_vec().normalized();
      Vector start = random_unit_vec();
      Vector end = Eigen::AngleAxis<double>(angle, axis) * start;
      
      Vector r = axis * axis.dot(start);
      auto f = (start - r) * cos(x) + axis.cross(start) * sin(x) + r;
      Vector err = end - sub(f, Var<>() = angle);
      
      UNIT_TEST_CHECK(std::abs(err[0]) < errlvl);
      UNIT_TEST_CHECK(std::abs(err[1]) < errlvl);
      UNIT_TEST_CHECK(std::abs(err[2]) < errlvl);
    }

  UNIT_TEST_CHECK((toArithmetic(Vector{1,2,3}) == Vector{1,2,3}));
}

UNIT_TEST( polynomials_derivative_subtraction )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};
  //Test Polynomial derivatives on subtraction Operation types
  auto poly1 = expand(derivative(2*x*x - x, Var<>()));
  //derivative will automatically combine polynomials
  UNIT_TEST_CHECK_EQUAL(poly1[0], -1);
  UNIT_TEST_CHECK_EQUAL(poly1[1], 4);
}

UNIT_TEST( polynomials_multiply_expansion )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};
  //Test Polynomial simplification on multiplication Operation types
  auto poly1 = expand((x + 1)*(x + 3));
  UNIT_TEST_CHECK_EQUAL(poly1[0], 3);
  UNIT_TEST_CHECK_EQUAL(poly1[1], 4);
  UNIT_TEST_CHECK_EQUAL(poly1[2], 1);
}

UNIT_TEST( function_poly_derivatives_special )
{ 
  using namespace sym;

  //Check special case derivatives of Functions with constant
  //arguments.
  UNIT_TEST_CHECK(compare_expression(derivative(sin(Polynomial<0>{1}), Var<>()), Null()));
  UNIT_TEST_CHECK(compare_expression(derivative(cos(Polynomial<0>{1}), Var<>()), Null()));
}
