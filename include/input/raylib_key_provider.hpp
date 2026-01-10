#pragma once
#include "key_codes.hpp"
#include <raylib.h>

namespace chip8 {


class RaylibKeyProvider : public IKeyStateProvider {
public:
  bool is_key_down(Key key) const override {
    return IsKeyDown(to_raylib_key(key));
  }

  bool is_key_pressed(Key key) const override {
    return IsKeyPressed(to_raylib_key(key));
  }

  void wait_time(double seconds) const override { WaitTime(seconds); }
  bool should_quit() const override { return WindowShouldClose(); }

private:
  static int to_raylib_key(Key key) {
    switch (key) {
    case Key::ONE:
      return KEY_ONE;
    case Key::TWO:
      return KEY_TWO;
    case Key::THREE:
      return KEY_THREE;
    case Key::FOUR:
      return KEY_FOUR;
    case Key::Q:
      return KEY_Q;
    case Key::W:
      return KEY_W;
    case Key::E:
      return KEY_E;
    case Key::R:
      return KEY_R;
    case Key::A:
      return KEY_A;
    case Key::S:
      return KEY_S;
    case Key::D:
      return KEY_D;
    case Key::F:
      return KEY_F;
    case Key::Z:
      return KEY_Z;
    case Key::X:
      return KEY_X;
    case Key::C:
      return KEY_C;
    case Key::V:
      return KEY_V;
    case Key::SPACE:
      return KEY_SPACE;
    case Key::ESCAPE:
      return KEY_ESCAPE;
    case Key::F5:
      return KEY_F5;
    case Key::F11:
      return KEY_F11;
    default:
      return KEY_NULL;
    }
  }
};


}