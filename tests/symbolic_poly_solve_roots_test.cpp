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
#define UNIT_TEST_SUITE_NAME Symbolic_Poly_Solve_Roots
#define UNIT_TEST_GOOGLE
#include <stator/unit_test.hpp>

//C++
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <complex>
#include <set>
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
void compare_roots(T1 roots, T2 actual_roots, Func f, double tolerance = 0.00124){
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
      double tol = tolerance;
      //If the root is a multiple root, then increase tolerance as precision is lost here
      if (test_root.second.multiplicity > 1)
	tol *= 2;
      const double root_error = std::abs((root - test_root.first) / (test_root.first + (test_root.first == 0)));
      if (root_error < tol) {
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

UNIT_TEST(poly_quadratic_roots_simple)
{
  using namespace sym;
  Polynomial<1> x{0, 1};
  
  {//Quadratic with no roots
    auto poly = expand(x * x - 3 * x + 4);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 0u);
  }
  
  {//Quadratic with one root
    auto poly = expand(-4 * x * x + 12 * x - 9);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 1u);
    if (roots.size() == 1) {
      UNIT_TEST_CHECK_CLOSE(roots[0], 1.5, 1e-10);
    }
  }
  
  {//quadratic but linear function with one root
    auto poly =  expand(0 * x * x + 12 * x - 9);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 1u);
    if (roots.size() == 1) {
      UNIT_TEST_CHECK_CLOSE(roots[0], 0.75, 1e-10);
    }
  }

  {//constant function, with no roots
    auto poly =  expand(0 * x * x + 0 * x - 9);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 0u);
  }

  {//Test a zero constant quadratic
    auto poly =  expand(3 * x * x + 2 * x - 0);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 2u);
    UNIT_TEST_CHECK_CLOSE(roots[0], -2.0/3.0, 1e-10);
    UNIT_TEST_CHECK_CLOSE(roots[1], 0, 1e-10);
  }
}

UNIT_TEST( poly_quadratic_special_cases)
{
  using namespace sym;
  Polynomial<1> x{0, 1};
  
  {//Quadratic with catastrophic cancellation of error
    auto poly = expand(x * x + 712345.12 * x + 1.25);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 2u);
    UNIT_TEST_CHECK_CLOSE(roots[0], -712345.1199985961, 1e-10 * 712345.1199985961);
    UNIT_TEST_CHECK_CLOSE(roots[1], -1.754767408250742e-6, 1e-10 * 1.754767408250742e-6);
  }

  const double maxsqrt = std::sqrt(std::numeric_limits<double>::max());
  const double largeterm = maxsqrt * 100;
  {//Large linear coefficient
    auto poly = expand(x * x + largeterm * x + 1.25);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 2u);
    //Mathematica value
    UNIT_TEST_CHECK_CLOSE(roots[0], -1.3407807929942599e156, 1e-10 * 1.3407807929942599e156);
    UNIT_TEST_CHECK_CLOSE(roots[1], -9.322925914000258e-157, 1e-10 * 9.322925914000258e-157);
  }

  {//Large (+ve) constant coefficient
    auto poly = expand(x * x + x + largeterm);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 0u);
  }
  {//Large (-ve) constant coefficient
    auto poly = expand(x * x + x - largeterm);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 2u);

    //Mathematica value
    UNIT_TEST_CHECK_CLOSE(roots[0], -1.157920892373162e78, 1e-10 * 1.157920892373162e78);
    UNIT_TEST_CHECK_CLOSE(roots[1], 1.157920892373162e78, 1e-10 * 1.157920892373162e78);
  }
}

double cubic_rootvals[] = {-1e6, -1e3, -100, -1, 0, 1, +100, 1e3, 1e6};

UNIT_TEST( poly_linear_roots_full )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  for (double root1 : cubic_rootvals)
    for (double factor : cubic_rootvals)
	{
          if (factor == 0) continue;
	  auto f = expand(factor * (x - root1));
	  auto roots = solve_real_roots(f);
	  decltype(roots) actual_roots = {root1};
	  compare_roots(roots, actual_roots, f);
	}
}

UNIT_TEST( poly_quadratic_roots_full )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  for (double root1 : cubic_rootvals)
    for (double root2 : cubic_rootvals)
      //for (double factor : cubic_rootvals)
	{
          //if (factor == 0) continue;
	  auto f = expand((x - root1) * (x - root2));
	  auto roots = solve_real_roots(f);
	  decltype(roots) actual_roots = {root1,root2};
	  compare_roots(roots, actual_roots, f);
	}
}

UNIT_TEST( poly_cubic_triple_roots )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  for (double root1 : cubic_rootvals)
    for (double root2 : cubic_rootvals)
      for (double root3 : cubic_rootvals)
	{
	  auto f = expand((x - root1) * (x - root2) * (x - root3));
	  auto roots = solve_real_roots(f);
	  decltype(roots) actual_roots = {root1,root2, root3};
	  compare_roots(roots, actual_roots, f);
	}
}

UNIT_TEST( poly_cubic_single_roots )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  for (double root1 : cubic_rootvals)
    for (double root2real : cubic_rootvals)
      for (double root2im : cubic_rootvals)
	{
	  //Skip real second root cases, and the symmetric cases
	  if (root2im <= 0) continue;
	  
	  std::complex<double>
	    root1val(root1, 0),
	    root2val(root2real, root2im),
	    root3val(root2real, -root2im);

	  auto poly_c = expand((x - root1val) * (x - root2val) * (x - root3val));
	  Polynomial<3, double> f = {poly_c[0].real(), poly_c[1].real(), poly_c[2].real(), poly_c[3].real()};

	  auto roots = solve_real_roots(f);
	  decltype(roots) actual_roots = {root1};
	  compare_roots(roots, actual_roots, f);
	}
}

UNIT_TEST( poly_cubic_special_cases )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  {//Zero constant term with three roots
    auto poly = expand((x * x + 712345.12 * x + 1.25) * x);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 3u);
    UNIT_TEST_CHECK_CLOSE(roots[0], -712345.1199985961, 1e-10 * 712345.1199985961);
    UNIT_TEST_CHECK_CLOSE(roots[1], -1.754767408250742e-6, 1e-10 * 1.754767408250742e-6);
    UNIT_TEST_CHECK_CLOSE(roots[2], 0, 1e-10);
  }

  {//Zero constant term with one root
    auto poly = expand((x * x - 3 * x + 4) * x);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 1u);
    UNIT_TEST_CHECK_CLOSE(roots[0], 0, 1e-10);
  }

  {//Special case where f(x) = a * x^3 + d
    auto poly = expand(x * x * x + 1e3);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 1u);
    UNIT_TEST_CHECK_CLOSE(roots[0], -10, 1e-10);

    poly = expand(x * x * x - 1e3);
    roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 1u);
    UNIT_TEST_CHECK_CLOSE(roots[0], 10, 1e-10);
  }

  const double maxsqrt = std::sqrt(std::numeric_limits<double>::max());
  const double largeterm = maxsqrt * 100;

  {//Large x^2 term
    auto poly = expand(x * x * x - largeterm * x * x + 1.25);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 3u);
    UNIT_TEST_CHECK_CLOSE(roots[0], -9.655529977168658e-79, 1e-10 * 9.655529977168658e-79);
    UNIT_TEST_CHECK_CLOSE(roots[1], +9.655529977168658e-79, 1e-10 * 9.655529977168658e-79);
    UNIT_TEST_CHECK_CLOSE(roots[2], 1.3407807929942599e156, 1e-10 * 1.3407807929942599e156);
  }

  {//Large x term
    auto poly = expand(x * x * x - x * x - largeterm * x + 1.25);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 3u);
    UNIT_TEST_CHECK_CLOSE(roots[0], -1.1579208923731622e78, 1e-10 * 1.1579208923731622e78);
    UNIT_TEST_CHECK_CLOSE(roots[1], 9.322925914000258e-157, 1e-10 * 9.322925914000258e-157);
    UNIT_TEST_CHECK_CLOSE(roots[2], 1.1579208923731622e78,  1e-10 * 1.1579208923731622e78);
  }

  const double smallerterm = maxsqrt * 1e-1;
  {
    //Large v term
    auto poly = expand(x * x * x  - smallerterm * x * x - smallerterm * x + 2);
    auto roots = solve_real_roots(poly);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 3u);
    UNIT_TEST_CHECK_CLOSE(roots[0], -1.0, 1e-10);
    UNIT_TEST_CHECK_CLOSE(roots[1], 1.491668146240041472864517142264024641421371730393e-153, 1e-10 * 1.491668146240041472864517142264024641421371730393e-153);
    UNIT_TEST_CHECK_CLOSE(roots[2], 1.340780792994259598314974448015366224371799690462e153, 1e-10 * 1.340780792994259598314974448015366224371799690462e153);
  }
}

UNIT_TEST( poly_Sturm_chains )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  { //Example from wikipedia (x^4+x^3-x-1)
    auto f = expand(x*x*x*x + x*x*x - x - 1);
    auto chain = sturm_chain(f);

    UNIT_TEST_CHECK(compare_expression(chain.get(0), f));
    UNIT_TEST_CHECK(compare_expression(chain.get(1), expand(4*x*x*x + 3*x*x - 1)));
    UNIT_TEST_CHECK(compare_expression(chain.get(2), expand((3.0/16) * x*x + (3.0/4)*x + (15.0/16))));
    UNIT_TEST_CHECK(compare_expression(chain.get(3), expand(-32*x -64)));
    UNIT_TEST_CHECK(compare_expression(chain.get(4), Polynomial<0>{-3.0/16}));
    UNIT_TEST_CHECK(compare_expression(chain.get(5), Polynomial<0>{0}));
    UNIT_TEST_CHECK(compare_expression(chain.get(6), Polynomial<0>{0}));
    
    //This polynomial has roots at -1 and +1
    UNIT_TEST_CHECK_EQUAL(chain.sign_changes(-HUGE_VAL), 3u);
    UNIT_TEST_CHECK_EQUAL(chain.sign_changes(0), 2u);
    UNIT_TEST_CHECK_EQUAL(chain.sign_changes(HUGE_VAL), 1u);
    
    UNIT_TEST_CHECK_EQUAL(chain.roots(0.5, 3.0), 1u);
    UNIT_TEST_CHECK_EQUAL(chain.roots(-2.141, -0.314159265), 1u);
    UNIT_TEST_CHECK_EQUAL(chain.roots(-HUGE_VAL, HUGE_VAL), 2u);
 }
}

UNIT_TEST( descartes_sturm_and_budan_01_alesina_rootcount_test )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};
  
  //The values 0.5, and 2.0 are strong tests of the algorithms, as
  //these are the division points for VCA and VAS algorithms.
  const double roots[] = {-1e5, -0.14159265, -0.0001,0.1, 0.3333, 0.5, 0.8, 1.001, 2.0, 3.14159265, 1e7};
  
  for (const double root1: roots)
    for (const double root2: roots)
      if (root1 != root2)
	for (const double root3: roots)
	  if (!std::set<double>{root1, root2}.count(root3))
	    for (const double root4: roots)
	      if (!std::set<double>{root1, root2, root3}.count(root4))
		for (const double root5: roots)
		  if (!std::set<double>{root1, root2, root3, root4}.count(root5))
		    for (int sign : {-1, +1}) {
		      //Test where all 5 roots of a 5th order
		      //Polynomial are real
		      auto f1 = expand(sign * (x - root1) * (x - root2) * (x - root3) * (x - root4) * (x - root5));
		      
		      //Test with the same roots, but an additional 2
		      //imaginary roots
		      auto f2 = expand(f1 * (x * x - 3 * x + 4));

		      auto roots_in_range = [&](double a, double b) {
			return size_t
			(((root1 > a) && (root1 < b))
			+((root2 > a) && (root2 < b))
			+((root3 > a) && (root3 < b))
			+((root4 > a) && (root4 < b))
			 +((root5 > a) && (root5 < b)))
			; };

		      size_t roots_in_01 = roots_in_range(0, 1);

		      auto chain1 = sturm_chain(f1);
		      auto chain2 = sturm_chain(f2);

		      //Test interval [0,1]
		      switch (roots_in_01) {
		      case 0: 
		      case 1: 
			UNIT_TEST_CHECK_EQUAL(budan_01_test(f1), roots_in_01);
			UNIT_TEST_CHECK_EQUAL(budan_01_test(f2), roots_in_01);
			UNIT_TEST_CHECK_EQUAL(alesina_galuzzi_test(f1, 0.0, 1.0), roots_in_01);
			UNIT_TEST_CHECK_EQUAL(alesina_galuzzi_test(f2, 0.0, 1.0), roots_in_01);
			break;
		      default: 
			UNIT_TEST_CHECK(budan_01_test(f1) >= roots_in_01); 
			UNIT_TEST_CHECK(budan_01_test(f2) >= roots_in_01); 
			UNIT_TEST_CHECK(alesina_galuzzi_test(f1, 0.0, 1.0) >= roots_in_01); 
			UNIT_TEST_CHECK(alesina_galuzzi_test(f2, 0.0, 1.0) >= roots_in_01); 
			break;
		      }
		      UNIT_TEST_CHECK_EQUAL(chain1.roots(0.0, 1.0), roots_in_01); 
		      UNIT_TEST_CHECK_EQUAL(chain2.roots(0.0, 1.0), roots_in_01); 
		      
		      //Test interval [0, \infty]
		      size_t positive_roots = roots_in_range(0, HUGE_VAL);
		      switch (positive_roots) {
		      case 0: 
		      case 1:
			UNIT_TEST_CHECK_EQUAL(descartes_rule_of_signs(f1), positive_roots); 
			UNIT_TEST_CHECK_EQUAL(descartes_rule_of_signs(f2), positive_roots);
			break;
		      default: 
			UNIT_TEST_CHECK(descartes_rule_of_signs(f1) >= positive_roots); 
			break;
		      }
		      UNIT_TEST_CHECK_EQUAL(chain1.roots(0.0, HUGE_VAL), positive_roots); 
		      UNIT_TEST_CHECK_EQUAL(chain2.roots(0.0, HUGE_VAL), positive_roots); 

		      //Try some others
		      UNIT_TEST_CHECK_EQUAL(chain1.roots(-HUGE_VAL, HUGE_VAL), 5u);
		      UNIT_TEST_CHECK_EQUAL(chain2.roots(-HUGE_VAL, HUGE_VAL), 5u); 
		      UNIT_TEST_CHECK(alesina_galuzzi_test(f1,-1.0, 30.0) >= roots_in_range(-1, 30));
		      UNIT_TEST_CHECK(alesina_galuzzi_test(f1,-0.01, 5.0) >= roots_in_range(-0.01, 5));
		    }
}

UNIT_TEST( LMQ_upper_bound_test )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  const double roots[] = {-1e5, -0.14159265, 3.14159265, -0.0001,0.1, 0.3333, 0.6, 1.001, 2.0, 3.14159265, 1e7};

  //Test simple expressions
  for (const double root1: roots)
    for (const double root2: roots)
      for (const double root3: roots)
	for (const double root4: roots)
	  for (int sign : {-1, +1})
	    {
	      auto f = expand(sign * (x - root1) * (x - root2) * (x - root3) * (x - root4));

	      double max_root = root1;
	      max_root = std::max(max_root, root2);
	      max_root = std::max(max_root, root3);
	      max_root = std::max(max_root, root4);
	    
	      double bound = LMQ_upper_bound(f);
	      if (max_root < 0)
		UNIT_TEST_CHECK_EQUAL(bound, 0);
	      else
		UNIT_TEST_CHECK(bound >= max_root);
	    }

  //Test expressions with zero coefficients
  for (const double root1: roots)
    for (const double root2: roots)
      for (int sign : {-1, +1})
	{
	  auto f = expand(sign * (x - root1) * (x - root2) + 0 * x*x*x*x*x);
	  double max_root = std::max(root1, root2);
	  
	  double bound = LMQ_upper_bound(f);
	  if (max_root < 0)
	    UNIT_TEST_CHECK_EQUAL(bound, 0);
	  else
	    UNIT_TEST_CHECK(bound >= max_root);
	}

  //Test constant coefficients 
  UNIT_TEST_CHECK_EQUAL(LMQ_upper_bound(expand(1 + 0 * x*x*x*x*x)), 0);
}

UNIT_TEST( LMQ_lower_bound_test )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  const double roots[] = {-1e5, -0.14159265, 3.14159265, -0.0001,0.1, 0.3333, 0.6, 1.001, 2.0, 3.14159265, 1e7};

  for (const double root1: roots)
    for (const double root2: roots)
      for (const double root3: roots)
	for (const double root4: roots)
	  for (int sign : {-1, +1})
	    {
	      auto f = expand(sign * (x - root1) * (x - root2) * (x - root3) * (x - root4));

	      double min_pos_root = HUGE_VAL;
	      if (root1 >= 0)
		min_pos_root = std::min(min_pos_root, root1);
	      if (root2 >= 0)
		min_pos_root = std::min(min_pos_root, root2);
	      if (root3 >= 0)
		min_pos_root = std::min(min_pos_root, root3);
	      if (root4 >= 0)
		min_pos_root = std::min(min_pos_root, root4);
	    
	      double bound = LMQ_lower_bound(f);
	      if (min_pos_root == HUGE_VAL)
		UNIT_TEST_CHECK_EQUAL(bound, HUGE_VAL);
	      else
		UNIT_TEST_CHECK(bound <= min_pos_root);
	    }

  //Test expressions with zero coefficients
  for (const double root1: roots)
    for (const double root2: roots)
      for (int sign : {-1, +1})
	{
	  auto f = expand(sign * (x - root1) * (x - root2) + 0 * x*x*x*x*x);

	  double min_pos_root = HUGE_VAL;
	  if (root1 >= 0)
	    min_pos_root = std::min(min_pos_root, root1);
	  if (root2 >= 0)
	    min_pos_root = std::min(min_pos_root, root2);
	  
	  double bound = LMQ_lower_bound(f);
	  if (min_pos_root == HUGE_VAL)
	    UNIT_TEST_CHECK_EQUAL(bound, HUGE_VAL);
	  else
	    UNIT_TEST_CHECK(bound <= min_pos_root);
	}

  //Test constant coefficients 
  UNIT_TEST_CHECK_EQUAL(LMQ_lower_bound(expand(1 + 0 * x*x*x*x*x)), HUGE_VAL);
}

UNIT_TEST( poly_root_tests)
{
  using namespace sym;
  const Polynomial<1> x{0, 1};

  {//Check cubic
    auto f1 = expand(4 * (x*x*x) - x * x - 2 * x + 12);
    
    auto roots = solve_real_roots(f1);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 1u);
    UNIT_TEST_CHECK_CLOSE(roots[0], -1.472711896724616002268033950475380144341, 1e-10);
    
    auto droots = solve_real_roots(derivative(f1, Var<>()));
    UNIT_TEST_CHECK_EQUAL(droots.size(), 2u);
    UNIT_TEST_CHECK_CLOSE(droots[0], -1.0/3, 1e-10);
    UNIT_TEST_CHECK_CLOSE(droots[1], 0.5, 1e-10);
  }

  {//Check quartic
    auto f1 = expand(10 * (x*x*x*x) + x*x*x - 30 * x * x -23);

    auto roots = solve_real_roots<>(f1);
    UNIT_TEST_CHECK_EQUAL(roots.size(), 2u);
    UNIT_TEST_CHECK_CLOSE(roots[0], -1.949403904489790210996459054473124835057, 1e-10);
    UNIT_TEST_CHECK_CLOSE(roots[1], +1.864235880634589025006445510389799368569, 1e-10);

    auto droots = solve_real_roots(derivative(f1, Var< >()));
    UNIT_TEST_CHECK_EQUAL(droots.size(),3u);
    UNIT_TEST_CHECK_CLOSE(droots[0], -1.262818836058599076329128653113014315066, 1e-10);
    UNIT_TEST_CHECK_CLOSE(droots[1], 0, 1e-10);
    UNIT_TEST_CHECK_CLOSE(droots[2], +1.187818836058599076329128653113014315066, 1e-10);
  }

  {//PowerOp quartic
    auto f1 = expand(pow(30 * x * x + x - 23, C<2>()));

    auto roots = solve_real_roots(f1);
    //FIX THIS UNIT TEST!
    UNIT_TEST_CHECK_EQUAL(roots.size(), 2u);
    ////NOTE THESE ROOTS ARE DOUBLE ROOTS (roots.size() may equal 2,3, or 4)
    UNIT_TEST_CHECK_CLOSE(roots[0], -0.8924203103613100773375343963347855860436, 1e-7);
    UNIT_TEST_CHECK_CLOSE(roots[1], -0.8924203103613100773375343963347855860436, 1e-7);

    auto droots = solve_real_roots(derivative(f1, Var< >()));
    UNIT_TEST_CHECK_EQUAL(droots.size(), 3u);
    UNIT_TEST_CHECK_CLOSE(droots[0], -0.8924203103613100773375343963347855860436, 1e-10);
    UNIT_TEST_CHECK_CLOSE(droots[1], -0.01666666666666666666666666666666666666667, 1e-10);
    UNIT_TEST_CHECK_CLOSE(droots[2], +0.8590869770279767440042010630014522527103, 1e-10);
  }
}

UNIT_TEST( generic_solve_real_roots )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};
  const Polynomial<1, long double> x_hp{0, 1};

  std::vector<double> roots{-0.14159265, -3.14159265, -0.0001,0.1, 0.3333, 0.6, 1.001, 2.0, 3.14159265};

  //MSVC (long double) == (double), thus there is not the precision required to test some of these roots on MSVC!
  if ((std::numeric_limits<double>::digits == std::numeric_limits<long double>::digits))
    return;
  
  for (auto it1 = roots.begin(); it1 != roots.end(); ++it1)
    for (auto it2 = it1; it2 != roots.end(); ++it2)
      for (auto it3 = it2; it3 != roots.end(); ++it3)
	for (auto it4 = it3; it4 != roots.end(); ++it4)
	  for (auto it5 = it4; it5 != roots.end(); ++it5)
	    for (int sign : {-1, +1})
	      {
		StackVector<double, 5> test_roots{*it1, *it2, *it3, *it4, *it5};

		//Test where all 5 roots of a 5th order Polynomial are real
		auto f1_hp = expand(sign * (x_hp - *it1) * (x_hp - *it2) * (x_hp - *it3) * (x_hp - *it4) * (x_hp - *it5));
		//Test where 5 roots of a 7th order Polynomial are real
		auto f2_hp = expand(f1_hp * (x_hp * x_hp - 3 * x_hp + 4));
		//Test where 5 roots of a 9th order Polynomial are real
		auto f3_hp = expand(f1_hp * (x_hp * x_hp - 3 * x_hp + 4) * (x_hp * x_hp - 3 * x_hp + 30));

		//long double precision is used to calculate the
		//coefficients of the polynomial, as loss of precision
		//is significant here.
		Polynomial<5> f1 = convert_poly<double>(f1_hp);
		Polynomial<7> f2 = convert_poly<double>(f2_hp);
		Polynomial<9> f3 = convert_poly<double>(f3_hp);
		
		//Test the default implementation
		compare_roots(solve_real_roots(f1), test_roots, f1);
		compare_roots(solve_real_roots(f2), test_roots, f2);
		compare_roots(solve_real_roots(f3), test_roots, f3);

		//These root finding methods only work for
		//squarefree polynomials, so we only test against those
		if (std::set<double>{*it1, *it2, *it3, *it4, *it5}.size() == 5) {
		  //Test VCA + BISECTION
		  compare_roots(solve_real_roots<PolyRootBounder::VCA, PolyRootBisector::BISECTION>(f1), test_roots, f1);
		  compare_roots(solve_real_roots<PolyRootBounder::VCA, PolyRootBisector::BISECTION>(f2), test_roots, f2);
			
		  //Test VAS + BISECTION
		  compare_roots(solve_real_roots<PolyRootBounder::VAS, PolyRootBisector::BISECTION>(f1), test_roots, f1);
		  compare_roots(solve_real_roots<PolyRootBounder::VAS, PolyRootBisector::BISECTION>(f2), test_roots, f2);
		}
	      }
}

UNIT_TEST( generic_solve_real_roots_2 )
{
  using namespace sym;
  const Polynomial<1> x{0, 1};
  std::cout.precision(20);
  //Roots are, -1+-i, 0.5*(1+-i\sqrt{11})
  //std::cout << LinBairstowSolve(Polynomial<4>{6,4,3,1,1}, 1e-14) << std::endl;
}

UNIT_TEST( quartic_root_tests)
{
  //Equations with zeros in them are an edge case for the bounds check in the Sturm solver
  {
    auto roots = sym::solve_real_roots(sym::Polynomial<4> {1, 0, 0, 1, 1});
    UNIT_TEST_CHECK_EQUAL(roots.size(), 0u);
  }


  {
    //https://www.wolframalpha.com/input?i2d=true&i=35842-x*57720%2Bx*x*59799.999999999985448084771633148-x*x*x*5.2252677024996035907674102414044e-12%2Bx*x*x*x*2.0194839173657902218540251271239e-28
    sym::Polynomial<4> g = {
      35842,
      -57720,
      59799.999999999985448084771633148,
      -5.2252677024996035907674102414044e-12,
      2.0194839173657902218540251271239e-28
    };

    auto roots = sym::solve_real_roots(g);
    DISABLED_UNIT_TEST_CHECK_EQUAL(roots.size(), 0u);

    //https://www.wolframalpha.com/input?i2d=true&i=14482.000000000001818989403545856-83591.999999999985448084771633148*x%2B84887.999999999970896169543266296*x*x%2Bx*x*x*1.3022974889306702395208061776649e-11%2Bx*x*x*x*8.0779356694631608874161005084957e-28
    sym::Polynomial<4> f = {
      14482.000000000001818989403545856,
      -83591.999999999985448084771633148,
      84887.999999999970896169543266296,
      1.3022974889306702395208061776649e-11,
      8.0779356694631608874161005084957e-28
    };

    roots = sym::solve_real_roots(f);
    DISABLED_UNIT_TEST_CHECK_EQUAL(roots.size(), 2u);

    sym::StackVector<double, 2> f_roots{
      0.2243674923730396943773527317913,
      0.760365332054441265251282862558
    };

    compare_roots(roots, f_roots, f);
  }
}
