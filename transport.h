// ---------------------------------------------------------------------------
// "car.h"

struct car_t {
  // stuff about going fast and carrying stuff stuff in the trunk
};
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// "plane.h"

struct plane_t {
  // stuff about going really, really fast and carrying cargo
};
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// "horse.h"

struct horse_t {
  // stuff about neighing, running fast, and carrying people and cargo
};
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// "transport.h"

#include "car.h"
#include "plane.h"
#include "horse.h"

using transport_t = variant_t<car_t, plane_t, horse_t>;
// stuff about going fast and carrying cargo
// ---------------------------------------------------------------------------
