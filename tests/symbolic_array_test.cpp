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
#include <stator/symbolic/array.hpp>
#define UNIT_TEST_SUITE_NAME Symbolic_
#define UNIT_TEST_GOOGLE
#include <stator/unit_test.hpp>

//static constexpr char x_str[] = "x";
//static constexpr char y_str[] = "y";

UNIT_TEST( symbolic_array_RowMajor ) {
  sym::RowMajor<2> rm({2, 3});
  UNIT_TEST_CHECK_EQUAL(rm.size(), 6);
  UNIT_TEST_CHECK_EQUAL((rm[{0,2}]), 2);
  UNIT_TEST_CHECK_EQUAL((rm[{1,2}]), 5);
  
}

UNIT_TEST( symbolic_array_static_access ) {
  sym::Array<double, 2> A;
  UNIT_TEST_CHECK_EQUAL(A.size(), 0);
  UNIT_TEST_CHECK_EQUAL(A.empty(), true);

  A.resize({2,3});
  UNIT_TEST_CHECK_EQUAL(A.size(), 6);
  UNIT_TEST_CHECK_EQUAL(A.getStore().size(), 6);
  UNIT_TEST_CHECK_EQUAL(A.empty(), false);

  //Check array operators
  A[0][0] = 1;
  A[1][0] = 2;
  A[0][1] = 3;
  A[1][1] = 4;
  A[0][2] = 5;
  A[1][2] = 6;
    
  UNIT_TEST_CHECK_EQUAL(A[0][0], 1);
  UNIT_TEST_CHECK_EQUAL(A[1][0], 2);
  UNIT_TEST_CHECK_EQUAL(A[0][1], 3);
  UNIT_TEST_CHECK_EQUAL(A[1][1], 4);
  UNIT_TEST_CHECK_EQUAL(A[0][2], 5);
  UNIT_TEST_CHECK_EQUAL(A[1][2], 6);

  //Check read-only array access
  const sym::Array<double, 2>& B = A;

  UNIT_TEST_CHECK_EQUAL(B[0][0], 1);
  UNIT_TEST_CHECK_EQUAL(B[1][0], 2);
  UNIT_TEST_CHECK_EQUAL(B[0][1], 3);
  UNIT_TEST_CHECK_EQUAL(B[1][1], 4);
  UNIT_TEST_CHECK_EQUAL(B[0][2], 5);
  UNIT_TEST_CHECK_EQUAL(B[1][2], 6);
}

UNIT_TEST(symbolic_array_dynamic_access) {
  sym::Array<double, -1u> A;
  
  UNIT_TEST_CHECK_EQUAL(A.size(), 0);
  UNIT_TEST_CHECK_EQUAL(A.empty(), true);

  A.resize({2,3});
  UNIT_TEST_CHECK_EQUAL(A.getAddressing()._dimensions, (std::vector<size_t>{2,3}));
  UNIT_TEST_CHECK_EQUAL(A.size(), 6);
  UNIT_TEST_CHECK_EQUAL(A.getStore().size(), 6);
  UNIT_TEST_CHECK_EQUAL(A.empty(), false);
  UNIT_TEST_CHECK_EQUAL(A[0][2]._coords, (std::vector<size_t>{0,2}));
  
  //Check array operators
  A[0][0] = 1;
  A[1][0] = 2;
  A[0][1] = 3;
  A[1][1] = 4;
  A[0][2] = 5;
  A[1][2] = 6;
    
  UNIT_TEST_CHECK_EQUAL(A[0][0], 1);
  UNIT_TEST_CHECK_EQUAL(A[1][0], 2);
  UNIT_TEST_CHECK_EQUAL(A[0][1], 3);
  UNIT_TEST_CHECK_EQUAL(A[1][1], 4);
  UNIT_TEST_CHECK_EQUAL(A[0][2], 5);
  UNIT_TEST_CHECK_EQUAL(A[1][2], 6);

  //Check read-only array access
  const sym::Array<double, -1u>& B = A;
  
  UNIT_TEST_CHECK_EQUAL(B[0][0], 1);
  UNIT_TEST_CHECK_EQUAL(B[1][0], 2);
  UNIT_TEST_CHECK_EQUAL(B[0][1], 3);
  UNIT_TEST_CHECK_EQUAL(B[1][1], 4);
  UNIT_TEST_CHECK_EQUAL(B[0][2], 5);
  UNIT_TEST_CHECK_EQUAL(B[1][2], 6);
}

UNIT_TEST(symbolic_array_stack_access) {
  sym::Array<double, 2, 9> A;
  
  UNIT_TEST_CHECK_EQUAL(A.size(), 0);
  UNIT_TEST_CHECK_EQUAL(A.empty(), true);

  A.resize({2,3});
  UNIT_TEST_CHECK_EQUAL(A.getAddressing()._dimensions, (std::array<size_t, 2>{2,3}));
  UNIT_TEST_CHECK_EQUAL(A.size(), 6);
  UNIT_TEST_CHECK_EQUAL(A.getStore().size(), 9);
  UNIT_TEST_CHECK_EQUAL(A.empty(), false);
  
  //Check array operators
  A[0][0] = 1;
  A[1][0] = 2;
  A[0][1] = 3;
  A[1][1] = 4;
  A[0][2] = 5;
  A[1][2] = 6;
    
  UNIT_TEST_CHECK_EQUAL(A[0][0], 1);
  UNIT_TEST_CHECK_EQUAL(A[1][0], 2);
  UNIT_TEST_CHECK_EQUAL(A[0][1], 3);
  UNIT_TEST_CHECK_EQUAL(A[1][1], 4);
  UNIT_TEST_CHECK_EQUAL(A[0][2], 5);
  UNIT_TEST_CHECK_EQUAL(A[1][2], 6);

  //Check read-only array access
  const auto& B = A;
  
  UNIT_TEST_CHECK_EQUAL(B[0][0], 1);
  UNIT_TEST_CHECK_EQUAL(B[1][0], 2);
  UNIT_TEST_CHECK_EQUAL(B[0][1], 3);
  UNIT_TEST_CHECK_EQUAL(B[1][1], 4);
  UNIT_TEST_CHECK_EQUAL(B[0][2], 5);
  UNIT_TEST_CHECK_EQUAL(B[1][2], 6);
}