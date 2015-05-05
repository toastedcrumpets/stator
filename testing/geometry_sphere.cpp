/*
  Copyright (C) 2015 Marcus Bannerman <m.bannerman@gmail.com>

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

//stator
#include <stator/geometry/sphere.hpp>

//boost
#define BOOST_TEST_MODULE Geometry_sphere_test
#include <boost/test/included/unit_test.hpp>

using namespace stator::geometry;

BOOST_AUTO_TEST_CASE( CenteredBall_test )
{
  BOOST_CHECK_EQUAL(measure(CenteredBall<double, 0>(1.0)), 1);
  BOOST_CHECK_EQUAL(measure(CenteredBall<double, 1>(1.0)), 2);
  BOOST_CHECK_CLOSE(measure(CenteredBall<double, 2>(1.0)), M_PI, 1e-11);
  BOOST_CHECK_CLOSE(measure(CenteredBall<double, 3>(2.0)), 4.0/3.0 * M_PI * 2 * 2 * 2, 1e-11);

  BOOST_CHECK_EQUAL(measure(CenteredSphere<double, 1>(1.0)), 2);
  BOOST_CHECK_CLOSE(measure(CenteredSphere<double, 2>(1.0)), 2 * M_PI, 1e-11);
  BOOST_CHECK_CLOSE(measure(CenteredSphere<double, 3>(2.0)), 4.0 * M_PI * 2 * 2, 1e-11);
}
