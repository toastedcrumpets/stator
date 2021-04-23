/*
  Copyright (C) 2017 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#include <stator/symbolic/runtime.hpp>
#include <stator/exception.hpp>
#include <algorithm>
#include <iostream>
#include <cctype>
#include <map>
#include <sstream>

namespace sym {
  namespace detail {
    /*! \brief Implementation of expression tokenization and parsing into Expr types.

      The structure of the code:
        - The splitting of the string into tokens occurs in the ExprTokenizer::consume member function. 
        - Conversion of tokens into unary/prefix operators occurs in \ref ExprTokenizer::parseToken. 
	- Finally, the binary operator precedence and parsing takes place in .
	
      The implementation is largely based around the pseudocode
      implementations of Theodore Norvell from <a
      href="http://www.engr.mun.ca/~theo/Misc/exp_parsing.htm">here</a>
      and in particular <a
      href="http://www.engr.mun.ca/~theo/Misc/pratt_parsing.htm">here</a>.
      
      The algorithm is implemented over four key functions:
        1. Initialisation of all operator definitions takes place in the constructor ExprTokenizer::ExprTokenizer.
        2. Expression strings are first broken down into tokens. Tokens are substrings such as "2", "*", "sin", or "(". ExprTokenizer::next yields the current token, and the system is moved onto the next using ExprTokenizer::consume (where the actual tokenization takes place).
	3. Parsing of "leaves" of the Abstract Syntax Tree (AST), such as variables, numbers, including functions/prefix-operators are handled via ExprTokenizer::parseToken. Functions and prefix-operators may contain sub-trees and these are parsed recursively. 
	4. Full expression strings/trees are parsed via \ref ExprTokenizer::parseExpression. Its main purpose is to resolve binary operator precedence.

      The hardest part for me to understand from the work of Theodore
      Norvell was how the ExprTokenizer::parseExpression function
      created the AST via recursion and "precedence climbing". My
      notes on this are available in the documentation for
      ExprTokenizer::parseExpression.
    */
    class ExprTokenizer {
    public:
      ExprTokenizer(const std::string& str):
	_str(str),
	_start(0),
	_end(0)
      {
	//This is a discussion of parsing precedence and how it works.

	//Left operators are variables, numbers, functions, etc. They
	//are potentially the left operand of a binary operation.

	//Right operators always come after left operators, they can
	//be binary operators like "+" or they can be other things
	//like closing parenthesis.

	//Complex operations like the parenthetical grouping operator
	//"()" start with a left operator "(", then finish with a
	//right operator ")". On the other hand, array access starts
	//with a right operator "[" as it needs to bind to something
	//on the left and right of the "[", and it finishes with a
	//right operator "]".

	// Binding powers set which operator will bind to a
	// token. Right operators have two binding powers (left and
	// right), while left operators only have a right binding
	// power. The way Pratt parsing works is that it climbs up the
	// binding power in a loop, while recursing down in binding
	// power.  Ignoring the implementation details, lets look at an example

	//Token           :  sin  x    +    2    +    3
	//Binding powers  :  inf    10   11   10   11
	//Parsed Tree     :  (((sin  x) + 2) + 3)
	//
	// Sin has a very high (inf) binding power, as we want it to
	// grab whatever is to its right. Plus has a stronger right
	// binding power than left as we want it to be left
	// associative. To see this, we assigning tokens to the
	// operators either side of them with the highest binding
	// power which gives the resulting parse.
	
	_right_operators["="].reset(new BinaryOpToken<detail::Equality>());
	_right_operators["+"].reset(new BinaryOpToken<detail::Add>());
	_right_operators["-"].reset(new BinaryOpToken<detail::Subtract>());
	_right_operators["*"].reset(new BinaryOpToken<detail::Multiply>());
	_right_operators["/"].reset(new BinaryOpToken<detail::Divide>());
	_right_operators["^"].reset(new BinaryOpToken<detail::Power>());

	//The unary operators, slightly higher binding power than addition/subtraction
	_left_operators["+"].reset(new SkipToken<detail::Add::leftBindingPower+1>());
	_left_operators["-"].reset(new UnaryNegative<detail::Add::leftBindingPower+1>());
 
	_left_operators["("].reset(new ParenthesisToken);
	//A halt token stops parseExpression processing. The right
	//parenthesis should be handled by the previous entry.
	_right_operators[")"].reset(new HaltToken);

	//List construction token (i.e. [1,2, x + y])
	_left_operators["["].reset(new ListToken);

	//To allow commas to delimit statements/expressions i.e. in a list or dictionary.
	_right_operators[","].reset(new HaltToken); 
	
	//List access token (i.e. x[1])
	_right_operators["["].reset(new WrappedBinaryOpToken<detail::Array>("]"));

	//Halt token for list access AND list construction
	_right_operators["]"].reset(new HaltToken);

	//Dictionary construction
	_left_operators["{"].reset(new DictToken);
	_right_operators[":"].reset(new HaltToken); 
	_right_operators["}"].reset(new HaltToken);
	
	//Most unary operators have high binding powers to grab the very next argument
	_left_operators["sin"].reset(new UnaryOpToken<detail::Sine, std::numeric_limits<int>::max()>());
	_left_operators["cos"].reset(new UnaryOpToken<detail::Cosine, std::numeric_limits<int>::max()>());
	_left_operators["exp"].reset(new UnaryOpToken<detail::Exp, std::numeric_limits<int>::max()>());
	_left_operators["ln"].reset(new UnaryOpToken<detail::Log, std::numeric_limits<int>::max()>());
	
	//The actual tokenisation is done in the consume() member
	//function. The first call starts the process and clears the "end" state.
	consume();
      }

      std::string next() {
	return empty() ? "" : _str.substr(_start, _end - _start);
      }

      void expect(std::string token) {
	if (next() != token)
	  stator_throw() << "Expected " << ((token.empty()) ? "end of expression": ("\""+token+"\"")) << " but found " << (empty() ? "the end of expression" : next()) << "\" instead?\n" << parserLoc();
	consume();
      }

      bool empty() const {
	return _start == _str.size();
      }
      
      void consume() {
	_start = _end;

	//Skip whitespace
	while ((_str[_start] == ' ') && (_start < _str.size())) ++_start;

	_end = _start;

	//Check for sequence end
	if (empty()) return;

	//Not at end of sequence so at least one character in symbol
	_end = _start + 1;

	//Parse numbers with decimal points and (possible signed)
	//exponents. Signs at the front of numbers are parsed as unary
	//operators.
	if (std::isdigit(_str[_start])) {
	  consumeFloat();
	  return;
	}

	//Parsing a string
	if (std::isalpha(_str[_start])) {
	  while ((_end < _str.size()) && std::isalpha(_str[_end]))
	    ++_end;
	  return;
	}

	//Allow non-alpha single character operators (longer operators are usually strings and caught above!)
	if (_right_operators.find(_str.substr(_start, 1)) != _right_operators.end())
	  return;
	
	if (_left_operators.find(_str.substr(_start, 1)) != _left_operators.end())
	  return;
	
	stator_throw() << "Unrecognised token \"" << _str[_start] << "\"\n" << parserLoc();
      }

      void consumeFloat() {
	bool decimal = false;
	bool exponent = false;
      
	while (_end != _str.size()) {
	  if (_str[_end] == '.') {
	    if (!decimal && !exponent) {
	      decimal = true;
	      ++_end;
	      continue;
	    } else {
	      stator_throw() << "Unexpected decimal point?\n" << parserLoc();
	    }
	  }
	
	  if ((_str[_end] == 'e') || (_str[_end] == 'E')) {
	    if (!exponent) {
	      exponent = true;
	      decimal = true; //Don't allow decimals in the exponent
	      ++_end;

	      if (_end == _str.size())
		stator_throw() << "String ended during parsing of exponent\n" << parserLoc();

	      //Eat the exponent sign if present
	      if ((_str[_end] == '+') || (_str[_end] == '-'))
		++_end;
		
	      if (_end == _str.size())
		stator_throw() << "String ended during parsing of exponent\n" << parserLoc();
		
	      if (!std::isdigit(_str[_end]))
		stator_throw() << "Malformed exponent?\n" << parserLoc();
		
	      continue;
	    } else
	      stator_throw() << "Double exponent?\n" << parserLoc();
	  }
	  
	  if (std::isdigit(_str[_end])) {
	    ++_end;
	    continue;
	  }
	  
	  break;
	}
      }
      
      std::string parserLoc() {
	return _str + "\n"
	  + std::string(_start, ' ') + std::string((_start < _end) ? _end - _start -1: 0, '-') + "^";
      }

      struct RightOperatorBase {
	virtual ~RightOperatorBase() {}
	
	/*! \brief Takes left operand and returns the corresponding
            Expr, fetching the right operands from the tokenizer.
	 */
	virtual Expr apply(Expr, ExprTokenizer&) const = 0;
	/*! \brief Left binding power (Precedence of this operator)*/
	virtual int LBP() const = 0;
	/*! \brief Next binding power (highest precedence of the operator that this operator can be a left operand of)*/
	virtual int NBP() const = 0;
      };

      struct LeftOperatorBase {
	virtual ~LeftOperatorBase() {}
	
	/*! \brief Takes one operand and returns the corresponding Expr.
	*/
	virtual Expr apply(ExprTokenizer&) const = 0;
	/*! \brief Binding power to the right arguments of the Token*/
	virtual int BP() const = 0;
      };
      
      template<class Op>
      struct BinaryOpToken : RightOperatorBase {
	Expr apply(Expr l, ExprTokenizer& tk) const {
	  return Op::apply(l, tk.parseExpression(detail::RBP<Op>()));
	}
	    
	int LBP() const { return Op::leftBindingPower; }

	int NBP() const { return detail::NBP<Op>(); }
      };

      struct HaltToken : RightOperatorBase {
	virtual Expr apply(Expr, ExprTokenizer&) const { stator_throw() << "Should never be called!"; }
	//To ensure it is always treated as a separate expression, its BP is negative (nothing can claim it)
	virtual int LBP() const { return -1; }
	int NBP() const { stator_throw() << "Should never be called!"; }
      };

      struct ParenthesisToken : public LeftOperatorBase {
	Expr apply(ExprTokenizer& tk) const {
	  Expr arg = tk.parseExpression(BP());
	  tk.expect(")");
	  return arg;
	}

	//Parenthesis bind the whole following expression
	int BP() const {
	  return 0;
	}
      };

      struct ListToken : public LeftOperatorBase {
	Expr apply(ExprTokenizer& tk) const {
	  List a;
	  
	  if (tk.next() == "]") {
	    tk.consume();
	    return a;
	  }
	  
	  while (true) {
	    a.push_back(tk.parseExpression(BP()));
	    if (tk.next() == "]") break;
	    tk.expect(",");
	  }
	  
	  tk.expect("]");
	  return a;
	}

	//Parenthesis bind the whole following expression
	int BP() const {
	  return 0;
	}
      };


      struct DictToken : public LeftOperatorBase {
	Expr apply(ExprTokenizer& tk) const {
	  Dict a;
	  
	  if (tk.next() == "}") {
	    tk.consume();
	    return a;
	  }
	  
	  while (true) {
	    Expr key_expr = tk.parseExpression(BP());
	    const VarRT& key = key_expr.as<VarRT>();
	    
	    tk.expect(":");
	    Expr value = tk.parseExpression(BP());
	    a[key.getID()] = value;
	    if (tk.next() == "}") break;
	    tk.expect(",");
	  }
	  
	  tk.expect("}");
	  return a;
	}

	//Parenthesis bind the whole following expression
	int BP() const {
	  return 0;
	}
      };
      
      template<class Op>
      struct WrappedBinaryOpToken : RightOperatorBase {
	WrappedBinaryOpToken(std::string end):
	  _end(end)
	{}
	
	Expr apply(Expr l, ExprTokenizer& tk) const {
	  Expr retval = Op::apply(l, tk.parseExpression(detail::RBP<Op>()));
	  tk.expect(_end);
	  return retval;
	}
	    
	int LBP() const { return Op::leftBindingPower; }

	int NBP() const { return detail::NBP<Op>(); }

	std::string _end;
      };

      
      template<class Op, int tBP>
      struct UnaryOpToken : LeftOperatorBase {
	Expr apply(ExprTokenizer& tk) const {
	  return Op::apply(tk.parseExpression(BP()));
	}

	int BP() const {
	  return tBP;
	}
      };

      template<int tBP>
      struct SkipToken : public LeftOperatorBase {
	Expr apply(ExprTokenizer& tk) const {
	  return tk.parseExpression(BP());
	}
	
	int BP() const {
	  return tBP;
	}
      };

      template<int tBP>
      struct UnaryNegative : public LeftOperatorBase {
	struct Visitor : VisitorHelper<Visitor> {
	  template<class T> Expr apply(const T& val) {
	    return -val;
	  }
	};
	
	Expr apply(ExprTokenizer& tk) const {
	  Visitor v;
	  return tk.parseExpression(BP())->visit(v);
	}
	
	int BP() const {
	  return tBP;
	}
      };
      
      std::map<std::string, shared_ptr<LeftOperatorBase> > _left_operators;
      std::map<std::string, shared_ptr<RightOperatorBase> > _right_operators;

      /*!\brief Parses a single token (unary/prefix op, variable, or
         number), where precedence issues do not arise.
       */
      Expr parseToken() {
	std::string token = next();
	consume();

	if (token.empty())
	  stator_throw() << "Unexpected end of expression?\n" << parserLoc();

	//Parse numbers
	if (std::isdigit(token[0])) {
	  std::stringstream ss;
	  ss << token;
	  double val;
	  ss >> val;
	  return Expr(val);
	}

	//Parse left operators
	auto it = _left_operators.find(token);
	if (it != _left_operators.end()) {
	  return it->second->apply(*this);
	}

	//Its not a prefix operator or a number, if it is a single
	//alpha character, then assume its a variable!
	
	for (const char& c : token)
	  if (!std::isalpha(c))
	    stator_throw() << "Could not parse \""+token+"\" as a valid token?\n" << parserLoc();
	    
	return Expr(VarRT(token));
      }

      /*!\brief Main parsing entry function.
	
	\arg minLBP The minimum binding power of an operator that this call
	can collect. A value of zero collects all operators.

	This function handles all cases where operator precedence
	arises.
	
	Expressions are streams of tokens. As the tokens are read
	left-to-right, we always begin with the leftmost leaf of the
	AST. The first call to parseExpression must then climb UP the
	AST to its top. The variable maxLBP ensures that each call to
	parseExpression only climbs up by ensuring that operators with
	higher precendence than p are handled by a recursion, possibly
	to become left leafs of a lower-precedence operator higher up
	the AST (low precedence operators always end up high up the
	AST). The variable minLBP is used to make sure that each
	recursive call to parseExpression does not climb above its
	parent parseExpression call.
	
	There are three types of precedence used.
	
	Left Binding Precedence (LBP) and Right Binding Precedence
	(RBP) are used to determine which operator binds to an
	argument between two operators. For example, in the expression
	2*3+4, the multiply binds to the 3 as its RBP is higher than
	the addition's LBP. If the LBP and RBP are equal, the right
	operator "wins". Thus operators who are left-associative must
	have their RBP > LBP, and right-associative operators must
	have RBP <= LBP.

	The last argument is the Next Binding Precedence (NBP). As
	parseExpression climbs up the AST, the NBP of each operator is
	used to determine the maximum LBP which will cause the
	parseExpression to replace its current root with a higher
	level. Generally, an operator must have NBP<=LBP. For
	left-associative operators we have NBP=LBP, and for right
	associative we have NBP<LBP, as we are moving from left to
	right along the tree and thus only left-associative operators
	allow us to ascend (which we do at each repeated
	left-associative operator).

       */
      Expr parseExpression(int minLBP = 0) {
	//Grab the first token (should be either a unary/prefix
	//operator or a number/variable all of which may be a LHS of a
	//multi arg operator (which are handled in this
	//function). Unary/prefix operators are handled directly by
	//parseToken()
	Expr t = parseToken();
	
	//maxLBP is only allowed to decrease as it ensures the while
	//loop climbs down the precedence tree (up the AST) to the
	//root of the expression.
	int maxLBP = std::numeric_limits<int>::max();
	while (true) {
	  std::string token = next();

	  //Handle the special case of end of string
	  if (token.empty()) break;

	  //Determine the type of operator found
	  auto it = _right_operators.find(token);

	  //If there is no match, then return an error
	  if (it == _right_operators.end())
	    stator_throw() << "Expected right operator but got \""<< token << "\"?\n" << parserLoc();

	  //If the operator has a lower binding power than what this
	  //call can collect (as limited by minLBP), then return. If
	  //an operator has already been collected and its next
	  //binding power (stored in maxLBP) doesn't allow it to
	  //collect this operator, then exit and allow the calling
	  //function to handle this operator.
	  if ((minLBP > it->second->LBP()) || (it->second->LBP() > maxLBP)) break;
	  
	  consume();
	  t = it->second->apply(t, *this);
	  maxLBP = it->second->NBP();
	}
	
	return t;
      }
      
    private:
    
      std::string _str;
      std::size_t _start;
      std::size_t _end;
    };
  }

  inline
  Expr::Expr(const std::string& str)
  {
    auto tokenizer = detail::ExprTokenizer(str);
    *this = simplify(tokenizer.parseExpression());

    //Check that the full string was parsed
    if (!tokenizer.empty())
      stator_throw() << "Parsing terminated unexpectedly early?\n" << tokenizer.parserLoc();
  }

  inline
  Expr::Expr(const char* str) : Expr(std::string(str)) {}
}
