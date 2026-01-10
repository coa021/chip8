#pragma once

#include "i_input.hpp"
#include "key_codes.hpp"

#include <memory>

namespace chip8 {

struct KeyMapping {
  Key platform_key;
  Byte chip8_key;

  constexpr KeyMapping(Key pk, Byte ck) : platform_key{pk}, chip8_key{ck} {
  }
};

inline constexpr std::array<KeyMapping, 16> DEFAULT_KEY_MAP{
    {
        {Key::X, 0x0},
        {Key::ONE, 0x1},
        {Key::TWO, 0x2},
        {Key::THREE, 0x3},
        {Key::Q, 0x4},
        {Key::W, 0x5},
        {Key::E, 0x6},
        {Key::A, 0x7},
        {Key::S, 0x8},
        {Key::D, 0x9},
        {Key::Z, 0xA},
        {Key::C, 0xB},
        {Key::FOUR, 0xC},
        {Key::R, 0xD},
        {Key::F, 0xE},
        {Key::V, 0xF},

    }};

class Keyboard : public IInput {
public:
  Keyboard() = default;

  explicit Keyboard(std::shared_ptr<IKeyStateProvider> provider)
    : m_Provider{std::move(provider)} {
    set_mappings(DEFAULT_KEY_MAP);
  }

  explicit Keyboard(std::shared_ptr<IKeyStateProvider> provider,
                    std::span<const KeyMapping> mappings)
    : m_Provider{std::move(provider)} {
    set_mappings(mappings);
  }

  void update() override {
    // prev state is stored for detecting key presses
    m_Previous_state = m_Current_state;
    m_Last_key_pressed.reset();

    for (const auto &mapping : m_Mappings) {
      const bool pressed{m_Provider->is_key_down(mapping.platform_key)};
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
    while (!m_Provider->should_quit()) {
      for (const auto &mapping : m_Mappings) {
        if (m_Provider->is_key_pressed(mapping.platform_key))
          return KeyIndex{mapping.chip8_key};
      }
      m_Provider->wait_time(0.001);
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
      if (m_Provider->is_key_pressed(mapping.platform_key))
        return KeyIndex{mapping.chip8_key};

    return std::nullopt;
  }

  // custom keybindings section
  bool is_reset_pressed() { return m_Provider->is_key_pressed(Key::F5); }

  bool is_pause_pressed() {
    return m_Provider->is_key_pressed(Key::SPACE);
  }

  bool is_fullscreen_pressed() {
    return m_Provider->is_key_pressed(Key::F11);
  }

  bool is_quit_pressed() {
    return m_Provider->is_key_pressed(Key::ESCAPE);
  }

private:
  std::shared_ptr<IKeyStateProvider> m_Provider;
  std::array<KeyMapping, 16> m_Mappings{DEFAULT_KEY_MAP};
  KeyState m_Current_state{};
  KeyState m_Previous_state{};
  std::optional<KeyIndex> m_Last_key_pressed;
};
}