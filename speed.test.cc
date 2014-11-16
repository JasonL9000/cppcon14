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

#include <chrono>
#include <iostream>

#include "variant.h"

#include <boost/variant.hpp>

using namespace std;
using namespace cppcon14::variant;

size_t n = 90000000;

auto virtual_dispatch() {
  struct shape_t {
    virtual double get_area() const = 0;
  };
  struct circle_t : shape_t {
    circle_t(double radius) : radius(radius) {}
    virtual double get_area() const { return 3.14 * radius * radius; }
    double radius;
  };
  struct square_t : shape_t {
    square_t(double side) : side(side) {}
    virtual double get_area() const { return side * side; }
    double side;
  };
  struct triangle_t : shape_t {
    triangle_t(double base, double height) : base(base), height(height) {}
    virtual double get_area() const { return base * height / 2; }
    double base;
    double height;
  };
  std::vector<std::unique_ptr<shape_t>> shapes;
  shapes.reserve(n);
  for (size_t i = 0; i < n / 3; ++i) {
    shapes.push_back(std::make_unique<circle_t>(101));
  }  // for
  for (size_t i = 0; i < n / 3; ++i) {
    shapes.push_back(std::make_unique<square_t>(101));
  }  // for
  for (size_t i = 0; i < n / 3; ++i) {
    shapes.push_back(std::make_unique<triangle_t>(101, 202));
  }  // for
  std::vector<double> results;
  results.reserve(n);
  auto start = std::chrono::steady_clock::now();
  for (size_t i = 0; i < shapes.size(); ++i) {
    results[i] = shapes[i]->get_area();
  }  // for
  auto end = std::chrono::steady_clock::now();
  return end - start;
}

struct circle_t {
  circle_t(double radius) : radius(radius) {}
  double get_area() const { return 3.14 * radius * radius; }
  double radius;
};
struct square_t {
  square_t(double side) : side(side) {}
  double get_area() const { return side * side; }
  double side;
};
struct triangle_t {
  triangle_t(double base, double height) : base(base), height(height) {}
  double get_area() const { return base * height / 2; }
  double base;
  double height;
};

struct boost_get_area_t : boost::static_visitor<double> {
  template <typename T>
  double operator()(const T &t) const { return t.get_area(); }
};

auto boost_variant() {
  using shape_t = boost::variant<circle_t, square_t, triangle_t>;
  auto get_area = boost_get_area_t();
  std::vector<shape_t> shapes;
  shapes.reserve(n);
  for (size_t i = 0; i < n / 3; ++i) {
    shapes.push_back(circle_t(101));
  }  // for
  for (size_t i = 0; i < n / 3; ++i) {
    shapes.push_back(square_t(101));
  }  // for
  for (size_t i = 0; i < n / 3; ++i) {
    shapes.push_back(triangle_t(101, 202));
  }  // for
  std::vector<double> results;
  results.reserve(n);
  auto start = std::chrono::steady_clock::now();
  for (size_t i = 0; i < shapes.size(); ++i) {
    results[i] = boost::apply_visitor(get_area, shapes[i]);
  }  // for
  auto end = std::chrono::steady_clock::now();
  return end - start;
}

struct get_area_t {
  using ret_t = double;
  template <typename T>
  double operator()(const T &that) { return that.get_area(); }
};

auto variant() {
  using shape_t = variant_t<circle_t, square_t, triangle_t>;
  auto get_area = get_area_t();
  std::vector<shape_t> shapes;
  shapes.reserve(n);
  for (size_t i = 0; i < n / 3; ++i) {
    shapes.push_back(circle_t(101));
  }  // for
  for (size_t i = 0; i < n / 3; ++i) {
    shapes.push_back(square_t(101));
  }  // for
  for (size_t i = 0; i < n / 3; ++i) {
    shapes.push_back(triangle_t(101, 202));
  }  // for
  std::vector<double> results;
  results.reserve(n);
  auto start = std::chrono::steady_clock::now();
  for (size_t i = 0; i < shapes.size(); ++i) {
    results[i] = apply(get_area, shapes[i]);
  }  // for
  auto end = std::chrono::steady_clock::now();
  return end - start;
}

int main() {
  std::cout << "took: " << std::chrono::duration_cast<std::chrono::seconds>(
                               virtual_dispatch()).count() << std::endl;
}

