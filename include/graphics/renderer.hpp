#pragma once
#include "i_renderer.hpp"
#include <raylib.h>

#include <algorithm>

namespace chip8 {


class RaylibRenderer : public IRenderer {
public:
  static constexpr int DEFAULT_SCALE{12};
  static constexpr int MIN_SCALE{1};
  static constexpr int MAX_SCALE{32};

  explicit RaylibRenderer(int scale = DEFAULT_SCALE)
    : m_Scale{std::clamp(scale, MIN_SCALE, MAX_SCALE)} {
  }

  ~RaylibRenderer() override {
    if (m_Initialized)
      shutdown();
  }

  bool initialize() override {
    if (m_Initialized)
      return true;

    const int width{static_cast<int>(constants::DISPLAY_WIDTH) * m_Scale};
    const int height{static_cast<int>(constants::DISPLAY_HEIGHT) * m_Scale};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(width, height, "CHIP-8 Interpreter");

    if (!IsWindowReady())
      return false;

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    create_render_texture();

    m_Initialized = true;
    return true;
  }

  void shutdown() override {
    if (!m_Initialized)
      return;

    if (m_Render_texture.id != 0)
      UnloadRenderTexture(m_Render_texture);

    CloseWindow();
    m_Initialized = false;
  }

  bool should_close() const override { return WindowShouldClose(); }

  void begin_frame() override {
    BeginDrawing();
    ClearBackground(BLACK);
    // TODO: thinking of doing custom themes, if implemented change here
  }

  void render(const DisplayBuffer &buffer) override {
    BeginTextureMode(m_Render_texture);
    ClearBackground(BLACK); // TODO: theming here too

    for (std::size_t y{0}; y < constants::DISPLAY_HEIGHT; ++y) {
      for (std::size_t x{0}; x < constants::DISPLAY_WIDTH; ++x) {
        if (buffer[y * constants::DISPLAY_WIDTH + x]) {
          DrawRectangle(
              static_cast<int>(x) * m_Scale,
              static_cast<int>(y) * m_Scale,
              m_Scale,
              m_Scale,
              GREEN); // TODO: theming here too
        }
      }
    }

    EndTextureMode();

    draw_display_texture();
  }

  void end_frame() override {
    EndDrawing();
  }

  void set_scale(int scale) override {
    m_Scale = std::clamp(scale, MIN_SCALE, MAX_SCALE);
    if (m_Initialized)
      create_render_texture();
  }

  int get_scale() const override { return m_Scale; }
  int get_window_width() const override { return GetScreenWidth(); }
  int get_window_height() const override { return GetScreenHeight(); }
  void set_title(const char *title) override { SetWindowTitle(title); }
  void toggle_fullscreen() override { ToggleFullscreen(); }

private:
  void create_render_texture() {
    if (m_Render_texture.id != 0)
      UnloadRenderTexture(m_Render_texture);

    const int width{static_cast<int>(constants::DISPLAY_WIDTH) * m_Scale};
    const int height{static_cast<int>(constants::DISPLAY_HEIGHT) * m_Scale};

    m_Render_texture = LoadRenderTexture(width, height);
    SetTextureFilter(m_Render_texture.texture, TEXTURE_FILTER_POINT);

  }

  void draw_display_texture() {
    const int tex_width{m_Render_texture.texture.width};
    const int tex_height{m_Render_texture.texture.height};
    const int screen_width{GetScreenWidth()};
    const int screen_height{GetScreenHeight()};

    const float scale_x{
        static_cast<float>(screen_width) / static_cast<float>(tex_width)};
    const float scale_y{
        static_cast<float>(screen_height) / static_cast<float>(tex_height)};
    const float scale{std::min(scale_x, scale_y)};

    const float dest_width{static_cast<float>(tex_width) * scale};
    const float dest_height{static_cast<float>(tex_height) * scale};
    const float dest_x{(static_cast<float>(screen_width) - dest_width) / 2.0f};
    const float dest_y
        {(static_cast<float>(screen_height) - dest_height) / 2.0f};

    const Rectangle source{
        0.0f,
        static_cast<float>(tex_height),
        static_cast<float>(tex_width),
        -static_cast<float>(tex_height)
    };
    const Rectangle dest{
        dest_x,
        dest_y,
        dest_width,
        dest_height
    };
    const Color white_color{255, 255, 255, 255};
    DrawTexturePro(m_Render_texture.texture, source, dest, {0, 0}, 0.0f,
                   WHITE);
  }

  int m_Scale;
  RenderTexture2D m_Render_texture{};
  bool m_Initialized{false};

};

}