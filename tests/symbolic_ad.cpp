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

template<typename Func>
void runtests()
{
  Func F;
  
  sym::Var<sym::vidx<'x'>> x;
  sym::Var<sym::vidx<'y'>> y;

  //Check derivatives of doubles are zero (apart from the zeroth derivative)
  {
    auto v = sym::ad<3>(F(1.23), x=0);
    UNIT_TEST_CHECK_EQUAL(v[0], 1.23);
    UNIT_TEST_CHECK_EQUAL(v[1], 0);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Check integer derivatives
  {
    auto v = sym::ad<3>(F(5), x=2.2);
    UNIT_TEST_CHECK_EQUAL(v[0], 5);
    UNIT_TEST_CHECK_EQUAL(v[1], 0);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Check symbolic constant derivatives
  {
    auto v = sym::ad<5>(F(sym::C<1,2>()), x=0);
    UNIT_TEST_CHECK_EQUAL(v[0], 0.5);
    UNIT_TEST_CHECK_EQUAL(v[1], 0);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
    UNIT_TEST_CHECK_EQUAL(v[4], 0);
    UNIT_TEST_CHECK_EQUAL(v[5], 0);
  }

  //Check variable derivatives are correct
  {
    auto v = sym::ad<3>(F(x), x=3);
    UNIT_TEST_CHECK_EQUAL(v[0], 3);
    UNIT_TEST_CHECK_EQUAL(v[1], 1);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Check incorrect variable derivatives are zero (with NaN for the evaluation
  {
    auto v = sym::ad<3>(F(y), x=3);
    UNIT_TEST_CHECK(std::isnan(v[0]));
    UNIT_TEST_CHECK_EQUAL(v[1], 0);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Addition
  {
    auto v = sym::ad<3>(F(x+3), x=3);
    UNIT_TEST_CHECK_EQUAL(v[0], 6);
    UNIT_TEST_CHECK_EQUAL(v[1], 1);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Subtraction
  {
    auto v = sym::ad<3>(F(3-x), x=2);
    UNIT_TEST_CHECK_EQUAL(v[0], 1);
    UNIT_TEST_CHECK_EQUAL(v[1], -1);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Multiplication
  //Remember that ad actually returns d^n f/dx^n / (n!)
  {
    auto v = sym::ad<2>(F(3*x), x=2);
    UNIT_TEST_CHECK_EQUAL(v[0], 6);
    UNIT_TEST_CHECK_EQUAL(v[1], 3);
    UNIT_TEST_CHECK_EQUAL(v[2], 0);
  }
  {
    auto v = sym::ad<3>(F(x*x), x=3);
    UNIT_TEST_CHECK_EQUAL(v[0], 9);
    UNIT_TEST_CHECK_EQUAL(v[1], 6);
    UNIT_TEST_CHECK_EQUAL(v[2], 1);
    UNIT_TEST_CHECK_EQUAL(v[3], 0);
  }

  //Division
  {
    auto v = sym::ad<3>(F((x+1)*(x-2)/(x+3)), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], 2.0/3, 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], 13.0/18, 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], 5.0/54/2, 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[3], -5.0/108/6, 1e-14);
  }

  //Power (of constants)
  {
    auto v = sym::ad<3>(F(sym::pow(x, 3.5)), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], std::pow(3, 3.5), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], 3.5 * std::pow(3, 2.5) * sym::InvFactorial<1>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], 3.5 * 2.5 * std::pow(3, 1.5) * sym::InvFactorial<2>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[3], 3.5 * 2.5 * 1.5 * std::pow(3, 0.5) * sym::InvFactorial<3>::value(), 1e-14);
  }
  
  //Exponentiation
  {
    auto v = sym::ad<4>(F(sym::exp(x)), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], std::exp(3), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], std::exp(3) * sym::InvFactorial<1>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], std::exp(3) * sym::InvFactorial<2>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[3], std::exp(3) * sym::InvFactorial<3>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[4], std::exp(3) * sym::InvFactorial<4>::value(), 1e-14); 
  }

  {
    auto f = sym::exp(x * x);
    auto v = sym::ad<2>(F(f), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], std::exp(3*3), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], (2 * 3 * std::exp(3*3)) * sym::InvFactorial<1>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], (2 * 3 * 3 + 1) * (2 * std::exp(3*3)) * sym::InvFactorial<2>::value(), 1e-14);
  }

  //Logarithm
  {
    auto v = sym::ad<3>(F(sym::log(x)), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], std::log(3), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], (1 / 3.0) * sym::InvFactorial<1>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], (-1 / 3.0 / 3.0) * sym::InvFactorial<2>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[3], (2 / 3.0 / 3.0 / 3.0) * sym::InvFactorial<3>::value(), 1e-14);
  }

  //Cosine/Sine
  {
    auto v = sym::ad<3>(F(sym::cos(x)), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], +std::cos(3), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], -std::sin(3) * sym::InvFactorial<1>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], -std::cos(3) * sym::InvFactorial<2>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[3], +std::sin(3) * sym::InvFactorial<3>::value(), 1e-14);
  }
  {
    auto v = sym::ad<2>(F(sym::sin(x*x)), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], +std::sin(3*3), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], +2 * 3 * std::cos(3*3) * sym::InvFactorial<1>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], 2 * (std::cos(3 * 3)- 2 * 3 * 3 * std::sin(3 * 3)) * sym::InvFactorial<2>::value(), 1e-14);
  }

  //Generalized power law
  {
    auto v = sym::ad<2>(F(sym::pow(x,x)), x=3);
    UNIT_TEST_CHECK_CLOSE(v[0], std::pow(3,3), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[1], std::pow(3,3) * (std::log(3) + 1) * sym::InvFactorial<1>::value(), 1e-14);
    UNIT_TEST_CHECK_CLOSE(v[2], std::pow(3,3) * (1.0 / 3 +std::pow(std::log(3) + 1, 2)) * sym::InvFactorial<2>::value(), 1e-14);
  }
}

struct Bypass {
  template<typename T>
  const T& operator()(const T& v) { return v; }
};

UNIT_TEST( automatic_differentiation_compiletime )
{  
  runtests<Bypass>();
}

struct ConvertToExpr {
  template<typename T>
  sym::Expr operator()(const T& v) { return sym::Expr(v); }
};

UNIT_TEST( automatic_differentiation_runtime )
{  
  runtests<ConvertToExpr>();
}
