/*
  Copyright (C) 2015 Marcus Bannerman <m.bannerman@gmail.com>
                2015 Severin Strobl <severin.strobl@fau.de>

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

// C++
#include <type_traits>

namespace stator {
  // The physical constants are based on the CODATA 2010 recommended values.
  // details: http://physics.nist.gov/cuu/index.html

  template<typename Scalar,
           typename = typename std::is_floating_point<Scalar>::type>
  struct constant {
    static inline Scalar k() {
      return Scalar(1.3806488e-23);
    }
    
    static inline Scalar pi() {
      return Scalar(3.1415926535897932384626433832795029L);
    }

    static inline Scalar avogadro() {
      return Scalar(6.02214129e23);
    }
  };
} // namespace stator
