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

separate visitor into accessor and mutator
add construction between unions of intersecting sets, with bad_cast

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
Compile-time utilities used internally by variant_t.
--------------------------------------------------------------------------- */

/* This is the same as the std::max() function except that it works as a
   compile-time expression. */
template <typename val_t>
inline constexpr const val_t &static_max(
    const val_t &lhs, const val_t &rhs) {
  return (lhs < rhs) ? rhs : lhs;
}

/* A place in which to look up static properties about a set of elements.
   We'll use this to determine the size and alignment required for a variant,
   among other things. */
template <typename... elems_t>
struct for_elems;

/* The base case, for no elements. */
template <>
struct for_elems<> {

  /* Just false. */
  template <typename subj_t>
  static constexpr bool contains() noexcept {
    return false;
  }

  /* Just zero. */
  static constexpr size_t get_max_alignof() noexcept {
    return 0;
  }

  /* Just zero. */
  static constexpr size_t get_max_sizeof() noexcept {
    return 0;
  }

};  // for_elems<>

/* The recurring case, for one or more elements. */
template <typename elem_t, typename... more_elems_t>
struct for_elems<elem_t, more_elems_t...> {

  /* The for_elems<> specialization following this one. */
  using for_more_elems = for_elems<more_elems_t...>;

  /* True iff. the set of elements contains the subject. */
  template <typename subj_t>
  static constexpr bool contains() noexcept {
    return std::is_same<subj_t, elem_t>() ||
        for_more_elems::template contains<subj_t>();
  }

  /* The largest alignof(elem_t) among elems_t. */
  static constexpr size_t get_max_alignof() noexcept {
    return static_max(alignof(elem_t), for_more_elems::get_max_alignof());
  }

  /* The largest sizeof(elem_t) among elems_t. */
  static constexpr size_t get_max_sizeof() noexcept {
    return static_max(sizeof(elem_t), for_more_elems::get_max_sizeof());
  }

};  // for_elems<elem_t, more_elems_t...>

/* True iff. the set of elements contains the subject. */
template <typename subj_t, typename... elems_t>
inline constexpr bool contains() noexcept {
  return for_elems<elems_t...>::template contains<subj_t>();
}

/* The largest alignof(elem_t) among elems_t. */
template <typename... elems_t>
inline constexpr size_t get_max_alignof() noexcept {
  return for_elems<elems_t...>::get_max_alignof();
}

/* The largest sizeof(elem_t) among elems_t. */
template <typename... elems_t>
inline constexpr size_t get_max_sizeof() noexcept {
  return for_elems<elems_t...>::get_max_sizeof();
}

/* ---------------------------------------------------------------------------
The base class for all visitors to variants.
--------------------------------------------------------------------------- */

/* TODO */
template <typename... elems_t>
struct visitor_t;

/* TODO */
template <>
struct visitor_t<> {

  /* TODO */
  virtual void operator()() const = 0;

};  // visitor_t<>

/* TODO */
template <typename elem_t, typename... more_elems_t>
struct visitor_t<elem_t, more_elems_t...>
    : visitor_t<more_elems_t...> {

  /* TODO */
  using visitor_t<more_elems_t...>::operator();

  /* TODO */
  virtual void operator()(const elem_t &) const = 0;

};  // visitor_t<elem_t, more_elems_t...>

/* ---------------------------------------------------------------------------
The variant class template itself.
--------------------------------------------------------------------------- */

/* TODO */
template <typename... elems_t>
class variant_t final {
  public:

  /* TODO */
  using visitor_t = variant::visitor_t<elems_t...>;

  /* True iff. elem_t is among our elems_t; that is, if this variant can
     ever contain a value of type elem_t.  We use this in a few places as a
     predicate to std::enable_if<>. */
  template <typename elem_t>
  static constexpr bool contains() noexcept {
    return variant::contains<elem_t, elems_t...>();
  }

  /* Default-construct null. */
  variant_t() noexcept {
    tag = get_null_tag();
  }

  /* Move-construct, leaving the donor null. */
  variant_t(variant_t &&that) noexcept {
    (that.tag->move_construct)(*this, std::move(that));
    tag = that.tag;
    that.tag = get_null_tag();
  }

  /* Copy-construct, leaving the exemplar intact. */
  variant_t(const variant_t &that) {
    (that.tag->copy_construct)(*this, that);
    tag = that.tag;
  }

  /* Move-construct from a donor element, leaving the donor 'empty'.  If we
     cannot assume the requested type (that is, if elem_t is not among our
     elems_t), this constructor is disabled. */
  template <
      typename elem_t, typename = std::enable_if_t<contains<elem_t>()>>
  variant_t(elem_t &&elem) noexcept {
    assert(&elem);
    new (data) elem_t(std::move(elem));
    tag = get_tag<elem_t>();
  }

  /* Copy-construct from an exemplar element, leaving the exemplar intact.
     If we cannot assume the requested type (that is, if elem_t is not among
     our elems_t), this constructor is disabled. */
  template <
      typename elem_t, typename = std::enable_if_t<contains<elem_t>()>>
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

  /* Move-assign, leaving the donor null. */
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

  /* True iff. we're not null. */
  operator bool() const noexcept {
    assert(this);
    return tag == get_null_tag();
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
    tag = get_null_tag();
    return *this;
  }

  /* Access our state as a particular type.  If we're not currently of the
     requested type, return null.  If we can never be of the requested type
     (that is, elem_t is not among our elems_t), this function is disabled. */
  template <
      typename elem_t, typename = std::enable_if_t<contains<elem_t>()>>
  const elem_t *try_as() const {
    ptr_getter_t<elem_t> try_as_helper;
    apply(try_as_helper, *this);
    return try_as_helper.ptr;
  }

  private:

  /* Look up constants defined by our set of possible types, then use those
     constants to define a type for our data storage space.  The space must
     be large enough to hold the largest of our types and aligned correctly
     for the most alignment-critical of our types. */
  using data_t =
      alignas(get_max_alignof<elems_t...>())
      char[get_max_sizeof<elems_t...>()];

  /* TODO */
  template <typename elem_t>
  struct ptr_getter_t final {

    /* TODO */
    using fn_t = void ();

    /* TODO */
    void operator()() {}

    /* TODO */
    template <typename other_t>
    void operator()(const other_t &) {}

    /* TODO */
    void operator()(const elem_t &val) {
      ptr = &val;
    }

    /* TODO */
    const elem_t *ptr = nullptr;

  };  // ptr_getter_t<elem_t>

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
  elem_t &force_as() & noexcept {
    assert(this);
    return reinterpret_cast<elem_t &>(data);
  }

  /* TODO */
  template <typename elem_t>
  elem_t &&force_as() && noexcept {
    assert(this);
    return reinterpret_cast<elem_t &&>(data);
  }

  /* TODO */
  template <typename elem_t>
  const elem_t &force_as() const noexcept {
    assert(this);
    return reinterpret_cast<const elem_t &>(data);
  }

  /* TODO */
  template <typename elem_t>
  static const tag_t *get_tag() noexcept {
    static const tag_t tag = {
      // move_construct
      [](variant_t &self, variant_t &&other) {
        new (self.data) elem_t(std::move(other).force_as<elem_t>());
        other.reset();
      },
      // copy_construct
      [](variant_t &self, const variant_t &other) {
        new (self.data) elem_t(other.force_as<elem_t>());
      },
      // destroy
      [](variant_t &self) {
        self.force_as<elem_t>().~elem_t();
      },
      // accept
      [](const variant_t &self, const visitor_t &visitor) {
        visitor(self.force_as<elem_t>());
      }
    };
    return &tag;
  }

  /* The tag we use iff. we're null. */
  static const tag_t *get_null_tag() noexcept {
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
Unary functor application.
--------------------------------------------------------------------------- */

/* TODO */
template <typename fn_t>
struct for_fn;

/* Explicit specialization for a function which returns non-void. */
template <typename ret_t, typename... params_t>
struct for_fn<ret_t (params_t...)> {

  /* This will be our visitor, recursively defined.  It functions as an
     adapter, passing control from the virtual functions of the visitor
     to the appropriate overloads of operator() in the functor functor. */
  template <typename functor_t, typename visitor_t, typename... elems_t>
  class applier_t;

  /* The base case inherits from the visitor for the variant and caches
     references to the functor and to the storage location for the value
     to be returned by the functor, as well as the extra (non-
     variant) arguments to be passed to the functor.  This class also
     defines the nullary operator() override. */
  template <typename functor_t, typename visitor_t>
  class applier_t<functor_t, visitor_t>
      : public visitor_t {
    public:

    /* Cache stuff for later, when operator() gets dispatched. */
    applier_t(functor_t &functor, ret_t &ret, params_t &&... params)
        : functor(functor), ret(ret),
          tuple(std::forward_as_tuple(params)...) {}

    /* Override of the definition in visitor_t.  This is the nullary version,
       used when the variant is null.  It is just a hand-ff to apply_tuple(),
       below. */
    virtual void operator()() const override final {
      assert(this);
      apply_tuple(std::index_sequence_for<params_t...>());
    }

    protected:

    /* The functor we will apply. */
    functor_t &functor;

    /* The location in which to cache the result of the functor. */
    ret_t &ret;

    /* The extra arguments (beyond the variant) to pass to the functor. */
    const std::tuple<params_t &&...> tuple;

    private:

    /* Unrolls the cached tuple of extra arguments into an application of the
       functor and caches the valued returned. */
    template <size_t... i>
    void apply_tuple(std::index_sequence<i...> &&) const {
      assert(this);
      ret = functor(std::get<i>(tuple)...);
    }

  };  // for_fn<ret_t (params_t...)>::applier_t<functor_t, visitor_t>

  /* Each iteration of the recursive case defines a unary overload of
     operator(). */
  template <
      typename functor_t, typename visitor_t,
      typename elem_t, typename... more_elems_t>
  class applier_t<functor_t, visitor_t, elem_t, more_elems_t...>
      : public applier_t<functor_t, visitor_t, more_elems_t...> {
    public:

    /* Pass everything up to the base case. */
    applier_t(functor_t &functor, ret_t &ret, params_t &&... params)
        : recur_t(functor, ret, std::forward<params_t>(params)...) {}

    /* Override of the definition in visitor_t.  This is the unary version,
       used when the variant is non-null.  It is just a hand-ff to
       apply_tuple(), below. */
    virtual void operator()(const elem_t &elem) const override final {
      assert(this);
      apply_tuple(elem, std::index_sequence_for<params_t...>());
    }

    private:

    /* The base class to which we recur. */
    using recur_t = applier_t<functor_t, visitor_t, more_elems_t...>;

    /* Passes the variant state as well as the unrolled cached tuple of extra
       arguments into an application of the functor and caches the valued
       returned. */
    template <size_t... i>
    void apply_tuple(
        const elem_t &elem, std::index_sequence<i...> &&) const {
      assert(this);
      this->ret = (this->functor)(elem, std::get<i>(this->tuple)...);
    }

  };  /* for_fn<ret_t (params_t...)>
             ::applier_t<functor_t, visitor_t, elem_t, more_elems_t...> */

  /* Applies a functor to a variant, passing zero or more additional
     arguments and returning a non-null result. */
  template <typename functor_t, typename... elems_t, typename... args_t>
  static ret_t apply(
      functor_t &functor, const variant_t<elems_t...> &that,
      args_t &&... args) {
    assert(&that);
    ret_t ret;
    that.accept(applier_t<functor_t, visitor_t<elems_t...>, elems_t...>(
        functor, ret, std::forward<args_t>(args)...));
    return std::move(ret);
  }

};  // for_fn<ret_t (params_t...)>

/* Explicit specialization for a function which returns void. */
template <typename... params_t>
struct for_fn<void (params_t...)> {

  /* This will be our visitor, recursively defined.  It functions as an
     adapter, passing control from the virtual functions of the visitor
     to the appropriate overloads of operator() in the functor functor. */
  template <typename functor_t, typename visitor_t, typename... elems_t>
  class applier_t;

  /* The base case inherits from the visitor for the variant and caches
     references to the functor as well as the extra (non-variant) arguments
     to be passed to the functor.  This class also defines the nullary
     operator() override. */
  template <typename functor_t, typename visitor_t>
  class applier_t<functor_t, visitor_t>
      : public visitor_t {
    public:

    /* Cache stuff for later, when operator() gets dispatched. */
    applier_t(functor_t &functor, params_t &&... params)
        : functor(functor), tuple(std::forward_as_tuple(params)...) {}

    /* Override of the definition in visitor_t.  This is the nullary version,
       used when the variant is null.  It is just a hand-ff to apply_tuple(),
       below. */
    virtual void operator()() const override final {
      assert(this);
      apply_tuple(std::index_sequence_for<params_t...>());
    }

    protected:

    /* The functor we will apply. */
    functor_t &functor;

    /* The extra arguments (beyond the variant) to pass to the functor. */
    const std::tuple<params_t &&...> tuple;

    private:

    /* Unrolls the cached tuple of extra arguments into an application of the
       functor. */
    template <size_t... i>
    void apply_tuple(std::index_sequence<i...> &&) const {
      assert(this);
      functor(std::get<i>(tuple)...);
    }

  };  // for_fn<void (params_t...)>::applier_t<functor_t, visitor_t>

  /* Each iteration of the recursive case defines a unary overload of
     operator(). */
  template <
      typename functor_t, typename visitor_t,
      typename elem_t, typename... more_elems_t>
  class applier_t<functor_t, visitor_t, elem_t, more_elems_t...>
      : public applier_t<functor_t, visitor_t, more_elems_t...> {
    public:

    /* Pass everything up to the base case. */
    applier_t(functor_t &functor, params_t &&... params)
        : recur_t(functor, std::forward<params_t>(params)...) {}

    /* Override of the definition in visitor_t.  This is the unary version,
       used when the variant is non-null.  It is just a hand-ff to
       apply_tuple(), below. */
    virtual void operator()(const elem_t &elem) const override final {
      assert(this);
      apply_tuple(elem, std::index_sequence_for<params_t...>());
    }

    private:

    /* The base class to which we recur. */
    using recur_t = applier_t<functor_t, visitor_t, more_elems_t...>;

    /* Passes the variant state as well as the unrolled cached tuple of extra
       arguments into an application of the functor. */
    template <size_t... i>
    void apply_tuple(
        const elem_t &elem, std::index_sequence<i...> &&) const {
      assert(this);
      (this->functor)(elem, std::get<i>(this->tuple)...);
    }

  };  /* for_fn<void (params_t...)>
             ::applier_t<functor_t, visitor_t, elem_t, more_elems_t...> */

  /* Applies a functor to a variant, passing zero or more additional
     arguments. */
  template <typename functor_t, typename... elems_t, typename... args_t>
  static void apply(
      functor_t &functor, const variant_t<elems_t...> &that,
      args_t &&... args) {
    assert(&that);
    that.accept(applier_t<functor_t, visitor_t<elems_t...>, elems_t...>(
        functor, std::forward<args_t>(args)...));
  }

};  // for_fn<void (params_t...)>

/* Applies a functor to a variant, passing zero or more additional arguments,
   and possibly returning a value. */
template <typename functor_t, typename... elems_t, typename... args_t>
inline decltype(auto) apply(
    functor_t &functor, const variant_t<elems_t...> &that,
    args_t &&... args) {
  return for_fn<typename functor_t::fn_t>::apply(
      functor, that, std::forward<args_t>(args)...);
}

/* Applies a temporary functor to a variant, passing zero or more additional
   arguments, and possibly returning a value. */
template <typename functor_t, typename... elems_t, typename... args_t>
inline decltype(auto) apply(
    functor_t &&functor, const variant_t<elems_t...> &that,
    args_t &&... args) {
  return for_fn<typename functor_t::fn_t>::apply(
      functor, that, std::forward<args_t>(args)...);
}

}  // variant
}  // cppcon14
