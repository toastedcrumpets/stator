/*
  Copyright (C) 2015 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#include <stator/orphan/stack_vector.hpp>
#include <stator/unit_test.hpp>

using namespace stator::orphan;

UNIT_TEST( StackVector_size )
{
  StackVector<int, 3> vec;
  UNIT_TEST_CHECK(vec.size() == 0);
  UNIT_TEST_CHECK(vec.empty());  
  
  vec.push_back(1);
  UNIT_TEST_CHECK(vec.size() == 1);
  UNIT_TEST_CHECK(!vec.empty());  

  vec.push_back(2);
  UNIT_TEST_CHECK(vec.size() == 2);
  UNIT_TEST_CHECK(!vec.empty());  
}

UNIT_TEST( StackVector_initializer_list )
{
  StackVector<double, 3> vec1{};
  UNIT_TEST_CHECK(vec1.size() == 0);
  UNIT_TEST_CHECK(vec1.empty());  
  
  StackVector<double, 3> vec2{0.5, 0.25};
  UNIT_TEST_CHECK(vec2.size() == 2);
  UNIT_TEST_CHECK(!vec2.empty());
  UNIT_TEST_CHECK_EQUAL(vec2[0], 0.5);
  UNIT_TEST_CHECK_EQUAL(vec2[1], 0.25);
  
  StackVector<double, 3> vec3{0.5, 0.25, 0.125};
  UNIT_TEST_CHECK(vec3.size() == 3);
  UNIT_TEST_CHECK(!vec3.empty());
  UNIT_TEST_CHECK_EQUAL(vec3[0], 0.5);
  UNIT_TEST_CHECK_EQUAL(vec3[1], 0.25);
  UNIT_TEST_CHECK_EQUAL(vec3[2], 0.125);

  StackVector<double, 3> vec4{0.5, 0.25, 0.125, 0.1};
  UNIT_TEST_CHECK(vec4.size() == 3);
  UNIT_TEST_CHECK(!vec4.empty());
  UNIT_TEST_CHECK_EQUAL(vec4[0], 0.5);
  UNIT_TEST_CHECK_EQUAL(vec4[1], 0.25);
  UNIT_TEST_CHECK_EQUAL(vec4[2], 0.125);
}

UNIT_TEST( StackVector_foreach )
{
  StackVector<double, 3> vec4{0.5, 0.25, 0.125};

  double sum = 0.0;
  for (const auto& val: vec4)
    sum += val;
  UNIT_TEST_CHECK_CLOSE(sum, 0.875, 0.001);

  for (auto& val: vec4)
    val *= 2;
  UNIT_TEST_CHECK_CLOSE(vec4[0], 1.0, 0.001);
}
