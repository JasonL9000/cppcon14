struct pnt_t final {
  double x, y;
};

struct shape_t {
  struct visitor_t;
  virtual void accept(const visitor_t &) const = 0;
};

struct circle_t final : shape_t {
  pnt_t ctr;
  double radius;
  virtual void accept(const visitor_t &) const override;
};

struct square_t final : shape_t {
  pnt_t ctr;
  double size;
  virtual void accept(const visitor_t &) const override;
};

struct triangle_t final : shape_t {
  pnt_t pnts[3];
  virtual void accept(const visitor_t &) const override;
};

// can't define this until all the finals are defined
struct shape_t::visitor_t final {
  virtual void operator()(const circle_t &) const = 0;
  virtual void operator()(const square_t &) const = 0;
  virtual void operator()(const triangle_t &) const = 0;
};

// can't define these until the visitor is defined
void circle_t::accept(const visitor_t &visitor) const { visitor(this); }
void square_t::accept(const visitor_t &visitor) const { visitor(this); }
void triangle_t::accept(const visitor_t &visitor) const { visitor(this); }

inline double get_area(const shape_t *shape) {
  struct visitor_t final : shape_t::visitor_t {
    double *result;
    virtual void operator()(const circle_t &) const override {
      *result = /* pi-r-squared */ 0;
    }
    virtual void operator()(const square_t &) const override {
      *result = /* size-squared */ 0;
    }
    virtual void operator()(const triangle_t &) const override {
      *result = /* half-base-height */ 0;
    }
  };
  double result;
  shape->accept(visitor_t { &result });
  return result;
}

inline double get_perimeter(const shape_t *shape) {
  struct visitor_t final : shape_t::visitor_t {
    double *result;
    virtual void operator()(const circle_t &) const override {
      *result = /* pi-diameter */ 0;
    }
    virtual void operator()(const square_t &) const override {
      *result = /* 4-size */ 0;
    }
    virtual void operator()(const triangle_t &) const override {
      *result = /* sum-of-edges */ 0;
    }
  };
  double result;
  shape->accept(visitor_t { &result });
  return result;
}

inline std::ostream &operator<<(std::ostream &strm, const shape_t *shape) {
  struct visitor_t final : shape_t::visitor_t {
    std::ostream *strm;
    virtual void operator()(const circle_t &that) const override {
      (*strm) << that.ctr << ", " << that.radius;
    }
    virtual void operator()(const square_t &that) const override {
      (*strm) << that.ctr << ", " that.size;
    }
    virtual void operator()(const triangle_t &that) const override {
      (*strm) << that.pnts[0] << ", "
              << that.pnts[1] << ", "
              << that.pnts[2];
    }
  };
  shape->accept(visitor_t { &strm });
  return strm;
}
