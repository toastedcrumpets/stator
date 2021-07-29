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
#include <stator/symbolic/interval.hpp>
#define UNIT_TEST_SUITE_NAME Symbolic_
#define UNIT_TEST_GOOGLE
#include <stator/unit_test.hpp>

static constexpr char x_str[] = "x";
static constexpr char y_str[] = "y";

UNIT_TEST( symbolic_interval_basic ) {
  sym::interval<double> i1(0, 2);
  auto out = i1 * i1 - 2.0 * i1 + 1.0;

  UNIT_TEST_CHECK_EQUAL(out.lower(), -3.0);
  UNIT_TEST_CHECK_EQUAL(out.upper(), 5.0);

  sym::Var<x_str> x;

  {
    auto f = x * x - 2.0 * x + 1.0;
    auto res = sym::sub(f, x = i1);
    UNIT_TEST_CHECK_EQUAL(res.lower(), -3.0);
    UNIT_TEST_CHECK_EQUAL(res.upper(), 5.0);
  }

  UNIT_TEST_CHECK_EQUAL(sym::repr(out), "-3...5");
}
