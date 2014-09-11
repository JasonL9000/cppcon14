#pragma once

#include "shapes4.h"

bool intersects(const shape_t &lhs, const shape_t &rhs) {
  return cppcon14::variant::match<bool>(
             lhs, rhs,
             [](const circle_t   &, const circle_t   &) { /* ... */ },
             [](const circle_t   &, const square_t   &) { /* ... */ },
             [](const circle_t   &, const triangle_t &) { /* ... */ },
             [](const square_t   &, const circle_t   &) { /* ... */ },
             [](const square_t   &, const square_t   &) { /* ... */ },
             [](const square_t   &, const triangle_t &) { /* ... */ },
             [](const triangle_t &, const circle_t   &) { /* ... */ },
             [](const triangle_t &, const square_t   &) { /* ... */ },
             [](const triangle_t &, const triangle_t &) { /* ... */ });
}
