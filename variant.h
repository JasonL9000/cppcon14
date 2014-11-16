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

A stack-based discriminated union.

See "variant.test.cc" for examples of use.
--------------------------------------------------------------------------- */

#pragma once

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace lib {

/* C++17 std::apply */
template <typename fn_t, typename tuple_t, size_t... i>
decltype(auto) apply_impl(fn_t &&fn, 
                          tuple_t &&tuple, 
                          std::index_sequence<i...>) {
  return std::forward<fn_t>(fn)(std::get<i>(std::forward<tuple_t>(tuple))...);
}

template <typename fn_t, typename tuple_t>
decltype(auto) apply(fn_t &&fn, tuple_t &&tuple) {
  return apply_impl(std::forward<fn_t>(fn),
                    std::forward<tuple_t>(tuple),
                    std::make_index_sequence<
                        std::tuple_size<std::decay_t<tuple_t>>::value>());
}

/* C++14 std::max_element. */
template <typename ForwardIter>
constexpr ForwardIter max_element(ForwardIter begin, ForwardIter end) {
  if (begin == end) {
    return end;
  }  // if
  ForwardIter largest = begin;
  ++begin;
  for (; begin != end; ++begin) {
    if (*largest < *begin) {
      largest = begin;
    }  // if
  }  // for
  return largest;
}

/* C++14 std::max. */
template <typename T>
constexpr T max(std::initializer_list<T> elems) {
  assert(elems.size() > 0);
  return *max_element(std::begin(elems), std::end(elems));
}

}  // lib

namespace cppcon14 {
namespace variant {

/* Identity type trait. Useful for succintly defining type traits. */
template <typename T>
struct identity { using type = T; };

/* The type that represents the null state. */
struct null_t {};

/**
 *  The base class for all visitors to variants.
 **/

/* We'll define the visitor class recursively. */
template <typename elem_t, typename... elems_t>
struct visitor_t;

/* The base case. */
template <typename elem_t>
struct visitor_t<elem_t> {

  /* Override to handle an elem_t. */
  virtual void operator()(const elem_t &) const = 0;

};  // visitor_t<elem_t>

/* Each iteration of the recursive case handles one type of element. */
template <typename elem_t, typename... more_elems_t>
struct visitor_t : visitor_t<more_elems_t...> {

  /* Pull the operator() overloads above us into scope. */
  using visitor_t<more_elems_t...>::operator();

  /* Override to handle an elem_t. */
  virtual void operator()(const elem_t &) const = 0;

};  // visitor_t

/* A class that inherits from lambdas and results in an overloaded lambda. */
template <typename ret_t, typename... lambdas_t>
struct overload_t;

/* Base case. */
template <typename ret_t_, typename lambda_t>
struct overload_t<ret_t_, lambda_t> : lambda_t {

  /* Used to look-up the expected return type of the lambdas. */
  using ret_t = ret_t_;

  using lambda_t::operator();

  overload_t(lambda_t lambda) : lambda_t(std::move(lambda)) {}

};  // overload_t<ret_t_, lambda_t>

/* Recursive case. */
template <typename ret_t, typename lambda_t, typename... more_lambdas_t>
struct overload_t<ret_t, lambda_t, more_lambdas_t...>
    : lambda_t, overload_t<ret_t, more_lambdas_t...> {

  using super_t = overload_t<ret_t, more_lambdas_t...>;

  using lambda_t::operator();
  using super_t::operator();

  /* Cache the lambdas. */
  overload_t(lambda_t lambda, more_lambdas_t... more_lambdas)
      : lambda_t(std::move(lambda)), super_t(std::move(more_lambdas)...) {}

};  // overload<ret_t, lambda_t, more_lambdas_t...>

/* Factory function for overload. */
template <typename ret_t, typename... lambdas_t>
auto make_overload(lambdas_t &&... lambdas) {
  return overload_t<ret_t, std::decay_t<lambdas_t>...>(
      std::forward<lambdas_t>(lambdas)...);
}

/**
 *  The variant class template itself.
 **/

/* A stack-based discriminated union. */
template <typename... elems_t>
class variant_t {
  private:

  /* The members of this variant.  If you get a compilation error here,
     you probably have duplicates types. */
  struct members_t : identity<elems_t>... {};

  /* True iff. elem_t is among our elems_t; that is, if this variant can
     ever contain a value of type elem_t.  We use this in a few places as a
     predicate to std::enable_if<>. */
  template <typename elem_t>
  using contains = std::is_base_of<identity<elem_t>, members_t>;

  /* If we have a null state, we must have > 1 states,
     otherwise we need > 0 states. */
  static_assert(sizeof...(elems_t) > contains<null_t>::value,
                "We need at least 1 state, and more than 1 if we are nullable.");

  public:

  /* The type of visitor we accept. */
  using visitor_t = variant::visitor_t<elems_t...>;

  /* Default construct to a null state.
     Only provided if we are nullable. */
  template <typename T = null_t,
            typename = std::enable_if_t<contains<T>::value>>
  variant_t(null_t = null_t()) noexcept { tag = get_null_tag(); }

  /* Construct off of an element. 
     If we cannot assume the requested type (that is, if elem_t is not among our
     elems_t), this constructor is disabled. */
  template <typename elem_t,
            typename = std::enable_if_t<contains<std::decay_t<elem_t>>::value>>
  variant_t(elem_t &&elem) noexcept {
    assert(&elem);
    new (data) std::decay_t<elem_t>(std::forward<elem_t>(elem));
    tag = get_tag<std::decay_t<elem_t>>();
  }

  /* Move-construct, leaving the donor null if it has a null state. */
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
  virtual ~variant_t() {
    assert(this);
    (tag->destroy)(*this);
  }

  /* Returns true if we are not null, otherwise false.
     Only provided if null is one of the possible states.
     This should be marked explicit but it's not for now just for test cases. */
  template <typename..., typename T = null_t>
  /* explicit */ operator std::enable_if_t<contains<T>::value, bool>() const {
    return tag != get_null_tag();
  }

  /* Move-assign, leaving the donor null. */
  variant_t &operator=(variant_t &&that) noexcept {
    assert(this);
    assert(&that);
    if (this != &that) {
      this->~variant_t();
      new (this) variant_t(std::move(that));
    }  // if
    return *this;
  }

  /* Copy-assign, leaving the exemplar intact. */
  variant_t &operator=(const variant_t &that) {
    assert(this);
    assert(&that);
    if (this != &that) {
      *this = variant_t(that);
    }  // if
    return *this;
  }

  /* Accept the visitor and dispatch based on our contents. */
  void accept(const visitor_t &visitor) const {
    assert(this);
    (tag->accept)(*this, visitor);
  }

  /* Try to access our contents as a particular type.  If we don't currently
     contain a value of the requested type, throw std::bad_cast.  If we can
     never be of the requested type (that is, elem_t is not among our
     elems_t), this function is disabled. */
  template <typename elem_t>
  std::enable_if_t<contains<elem_t>::value, const elem_t &> as() const {
    assert(this);
    const elem_t *ptr = try_as<elem_t>();
    if (!ptr) {
      throw std::bad_cast();
    }  // if
    return *ptr;
  }

  /* The std::type_info for our contents, or a null pointer if we're null. */
  const std::type_info *get_type_info() const noexcept {
    assert(this);
    return (tag->get_type_info)();
  }

  /* Be null. */
  template <typename..., typename T = null_t>
  std::enable_if_t<contains<T>::value, variant_t &> &reset() noexcept {
    assert(this);
    this->~variant_t();
    tag = get_null_tag();
    return *this;
  }

  /* Try to access our contents as a particular type.  If we don't currently
     contain a value of the requested type, return a null pointer.  If we can
     never be of the requested type (that is, elem_t is not among our
     elems_t), this function is disabled. */
  template <typename elem_t>
  std::enable_if_t<contains<elem_t>::value, const elem_t *> try_as() const {
    assert(this);
    return apply(
        make_overload<const elem_t *>([](const elem_t &val) { return &val; },
                                      [](const auto &) { return nullptr; }),
        *this);
  }

  private:

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
  const elem_t &force_as() const & noexcept {
    assert(this);
    return reinterpret_cast<const elem_t &>(data);
  }

  /* The tag we use when we contain an instance of elem_t. */
  template <typename elem_t>
  static const tag_t *get_tag() {
    static tag_t tag {
      [](variant_t &self, variant_t &&other) {
        new (self.data) elem_t(std::move(other).template force_as<elem_t>());
        make_overload<void>(
            [](std::true_type, auto &&other) { std::move(other).reset(); },
            [](std::false_type, auto &&) {})
          (contains<null_t>(), std::move(other));
      },
      [](variant_t &self, const variant_t &other) {
        new (self.data) elem_t(other.force_as<elem_t>());
      },  // copy_construct
      [](variant_t &self) { self.force_as<elem_t>().~elem_t(); },  // destroy
      [](const variant_t &self, const visitor_t &visitor) {
        visitor(self.force_as<elem_t>());
      },  // accept
      []() { return &typeid(elem_t); }  // get_type_info
    };
    return &tag;
  }

  /* The tag we use iff. we're null. */
  static const tag_t *get_null_tag() {
    static tag_t tag {
      [](variant_t &, variant_t &&) {},  // move_construct
      [](variant_t &, const variant_t &) {},  // copy_construct
      [](variant_t &) {},  // destroy
      [](const variant_t &, const visitor_t &visitor) {
        visitor(null_t());
      },  // accept
      []() -> const std::type_info * { return nullptr; }  // get_type_info
    };
    return &tag;
  }

  /* The tag which takes action for us.  Never null. */
  const tag_t *tag = nullptr;

  /* The data to be interpreted by our tag.  This always passes through one
     of the overloads of force_as() before we use it. */
  alignas(lib::max({alignof(elems_t)...}))
      char data[lib::max({sizeof(elems_t)...})];

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

template <typename some_visitor_t,
          typename ret_t,
          typename functor_t,
          typename members_t,
          typename variants_t,
          typename... elems_t>
class applier_t;

template <typename some_visitor_t,
          typename ret_t,
          typename functor_t,
          typename... members_t,
          typename... variants_t>
class applier_t<some_visitor_t,
                ret_t,
                functor_t,
                std::tuple<const members_t &...>,
                std::tuple<const variants_t &...>> : public some_visitor_t {
  public:

  applier_t(ret_t *ret,
            functor_t &functor,
            std::tuple<const members_t &...> &&members,
            std::tuple<const variants_t &...> &&variants)
      : functor(functor),
        ret(ret),
        members(std::move(members)),
        variants(std::move(variants)) {}

  functor_t &functor;

  ret_t *ret;

  std::tuple<const members_t &...> members;

  std::tuple<const variants_t &...> variants;

};  // applier_t<some_visitor_t,
    //           ret_t,
    //           functor_t,
    //           members_t...,
    //           variants_t...>

template <typename some_visitor_t,
          typename ret_t,
          typename functor_t,
          typename... members_t,
          typename... variants_t,
          typename elem_t,
          typename... more_elems_t>
class applier_t<some_visitor_t,
                ret_t,
                functor_t,
                std::tuple<const members_t &...>,
                std::tuple<const variants_t &...>,
                elem_t,
                more_elems_t...>
    : public applier_t<some_visitor_t,
                       ret_t,
                       functor_t,
                       std::tuple<const members_t &...>,
                       std::tuple<const variants_t &...>,
                       more_elems_t...> {
  public:

  /* Pass everything up to the base case. */
  applier_t(ret_t *ret,
            functor_t &functor,
            std::tuple<const members_t &...> &&members,
            std::tuple<const variants_t &...> &&variants)
      : applier_t<some_visitor_t,
                  ret_t,
                  functor_t,
                  std::tuple<const members_t &...>,
                  std::tuple<const variants_t &...>,
                  more_elems_t...>(
            ret, functor, std::move(members), std::move(variants)) {}

  /* Override of the definition in lhs_visitor_t.  This is version used
     when the variant is non-null.  It is just a hand-off to
     apply_tuple(), below. */
  virtual void operator()(const elem_t &elem) const override final {
    assert(this);
    dispatch(std::index_sequence_for<members_t...>(),
             elem,
             std::index_sequence_for<variants_t...>());
  }

  private:

  template <typename... more_members_t>
  void dispatch_impl(
      std::tuple<const more_members_t &...> &&more_members) const {
    make_overload<void>(
        [&](auto *ret) {
          *ret = lib::apply(this->functor, std::move(more_members));
        },
        [&](void *) { lib::apply(this->functor, std::move(more_members)); })
      (this->ret);
  }

  template <typename... more_members_t,
            typename... elems_t,
            typename... more_variants_t>
  void dispatch_impl(std::tuple<const more_members_t &...> &&more_members,
                     const variant_t<elems_t...> &variant,
                     const more_variants_t &... more_variants) const {
    using applier_t = applier_t<typename variant_t<elems_t...>::visitor_t,
                                ret_t,
                                functor_t,
                                std::tuple<const more_members_t &...>,
                                std::tuple<const more_variants_t &...>,
                                elems_t...>;
    assert(&variant);
    variant.accept(applier_t(this->ret,
                             this->functor,
                             std::move(more_members),
                             std::forward_as_tuple(more_variants...)));
  }

  template <size_t... i, size_t... j, size_t... k>
  void dispatch(std::index_sequence<i...>,
                const elem_t &elem,
                std::index_sequence<j...>) const {
    dispatch_impl(std::forward_as_tuple(std::get<i>(this->members)..., elem),
                  std::get<j>(this->variants)...);
  }

};  //  applier_t<some_visitor_t,
    //            ret_t,
    //            functor_t,
    //            members_t...,
    //            variants_t...,
    //            elem_t,
    //            more_elems_t...>

/* ---------------------------------------------------------------------------
A helper for picking apart a template parameter of function type.  We'll use
explicit specializations to separate two cases: functions which return non-
void and functions which return void.  Within each, we'll define an apply()
function which will apply a functor to a variant, passing zero or more
additional arguments.
--------------------------------------------------------------------------- */

/* Returning non-void. */
template <typename ret_t>
struct storage_t {

  ret_t get() { return std::move(ret); }

  ret_t *ptr() { return &ret; }

  ret_t ret;

};  // storage_t

/* Returning void. */
template <>
struct storage_t<void> {

  void get() && {}

  void *ptr() { return nullptr; }

};  // storage_t<void>

template <typename functor_t,
          typename... elems_t,
          typename... more_variants_t>
decltype(auto) apply(functor_t &&functor,
                     const variant_t<elems_t...> &variant,
                     const more_variants_t &... more_variants) {
  using ret_t = typename std::decay_t<functor_t>::ret_t;
  using applier_t = applier_t<visitor_t<elems_t...>,
                              ret_t,
                              functor_t,
                              std::tuple<>,
                              std::tuple<const more_variants_t &...>,
                              elems_t...>;
  assert(&variant);
  storage_t<ret_t> ret;
  variant.accept(applier_t(ret.ptr(),
                           functor,
                           std::forward_as_tuple(),
                           std::forward_as_tuple(more_variants...)));
  return std::move(ret).get();
}

template <typename ret_t, typename... elems_t, typename... lambdas_t>
decltype(auto)
    match(const variant_t<elems_t...> &that, lambdas_t &&... lambdas) {
  return apply(make_overload<ret_t>(std::forward<lambdas_t>(lambdas)...), that);
}

template <typename ret_t,
          typename... lhs_elems_t,
          typename... rhs_elems_t,
          typename... lambdas_t>
decltype(auto) match(const variant_t<lhs_elems_t...> &lhs,
                     const variant_t<rhs_elems_t...> &rhs,
                     lambdas_t &&... lambdas) {
  return apply(
      make_overload<ret_t>(std::forward<lambdas_t>(lambdas)...), lhs, rhs);
}

template <typename ret_t, typename... variants_t, typename... lambdas_t>
decltype(auto) match(const std::tuple<variants_t...> &that,
                     lambdas_t &&... lambdas) {
  return lib::apply([&](auto && ... args)->decltype(auto) {
                      return apply(make_overload<ret_t>(
                                       std::forward<lambdas_t>(lambdas)...),
                                   std::forward<decltype(args)>(args)...);
                    },
                    that);
}

}  // variant
}  // cppcon14
