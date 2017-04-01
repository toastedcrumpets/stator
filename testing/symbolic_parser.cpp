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

#include <iostream>
//stator
#include <stator/symbolic/parser.hpp>
#include <stator/unit_test.hpp>

using namespace sym;

UNIT_TEST( symbolic_parser_tokenizer )
{
  UNIT_TEST_CHECK(detail::ExprTokenizer("").empty());
  UNIT_TEST_CHECK(detail::ExprTokenizer("  ").empty());

  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer("23 ").next(), "23");
  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer("1.11 ").next(), "1.11");
  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer("   1.11e12 ").next(), "1.11e12");
  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer(" 1.23*12 ").next(), "1.23");

  {
    detail::ExprTokenizer tk(" 1.23* (12 + 4 )*exp(T)");
    UNIT_TEST_CHECK_EQUAL(tk.next(), "1.23");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "*");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "(");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "12");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "+");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "4");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), ")");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "*");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "exp");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "(");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "T");
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), ")");
  }
}
