#pragma once

#include "animal.h"
#include "transport.h"

bool can_transport_animal(const transport_t &transport,
                          const animal_t &animal) {
  return cppcon14::variant::match<bool>(
             transport, animal,
             [](const car_t &, const horse_t &) {
               // horse is too big for a car.
               return false;
             },
             [](const plane_t &, const auto &) {
               // don't take animals onto planes.
               return false;
             },
             [](const horse_t &, const horse_t &) {
               // don't put a horse on a horse! that's dumb.
               return false;
             },
             [](const auto &, const auto &) {
               // you're fine.
               return true;
             });
}

