#include "catch2/catch_test_macros.hpp"
#include "core/timers.hpp"

#include <thread>

using namespace chip8;
using namespace std::chrono_literals;

TEST_CASE("Timers initialize to zero", "[timers]") {
  Timers timers;

  REQUIRE(timers.delay() ==0);
  REQUIRE(timers.sound() == 0);
  REQUIRE_FALSE(timers.state().is_sound_active());
}

TEST_CASE("Timers can be set", "[timers]") {
  Timers timers;

  timers.set_delay(10);
  timers.set_sound(5);

  REQUIRE(timers.delay() == 10);
  REQUIRE(timers.sound() == 5);
}

TEST_CASE("Timers decrement at 60hz", "[timers]") {
  Timers timers;

  timers.set_delay(10);
  std::this_thread::sleep_for(35ms);

  int ticks{timers.update()};

  REQUIRE(ticks >= 1);
  REQUIRE(timers.delay() < 10);
}

TEST_CASE("Timers dont go below zero", "[timers]") {
  Timers timers;
  timers.set_delay(1);
  std::this_thread::sleep_for(100ms);
  timers.update();

  REQUIRE(timers.delay() == 0);
}

TEST_CASE("Sound callback fires on state change", "[timers]") {
  Timers timers;

  bool callback_fired{false};
  bool sound_state{false};

  timers.set_sound_callback([&](bool active) {
    callback_fired = true;
    sound_state = active;
  });

  timers.set_sound(5);

  REQUIRE(callback_fired);
  REQUIRE(sound_state == true);
}

TEST_CASE("Timer reset zeroes everything", "[timers]") {
  Timers timers;
  timers.set_delay(100);
  timers.set_sound(50);

  timers.reset();

  REQUIRE(timers.sound() == 0);
  REQUIRE(timers.delay() == 0);
}