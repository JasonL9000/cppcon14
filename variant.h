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

namespace cppcon14 {
namespace variant {

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
template <typename elem_t>
struct for_elem {

  /* Just alignof. */
  static constexpr size_t safe_alignof = alignof(elem_t);

  /* Just sizeof. */
  static constexpr size_t safe_sizeof = sizeof(elem_t);

};  // for_elem<elem_t>

/* TODO */
template <typename elem_t>
constexpr size_t for_elem<elem_t>::safe_alignof;
template <typename elem_t>
constexpr size_t for_elem<elem_t>::safe_sizeof;

/* TODO */
template <>
struct for_elem<void> {

  /* Just zero. */
  static constexpr size_t safe_alignof = 0;

  /* Just zero. */
  static constexpr size_t safe_sizeof = 0;

};  // for_elem<void>

/* TODO */
constexpr size_t for_elem<void>::safe_alignof;
constexpr size_t for_elem<void>::safe_sizeof;

/* TODO */
template <typename... elems_t>
struct for_elems;

/* The base case, for no elements. */
template <>
struct for_elems<> {

  /* Just zero. */
  static constexpr size_t max_alignof = 0;

  /* Just zero. */
  static constexpr size_t max_sizeof = 0;

};  // for_elems<>

/* TODO */
constexpr size_t for_elems<>::max_alignof;
constexpr size_t for_elems<>::max_sizeof;

/* TODO */
template <typename val_t>
inline constexpr const val_t &max14(const val_t &lhs, const val_t &rhs) {
  return (lhs < rhs) ? rhs : lhs;
}

/* The recurring case, for one or more elements. */
template <typename elem_t, typename... more_elems_t>
struct for_elems<elem_t, more_elems_t...> {

  /* The for_elems<> specialization following this one. */
  using for_more_elems = for_elems<more_elems_t...>;

  /* The largest alignof(elem_t) among elems_t. */
  static constexpr size_t max_alignof
      = max14(for_elem<elem_t>::safe_alignof, for_more_elems::max_alignof);

  /* The largest sizeof(elem_t) among elems_t. */
  static constexpr size_t max_sizeof
      = max14(for_elem<elem_t>::safe_sizeof, for_more_elems::max_sizeof);

};  // for_elems<elem_t, more_elems_t...>

/* TODO */
template <typename elem_t, typename... more_elems_t>
constexpr size_t for_elems<elem_t, more_elems_t...>::max_alignof;
template <typename elem_t, typename... more_elems_t>
constexpr size_t for_elems<elem_t, more_elems_t...>::max_sizeof;

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
template <typename subj_t, typename... elems_t>
struct contains;

/* TODO */
template <typename subj_t>
struct contains<subj_t> {
  static constexpr bool value = false;
};

/* TODO */
template <typename subj_t>
constexpr bool contains<subj_t>::value;

/* TODO */
template <typename subj_t, typename elem_t, typename... more_elems_t>
struct contains<subj_t, elem_t, more_elems_t...> {
  static constexpr bool value =
      std::is_same<subj_t, elem_t>::value ||
      contains<subj_t, more_elems_t...>::value;
};

/* TODO */
template <typename subj_t, typename elem_t, typename... more_elems_t>
constexpr bool contains<subj_t, elem_t, more_elems_t...>::value;

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

/* TODO */
template <typename... elems_t>
class variant_t final {
  public:

  /* TODO */
  using visitor_t = variant::visitor_t<elems_t...>;

  /* Default-construct void. */
  variant_t() noexcept {
    tag = get_void_tag();
  }

  /* Move-construct, leaving the donor void. */
  variant_t(variant_t &&that) noexcept {
    (that.tag->move_construct)(data, that.data);
    tag = that.tag;
    that.tag = get_void_tag();
  }

  /* Copy-construct, leaving the exemplar intact. */
  variant_t(const variant_t &that) {
    (that.tag->copy_construct)(data, that.data);
    tag = that.tag;
  }

  /* Destroy. */
  ~variant_t() {
    assert(this);
    (tag->destroy)(data);
  }

  /* Move-assign, leaving the donor void. */
  variant_t &operator=(variant_t &&that) noexcept {
    assert(this);
    assert(&that);
    if (this != &that) {
      this->~variant_t();
      new (this) variant_t(std::move(that));
    }
    return *this;
  }

  /* Copy-assign, leaving the exemplar intact. */
  variant_t &operator=(const variant_t &that) {
    assert(this);
    assert(&that);
    return *this = variant_t(that);
  }

  /* True iff. we're not void. */
  operator bool() const noexcept {
    assert(this);
    return tag == get_void_tag();
  }

  void accept(const visitor_t &visitor) const {
    assert(this);
    (tag->accept)(data, visitor);
  }

  private:

  /* Look up constants defined by our set of possible types, then use those
     constants to define a type for our data storage space.  The space must
     be large enough to hold the largest of our types and aligned correctly
     for the most alignment-critical of our types. */
  using for_elems = variant::for_elems<elems_t...>;
  static constexpr size_t max_alignof = for_elems::max_alignof;
  static constexpr size_t max_sizeof = for_elems::max_sizeof;
  using data_t = alignas(max_alignof) char[max_sizeof];

  /* TODO */
  struct tag_t final {
    void (*destroy)(void *self) noexcept;
    void (*move_construct)(void *self, void *other) noexcept;
    void (*copy_construct)(void *self, const void *other);
    void (*accept)(const void *self, const visitor_t &visitor);
  };  // variant_t

  /* The tag we use iff. we're void. */
  static const tag_t *get_void_tag() noexcept {
    static const tag_t tag = {
      /* destroy */
      [](void *) {},
      /* move_construct */
      [](void *, void *) {},
      /* copy_construct */
      [](void *, const void *) {},
      /* accept */
      [](const void *, const visitor_t &visitor) {
        visitor();
      }
    };
    return &tag;
  }

  /* TODO.  Never null. */
  const tag_t *tag;

  /* The data to be interpreted by our tag. */
  data_t data;

};  // variant_t<elem_t>

}  // variant
}  // cppcon14
