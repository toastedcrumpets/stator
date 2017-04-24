//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Copyright (c) 2016 Marcus Bannerman
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <type_traits>

namespace stator {
  namespace orphan {
    template<typename ID, typename T, T value>
    struct value_conf_t : std::integral_constant<T, value> {
      using type_id = ID;
    };

    template<typename ID>
    struct basic_conf_t {
      using type_id = ID;
    };

    template<typename ID, typename T>
    struct type_conf_t {
      using type_id = ID;
      using value = T;
    };

    template<typename ID, template<typename> class T>
    struct template_type_conf_t {
      using type_id = ID;
    
      template<typename C>
      using value = T<C>;
    };


    template<typename D, typename... Args>
    struct get_value;

    template<typename D, typename T2, typename... Args>
    struct get_value<D, T2, Args...> {
      template<typename D2, typename T22, typename Enable = void>
      struct impl 
	: std::integral_constant<decltype(D::value), get_value<D, Args...>::value> {};

      template<typename D2, typename T22>
      struct impl <D2, T22, typename std::enable_if<std::is_same<typename D2::type_id, typename T22::type_id>::value>::type> 
	: std::integral_constant<decltype(D::value), T22::value> {};

      static constexpr const auto value = impl<D, T2>::value;
    };

    template<typename D>
    struct get_value<D> : std::integral_constant<decltype(D::value), D::value> {};


    template<typename D, typename... Args>
    struct get_type;

    template<typename D, typename T2, typename... Args>
    struct get_type<D, T2, Args...> {
      template<typename D2, typename T22, typename Enable = void>
      struct impl {
	using value = typename get_type<D, Args...>::value;
      };

      template<typename D2, typename T22>
      struct impl <D2, T22, typename std::enable_if<std::is_same<typename D2::type_id, typename T22::type_id>::value>::type> {
	using value = typename T22::value;
      };

      using value = typename impl<D, T2>::value;
    };

    template<typename D>
    struct get_type<D> {
      using value = typename D::value;
    };

    template<typename D, typename... Args>
    struct get_template_type;

    template<typename D, typename T2, typename... Args>
    struct get_template_type<D, T2, Args...> {
      template<typename D2, typename T22, typename Enable = void>
      struct impl {
	template<typename C>
	using value = typename get_template_type<D, Args...>::template value<C>;
      };

      template<typename D2, typename T22>
      struct impl <D2, T22, typename std::enable_if<std::is_same<typename D2::type_id, typename T22::type_id>::value>::type> {
	template<typename C>
	  using value = typename T22::template value<C>;
      };

      template<typename C>
      using value = typename impl<D, T2>::template value<C>;
    };

    template<typename D>
    struct get_template_type<D> {
      template<typename C>
      using value = typename D::template value<C>;
    };

    template<typename T1, typename... Args>
    struct is_present;

    template<typename T1, typename T2, typename... Args>
    struct is_present<T1, T2, Args...> : std::integral_constant<bool, std::is_same<T1, T2>::value || is_present<T1, Args...>::value> {};

    template<typename T1>
    struct is_present<T1> : std::false_type {};


    template<typename... Valid>
    struct tmp_list {
      template<typename T>
      struct contains : std::integral_constant<bool, is_present<typename T::type_id, Valid...>::value> {};
    };

    template<typename L, typename... Args>
    struct is_valid;

    template<typename L, typename T1, typename... Args>
    struct is_valid <L, T1, Args...> : std::integral_constant<bool, L::template contains<T1>::value && is_valid<L, Args...>::value> {};

    template<typename L>
    struct is_valid <L> : std::true_type {};
  }
}
