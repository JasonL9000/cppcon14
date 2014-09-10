struct pnt_t final {
  double x, y;
};

struct shape_t {};

struct circle_t final : shape_t {
  pnt_t ctr;
  double radius;
};

struct square_t final : shape_t {
  pnt_t ctr;
  double size;
};

struct triangle_t final : shape_t {
  pnt_t pnts[3];
};
