#include "variant.h"

namespace cppcon14 {

struct pnt_t final {
  double x, y;
};

struct circle_t final {
  pnt_t ctr;
  double radius;
};

struct square_t final {
  pnt_t ctr;
  double size;
};

struct triangle_t final {
  pnt_t pnts[3];
};

using shape_t = variant::variant_t<circle_t, square_t, triangle_t>;

inline double get_area(const shape_t &shape) {
  struct func_t {
    using fn1_t = double ();
    double operator()(nullptr_t) const {
      return 0;
    }
    double operator()(const circle_t &) const {
      return 0;
    }
    double operator()(const square_t &) const {
      return 0;
    }
    double operator()(const triangle_t &) const {
      return 0;
    }
  };
  return variant::apply(func_t(), shape);
}

}  // cppcon14
