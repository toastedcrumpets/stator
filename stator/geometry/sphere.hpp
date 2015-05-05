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

#pragma once

#include "stator/constants.hpp"
#include "stator/geometry/object.hpp"

namespace stator {
  namespace geometry {
    /*! \brief An empty-class representation of a unit ball.
      
      A UnitBall represents the space contained within a UnitSphere.

      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the ball.
     */
    template<typename Scalar, size_t D> class UnitBall{};

    /*! \brief An empty-class representation of a unit sphere.
      
      A unit sphere is a sphere of radius 1.

      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the ball.
     */
    template<typename Scalar, size_t D> class UnitSphere{};
    
    /*! \brief An n-ball in the body-frame of reference.

      In three dimensions, a ball represents the volume of the
      interior of a sphere, but this class is generalised to other
      dimensions (e.g. a disc/filled-circle in 2D).

      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the ball.
    */
    template<typename Scalar, size_t D>
    class CenteredBall {
    public:
      /*! \brief Default constructor.
        
        This constructor deliberately leaves the object uninitialised
        to allow convenient stack-based construction without
        comprimising the detection of uninitialised accesses by tools
        such as valgrind.
      */
      CenteredBall() {}
      
      /*! \brief RAII constructor. */
      CenteredBall(Scalar radius): radius_(radius) {}
      
      /*! \brief Get function for the ball radius. */
      const Scalar& radius() const { return radius_; }
      
    protected:
      /*! \brief Radius of the ball. */
      Scalar radius_;
    };

    /*! \brief An n-sphere in the body-frame of reference.

      A sphere is a surface (not to be confused with a ball, which is
      the volume encased by a sphere).

      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the sphere.
    */
    template<typename Scalar, size_t D>
    class CenteredSphere {
    public:
      /*! \brief Default constructor.
        
        This constructor deliberately leaves the object uninitialised
        to allow convenient stack-based construction without
        comprimising the detection of uninitialised accesses by tools
        such as valgrind.
      */
      CenteredSphere() {}
      
      /*! \brief RAII constructor. */
      CenteredSphere(Scalar radius): radius_(radius) {}
      
      /*! \brief Get function for the sphere radius. */
      const Scalar& radius() const { return radius_; }
      
    protected:
      /*! \brief Radius of the sphere. */
      Scalar radius_;
    };
        
    template<typename Scalar>
    constexpr Scalar measure(const UnitBall<Scalar, 0>& ball) { return Scalar(1); }

    template<typename Scalar>
    constexpr Scalar measure(const UnitBall<Scalar, 1>& ball) { return Scalar(2); }
        
    template<typename Scalar, size_t D>
    constexpr typename std::enable_if<(D%2) && (D>1), Scalar>::type
      measure(const UnitBall<Scalar, D>& ball) {
      return 2 * std::tgamma((D-1)/2+1)
        * std::pow(4 * stator::constant<Scalar>::pi(), (D-1)/2)
        / std::tgamma(D+1);
    }

    template<typename Scalar, size_t D>
    constexpr typename std::enable_if<(!(D%2)) && (D>1), Scalar>::type 
      measure(const UnitBall<Scalar, D>& ball) {
      return std::pow(stator::constant<Scalar>::pi(), D/2) / std::tgamma(D/2+1);
    }
    
    template<typename Scalar, size_t D>
    Scalar measure(const UnitSphere<Scalar, D>& ball) {
      return D * measure(UnitBall<Scalar,D>());
    }


    template<typename Scalar, size_t D>
    Scalar measure(const CenteredBall<Scalar, D>& ball) {
      return measure(UnitBall<Scalar, D>()) * std::pow(ball.radius(), D);
    }
    
    template<typename Scalar, size_t D>
    Scalar measure(const CenteredSphere<Scalar, D>& sphere) {
      return measure(UnitSphere<Scalar, D>()) * std::pow(sphere.radius(), D-1);
    }
    
  } // namespace geometry
} // namespace stator

