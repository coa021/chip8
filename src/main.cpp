#include "audio/beeper.hpp"
#include "core/emulator.hpp"
#include "utils/argument_parser.hpp"

#include <iostream>
#include <raylib.h>

int main(int argc, char *argv[]) {
  using namespace chip8;

  auto args{ArgumentParser::parse(argc, argv)};
  if (!args) {
    ArgumentParser::print_help();
    return EXIT_FAILURE;
  }

  if (args->help) {
    ArgumentParser::print_help();
    return EXIT_SUCCESS;
  }
  if (args->version) {
    ArgumentParser::print_version();
    return EXIT_SUCCESS;
  }

  Config config{args->config};

  chip8::LogConfig log_config;
  log_config.min_level = args->config.log_level;
  log_config.show_timestamp = true;
  log_config.show_level = true;
  log_config.show_location = args->config.debug_mode;
  chip8::Logger::instance().configure(log_config);

  LOG_INFO("Starting {} v{}", "1.0", "something");

  Emulator emulator{config};

  if (auto result{emulator.initialize()}; !result) {
    LOG_ERROR("Init failed: {}", result.error().message());
    return EXIT_FAILURE;
  }

  if (auto result{emulator.load_rom(args->rom_path)}; !result) {
    LOG_ERROR("ROM load failed: {}", result.error().message());
    return EXIT_FAILURE;
  }

  emulator.run();

  while (!emulator.should_quit()) {
    if (auto result{emulator.update()}; !result) {
      LOG_ERROR("Error: {}", result.error().message());
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;

  return 0;
}