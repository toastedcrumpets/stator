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
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>

//stator
#include <stator/symbolic/ad.hpp>
#include <stator/unit_test.hpp>

#include <random>

UNIT_TEST( automatic_differentiation )
{
  sym::Var<sym::vidx<'x'>> x;
  sym::Var<sym::vidx<'y'>> y;

  //Check derivatives of doubles are zero (apart from the zeroth derivative)
  {
    auto v = sym::ad<3>(1.23, x=0);
    UNIT_TEST_CHECK_EQUAL(v[0], 1.23);
    UNIT_TEST_CHECK_EQUAL(v[1], 0);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Check integer derivatives
  {
    auto v = sym::ad<3>(5, x=2.2);
    UNIT_TEST_CHECK_EQUAL(v[0], 5);
    UNIT_TEST_CHECK_EQUAL(v[1], 0);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Check symbolic constant derivatives
  {
    auto v = sym::ad<5>(sym::C<1,2>(), x=0);
    UNIT_TEST_CHECK_EQUAL(v[0], 0.5);
    UNIT_TEST_CHECK_EQUAL(v[1], 0);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
    UNIT_TEST_CHECK_EQUAL(v[4], 0);
    UNIT_TEST_CHECK_EQUAL(v[5], 0);
  }

  //Check variable derivatives are correct
  {
    auto v = sym::ad<3>(x, x=3);
    UNIT_TEST_CHECK_EQUAL(v[0], 3);
    UNIT_TEST_CHECK_EQUAL(v[1], 1);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Check incorrect variable derivatives are zero (with NaN for the evaluation
  {
    auto v = sym::ad<3>(y, x=3);
    UNIT_TEST_CHECK(std::isnan(v[0]));
    UNIT_TEST_CHECK_EQUAL(v[1], 0);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Addition
  {
    auto v = sym::ad<3>(x+3, x=3);
    UNIT_TEST_CHECK_EQUAL(v[0], 6);
    UNIT_TEST_CHECK_EQUAL(v[1], 1);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Subtraction
  {
    auto v = sym::ad<3>(3-x, x=2);
    UNIT_TEST_CHECK_EQUAL(v[0], 1);
    UNIT_TEST_CHECK_EQUAL(v[1], -1);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Multiplication
  //Remember that ad actually returns d^n f/dx^n / (n!)
  {
    auto v = sym::ad<2>(3*x, x=2);
    UNIT_TEST_CHECK_EQUAL(v[0], 6);
    UNIT_TEST_CHECK_EQUAL(v[1], 3);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
  }
  {
    auto v = sym::ad<3>(x*x, x=3);
    UNIT_TEST_CHECK_EQUAL(v[0], 9);
    UNIT_TEST_CHECK_EQUAL(v[1], 6);
    UNIT_TEST_CHECK_EQUAL(v[2], 1);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Division
  {
    auto v = sym::ad<3>((x+1)*(x-2)/(x+3), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], 2.0/3, 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], 13.0/18, 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], 5.0/54/2, 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[3], -5.0/108/6, 1e-14);
  }

  //Exponentiation
  {
    auto v = sym::ad<4>(sym::exp(x), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], std::exp(3), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], std::exp(3) * sym::InvFactorial<1>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], std::exp(3) * sym::InvFactorial<2>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[3], std::exp(3) * sym::InvFactorial<3>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[4], std::exp(3) * sym::InvFactorial<4>::value(), 1e-14); 
  }

  {
    auto f = sym::exp(x * x);
    auto df = derivative(f, x);
    auto ddf = derivative(df, x);
    auto v = sym::ad<2>(f, x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], sub(f, x=3), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], sub(df, x=3) * sym::InvFactorial<1>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], sub(ddf, x=3) * sym::InvFactorial<2>::value(), 1e-14);
  }
}

