#pragma once
#include "input/key_codes.hpp"

#include <unordered_map>


namespace chip8::test {

class MockKeyProvider : public IKeyStateProvider {
public:
  void set_key_down(Key key, bool down) {
    m_Keys_down[key] = down;
  }

  void set_key_pressed(Key key, bool pressed) {
    m_Keys_pressed[key] = pressed;
  }

  void set_should_quit(Key key, bool quit) {
    m_Should_quit = quit;
  }

  bool is_key_down(Key key) const override {
    auto it{m_Keys_down.find(key)};
    return it != m_Keys_down.end() ? it->second : false;
  }

  bool is_key_pressed(Key key) const override {
    auto it{m_Keys_pressed.find(key)};
    return it != m_Keys_pressed.end() ? it->second : false;
  }

  void wait_time(double seconds) const override {
    // empty for test cases
  }

  bool should_quit() const override { return m_Should_quit; }

private:
  std::unordered_map<Key, bool> m_Keys_down;
  std::unordered_map<Key, bool> m_Keys_pressed;
  bool m_Should_quit{false};

};

}