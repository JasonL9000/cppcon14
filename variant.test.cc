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
  EXPECT_EQ(nothing_t::max_alignof, 0u);
  EXPECT_EQ(nothing_t::max_sizeof, 0u);
  using just_void_t = for_elems<void>;
  EXPECT_EQ(just_void_t::max_alignof, 0u);
  EXPECT_EQ(just_void_t::max_sizeof, 0u);
  using just_char_t = for_elems<char>;
  EXPECT_EQ(just_char_t::max_alignof, alignof(char));
  EXPECT_EQ(just_char_t::max_sizeof, sizeof(char));
  using char_and_void_t = for_elems<char, void>;
  EXPECT_EQ(char_and_void_t::max_alignof, alignof(char));
  EXPECT_EQ(char_and_void_t::max_sizeof, sizeof(char));
  using char_and_short_t = for_elems<char, short>;
  EXPECT_EQ(char_and_short_t::max_alignof, alignof(short));
  EXPECT_EQ(char_and_short_t::max_sizeof, sizeof(short));
  struct int_or_str_t { short a, b, c; };
  using char_and_struct_t = for_elems<char, int_or_str_t>;
  EXPECT_EQ(char_and_struct_t::max_alignof, alignof(short));
  EXPECT_EQ(char_and_struct_t::max_sizeof, sizeof(int_or_str_t));
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

using int_or_str_t = variant_t<int, string>;

struct keeper_t final {

  using fn_t = void ();

  void operator()() {
    that_int = 0;
    that_str = "";
  }

  void operator()(int that) {
    that_int = that;
    that_str = "";
  }

  void operator()(const string &that) {
    that_int = 0;
    that_str = that;
  }

  int that_int = 0;

  string that_str;

};  // keeper_t

FIXTURE(keeper) {
  int_or_str_t
      a;
  keeper_t keeper;
  keeper.that_int = 99;
  keeper.that_str = "goner";
  apply(keeper, a);
  EXPECT_EQ(keeper.that_int, 0);
  EXPECT_EQ(keeper.that_str, "");
  #if 0
  int_or_str_t
      a,
      b(101),
      c(string("yesterday"));
  keeper_t keeper;
  EXPECT_EQ(keeper.that_int, 0);
  EXPECT_EQ(keeper.that_str, "");
  keeper.that_int = 99;
  keeper.that_str = "goner";
  apply(keeper, a);
  EXPECT_EQ(keeper.that_int, 0);
  EXPECT_EQ(keeper.that_str, "");
  apply(keeper, b);
  EXPECT_EQ(keeper.that_int, 101);
  EXPECT_EQ(keeper.that_str, "");
  apply(keeper, b);
  EXPECT_EQ(keeper.that_int, 0);
  EXPECT_EQ(keeper.that_str, "yesterday");
  #endif
}

/* TODO */
struct greeter_t final {

  /* TODO */
  using fn_t = string (const string &);

  /* TODO */
  string operator()(const string &greeting) {
    ostringstream strm;
    strm << greeting << ", nobody.";
    return strm.str();
  }

  /* TODO */
  template <typename elem_t>
  string operator()(const elem_t &elem, const string &greeting) {
    ostringstream strm;
    strm << greeting << ", '" << elem << "'.";
    return strm.str();
  }

};  // greeter_t

FIXTURE(greeter) {
  int_or_str_t
      a,
      b(101),
      c(string("doctor"));
  EXPECT_EQ(apply(greeter_t(), a, "Hey there"), "Hey there, nobody.");
  EXPECT_EQ(apply(greeter_t(), b, "What's up"), "What's up, '101'.");
  EXPECT_EQ(apply(greeter_t(), c, "Hello"), "Hello, 'doctor'.");
}
