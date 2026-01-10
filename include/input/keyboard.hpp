#pragma once
#include "i_input.hpp"
#include <raylib.h>


namespace chip8 {

struct KeyMapping {
  int raylib_key;
  Byte chip8_key;

  constexpr KeyMapping(int rk, Byte ck) : raylib_key{rk}, chip8_key{ck} {
  }
};

inline constexpr std::array<KeyMapping, 16> DEFAULT_KEY_MAP{
    {
        {KEY_X, 0x0},
        {KEY_ONE, 0x1},
        {KEY_TWO, 0x2},
        {KEY_THREE, 0x3},
        {KEY_Q, 0x4},
        {KEY_W, 0x5},
        {KEY_E, 0x6},
        {KEY_A, 0x7},
        {KEY_S, 0x8},
        {KEY_D, 0x9},
        {KEY_Z, 0xA},
        {KEY_C, 0xB},
        {KEY_FOUR, 0xC},
        {KEY_R, 0xD},
        {KEY_F, 0xE},
        {KEY_V, 0xF},

    }};

class Keyboard : public IInput {
public:
  Keyboard() = default;

  explicit Keyboard(std::span<const KeyMapping> mappings) {
    set_mappings(mappings);
  }

  void update() override {
    // prev state is stored for detecting key presses
    m_Previous_state = m_Current_state;
    m_Last_key_pressed.reset();

    for (const auto &mapping : m_Mappings) {
      const bool pressed{IsKeyDown(mapping.raylib_key)};
      m_Current_state[mapping.chip8_key] = pressed;

      if (pressed && !m_Previous_state[mapping.chip8_key])
        m_Last_key_pressed = KeyIndex{mapping.chip8_key};
    }
  }


  bool is_key_pressed(KeyIndex key) const override {
    if (key.get() >= constants::NUM_KEYS)
      return false;
    return m_Current_state[key.get()];
  }

  const KeyState &get_key_state() const override {
    return m_Current_state;
  }

  KeyIndex wait_for_key() override {
    // blocking code
    while (!WindowShouldClose()) {
      for (const auto &mapping : m_Mappings) {
        if (IsKeyPressed(mapping.raylib_key))
          return KeyIndex{mapping.chip8_key};
      }
      WaitTime(0.001);
    }
    return KeyIndex{0};
  }

  bool any_key_pressed() const override {
    return m_Last_key_pressed.has_value();
  }

  std::optional<KeyIndex> get_last_key_pressed() const override {
    return m_Last_key_pressed;
  }

  void set_mappings(std::span<const KeyMapping> mappings) {
    const auto count{std::min(mappings.size(), m_Mappings.size())};

    for (std::size_t i{0}; i < count; ++i)
      m_Mappings[i] = mappings[i];
  }

  void reset_to_default() { set_mappings(DEFAULT_KEY_MAP); }

  // non blocking key wait
  std::optional<KeyIndex> poll_key_press() const {
    for (const auto &mapping : m_Mappings)
      if (IsKeyPressed(mapping.raylib_key))
        return KeyIndex{mapping.chip8_key};

    return std::nullopt;
  }

  // custom keybindings section
  static bool is_reset_pressed() { return IsKeyPressed(KEY_F5); }
  static bool is_pause_pressed() { return IsKeyPressed(KEY_SPACE); }
  static bool is_fullscreen_pressed() { return IsKeyPressed(KEY_F11); }
  static bool is_quit_pressed() { return IsKeyPressed(KEY_ESCAPE); }

private:
  std::array<KeyMapping, 16> m_Mappings{DEFAULT_KEY_MAP};
  KeyState m_Current_state{};
  KeyState m_Previous_state{};
  std::optional<KeyIndex> m_Last_key_pressed;
};
}