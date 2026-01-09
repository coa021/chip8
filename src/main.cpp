#include "graphics/renderer.hpp"

#include <iostream>
#include <raylib.h>
#include <thread>

int main() {
  // std::cout << "Hello, World!" << std::endl;
  //
  // InitWindow(800, 600, "Test");
  // while (!WindowShouldClose()) {
  //   BeginDrawing();
  //   ClearBackground(BLACK);
  //   DrawText("Raylib works!", 300, 280, 20, WHITE);
  //   EndDrawing();
  // }
  // CloseWindow();

  chip8::RaylibRenderer renderer{};

  if (!renderer.initialize())
    return 1;

  renderer.set_title("Testing");
  chip8::DisplayBuffer buffer{};
  bool blink{false};

  while (!renderer.should_close()) {
    std::fill(buffer.begin(), buffer.end(), false);

    for (std::size_t i{0}; i < chip8::constants::DISPLAY_WIDTH && i <
                           chip8::constants::DISPLAY_HEIGHT; ++i)
      buffer[i * chip8::constants::DISPLAY_WIDTH + i] = true;

    for (std::size_t y{5}; y < 10; ++y)
      for (std::size_t x{5}; x < 10; ++x)
        buffer[y * chip8::constants::DISPLAY_WIDTH + x] = true;

    if (blink)
      buffer[(chip8::constants::DISPLAY_HEIGHT - 1) *
             chip8::constants::DISPLAY_WIDTH + (
               chip8::constants::DISPLAY_WIDTH - 1)] = true;

    blink = !blink;

    renderer.render_frame(buffer);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  renderer.shutdown();

  return 0;
}