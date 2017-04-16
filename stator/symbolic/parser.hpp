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
#include <cctype>
#include <map>
#include <sstream>

namespace sym {
  namespace detail {
    /*! \brief Implementation of expression tokenization and parsing into Expr types.

      The implementation is largely based around the pseudocode
      implementations of Theodore Norvell from <a
      href="http://www.engr.mun.ca/~theo/Misc/exp_parsing.htm">here</a>
      and in particular <a
      href="http://www.engr.mun.ca/~theo/Misc/pratt_parsing.htm">here</a>.
      
      The actual ideas of Top Down Operator Precedence parsing is
      explained well <a
      href="http://eli.thegreenplace.net/2010/01/02/top-down-operator-precedence-parsing">here</a>.

      The splitting of the string into tokens occurs in the ExprTokenizer::consume
      member function. Conversion of tokens into unary/prefix operators occurs
      in \ref ExprTokenizer::parseToken. Finally, the binary operator
      precedence and parsing takes place in \ref
      ExprTokenizer::parseExpression.
    */
    class ExprTokenizer {
    public:
      ExprTokenizer(const std::string& str):
	_str(str),
	_start(0),
	_end(0)
      {
	//Initialise the operators

	//These are left-associative operators, so we expect their RBP
	//to be BP+1, and they can group with themselves
	//BP, RBP, NBP
	_operators["+"].reset(new BinaryOpToken<detail::Add, 20, 21, 20>());
	_operators["-"].reset(new BinaryOpToken<detail::Subtract, 20, 21, 20>());
	_operators["*"].reset(new BinaryOpToken<detail::Multiply, 30, 31, 30>());
	_operators["/"].reset(new BinaryOpToken<detail::Divide, 30, 31, 30>());

	//Power is right-associative, thus its RBP is equal to its BP
	//to ensure a^b^c is a^(b^c).
	_operators["^"].reset(new BinaryOpToken<detail::Power, 50, 50, 49>());


	//The parenthesis operator is entirely handled by Parenthesis.
	_operators["("].reset(new ParenthesisToken);
	//A dummy token is needed here to allow the tokenizer to
	//identify this as a token, but the actual processing of ( is
	//handled in the Parenthesis operator above).
	_operators[")"].reset(new DummyToken);

	//Unary operators
	_operators["sin"].reset(new UnaryOpToken<detail::Sine, 0>());
	_operators["cos"].reset(new UnaryOpToken<detail::Cosine, 0>());
	_operators["exp"].reset(new UnaryOpToken<detail::Exp, 0>());
	_operators["log"].reset(new UnaryOpToken<detail::Log, 0>());
	
	//The actual tokenisation is done in the consume() member
	//function. The first call starts the process and clears the "end" state.
	consume();
      }

      std::string next() {
	return (_start == _str.size()) ? "" : _str.substr(_start, _end-_start);
      }

      void expect(std::string token) {
	if (next() != token)
	  stator_throw() << "Expected " << ((token.empty()) ? "end of expression": ("\""+token+"\"")) << " but got " << next() << " instead?\n" << parserLoc();
	consume();
      }
      
      void consume() {
	_start = _end;

	//Skip whitespace
	while ((_str[_start] == ' ') && (_start < _str.size())) ++_start;

	_end = _start;

	//Check for sequence end
	if (_start == _str.size())
	  return;

	//Not at end of sequence so at least one character in symbol
	_end = _start + 1;

	//Parse numbers with decimal points and (possible signed)
	//exponents. Signs at the front of numbers are parsed as unary
	//operators.
	if (std::isdigit(_str[_start])) {
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
	      } else {
		stator_throw() << "Malformed exponent?\n" << parserLoc();
	      }
	    }
	    if (std::isdigit(_str[_end])) {
	      ++_end;
	      continue;
	    }
	
	    break;
	  }
	  return;
	}

	//Parsing a string
	if (std::isalpha(_str[_start])) {
	  while ((_end < _str.size()) && std::isalpha(_str[_end]))
	    ++_end;
	  return;
	}

	//Allow non-alpha single character operators (longer operators should be above!)
	if (_operators.find(_str.substr(_start, 1)) != _operators.end())
	  return;
	
	stator_throw() << "Unrecognised token \"" << _str.substr(_start) << "\"\n" << parserLoc();
      }

      std::string parserLoc() {
	return _str + "\n"
	  + std::string(_start, ' ') + std::string(_end - _start -1, '-') + "^";
      }

      struct TokenBase {
	/*! \brief Takes one operand (either left or only operand) and returns the corresponding Expr.
	  
	  The token stream is also passed to allow binary operators to
	  collect their second (right) operand.
	 */
	virtual Expr apply(Expr, ExprTokenizer&) const = 0;
	/*! \brief Left binding power (Precedence of this operator)*/
	virtual int BP() const = 0;
	/*! \brief Next binding power (highest precedence of the operator that this operator can be a left operand of)*/
	virtual int NBP() const = 0;
      };

      struct PrefixOp: public TokenBase {
	int NBP() const { stator_throw() << "Should never be called!"; } 
      };

      struct BinaryOrPostfixOp: public TokenBase {
      };

      template<class Op, int tBP, int tRBP, int tNBP>
      struct BinaryOpToken : BinaryOrPostfixOp {
	Expr apply(Expr l, ExprTokenizer& tk) const {
	  return Op::apply(l, tk.parseExpression(tRBP));
	}

	int BP() const {
	  return tBP;
	}

	int NBP() const {
	  return tNBP;
	}
      };

      struct ParenthesisToken : public PrefixOp {
	Expr apply(Expr l, ExprTokenizer& tk) const {
	  tk.expect(")");
	  return l;
	}

	//Parenthesis bind the whole following expression
	int BP() const {
	  return 0;
	}
      };

      template<class Op, int tBP>
      struct UnaryOpToken : PrefixOp {
	Expr apply(Expr l, ExprTokenizer& tk) const {
	  return Op::apply(l);
	}

	int BP() const {
	  return tBP;
	}
      };
      
      struct DummyToken : TokenBase {
	virtual Expr apply(Expr, ExprTokenizer&) const { stator_throw() << "Should never be called!"; }
	//To ensure it is always treated as a separate expression, its BP is large
	virtual int BP() const { return -1; }
	virtual int NBP() const { stator_throw() << "Should never be called!"; }
      };
      
      std::map<std::string, shared_ptr<TokenBase> > _operators;

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

	//Parse unary operators which are prefixes
	auto it = _operators.find(token);
	if (it != _operators.end()) {
	  if (!dynamic_pointer_cast<PrefixOp>(it->second))
	    stator_throw() << "Operator is not a prefix/unary operator?\n" << parserLoc();
	  return it->second->apply(parseExpression(it->second->BP()), *this);
	}

	//Its not a prefix operator or a number, if it is a single
	//alpha character, then assume its a variable!
	if ((token.size() == 1) && (std::isalpha(token[0])))
	  return Expr(VarRT(token[0]));

	stator_throw() << "Could not parse as a valid token?\n" << parserLoc();
      }

      /*!\brief Main parsing entry function.
	
	\arg p The minimum binding power of an operator that this call
	can collect. A value of zero collects all operators.

	This handles all cases where operator precedence arises. Each
	operator has a binding power.
       */
      Expr parseExpression(int p = 0) {
	//Grab the first token (should be either a unary/prefix
	//operator or a number/variable all of which may be a LHS of a
	//multi arg operator which is handled here. Unary/prefix
	//operators are handled by parseToken()
	Expr t = parseToken();

	//Here we cache the next binding power
	int r = std::numeric_limits<int>::max();
	while (true) {
	  std::string token = next();

	  //Handle the special case of end of string
	  if (token.empty()) break;

	  //Determine the type of operator found
	  auto it = _operators.find(token);

	  //If there is no match, then return an error
	  if (it == _operators.end())
	    stator_throw() << "Expected binary or postfix operator but got \""<< token << "\"?\n" << parserLoc();

	  //If the operator is a prefix operator, then return an error
	  if (dynamic_pointer_cast<PrefixOp>(it->second))
	    stator_throw() << "Expected binary or postfix operator but found Prefix operator \""<< token << "\"?\n" << parserLoc();

	  //If the operator has a lower binding power than what this
	  //call can collect, then return. If an operator has already
	  //been collected, but its next binding power (NBP) doesn't
	  //allow it to collect this operator, then exit.
	  if ((p > it->second->BP())
	      || (it->second->BP() > r)) break;
	  
	  consume();
	  t = it->second->apply(t, *this);
	  r = it->second->NBP();
	}
	
	return t;
      }
      
    private:
    
      std::string _str;
      std::size_t _start;
      std::size_t _end;
    };
  }

  Expr::Expr(const std::string& str) : Expr(detail::ExprTokenizer(str).parseExpression()) {}
  Expr::Expr(const char* str) : Expr(std::string(str)) {}
}
