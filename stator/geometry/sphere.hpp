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

// stator
#include "stator/config.hpp"
#include "stator/constants.hpp"

namespace stator {
  namespace geometry {
    /*! \brief An n-ball (an n-sphere including its interior volume).

      In three dimensions, a ball represents the volume of the
      interior of a sphere, but this class is generalised to other
      dimensions (e.g. a disc/filled-circle in 2D).

      \tparam Scalar The scalar type used for computation of
      properties of the object.
      
      \tparam D The dimensionality of the ball.
    */
    template<typename Scalar, size_t D>
    class Ball {
    public:
      /*! \brief Default constructor.
        
        This constructor deliberately leaves the object uninitialised
        to allow convenient stack-based construction without
        comprimising the detection of uninitialised accesses by tools
        such as valgrind.
      */
      Ball() {}
      
      /*! \brief RAII constructor. */
      Ball(const Scalar& radius, const Vector<Scalar, D>& center = Vector<Scalar, D>::Zero().eval()): radius_(radius), center_(center) {}
      
      /*! \brief Get function for the ball radius. */
      const Scalar& radius() const { return radius_; }
      
      /*! \brief Get function for the ball center. */
      const Vector<Scalar, D>& center() const { return center_; }

    protected:
      /*! \brief Radius of the ball. */
      Scalar radius_;

      /*! \brief Center of the ball. */
      Vector<Scalar, D> center_;
    };

    /*! \brief An inverse n-ball (an n-sphere including its exterior volume).
      
      \see Ball
     */
    template<typename Scalar, size_t D>
    class InverseBall: public Ball<Scalar, D> {
    public:
      using Ball<Scalar, D>::Ball;
    };

    namespace detail {
      /*! \brief An empty-class representation of a unit ball.
      
	A UnitBall represents the space contained within a unit radius
	sphere.

	\tparam Scalar The scalar type used for computation of
	properties of the object.
      
	\tparam D The dimensionality of the ball.
      */
      template<typename Scalar, size_t D> class UnitBall{};
    }// namespace detail
        
    /*! \cond INTERNAL */
    template<typename Scalar>
    constexpr Scalar volume(const detail::UnitBall<Scalar, 0>& ball) { return Scalar(1); }

    template<typename Scalar>
    constexpr Scalar volume(const detail::UnitBall<Scalar, 1>& ball) { return Scalar(2); }
        
    template<typename Scalar, size_t D>
    constexpr typename std::enable_if<(D%2) && (D>1), Scalar>::type
      volume(const detail::UnitBall<Scalar, D>& ball) {
      return 2 * std::tgamma((D-1)/2+1)
        * std::pow(4 * stator::constant<Scalar>::pi(), (D-1)/2)
        / std::tgamma(D+1);
    }

    template<typename Scalar, size_t D>
    constexpr typename std::enable_if<(!(D%2)) && (D>1), Scalar>::type 
      volume(const detail::UnitBall<Scalar, D>& ball) {
      return std::pow(stator::constant<Scalar>::pi(), D/2) / std::tgamma(D/2+1);
    }
    
    template<typename Scalar, size_t D>
    Scalar area(const detail::UnitBall<Scalar, D>& ball) {
      return D * volume(detail::UnitBall<Scalar,D>());
    }
    /*! \endcond */
    
    /*! \brief Calculate the volume of a n-ball.*/
    template<typename Scalar, size_t D>
    Scalar volume(const Ball<Scalar, D>& ball) {
      return volume(detail::UnitBall<Scalar, D>()) * std::pow(ball.radius(), D);
    }
    
    /*! \brief Calculate the surface area of a n-ball.*/
    template<typename Scalar, size_t D>
    Scalar area(const Ball<Scalar, D>& sphere) {
      return area(detail::UnitBall<Scalar, D>()) * std::pow(sphere.radius(), D-1);
    }
  } // namespace geometry
} // namespace stator

