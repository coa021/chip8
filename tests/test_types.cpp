#include "catch2/catch_test_macros.hpp"
#include "core/types.hpp"

#include <bitset>
#include <iostream>

TEST_CASE("Hardware constants are correct", "[types][constants]") {
  REQUIRE(chip8::constants::MEMORY_SIZE == 4096);
  REQUIRE(chip8::constants::PROGRAM_START == 0x200);
  REQUIRE(chip8::constants::DISPLAY_WIDTH == 64);
  REQUIRE(chip8::constants::DISPLAY_HEIGHT == 32);
  REQUIRE(chip8::constants::FONT_SET.size() == 80);
}

TEST_CASE("StrongType different types have same underlying value",
          "[types][strongtype]") {
  chip8::Address addr{0x200};
  chip8::Opcode op{0x200};

  // same underlying value, passes
  REQUIRE(addr.get() == op.get());

  // following would not compile, different types
  // REQUIRE(addr == op);
}

TEST_CASE("StrongType arithmetic works correctly",
          "[types][strongtype][arithmetics]") {
  chip8::Address addr{0x200};

  REQUIRE((addr + chip8::Address{2}).get() == 0x202);

  chip8::RegisterValue val{0xFF};
  REQUIRE((val + chip8::RegisterValue{1}).get() == 0x00); // overflows

  REQUIRE((++addr).get() == 0x201);
  REQUIRE((addr++).get() == 0x201);
  REQUIRE(addr.get() == 0x202);

  REQUIRE((--addr).get() == 0x201);
  REQUIRE((addr--).get() == 0x201);
  REQUIRE(addr.get() == 0x200);
}

TEST_CASE("StrongType bitwise operations work",
          "[types][strongtype][bitwise]") {
  chip8::Opcode op{0xA000};
  chip8::Opcode op2{0x000A};

  REQUIRE((op & chip8::Opcode{0xF000}).get() == 0xA000);
  REQUIRE((op | chip8::Opcode{0x0B00}).get() == 0xAB00);
  REQUIRE((op ^ chip8::Opcode{0x0FFF}).get() == 0xAFFF);
  REQUIRE((~op).get() == 0x5FFF);
  REQUIRE((op2 << 12).get() == 0xA000);
  REQUIRE((op >> 12).get() == 0x000A);
}

TEST_CASE("chip8::bits::* functions work", "[types][bits]") {
  REQUIRE(chip8::bits::high_nibble(0xAB) == 0x0A);
  REQUIRE(chip8::bits::low_nibble(0xAB) == 0x0B);

  REQUIRE(chip8::bits::high_byte(0xABCD) == 0xAB);
  REQUIRE(chip8::bits::low_byte(0xABCD) == 0xCD);

  REQUIRE(chip8::bits::combine(0xAB, 0xCD) == 0xABCD);

  chip8::Byte b{0b10000001};
  REQUIRE(chip8::bits::is_bit_set(b, 0) == true);
  REQUIRE(chip8::bits::is_bit_set(b, 1) == false);
  REQUIRE(chip8::bits::is_bit_set(b, 2) == false);
  REQUIRE(chip8::bits::is_bit_set(b, 3) == false);
  REQUIRE(chip8::bits::is_bit_set(b, 4) == false);
  REQUIRE(chip8::bits::is_bit_set(b, 5) == false);
  REQUIRE(chip8::bits::is_bit_set(b, 6) == false);
  REQUIRE(chip8::bits::is_bit_set(b, 7) == true);

  REQUIRE(chip8::bits::msb(0b10000001) == true);
  REQUIRE(chip8::bits::msb(0b01000001) == false);
  REQUIRE(chip8::bits::lsb(0b01000001) == true);
  REQUIRE(chip8::bits::lsb(0b01000010) == false);
}


TEST_CASE("Containers have correct size", "[types][containers]") {
  chip8::Stack stack{};
  chip8::KeyState keys{};
  chip8::RegisterFile regs{};
  chip8::MemoryBuffer memory{};
  chip8::DisplayBuffer display{};

  REQUIRE(stack.size() == chip8::constants::STACK_SIZE);
  REQUIRE(stack.size() == 16);

  REQUIRE(keys.size() == chip8::constants::NUM_KEYS);
  REQUIRE(keys.size() == 16);

  REQUIRE(regs.size() == chip8::constants::NUM_REGISTERS);
  REQUIRE(regs.size() == 16);

  REQUIRE(memory.size() == chip8::constants::MEMORY_SIZE);
  REQUIRE(memory.size() == 4096);

  REQUIRE(display.size() == chip8::constants::DISPLAY_PIXELS);
  REQUIRE(display.size() == 64 * 32);
}