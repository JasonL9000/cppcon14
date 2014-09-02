/* ---------------------------------------------------------------------------
Copyright 2014
  Jason Lucas (JasonL9000@gmail.com) and
  Michael Park (mcypark@gmail.com).

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
  HTTP://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
--------------------------------------------------------------------------- */

#include "variant.h"

#include <string>

#include "lick.h"

using namespace std;
using namespace cppcon14::variant;

FIXTURE(max14) {
  EXPECT_EQ(max14(101, 102), 102);
}

FIXTURE(for_elems) {
  using nothing_t = for_elems<>;
  EXPECT_EQ(nothing_t::max_alignof, 0);
  EXPECT_EQ(nothing_t::max_sizeof, 0);
  using just_void_t = for_elems<void>;
  EXPECT_EQ(just_void_t::max_alignof, 0);
  EXPECT_EQ(just_void_t::max_sizeof, 0);
  using just_char_t = for_elems<char>;
  EXPECT_EQ(just_char_t::max_alignof, alignof(char));
  EXPECT_EQ(just_char_t::max_sizeof, sizeof(char));
  using char_and_void_t = for_elems<char, void>;
  EXPECT_EQ(char_and_void_t::max_alignof, alignof(char));
  EXPECT_EQ(char_and_void_t::max_sizeof, sizeof(char));
  using char_and_short_t = for_elems<char, short>;
  EXPECT_EQ(char_and_short_t::max_alignof, alignof(short));
  EXPECT_EQ(char_and_short_t::max_sizeof, sizeof(short));
  struct foo_t { short a, b, c; };
  using char_and_struct_t = for_elems<char, foo_t>;
  EXPECT_EQ(char_and_struct_t::max_alignof, alignof(short));
  EXPECT_EQ(char_and_struct_t::max_sizeof, sizeof(foo_t));
}

FIXTURE(contains) {
  EXPECT_TRUE((contains<void, void>::value));
  EXPECT_TRUE((contains<void, void, bool, char>::value));
  EXPECT_TRUE((contains<bool, void, bool, char>::value));
  EXPECT_TRUE((contains<char, void, bool, char>::value));
  EXPECT_FALSE((contains<void>::value));
  EXPECT_FALSE((contains<void, int>::value));
  EXPECT_FALSE((contains<int, void, bool, char>::value));
}

FIXTURE(def_ctor) {
  variant_t<int, string> a;
}
