#pragma once
#include "config.hpp"

#include <string>


namespace chip8 {

struct CommandLineArgs {
  std::string rom_path;
  Config config;

  bool help{false};
  bool version{false};
};


class ArgumentParser {
public:
  static std::optional<CommandLineArgs> parse(int argc, char *argv[]) {
    CommandLineArgs result;
    std::cerr << "argc: " << argc << "\n";
    for (int i{1}; i < argc; ++i) {
      std::cerr << "argv[" << i << "]: '" << argv[i] << "'\n";
      std::string_view arg{argv[i]};

      if (arg == "-h" || arg == "--help") {
        result.help = true;
        return result;
      } else if (arg == "-v" || arg == "--version") {
        result.version = true;
        return result;
      } else if (arg == "-s" || arg == "--scale") {
        if (i + 1 >= argc) {
          std::cerr << "Error: --scale required a value\n";
          return std::nullopt;
        }
        result.config.display_scale = std::atoi(argv[++i]);
      } else if (arg == "-f" || arg == "--frequency") {
        if (i + 1 >= argc) {
          std::cerr << "Error: --frequency required a value\n";
          return std::nullopt;
        }
        result.config.cpu_frequency = std::atof(argv[++i]);
      } else if (arg == "--fullscreen") {
        result.config.start_fullscreen = true;
      } else if (arg == "--no-audio") {
        result.config.audio_enabled = false;
      } else if (arg[0] == '-') {
        std::cerr << std::format("Error: unknown option {}", arg);
        return std::nullopt;
      } else {
        if (result.rom_path.empty())
          result.rom_path = arg;
        else {
          result.rom_path += "";
          result.rom_path += arg;
        }
      }
    }

    if (result.rom_path.empty() && !result.help && !result.version) {
      std::cerr << "Error: No ROM file specifiedn\n";
      return std::nullopt;
    }
    return result;
  }

  static void print_help() {
    std::cout << HELP_TEXT << '\n';
  }

  static void print_version() {
    std::cout << VERSION_INFO << '\n';
  }

private:
  static constexpr std::string_view HELP_TEXT{R"(
USAGE:
  chip8 [OPTIONS] <rom>
  chip8 --help

ARGUMENTS:
  <rom>                   Path to chip8 rom file

OPTIONS:
  -h, --help              Show this help message
  -v, --version           Show version
  -s, --scale <N>         Set display scale factor (1-32, 12 is default)
  -f, --frequency <N>     Set CPU frequency in Hz (1-10k, 500 is default)
  --fullscreen            Start in fullscreen mode
  --no-audio              Disable audio

EXAMPLES:
  chip8 roms/pong.ch8
  chip8 --scale 5 --fullscreen game.rom
  chip8 -f 1000 game.ch8
)"};

  static constexpr std::string_view VERSION_INFO{R"(
Chip8 Interpreter v1.0.0
)"};
};


}