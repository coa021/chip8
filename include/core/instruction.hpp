#pragma once
#include "types.hpp"

namespace chip8::opcode_bits {

constexpr Byte category(Opcode op) noexcept {
  return static_cast<Byte>((op.get() >> 12) & 0x0F);
}

constexpr RegisterIndex x_reg(Opcode op) noexcept {
  return RegisterIndex{static_cast<Byte>((op.get() >> 8) & 0x0F)};
}


constexpr RegisterIndex y_reg(Opcode op) noexcept {
  return RegisterIndex{static_cast<Byte>((op.get() >> 4) & 0x0F)};
}

constexpr Byte n(Opcode op) noexcept {
  return static_cast<Byte>(op.get() & 0x0F);
}

constexpr Byte nn(Opcode op) noexcept {
  return static_cast<Byte>(op.get() & 0xFF);
}

constexpr Address nnn(Opcode op) noexcept {
  return Address{static_cast<Word>(op.get() & 0x0FFF)};
}

}