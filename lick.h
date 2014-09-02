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
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/* A macro to define a fixture. */
#ifndef FIXTURE
#define FIXTURE(name)  \
    static void name##_func();  \
    static const ::cppcon14::lick::fixture_t  \
        name##_fixture(__LINE__, #name, name##_func);  \
    static void name##_func()
#endif

/* Macros to construct a expectations. */
#ifndef EXPECT_EQ
#define EXPECT_EQ(lhs, rhs) ::cppcon14::lick::expect_t(  \
    __LINE__, #lhs, lhs, ::cppcon14::lick::expect_t::eq, #rhs, rhs)
#endif
#ifndef EXPECT_NE
#define EXPECT_NE(lhs, rhs) ::cppcon14::lick::expect_t(  \
    __LINE__, #lhs, lhs, ::cppcon14::lick::expect_t::ne, #rhs, rhs)
#endif
#ifndef EXPECT_LT
#define EXPECT_LT(lhs, rhs) ::cppcon14::lick::expect_t(  \
    __LINE__, #lhs, lhs, ::cppcon14::lick::expect_t::lt, #rhs, rhs)
#endif
#ifndef EXPECT_LE
#define EXPECT_LE(lhs, rhs) ::cppcon14::lick::expect_t(  \
    __LINE__, #lhs, lhs, ::cppcon14::lick::expect_t::le, #rhs, rhs)
#endif
#ifndef EXPECT_GT
#define EXPECT_GT(lhs, rhs) ::cppcon14::lick::expect_t(  \
    __LINE__, #lhs, lhs, ::cppcon14::lick::expect_t::gt, #rhs, rhs)
#endif
#ifndef EXPECT_GE
#define EXPECT_GE(lhs, rhs) ::cppcon14::lick::expect_t(  \
    __LINE__, #lhs, lhs, ::cppcon14::lick::expect_t::ge, #rhs, rhs)
#endif
#ifndef EXPECT_TRUE
#define EXPECT_TRUE(arg) ::cppcon14::lick::expect_t(  \
    __LINE__, ::cppcon14::lick::expect_t::pt, #arg, arg)
#endif
#ifndef EXPECT_FALSE
#define EXPECT_FALSE(arg) ::cppcon14::lick::expect_t(  \
    __LINE__, ::cppcon14::lick::expect_t::pf, #arg, arg)
#endif

namespace cppcon14 {
namespace lick {

/* Manages the single instance of a class (obj_t) with singleton semantics.
   We don't create or destroy this instance, but we do keep track of it
   while it exists, preventing more than one instance from existing at a
   time. */
template <typename obj_t>
class singleton_t final {
  public:

  /* No construction, copying, or moving. */
  singleton_t() = delete;
  singleton_t(const singleton_t &) = delete;
  singleton_t &operator=(const singleton_t &) = delete;

  /* The current instance of obj_t. Never null. */
  static obj_t *get_instance() noexcept {
    assert(instance);
    return instance;
  }

  /* Called by the constructor of obj_t to register the creation of the
     singleton instance. */
  static void on_construction(obj_t *obj) {
    assert(obj);
    assert(!instance);
    instance = obj;
  }

  /* Called by the destructor of obj_t to register the destruction of the
     singleton instance. */
  static void on_destruction(obj_t *obj) {
    assert(obj);
    assert(instance == obj);
    instance = nullptr;
  }

  private:

  /* The single instance of obj_t, or null if there is none right now. */
  static obj_t *instance;

};  // singleton_t<obj_t>

/* See declaration. */
template <typename obj_t>
obj_t *singleton_t<obj_t>::instance = nullptr;

/* A node in a linked list of instances of obj_t in the data segment.  We
   don't create or destroy the instances of obj_t, we just maintain a
   linked list of them.  The intent is for an instance of this class to be
   aggregated within obj_t and to live along with as constant data it in the
   data segment. */
template <typename obj_t>
class static_list_node_t final {
  public:

  /* No copying or moving. */
  static_list_node_t(const static_list_node_t &) = delete;
  static_list_node_t &operator=(const static_list_node_t &) = delete;

  /* Caches the pointer to the object and links this node to the end of
     the list. */
  explicit static_list_node_t(const obj_t *obj)
      : obj(obj), next_node(nullptr) {
    assert(obj);
    (last_node ? last_node->next_node : first_node) = this;
    last_node = this;
  }

  /* This is an explicit do-nothing.  As we assume the instances of obj_t
     and static_list_node_t are in the data segment, we don't have to clean
     them up. */
  ~static_list_node_t() {}

  /* Call back for each instance of obj_t. */
  static bool for_each_instance(
      const std::function<bool (const obj_t *)> &cb) {
    bool ok = true;
    for (auto *node = first_node; node && ok; node = node->next_node) {
      ok = cb(node->obj);
    }
    return ok;
  }

  private:

  /* The object kept at this point in the linked list.  Never null. */
  const obj_t *obj;

  /* The next node in the linked list. */
  static_list_node_t *next_node;

  /* The first and last nodes in the linked list. */
  static static_list_node_t *first_node, *last_node;

};  // static_list_node_t<obj_t>

/* See declarations. */
template <typename obj_t>
static_list_node_t<obj_t> *static_list_node_t<obj_t>::first_node = nullptr;
template <typename obj_t>
static_list_node_t<obj_t> *static_list_node_t<obj_t>::last_node = nullptr;

/* TODO */
template <typename elem_t>
class opt_t final {
  public:

  /* TODO */
  opt_t() noexcept
      : known(false) {}

  /* TODO */
  opt_t(elem_t that) noexcept {
    new (&elem) elem_t(std::move(that));
    known = true;
  }

  /* TODO */
  opt_t(opt_t &&that) noexcept {
    assert(&that);
    if (that.known) {
      new (&elem) elem_t(std::move(that.elem));
      that.elem.~elem_t();
    }
    known = that.known;
    that.known = false;
  }

  /* TODO */
  opt_t(const opt_t &that) {
    assert(&that);
    if (that.known) {
      new (&elem) elem_t(that.elem);
    }
    known = that.known;
  }

  /* TODO */
  ~opt_t() {
    assert(this);
    if (known) {
      elem.~elem_t();
    }
  }

  /* TODO */
  operator bool () const noexcept {
    assert(this);
    return known;
  }

  /* TODO */
  opt_t &operator=(opt_t &&that) noexcept {
    assert(this);
    assert(&that);
    this->~opt_t();
    new (this) opt_t(std::move(that));
    return *this;
  }

  /* TODO */
  opt_t &operator=(const opt_t &that) noexcept {
    assert(this);
    assert(&that);
    return *this = opt_t(that);
  }

  /* TODO */
  const elem_t &get(const elem_t &def_elem) const noexcept {
    assert(this);
    assert(&def_elem);
    return known ? elem : def_elem;
  }

  /* TODO */
  bool try_set(elem_t that) {
    assert(this);
    bool result;
    if (!known) {
      new (&elem) elem_t(std::move(that));
      result = true;
    } else {
      result = false;
    }
    return result;
  }

  private:

  /* TODO */
  bool known;

  /* TODO */
  union {
    elem_t elem;
  };

};  // opt_t

/* TODO */
struct config_t final {

  /* TODO */
  enum output_t { to_stdout, to_stderr, to_nowhere };

  /* TODO */
  enum style_t { terse, verbose };

  /* TODO */
  struct channel_t final {

    /* TODO */
    std::ostream *get_strm() const noexcept {
      assert(this);
      return config_t::get_strm(output);
    }

    /* TODO */
    output_t output;

    /* TODO */
    style_t style;

  };  // config_t::channel_t

  /* A reasonable default configuration. */
  config_t() noexcept
      : prog_name(nullptr),
        pass_channel({ to_stdout, terse }),
        fail_channel({ to_stdout, verbose }),
        summary_output(to_stdout), use_color(true) {}

  /* Terminal codes for red text. */
  const char *red() const noexcept {
    assert(this);
    return use_color ? "\033[1;31m" : "";
  }

  /* Terminal codes for green text. */
  const char *green() const noexcept {
    assert(this);
    return use_color ? "\033[1;32m" : "";
  }

  /* Terminal codes for plain (that is, not red or green) text. */
  const char *plain() const noexcept {
    assert(this);
    return use_color ? "\033[0m" : "";
  }

  /* TODO */
  std::ostream *get_summary_strm() const noexcept {
    assert(this);
    return get_strm(summary_output);
  }

  /* TODO */
  static std::ostream *get_strm(output_t output) noexcept {
    std::ostream *result;
    switch (output) {
      case to_stdout: {
        result = &std::cout;
        break;
      }
      case to_stderr: {
        result = &std::cerr;
        break;
      }
      case to_nowhere: {
        result = nullptr;
        break;
      }
    }
    return result;
  }

  /* TODO */
  const char *prog_name;

  /* TODO */
  channel_t pass_channel, fail_channel;

  /* TODO */
  output_t summary_output;

  /* TODO */
  bool use_color;

  /* TODO */
  std::unique_ptr<std::regex> fixture_name_regex;

};  // config_t

/* A single expectation within a fixture, like "EXPECT_EQ(a, b)".  The
   expectation object should be constructed as a temporary and only while a
   an instance of outcome collector exists.  An expectation object will
   implicitly cast to bool, whether the expectation was met.  While the
   temporary expectation object exists, it can also be the target of
   insertion operators, allowing you to write additional descriptive text
   into the expectation. */
class expect_t final {
  public:

  /* No copying or moving. */
  expect_t(const expect_t &) = delete;
  expect_t &operator=(const expect_t &) = delete;

  /* The actual outcome of an expectation.  This class keeps most of its
     data in public fields, as it mostly just records stuff as it
     happens. */
  class outcome_t final {
    public:

    /* Shallow-copying is ok. */
    outcome_t(const outcome_t &) = default;
    outcome_t &operator=(const outcome_t &) = default;

    /* Collects outcomes.  This class enforces singleton semantics.  Make
       sure there is an instance of this class before you try constructing
       any expectations. */
    class collector_t {
      public:

      /* No copying or moving. */
      collector_t(const collector_t &) = delete;
      collector_t &operator=(const collector_t &) = delete;

      /* Convenience. */
      using singleton_t = lick::singleton_t<collector_t>;

      /* TODO */
      collector_t()
          : ok(true) {
        singleton_t::on_construction(this);
      }

      /* TODO */
      ~collector_t() {
        singleton_t::on_destruction(this);
      }

      /* Collect the given outcome and updating our overall success
         accordingly. */
      void on_outcome(std::unique_ptr<outcome_t> &&outcome) {
        assert(this);
        assert(outcome);
        ok = ok && outcome->ok;
        outcomes.emplace_back(std::move(outcome));
      }

      /* TODO */
      void on_exception() {
        assert(this);
        ok = false;
      }

      /* TODO */
      void on_exception(const std::exception &) {
        assert(this);
        ok = false;
      }

      /* True iff. no expectation has failed. */
      operator bool() const noexcept {
        assert(this);
        return ok;
      }

      /* TODO */
      void report(std::ostream &strm, const config_t &config) const {
        assert(this);
        for (const auto &outcome: outcomes) {
          outcome->report(strm, config);
        }
      }

      /* TODO */
      static collector_t *get_instance() noexcept {
        return singleton_t::get_instance();
      }

      private:

      /* TODO */
      bool ok;

      /* TODO */
      std::vector<std::unique_ptr<outcome_t>> outcomes;

    };  // expect_t::outcome_t::collector_t

    /* Cache the source line number, as it can't change, and assume the
       expectation will not be met. */
    explicit outcome_t(int line) noexcept
        : line(line), ok(false) {
      assert(line > 0);
    }

    /* Write a human-readable report. */
    void report(std::ostream &strm, const config_t &config) const {
      assert(this);
      assert(&strm);
      assert(&config);
      if (ok) {
        strm << config.green() << "  [ " << std::setw(4) << line
             << ", pass: " << src << " ]" << config.plain() << std::endl;
      } else {
        strm << config.red() << "  [ " << std::setw(4) << line
             << ", fail: " << src << " ]" << config.plain() << std::endl
             << "          expr: " << expr << std::endl;
        if (!desc.empty()) {
          strm << "          desc: " << desc << std::endl;
        }
      }
    }

    /* The 1-based line number within the fixture at which the expectation
       was constructed.  This is constant because it really won't ever
       change. */
    const int line;

    /* True iff. the expectation was met. */
    bool ok;

    /* The source text of the expectation, like "EXPECT_EQ(a, b)". */
    std::string src;

    /* The expression evaluated to determine if the expectation was met,
       like "101 == 202". */
    std::string expr;

    /* Additional descriptive text inserted into the (temporary) expectation
       object while it exists. */
    std::string desc;

  };  // expect_t::outcome_t

  /* The infix and prefix operators used in our constructors. */
  enum infix_op_t { eq, ne, lt, le, gt, ge };
  enum prefix_op_t { pt, pf };

  /* Construct an infix-style text, "like EXPECT_EQ(a, b)".  We record the
     line number at which the expectation is constructed, the source code
     and expressions used on its left- and right-hand sides, and the infix
     operator giving the expected condition. */
  template <typename lhs_t, typename rhs_t>
  expect_t(
      int line,
      const char *lhs_src, const lhs_t &lhs_expr,
      infix_op_t op,
      const char *rhs_src, const rhs_t &rhs_expr)
      : expect_t(line) {
    /* Interpret the operator. */
    const char *src, *op_expr;
    switch (op) {
      case eq:
        outcome->ok = (lhs_expr == rhs_expr);
        src = "EQ"; op_expr = " == ";
        break;
      case ne:
        outcome->ok = (lhs_expr != rhs_expr);
        src = "NE"; op_expr = " != ";
        break;
      case lt:
        outcome->ok = (lhs_expr < rhs_expr);
        src = "LT"; op_expr = " < ";
        break;
      case le:
        outcome->ok = (lhs_expr <= rhs_expr);
        src = "LE"; op_expr = " <= ";
        break;
      case gt:
        outcome->ok = (lhs_expr > rhs_expr);
        src = "GT"; op_expr = " > ";
        break;
      case ge:
        outcome->ok = (lhs_expr >= rhs_expr);
        src = "GE"; op_expr = " >= ";
        break;
    }
    /* Build the outcome strings. */
    strm << "EXPECT_" << src << '(' << lhs_src << ", " << rhs_src << ')';
    outcome->src = take_str();
    strm << lhs_expr << op_expr << rhs_expr;
    outcome->expr = take_str();
  }

  /* Construct a prefix-style text, "like EXPECT_TRUE(a)".  We record the
     line number at which the expectation is constructed, the prefix
     operator giving the expected condition, and the source code and
     expression used as the argument. */
  template <typename arg_t>
  expect_t(
      int line,
      prefix_op_t op, const char *arg_text, const arg_t &arg_expr)
      : expect_t(line) {
    /* Interpret the operator. */
    const char *src, *op_expr;
    switch (op) {
      case pt:
        outcome->ok = static_cast<bool>(arg_expr);
        src = "TRUE"; op_expr = "";
        break;
      case pf:
        outcome->ok = !static_cast<bool>(arg_expr);
        src = "FALSE"; op_expr = "!";
        break;
    }
    /* Build the outcome strings. */
    strm << "EXPECT_" << src << '(' << arg_text << ')';
    outcome->src = take_str();
    strm << op_expr << arg_expr;
    outcome->expr = take_str();
  }

  /* As we go, we finalize our outcome and hand it to the current outcome
     collector. */
  ~expect_t() {
    assert(this);
    outcome->desc = take_str();
    outcome_t::collector_t::get_instance()->on_outcome(std::move(outcome));
  }

  /* True iff. the expectation passed. */
  operator bool() const noexcept {
    assert(this);
    return outcome->ok;
  }

  /* We use this as a string-builder for the (optional) description of the
     expectation.  See the operator<< overload declared after the close
     of this namespace. */
  std::ostream &get_strm() noexcept {
    assert(this);
    return strm;
  }

  private:

  /* The other constructors delegate their common work to this one. */
  explicit expect_t(int line)
      : outcome(new outcome_t(line)) {}

  /* Take a copy of the string currently built in strm, then reset strm to
     its starting state so we can use it to build another string. */
  std::string take_str() {
    assert(this);
    auto temp = strm.str();
    strm.str(std::string());
    strm.clear();
    return std::move(temp);
  }

  /* The outcome of this expectation.  We construct this during our own
     constructor, then finalize it and move it to the collector during our
     destructor. */
  std::unique_ptr<outcome_t> outcome;

  /* See accessor.  We use this as a string-builder during construction,
     for the outcome src and expr strings.  After construction, we use
     it to collect the outcome desc string. */
  std::ostringstream strm;

};  // expect_t

/* TODO */
using collector_t = expect_t::outcome_t::collector_t;

/* Meta-data providing the name and location of a function which will
   conduct QA experiments.  We expect this class to be instantiated only as
   constants in the data segment. */
class fixture_t final {
  public:

  /* No copying or moving. */
  fixture_t(const fixture_t &) = delete;
  fixture_t &operator=(const fixture_t &) = delete;

  /* The signature of a text fixture function. */
  using func_t = void (*)();

  /* This helper maintains a list of instances of fixtures. */
  using static_list_node_t = lick::static_list_node_t<fixture_t>;

  /* Link to the end of the list of instances and cache the construction
     parameters. */
  fixture_t(int line, const char *name, func_t func)
      : node(this), line(line), name(name), func(func) {
    assert(line > 0);
    assert(name);
    assert(*name);
    assert(func);
  }

  /* Run the fixture. */
  void operator()() const noexcept {
    assert(this);
    try {
      func();
    } catch (const std::exception &ex) {
      collector_t::get_instance()->on_exception(ex);
    } catch (...) {
      collector_t::get_instance()->on_exception();
    }
  }

  /* The name of the fixture.  Never null or empty. */
  const char *get_name() const noexcept {
    assert(this);
    return name;
  }

  /* TODO */
  void report(std::ostream &strm, const config_t &config, bool ok) const {
    assert(this);
    assert(&strm);
    assert(&config);
    if (ok) {
      strm << config.green()
           << "[ " << std::setw(4) << line << ", pass: " << name << " ]"
           << config.plain() << std::endl;
    } else {
      strm << config.red()
           << "[ " << std::setw(4) << line << ", fail: " << name << " ]"
           << config.plain() << std::endl;
    }
  }

  private:

  /* Our node in the static linked list of instances of fixtures. */
  static_list_node_t node;

  /* The line number within the source at which this fixture is defined. */
  int line;

  /* See accessor. */
  const char *name;

  /* The function to run for this fixture.  Never null. */
  func_t func;

};  // fixture_t

/* TODO */
using fixtures_t = fixture_t::static_list_node_t;

/* TODO */
class driver_t final {
  public:

  /* No copying or moving. */
  driver_t(const driver_t &) = delete;
  driver_t &operator=(const driver_t &) = delete;

  /* TODO */
  static int drive(const config_t &config) {
    return driver_t(config)();
  }

  private:

  /* TODO */
  explicit driver_t(const config_t &config)
      : config(config), passed(0), failed(0), skipped(0) {}

  /* TODO */
  ~driver_t() {}

  /* Visit each fixture with on_fixture(), counting the number passed,
     failed, and skipped.  Report to cout and/or cerr as specified in the
     config.  Return an OS exit code. */
  int operator()() {
    assert(this);
    /* Visit each fixture with on_fixture(), counting the number passed,
       failed, and skipped.  Report on each as per config. */
    fixtures_t::for_each_instance(
        [this](const fixture_t *fixture) {
          on_fixture(fixture);
          return true;
        }
    );
    auto *strm = config.get_summary_strm();
    /* Write the summary report, as per config. */
    if (strm) {
      if (passed) {
        (*strm) << config.green();
      }
      (*strm) << "passed: " << passed << config.plain() << ", ";
      if (failed) {
        (*strm) << config.red();
      }
      (*strm) << "failed: " << failed << config.plain()
              << ", skipped: " << skipped << std::endl;
    }
    /* Return failure only if at least one fixture failed.  This means that
       set of no fixtures (or no fixtures matching the regex) never fails. */
    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  /* TODO */
  void on_fixture(const fixture_t *fixture) {
    assert(this);
    assert(fixture);
    /* If we're limited fixtures with names matching a regex and this one
       doesn't match, skip it. */
    if (config.fixture_name_regex &&
        !std::regex_match(fixture->get_name(), *config.fixture_name_regex)) {
      ++skipped;
      return;
    }
    /* Construct an outcome collector and run the fixture, noting whether it
       passed or failed. */
    collector_t collector;
    (*fixture)();
    const config_t::channel_t *channel;
    if (collector) {
      channel = &config.pass_channel;
      ++passed;
    } else {
      channel = &config.fail_channel;
      ++failed;
    }
    /* Write the fixture report, as per config. */
    auto *strm = channel->get_strm();
    if (strm) {
      switch (channel->style) {
        case config_t::terse: {
          fixture->report(*strm, config, collector);
          break;
        }
        case config_t::verbose: {
          fixture->report(*strm, config, collector);
          collector.report(*strm, config);
          break;
        }
      }
    }
  }

  /* TODO */
  const config_t &config;

  /* TODO */
  size_t passed, failed, skipped;

};  // driver_t

/* TODO */
class parser_t final {
  public:

  /* No copying or moving. */
  parser_t(const parser_t &) = delete;
  parser_t &operator=(const parser_t &) = delete;

  /* TODO */
  using msgs_t = std::vector<std::string>;

  /* TODO */
  class error_t final
      : public std::exception {
    public:

    /* TODO */
    error_t(msgs_t &&msgs)
        : msgs(std::move(msgs)) {
      assert(!this->msgs.empty());
    }

    /* TODO */
    virtual const char *what() const noexcept {
      assert(this);
      if (what_cache.empty()) {
        std::ostringstream strm;
        report(strm, " ", ", ");
        what_cache = strm.str();
      }
      return what_cache.c_str();
    }

    /* TODO */
    void report(
        std::ostream &strm,
        const char *first_sep, const char *other_seps) const {
      assert(this);
      assert(first_sep);
      assert(other_seps);
      strm << "error(s) parsing config:";
      const char *sep = first_sep;
      for (const auto &msg: msgs) {
        strm << sep << msg;
        sep = other_seps;
      }
    }

    private:

    /* TODO */
    msgs_t msgs;

    /* TODO */
    mutable std::string what_cache;

  };  // parser_t::error_t

  /* TODO */
  static std::unique_ptr<config_t> parse(int argc, char *argv[]) {
    return parser_t(argc, argv)();
  }

  private:

  /* Conditional compilation a-go-go!  When it comes to identifying a
     command line option, Windows goes its own way. */
  #ifdef __WIN32__
    static constexpr char opt_intro = '/';
  #else
    static constexpr char opt_intro = '-';
  #endif

  /* TODO */
  using opt_handler_t = void (parser_t::*)(char, const char *);

  /* Borrow these types from config_t. */
  using output_t = config_t::output_t;
  using style_t = config_t::style_t;

  /* TODO */
  class msg_logger_t final {
    public:

    /* No copying or moving. */
    msg_logger_t(const msg_logger_t &) = delete;
    msg_logger_t &operator=(const msg_logger_t &) = delete;

    /* TODO */
    explicit msg_logger_t(parser_t *parser)
        : parser(parser) {
      assert(parser);
    }

    /* TODO */
    ~msg_logger_t() {
      assert(this);
      parser->msgs.emplace_back(strm.str());
    }

    /* TODO */
    std::ostream &operator*() {
      assert(this);
      return strm;
    }

    private:

    /* TODO */
    parser_t *parser;

    /* TODO */
    std::ostringstream strm;

  };  // config_t::msg_logger_t;

  /* TODO */
  parser_t(int argc, char *argv[])
      : opt_handlers({
          { 'c', &parser_t::on_color_opt },
          { 'f', &parser_t::on_fail_opt },
          { 'n', &parser_t::on_name_opt },
          { 'p', &parser_t::on_pass_opt },
          { 's', &parser_t::on_summary_opt }
        }), args_cursor(argv), args_limit(argv + argc) {}

  /* TODO */
  ~parser_t() {}

  /* TODO */
  std::unique_ptr<config_t> operator()() {
    assert(this);
    /* The first arg is the name of the program. */
    std::unique_ptr<config_t> config(new config_t);;
    config->prog_name = pop_arg();
    /* Process the remaining args. */
    bool ignoring_opts = false;
    for (;;) {
      /* Pop the next arg.  If there isn't one, we're done. */
      const char *arg = pop_arg();
      if (!arg) {
        break;
      }
      /* Skip an empty arg, if we somehow got one. */
      if (!*arg) {
        continue;
      }
      /* We don't take positional args, so any non-opt arg is just junk. */
      if (ignoring_opts || *arg != opt_intro) {
        on_arg(arg);
        continue;
      }
      /* Skip any repeats of the opt intro char. */
      do {
        ++arg;
      } while (*arg && *arg == opt_intro);
      /* If the arg was nothing but intro chars, it means to treat the rest
         of the args as non-opts. */
      char opt = *arg++;
      if (!opt) {
        ignoring_opts = true;
        continue;
      }
      /* If this is a cry for help, show help and bail out of parsing. */
      if (opt == '?') {
        show_usage(*config);
        return nullptr;
      }
      /* Look up the handler for the opt. */
      auto iter = opt_handlers.find(opt);
      if (iter != opt_handlers.end()) {
        /* Skip any equals signs after the opt char. */
        for (; *arg && *arg == '='; ++arg);
        /* Hand-off to the handler. */
        (this->*(iter->second))(opt, arg);
      } else {
        /* No handler?  This opt is bogus. */
        (*msg_logger_t(this)) << "unknown option '" << opt << '\'';
      }
    }  // for
    /* If we logged any error messages, package them up into an exception
       and throw. */
    if (!msgs.empty()) {
      throw error_t(std::move(msgs));
    }
    /* Fill in the config with the opts we've parsed. */
    if (use_color) {
      config->use_color = *use_color;
    }
    config->fixture_name_regex = std::move(fixture_name_regex);
    return std::move(config);
  }

  /* TODO */
  void log_conflicting_opt(char opt) {
    assert(this);
    (*msg_logger_t(this)) << "conflicting option '" << opt << '\'';
  }

  /* TODO */
  void on_arg(const char *arg) {
    assert(this);
    assert(arg);
    (*msg_logger_t(this)) << "extra argument \"" << arg << '"';
  }

  /* TODO */
  void on_color_opt(char opt, const char *arg) {
    assert(this);
    arg = try_get_opt_arg(opt, arg, false);
    try_parse_bool_opt("color", arg, use_color);
  }

  /* TODO */
  void on_fail_opt(char opt, const char *arg) {
    assert(this);
    arg = try_get_opt_arg(opt, arg);
    if (!arg) {
      return;
    }
  }

  /* TODO */
  void on_name_opt(char opt, const char *arg) {
    assert(this);
    arg = try_get_opt_arg(opt, arg);
    if (!arg) {
      return;
    }
    if (fixture_name_regex) {
      log_conflicting_opt(opt);
      return;
    }
    try {
      fixture_name_regex.reset(new std::regex(arg));
    } catch (const std::exception &) {
      (*msg_logger_t(this))
          << "bad fixture name pattern \"" << arg << '"';
    }
  }

  /* TODO */
  void on_pass_opt(char opt, const char *arg) {
    assert(this);
    arg = try_get_opt_arg(opt, arg);
    if (!arg) {
      return;
    }
  }

  /* TODO */
  void on_summary_opt(char opt, const char *arg) {
    assert(this);
    arg = try_get_opt_arg(opt, arg);
    if (!arg) {
      return;
    }
  }

  /* TODO */
  const char *pop_arg() {
    assert(this);
    return (args_cursor < args_limit) ? *args_cursor++ : nullptr;
  }

  /* TODO */
  static void show_usage(const config_t &config) {
    assert(&config);
    std::cout << "usage: " << config.prog_name << std::endl;
  }

  /* TODO */
  const char *try_get_opt_arg(
      char opt, const char *arg, bool is_required = true) {
    assert(this);
    /* If we already have a non-empty arg string, use it. */
    if (arg && *arg) {
      return arg;
    }
    /* Peek at the next arg, if there is one.  If it's a non-arg, we'll
       use it. */
    if (args_cursor < args_limit && **args_cursor != opt_intro) {
      return *args_cursor++;
    }
    /* There's no arg available.  If one was required, log the error. */
    if (is_required) {
      (*msg_logger_t(this))
          << "missing argument for option '" << opt << '\'';
    }
    return nullptr;
  }

  /* TODO */
  bool try_parse_bool_opt(
      const char *desc,
      const char *&opt, std::unique_ptr<bool> &ptr) {
    assert(this);
    assert(desc);
    assert(&ptr);
    bool temp, success;
    if (opt && *opt) {
      switch (*opt) {
        case 't': {
          temp = true;
          success = true;
          break;
        }
        case 'f': {
          temp = false;
          success = true;
          break;
        }
        default: {
          temp = false;
          success = false;
        }
      }
    } else {
      temp = true;
      success = true;
    }
    if (success) {
      if (!ptr) {
        ptr.reset(new bool(temp));
      } else if (*ptr != temp) {
        (*msg_logger_t(this))
            << "conflicting " << desc << " option";
      }
      ++opt;
    }
    return success;
  }

  /* TODO */
  bool try_parse_output_opt(
      const char *desc,
      const char *&opt, std::unique_ptr<output_t> &ptr) {
    assert(this);
    assert(desc);
    assert(&opt);
    assert(&ptr);
    output_t temp;
    bool success;
    switch (*opt) {
      case 'o': {
        temp = config_t::to_stdout;
        success = true;
        break;
      }
      case 'e': {
        temp = config_t::to_stderr;
        success = true;
        break;
      }
      case 'x': {
        temp = config_t::to_nowhere;
        success = true;
        break;
      }
      default: {
        temp = config_t::to_nowhere;
        success = false;
      }
    }
    if (success) {
      if (!ptr) {
        ptr.reset(new output_t(temp));
      } else if (*ptr != temp) {
        (*msg_logger_t(this))
            << "conflicting " << desc << " option '" << *opt << '\'';
      }
      ++opt;
    }
    return success;
  }

  /* TODO */
  bool try_parse_style_opt(
      const char *desc,
      const char *&opt, std::unique_ptr<style_t> &ptr) {
    assert(this);
    assert(desc);
    assert(&opt);
    assert(&ptr);
    style_t temp;
    bool success;
    switch (*opt) {
      case 't': {
        temp = config_t::terse;
        success = true;
        break;
      }
      case 'v': {
        temp = config_t::verbose;
        success = true;
        break;
      }
      default: {
        temp = config_t::verbose;
        success = false;
      }
    }
    if (success) {
      if (!ptr) {
        ptr.reset(new style_t(temp));
      } else if (*ptr != temp) {
        (*msg_logger_t(this))
            << "conflicting " << desc << " option '" << *opt << '\'';
      }
      ++opt;
    }
    return success;
  }

  /* TODO */
  const std::map<char, opt_handler_t> opt_handlers;

  /* TODO */
  char **args_cursor, **args_limit;

  /* TODO */
  msgs_t msgs;

  /* TODO */
  struct channel_t final {

    /* TODO */
    std::unique_ptr<output_t> output;

    /* TODO */
    std::unique_ptr<style_t> style;

  } pass_channel, fail_channel;

  /* TODO */
  std::unique_ptr<output_t> summary_output;

  /* TODO */
  std::unique_ptr<bool> use_color;

  /* TODO */
  std::unique_ptr<std::regex> fixture_name_regex;

};  // parser_t

/* TODO */
inline int main(int argc, char *argv[]) {
  int result;
  try {
    auto config = parser_t::parse(argc, argv);
    result = config ? driver_t::drive(*config) : EXIT_SUCCESS;
  } catch (const parser_t::error_t &error) {
    error.report(std::cerr, "\n  ", "\n  ");
    std::cerr << std::endl;
    result = EXIT_FAILURE;
  }
  return result;
}

}  // lick
}  // cppcon14

/* Inserts arbitrary types of data into the description of an expectation
   object.  Expectation objects are temporary, so we use an r-value reference
   here. */
template <typename that_t>
inline cppcon14::lick::expect_t &&operator<<(
    cppcon14::lick::expect_t &&expectation, const that_t &that) {
  assert(&expectation);
  expectation.get_strm() << that;
  return std::move(expectation);
}
