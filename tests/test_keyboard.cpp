#include "catch2/catch_test_macros.hpp"
#include "core/types.hpp"
#include "input/keyboard.hpp"
#include "mocks/mock_key_provider.hpp"

#include <set>

using namespace chip8;

TEST_CASE("Default key map has 16 keys", "[input]") {
  std::set<Byte> mapped_keys;
  for (const auto &m : DEFAULT_KEY_MAP)
    mapped_keys.insert(m.chip8_key);

  REQUIRE(mapped_keys.size() == 16);
  for (Byte i{0}; i < 16; ++i)
    REQUIRE(mapped_keys.count(i) == 1);
}

TEST_CASE("Keyboard detects key presses", "[input]") {
  auto mock_provider{std::make_shared<test::MockKeyProvider>()};
  Keyboard keyboard{mock_provider};

  SECTION("Initially no keys are pressed") {
    keyboard.update();
    REQUIRE_FALSE(keyboard.any_key_pressed());
    for (Byte i{0}; i < 16; ++i)
      REQUIRE_FALSE(keyboard.is_key_pressed(KeyIndex{i}));
  }

  SECTION("Detects when key 0 is pressed") {
    mock_provider->set_key_down(Key::X, true);
    keyboard.update();

    REQUIRE(keyboard.is_key_pressed(KeyIndex{0x0}));
    REQUIRE(keyboard.any_key_pressed() == true);
    REQUIRE(keyboard.get_last_key_pressed() == KeyIndex{0x0});
  }

  SECTION("Detects multiple keys") {
    mock_provider->set_key_down(Key::Q, true);
    mock_provider->set_key_down(Key::W, true);
    keyboard.update();

    REQUIRE(keyboard.is_key_pressed(KeyIndex{0x04})); // for q
    REQUIRE(keyboard.is_key_pressed(KeyIndex{0x05})); // for w
  }

  SECTION("Detects key release") {
    mock_provider->set_key_down(Key::A, true);
    keyboard.update();
    REQUIRE(keyboard.is_key_pressed(KeyIndex{0x07}));

    mock_provider->set_key_down(Key::A, false);
    keyboard.update();
    REQUIRE_FALSE(keyboard.is_key_pressed(KeyIndex{0x07}));
  }
}


TEST_CASE("Keyboard custom keys", "[input]") {
  auto mock_provider{std::make_shared<test::MockKeyProvider>()};
  Keyboard keyboard{mock_provider};

  SECTION("Detects reset key") {
    mock_provider->set_key_pressed(Key::F5, true);
    keyboard.update();
    REQUIRE(keyboard.is_reset_pressed());
  }
  SECTION("Detects pause key") {
    mock_provider->set_key_pressed(Key::SPACE, true);
    keyboard.update();
    REQUIRE(keyboard.is_pause_pressed());
  }
  SECTION("Detects fullscreen key") {
    mock_provider->set_key_pressed(Key::F11, true);
    keyboard.update();
    REQUIRE(keyboard.is_fullscreen_pressed());
  }
}