#include <ostream>

struct pnt_t final {
  double x, y;
};

struct shape_t {
  virtual double get_area() const = 0;
  virtual double get_perimeter() const = 0;
  virtual void write(std::ostream &strm) const = 0;
};

struct circle_t final : shape_t {
  virtual double get_area() const override;
  virtual double get_perimeter() const override;
  virtual void write(std::ostream &strm) const override;
  pnt_t ctr;
  double radius;
};

struct square_t final : shape_t {
  virtual double get_area() const override;
  virtual double get_perimeter() const override;
  virtual void write(std::ostream &strm) const override;
  pnt_t ctr;
  double size;
};

struct triangle_t final : shape_t {
  virtual double get_area() const override;
  virtual double get_perimeter() const override;
  virtual void write(std::ostream &strm) const override;
  pnt_t pnts[3];
};
