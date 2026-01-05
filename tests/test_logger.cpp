#include "catch2/catch_test_macros.hpp"
#include "utils/logger.hpp"

TEST_CASE("Logger respects log level", "[logger]") {
  std::ostringstream output;

  chip8::Logger::instance().configure({
      .min_level = chip8::LogLevel::Warning,
      .show_timestamp = false,
      .show_level = false,
      .show_location = false,
      .output = &output});

  LOG_DEBUG("This should not appear");
  LOG_WARNING("This should appear");

  auto result{output.str()};

  REQUIRE(result.find("should not appear") == std::string::npos);
  REQUIRE(result.find("should appear") != std::string::npos);
}


TEST_CASE("Logger formats messages correctly", "[logger]") {
  std::ostringstream output;

  chip8::Logger::instance().configure({
      .min_level = chip8::LogLevel::Trace,
      .show_timestamp = false,
      .show_level = false,
      .show_location = false,
      .output = &output});

  LOG_INFO("Value: {}", 42);
  REQUIRE(output.str().find("Value: 42") != std::string::npos);
}