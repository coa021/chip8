#pragma once
#include "core/types.hpp"

#include <algorithm>
#include <functional>


namespace chip8 {

class Display {
public:
  using UpdateCallback = std::function<void(const DisplayBuffer &)>;

  Display() = default;

  [[nodiscard]] bool get_pixel(std::size_t x, std::size_t y) const noexcept {
    if (x >= constants::DISPLAY_WIDTH || y >= constants::DISPLAY_HEIGHT)
      return false;
    return m_Buffer[y * constants::DISPLAY_WIDTH + x];
  }

  void set_pixel(std::size_t x, std::size_t y, bool value) noexcept {
    if (x < constants::DISPLAY_WIDTH && y < constants::DISPLAY_HEIGHT) {
      m_Buffer[y * constants::DISPLAY_WIDTH + x] = value;
      m_Dirty = true;
    }
  }

  /// returns true if pixel was turned off (collision)
  bool xor_pixel(std::size_t x, std::size_t y, bool value) noexcept {
    if (x >= constants::DISPLAY_WIDTH || y >= constants::DISPLAY_HEIGHT)
      return false;

    const std::size_t index{y * constants::DISPLAY_WIDTH + x};
    const bool was_on{m_Buffer[index]};
    m_Buffer[index] = was_on ^ value;
    m_Dirty = true;

    // collision when pixel is turned off
    return was_on && value;
  }

  bool draw_sprite(Byte start_x, Byte start_y,
                   MemoryView sprite_data) noexcept {
    bool collision{false};

    const std::size_t wrapped_x{start_x % constants::DISPLAY_WIDTH};
    const std::size_t wrapped_y{start_y % constants::DISPLAY_HEIGHT};

    for (std::size_t row{0}; row < sprite_data.size(); ++row) {
      const std::size_t y{wrapped_y + row};
      if (y >= constants::DISPLAY_HEIGHT)
        break;

      const Byte sprite_row{sprite_data[row]};
      for (std::size_t col{0}; col < 8; ++col) {
        const std::size_t x{wrapped_x + col};

        if (x >= constants::DISPLAY_WIDTH)
          break;

        const bool sprite_pixel{(sprite_row & (0x80 >> col)) != 0};
        if (sprite_pixel && xor_pixel(x, y, true))
          collision = true;
      }
    }
    return collision;
  }


  /// clear the display
  void clear() noexcept {
    m_Buffer.fill(false);
    m_Dirty = true;
    if (m_Update_callback)
      m_Update_callback(m_Buffer);
  }

  [[nodiscard]] const DisplayBuffer &buffer() const noexcept {
    return m_Buffer;
  }

  [[nodiscard]] bool is_dirty() const noexcept { return m_Dirty; }

  void clear_dirty() noexcept { m_Dirty = false; }

  /// notify observers of display update
  void notify_update() {
    if (m_Update_callback)
      m_Update_callback(m_Buffer);
  }

  void set_update_callback(UpdateCallback callback) {
    m_Update_callback = std::move(callback);
  }

  [[nodiscard]] std::size_t count_on_pixels() const noexcept {
    return static_cast<std::size_t>(std::count(m_Buffer.begin(), m_Buffer.end(),
                                               true));
  }

  [[nodiscard]] bool is_clear() const noexcept {
    return std::ranges::none_of(m_Buffer.begin(), m_Buffer.end(),
                                [](bool b) { return b; });
  }

  static constexpr std::size_t coords_to_index(std::size_t x,
                                               std::size_t y) noexcept {
    return y * constants::DISPLAY_WIDTH + x;
  }

  static constexpr Coordinate index_to_coords(std::size_t index) noexcept {
    return {
        index % constants::DISPLAY_WIDTH,
        index / constants::DISPLAY_HEIGHT
    };
  }

private:
  DisplayBuffer m_Buffer{};
  bool m_Dirty{false};
  UpdateCallback m_Update_callback;
};

}