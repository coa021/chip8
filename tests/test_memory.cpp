#include "catch2/catch_test_macros.hpp"
#include "core/memory.hpp"

#include <iostream>

using namespace chip8;

TEST_CASE("Memory initializes with font data", "[memory]") {
  Memory mem;

  Address font_addr{Memory::font_sprite_address(0)};
  REQUIRE(mem.read(font_addr) == 0xF0); // first byte of '0' sprite
}

TEST_CASE("Memory read/write single byte", "[memory]") {
  Memory mem;
  Address addr{0x300};

  mem.write(addr, 0xAB);
  REQUIRE(mem.read(addr) == 0xAB);
}

TEST_CASE("Memory read_word is big endian", "[memory]") {
  Memory mem;
  Address addr{0x300};

  mem.write(addr, 0xAB);
  mem.write(Address{0x301}, 0xCD);

  REQUIRE(mem.read_word(addr) == 0xABCD);
}

TEST_CASE("Memory load_rom succeeds with valid ROM", "[memory]") {
  Memory mem;
  std::array<Byte, 4> rom{{0x00, 0xE0, 0x12, 0x00}};
  auto result{mem.load_rom(rom)};

  REQUIRE(result.is_ok());
  REQUIRE(mem.rom_size() == 4);
  REQUIRE(mem.read(Address{constants::PROGRAM_START}) == 0x00);
  REQUIRE(mem.read(Address{constants::PROGRAM_START + 1}) == 0xE0);
}

TEST_CASE("Memory load_rom fails when ROM too large", "[memory]") {
  Memory mem;
  std::vector<Byte> rom(4096, 0x00);
  auto result{mem.load_rom(rom)};

  REQUIRE(result.is_err());
  REQUIRE(result.error().category() == Error::Category::Memory);
}

TEST_CASE("Memory clear resets to initial state", "[memory]") {
  Memory mem;
  mem.write(Address{0x300}, 0xFF);

  mem.clear();

  REQUIRE(mem.read(Address{0x300}) == 0x00);
  REQUIRE(mem.read(Memory::font_sprite_address(0)) == 0xF0);
}

TEST_CASE("Memory view returns correct span", "[memory]") {
  Memory mem;
  mem.write(Address{0x300}, 0xAA);
  mem.write(Address{0x301}, 0xBB);
  mem.write(Address{0x302}, 0xCC);

  auto span{mem.view(Address{0x300}, 3)};

  REQUIRE(span.size() == 3);
  REQUIRE(span[0] == 0xAA);
  REQUIRE(span[1] == 0xBB);
  REQUIRE(span[2] == 0xCC);
}