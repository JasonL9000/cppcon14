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

using int_or_str_t = variant_t<int, string>;

static const string empty, hello("hello"), doctor("doctor");

FIXTURE(static_max) {
  EXPECT_EQ(static_max(101, 102), 102);
  EXPECT_EQ(static_max(102, 101), 102);
  EXPECT_EQ(static_max(101, 101), 101);
}

FIXTURE(for_elems) {
  EXPECT_EQ((get_max_alignof<>)(), 0u);
  EXPECT_EQ((get_max_sizeof<>)(), 0u);
  EXPECT_EQ((get_max_alignof<char>)(), alignof(char));
  EXPECT_EQ((get_max_sizeof<char>)(), sizeof(char));
  EXPECT_EQ((get_max_alignof<char, short>)(), alignof(short));
  EXPECT_EQ((get_max_sizeof<char, short>)(), sizeof(short));
  EXPECT_EQ((get_max_alignof<short, char>)(), alignof(short));
  EXPECT_EQ((get_max_sizeof<short, char>)(), sizeof(short));
  EXPECT_TRUE((contains<int, int>()));
  EXPECT_TRUE((contains<int, int, bool, char>()));
  EXPECT_TRUE((contains<bool, int, bool, char>()));
  EXPECT_TRUE((contains<char, int, bool, char>()));
  EXPECT_FALSE((contains<int>()));
  EXPECT_FALSE((contains<int, float>()));
  EXPECT_FALSE((contains<float, int, bool, char>()));
}

FIXTURE(try_as) {
  int_or_str_t a, b(101), c(hello);
  EXPECT_FALSE(a.try_as<int>());
  EXPECT_FALSE(a.try_as<string>());
  if (EXPECT_TRUE(b.try_as<int>())) {
    EXPECT_EQ(*b.try_as<int>(), 101);
  }
  EXPECT_FALSE(b.try_as<string>());
  if (EXPECT_TRUE(c.try_as<string>())) {
    EXPECT_EQ(*c.try_as<string>(), hello);
  }
  EXPECT_FALSE(c.try_as<int>());
}

/* TODO */
struct keeper_t final {

  /* TODO */
  using fn1_t = void ();

  /* TODO */
  void operator()(nullptr_t) {
    that_int = 0;
    that_str.clear();
  }

  /* TODO */
  void operator()(int that) {
    that_int = that;
    that_str.clear();
  }

  /* TODO */
  void operator()(const string &that) {
    that_int = 0;
    that_str = that;
  }

  /* TODO */
  int that_int = 0;

  /* TODO */
  string that_str;

};  // keeper_t

FIXTURE(keeper) {
  int_or_str_t a, b(101), c(hello);
  keeper_t keeper;
  keeper.that_int = 99;
  keeper.that_str = doctor;
  apply(keeper, a);
  EXPECT_EQ(keeper.that_int, 0);
  EXPECT_EQ(keeper.that_str, empty);
  keeper.that_int = 99;
  keeper.that_str = doctor;
  apply(keeper, b);
  EXPECT_EQ(keeper.that_int, 101);
  EXPECT_EQ(keeper.that_str, empty);
  keeper.that_int = 99;
  keeper.that_str = doctor;
  apply(keeper, c);
  EXPECT_EQ(keeper.that_int, 0);
  EXPECT_EQ(keeper.that_str, hello);
}

/* TODO */
struct greeter_t final {

  /* TODO */
  using fn1_t = string (const string &);

  /* TODO */
  string operator()(nullptr_t, const string &greeting) {
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
  int_or_str_t a, b(101), c(doctor);
  EXPECT_EQ(apply(greeter_t(), a, "Hey there"), "Hey there, nobody.");
  EXPECT_EQ(apply(greeter_t(), b, "What's up"), "What's up, '101'.");
  EXPECT_EQ(apply(greeter_t(), c, "Hello"), "Hello, 'doctor'.");
}

/* TODO */
struct infix_t final {

  /* TODO */
  using fn2_t = string ();

  /* TODO */
  template <typename lhs_t, typename rhs_t>
  string operator()(const lhs_t &lhs, const rhs_t &rhs) {
    ostringstream strm;
    write(strm, lhs);
    strm << ", ";
    write(strm, rhs);
    return strm.str();
  }

  template <typename elem_t>
  static void write(ostream &strm, const elem_t &elem) {
    assert(&strm);
    assert(&elem);
    strm << elem;
  }

  static void write(ostream &strm, const nullptr_t &) {
    assert(&strm);
    strm << "null";
  }

};  // infix_t

FIXTURE(infix) {
  int_or_str_t lhs(101), rhs(202);
  EXPECT_EQ(apply(infix_t(), lhs, rhs), "101, 202");
}
