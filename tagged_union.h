/* TODO

only include default constructor if type set includes void
add passing conventions to visitor handlers
separate visitor into accessor and mutator
disallow empty type set
what is the relationship between tagged_union_t<void, T> and opt_t<t>?
make sample application
wrap implementation in privacy
rewrite 'contains'
can we have a set of function objects as our verb?
add as<T> and try_as<t>
add construction between unions of intersecting sets, with bad_cast
*/

#pragma once

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace tagged_union {

template <size_t idx, typename... elems_t>
struct type_list_impl_t;

template <size_t idx>
struct type_list_impl_t<idx> {
  template <typename elem_t>
  static constexpr bool contains(elem_t *) { return false; }
};

template <size_t idx, typename elem_t, typename... more_elems_t>
struct type_list_impl_t<idx, elem_t, more_elems_t...>
    : type_list_impl<idx + 1, more_elems_t...> {
  static constexpr bool contains(elem_t *) { return true; }
};

template <typename elem_t, typename... elems_t>
struct join { using type = elem_t elems_t...; };

/* TODO */
template <typename val_t>
constexpr val_t max(val_t lhs, val_t rhs) {
  return (lhs < rhs) ? rhs : lhs;
}

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
      = max(alignof(elem_t), for_more_elems::max_alignof);

  /* The largest sizeof(elem_t) among elems_t. */
  static constexpr size_t max_sizeof
      = max(sizeof(elem_t), for_more_elems::max_sizeof);

};  // for_elems<elem_t, more_elems_t...>

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

template <typename elem_t, typename... elems_t>
struct contains;

template <typename elem_t>
struct contains<elem_t> { static constexpr bool value = false; };

template <typename elem_t, typename other_t, typename... more_elems_t>
struct contains<elem_t, other_t, more_elems_t...> { static constexpr bool value = std::is_same<elem_t, other_t>::value || contains<elem_t, more_elems_t...>::value; };

/* TODO */
template <typename... elems_t>
class tagged_union_t {
  public:

  /* The type of visitor passed to accept(). */
  using visitor_t = tagged_union::visitor_t<elems_t...>;

  /* Default-construct as void. */
  tagged_union_t() noexcept {
    tag = &for_elem<void>::tag;
  }

  /* Move-construct from another tagged union.  The donor ends up in the
     default-constructed state. */
  tagged_union_t(tagged_union_t &&that) noexcept {
    assert(&that);
    tag = that.tag;
    that.tag = &for_elem<void>::tag;
    tag->move_construct_and_destroy(data, that.data);
  }

  /* Copy-construct from another tagged union. */
  tagged_union_t(const tagged_union_t &that) {
    assert(&that);
    tag = that.tag;
    tag->copy_construct(data, that.data);
  }

  /* Move-construct from a donor object.  The donor's type must be among our
     set of possible types. */
  template <
      typename elem_t,
      typename = typename std::enable_if<
          contains<elem_t, elems_t...>::value>::type>
  tagged_union_t(elem_t &&elem) noexcept {
    assert(&elem);
    tag = &for_elem<elem_t>::tag;
    new (data) elem_t(std::move(elem));
  }

  /* Copy-construct from a sample object.  The sample's type must be among our
     set of possible types. */
  template <
      typename elem_t,
      typename = typename std::enable_if<
          contains<elem_t, elems_t...>::value>::type>
  tagged_union_t(const elem_t &elem) {
    assert(&elem);
    tag = &for_elem<elem_t>::tag;
    new (data) elem_t(elem);
  }

  /* TODO */
  virtual ~tagged_union_t() {
    assert(this);
    tag->destroy(data);
  }

  /* TODO */
  tagged_union_t &operator=(tagged_union_t &&that) noexcept {
    assert(this);
    assert(&that);
    this->~tagged_union_t();
    return *new (this) tagged_union_t(std::move(that));
  }

  /* TODO */
  tagged_union_t &operator=(const tagged_union_t &that) {
    assert(this);
    assert(&that);
    tagged_union_t temp = that;
    return *this = std::move(temp);
  }

  /* True iff. we are contain some value; that is, when we are not in the
     default-constructed (void) state. */
  explicit operator bool() const noexcept {
    assert(this);
    return tag == &for_elem<void>::tag;
  }

  /* TODO */
  void accept(const visitor_t &visitor) const {
    assert(this);
    assert(&visitor);
    tag->accept(visitor, data);
  }

  /* Resume the default-constructed state. */
  tagged_union_t &reset() noexcept {
    assert(this);
    tag->destroy(data);
    tag = &for_elem<void>::tag;
    return *this;
  }

  private:

  /* Look up constants defined by our set of possible types, then use those
     constants to define a type for our data storage space.  The space must
     be large enough to hold the largest of our types and aligned correctly
     for the most alignment-critical of our types. */
  using for_elems = tagged_union::for_elems<elems_t...>;
  static constexpr size_t max_alignof = for_elems::max_alignof;
  static constexpr size_t max_sizeof = for_elems::max_sizeof;
  using data_t = alignas(max_alignof) char[max_sizeof];

  /* TODO */
  struct any_tag_t {
    virtual void accept(
        const visitor_t &visitor, const data_t &data) const = 0;
    virtual void copy_construct(data_t &dst, const data_t &src) const = 0;
    virtual void destroy(data_t &data) const = 0;
    virtual void move_construct_and_destroy(
        data_t &dst, data_t &src) const noexcept = 0;
  };

  /* TODO */
  template <typename elem_t, int dummy>
  struct tag_t final : any_tag_t {

    /* Interpret the data as an instance of elem_t and dispatch it to the
       appropriate handler in the given visitor. */
    virtual void accept(
        const visitor_t &visitor, const data_t &data) const override {
      visitor(as_elem(data));
    }

    /* Copy-construct an instance of elem_t into the given storage area.
       Interpret the source data as the instance of elem_t to copy from. */
    virtual void copy_construct(
        data_t &dst, const data_t &src) const override {
      new (dst) elem_t(as_elem(src));
    }

    /* Interpret the data as an instance of elem_t and destroy it in place. */
    virtual void destroy(data_t &data) const override {
      as_elem(data).~elem_t();
    }

    /* Move-construct an instance of elem_t into the given storage area.
       Interpret the source data as the instance of elem_t to move from,
       then destroy the source in place. */
    virtual void move_construct_and_destroy(
        data_t &dst, data_t &src) const noexcept override {
      elem_t &elem = as_elem(src);
      new (dst) elem_t(std::move(elem));
      elem.~elem_t();
    }

    /* Interpret the data as an instance of elem_t. */
    static elem_t &as_elem(data_t &data) {
      return reinterpret_cast<elem_t &>(data);
    }

    /* Interpret the data as an instance of elem_t. */
    static const elem_t &as_elem(const data_t &data) {
      return reinterpret_cast<const elem_t &>(data);
    }

  };  // tagged_union_t<elems_t...>::tag_t<elem_t, dummy>

  /* Provides singleton constant instances of tag_t<elem_t> to be shared by
     all instances of the tagged union. */
  template <typename elem_t>
  struct for_elem {
    static const tag_t<elem_t, 0> tag;
  };

  /* The tag which will interpret our data for us.  Never null.  This always
     points to one of the singletons declared in for_elem<elem_t>, above. */
  const any_tag_t *tag;

  /* The data to be interpreted by our current tag. */
  data_t data;

};  // tagged_union_t<elems_t...>

/* The type of tag we use for a default-constructed tagged union.  It
   specializes on void, which is otherwise not allowed to appear in a tagged
   union, and so uniquely identifies the default-constructed case. */
template <typename... elems_t>
template <int dummy>
struct tagged_union_t<elems_t...>::tag_t<void, dummy>
    : tagged_union_t<elems_t...>::any_tag_t {

  /* Pull these types in from tagged_union_t. */
  using data_t = tagged_union_t<elems_t...>::data_t;
  using visitor_t = tagged_union_t<elems_t...>::visitor_t;

  /* For void, we invoke the visitor's nullary handler. */
  virtual void accept(
      const visitor_t &visitor, const data_t &) const override {
    visitor();
  }

  /* For void, these are do-nothings. */
  virtual void destroy(data_t &data) const override {}
  virtual void copy_construct(data_t &, const data_t &) const override {}
  virtual void move_construct_and_destroy(
      data_t &, data_t &) const noexcept override {}

};  // tagged_union_t<elems_t...>::tag_t<void, dummy>

/* Defines the singleton instances of the tags. */
template <typename... elems_t>
template <typename elem_t>
const typename tagged_union_t<elems_t...>::template tag_t<elem_t, 0>
    tagged_union_t<elems_t...>::for_elem<elem_t>::tag;

}  // tagged_union
