#include "catch2/catch_test_macros.hpp"
#include "core/instruction.hpp"
#include "core/types.hpp"
#include "utils/config.hpp"

using namespace chip8;

TEST_CASE("Instruction category extracts correct bits", "[instruction]") {
  Opcode op{0xABCD};

  REQUIRE(opcode_bits::category(op) == 0x0A);
}


TEST_CASE("Instruction n extracts correct bits", "[instruction]") {
  Opcode op{0xABCD};

  REQUIRE(opcode_bits::n(op) == 0x0D);
}


TEST_CASE("Instruction nn extracts correct bits", "[instruction]") {
  Opcode op{0xABCD};

  REQUIRE(opcode_bits::nn(op) == 0xCD);
}


TEST_CASE("Instruction nnn return correct Address", "[instruction]") {
  Opcode op{0xABCD};
  Address addr{opcode_bits::nnn(op)};

  REQUIRE(addr.get() == 0x0BCD);
}


TEST_CASE("Instruction x_reg returns correct Register Index", "[instruction]") {
  Opcode op{0xABCD};

  REQUIRE(opcode_bits::x_reg(op).get() == 0x0B);
}

TEST_CASE("Instruction y_reg returns correct Register Index", "[instruction]") {
  Opcode op{0xABCD};

  REQUIRE(opcode_bits::y_reg(op).get() == 0x0C);
}