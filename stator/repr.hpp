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

#include <limits>
#include <memory>
#include <cstdio>

namespace stator {
  /*! \brief A C++ version of snprintf allowing simple formatting of output.
    \in format A printf-style formatting string, such as "%.17f %s %g".
    \in args The arguments for substitution into the placeholders in the formatting string.
    \returns The final resultant string.
  */
  template<typename ... Args>
  std::string string_format( const std::string& format, Args ... args )
  {
    size_t size = std::snprintf(nullptr, 0, format.c_str(), args ... ) + 1; // +1 Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[ size ]); 
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
  }

  /*! \brief Search and replace elements in a std::string. 
    \param in The string to search within.
    \param from A string giving the text sequence to replace.
    \param to A string with the replacement text sequence.
    \returns The string "in" with all occurences of "from" replaced with "to".
   */
  inline std::string search_replace(std::string in, const std::string& from, const std::string& to)
  {
    if (!in.empty())
      {
	std::string::size_type toLen = to.length();
	std::string::size_type frLen = from.length();
	std::string::size_type loc = 0;
	
	while (std::string::npos != (loc = in.find(from, loc)))
	  {
	    in.replace(loc, frLen, to);
	    loc += toLen;
	    
	    if (loc >= in.length())
	      break;
	  }
      }
    return in;
  }

  
  inline std::string repr(int a) { return std::to_string(a); }
  
  inline std::string repr(long a) { return std::to_string(a); }
  
  inline std::string repr(unsigned a) { return std::to_string(a); }
  
  inline std::string repr(unsigned long a) { return std::to_string(a); }
  
  inline std::string repr(float a)
  { return stator::string_format("%.*g", std::numeric_limits<float>::max_digits10, a); }
  
  inline std::string repr(double a)
  { return stator::string_format("%.*g", std::numeric_limits<double>::max_digits10, a); }
}
