/*
  Copyright (C) 2015 Severin Strobl <severin.strobl@fau.de>

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
#include <cstddef>

namespace stator {

  namespace orphan {

    /*! \brief A static list of variable type.
    */
    template<typename T, T...>
    struct static_list {
    };

    namespace detail {

      template<typename T, typename StaticList, T... List>
      struct static_list_helper;

      template<typename T, T... ListItems, T Head, T... Tail>
      struct static_list_helper<T, static_list<T, ListItems...>, Head,
        Tail...> {

        typedef typename static_list_helper<T, static_list<T, ListItems...,
          Head>, Tail...>::type type;
      };

      template<typename T, T... ListItems>
      struct static_list_helper<T, static_list<T, ListItems...>> {
        typedef static_list<T, ListItems...> type;
      };

      template<size_t Index, size_t ListIndex, typename List>
      struct get_static_list_item_helper {
        static const int value = -1;
      };

      template<size_t Index, typename T, size_t ListIndex, T Head, T... Tail>
      struct get_static_list_item_helper<Index, ListIndex, static_list<T, Head,
        Tail...>> {

        static const T value = (Index == ListIndex) ? Head :
          get_static_list_item_helper<Index, ListIndex + 1, static_list<T,
          Tail...>>::value;
      };

      template<typename T, typename ReverseList, typename StaticList>
      struct reverse_static_list_helper;

      template<typename T, T... ReverseItems, T Head, T... Tail>
      struct reverse_static_list_helper<T, static_list<T, ReverseItems...>,
        static_list<T, Head, Tail...>> {

        typedef typename reverse_static_list_helper<T, static_list<T, Head,
          ReverseItems...>, static_list<T, Tail...>>::type type;
      };

      template<typename T, T... ReverseItems>
      struct reverse_static_list_helper<T, static_list<T, ReverseItems...>,
        static_list<T>> {

        typedef static_list<T, ReverseItems...> type;
      };

      template<typename T, size_t Index, size_t ListIndex, typename NewList,
        typename OldList, T Value>
      struct replace_static_list_item_helper;

      template<typename T, size_t Index, size_t ListIndex, T... NewItems,
        T Head, T... Tail, T Value>
      struct replace_static_list_item_helper<T, Index, ListIndex,
        static_list<T, NewItems...>, static_list<T, Head, Tail...>, Value> {

        typedef typename replace_static_list_item_helper<T, Index,
          ListIndex + 1, static_list<T, NewItems..., Head>,
          static_list<T, Tail...>, Value>::type type;
      };

      template<typename T, size_t Index, T... NewItems, T Head, T... Tail,
        T Value>
      struct replace_static_list_item_helper<T, Index, Index, static_list<T,
        NewItems...>, static_list<T, Head, Tail...>, Value> {

        typedef static_list<T, NewItems..., Value, Tail...> type;
      };

      template<size_t Index, size_t ListIndex, typename FirstList, typename
        SecondList, typename List, bool Split>
      struct split_static_list_helper;

      template<size_t Index, size_t ListIndex, typename T, T... FirstItems,
        T Head, T... Tail>
      struct split_static_list_helper<Index, ListIndex, static_list<T,
        FirstItems...>, static_list<T>, static_list<T, Head,
        Tail...>, false> {

        typedef split_static_list_helper<Index, ListIndex + 1, static_list<T,
          FirstItems..., Head>, static_list<T>, static_list<T, Tail...>,
          ListIndex + 1 >= Index> next_type;

        typedef typename next_type::first first;
        typedef typename next_type::second second;
      };

      template<size_t Index, size_t ListIndex, typename T, T... FirstItems,
        T... SecondItems, T Head, T... Tail>
      struct split_static_list_helper<Index, ListIndex, static_list<T,
        FirstItems...>, static_list<T, SecondItems...>, static_list<T, Head,
        Tail...>, true> {

        typedef split_static_list_helper<Index, ListIndex + 1, static_list<T,
          FirstItems...>, static_list<T, SecondItems..., Head>, static_list<T,
          Tail...>, ListIndex + 1 >= Index> next_type;

        typedef typename next_type::first first;
        typedef typename next_type::second second;
      };

      template<size_t Index, size_t ListIndex, typename T, T... FirstItems,
        T... SecondItems, bool Split>
      struct split_static_list_helper<Index, ListIndex, static_list<T,
        FirstItems...>, static_list<T, SecondItems...>,
        static_list<T>, Split> {

        typedef static_list<T, FirstItems...> first;
        typedef static_list<T, SecondItems...> second;
      };

    } // namespace detail

    /*! \brief Convert a parameter pack into a static list.
    */
    template<typename T, T... ListItems>
    struct make_static_list {
      typedef typename detail::static_list_helper<T, static_list<T>,
        ListItems...>::type type;
    };

    /*! \brief Access an item in a static list by index.
    */
    template<typename List, size_t Index>
    struct get_static_list_item;

    template<typename T, T... ListItems, size_t Index>
    struct get_static_list_item<static_list<T, ListItems...>, Index> {
      static_assert(Index < sizeof...(ListItems), "Index out of range!");

      static const T value = detail::get_static_list_item_helper<Index, 0,
        static_list<T, ListItems...>>::value;
    };

    /*! \brief Reverse a static list.
    */
    template<typename List>
    struct reverse_static_list;

    template<typename T, T... ListItems>
    struct reverse_static_list<static_list<T, ListItems...>> {
      typedef typename detail::reverse_static_list_helper<T, static_list<T>,
        static_list<T, ListItems...>>::type type;
    };

    /*! \brief Replace a value at a certain position of a static list by a
        different one.
    */
    template<typename T, typename List, size_t Index, T Value>
    struct replace_static_list_item;

    template<typename T, size_t Index, T Value>
    struct replace_static_list_item<T, static_list<T>, Index, Value> {
      typedef static_list<T> type;
    };

    template<typename T, T... ListItems, size_t Index, T Value>
    struct replace_static_list_item<T, static_list<T, ListItems...>, Index,
      Value> {

      static_assert(Index < sizeof...(ListItems), "Index out of range!");

      typedef typename detail::replace_static_list_item_helper<T, Index, 0,
        static_list<T>, static_list<T, ListItems...>, Value>::type type;
    };

    /*! \brief Append an item to a static list.
    */
    template<typename T, typename List, T Value>
    struct append_static_list_item;

    template<typename T, T... ListItems, T Value>
    struct append_static_list_item<T, static_list<T, ListItems...>, Value> {
      typedef static_list<T, ListItems..., Value> type;
    };

    /*! \brief Prepend an item to a static list.
    */
    template<typename T, typename List, T Value>
    struct prepend_static_list_item;

    template<typename T, T... ListItems, T Value>
    struct prepend_static_list_item<T, static_list<T, ListItems...>, Value> {
      typedef static_list<T, Value, ListItems...> type;
    };

    /*! \brief Get the size of a static list.
    */
    template<typename List>
    struct static_list_size;

    template<typename T, T... ListItems>
    struct static_list_size<static_list<T, ListItems...>> {
      static const size_t value = sizeof...(ListItems);
    };

    /*! \brief Merge two static lists into one.
    */
    template<typename List0, typename List1>
    struct merge_static_lists;

    template<typename T, T... ListItems0, T... ListItems1>
    struct merge_static_lists<static_list<T, ListItems0...>, static_list<T,
      ListItems1...>> {

        typedef static_list<T, ListItems0..., ListItems1...> type;
    };

    /*! \brief Split a static list at a certain index.
    */
    template<typename List, size_t Index>
    struct split_static_list;

    template<typename T, size_t Index>
    struct split_static_list<static_list<T>, Index> {
      typedef static_list<T> first;
      typedef static_list<T> second;
    };

    template<typename T, T... ListItems, size_t Index>
    struct split_static_list<static_list<T, ListItems...>, Index> {
      typedef typename detail::split_static_list_helper<Index, 0,
        static_list<T>, static_list<T>, static_list<T, ListItems...>,
        Index == 0>::first first;

      typedef typename detail::split_static_list_helper<Index, 0,
        static_list<T>, static_list<T>, static_list<T, ListItems...>,
        Index == 0>::second second;
    };

    namespace detail {

      template<size_t Index, size_t End, typename List>
      struct static_foreach_helper;

      template<size_t Index, size_t End, typename T, T... ListItems>
      struct static_foreach_helper<Index, End, static_list<T, ListItems...>> {
        using List = static_list<T, ListItems...>;

        template<typename F, typename...Args>
        void operator()(F f, Args&&... args) {
          f(get_static_list_item<List, Index>::value, args...);
          static_foreach_helper<Index + 1, End, List>()(f, args...);
        }
      };

      template<size_t End, typename T, T... ListItems>
      struct static_foreach_helper<End, End, static_list<T, ListItems...>> {

        template<typename F, typename...Args>
        void operator()(F f, Args&&... args) {
        }
      };

    } // namespace detail

    /*! \brief Call functor for each element in a static list.
    */
    template<typename List>
    struct static_foreach;

    template<typename T, T... ListItems>
    struct static_foreach<static_list<T, ListItems...>> {
      using List = static_list<T, ListItems...>;

      template<typename F, typename...Args>
      void operator()(F f, Args&&... args) {
        detail::static_foreach_helper<0, static_list_size<List>::value,
          List>()(f, args...);
      }
    };

  } // namespace orphan

} // namespace stator
