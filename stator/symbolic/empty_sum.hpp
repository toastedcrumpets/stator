#pragma once

namespace sym {
  /*! \brief Returns the empty sum (i.e. equivalent of zero) for arithmetic types (int=0, double=0.0, float=0.0f, etc).
    
    The empty sum is a term whose additive (and typically its
    subtractive) action is null (can be ignored). This definition
    only applies for selected types and assumes that their default
    constructors create the empty sum.red
  */
  template<class T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
  T empty_sum(const T&) { return T(); }

  /*! \brief Returns the empty sum (i.e. equivalent of zero) for Eigen
      vectors/matrices.
  */
  template<class T, typename = typename std::enable_if<std::is_base_of<Eigen::EigenBase<T>, T>::value>::type>
  auto empty_sum(const T&) { return store(T::Zero()); }

  /*! \brief Returns the empty sum (i.e. equivalent of zero) for
      complex types.
  */
  template<class T>
  T empty_sum(const std::complex<T>&) { return T(); }
}
