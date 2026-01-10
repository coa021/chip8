#pragma once
#include "types.hpp"

#include <variant>

namespace chip8 {

namespace instructions {
/// 00E0 Clear Display
struct ClearDisplay {
  static constexpr std::string_view mnemonic() noexcept { return "CLS"; }
};

/// 00EE Return from subroutine
struct Return {
  static constexpr std::string_view mnemonic() noexcept { return "RET"; }
};

/// 0NNN Call machine code routine
struct SysCall {
  Address address;
  static constexpr std::string_view mnemonic() noexcept { return "SYS"; }
};

/// 1NNN Jump to address
struct Jump {
  Address address;
  static constexpr std::string_view mnemonic() noexcept { return "JP"; }
};

/// 2NNN Call subroutine
struct Call {
  Address address;
  static constexpr std::string_view mnemonic() noexcept { return "CALL"; }
};

/// 3XNN Skip if VX equals NN
struct SkipIfEqual {
  RegisterIndex reg;
  Byte value;
  static constexpr std::string_view mnemonic() noexcept { return "SE"; }
};

/// 3XNN Skip if VX not equals NN
struct SkipIfNotEqual {
  RegisterIndex reg;
  Byte value;
  static constexpr std::string_view mnemonic() noexcept { return "SNE"; }
};

/// 5XY0 Skip if VX == VY
struct SkipIfRegistersEqual {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "SE"; }
};

/// 6XNN Set VX to NN
struct LoadImmediate {
  RegisterIndex reg;
  Byte value;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// 7XNN Add NN to VX, no carry flag
struct AddImmediate {
  RegisterIndex reg;
  Byte value;
  static constexpr std::string_view mnemonic() noexcept { return "ADD"; }
};

/// 8XY0 Set VX to VY
struct LoadRegister {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};


/// 8XY1  VX = VX OR VY
struct Or {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "OR"; }
};


/// 8XY2 VX = VX AND VY
struct And {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "AND"; }
};


/// 8XY3 VX = VX XOR VY
struct Xor {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "XOR"; }
};

/// 8XY4 VX = VX + VY (VF = carry)
struct AddRegisters {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "ADD"; }
};

/// 8XY5 VX = VX - VY (VF = NOT BORROW)
struct SubRegisters {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "SUB"; }
};

/// 8XY6 VX = Vy >> 1 (VF = lsb before shift)
struct ShiftRight {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "SHR"; }
};

/// 8XY7 VX = VY - VX (VF = NOT BORROW)
struct SubRegistersReverse {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "SUBN"; }
};

/// 8XY6 VX = Vy << 1 (VF = msb before shift)
struct ShiftLeft {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "SHL"; }
};

/// 9XY0 skip if VX not equals VY
struct SkipIfRegistersNotEqual {
  RegisterIndex x;
  RegisterIndex y;
  static constexpr std::string_view mnemonic() noexcept { return "SNE"; }
};

/// ANNN Set index register to NNN
struct LoadIndex {
  Address address;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// BNNN jump to NNN + V0
struct JumpOffset {
  Address address;
  static constexpr std::string_view mnemonic() noexcept { return "JP"; }
};

/// CXNN  VX = random AND NN
struct Random {
  RegisterIndex reg;
  Byte mask;
  static constexpr std::string_view mnemonic() noexcept { return "RND"; }
};

/// DXYN Draw sprite at (VX, VY) with N bytes from memory[I]
struct Draw {
  RegisterIndex x;
  RegisterIndex y;
  Byte height;
  static constexpr std::string_view mnemonic() noexcept { return "DRW"; }
};

/// EX9E skip if key VX is pressed
struct SkipIfKeyPressed {
  RegisterIndex reg;
  static constexpr std::string_view mnemonic() noexcept { return "SKP"; }
};

/// EXA1 skip if key VX is not pressed
struct SkipIfKeyNotPressed {
  RegisterIndex reg;
  static constexpr std::string_view mnemonic() noexcept { return "SKNP"; }
};

/// FX07 vx = delay timer
struct LoadDelayTimer {
  RegisterIndex reg;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// FX0A wait for key press, store in VX
struct WaitForKey {
  RegisterIndex reg;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// FX15 delay timer = VX
struct SetDelayTimer {
  RegisterIndex reg;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// FX18 sound timer
struct SetSoundTimer {
  RegisterIndex reg;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// FX1E I = I + VX
struct AddToIndex {
  RegisterIndex reg;
  static constexpr std::string_view mnemonic() noexcept { return "ADD"; }
};

/// FX29 I = sprite address for digit VX
struct LoadFontSprite {
  RegisterIndex reg;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// FX33 store BCD of VX at I, I+1, I+2
struct StoreBCD {
  RegisterIndex reg;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// FX55 store V0..VX at I..I+X
struct StoreRegisters {
  RegisterIndex max_reg;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// FX65 Load V0..VX from I..I+X
struct LoadRegisters {
  RegisterIndex max_reg;
  static constexpr std::string_view mnemonic() noexcept { return "LD"; }
};

/// Unknown/invalid instruction
struct Unknown {
  Opcode opcode;
  static constexpr std::string_view mnemonic() noexcept { return "???"; }
};
}

using Instruction = std::variant<
  instructions::ClearDisplay,
  instructions::Return,
  instructions::SysCall,
  instructions::Jump,
  instructions::Call,
  instructions::SkipIfEqual,
  instructions::SkipIfNotEqual,
  instructions::SkipIfRegistersEqual,
  instructions::LoadImmediate,
  instructions::AddImmediate,
  instructions::LoadRegister,
  instructions::Or,
  instructions::And,
  instructions::Xor,
  instructions::AddRegisters,
  instructions::SubRegisters,
  instructions::ShiftRight,
  instructions::SubRegistersReverse,
  instructions::ShiftLeft,
  instructions::SkipIfRegistersNotEqual,
  instructions::LoadIndex,
  instructions::JumpOffset,
  instructions::Random,
  instructions::Draw,
  instructions::SkipIfKeyPressed,
  instructions::SkipIfKeyNotPressed,
  instructions::LoadDelayTimer,
  instructions::WaitForKey,
  instructions::SetDelayTimer,
  instructions::SetSoundTimer,
  instructions::AddToIndex,
  instructions::LoadFontSprite,
  instructions::StoreBCD,
  instructions::StoreRegisters,
  instructions::LoadRegisters,
  instructions::Unknown
>;


constexpr Instruction decode(Opcode opcode) noexcept {
  using namespace instructions;
  using namespace opcode_bits;

  const auto cat{category(opcode)};
  const auto x{x_reg(opcode)};
  const auto y{y_reg(opcode)};
  const auto n_val{n(opcode)};
  const auto nn_val{nn(opcode)};
  const auto nnn_val{nnn(opcode)};

  switch (cat) {
  case 0x0:
    switch (opcode.get()) {
    case 0x00E0:
      return ClearDisplay{};
    case 0x00EE:
      return Return{};
    default:
      return SysCall{nnn_val};
    }
  case 0x1:
    return Jump{nnn_val};
  case 0x2:
    return Call{nnn_val};
  case 0x3:
    return SkipIfEqual{x, nn_val};
  case 0x4:
    return SkipIfNotEqual{x, nn_val};
  case 0x5:
    if (n_val == 0)
      return SkipIfRegistersEqual{x, y};
    return Unknown{opcode};

  case 0x6:
    return LoadImmediate{x, nn_val};
  case 0x7:
    return AddImmediate{x, nn_val};

  case 0x8:
    switch (n_val) {
    case 0x0:
      return LoadRegister{x, y};
    case 0x1:
      return Or{x, y};
    case 0x2:
      return And{x, y};
    case 0x3:
      return Xor{x, y};
    case 0x4:
      return AddRegisters{x, y};
    case 0x5:
      return SubRegisters{x, y};
    case 0x6:
      return ShiftRight{x, y};
    case 0x7:
      return SubRegistersReverse{x, y};
    case 0xE:
      return ShiftLeft{x, y};
    default:
      return Unknown{opcode};
    }

  case 0x9:
    if (n_val == 0)
      return SkipIfRegistersNotEqual{x, y};
    return   Unknown{opcode};

  case 0xA:
    return LoadIndex{nnn_val};
  case 0xB:
    return JumpOffset{nnn_val};
  case 0xC:
    return Random{x, nn_val};
  case 0xD:
    return Draw{x, y, n_val};

  case 0xE:
    switch (nn_val) {
    case 0x9E:
      return SkipIfKeyPressed{x};
    case 0xA1:
      return SkipIfKeyNotPressed{x};
    default:
      return Unknown{opcode};
    }
  case 0xF:
    switch (nn_val) {
    case 0x07:
      return LoadDelayTimer{x};
    case 0x0A:
      return WaitForKey{x};
    case 0x15:
      return SetDelayTimer{x};
    case 0x18:
      return SetSoundTimer{x};
    case 0x1E:
      return AddToIndex{x};
    case 0x29:
      return LoadFontSprite{x};
    case 0x33:
      return StoreBCD{x};
    case 0x55:
      return StoreRegisters{x};
    case 0x65:
      return LoadRegisters{x};
    default:
      return Unknown{opcode};
    }
  default:
    return Unknown{opcode};
  }
}


}