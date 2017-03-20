/*
  Copyright (C) 2015 Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#ifdef STATOR_BOOST_SHARED
# include <boost/shared_ptr.hpp>
# include <boost/make_shared.hpp>
namespace sym {
  using boost::shared_ptr;
  using boost::make_shared;
  using boost::dynamic_pointer_cast;
}
#else
# include <memory>
namespace sym {
  using std::shared_ptr;
  using std::make_shared;
  using std::dynamic_pointer_cast;
}
#endif

#include <stator/symbolic/symbolic.hpp>

namespace sym {

  class RTInterface;
  
  typedef shared_ptr<RTInterface> Expr;
  
  class RTInterface {
  public:
    virtual ~RTInterface() {}

    virtual Expr clone() const = 0;

    virtual Expr sub(const Expr var, const Expr repl) const = 0;

    virtual bool operator==(const Expr o) const = 0;
    
    virtual void unwrap() const = 0;
  };

//  template<class Var, class Arg>
//  shared_ptr<RTExpression> sub(const shared_ptr<RTExpression>& f,
//			       const VarSub<Var, Arg>&) {
//    return f->sub()
//  };
  
}
