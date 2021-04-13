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
#include <stator/symbolic/symbolic.hpp>
#include <stator/unit_test.hpp>

using namespace sym;

UNIT_TEST( symbolic_parser_tokenizer )
{
  //Some basic checks, such as empty strings, or individual tokens
  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer("").next(), "");
  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer("  ").next(), "");
  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer("23 ").next(), "23");
  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer("1.11 ").next(), "1.11");
  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer("   1.11e12 ").next(), "1.11e12");
  UNIT_TEST_CHECK_EQUAL(detail::ExprTokenizer(" 1.23*12 ").next(), "1.23");

  //Full test of a expression parse (with deliberately poor formatting choices)
  //////Unit tests fordetail::ExprTokenizer::P()
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
    tk.consume();
    UNIT_TEST_CHECK_EQUAL(tk.next(), "");
  }
}

UNIT_TEST( symbolic_parser_P )
{
  //Test float recognition and parsing
  {
    detail::ExprTokenizer tk(" 1.23*1.2 ");
    Expr v = tk.parseToken();
    UNIT_TEST_CHECK_EQUAL(v.as<double>(), 1.23);
  }

  //Test variable recognition and parsing
  {
    detail::ExprTokenizer tk(" p+2");
    Expr v = tk.parseToken();
    shared_ptr<const VarRT> v2 = dynamic_pointer_cast<const VarRT>(v);
    
    UNIT_TEST_CHECK(bool(v2));
    UNIT_TEST_CHECK_EQUAL(v2->idx, 'p');
  }
}

UNIT_TEST( symbolic_parser_Exp ) {

  {
    detail::ExprTokenizer tk("1.23");
    Expr v = tk.parseExpression();
    UNIT_TEST_CHECK_EQUAL(v.as<double>(), 1.23);
  }

  //Test of Add
  {
    detail::ExprTokenizer tk("1.23+2.3");
    Expr v = tk.parseExpression();    
    UNIT_TEST_CHECK_EQUAL(simplify(v).as<double>(), 3.53);
  }

  //Test of multiply
  {
    detail::ExprTokenizer tk("  2*3 * 4 ");
    Expr v = tk.parseExpression();
    UNIT_TEST_CHECK_EQUAL(simplify(v).as<double>(), 24.0);
  }

  //Test of operator precendence between add and multiply
  {
    detail::ExprTokenizer tk("2*3+4");
    Expr v = tk.parseExpression();    
    UNIT_TEST_CHECK_EQUAL(simplify(v).as<double>(), 10.0);
  } 
  {
    detail::ExprTokenizer tk("4+2*3");
    Expr v = tk.parseExpression();    
    UNIT_TEST_CHECK_EQUAL(simplify(v).as<double>(), 10.0);
  } 

  //Test of right-associativity of power operations
  {
    detail::ExprTokenizer tk("2^3^4");
    Expr v = tk.parseExpression();    
    UNIT_TEST_CHECK_CLOSE(simplify(v).as<double>(), 2.417851639e24, 1e-10);
  } 

  //Test of parenthesis having high binding power
  {
    detail::ExprTokenizer tk("(2 + 2 )^3");
    Expr v = tk.parseExpression();    
    UNIT_TEST_CHECK_EQUAL(simplify(v).as<double>(), 64);
  } 
  
  //Test of unary operator (sine) binding power
  {
    detail::ExprTokenizer tk("2*sin1");
    Expr v = tk.parseExpression();
    UNIT_TEST_CHECK_CLOSE(simplify(v).as<double>(), 1.682941969615793, 1e-10);
  }   

  //Test of unary operator (sine) binding power
  {
    detail::ExprTokenizer tk("sin1*2");
    Expr v = tk.parseExpression();
    UNIT_TEST_CHECK_CLOSE(simplify(v).as<double>(), 1.682941969615793, 1e-10);
  }   

  //Test of unary operator (cosine/sine) binding power
  {
    detail::ExprTokenizer tk("cos sin 1");
    Expr v = tk.parseExpression();
    UNIT_TEST_CHECK_CLOSE(simplify(v).as<double>(), 0.6663667453928805, 1e-10);
  }

  //Test of division/multiplication binding
  {
    detail::ExprTokenizer tk("2.0/3.0*4.0");
    Expr v = tk.parseExpression();
    UNIT_TEST_CHECK_CLOSE(simplify(v).as<double>(), 2.0/3.0*4.0, 1e-10);
  }

  //Test unary positive and negative parsing
  {
    detail::ExprTokenizer tk("+---+-+1");
    Expr v = tk.parseExpression();
    UNIT_TEST_CHECK_CLOSE(simplify(v).as<double>(), 1, 1e-10);
  }
}

template<class T>
void expr_string_expr_conversion_check(const T in_expr) {
  std::ostringstream os;
  os << in_expr;
  std::string initial_string = os.str();
  os.str("");
  os.clear();
  
  Expr e(initial_string);
  os << e;
  std::string final_string = os.str();
  UNIT_TEST_CHECK_EQUAL(initial_string, final_string);
}

UNIT_TEST( symbolic_parser_Expr_string_loop ) {
  //Check for consistency between saving and loading of expressions
  Var<vidx<'x'>> x;
  Var<vidx<'T'>> T;

  expr_string_expr_conversion_check(simplify(80*T*(1-log(T))+-318862-T*-410.30773724552921));
  expr_string_expr_conversion_check(x * (-123.2));
  expr_string_expr_conversion_check(1 - x * (-123.2));
  expr_string_expr_conversion_check(x + sin(x));
  expr_string_expr_conversion_check(x*x*x+x+x*sin(x));
  expr_string_expr_conversion_check(2*x/2*x);
  expr_string_expr_conversion_check(x*(x*x));
}

UNIT_TEST( Parsing_Errors )
{
  try {
    Expr("");
    UNIT_TEST_ERROR("Blank expression was parsed without error");
  } catch (const stator::Exception& e) {
  }

  try {
    Expr("T-2)");
    UNIT_TEST_ERROR("Unmatched right parenthesis was parsed without error");
  } catch (const stator::Exception& e) {
  }

  try {
    Expr("((T-2)*2");
    UNIT_TEST_ERROR("Unmatched left parenthesis was parsed without error");
  } catch (const stator::Exception& e) {
  }
}
