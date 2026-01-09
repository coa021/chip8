#pragma once
#include "core/types.hpp"


namespace chip8 {

class IRenderer {
public:
  virtual ~IRenderer() = default;

  virtual bool initialize() = 0;
  virtual void shutdown() = 0;
  virtual bool should_close() const = 0;

  virtual void begin_frame() = 0;
  virtual void render(const DisplayBuffer &buffer) = 0;
  virtual void end_frame() = 0;

  // AIO
  virtual void render_frame(const DisplayBuffer &buffer) {
    begin_frame();
    render(buffer);
    end_frame();
  }

  virtual void set_scale(int scale) = 0;
  virtual int get_scale() const = 0;

  virtual int get_window_width() const = 0;
  virtual int get_window_height() const = 0;
  virtual void set_title(const char *title) = 0;
  virtual void toggle_fullscreen() = 0;

protected:
  IRenderer() = default;

  // no copy or move
  IRenderer(const IRenderer &) = delete;
  IRenderer &operator=(const IRenderer &) = delete;
  IRenderer(IRenderer &&) = delete;
  IRenderer &operator=(IRenderer &&) = delete;

};

}