/* ---------------------------------------------------------------------------
Copyright 2014
  Jason Lucas (JasonL9000@gmail.com) and
  Michael Park (mcypark@gmail.com).

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
  HTTP://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This module simplifies building unit tests with lick.  It contains nothing
but a main function, and this main just hands off the the main entry point
in lick, parsing the command line and initiating the test fixture driver.

Compile and link your unit test modules with this module, or provide your
own main, whichever you prefer.
--------------------------------------------------------------------------- */

#include "lick.h"

int main(int argc, char *argv[]) {
  return cppcon14::lick::main(argc, argv);
}
