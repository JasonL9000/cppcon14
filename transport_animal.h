#include "animal.h"
#include "transport.h"

bool can_transport_animal(const transport_t &transport,
                          const animal_t &animal) {
  return match(transport, animal,
               [](const car_t &, const horse_t &) {
                 // horse is too big for a car.
                 return false;
               },
               [](const plane_t &, const auto &) {
                 // don't take animals onto planes.
                 return false;
               },
               [](const horse_t &lhs, const horse_t &rhs) {
                 // can't be the same horse.
                 return lhs != rhs;
               },
               [](const auto &, const auto &) {
                 // you're fine.
                 return true;
               });
}

