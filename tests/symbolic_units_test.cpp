/*
  Copyright (C) 2021 Marcus Bannerman <m.bannerman@gmail.com>

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
#include <stator/symbolic/units.hpp>
#define UNIT_TEST_SUITE_NAME Symbolic_
#define UNIT_TEST_GOOGLE
#include <stator/unit_test.hpp>

UNIT_TEST( symbolic_units_static ) {

}


#include <stator/symbolic/runtime.hpp>

UNIT_TEST(symbolic_units_dynamic) {

    auto e = sym::Expr("1.2{m^2/s}");
    UNIT_TEST_CHECK_EQUAL(sym::repr(e), "1.2{m^2/s}");
}
