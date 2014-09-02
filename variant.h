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

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace cppcon {
namespace variant {

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
template <typename val_t>
inline constexpr const val_t &max14(const val_t &lhs, const val_t &rhs) {
  return (lhs < rhs) ? rhs : lhs;
}

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
template <typename... elems_t>
struct for_elems;

/* The base case, for no elements. */
template <>
struct for_elems<> final {

  /* Just zero. */
  static constexpr size_t max_alignof = 0;

  /* Just zero. */
  static constexpr size_t max_sizeof = 0;

};  // for_elems<>

/* The recurring case, for one or more elements. */
template <typename elem_t, typename... more_elems_t>
struct for_elems<elem_t, more_elems_t...> final {

  /* The for_elems<> specialization following this one. */
  using for_more_elems = for_elems<more_elems_t...>;

  /* The largest alignof(elem_t) among elems_t. */
  static constexpr size_t max_alignof
      = max14(alignof(elem_t), for_more_elems::max_alignof);

  /* The largest sizeof(elem_t) among elems_t. */
  static constexpr size_t max_sizeof
      = max14(sizeof(elem_t), for_more_elems::max_sizeof);

};  // for_elems<elem_t, more_elems_t...>

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
template <typename elem_t, typename... elems_t>
struct contains;

/* TODO */
template <typename elem_t>
struct contains<elem_t> {
  static constexpr bool value = false;
};

/* TODO */
template <typename elem_t, typename other_t, typename... more_elems_t>
struct contains<elem_t, other_t, more_elems_t...> {
  static constexpr bool value =
      std::is_same<elem_t, other_t>::value ||
      contains<elem_t, more_elems_t...>::value;
};

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
template <typename... elems_t>
struct visitor_t;

/* TODO */
template <>
struct visitor_t<> {
  virtual void operator()() const = 0;
};

/* TODO */
template <typename elem_t, typename... more_elems_t>
struct visitor_t<elem_t, more_elems_t...> : visitor_t<more_elems_t...> {
  using visitor_t<more_elems_t...>::operator();
  virtual void operator()(const elem_t &) const = 0;
};

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

}  // variant
}  // cppcon
