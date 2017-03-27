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


// stator
#include "stator/orphan/static_list.hpp"
#include "stator/exception.hpp"
#include <stator/unit_test.hpp>

// C++
#include <iostream>
#include <type_traits>

using namespace stator::orphan;

UNIT_TEST(StaticListTest) {
  typedef static_list<int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9> list;

  static_assert(static_list_size<list>::value == 10,
    "Failed to determine size of static_list<int>!");

  static_assert(get_static_list_item<list, 2>::value == 2,
    "Failed to access item in static_list<int> by index!");
 
  {
    typedef replace_static_list_item<int, list, 0, -1>::type list;
    static_assert(get_static_list_item<list, 0>::value == -1,
      "Failed to replace first item in static_list<int>!");
  }
 
  {
    typedef replace_static_list_item<int, list, 9, 0>::type list;
    static_assert(get_static_list_item<list, 9>::value == 0,
      "Failed to replace last item in static_list<int>!");
  }

  typedef split_static_list<list, 4>::first list_head;
  typedef split_static_list<list, 4>::second list_tail;

  static_assert(static_list_size<list_head>::value == 4 &&
    static_list_size<list_tail>::value == 6,
    "Failed to split static_list<int>!");

  static_assert(get_static_list_item<list_head, 3>::value == 3 &&
    get_static_list_item<list_tail, 0>::value == 4,
    "Failed to split static_list<int>!");

  typedef merge_static_lists<list_tail, list_head>::type merged_list;
  static_assert(static_list_size<merged_list>::value == 10,
    "Failed to merge two static_list<int>'s!");

  {
    typedef merge_static_lists<list_head, list_tail>::type merged_list;
    static_assert(std::is_same<merged_list, list>::value,
      "Failed to merge two static_list<int>'s!");
  }

  typedef make_static_list<char, 'a', 'b', 'c'>::type char_list;
  typedef reverse_static_list<char_list>::type rev_char_list;

  static_assert(get_static_list_item<rev_char_list, 2>::value == 'a',
    "Failed to reverse static_list<char>!");

  {
    typedef prepend_static_list_item<char, char_list, '0'>::type char_list;
    static_assert(get_static_list_item<char_list, 3>::value == 'c',
      "Failed to prepend item to static_list<char>!");
  }

  {
    typedef append_static_list_item<char, char_list, 'd'>::type char_list;
    static_assert(get_static_list_item<char_list, 3>::value == 'd',
      "Failed to append item to static_list<char>!");
  }

  {
    static_foreach<list>()([](const int& i, int mod) {
        if(i % mod)
          std::cout << "-> " << i << std::endl;
      }, 3);
  }
}
