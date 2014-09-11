#include "shapes4.h"

bool intersects(const shape_t &lhs, const shape_t &rhs) {
  return match(lhs, rhs,
               [](const circle_t &lhs, const circle_t &rhs) { /* ... */ }
               [](const circle_t &lhs, const square_t &rhs) { /* ... */ }
               [](const circle_t &lhs, const triangle_t &rhs) { /* ... */ }
               [](const square_t &lhs, const circle_t &rhs) { /* ... */ }
               [](const square_t &lhs, const square_t &rhs) { /* ... */ }
               [](const square_t &lhs, const triangle_t &rhs) { /* ... */ }
               [](const square_t &lhs, const circle_t &rhs) { /* ... */ }
               [](const square_t &lhs, const square_t &rhs) { /* ... */ }
               [](const square_t &lhs, const triangle_t &rhs) { /* ... */ });
}
