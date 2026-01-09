#include "audio/beeper.hpp"
#include "graphics/renderer.hpp"

#include <iostream>
#include <raylib.h>

int main() {
  chip8::RaylibRenderer renderer{};
  chip8::Beeper beeper;

  if (!renderer.initialize())
    return 1;
  if (!beeper.initialize())
    return 1;

  renderer.set_title("Testing");
  chip8::DisplayBuffer buffer{};
  bool blink{false};

  while (!renderer.should_close()) {
    if (IsKeyPressed(KEY_SPACE))
      beeper.start_beep();
    if (IsKeyReleased(KEY_SPACE))
      beeper.stop_beep();

    std::fill(buffer.begin(), buffer.end(), false);

    for (std::size_t i{0}; i < chip8::constants::DISPLAY_WIDTH &&
                           i < chip8::constants::DISPLAY_HEIGHT; ++i)
      buffer[i * chip8::constants::DISPLAY_WIDTH + i] = true;

    for (std::size_t y{5}; y < 10; ++y)
      for (std::size_t x{5}; x < 10; ++x)
        buffer[y * chip8::constants::DISPLAY_WIDTH + x] = true;

    if (blink)
      buffer[(chip8::constants::DISPLAY_HEIGHT - 1) *
             chip8::constants::DISPLAY_WIDTH +
             (chip8::constants::DISPLAY_WIDTH - 1)] = true;

    blink = !blink;

    renderer.render_frame(buffer);

    beeper.update();
  }

  renderer.shutdown();
  beeper.shutdown();
  return 0;
}