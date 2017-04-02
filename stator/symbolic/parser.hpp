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

//#include <stator/symbolic/runtime.hpp>
#include <stator/exception.hpp>
#include <algorithm>
#include <cctype>

namespace sym {
  namespace detail {
    class ExprTokenizer {
    public:
      ExprTokenizer(std::string str):
	_str(str),
	_start(0),
	_end(0)
      {
	//The actual tokenisation is done in the consume() member
	//function. Start the process.
	consume();
      }

      bool empty() {
	return _start == _str.size();
      }
    
      std::string next() {
	return _str.substr(_start, _end-_start);
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

	//Parsing single character operators (other operators should be strings)
	switch (_str[_start]) {
	case '+':
	case '-':
	case '/':
	case '*':
	case '(':
	case ')':
	  return;
	}

	stator_throw() << "Unrecognised token \"" << _str.substr(_start) << "\"\n" << parserLoc();
      }

      std::string parserLoc() {
	return _str + "\n"
	  + std::string(_start, ' ') + std::string(_end - _start -1, '-') + "^";
      }

    private:
    
      std::string _str;
      std::size_t _start;
      std::size_t _end;
    };
  }
}
