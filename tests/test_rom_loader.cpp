#include "catch2/catch_test_macros.hpp"
#include "utils/rom_loader.hpp"

TEST_CASE("RomLoader loads valid ROM file", "[rom]") {
  std::filesystem::path path{"roms/programs/Chip8 Picture.ch8"};
  auto result{chip8::RomLoader::load(path)};

  REQUIRE(result.is_ok());
  REQUIRE(result.value().size() == 164);
}


TEST_CASE("RomLoader loads non existant ROM file", "[rom]") {
  std::filesystem::path path{"roms/programs/this file does not exist.ch8"};
  auto result{chip8::RomLoader::load(path)};

  REQUIRE(result.is_err());
  REQUIRE(result.error().category() == chip8::Error::Category::IO);
}