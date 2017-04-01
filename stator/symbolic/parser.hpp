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

namespace sym {
  namespace detail {
    class ExprTokenizer {
    public:
      ExprTokenizer(std::string str):
	_str(str),
	_start(0),
	_end(0)
      {
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
      
	if (std::isdigit(_str[_start])) {
	  grabNumber();
	  return;
	}

	if (std::isalpha(_str[_start])) {
	  while ((_end < _str.size()) && std::isalpha(_str[_end]))
	    ++_end;
	  return;
	}

	//Everything else should be a single character operator

	switch (_str[_start]) {
	case '+':
	case '-':
	case '/':
	case '*':
	case '(':
	case ')':
	  return;
	}

	stator_throw() << "Unrecognised token \"" << _str.substr(_start) << "\"";
      }

    private:

      void grabNumber() {
	bool decimal = false;
	bool exponent = false;
      
	while (_end != _str.size()) {
	  if (_str[_end] == '.') {
	    if (!decimal && !exponent) {
	      decimal = true;
	      ++_end;
	      continue;
	    } else {
	      stator_throw() << "Unexpected decimal point\n" << errortxt();
	    }
	  }
	
	  if ((_str[_end] == 'e') || (_str[_end] == 'E')) {
	    if (!exponent) {
	      exponent = true;
	      ++_end;
	      continue;
	    } else {
	      stator_throw() << "Unexpected exponent\n" << errortxt();
	    }
	  }
	  if (std::isdigit(_str[_end])) {
	    ++_end;
	    continue;
	  }
	
	  break;
	}
      }

      std::string errortxt() {
	return _str.substr(0, _end) + "\n" + std::string(_end, ' ') + _str.substr(_end);
      }
    
      std::string _str;
      std::size_t _start;
      std::size_t _end;
    };
  }
}
