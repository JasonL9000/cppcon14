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

Unit tests for and examples of variant_t.
--------------------------------------------------------------------------- */

#include "variant.h"

#include <string>

#include "lick.h"

using namespace std;
using namespace cppcon14::variant;

/* A union of int and string types. */
using int_or_str_t = variant_t<int, string>;

/* A few string constants we'll use in various text fixtures. */
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

/* A type we'll use as a test of unary functor application.  It can be applied
   to an instance of int_or_str_t and keeps a copy of whatever it finds there.
   You can examine the functor after the application to see what it found. */
struct keeper_t final {

  /* You must define a type called "fn1_t" in any structure you intend to
     use as a unary variant functor.  The type must be a function describing
     your overloads of operator().  You must give the return type as well
     as the types of any non-variant (that is, extra) parameters taken.  In
     this case, we return nothing and take nothing but a variant. */
  using fn1_t = void ();

  /* Handle the case when the variant is null. */
  void operator()(nullptr_t) {
    that_int = 0;
    that_str.clear();
  }

  /* Handle the case when the variant is an int. */
  void operator()(int that) {
    that_int = that;
    that_str.clear();
  }

  /* Handle the case when the variant is a string. */
  void operator()(const string &that) {
    that_int = 0;
    that_str = that;
  }

  /* The int we found, or 0 if we didn't find one. */
  int that_int = 0;

  /* The string we found, or empty if we didn't find one. */
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

/* Another test of unary functor application.  This one takes an extra
   argument and returns a string. */
struct greeter_t final {

  /* The mandatory function type definition.  Here we indicate that we'll be
     returning a string and expect a string to be passed to use after the
     variant. */
  using fn1_t = string (const string &);

  /* Handle the case when the variant is null. */
  string operator()(nullptr_t, const string &greeting) {
    ostringstream strm;
    strm << greeting << ", nobody.";
    return strm.str();
  }

  /* Handle the cases when the variant is non-null.  We can use a template
     here because we handle all the non-null cases the same way. */
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

/* A type we'll use as a test of binary functor application.  It can be
   applied to a pair of instances of int_or_str_t and returns a string
   describing that they are. */
struct pair_writer_t final {

  /* Just as the "fn1_t" type is mandatory for all unary functors, the "fn2_t"
     type is mandatory for all binary functors.  It is similar to fn1_t but
     assumes we will be passed a pair of variants instead of just one. */
  using fn2_t = string ();

  /* Handles all cases. */
  template <typename lhs_t, typename rhs_t>
  string operator()(const lhs_t &lhs, const rhs_t &rhs) {
    ostringstream strm;
    write(strm, lhs);
    strm << ", ";
    write(strm, rhs);
    return strm.str();
  }

  /* Writes a non-null value. */
  template <typename elem_t>
  static void write(ostream &strm, const elem_t &elem) {
    assert(&strm);
    assert(&elem);
    strm << elem;
  }

  /* Writes a null value. */
  static void write(ostream &strm, const nullptr_t &) {
    assert(&strm);
    strm << "null";
  }

};  // pair_writer_t

FIXTURE(pair_writer) {
  int_or_str_t nil, lhs(101), rhs(202);
  EXPECT_EQ(apply(pair_writer_t(), lhs, rhs), "101, 202");
  EXPECT_EQ(apply(pair_writer_t(), nil, rhs), "null, 202");
  EXPECT_EQ(apply(pair_writer_t(), lhs, nil), "101, null");
  EXPECT_EQ(apply(pair_writer_t(), nil, nil), "null, null");
}
