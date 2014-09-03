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
#include <tuple>
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

/* This is the same as the std::max() function in '14.  It differs from '11
   only in that it returns a constexpr.  When you upgrade to '14, you should
   drop this function in favor of the standard one. */
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
    (that.tag->move_construct)(*this, std::move(that));
    tag = that.tag;
    that.tag = get_void_tag();
  }

  /* Copy-construct, leaving the exemplar intact. */
  variant_t(const variant_t &that) {
    (that.tag->copy_construct)(*this, that);
    tag = that.tag;
  }

  /* Move-construct from a donor element, leaving the donor 'empty'. */
  template <
      typename elem_t,
      typename = typename std::enable_if<
          contains<elem_t, elems_t...>::value>::type>
  variant_t(elem_t &&elem) noexcept {
    assert(&elem);
    new (data) elem_t(std::move(elem));
    tag = get_tag<elem_t>();
  }

  /* Copy-construct from an exemplar element, leaving the exemplar intact. */
  template <
      typename elem_t,
      typename = typename std::enable_if<
          contains<elem_t, elems_t...>::value>::type>
  variant_t(const elem_t &elem) {
    assert(&elem);
    new (data) elem_t(elem);
    tag = get_tag<elem_t>();
  }

  /* Destroy. */
  ~variant_t() {
    assert(this);
    (tag->destroy)(*this);
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

  /* TODO */
  void accept(const visitor_t &visitor) const {
    assert(this);
    (tag->accept)(*this, visitor);
  }

  /* TODO */
  variant_t &reset() noexcept {
    assert(this);
    this->~variant_t();
    tag = get_void_tag();
    return *this;
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

    /* TODO */
    void (*move_construct)(variant_t &self, variant_t &&other) noexcept;

    /* TODO */
    void (*copy_construct)(variant_t &self, const variant_t &other);

    /* TODO */
    void (*destroy)(variant_t &) noexcept;

    /* TODO */
    void (*accept)(const variant_t &self, const visitor_t &visitor);

  };  // variant_t

  /* TODO */
  template <typename elem_t>
  elem_t &as() & noexcept {
    assert(this);
    return reinterpret_cast<elem_t &>(data);
  }

  /* TODO */
  template <typename elem_t>
  elem_t &&as() && noexcept {
    assert(this);
    return reinterpret_cast<elem_t &&>(data);
  }

  /* TODO */
  template <typename elem_t>
  const elem_t &as() const noexcept {
    assert(this);
    return reinterpret_cast<const elem_t &>(data);
  }

  /* TODO */
  template <typename elem_t>
  static const tag_t *get_tag() noexcept {
    static const tag_t tag = {
      // move_construct
      [](variant_t &self, variant_t &&other) {
        new (self.data) elem_t(std::move(other).as<elem_t>());
        other.reset();
      },
      // copy_construct
      [](variant_t &self, const variant_t &other) {
        new (self.data) elem_t(other.as<elem_t>());
      },
      // destroy
      [](variant_t &self) {
        self.as<elem_t>().~elem_t();
      },
      // accept
      [](const variant_t &self, const visitor_t &visitor) {
        visitor(self.as<elem_t>());
      }
    };
    return &tag;
  }

  /* The tag we use iff. we're void. */
  static const tag_t *get_void_tag() noexcept {
    static const tag_t tag = {
      // move_construct
      [](variant_t &, variant_t &&) {},
      // copy_construct
      [](variant_t &, const variant_t &) {},
      // destroy
      [](variant_t &) {},
      // accept
      [](const variant_t &, const visitor_t &visitor) {
        visitor();
      }
    };
    return &tag;
  }

  /* TODO.  Never null. */
  const tag_t *tag;

  /* The data to be interpreted by our tag. */
  data_t data;

};  // variant_t<elems_t...>

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

template <typename fn_t>
struct for_fn;

/* TODO */
template <typename ret_t_, typename... args_t>
struct for_fn<ret_t_ (args_t...)> {

  /* TODO */
  using ret_t = ret_t_;

  /* TODO */
  template <typename applied_t, typename visitor_t, typename... elems_t>
  class applier_t;

  /* TODO */
  template <typename applied_t, typename visitor_t>
  class applier_t<applied_t, visitor_t>
      : public visitor_t {
    public:

    /* TODO */
    applier_t(applied_t &applied, ret_t *ret, args_t &&... args)
        : applied(applied), ret(ret), tuple(std::forward_as_tuple(args)...) {
      assert(ret);
    }

    /* TODO */
    virtual void operator()() const override final {
      assert(this);
      apply(ret, std::index_sequence_for<args_t...>());
    }

    protected:

    /* TODO */
    applied_t &applied;

    /* TODO */
    ret_t *ret;

    /* TODO */
    const std::tuple<args_t &&...> tuple;

    private:

    /* TODO */
    template <typename ret_t, size_t... i>
    void apply(ret_t *ret, std::index_sequence<i...> &&) const {
      assert(this);
      *ret = applied(std::get<i>(tuple)...);
    }

    /* TODO */
    template <size_t... i>
    void apply(void *, std::index_sequence<i...> &&) const {
      assert(this);
      applied(std::get<i>(tuple)...);
    }

  };  // for_fn<ret_t (args_t...)>::applier_t<applied_t, visitor_t>

  /* TODO */
  template <
      typename applied_t, typename visitor_t,
      typename elem_t, typename... more_elems_t>
  class applier_t<applied_t, visitor_t, elem_t, more_elems_t...>
      : public applier_t<applied_t, visitor_t, more_elems_t...> {
    public:

    /* TODO */
    applier_t(applied_t &applied, ret_t *ret, args_t &&... args)
        : recur_t(applied, ret, std::forward<args_t>(args)...) {}

    /* TODO */
    virtual void operator()(const elem_t &elem) const override final {
      assert(this);
      apply(this->ret, elem, std::index_sequence_for<args_t...>());
    }

    private:

    /* TODO */
    using recur_t = applier_t<applied_t, visitor_t, more_elems_t...>;

    /* TODO */
    template <typename ret_t, size_t... i>
    void apply(
        ret_t *ret, const elem_t &elem, std::index_sequence<i...> &&) const {
      assert(this);
      *ret = (this->applied)(elem, std::get<i>(this->tuple)...);
    }

    /* TODO */
    template <size_t... i>
    void apply(
        void *, const elem_t &elem, std::index_sequence<i...> &&) const {
      assert(this);
      (this->applied)(elem, std::get<i>(this->tuple)...);
    }

  };  // for_fn<ret_t (args_t...)>::applier_t<applied_t, visitor_t, elem_t, more_elems_t...>

};  // for_fn<ret_t (args_t...)>

/* TODO */
template <typename applied_t, typename... elems_t, typename... more_args_t>
typename for_fn<typename applied_t::fn_t>::ret_t apply(
    applied_t &&applied, const variant_t<elems_t...> &arg,
    more_args_t &&... more_args) {
  using visitor_t = variant::visitor_t<elems_t...>;
  using for_fn = variant::for_fn<typename applied_t::fn_t>;
  using applier_t =
      typename for_fn::template applier_t<applied_t, visitor_t, elems_t...>;
  typename for_fn::ret_t ret;
  arg.accept(applier_t(
      applied, &ret, std::forward<more_args_t>(more_args)...));
  return std::move(ret);
}

}  // variant
}  // cppcon14
