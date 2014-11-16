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

#include "variant.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "lick.h"

using namespace std;
using namespace cppcon14::variant;

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
struct expr_t;
using expr_ptr_t = shared_ptr<expr_t>;

/* TODO */
struct val_t;
using val_ptr_t = shared_ptr<val_t>;

/* TODO */
class type_mismatch_error_t final : public runtime_error {
  public:
  type_mismatch_error_t() : runtime_error("type mismatch error") {}
};

/* TODO */
class null_value_error_t final : public runtime_error {
  public:
  null_value_error_t() : runtime_error("null value error") {}
};

/* TODO */
class undef_ref_error_t final : public runtime_error {
  public:
  undef_ref_error_t() : runtime_error("undefined ref error") {}
};

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
struct lambda_t final {
  using params_t = vector<string>;
  lambda_t(params_t &&params, const expr_ptr_t &def)
      : params(move(params)), def(move(def)) {}
  params_t params;
  expr_ptr_t def;
};

/* TODO */
struct val_t final : public variant_t<int, string, lambda_t, null_t> {
  using variant_t::variant_t;
};

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
struct unary_val_functor_t {
  using ret_t = val_t;
  val_t operator()(null_t) const { throw null_value_error_t(); }
  template <typename that_t>
  val_t operator()(const that_t &) const {
    throw type_mismatch_error_t();
  }
};

/* TODO */
struct binary_val_functor_t {
  using ret_t = val_t;
  val_t operator()(null_t, null_t) const { throw null_value_error_t(); }
  template <typename lhs_t>
  val_t operator()(const lhs_t &, null_t) const {
    throw null_value_error_t();
  }
  template <typename rhs_t>
  val_t operator()(null_t, const rhs_t &) const {
    throw null_value_error_t();
  }
  template <typename lhs_t, typename rhs_t>
  val_t operator()(const lhs_t &, const rhs_t &) const {
    throw type_mismatch_error_t();
  }
};

/* TODO */
struct neg_t final : unary_val_functor_t {
  using unary_val_functor_t::operator();
  val_t operator()(int that) const { return -that; }
};

/* TODO */
struct not_t final : unary_val_functor_t {
  using unary_val_functor_t::operator();
  val_t operator()(int that) const { return static_cast<int>(!that); }
};

/* TODO */
struct to_int_t final : unary_val_functor_t {
  using unary_val_functor_t::operator();
  val_t operator()(int that) const { return that; }
  val_t operator()(const string &that) const { return stoi(that); }
};

/* TODO */
struct to_str_t final : unary_val_functor_t {
  using unary_val_functor_t::operator();
  val_t operator()(int that) const {
    ostringstream strm;
    strm << that;
    return strm.str();
  }
  val_t operator()(const string &that) const { return that; }
};

/* TODO */
struct add_t final : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const { return lhs + rhs; }
  val_t operator()(const string &lhs, const string &rhs) const {
    return lhs + rhs;
  }
};

/* TODO */
struct mul_t final : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const { return lhs * rhs; }
  val_t operator()(const string &lhs, int rhs) const {
    ostringstream strm;
    for (int i = 0; i < rhs; ++i) {
      strm << lhs;
    }
    return strm.str();
  }
};

/* TODO */
struct and_t final : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const {
    return static_cast<int>(lhs && rhs);
  }
};

/* TODO */
struct or_t final : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const {
    return static_cast<int>(lhs || rhs);
  }
};

/* TODO */
struct lt_t final : binary_val_functor_t {
  using binary_val_functor_t::operator();
  val_t operator()(int lhs, int rhs) const {
    return static_cast<int>(lhs < rhs);
  }
  val_t operator()(const string &lhs, const string &rhs) const {
    return static_cast<int>(lhs < rhs);
  }
};

FIXTURE(val_functors) {
  EXPECT_EQ(apply(neg_t(), val_t(1)).as<int>(), -1);
  EXPECT_EQ(apply(not_t(), val_t(1)).as<int>(),  0);
  EXPECT_EQ(apply(to_int_t(), val_t(1)).as<int>(), 1);
  EXPECT_EQ(apply(to_int_t(), val_t(string("101"))).as<int>(), 101);
  EXPECT_EQ(apply(to_str_t(), val_t(101)).as<string>(), "101");
  EXPECT_EQ(apply(to_str_t(), val_t(string("hello"))).as<string>(), "hello");
  EXPECT_EQ(apply(add_t(), val_t(1), val_t(2)).as<int>(), 3);
  EXPECT_EQ(apply(add_t(), val_t(string("hello")), val_t(string("doctor"))).as<string>(), "hellodoctor");
  EXPECT_EQ(apply(mul_t(), val_t(2), val_t(3)).as<int>(), 6);
  EXPECT_EQ(apply(mul_t(), val_t(string("two")), val_t(3)).as<string>(), "twotwotwo");
  EXPECT_EQ(apply(lt_t(), val_t(1), val_t(2)).as<int>(), 1);
  EXPECT_EQ(apply(and_t(), val_t(1), val_t(1)).as<int>(), 1);
  EXPECT_EQ(apply(and_t(), val_t(0), val_t(1)).as<int>(), 0);
  EXPECT_EQ(apply(or_t(), val_t(0), val_t(0)).as<int>(), 0);
  EXPECT_EQ(apply(or_t(), val_t(0), val_t(1)).as<int>(), 1);
  EXPECT_EQ(apply(lt_t(), val_t(string("hello")), val_t(string("doctor"))).as<int>(), 0);
}

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
class scope_t final {
  public:

  /* TODO */
  scope_t(const scope_t &) = delete;
  scope_t &operator=(const scope_t &) = delete;

  /* TODO */
  using defs_t = map<string, val_t>;

  /* TODO */
  explicit scope_t(const scope_t *parent = nullptr) noexcept : parent(parent) {}

  /* TODO */
  void def(const string &name, val_t &&val) {
    assert(this);
    defs[name] = move(val);
  }

  /* TODO */
  const val_t &ref(const string &name) const {
    assert(this);
    const scope_t *scope = this;
    do {
      auto iter = scope->defs.find(name);
      if (iter != scope->defs.end()) {
        return iter->second;
      }
      scope = scope->parent;
    } while (scope);
    throw undef_ref_error_t();
  }

  private:

  /* TODO */
  const scope_t *parent;

  /* TODO */
  defs_t defs;

};  // scope_t;

FIXTURE(scope) {
  scope_t s1;
  s1.def("name", string("alice"));
  s1.def("age", 42);
  EXPECT_EQ(s1.ref("name").as<string>(), "alice");
  EXPECT_EQ(s1.ref("age").as<int>(), 42);
  scope_t s2(&s1);
  s2.def("age", 29);
  EXPECT_EQ(s2.ref("age").as<int>(), 29);
}

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
struct lit_t final {
  lit_t(const val_ptr_t &val) : val(val) {}
  val_ptr_t val;
};

/* TODO */
struct affix_t final {
  enum op_t {
    neg,
    not_,
    to_int,
    to_str
  };
  affix_t(op_t op, const expr_ptr_t &arg) : op(op), arg(arg) {}
  op_t op;
  expr_ptr_t arg;
};

/* TODO */
struct infix_t final {
  enum op_t {
    add,
    mul,
    lt,
    and_,
    or_
  };
  infix_t(op_t op, const expr_ptr_t &lhs, const expr_ptr_t &rhs)
      : op(op), lhs(lhs), rhs(rhs) {}
  op_t op;
  expr_ptr_t lhs, rhs;
};

/* TODO */
struct ref_t final {
  ref_t(string name) : name(move(name)) {}
  string name;
};

/* TODO */
struct apply_t final {
  using args_t = vector<expr_ptr_t>;
  apply_t(const expr_ptr_t &fn, args_t args) : fn(fn), args(move(args)) {}
  expr_ptr_t fn;
  args_t args;
};

/* TODO */
struct expr_t final
    : public variant_t<lit_t, affix_t, infix_t, ref_t, apply_t> {
  using variant_t::variant_t;
};

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
struct eval_t final {

  /* TODO */
  using ret_t = val_t;

  /* TODO */
  val_t operator()(nullptr_t) const { return val_t(); }

  /* TODO */
  val_t operator()(const lit_t &that) const {
    return *that.val;
  }

  /* TODO */
  val_t operator()(const affix_t &that) const {
    val_t result, arg = apply(*this, *that.arg);
    switch (that.op) {
      case affix_t::neg: {
        result = apply(neg_t(), arg);
        break;
      }
      case affix_t::not_: {
        result = apply(not_t(), arg);
        break;
      }
      case affix_t::to_int: {
        result = apply(to_int_t(), arg);
        break;
      }
      case affix_t::to_str: {
        result = apply(to_str_t(), arg);
        break;
      }
    }  // switch
    return move(result);
  }

  /* TODO */
  val_t operator()(const infix_t &that) const {
    val_t result, lhs = apply(*this, *that.lhs), rhs = apply(*this, *that.rhs);
    switch (that.op) {
      case infix_t::add: {
        result = apply(add_t(), lhs, rhs);
        break;
      }
      case infix_t::mul: {
        result = apply(mul_t(), lhs, rhs);
        break;
      }
      case infix_t::lt: {
        result = apply(lt_t(), lhs, rhs);
        break;
      }
      case infix_t::and_: {
        result = apply(and_t(), lhs, rhs);
        break;
      }
      case infix_t::or_: {
        result = apply(or_t(), lhs, rhs);
        break;
      }
    }  // switch
    return move(result);
  }

  /* TODO */
  val_t operator()(const ref_t &that) const {
    return scope->ref(that.name);
  }

  /* TODO */
  val_t operator()(const apply_t &that) const {
    /* Get the lambda we're going to apply. */
    val_t fn = apply(*this, *that.fn);
    const auto *lambda = fn.try_as<lambda_t>();
    if (!lambda || lambda->params.size() != that.args.size()) {
      throw type_mismatch_error_t();
    }
    /* Evaluate the arguments and put them into scope. */
    scope_t local_scope(scope);
    for (size_t i = 0; i < that.args.size(); ++i) {
      local_scope.def(lambda->params[i], apply(*this, *that.args[i]));
    }
    /* Evaluate the lambda's definition. */
    return apply(eval_t{&local_scope}, *lambda->def);
  }

  const scope_t *scope;

};

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
struct token_t final {
  enum kind_t {
    end,
    open_paren,
    close_paren,
    plus,
    minus,
    star,
    lt,
    eq,
    comma,
    lit,
    name,
    and_kwd,
    fn_kwd,
    int_kwd,
    not_kwd,
    or_kwd,
    str_kwd
  };
  kind_t kind;
  val_ptr_t val;
};

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
class scanner_t final {
  public:

  /* No copying or moving. */
  scanner_t(const scanner_t &) = delete;
  scanner_t &operator=(const scanner_t &) = delete;

  /* TODO */
  scanner_t(istream &strm)
      : strm(strm), token_is_cached(false) {
    assert(strm);
  }

  /* TODO */
  const token_t &operator*() const {
    assert(this);
    refresh_cache();
    return cached_token;
  }

  /* TODO */
  const token_t *operator->() const {
    assert(this);
    refresh_cache();
    return &cached_token;
  }

  /* TODO */
  scanner_t &operator++() {
    assert(this);
    refresh_cache();
    if (token_is_cached && cached_token.kind != token_t::end) {
      cached_token.val.reset();
      token_is_cached = false;
    }
    return *this;
  }

  private:

  /* TODO */
  void refresh_cache() const {
    assert(this);
    if (!token_is_cached) {
      enum { start, str_lit, int_lit, name } state = start;
      ostringstream accum;
      do {
        auto c = strm.peek();
        switch (state) {
          case start: {
            switch (c) {
              case '(': {
                strm.ignore();
                cached_token.kind = token_t::open_paren;
                token_is_cached = true;
                break;
              }
              case ')': {
                strm.ignore();
                cached_token.kind = token_t::close_paren;
                token_is_cached = true;
                break;
              }
              case '+': {
                strm.ignore();
                cached_token.kind = token_t::plus;
                token_is_cached = true;
                break;
              }
              case '-': {
                strm.ignore();
                cached_token.kind = token_t::minus;
                token_is_cached = true;
                break;
              }
              case '*': {
                strm.ignore();
                cached_token.kind = token_t::star;
                token_is_cached = true;
                break;
              }
              case '<': {
                strm.ignore();
                cached_token.kind = token_t::lt;
                token_is_cached = true;
                break;
              }
              case '=': {
                strm.ignore();
                cached_token.kind = token_t::eq;
                token_is_cached = true;
                break;
              }
              case ',': {
                strm.ignore();
                cached_token.kind = token_t::comma;
                token_is_cached = true;
                break;
              }
              case '"': {
                strm.ignore();
                state = str_lit;
                break;
              }
              default: {
                if (c < 0) {
                  cached_token.kind = token_t::end;
                  token_is_cached = true;
                  break;
                }
                if (isspace(c)) {
                  strm.ignore();
                  break;
                }
                if (isdigit(c)) {
                  state = int_lit;
                  break;
                }
                if (isalpha(c) || c == '_') {
                  state = name;
                  break;
                }
                throw;
              }
            }  // switch
            break;
          }
          case str_lit: {
            if (c < 0) {
              throw;
            }
            if (c == '"') {
              strm.ignore();
              cached_token.kind = token_t::lit;
              cached_token.val = make_shared<val_t>(accum.str());
              token_is_cached = true;
              break;
            }
            if (isprint(c)) {
              strm.ignore();
              accum.put(c);
              break;
            }
            throw;
          }
          case int_lit: {
            if (isdigit(c)) {
              strm.ignore();
              accum.put(c);
              break;
            }
            istringstream strm(accum.str());
            int temp;
            strm >> temp;
            cached_token.kind = token_t::lit;
            cached_token.val = make_shared<val_t>(temp);
            token_is_cached = true;
            break;
          }
          case name: {
            if (isalnum(c) || c == '_') {
              strm.ignore();
              accum.put(c);
              break;
            }
            string name = accum.str();
            static const map<string, token_t::kind_t> kwds = {
              { "and", token_t::and_kwd },
              { "fn", token_t::fn_kwd },
              { "int", token_t::int_kwd },
              { "not", token_t::not_kwd },
              { "or", token_t::or_kwd },
              { "str", token_t::str_kwd }
            };
            auto iter = kwds.find(name);
            if (iter != kwds.end()) {
              cached_token.kind = iter->second;
            } else {
              cached_token.kind = token_t::name;
              cached_token.val = make_shared<val_t>(accum.str());
            }
            token_is_cached = true;
            break;
          }
        }  // switch
      } while (!token_is_cached);
    }  // if
  }

  /* TODO */
  istream &strm;

  /* TODO */
  mutable bool token_is_cached;

  /* TODO */
  mutable token_t cached_token;

};  // scanner_t

FIXTURE(scanner) {
  istringstream strm("1 + 2");
  scanner_t scanner(strm);
  if (EXPECT_TRUE(scanner->kind == token_t::lit)) {
    EXPECT_EQ(scanner->val->as<int>(), 1);
  }
  ++scanner;
  EXPECT_TRUE(scanner->kind == token_t::plus);
  ++scanner;
  if (EXPECT_TRUE(scanner->kind == token_t::lit)) {
    EXPECT_EQ(scanner->val->as<int>(), 2);
  }
  ++scanner;
  EXPECT_TRUE(scanner->kind == token_t::end);
}

/* ---------------------------------------------------------------------------
TODO
--------------------------------------------------------------------------- */

/* TODO */
class expr_parser_t final {
  public:

  /* TODO */
  static expr_ptr_t parse(scanner_t &scanner) {
    return expr_parser_t(scanner).parse_expr();
  }

  private:

  /* TODO */
  expr_parser_t(scanner_t &scanner)
      : scanner(scanner) {}

  /* TODO */
  expr_ptr_t parse_and() {
    assert(this);
    expr_ptr_t result = parse_not();
    while (scanner->kind == token_t::and_kwd) {
      ++scanner;
      result = make_shared<expr_t>(
          infix_t(infix_t::or_, result, parse_not()));
    }
    return result;
  }

  /* TODO */
  expr_ptr_t parse_arith() {
    assert(this);
    expr_ptr_t result = parse_term();
    while (scanner->kind == token_t::plus) {
      ++scanner;
      result = make_shared<expr_t>(
          infix_t(infix_t::add, result, parse_term()));
    }
    return result;
  }

  /* TODO */
  expr_ptr_t parse_atom() {
    assert(this);
    expr_ptr_t result;
    switch (scanner->kind) {
      case token_t::lit: {
        result = make_shared<expr_t>(lit_t(scanner->val));
        ++scanner;
        break;
      }
      case token_t::name: {
        result = make_shared<expr_t>(ref_t(scanner->val->as<string>()));
        ++scanner;
        break;
      }
      case token_t::open_paren: {
        ++scanner;
        result = parse_expr();
        match(token_t::close_paren);
        break;
      }
      default: {
        throw;
      }
    }  // switch
    return result;
  }

  /* TODO */
  expr_ptr_t parse_cmp() {
    assert(this);
    expr_ptr_t result = parse_arith();
    if (scanner->kind == token_t::lt) {
      ++scanner;
      result = make_shared<expr_t>(
          infix_t(infix_t::lt, result, parse_arith()));
    }
    return result;
  }

  /* TODO */
  expr_ptr_t parse_expr() {
    assert(this);
    expr_ptr_t result;
    switch (scanner->kind) {
      case token_t::fn_kwd: {
        ++scanner;
        lambda_t::params_t params;
        for (;;) {
          if (scanner->kind == token_t::eq) {
            ++scanner;
            break;
          }
          if (scanner->kind != token_t::name) {
            throw;
          }
          params.emplace_back(scanner->val->as<string>());
          ++scanner;
          match(token_t::comma);
        }
        result = make_shared<expr_t>(
            lit_t(make_shared<val_t>(
                lambda_t(move(params), parse_expr()))));
        break;
      }
      default: {
        result = parse_or();
      }
    }
    return result;
  }

  /* TODO */
  expr_ptr_t parse_factor() {
    assert(this);
    bool flag = false;
    for (;;) {
      if (scanner->kind == token_t::plus) {
        ++scanner;
        continue;
      }
      if (scanner->kind == token_t::minus) {
        ++scanner;
        flag = !flag;
        continue;
      }
      break;
    }
    expr_ptr_t result = parse_atom();
    if (flag) {
      result = make_shared<expr_t>(affix_t(affix_t::neg, result));
    }
    return result;
  }

  /* TODO */
  expr_ptr_t parse_not() {
    assert(this);
    bool flag = false;
    while (scanner->kind == token_t::not_kwd) {
      ++scanner;
      flag = !flag;
    }
    expr_ptr_t result = parse_cmp();
    if (flag) {
      result = make_shared<expr_t>(affix_t(affix_t::not_, result));
    }
    return result;
  }

  /* TODO */
  expr_ptr_t parse_or() {
    assert(this);
    expr_ptr_t result = parse_and();
    while (scanner->kind == token_t::or_kwd) {
      ++scanner;
      result = make_shared<expr_t>(
          infix_t(infix_t::or_, result, parse_and()));
    }
    return result;
  }

  /* TODO */
  expr_ptr_t parse_term() {
    assert(this);
    expr_ptr_t result = parse_factor();
    while (scanner->kind == token_t::star) {
      ++scanner;
      result = make_shared<expr_t>(
          infix_t(infix_t::mul, result, parse_factor()));
    }
    return result;
  }

  private:

  /* TODO */
  void match(token_t::kind_t kind) {
    assert(this);
    if (scanner->kind != kind) {
      throw;
    }
    ++scanner;
  }

  /* TODO */
  scanner_t &scanner;

};  // expr_parser_t

/* TODO */
inline expr_ptr_t parse_expr(const string &text) {
  istringstream strm(text);
  scanner_t scanner(strm);
  return expr_parser_t::parse(scanner);
}

/* TODO */
inline val_t eval(const string &text) {
  scope_t scope;
  return apply(eval_t{&scope}, *parse_expr(text));
}

/* TODO */
inline string eval_as_str(const string &text) {
  return apply(to_str_t(), eval(text)).as<string>();
}

FIXTURE(parse_expr) {
  EXPECT_EQ(eval_as_str("1 + 2"), "3");
}

