#include "catch2/catch_test_macros.hpp"
#include "graphics/Display.hpp"

using namespace chip8;

TEST_CASE("Display initializes clear", "[display]") {
  Display display;
  REQUIRE(display.is_clear());
  REQUIRE(display.count_on_pixels() == 0);
}

TEST_CASE("Display set/get pixel", "[display]") {
  Display display;
  display.set_pixel(10, 20, true);

  REQUIRE(display.get_pixel(10,20) == true);
  REQUIRE(display.get_pixel(10,21) == false);
}

TEST_CASE("XOR returns collision", "[display]") {
  Display display;

  // xor on empty, no collision
  REQUIRE(display.xor_pixel(0,0,true) == false);
  REQUIRE(display.get_pixel(0,0) == true);

  // xor on lit, collision
  REQUIRE(display.xor_pixel(0,0,true) == true);
  REQUIRE(display.get_pixel(0,0) == false);
}

TEST_CASE("Sprite drawing", "[display]") {
  Display display;
  std::array<Byte, 1> sprite = {0b11110000};
  bool collision{display.draw_sprite(0, 0, sprite)};

  REQUIRE(collision == false);
  REQUIRE(display.get_pixel(0,0) == true);
  REQUIRE(display.get_pixel(3,0) == true);
  REQUIRE(display.get_pixel(4,0) == false);
}

TEST_CASE("Sprite collision detection", "[display]") {
  Display display;
  std::array<Byte, 1> sprite = {0x80};

  display.draw_sprite(0, 0, sprite);
  bool collision{display.draw_sprite(0, 0, sprite)};

  REQUIRE(collision == true);
  REQUIRE(display.get_pixel(0,0) == false);
}

TEST_CASE("Sprite wrapping", "[display]") {
  Display display;
  std::array<Byte, 1> sprite = {0xFF};

  display.draw_sprite(60, 0, sprite);

  REQUIRE(display.get_pixel(60,0) == true);
  REQUIRE(display.get_pixel(0,0) == true); // wrapped
}

TEST_CASE("Display clear", "[display]") {
  Display display;

  display.set_pixel(10, 10, true);
  display.clear();

  REQUIRE(display.is_clear());
}

TEST_CASE("Display dirty flag", "[display]") {
  Display display;

  REQUIRE(display.is_dirty() == true);
  display.clear_dirty();
  REQUIRE_FALSE(display.is_dirty());

  display.set_pixel(0, 0, true);
  REQUIRE(display.is_dirty());
}