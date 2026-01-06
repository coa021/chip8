#pragma once

#include "logger.hpp"
#include <filesystem>

namespace chip8 {

struct Config {
  bool debug_mode{false};
  LogLevel log_level{LogLevel::Info};

  std::filesystem::path rom_directory{""};
  std::filesystem::path last_rom_path{""};
};

}