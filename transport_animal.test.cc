#include "transport_animal.h"
#include "variant.h"

#include "lick.h"

FIXTURE(transport_animal) {
  transport_t transport = car_t();
  animal_t animal = dog_t();
  EXPECT_TRUE(can_transport_animal(transport, animal));
  animal = horse_t();
  EXPECT_FALSE(can_transport_animal(transport, animal));
}

