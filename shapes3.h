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

double get_area(const shape_t *shape) {
  struct visitor_t final : shape_t::visitor_t {
    double *result;
    virtual void operator()(const circle_t &) const override {
      *result = /* something */ 0;
    }
    virtual void operator()(const square_t &) const override {
      *result = /* something */ 0;
    }
    virtual void operator()(const triangle_t &) const override {
      *result = /* something */ 0;
    }
  };
  double result;
  shape->accept(visitor_t { &result });
  return result;
}

double get_perimeter(const shape_t *shape) {
  struct visitor_t final : shape_t::visitor_t {
    double *result;
    virtual void operator()(const circle_t &) const override {
      *result = /* something */ 0;
    }
    virtual void operator()(const square_t &) const override {
      *result = /* something */ 0;
    }
    virtual void operator()(const triangle_t &) const override {
      *result = /* something */ 0;
    }
  };
  double result;
  shape->accept(visitor_t { &result });
  return result;
}
