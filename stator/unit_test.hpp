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

#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <chrono>
#include <cmath>

class UnitTests {
public:
  UnitTests() : _error_counter(0) {}
  
  static UnitTests& get() {
    static UnitTests instance;
    return instance;
  }

  void register_test(std::string name, std::function<void()> cb) {
    _tests.emplace_back(name, cb);
  }

  int run_tests() {
    for (auto& test : _tests) {
      std::cout << "### Running test: " << test._name << "\n";
      _running_test_name = test._name;
      auto start = std::chrono::steady_clock::now();
      try {
	test._callback();
      } catch (std::exception& e) {
	_error_counter += 1;
	std::cerr << "Aborting test, exception thrown in \"" << test._name << "\":" << std::endl
		  << e.what() << std::endl;
	break;
      } catch (...) {
	std::cerr << "Aborting all tests, uncaught exception!" << std::endl;
	throw;
      }
      auto end = std::chrono::steady_clock::now();
      std::cout << "## " << test._name << " complete in " << std::chrono::duration <double, std::nano> (end-start).count() / 1e6 << " ms" << std::endl;
    }
    
    std::cout << "# Tests complete with " << _error_counter << " errors" << std::endl;
    return _error_counter > 0;
  }

  template<class L, class R>
  void check_equal(const L& l, const R& r, std::string file, int line, std::string Lname, std::string Rname) {
    if (l != r) {
      ++_error_counter;
      std::cerr << file << "(" << line << "): error in \"" << _running_test_name << "\": check " << Lname << " == " << Rname << " failed, " << l << " != " << r << std::endl;
    }
  }

  void check(bool l, std::string file, int line, std::string Lname) {
    if (!l) {
      ++_error_counter;
      std::cerr << file << "(" << line << "): error in \"" << _running_test_name << "\": check " << Lname << " failed" << std::endl;
    }
  }

  template<class L, class R, class Tol_t>
  void check_close(L l, R r,  std::string file, int line, std::string Lname, std::string Rname, Tol_t tol)
  {
    double difference = std::max(std::abs(l - r) / std::abs(l), std::abs(l - r) / std::abs(r));
    
    if (l == 0)
      difference = std::abs(r);

    //Check if the values are close
    if (difference <= tol) return;

    ++_error_counter;
    std::cerr << file << "(" << line << "): error in \"" << _running_test_name << "\": difference (" << difference*100 << "%) between " << Lname << "{"<<l<<"} and " << Rname << "{"<<r<<"} exceeds " << tol*100.00 << "%" << std::endl; 
  }

  template<class T, class Tol_t>
  void check_small(T l,  std::string file, int line, std::string Lname,  Tol_t tol) {
    if (!(std::abs(l) <= std::abs(tol))) {
      ++_error_counter;
      std::cerr << file << "(" << line << "): error in \"" << _running_test_name << "\": absolute value of " << Lname << "{"<<l<<"} exceeds " << tol << std::endl;
    }
  }

  void error(std::string msg, std::string file, int line) {
    ++_error_counter;
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

  size_t _error_counter;
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

int main() {
  try {
    return UnitTests::get().run_tests();
  } catch (const std::exception& e) {
    std::cerr << "Unit tests aborting due to exception:\n" << e.what() << std::endl;
    return 1;
  }
}
