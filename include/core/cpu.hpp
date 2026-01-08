#pragma once

#include "instruction.hpp"
#include "memory.hpp"
#include "timers.hpp"
#include "types.hpp"

#include <random>

namespace chip8 {

struct CpuState {
  RegisterFile registers{}; // V0 - VF
  Address index{0}; // I register
  Address program_counter{constants::PROGRAM_START}; // PC
  Stack stack{}; // call stack
  Byte stack_pointer{0}; // SP
  bool waiting_for_key{false};
  RegisterIndex key_register{0};
};

struct CpuConfig {
  bool shift_quirk{false}; // true = VX >>= 1; false = VX = VY >> 1
  bool load_store_quirk{false}; // true = I unchanged, false I+= X + 1
  bool jump_quirk{false}; // true = BNNN jumps to NNN + V, false = NNN + V0

  double frequency_hz{500.0};
};

class Cpu {
public:
  using KeyCheckFn = std::function<bool(KeyIndex)>;
  using KeyWaitFn = std::function<std::optional<KeyIndex>()>;
  using DrawFn = std::function<bool(Byte x, Byte y, MemoryView sprite)>;
  using ClearDisplayFn = std::function<void()>;

  explicit Cpu(Memory &memory, Timers &timers, CpuConfig config = {})
    : m_Memory{memory},
      m_Timers{timers},
      m_Config{config},
      m_Rng{std::random_device{}()},
      m_Dist{0, 255} {
  }

  [[nodiscard]] RegisterValue reg(RegisterIndex idx) const {
    return m_State.registers[idx.get()];
  }

  void set_reg(RegisterIndex idx, RegisterValue value) {
    m_State.registers[idx.get()] = value;
  }

  void set_reg(RegisterIndex idx, Byte value) {
    m_State.registers[idx.get()] = RegisterValue{value};
  }

  [[nodiscard]] RegisterValue vf() const { return m_State.registers[0xF]; }
  void set_vf(Byte value) { m_State.registers[0xF] = RegisterValue{value}; }

  [[nodiscard]] Address index() const noexcept { return m_State.index; }
  void set_index(Address addr) noexcept { m_State.index = addr; }
  [[nodiscard]] Address pc() const noexcept { return m_State.program_counter; }
  void set_pc(Address addr) noexcept { m_State.program_counter = addr; }

  [[nodiscard]] Byte sp() const noexcept {
    return m_State.program_counter.get();
  }

  [[nodiscard]] const CpuState &state() const noexcept { return m_State; }
  [[nodiscard]] const CpuConfig &config() const noexcept { return m_Config; }

  // callback registration
  void set_key_check(KeyCheckFn fn) { m_Key_check = std::move(fn); }
  void set_key_wait(KeyWaitFn fn) { m_Key_wait = std::move(fn); }
  void set_draw(DrawFn fn) { m_Draw = std::move(fn); }
  void set_clear_display(ClearDisplayFn fn) { m_Clear_display = std::move(fn); }


  // execution
  Result<void> step() {
    if (m_State.waiting_for_key) {
      if (m_Key_wait) {
        if (auto key{m_Key_wait()}) {
          m_State.registers[m_State.key_register.get()] = RegisterValue{
              key->get()};
        } else {
          return Ok(); // no instruction
        }
      } else {
        return Error::runtime("No key wait handler registered");
      }
    }

    const Opcode opcode{m_Memory.read_opcode(m_State.program_counter)};

    const Instruction instr{decode(opcode)};

    // advance PC
    m_State.program_counter = Address{
        static_cast<Word>(m_State.program_counter.get() + 2)};

    return execute(instr);
  }

private:
  Result<void> execute(const Instruction &instr) {
    return std::visit([this](const auto &i) -> Result<void> {
      return execute_impl(i);
    }, instr);
  }

  /// 00E0 Clear display
  Result<void> execute_impl(const instructions::ClearDisplay &) {
    if (m_Clear_display)
      m_Clear_display();
    return Ok();
  }

  /// 00EE Return from subroutine
  Result<void> execute_impl(const instructions::Return &) {
    if (m_State.stack_pointer == 0)
      return Error::stack("Stack underflow on RET");

    --m_State.stack_pointer;
    m_State.program_counter = m_State.stack[m_State.stack_pointer];
    return Ok();
  }

  /// 0NNN system call (mostly ignored on modern hw)
  Result<void> execute_impl(const instructions::SysCall &) {
    return Ok();
  }

  /// 1NNN jump to address
  Result<void> execute_impl(const instructions::Jump &i) {
    m_State.program_counter = i.address;
    return Ok();
  }

  /// 2NNN call subroutine
  Result<void> execute_impl(const instructions::Call &i) {
    if (m_State.stack_pointer >= constants::STACK_SIZE)
      return Error::stack("Stack overflow on CALL");

    m_State.stack[m_State.stack_pointer] = m_State.program_counter;
    ++m_State.stack_pointer;
    m_State.program_counter = i.address;
    return Ok();
  }

  /// 3XNN skip if VX equals NN
  Result<void> execute_impl(const instructions::SkipIfEqual &i) {
    if (reg(i.reg).get() == i.value)
      skip_instruction();
    return Ok();
  }

  /// 4XNN skip if VX not equals NN
  Result<void> execute_impl(const instructions::SkipIfNotEqual &i) {
    if (reg(i.reg).get() != i.value)
      skip_instruction();
    return Ok();
  }

  // 5XY0 skip if VX equals VY
  Result<void> execute_impl(const instructions::SkipIfRegistersEqual &i) {
    if (reg(i.x).get() == reg(i.y).get())
      skip_instruction();
    return Ok();
  }

  // 6XNN load immediate value into VX
  Result<void> execute_impl(const instructions::LoadImmediate &i) {
    set_reg(i.reg, i.value);
    return Ok();
  }

  /// 7XNN add immediate value to VX, no carry
  Result<void> execute_impl(const instructions::AddImmediate &i) {
    const auto result{static_cast<Byte>(reg(i.reg).get() + i.value)};
    set_reg(i.reg, result);
    return Ok();
  }

  /// 8XY0 load VY into VX
  Result<void> execute_impl(const instructions::LoadRegister &i) {
    set_reg(i.x, reg(i.y));
    return Ok();
  }

  /// 8XY1 VX=VX OR VY
  Result<void> execute_impl(const instructions::Or &i) {
    set_reg(i.x, static_cast<Byte>(reg(i.x).get() | reg(i.y).get()));
    set_vf(0);
    return Ok();
  }

  /// 8XY2 VX=VX AND VY
  Result<void> execute_impl(const instructions::And &i) {
    set_reg(i.x, static_cast<Byte>(reg(i.x).get() & reg(i.y).get()));
    set_vf(0);
    return Ok();
  }

  /// 8XY3 VX=VX XOR VY
  Result<void> execute_impl(const instructions::Xor &i) {
    set_reg(i.x, static_cast<Byte>(reg(i.x).get() ^ reg(i.y).get()));
    set_vf(0);
    return Ok();
  }

  /// 8XY4 VX=VX+VY, VF=carry
  Result<void> execute_impl(const instructions::AddRegisters &i) {
    const int sum{reg(i.x).get() + reg(i.y).get()};
    const Byte carry{(sum > 255) ? 1 : 0};
    set_reg(i.x, static_cast<Byte>(sum & 0xFF));
    set_vf(carry);
    return Ok();
  }

  /// 8XY5 VX=VX-VY, VF=not borrow
  Result<void> execute_impl(const instructions::SubRegisters &i) {
    const Byte vx{reg(i.x).get()};
    const Byte vy{reg(i.y).get()};
    const Byte no_borrow{(vx >= vy) ? 1 : 0};
    set_reg(i.x, static_cast<Byte>(vx - vy));
    set_vf(no_borrow);
    return Ok();
  }

  /// 8XY6 shift right
  Result<void> execute_impl(const instructions::ShiftRight &i) {
    const Byte value{m_Config.shift_quirk ? reg(i.x).get() : reg(i.y).get()};
    const Byte lsb{value & 0x01};
    set_reg(i.x, static_cast<Byte>(value >> 1));
    set_vf(lsb);
    return Ok();
  }

  /// 8XY7 VX=VY - VX, VF=not borrow
  Result<void> execute_impl(const instructions::SubRegisters &i) {
    const Byte vx{reg(i.x).get()};
    const Byte vy{reg(i.y).get()};
    const Byte no_borrow{(vy >= vx) ? 1 : 0};
    set_reg(i.x, static_cast<Byte>(vy - vx));
    set_vf(no_borrow);
    return Ok();
  }

  /// 8XYE shift left
  Result<void> execute_impl(const instructions::ShiftLeft &i) {
    const Byte value{m_Config.shift_quirk ? reg(i.x).get() : reg(i.y).get()};
    const Byte msb{(value >> 7) & 0x01};
    set_reg(i.x, static_cast<Byte>(value << 1));
    set_vf(msb);
    return Ok();
  }

  /// 9XY0 skip if VX not equals VY
  Result<void> execute_impl(const instructions::SkipIfRegistersNotEqual &i) {
    if (reg(i.x).get() != reg(i.x).get())
      skip_instruction();

    return Ok();
  }

  /// ANNN set index register
  Result<void> execute_impl(const instructions::LoadIndex &i) {
    m_State.index = i.address;
    return Ok();
  }

  /// BNNN jump with offset
  Result<void> execute_impl(const instructions::JumpOffset &i) {
    const Word offset{
        m_Config.jump_quirk
          ? reg(opcode_bits::x_reg(Opcode{i.address.get()})).get()
          : reg(RegisterIndex{0}.get())};

    m_State.program_counter = Address{
        static_cast<Word>(i.address.get() + offset)};

    return Ok();
  }

  /// CXNN random number
  Result<void> execute_impl(const instructions::Random &i) {
    const Byte random_value{static_cast<Byte>(m_Dist(m_Rng))};
    set_reg(i.reg, static_cast<Byte>(random_value & i.mask));
    return Ok();
  }

  /// DXYN draw sprite
  Result<void> execute_impl(const instructions::Draw &i) {
    if (!m_Draw)
      return Error::runtime("No draw handler registered");

    const Byte x{reg(i.x).get()};
    const Byte y{reg(i.y).get()};
    const auto sprite{m_Memory.sprite_data(m_State.index, i.height)};
    const bool collision{m_Draw(x, y, sprite)};
    set_vf(collision ? 1 : 0);

    return Ok();
  }

  /// EX9E skip if key pressed
  Result<void> execute_impl(const instructions::SkipIfKeyPressed &i) {
    if (m_Key_check) {
      const KeyIndex key{static_cast<Byte>(reg(i.reg.get() & 0x0F))};
      if (m_Key_check(key))
        skip_instruction();
    }
    return Ok();
  }

  /// EXA1 skip if key not pressed
  Result<void> execute_impl(const instructions::SkipIfKeyNotPressed &i) {
    if (m_Key_check) {
      const KeyIndex key{static_cast<Byte>(reg(i.reg.get() & 0x0F))};
      if (!m_Key_check(key))
        skip_instruction();
    } else {
      skip_instruction();
    }
    return Ok();
  }

  /// FX07 load delay timer value
  Result<void> execute_impl(const instructions::LoadDelayTimer &i) {
    set_reg(i.reg, m_Timers.delay());
    return Ok();
  }

  /// FX0A wait for key press
  Result<void> execute_impl(const instructions::WaitForKey &i) {
    m_State.waiting_for_key = true;
    m_State.key_register = i.reg;
    return Ok();
  }

  /// FX15 set delay timer
  Result<void> execute_impl(const instructions::SetDelayTimer &i) {
    m_Timers.set_delay(reg(i.reg).get());
    return Ok();
  }

  /// FX18 set sound timer
  Result<void> execute_impl(const instructions::SetSoundTimer &i) {
    m_Timers.set_sound(reg(i.reg).get());
    return Ok();
  }

  /// FX1E add to index
  Result<void> execute_impl(const instructions::AddToIndex &i) {
    const Word new_index{m_State.index.get() + reg(i.reg).get()};
    m_State.index = Address{new_index};
    return Ok();
  }

  /// FX29 set index to font sprite
  Result<void> execute_impl(const instructions::LoadFontSprite &i) {
    const Byte digit{reg(i.reg).get() & 0x0F};
    m_State.index = Memory::font_sprite_address(digit);
    return Ok();
  }

  /// FX33 store BCD representation
  Result<void> execute_impl(const instructions::StoreBCD &i) {
    const Byte value{reg(i.reg).get()};
    m_Memory.write(m_State.index, static_cast<Byte>(value / 100));
    m_Memory.write(Address{static_cast<Word>(m_State.index.get() + 1)},
                   static_cast<Byte>((value / 10) % 10));

    m_Memory.write(Address{static_cast<Word>(m_State.index.get() + 2)},
                   static_cast<Byte>(value % 10));

    return Ok();
  }

  /// FX55 store registers V0-VX
  Result<void> execute_impl(const instructions::StoreRegisters &i) {
    for (Byte reg_idx{0}; regidx <= i.max_reg.get(); ++reg_idx) {
      m_Memory.write(Address{static_cast<Word>(m_State.index.get() + reg_idx)},
                     m_State.registers[reg_idx].get());
    }

    if (!m_Config.load_store_quirk)
      m_State.index = Address{
          static_cast<Word>(m_State.index.get() + i.max_reg.get() + 1)};

    return Ok();
  }

  /// FX65 load registers V0-VX
  Result<void> execute_impl(const instructions::LoadRegisters &i) {
    for (Byte reg_idx{0}; regidx <= i.max_reg.get(); ++reg_idx) {
      m_State.registers[reg_idx] = RegisterValue{
          m_Memory.read(
              Address{static_cast<Word>(m_State.index.get() + reg_idx)})};
    }

    if (!m_Config.load_store_quirk)
      m_State.index = Address{
          static_cast<Word>(m_State.index.get() + i.max_reg.get() + 1)};

    return Ok();
  }


  /// Unknown instruction
  Result<void> execute_impl(const instructions::Unknown &i) {
    return Error::opcode(std::format("Unknown opcode: ${:04X}",
                                     i.opcode.get()));
  }


  // helpers
  void skip_instruction() noexcept {
    m_State.program_counter = Address{
        static_cast<Word>(m_State.program_counter.get() + 2)};
  }

  Memory &m_Memory;
  Timers &m_Timers;
  CpuConfig m_Config;
  CpuState m_State;

  std::mt19937 m_Rng;
  std::uniform_int_distribution<int> m_Dist;

  // callbacks
  KeyCheckFn m_Key_check;
  KeyWaitFn m_Key_wait;
  DrawFn m_Draw;
  ClearDisplayFn m_Clear_display;
};


}