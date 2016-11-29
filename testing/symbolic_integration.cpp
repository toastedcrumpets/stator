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
#include <vector>
#include <cmath>
#include <complex>

//stator
#include <stator/symbolic/symbolic.hpp>

//C++
#include <iostream>

//boost
#define BOOST_TEST_MODULE Integration_test
#include <boost/test/included/unit_test.hpp>

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

#define CHECK_TYPE(EXPRESSION, TYPE) static_assert(std::is_same<std::decay<decltype(EXPRESSION)>::type, TYPE>::value, "Type of calculation is not what was expected!")

BOOST_AUTO_TEST_CASE( symbolic_integration_variable )
{ 
  using namespace sym;  

  Variable<'x'> x;
  Variable<'y'> y;
  //Check basic integration of constants
  CHECK_TYPE(integrate(Unity(), x), Variable<'x'>);
  CHECK_TYPE(integrate(Null(), x), Null);

  //Check integration of Variables
  BOOST_CHECK(compare_expression(integrate(x, x), C<1,2>() * pow<2>(x)));
  BOOST_CHECK(compare_expression(integrate(pow<3>(x), x), C<1,4>() * pow<4>(x)));
  BOOST_CHECK(compare_expression(integrate(3 * pow<3>(x), x), 3 * (C<1,4>() * pow<4>(x))));
  BOOST_CHECK(compare_expression(C<3>() * integrate(pow<3>(x), x), C<3,4>() * pow<4>(x)));
  BOOST_CHECK(compare_expression(integrate(y, x), y * x));
  BOOST_CHECK(compare_expression(integrate(pow<2>(y), x), pow<2>(y) * x));
}
