#pragma once


namespace chip8 {
enum class Key: int {
  ONE = 0,
  TWO = 1,
  THREE = 2,
  FOUR = 3,
  Q = 4,
  W = 5,
  E = 6,
  R = 7,
  A = 8,
  S = 9,
  D = 10,
  F = 11,
  Z = 12,
  X = 13,
  C = 14,
  V = 15,
  SPACE = 16,
  ESCAPE = 17,
  F5 = 18,
  F11 = 19,

  KEY_COUNT = 20 // TODO: update the value as we add more keybinding later on
};

class IKeyStateProvider {
public:
  virtual ~IKeyStateProvider() = default;

  virtual bool is_key_down(Key key) const = 0;
  virtual bool is_key_pressed(Key key) const = 0;
  virtual void wait_time(double seconds) const = 0;
  virtual bool should_quit() const = 0;
};

}