#pragma once
#include "types.hpp"
#include "utils/result.hpp"

#include <format>
#include <stdexcept>
#include <bits/ranges_algobase.h>


namespace chip8 {

class Memory {
public:
  Memory() noexcept {
    clear();
    load_font();
  }

  // read operations
  [[nodiscard]] Byte read(Address addr) const {
    validate_address(addr);
    return m_Data[addr.get()];
  }

  [[nodiscard]] Word read_word(Address addr) const {
    validate_address(addr);
    validate_address(Address{static_cast<Word>(addr.get() + 1)});

    return bits::combine(m_Data[addr.get()], m_Data[addr.get() + 1]);
  }

  [[nodiscard]] Opcode read_opcode(Address addr) const {
    return Opcode{read_word(addr)};
  }


  [[nodiscard]] MemoryView view(Address addr, std::size_t length) const {
    validate_range(addr, length);
    return MemoryView{m_Data.data() + addr.get(), length};
  }

  [[nodiscard]] MemoryView sprite_data(Address addr, Byte height) const {
    return view(addr, height);
  }

  // write operations
  void write(Address addr, Byte value) {
    validate_address(addr);
    m_Data[addr.get()] = value;
  }

  void write_range(Address addr, std::span<const Byte> data) {
    validate_range(addr, data.size());
    std::ranges::copy(data, m_Data.begin() + addr.get());
  }

  // rom loading
  Result<void> load_rom(std::span<const Byte> rom_data) {
    constexpr auto max_rom_size{
        constants::MEMORY_SIZE - constants::PROGRAM_START};

    if (rom_data.empty())
      return Error::io("ROM data is empty");
    if (rom_data.size() > max_rom_size)
      return Error::memory(std::format(
          "ROM too large: {} bytes (max: {} bytes)",
          rom_data.size(), max_rom_size));

    // clear program area
    std::fill(m_Data.begin() + constants::PROGRAM_START, m_Data.end(), Byte{0});
    // copy rom to program memory
    std::ranges::copy(rom_data, m_Data.begin() + constants::PROGRAM_START);

    m_Rom_size = rom_data.size();
    return Ok();
  }

  // memory management
  /// Clear all memory and reload font
  void clear() {
    m_Data.fill(0);
    load_font();
    m_Rom_size = 0;
  }

  /// Clear only the program area, preserve font
  void clear_program_area() noexcept {
    std::fill(m_Data.begin() + constants::PROGRAM_START, m_Data.end(), Byte{0});
    m_Rom_size = 0;
  }

  [[nodiscard]] std::size_t size() const noexcept {
    return constants::MEMORY_SIZE;
  }

  [[nodiscard]] std::size_t rom_size() const noexcept {
    return m_Rom_size;
  }

  // font access
  static constexpr Address font_sprite_address(Byte digit) noexcept {
    // each sprite is 5bytes
    const auto offset{
        static_cast<Word>(digit & 0x0F) * constants::FONT_SPRITE_HEIGHT};
    return Address{static_cast<Word>(constants::FONT_START + offset)};
  }

  static bool is_valid_range(Address addr, std::size_t length) noexcept {
    const auto end{static_cast<std::size_t>(addr.get() + length)};
    return end <= constants::MEMORY_SIZE;
  }

private:
  void load_font() {
    std::ranges::copy(constants::FONT_SET,
                      m_Data.begin() + constants::FONT_START);
  }

  static void validate_address(Address addr) {
    if (addr.get() >= constants::MEMORY_SIZE)
      throw std::out_of_range(std::format(
          "Memory access out of bounds: ${:03X} (max: ${:03X})", addr.get(),
          constants::MEMORY_SIZE - 1));
  }

  static void validate_range(Address addr, std::size_t length) {
    if (!is_valid_range(addr, length))
      throw std::out_of_range(
          std::format(
              "Memory range out of bounds: ${:03X}-${:03X} (max: ${:03})",
              addr.get(), addr.get() + length - 1, constants::MEMORY_SIZE - 1));

  }

  MemoryBuffer m_Data{};
  std::size_t m_Rom_size{0};
};
}