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

//stator
#include <stator/symbolic/numeric.hpp>
#include <stator/unit_test.hpp>

UNIT_TEST( Bisection_of_cubic )
{
  double tolerance = std::numeric_limits<double>::epsilon() * 5;

  double f_target = 1e-50;

  auto f = [&f_target](double x) {
    return x * x * x - f_target;
  };
  
  while (f_target < 1e50) {
    const double x_target = std::cbrt(f_target);

    double x = 0;
    bool result = stator::numeric::bisection(f, x, 0.0, 1e50);
    UNIT_TEST_CHECK(result);
    UNIT_TEST_CHECK_CLOSE(x, x_target, tolerance);
    f_target *= 1.11;
  }
}

UNIT_TEST( Newton_Raphson_of_cubic )
{
  double tolerance = std::numeric_limits<double>::epsilon() * 5;

  double f_target = 1e-50;

  auto f = [&f_target](double x) {
    return std::array<double,2>{{x * x * x - f_target, 3 * x * x}};
  };
  
  while (f_target < 1e50) {
    const double x_target = std::cbrt(f_target);

    double x = 1;
    bool result = stator::numeric::newton_raphson(f, x, 0.0, 1e50);
    UNIT_TEST_CHECK(result);
    UNIT_TEST_CHECK_CLOSE(x, x_target, tolerance);
    f_target *= 1.11;
  }
}

UNIT_TEST( Halleys_method_of_cubic )
{
  double tolerance = std::numeric_limits<double>::epsilon() * 5;

  double f_target = 1e-50;

  auto f = [&f_target](double x) {
    return std::array<double,3>{{x * x * x - f_target, 3 * x * x, 6 * x}};
  };
  
  while (f_target < 1e50) {
    const double x_target = std::cbrt(f_target);

    double x = 1;
    bool result = stator::numeric::halleys_method(f, x, 0.0, 1e50);
    UNIT_TEST_CHECK(result);
    UNIT_TEST_CHECK_CLOSE(x, x_target, tolerance);
    f_target *= 1.11;
  }
}
