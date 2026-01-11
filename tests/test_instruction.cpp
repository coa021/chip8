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

TEST_CASE("Decode 00E0 returns Jump", "[instruction]") {
  auto instr{decode(Opcode{0x00E0})};
  REQUIRE(std::holds_alternative<instructions::ClearDisplay>(instr));
}

TEST_CASE("Decode 00EE returns SysCall", "[instruction]") {
  auto instr{decode(Opcode{0x00EE})};
  REQUIRE(std::holds_alternative<instructions::Return>(instr));
}

TEST_CASE("Decode 0NNN returns Jump", "[instruction]") {
  auto instr{decode(Opcode{0x123})};
  REQUIRE(std::holds_alternative<instructions::SysCall>(instr));

  auto &syscall{std::get<instructions::SysCall>(instr)};
  REQUIRE(syscall.address.get() == 0x123);
}


TEST_CASE("Decode 1NNN returns Jump", "[instruction]") {
  auto instr{decode(Opcode{0x1ABC})};
  REQUIRE(std::holds_alternative<instructions::Jump>(instr));

  auto &jump{std::get<instructions::Jump>(instr)};
  REQUIRE(jump.address.get() == 0xABC);
}

TEST_CASE("Decode 2NNN returns Call", "[instruction]") {
  auto instr{decode(Opcode{0x2345})};
  REQUIRE(std::holds_alternative<instructions::Call>(instr));

  auto &call{std::get<instructions::Call>(instr)};
  REQUIRE(call.address.get() == 0x345);
}

TEST_CASE("Decode 3XNN returns SkipIfEqual", "[instruction]") {
  auto instr{decode(Opcode{0x3A45})};
  REQUIRE(std::holds_alternative<instructions::SkipIfEqual>(instr));

  auto &se{std::get<instructions::SkipIfEqual>(instr)};
  REQUIRE(se.reg.get() == 0x0A);
  REQUIRE(se.value == 0x45);
}

TEST_CASE("Decode 4XNN returns SkipIfNotEqual", "[instruction]") {
  auto instr{decode(Opcode{0x4B45})};
  REQUIRE(std::holds_alternative<instructions::SkipIfNotEqual>(instr));

  auto &sne{std::get<instructions::SkipIfNotEqual>(instr)};
  REQUIRE(sne.reg.get() == 0x0B);
  REQUIRE(sne.value == 0x45);
}

TEST_CASE("Decode 5XY0 returns SkipIfRegistersEqual", "[instruction]") {
  auto instr{decode(Opcode{0x5AB0})};
  REQUIRE(std::holds_alternative<instructions::SkipIfRegistersEqual>(instr));

  auto &se{std::get<instructions::SkipIfRegistersEqual>(instr)};
  REQUIRE(se.x.get() == 0x0A);
  REQUIRE(se.y.get() == 0x0B);
}

TEST_CASE("Decode 5XY1 returns Unkown", "[instruction]") {
  auto instr{decode(Opcode{0x5AB1})};
  REQUIRE(std::holds_alternative<instructions::Unknown>(instr));
}

TEST_CASE("Decode 6XNN returns LoadImmediate", "[instruction]") {
  auto instr{decode(Opcode{0x6CFF})};
  REQUIRE(std::holds_alternative<instructions::LoadImmediate>(instr));

  auto &li{std::get<instructions::LoadImmediate>(instr)};
  REQUIRE(li.reg.get() == 0x0C);
  REQUIRE(li.value == 0xFF);
}

TEST_CASE("Decode 7XNN returns AddImmediate", "[instruction]") {
  auto instr{decode(Opcode{0x7C20})};
  REQUIRE(std::holds_alternative<instructions::AddImmediate>(instr));

  auto &ai{std::get<instructions::AddImmediate>(instr)};
  REQUIRE(ai.reg.get() == 0x0C);
  REQUIRE(ai.value == 0x20);
}

TEST_CASE("Decode 8XY0 returns LoadRegister", "[instruction]") {
  auto instr{decode(Opcode{0x8120})};
  REQUIRE(std::holds_alternative<instructions::LoadRegister>(instr));

  auto &lr{std::get<instructions::LoadRegister>(instr)};
  REQUIRE(lr.x.get() == 0x01);
  REQUIRE(lr.y.get() == 0x02);
}

TEST_CASE("Decode 8XY1 returns Or", "[instruction]") {
  auto instr{decode(Opcode{0x8341})};
  REQUIRE(std::holds_alternative<instructions::Or>(instr));

  auto &op{std::get<instructions::Or>(instr)};
  REQUIRE(op.x.get() == 0x03);
  REQUIRE(op.y.get() == 0x04);
}

TEST_CASE("Decode 8XY2 returns And", "[instruction]") {
  auto instr{decode(Opcode{0x8342})};
  REQUIRE(std::holds_alternative<instructions::And>(instr));

  auto &op{std::get<instructions::And>(instr)};
  REQUIRE(op.x.get() == 0x03);
  REQUIRE(op.y.get() == 0x04);
}

TEST_CASE("Decode 8XY3 returns Xor", "[instruction]") {
  auto instr{decode(Opcode{0x8343})};
  REQUIRE(std::holds_alternative<instructions::Xor>(instr));

  auto &op{std::get<instructions::Xor>(instr)};
  REQUIRE(op.x.get() == 0x03);
  REQUIRE(op.y.get() == 0x04);
}

TEST_CASE("Decode 8XY4 returns AddRegisters", "[instruction]") {
  auto instr{decode(Opcode{0x8344})};
  REQUIRE(std::holds_alternative<instructions::AddRegisters>(instr));

  auto &op{std::get<instructions::AddRegisters>(instr)};
  REQUIRE(op.x.get() == 0x03);
  REQUIRE(op.y.get() == 0x04);
}

TEST_CASE("Decode 8XY5 returns SubRegisters", "[instruction]") {
  auto instr{decode(Opcode{0x8345})};
  REQUIRE(std::holds_alternative<instructions::SubRegisters>(instr));

  auto &op{std::get<instructions::SubRegisters>(instr)};
  REQUIRE(op.x.get() == 0x03);
  REQUIRE(op.y.get() == 0x04);
}

TEST_CASE("Decode 8XY6 returns ShiftRight", "[instruction]") {
  auto instr{decode(Opcode{0x8346})};
  REQUIRE(std::holds_alternative<instructions::ShiftRight>(instr));

  auto &op{std::get<instructions::ShiftRight>(instr)};
  REQUIRE(op.x.get() == 0x03);
  REQUIRE(op.y.get() == 0x04);
}

TEST_CASE("Decode 8XY7 returns SubRegistersReverse", "[instruction]") {
  auto instr{decode(Opcode{0x8347})};
  REQUIRE(std::holds_alternative<instructions::SubRegistersReverse>(instr));

  auto &op{std::get<instructions::SubRegistersReverse>(instr)};
  REQUIRE(op.x.get() == 0x03);
  REQUIRE(op.y.get() == 0x04);
}

TEST_CASE("Decode 8XYE returns ShiftLeft", "[instruction]") {
  auto instr{decode(Opcode{0x834E})};
  REQUIRE(std::holds_alternative<instructions::ShiftLeft>(instr));

  auto &op{std::get<instructions::ShiftLeft>(instr)};
  REQUIRE(op.x.get() == 0x03);
  REQUIRE(op.y.get() == 0x04);
}

TEST_CASE("Decode 9XY0 returns SkipIfRegistersNotEqual", "[instruction]") {
  auto instr{decode(Opcode{0x9340})};
  REQUIRE(std::holds_alternative<instructions::SkipIfRegistersNotEqual>(instr));

  auto &op{std::get<instructions::SkipIfRegistersNotEqual>(instr)};
  REQUIRE(op.x.get() == 0x03);
  REQUIRE(op.y.get() == 0x04);
}

TEST_CASE("Decode 9XY1 returns Unknown", "[instruction]") {
  auto instr{decode(Opcode{0x9341})};
  REQUIRE(std::holds_alternative<instructions::Unknown>(instr));
}

TEST_CASE("Decode 0xANNN returns LoadIndex", "[instruction]") {
  auto instr{decode(Opcode{0xA341})};
  REQUIRE(std::holds_alternative<instructions::LoadIndex>(instr));

  auto &idx{std::get<instructions::LoadIndex>(instr)};
  REQUIRE(idx.address.get() == 0x341);
}

TEST_CASE("Decode 0xBNNN returns JumpOffset", "[instruction]") {
  auto instr{decode(Opcode{0xB341})};
  REQUIRE(std::holds_alternative<instructions::JumpOffset>(instr));

  auto &idx{std::get<instructions::JumpOffset>(instr)};
  REQUIRE(idx.address.get() == 0x341);
}

TEST_CASE("Decode 0xCXNN returns Random", "[instruction]") {
  auto instr{decode(Opcode{0xC341})};
  REQUIRE(std::holds_alternative<instructions::Random>(instr));

  auto &rand{std::get<instructions::Random>(instr)};
  REQUIRE(rand.reg.get() == 0x03);
  REQUIRE(rand.mask == 0x41);
}

TEST_CASE("Decode 0xDXYN returns Draw", "[instruction]") {
  auto instr{decode(Opcode{0xD345})};
  REQUIRE(std::holds_alternative<instructions::Draw>(instr));

  auto &draw{std::get<instructions::Draw>(instr)};
  REQUIRE(draw.x.get() == 0x03);
  REQUIRE(draw.y.get() == 0x04);
  REQUIRE(draw.height == 0x5);
}


TEST_CASE("Decode 0xEX9E returns SkipIfKeyPressed", "[instruction]") {
  auto instr{decode(Opcode{0xE39E})};
  REQUIRE(std::holds_alternative<instructions::SkipIfKeyPressed>(instr));

  auto &skip{std::get<instructions::SkipIfKeyPressed>(instr)};
  REQUIRE(skip.reg.get() == 0x03);
}

TEST_CASE("Decode 0xEXA1 returns SkipIfKeyNotPressed", "[instruction]") {
  auto instr{decode(Opcode{0xE3A1})};
  REQUIRE(std::holds_alternative<instructions::SkipIfKeyNotPressed>(instr));

  auto &skip{std::get<instructions::SkipIfKeyNotPressed>(instr)};
  REQUIRE(skip.reg.get() == 0x03);
}

TEST_CASE("Decode 0xEXBB returns Unknown", "[instruction]") {
  auto instr{decode(Opcode{0xE3BB})};
  REQUIRE(std::holds_alternative<instructions::Unknown>(instr));
}


TEST_CASE("Decode 0xFX07 returns LoadDelayTimer", "[instruction]") {
  auto instr{decode(Opcode{0xF307})};
  REQUIRE(std::holds_alternative<instructions::LoadDelayTimer>(instr));

  auto &timer{std::get<instructions::LoadDelayTimer>(instr)};
  REQUIRE(timer.reg.get() == 0x03);
}

TEST_CASE("Decode 0xFX0A returns WaitForKey", "[instruction]") {
  auto instr{decode(Opcode{0xF30A})};
  REQUIRE(std::holds_alternative<instructions::WaitForKey>(instr));

  auto &timer{std::get<instructions::WaitForKey>(instr)};
  REQUIRE(timer.reg.get() == 0x03);
}

TEST_CASE("Decode 0xFX15 returns SetDelayTimer", "[instruction]") {
  auto instr{decode(Opcode{0xF315})};
  REQUIRE(std::holds_alternative<instructions::SetDelayTimer>(instr));

  auto &timer{std::get<instructions::SetDelayTimer>(instr)};
  REQUIRE(timer.reg.get() == 0x03);
}

TEST_CASE("Decode 0xFX18 returns SetSoundTimer", "[instruction]") {
  auto instr{decode(Opcode{0xF318})};
  REQUIRE(std::holds_alternative<instructions::SetSoundTimer>(instr));

  auto &timer{std::get<instructions::SetSoundTimer>(instr)};
  REQUIRE(timer.reg.get() == 0x03);
}

TEST_CASE("Decode 0xFX1E returns AddToIndex", "[instruction]") {
  auto instr{decode(Opcode{0xF31E})};
  REQUIRE(std::holds_alternative<instructions::AddToIndex>(instr));

  auto &timer{std::get<instructions::AddToIndex>(instr)};
  REQUIRE(timer.reg.get() == 0x03);
}

TEST_CASE("Decode 0xFX29 returns LoadFontSprite", "[instruction]") {
  auto instr{decode(Opcode{0xF329})};
  REQUIRE(std::holds_alternative<instructions::LoadFontSprite>(instr));

  auto &timer{std::get<instructions::LoadFontSprite>(instr)};
  REQUIRE(timer.reg.get() == 0x03);
}

TEST_CASE("Decode 0xFX33 returns StoreBCD", "[instruction]") {
  auto instr{decode(Opcode{0xF333})};
  REQUIRE(std::holds_alternative<instructions::StoreBCD>(instr));

  auto &timer{std::get<instructions::StoreBCD>(instr)};
  REQUIRE(timer.reg.get() == 0x03);
}

TEST_CASE("Decode 0xFX55 returns StoreRegisters", "[instruction]") {
  auto instr{decode(Opcode{0xF355})};
  REQUIRE(std::holds_alternative<instructions::StoreRegisters>(instr));

  auto &timer{std::get<instructions::StoreRegisters>(instr)};
  REQUIRE(timer.max_reg.get() == 0x03);
}

TEST_CASE("Decode 0xFX65 returns LoadRegisters", "[instruction]") {
  auto instr{decode(Opcode{0xF365})};
  REQUIRE(std::holds_alternative<instructions::LoadRegisters>(instr));

  auto &timer{std::get<instructions::LoadRegisters>(instr)};
  REQUIRE(timer.max_reg.get() == 0x03);
}

TEST_CASE("Decode 0xFX79 returns Unknown", "[instruction]") {
  auto instr{decode(Opcode{0xF379})};
  REQUIRE(std::holds_alternative<instructions::Unknown>(instr));

}