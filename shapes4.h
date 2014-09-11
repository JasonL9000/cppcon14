#include <ostream>

#include "variant.h"

struct pnt_t final {
  double x, y;
};

std::ostream &operator<<(std::ostream &strm, const pnt_t &that) {
  return strm << '[' << that.x << ", " << that.y << ']';
}

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

using shape_t = cppcon14::variant::variant_t<circle_t, square_t, triangle_t>;

double get_area(const shape_t &shape) {
  return cppcon14::variant::match<double>(
      shape,
      [](const circle_t &) {
        return  /* pi-r-squared */ 0;
      },
      [](const square_t &) {
        return /* size-squared */ 0;
      },
      [] (const triangle_t &) {
        return /* half-base-height */ 0;
      }
  );
}

double get_perimeter(const shape_t &shape) {
  return cppcon14::variant::match<double>(
      shape,
      [](const circle_t &) {
        return  /* pi-diameter */ 0;
      },
      [](const square_t &) {
        return /* 4-size */ 0;
      },
      [] (const triangle_t &) {
        return /* sum-of-edges*/ 0;
      }
  );
}

std::ostream &operator<<(std::ostream &strm, const shape_t &shape) {
  cppcon14::variant::match<void>(
    shape,
    [&](const circle_t &that) {
      strm << that.ctr << ", " << that.radius;
    },
    [&](const square_t &that) {
      strm << that.ctr << ", " << that.size;
    },
    [&](const triangle_t &that) {
      strm << that.pnts[0] << ", "
           << that.pnts[1] << ", "
           << that.pnts[2];
    }
  );
  return strm;
}
