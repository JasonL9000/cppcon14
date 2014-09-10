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

TODO
--------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <sstream>
#include <std::string>

#include "variant.h"

namespace cppcon14 {
namespace calc {

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* Forward-declaration of the expression type, for the benefit of lambda_t. */
struct expr_t;
using expr_ptr_t = std::shared_ptr<expr_t>;

/* TODO */
struct lambda_t final {

  /* TODO */
  using params_t = std::vector<std::string>;

  /* TODO */
  lambda_t(params_t &&params, const expr_ptr_t &def)
      : params(move(params)), def(move(def)) {}

  /* TODO */
  params_t params;

  /* TODO */
  expr_ptr_t def;

};  // lambda_t

/* TODO */
struct val_t final
    : variant_t<int, std::string, lambda_t> {
  using variant_t::variant_t;
};

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
struct type_mismatch_error_t final
    : std::runtime_error {

  /* TODO */
  type_mismatch_error_t()
      : runtime_error("type mismatch error") {}

};  // type_mismatch_error_t

/* TODO */
struct null_value_error_t final
    : std::runtime_error {

  /* TODO */
  null_value_error_t()
      : runtime_error("null value error") {}

};  // null_value_error_t

/* TODO */
struct unary_val_functor_t {

  /* TODO */
  using fn1_t = val_t ();

  /* TODO */
  val_t operator()(nullptr_t) const {
    throw null_value_error_t();
  }

  /* TODO */
  template <typename that_t>
  val_t operator()(const that_t &) const {
    throw type_mismatch_error_t();
  }

};  // unary_val_functor_t

/* TODO */
struct binary_val_functor_t {

  /* TODO */
  using fn2_t = val_t ();

  /* TODO */
  val_t operator()(nullptr_t, nullptr_t) const {
    throw null_value_error_t();
  }

  /* TODO */
  template <typename lhs_t>
  val_t operator()(const lhs_t &, nullptr_t) const {
    throw null_value_error_t();
  }

  /* TODO */
  template <typename rhs_t>
  val_t operator()(nullptr_t, const rhs_t &) const {
    throw null_value_error_t();
  }

  /* TODO */
  template <typename lhs_t, typename rhs_t>
  val_t operator()(const lhs_t &, const rhs_t &) const {
    throw type_mismatch_error_t();
  }

};  // binary_val_functor_t

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
struct neg_t final
    : unary_val_functor_t {
  using unary_val_functor_t::operator();
  val_t operator()(int that) const {
    return -that;
  }
};

/* TODO */
struct not_t final
    : unary_val_functor_t {
  using unary_val_functor_t::operator();
  val_t operator()(int that) const {
    return static_cast<int>(!that);
  }
};

/* TODO */
struct to_int_t final
    : unary_val_functor_t {
  using unary_val_functor_t::operator();
  val_t operator()(int that) const {
    return that;
  }
  val_t operator()(const std::string &that) const {
    return stoi(that);
  }
};

/* TODO */
struct to_str_t final
    : unary_val_functor_t {
  using unary_val_functor_t::operator();
  val_t operator()(int that) const {
    std::ostd::stringstream strm;
    strm << that;
    return strm.str();
  }
  val_t operator()(const std::string &that) const {
    return that;
  }
};

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
struct add_t final
    : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const {
    return lhs + rhs;
  }
  val_t operator()(const std::string &lhs, const std::string &rhs) const {
    return lhs + rhs;
  }
};

/* TODO */
struct mul_t final
    : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const {
    return lhs * rhs;
  }
  val_t operator()(const std::string &lhs, int rhs) const {
    std::ostd::stringstream strm;
    for (int i = 0; i < rhs; ++i) {
      strm << lhs;
    }
    return strm.str();
  }
};

/* TODO */
struct and_t final
    : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const {
    return static_cast<int>(lhs && rhs);
  }
};

/* TODO */
struct or_t final
    : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const {
    return static_cast<int>(lhs || rhs);
  }
};

/* TODO */
struct lt_t final
    : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const {
    return static_cast<int>(lhs < rhs);
  }
  val_t operator()(const std::string &lhs, const std::string &rhs) const {
    return static_cast<int>(lhs < rhs);
  }
};

}  // calc
}  // cppcon14
