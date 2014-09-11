#include "variant.h"

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

using shape_t = cppcon14::variant::variant_t<circle_t, square_t, triangle_t>;

inline double get_area(const shape_t &shape) {
  return cppcon14::variant::match<double>(
      shape,
      [](const circle_t &) const {
        return  /* pi-r-squared */ 0;
      },
      [](const square_t &) const {
        return /* size-squared */ 0;
      },
      [] (const triangle_t &) const {
        return /* half-base-height */ 0;
      }
  };
}

inline double get_perimeter(const shape_t &shape) {
  return cppcon14::variant::match<double>(
      shape,
      [](const circle_t &) const {
        return  /* pi-diameter */ 0;
      },
      [](const square_t &) const {
        return /* 4-size */ 0;
      },
      [] (const triangle_t &) const {
        return /* sum-of-edges*/ 0;
      }
  };
}

inline std::ostream &operator<<(std::ostream &strm, const shape_t &shape) {
  return cppcon14::variant::match<std::ostream &>(
    [&](const circle_t &that) {
      return strm << that.ctr << ", " << that.radius;
    },
    [&](const square_t &that) {
      return strm << that.ctr << ", " that.size;
    },
    [&](const triangle_t &that) {
      return strm << that.pnts[0] << ", "
                  << that.pnts[1] << ", "
                  << that.pnts[2];
    }
  );
}
