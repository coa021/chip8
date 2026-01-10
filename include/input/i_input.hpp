#pragma once
#include "core/types.hpp"

#include <optional>


namespace chip8 {

class IInput {
public:
  virtual ~IInput() = default;

  virtual void update() = 0;
  virtual bool is_key_pressed(KeyIndex key) const = 0;
  virtual const KeyState &get_key_state() const = 0;

  virtual KeyIndex wait_for_key() = 0;
  virtual bool any_key_pressed() const =0;

  virtual std::optional<KeyIndex> get_last_key_pressed() const = 0;

protected:
  IInput() = default;
  IInput(const IInput &) = delete;
  IInput &operator=(const IInput &) = delete;
  IInput(IInput &&) = delete;
  IInput &operator=(IInput &&) = delete;

};

}