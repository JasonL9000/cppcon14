#include "intersects.h"
#include "variant.h"

#include "lick.h"

FIXTURE(intersects) {
  shape_t lhs = circle_t{{0, 0}, 101};
  shape_t rhs = square_t{{0, 0}, 202};
  EXPECT_TRUE(intersects(lhs, rhs));
}

