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

template<class T>
auto initArray() {
  auto A_ptr = T::create();
  auto& A = sym::detail::unwrap(A_ptr);
  A.resize({2,3});
  //Check array operators
  A[0][0] = 1;
  A[1][0] = 2;
  A[0][1] = 3;
  A[1][1] = 4;
  A[0][2] = 5;
  A[1][2] = 6;
  return A_ptr;
}

template<class T>
void test_array_impl(size_t storesize = 6) {
  {
    auto A_ptr = T::create();
    auto& A = sym::detail::unwrap(A_ptr);
    UNIT_TEST_CHECK_EQUAL(A.size(), 0);
    UNIT_TEST_CHECK_EQUAL(A.empty(), true);
  }
  
  auto A_ptr = initArray<T>();
  auto& A = sym::detail::unwrap(A_ptr);
  UNIT_TEST_CHECK_EQUAL(A.size(), 6);
  UNIT_TEST_CHECK_EQUAL(A.getStore().size(), storesize);
  UNIT_TEST_CHECK_EQUAL(A.empty(), false);
  UNIT_TEST_CHECK_EQUAL(A[0][0], 1);
  UNIT_TEST_CHECK_EQUAL(A[1][0], 2);
  UNIT_TEST_CHECK_EQUAL(A[0][1], 3);
  UNIT_TEST_CHECK_EQUAL(A[1][1], 4);
  UNIT_TEST_CHECK_EQUAL(A[0][2], 5);
  UNIT_TEST_CHECK_EQUAL(A[1][2], 6);

  {
    //Check read-only array access
    const T& B = A;
    
    UNIT_TEST_CHECK_EQUAL(B[0][0], 1);
    UNIT_TEST_CHECK_EQUAL(B[1][0], 2);
    UNIT_TEST_CHECK_EQUAL(B[0][1], 3);
    UNIT_TEST_CHECK_EQUAL(B[1][1], 4);
    UNIT_TEST_CHECK_EQUAL(B[0][2], 5);
    UNIT_TEST_CHECK_EQUAL(B[1][2], 6);
  }
  
  auto B_ptr = A + A;
  auto& B = sym::detail::unwrap(B_ptr);
  UNIT_TEST_CHECK_EQUAL(B[0][0], 2 * 1);
  UNIT_TEST_CHECK_EQUAL(B[1][0], 2 * 2);
  UNIT_TEST_CHECK_EQUAL(B[0][1], 2 * 3);
  UNIT_TEST_CHECK_EQUAL(B[1][1], 2 * 4);
  UNIT_TEST_CHECK_EQUAL(B[0][2], 2 * 5);
  UNIT_TEST_CHECK_EQUAL(B[1][2], 2 * 6);

  {
    auto C_ptr = B - A;
    auto& C = sym::detail::unwrap(C_ptr);
    UNIT_TEST_CHECK_EQUAL(C[0][0], 1);
    UNIT_TEST_CHECK_EQUAL(C[1][0], 2);
    UNIT_TEST_CHECK_EQUAL(C[0][1], 3);
    UNIT_TEST_CHECK_EQUAL(C[1][1], 4);
    UNIT_TEST_CHECK_EQUAL(C[0][2], 5);
    UNIT_TEST_CHECK_EQUAL(C[1][2], 6);
  }
}



UNIT_TEST( symbolic_array_staticD_dynamicStore ) {
  typedef sym::Array<double, sym::RowMajorAddressing<-1u, 2> > Array;
  test_array_impl<Array>();
}

UNIT_TEST(symbolic_array_staticD_staticStore) {
  typedef sym::Array<double, sym::RowMajorAddressing<9, 2>> Array;
  test_array_impl<Array>(9);
}

#include <stator/symbolic/runtime.hpp>

UNIT_TEST(symbolic_array_runtime) {
  auto A_ptr = sym::ArrayRT::create();
  auto& A = sym::detail::unwrap(A_ptr);

  UNIT_TEST_CHECK_EQUAL(A.size(), 0);
  UNIT_TEST_CHECK_EQUAL(A.getStore().size(), 0);
  UNIT_TEST_CHECK_EQUAL(A.empty(), true);

  A.resize(6);
  //Check array operators
  A[0] = 1;
  A[1] = 2;
  A[2] = 3;
  A[3] = 4;
  A[4] = 5;
  A[5] = 6;

  UNIT_TEST_CHECK_EQUAL(A.size(), 6);
  UNIT_TEST_CHECK_EQUAL(A.getStore().size(), 6);
  UNIT_TEST_CHECK_EQUAL(A.empty(), false);
  UNIT_TEST_CHECK_EQUAL(A[0], 1);
  UNIT_TEST_CHECK_EQUAL(A[1], 2);
  UNIT_TEST_CHECK_EQUAL(A[2], 3);
  UNIT_TEST_CHECK_EQUAL(A[3], 4);
  UNIT_TEST_CHECK_EQUAL(A[4], 5);
  UNIT_TEST_CHECK_EQUAL(A[5], 6);

  {
    //Check read-only array access
    const auto& B = A;
    
    UNIT_TEST_CHECK_EQUAL(B[0], 1);
    UNIT_TEST_CHECK_EQUAL(B[1], 2);
    UNIT_TEST_CHECK_EQUAL(B[2], 3);
    UNIT_TEST_CHECK_EQUAL(B[3], 4);
    UNIT_TEST_CHECK_EQUAL(B[4], 5);
    UNIT_TEST_CHECK_EQUAL(B[5], 6);
  }
  
  sym::Expr B = simplify(A + A);
  UNIT_TEST_CHECK_EQUAL(B.as<sym::ArrayRT>()[0].as<double>(), 2 * 1);
  UNIT_TEST_CHECK_EQUAL(B.as<sym::ArrayRT>()[1].as<double>(), 2 * 2);
  UNIT_TEST_CHECK_EQUAL(B.as<sym::ArrayRT>()[2].as<double>(), 2 * 3);
  UNIT_TEST_CHECK_EQUAL(B.as<sym::ArrayRT>()[3].as<double>(), 2 * 4);
  UNIT_TEST_CHECK_EQUAL(B.as<sym::ArrayRT>()[4].as<double>(), 2 * 5);
  UNIT_TEST_CHECK_EQUAL(B.as<sym::ArrayRT>()[5].as<double>(), 2 * 6);

  /*{
    auto C_ptr = B - A;
    auto& C = sym::detail::unwrap(C_ptr);
    UNIT_TEST_CHECK_EQUAL(C[0], 1);
    UNIT_TEST_CHECK_EQUAL(C[1], 2);
    UNIT_TEST_CHECK_EQUAL(C[2], 3);
    UNIT_TEST_CHECK_EQUAL(C[3], 4);
    UNIT_TEST_CHECK_EQUAL(C[4], 5);
    UNIT_TEST_CHECK_EQUAL(C[5], 6);
  }*/

  auto x_ptr = sym::VarRT::create("x");
  auto& x = *x_ptr;
  auto y_ptr = sym::VarRT::create("y");
  auto& y = *y_ptr;

  auto test_array_ptr = sym::ArrayRT::create({sym::Expr(1), x, y});
  auto& test_array = *test_array_ptr;

  UNIT_TEST_CHECK_EQUAL(sym::repr(test_array), "[1, x, y]");
  UNIT_TEST_CHECK_EQUAL(sym::repr(derivative(test_array, x)), "[0, 1, 0]");
  UNIT_TEST_CHECK_EQUAL(sym::sub(sym::ArrayRT::create({sym::Expr(1), x, y}), sym::Expr(x=2)), sym::Expr("[1,2,y]"));
  UNIT_TEST_CHECK_EQUAL(sym::simplify(sym::Expr("[1, 1, 1]")+sym::Expr("[0, 1, 2]")), sym::Expr("[1,2,3]"));
}


//UNIT_TEST( symbolic_list_basic )
//{
//  auto x_ptr = VarRT::create("x");
//  auto& x = *x_ptr;
//  UNIT_TEST_CHECK_EQUAL(sub(Expr("[1, x, y]"), Expr(x=2)), Expr("[1,2,y]"));
//  UNIT_TEST_CHECK_EQUAL(derivative(Expr("[1, x, y]"), x), Expr("[0,1,0]"));
//  UNIT_TEST_CHECK_EQUAL(simplify(Expr("[1, 1, 1]")+Expr("[0, 1, 2]")), Expr("[1,2,3]"));
//  UNIT_TEST_CHECK_EQUAL(sub(Expr("[1, x, y]"), Expr("{x:2, y:3}")), Expr("[1,2,3]"));
//  UNIT_TEST_CHECK_EQUAL(sub(Expr("[1, x, y]"), v), Expr("[1,2,3]"));
//}
