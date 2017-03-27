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

#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <chrono>
#include <cmath>

class UnitTests {
public:

  static UnitTests& get() {
    static UnitTests instance;
    return instance;
  }

  void register_test(std::string name, std::function<void()> cb) {
    _tests.emplace_back(name, cb);
  }

  void run_tests() {
    for (auto& test : _tests) {
      std::cout << "### Running test: " << test._name << "\n";
      _running_test_name = test._name;
      auto start = std::chrono::steady_clock::now();
      try {
	test._callback();
      } catch (std::exception& e) {
	std::cerr << "Exception caught in test " << test._name << std::endl;
	throw e;
      } catch (...) {
	std::cerr << "Unknown exception thrown" << std::endl;
	throw;
      }
      auto end = std::chrono::steady_clock::now();
      std::cout << "## " << test._name << " complete in " << std::chrono::duration <double, std::nano> (end-start).count() / 1e6 << " ms" << std::endl;
    }
  }

  template<class L, class R>
  void check_equal(const L& l, const R& r, std::string file, int line, std::string Lname, std::string Rname) {
    if (l != r)
      std::cerr << file << "(" << line << "): error in \"" << _running_test_name << "\": check " << Lname << " == " << Rname << " failed, " << l << " != " << r << std::endl;
  }

  void check(bool l, std::string file, int line, std::string Lname) {
    if (!l)
      std::cerr << file << "(" << line << "): error in \"" << _running_test_name << "\": check " << Lname << " failed" << std::endl;
  }

  template<class L, class R, class Tol_t>
  void check_close(L l, R r,  std::string file, int line, std::string Lname, std::string Rname, Tol_t tol) {
    if (!((std::abs(l - r) / std::abs(l) <= tol) && (std::abs(l - r) / std::abs(r) <= tol)))
      std::cerr << file << "(" << line << "): error in \"" << _running_test_name << "\": difference between " << Lname << "{"<<l<<"} and " << Rname << "{"<<r<<"} exceeds " << tol/100.00 << "%" << std::endl;
  }

  template<class T, class Tol_t>
  void check_small(T l,  std::string file, int line, std::string Lname,  Tol_t tol) {
    if (!(std::abs(l) <= std::abs(tol)))
      std::cerr << file << "(" << line << "): error in \"" << _running_test_name << "\": absolute value of " << Lname << "{"<<l<<"} exceeds " << tol << std::endl;
  }

  void error(std::string msg, std::string file, int line) {
    std::cerr << file << "(" << line << "): error in \"" << _running_test_name << "\": " << msg << std::endl;
  }
  
private:
  struct Test {
    Test(std::string name, std::function<void()> cb): _name(name), _callback(cb) {}
    
    std::string _name;
    std::function<void()> _callback;    
  };
  
  std::vector<Test> _tests;

  std::string _running_test_name;
};

struct UnitTestRegisterer {
  UnitTestRegisterer(std::string name, std::function<void()> cb) {
    UnitTests::get().register_test(name, cb);
  };
};

#define UNIT_TEST(A) void A(); UnitTestRegisterer A ## _reg(#A, A); void A()

#define UNIT_TEST_CHECK_EQUAL(A, B) UnitTests::get().check_equal(A, B, __FILE__, __LINE__, #A, #B)
#define UNIT_TEST_CHECK(Expr) UnitTests::get().check(Expr, __FILE__, __LINE__, #Expr)
#define UNIT_TEST_CHECK_CLOSE(A, B, TOL) UnitTests::get().check_close(A, B, __FILE__, __LINE__, #A, #B, TOL)
#define UNIT_TEST_CHECK_SMALL(A, TOL) UnitTests::get().check_small(A, __FILE__, __LINE__, #A, TOL)
#define UNIT_TEST_ERROR(MSG) UnitTests::get().error(MSG, __FILE__, __LINE__)

int main(int argc, char** argv) { UnitTests::get().run_tests(); }
