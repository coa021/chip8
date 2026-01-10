#pragma once

#include "logger.hpp"
#include <filesystem>

namespace chip8 {

struct Config {
  int display_scale{12};
  bool start_fullscreen{false};

  float beep_frequency{440.0f};
  float beep_volume{0.3f};
  bool audio_enabled{true};

  double cpu_frequency{500.0};
  bool shift_quirk{false};
  bool load_store_quirk{false};
  bool jump_quirk{false};

  bool debug_mode{false};
  LogLevel log_level{LogLevel::Info};

  std::filesystem::path rom_directory{"./roms"};
  std::filesystem::path last_rom_path{""};
};

}