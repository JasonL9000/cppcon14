#pragma once

#include "variant.h"

// ---------------------------------------------------------------------------
// "dog.h"

struct dog_t {
  // stuff about barking, licking people, and chasing cars
};
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// "cat.h"

struct cat_t {
  // stuff about meowing and ignoring people
};
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// "horse.h"

struct horse_t {
  // stuff about neighing, running fast, and carrying people and cargo
};
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// "animal.h"

/*
#include "dog.h"
#include "cat.h"
#include "horse.h"
*/

using animal_t = cppcon14::variant::variant_t<dog_t, cat_t, horse_t>;
// stuff about making sounds and interacting with people
// ---------------------------------------------------------------------------
