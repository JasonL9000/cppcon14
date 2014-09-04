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

A stack-based discriminated union with a null state.

See "variant.test.cc" for examples of use.
--------------------------------------------------------------------------- */

#pragma once

#include <cassert>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <typeinfo>
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

/* We'll define the visitor class recursively. */
template <typename... elems_t>
struct visitor_t;

/* The base case handles a variant which is null. */
template <>
struct visitor_t<> {

  /* Override to handle a null variant. */
  virtual void operator()(nullptr_t) const = 0;

};  // visitor_t<>

/* Each iteration of the recursive case handles one type of element. */
template <typename elem_t, typename... more_elems_t>
struct visitor_t<elem_t, more_elems_t...>
    : visitor_t<more_elems_t...> {

  /* Pull the operator() overloads above us into scope. */
  using visitor_t<more_elems_t...>::operator();

  /* Override to handle a non-null variant. */
  virtual void operator()(const elem_t &) const = 0;

};  // visitor_t<elem_t, more_elems_t...>

/* ---------------------------------------------------------------------------
The variant class template itself.
--------------------------------------------------------------------------- */

/* A stack-based discriminated union with a null state. */
template <typename... elems_t>
class variant_t final {
  public:

  /* The type of visitor we accept. */
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

  /* Move-construct, leaving the donor null. */
  variant_t(variant_t &&that) noexcept {
    tag = that.tag;
    (tag->move_construct)(*this, std::move(that));
  }

  /* Copy-construct, leaving the exemplar intact. */
  variant_t(const variant_t &that) {
    (that.tag->copy_construct)(*this, that);
    tag = that.tag;
  }

  /* Destroy. */
  ~variant_t() {
    assert(this);
    (tag->destroy)(*this);
  }

  /* True iff. we're not null. */
  operator bool() const noexcept {
    assert(this);
    return tag != get_null_tag();
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

  /* Try to access our contents as a particular type.  If we don't currently
     contain a value of the requested type, throw std::bad_cast.  If we can
     never be of the requested type (that is, elem_t is not among our
     elems_t), this function is disabled. */
  template <
      typename elem_t, typename = std::enable_if_t<contains<elem_t>()>>
  const elem_t &as() const {
    assert(this);
    const elem_t *ptr = try_as<elem_t>();
    if (!ptr) {
      throw std::bad_cast();
    }
    return *ptr;
  }

  /* Accept the visitor and dispatch based on our contents. */
  void accept(const visitor_t &visitor) const {
    assert(this);
    (tag->accept)(*this, visitor);
  }

  /* The std::type_info for our contents, or a null pointer if we're null. */
  const std::type_info *get_type_info() const noexcept {
    assert(this);
    return (tag->get_type_info)();
  }

  /* Be null. */
  variant_t &reset() noexcept {
    assert(this);
    this->~variant_t();
    tag = get_null_tag();
    return *this;
  }

  /* Try to access our contents as a particular type.  If we don't currently
     contain a value of the requested type, return a null pointer.  If we can
     never be of the requested type (that is, elem_t is not among our
     elems_t), this function is disabled. */
  template <
      typename elem_t, typename = std::enable_if_t<contains<elem_t>()>>
  const elem_t *try_as() const {
    assert(this);
    try_as_functor_t<elem_t> try_as_functor;
    apply(try_as_functor, *this);
    return try_as_functor.ptr;
  }

  private:

  /* Look up constants defined by our set of possible types, then use those
     constants to define a type for our data storage space.  The space must
     be large enough to hold the largest of our types and aligned correctly
     for the most alignment-critical of our types. */
  using data_t =
      alignas(get_max_alignof<elems_t...>())
      char[get_max_sizeof<elems_t...>()];

  /* The functor used by try_as(), above. */
  template <typename elem_t>
  struct try_as_functor_t final {

    /* The signature of our operator() overloads, sans variant. */
    using fn1_t = void ();

    /* Do nothing for a null variant. */
    void operator()(nullptr_t) {}

    /* Do nothing for a variant of another type. */
    template <typename other_t>
    void operator()(const other_t &) {}

    /* Return a pointer to the value of a variant matching our type. */
    void operator()(const elem_t &val) {
      assert(this);
      ptr = &val;
    }

    /* After application, this either points to the value or remains null. */
    const elem_t *ptr = nullptr;

  };  // variant_t<elems_t...>::try_as_functor_t<elem_t>

  /* A variant keeps track of what state its in by keeping a pointer to an
     instance of this structure.  Think of it as a hand-rolled vtable,
     containing pointers to the functions that do the work of the variant.
     The get_tag() and get_null_tag() functions, below, are responsible for
     defining the instances of this type. */
  struct tag_t final {

    /* Move other into self and set other null. */
    void (*move_construct)(variant_t &self, variant_t &&other) noexcept;

    /* Copy other into self. */
    void (*copy_construct)(variant_t &self, const variant_t &other);

    /* Destroy self. */
    void (*destroy)(variant_t &) noexcept;

    /* Accept a visitor on behalf of self. */
    void (*accept)(const variant_t &self, const visitor_t &visitor);

    /* The std::type_info we handle, or a null pointer if we handle null. */
    const std::type_info *(*get_type_info)() noexcept;

  };  // variant_t

  /* Force our storage area into type. */
  template <typename elem_t>
  elem_t &force_as() & noexcept {
    assert(this);
    return reinterpret_cast<elem_t &>(data);
  }

  /* Force our storage area into type. */
  template <typename elem_t>
  elem_t &&force_as() && noexcept {
    assert(this);
    return reinterpret_cast<elem_t &&>(data);
  }

  /* Force our storage area into type. */
  template <typename elem_t>
  const elem_t &force_as() const noexcept {
    assert(this);
    return reinterpret_cast<const elem_t &>(data);
  }

  /* The tag we use when we contain an instance of elem_t. */
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
      },
      // get_type_info
      []() {
        return &typeid(elem_t);
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
        visitor(nullptr);
      },
      // get_type_info
      []() {
        return static_cast<const std::type_info *>(nullptr);
      }
    };
    return &tag;
  }

  /* The tag which takes action for us.  Never null. */
  const tag_t *tag;

  /* The data to be interpreted by our tag.  This always passes through one
     of the overloads of force_as() before we use it. */
  data_t data;

};  // variant_t<elems_t...>

/* ---------------------------------------------------------------------------
As we deal with applying functors (in the next two big sections), it would be
handy not to have to cope with differences between those which return a value
and those which return void.  The following two function overloads unify the
semantics of these cases by returning the result, if any, via indirection.  A
function which returns a value must be given a non-null pointer to a location
in which to store its result.  A function which returns void must be given a
void pointer, which will be ignored an so can (and should) be null.
--------------------------------------------------------------------------- */

/* Apply arguments to a functor and cache the non-void result to a given
   storage area. */
template <typename ret_t, typename functor_t, typename... args_t>
inline void apply_with_indirect_return(
    ret_t *ret, functor_t &functor, args_t &&... args) {
  assert(ret);
  assert(&functor);
  *ret = functor(std::forward<args_t>(args)...);
}

/* Apply arguments to a functor which returns void.  Note this overload is
   differentiated from the one above by the leading void pointer. */
template <typename functor_t, typename... args_t>
inline void apply_with_indirect_return(
    void *, functor_t &functor, args_t &&... args) {
  assert(&functor);
  functor(std::forward<args_t>(args)...);
}

/* ---------------------------------------------------------------------------
We now define overloads of apply() which take a functor, a variant, and zero
or more additional arguments.  These allow you to invoke unary operations on
variants without having to write your own visitors.  To do this, we provide
canned visitors, called appliers, which adapt your functor to the visitor-
style dispatch used by variant_t.
--------------------------------------------------------------------------- */

/* Definitions of appliers used by for_fn1<>, below. */
template <typename ret_t, typename... params_t>
struct fn1_appliers {

  /* This will be our visitor, recursively defined.  It functions as an
     adapter, passing control from the virtual functions of the visitor
     to the appropriate overloads of operator() in the functor. */
  template <typename functor_t, typename visitor_t, typename... elems_t>
  class applier_t;

  /* The base case inherits from the visitor for the variant and caches
     references to the functor and to the storage location for the value
     to be returned by the functor, as well as the extra (non-
     variant) arguments to be passed to the functor.  This base case also
     handles visiting a null variant. */
  template <typename functor_t, typename visitor_t>
  class applier_t<functor_t, visitor_t>
      : public visitor_t {
    public:

    /* Cache stuff for later, when operator() gets dispatched. */
    applier_t(functor_t &functor, ret_t *ret, params_t &&... params)
        : functor(functor), ret(ret),
          tuple(std::forward_as_tuple(params)...) {}

    /* Override of the definition in visitor_t.  This is the nullary version,
       used when the variant is null.  It is just a hand-ff to apply_tuple(),
       below. */
    virtual void operator()(nullptr_t) const override final {
      assert(this);
      apply_tuple(std::index_sequence_for<params_t...>());
    }

    protected:

    /* The functor we will apply. */
    functor_t &functor;

    /* The location in which to cache the result of the functor.  If the
       functor returns void, this is ignored and should be null. */
    ret_t *ret;

    /* The extra arguments (beyond the variant) to pass to the functor. */
    const std::tuple<params_t &&...> tuple;

    private:

    /* Unrolls the cached tuple of extra arguments into an application of the
       functor and caches the valued returned. */
    template <size_t... i>
    void apply_tuple(std::index_sequence<i...> &&) const {
      assert(this);
      apply_with_indirect_return(
          ret, functor, nullptr, std::get<i>(tuple)...);
    }

  };  // fn1_appliers<ret_t (params_t...)>::applier_t<functor_t, visitor_t>

  /* Each iteration of the recursive case defines a unary overload of
     operator(). */
  template <
      typename functor_t, typename visitor_t,
      typename elem_t, typename... more_elems_t>
  class applier_t<functor_t, visitor_t, elem_t, more_elems_t...>
      : public applier_t<functor_t, visitor_t, more_elems_t...> {
    public:

    /* Pass everything up to the base case. */
    applier_t(functor_t &functor, ret_t *ret, params_t &&... params)
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
      apply_with_indirect_return(
          this->ret, this->functor, elem, std::get<i>(this->tuple)...);
    }

  };  // fn1_appliers<ret_t (params_t...)>
      //   ::applier_t<functor_t, visitor_t, elem_t, more_elems_t...>

};  // fn1_appliers<ret_t (params_t...)>

/* ---------------------------------------------------------------------------
A helper for picking apart a template parameter of function type.  We'll use
explicit specializations to separate two cases: functions which return non-
void and functions which return void.  Within each, we'll define an apply()
function which will apply a functor to a variant, passing zero or more
additional arguments.
--------------------------------------------------------------------------- */

/* We'll leave the generic case undefined and just provide two explicit
   specializations. */
template <typename fn1_t>
struct for_fn1;

/* Returning non-void. */
template <typename ret_t, typename... params_t>
struct for_fn1<ret_t (params_t...)> {
  template <typename functor_t, typename... elems_t, typename... args_t>
  static ret_t apply(
        functor_t &functor, const variant_t<elems_t...> &that,
        args_t &&... args) {
    assert(&that);
    using applier_t = typename fn1_appliers<ret_t, params_t...>
        ::template applier_t<functor_t, visitor_t<elems_t...>, elems_t...>;
    ret_t ret;
    that.accept(applier_t(functor, &ret, std::forward<args_t>(args)...));
    return std::move(ret);
  }
};

/* Returning void. */
template <typename... params_t>
struct for_fn1<void (params_t...)> {
  template <typename functor_t, typename... elems_t, typename... args_t>
  static void apply(
        functor_t &functor, const variant_t<elems_t...> &that,
        args_t &&... args) {
    assert(&that);
    using applier_t = typename fn1_appliers<void, params_t...>
        ::template applier_t<functor_t, visitor_t<elems_t...>, elems_t...>;
    that.accept(applier_t(functor, nullptr, std::forward<args_t>(args)...));
  }
};

/* ---------------------------------------------------------------------------
These two overloads of apply() differ only in how they bind to their functor:
the first uses an lvalue, the second an rvalue.  In each case, apply() applies
the given functor to a given variant, possibly returning a value.  The functor
type must define a local type called "fn1_t" which must give the signature
shared by the functor's handler functions; that is, the function signature
with the leading variant parameter omitted.  We use SFINAE based on the
presence of this magic name to enable these overloads of apply().
--------------------------------------------------------------------------- */

/* Functor as lvalue. */
template <
    typename functor_t, typename... elems_t, typename... args_t,
    typename fn1_t = typename functor_t::fn1_t>
inline decltype(auto) apply(
    functor_t &functor, const variant_t<elems_t...> &that,
    args_t &&... args) {
  return for_fn1<fn1_t>::apply(
      functor, that, std::forward<args_t>(args)...);
}

/* Functor as rvalue. */
template <
    typename functor_t, typename... elems_t, typename... args_t,
    typename fn1_t = typename functor_t::fn1_t>
inline decltype(auto) apply(
    functor_t &&functor, const variant_t<elems_t...> &that,
    args_t &&... args) {
  return for_fn1<fn1_t>::apply(
      functor, that, std::forward<args_t>(args)...);
}

/* ---------------------------------------------------------------------------
We now define overloads of apply() which take a functor, a pair of variants,
and zero or more additional arguments.  These allow you to invoke binary
operations on variants without having to write your own chain of visitors.  To
do this, we provide canned visitors in pairs, called lhs appliers and rhs
appliers.  The lhs applier visits the lhs variant and forwards the lhs value
to the rhs applier.  The rhs applier visits the rhs variant and, now in
possession of both left- and right-hand values, dispatches to your functor.
--------------------------------------------------------------------------- */

/* Definitions of appliers and their helpers used by for_fn2<>, below. */
template <typename ret_t, typename... params_t>
struct fn2_appliers {

  /* The type of tuple we use when forwarding the extra (that is, non-variant)
     function parameters. */
  using tuple_t = std::tuple<params_t &&...>;

  /* This will be our visitor, recursively defined, to the rhs variant.  It
     functions as an adapter, passing control from the virtual functions of
     the visitor to the appropriate overloads of operator() in the functor. */
  template <
      typename functor_t, typename lhs_elem_t, typename rhs_visitor_t,
      typename... rhs_elems_t>
  class rhs_applier_t;

  /* The base case inherits from the visitor for the rhs variant and caches
     references to the functor, to the value discovered while visiting the lhs
     variant, to the storage location for the value to be returned by the
     functor, and to the tuple of extra arguments to be passed to the functor.
     This base case also handles visiting a null rhs variant. */
  template <
      typename functor_t, typename lhs_elem_t, typename rhs_visitor_t>
  class rhs_applier_t<functor_t, lhs_elem_t, rhs_visitor_t>
      : public rhs_visitor_t {
    public:

    /* Cache stuff for later, when operator() gets dispatched. */
    rhs_applier_t(
        functor_t &functor, const lhs_elem_t &lhs_elem,
        ret_t *ret, const tuple_t &tuple)
        : functor(functor), lhs_elem(lhs_elem), ret(ret), tuple(tuple) {}

    /* Override of the definition in rhs_visitor_t.  This is version used when
       the rhs-variant is null.  It's just a hand-off to apply_tuple(),
       below. */
    virtual void operator()(nullptr_t) const override final {
      assert(this);
      apply_tuple(std::index_sequence_for<params_t...>());
    }

    protected:

    /* The functor we will apply. */
    functor_t &functor;

    /* The value discovered when we visited the lhs variant. */
    const lhs_elem_t &lhs_elem;

    /* The location in which to cache the result of the functor.  If the
       functor returns void, this is ignored and should be null. */
    ret_t *ret;

    /* The extra arguments (beyond the variants) to pass to the functor. */
    const tuple_t &tuple;

    private:

    /* Passes the previously discovered lhs value and the newly discovered
       rhs value (which is null), along with the arguments unrolled from
       the tuple, to the functor, caching the value returned, if any. */
    template <size_t... i>
    void apply_tuple(std::index_sequence<i...> &&) const {
      assert(this);
      apply_with_indirect_return(
          ret, functor, lhs_elem, nullptr, std::get<i>(tuple)...);
    }

  };  // fn2_appliers<ret_t (params_t...)>
      //   ::rhs_applied_t<functor_t, lhs_elem_t, rhs_visitor>

  /* Each iteration of this recursive case defines an overload of
     operator() for some element of the rhs variant. */
  template <
      typename functor_t, typename lhs_elem_t, typename rhs_visitor_t,
      typename rhs_elem_t, typename... more_rhs_elems_t>
  class rhs_applier_t<
      functor_t, lhs_elem_t, rhs_visitor_t, rhs_elem_t, more_rhs_elems_t...>
      : public rhs_applier_t<
            functor_t, lhs_elem_t, rhs_visitor_t, more_rhs_elems_t...> {
    public:

    /* Pass everything up to the base. */
    rhs_applier_t(
        functor_t &functor, const lhs_elem_t &lhs_elem,
        ret_t *ret, const tuple_t &tuple)
        : recur_t(functor, lhs_elem, ret, tuple) {}

    /* Override of the definition in rhs_visitor_t.  This is version used when
       the rhs-variant is non-null.  It is just a hand-ff to apply_tuple(),
       below. */
    virtual void operator()(const rhs_elem_t &rhs_elem) const override final {
      assert(this);
      apply_tuple(rhs_elem, std::index_sequence_for<params_t...>());
    }

    private:

    /* The base class to which we recur. */
    using recur_t = rhs_applier_t<
        functor_t, lhs_elem_t, rhs_visitor_t, more_rhs_elems_t...>;

    /* Passes the previously discovered lhs value and the newly discovered
       rhs value (which is non-null), along with the arguments unrolled from
       the tuple, to the functor, caching the value returned, if any. */
    template <size_t... i>
    void apply_tuple(
        const rhs_elem_t &rhs_elem, std::index_sequence<i...> &&) const {
      assert(this);
      apply_with_indirect_return(
          this->ret, this->functor, this->lhs_elem, rhs_elem,
          std::get<i>(this->tuple)...);
    }

  };  // fn2_appliers<ret_t (params_t...)>
      //   ::rhs_applied_t<
      //       functor_t, lhs_elem_t, rhs_visitor,
      //       rhs_elem_t, more_rhs_elems_t...>

  /* We need to define our lhs applier with an understanding of what the
     rhs applier will be, so we do so in this space. */
  template <typename functor_t, typename... rhs_elems_t>
  struct for_rhs {

    /* The type of variant we have on our right-hand side.  We need to know
       this so we can cache a reference to it while we visit the left-hand
       side. */
    using rhs_variant_t = variant_t<rhs_elems_t...>;

    /* This will be our visitor, recursively defined, to the lhs variant.  It
       functions as an adapter, passing control from the virtual functions of
       the visitor to rhs applier. */
    template <typename lhs_visitor_t, typename... lhs_elems_t>
    class lhs_applier_t;

    /* The base case inherits from the visitor for the lhs variant and caches
       references to the functor, to the rhs variant, to the storage location
       for the value to be returned by the functor, and to the tuple of extra
       (non-variant) arguments to be passed to the functor.  This class also
       defines the nullary operator() override. */
    template <typename lhs_visitor_t>
    class lhs_applier_t<lhs_visitor_t>
        : public lhs_visitor_t {
      public:

      /* Cache stuff for later, when operator() gets dispatched. */
      lhs_applier_t(
          functor_t &functor, const rhs_variant_t &rhs_variant,
          ret_t *ret, params_t &&... params)
          : functor(functor), rhs_variant(rhs_variant),
            ret(ret), tuple(std::forward_as_tuple(params)...) {}

      /* Override of the definition in lhs_visitor_t.  This is version used
         when the variant is null.  It is just a hand-off to apply_tuple(),
         below. */
      virtual void operator()(nullptr_t) const override final {
        assert(this);
        apply_tuple(std::index_sequence_for<params_t...>());
      }

      protected:

      /* The functor we will pass to the rhs applier. */
      functor_t &functor;

      /* The right-hand side we will visit next. */
      const rhs_variant_t &rhs_variant;

      /* The location in which to cache the result of the functor.  If the
         functor returns void, this is ignored and should be null. */
      ret_t *ret;

      /* The extra arguments (beyond the variants) to pass to the functor. */
      const tuple_t tuple;

      private:

      /* Passes the functor, the lhs value (which is null), the location of
         the returned value cache, and the tuple of extra arguments to the rhs
         applier and visits the rhs variant. */
      template <size_t... i>
      void apply_tuple(std::index_sequence<i...> &&) const {
        assert(this);
        using applier_t = rhs_applier_t<
            functor_t, nullptr_t, visitor_t<rhs_elems_t...>, rhs_elems_t...>;
        rhs_variant.accept(applier_t(functor, nullptr, ret, tuple));
      }

    };  // fn2_appliers<ret_t (params_t...)>
        //   ::for_rhs<functor_t, rhs_elems_t...>
        //   ::lhs_applier_t<lhs_visitor_t>

    /* Each iteration of this recursive case defines an overload of
       operator() for some element of the lhs variant. */
    template <
        typename lhs_visitor_t,
        typename lhs_elem_t, typename... more_lhs_elems_t>
    class lhs_applier_t<lhs_visitor_t, lhs_elem_t, more_lhs_elems_t...>
        : public lhs_applier_t<lhs_visitor_t, more_lhs_elems_t...> {
      public:

      /* Pass everything up to the base case. */
      lhs_applier_t(
          functor_t &functor, const rhs_variant_t &rhs_variant,
          ret_t *ret, params_t &&... params)
          : recur_t(
                functor, rhs_variant, ret,
                std::forward<params_t>(params)...) {}

      /* Override of the definition in lhs_visitor_t.  This is version used
         when the variant is non-null.  It is just a hand-off to
         apply_tuple(), below. */
      virtual void operator()(
          const lhs_elem_t &lhs_elem) const override final {
        assert(this);
        apply_tuple(lhs_elem, std::index_sequence_for<params_t...>());
      }

      private:

      /* The base class to which we recur. */
      using recur_t = lhs_applier_t<lhs_visitor_t, more_lhs_elems_t...>;

      /* Passes the functor, the lhs value (which is non-null), the location
         of the returned value cache, and the tuple of extra arguments to the
         rhs applier and visits the rhs variant. */
      template <size_t... i>
      void apply_tuple(
          const lhs_elem_t &lhs_elem, std::index_sequence<i...> &&) const {
        assert(this);
        using applier_t = rhs_applier_t<
            functor_t, lhs_elem_t, visitor_t<rhs_elems_t...>, rhs_elems_t...>;
        this->rhs_variant.accept(applier_t(
            this->functor, lhs_elem, this->ret, this->tuple));
      }

    };  // fn2_appliers<ret_t (params_t...)>
        //   ::for_rhs<functor_t, rhs_elems_t...>
        //   ::lhs_applier_t<lhs_visitor_t, lhs_elem_t, more_lhs_elems_t...>

  };  // fn2_appliers<ret_t (params_t...)>
      //   ::for_rhs<functor_t, rhs_elems_t...>

};  // fn2_appliers<ret_t (params_t...)>

/* ---------------------------------------------------------------------------
A helper for picking apart a template parameter of function type.  We'll use
explicit specializations to separate two cases: functions which return non-
void and functions which return void.  Within each, we'll define an apply()
function which will apply a functor to a variant, passing zero or more
additional arguments.
--------------------------------------------------------------------------- */

/* We'll leave the generic case undefined and just provide two explicit
   specializations. */
template <typename fn2_t>
struct for_fn2;

/* Returning non-void. */
template <typename ret_t, typename... params_t>
struct for_fn2<ret_t (params_t...)> {
  template <
      typename functor_t, typename... lhs_elems_t, typename... rhs_elems_t,
      typename... args_t>
  static ret_t apply(
        functor_t &functor,
        const variant_t<lhs_elems_t...> &lhs,
        const variant_t<rhs_elems_t...> &rhs,
        args_t &&... args) {
    assert(&lhs);
    ret_t ret;
    using lhs_applier_t = typename fn2_appliers<ret_t, params_t...>
        ::template for_rhs<functor_t, rhs_elems_t...>
        ::template lhs_applier_t<visitor_t<lhs_elems_t...>, lhs_elems_t...>;
    lhs.accept(lhs_applier_t(
      functor, rhs, &ret, std::forward<args_t>(args)...));
    return std::move(ret);
  }
};

/* Returning void. */
template <typename... params_t>
struct for_fn2<void (params_t...)> {
  template <
      typename functor_t, typename... lhs_elems_t, typename... rhs_elems_t,
      typename... args_t>
  static void apply(
        functor_t &functor,
        const variant_t<lhs_elems_t...> &lhs,
        const variant_t<rhs_elems_t...> &rhs,
        args_t &&... args) {
    assert(&lhs);
    using lhs_applier_t = typename fn2_appliers<void, params_t...>
        ::template for_rhs<functor_t, rhs_elems_t...>
        ::template lhs_applier_t<visitor_t<lhs_elems_t...>, lhs_elems_t...>;
    lhs.accept(lhs_applier_t(
      functor, rhs, nullptr, std::forward<args_t>(args)...));
  }
};

/* ---------------------------------------------------------------------------
These two overloads of apply() differ only in how they bind to their functor:
the first uses an lvalue, the second an rvalue.  In each case, apply() applies
the given functor to a given pair of variants, lhs and rhs, possibly returning
a value.  The functor type must define a local type called "fn2_t" which must
give the signature shared by the functor's handler functions; that is, the
function signature with the two leading variant parameters omitted.  We use
SFINAE based on the presence of this magic name to enable these overloads of
apply().
--------------------------------------------------------------------------- */

/* Functor as lvalue. */
template <
    typename functor_t, typename... lhs_elems_t, typename... rhs_elems_t,
    typename... args_t, typename fn2_t = typename functor_t::fn2_t>
inline decltype(auto) apply(
    functor_t &functor,
    const variant_t<lhs_elems_t...> &lhs,
    const variant_t<rhs_elems_t...> &rhs,
    args_t &&... args) {
  return for_fn2<fn2_t>::apply(
      functor, lhs, rhs, std::forward<args_t>(args)...);
}

/* Functor as rvalue. */
template <
    typename functor_t, typename... lhs_elems_t, typename... rhs_elems_t,
    typename... args_t, typename fn2_t = typename functor_t::fn2_t>
inline decltype(auto) apply(
    functor_t &&functor,
    const variant_t<lhs_elems_t...> &lhs,
    const variant_t<rhs_elems_t...> &rhs,
    args_t &&... args) {
  return for_fn2<fn2_t>::apply(
      functor, lhs, rhs, std::forward<args_t>(args)...);
}

}  // variant
}  // cppcon14
