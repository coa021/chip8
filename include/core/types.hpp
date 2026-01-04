#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace chip8::constants
{
    inline constexpr std::size_t MEMORY_SIZE{4096};
    inline constexpr std::size_t PROGRAM_START{0x200};
    inline constexpr std::size_t FONT_START{0x050};
    inline constexpr std::size_t STACK_SIZE{16};

    inline constexpr std::size_t DISPLAY_WIDTH{64};
    inline constexpr std::size_t DISPLAY_HEIGHT{32};
    inline constexpr std::size_t DISPLAY_PIXELS{DISPLAY_WIDTH * DISPLAY_HEIGHT};

    inline constexpr std::size_t NUM_REGISTERS{16};
    inline constexpr std::size_t NUM_KEYS{16};

    inline constexpr double TIMER_FREQUENCY_HZ{60.0};
    inline constexpr double DEFAULT_CPU_FREQUENCY_HZ{500.0};

    inline constexpr std::size_t FONT_SPRITE_HEIGHT{5};
    inline constexpr std::size_t FONT_CHAR_COUNT{16};

    inline constexpr std::array<uint8_t, FONT_CHAR_COUNT * FONT_SPRITE_HEIGHT> FONT_SET{
        {
            {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80 // F
            }
        }
    };
}

template <typename T, typename Tag>
class StrongType
{
public:
    constexpr StrongType() noexcept : m_Value{}
    {
    }

    constexpr explicit StrongType(T val) noexcept : m_Value{val}
    {
    }

    [[nodiscard]] constexpr T get() const noexcept { return m_Value; }
    [[nodiscard]] constexpr explicit operator T() const noexcept { return m_Value; }

    [[nodiscard]] constexpr auto operator<=>(const StrongType&) const noexcept = default;
    [[nodiscard]] constexpr bool operator==(const StrongType&) const noexcept = default;

    // arithmetics
    constexpr StrongType& operator++() noexcept
    {
        ++m_Value;
        return *this;
    }

    constexpr StrongType operator++(int) noexcept
    {
        auto tmp = *this;
        ++m_Value;
        return tmp;
    }

    constexpr StrongType& operator--() noexcept
    {
        --m_Value;
        return *this;
    }

    constexpr StrongType operator--(int) noexcept
    {
        auto tmp = *this;
        --m_Value;
        return tmp;
    }

    [[nodiscard]] constexpr StrongType operator+(const StrongType& other) const noexcept
    {
        return StrongType{static_cast<T>(m_Value + other.m_Value)};
    }

    [[nodiscard]] constexpr StrongType operator-(const StrongType& other) const noexcept
    {
        return StrongType{static_cast<T>(m_Value - other.m_Value)};
    }

    constexpr StrongType& operator+=(const StrongType& other) noexcept
    {
        m_Value += other.m_Value;
        return *this;
    }

    constexpr StrongType& operator-=(const StrongType& other) noexcept
    {
        m_Value -= other.m_Value;
        return *this;
    }

    // bitwise
    [[nodiscard]] constexpr StrongType operator&(const StrongType& other) const noexcept
    {
        return StrongType{static_cast<T>(m_Value & other.m_Value)};
    }

    [[nodiscard]] constexpr StrongType operator|(const StrongType& other) const noexcept
    {
        return StrongType{static_cast<T>(m_Value | other.m_Value)};
    }

    [[nodiscard]] constexpr StrongType operator^(const StrongType& other) const noexcept
    {
        return StrongType{static_cast<T>(m_Value ^ other.m_Value)};
    }

    [[nodiscard]] constexpr StrongType operator~() const noexcept
    {
        return StrongType{static_cast<T>(~m_Value)};
    }

    [[nodiscard]] constexpr StrongType operator<<(int shift) const noexcept
    {
        return StrongType{static_cast<T>(m_Value << shift)};
    }

    [[nodiscard]] constexpr StrongType operator>>(int shift) const noexcept
    {
        return StrongType{static_cast<T>(m_Value >> shift)};
    }

private:
    T m_Value;
};

// type tags to differentiate strong types
namespace tags
{
    struct AddressTag
    {
    };

    struct OpcodeTag
    {
    };

    struct RegisterIndexTag
    {
    };

    struct RegisterValueTag
    {
    };

    struct KeyTag
    {
    };

    struct PixelTag
    {
    };
}

// Strong types
// 12bit address space
using Address = StrongType<std::uint16_t, tags::AddressTag>;

// 16bit opcode
using Opcode = StrongType<std::uint16_t, tags::OpcodeTag>;

// register index (0-15, v0-vf)
using RegisterIndex = StrongType<std::uint8_t, tags::RegisterIndexTag>;

// 8bit register value
using RegisterValue = StrongType<std::uint8_t, tags::RegisterValueTag>;

// key index
using KeyIndex = StrongType<std::uint8_t, tags::KeyTag>;

// aliases for convinience
using Byte = std::uint8_t;
using Word = std::uint16_t;
using DisplayBuffer = std::array<bool, chip8::constants::DISPLAY_PIXELS>;
using MemoryBuffer = std::array<Byte, chip8::constants::MEMORY_SIZE>;
using RegisterFile = std::array<RegisterValue, chip8::constants::NUM_REGISTERS>;
using KeyState = std::array<bool, chip8::constants::NUM_KEYS>;
using Stack = std::array<Address, chip8::constants::STACK_SIZE>;

// safe memory access
using MemoryView = std::span<const Byte>;
using MutableMemoryView = std::span<Byte>;

// bit manipulation
namespace bits
{
    //!
    //! @param value Byte value
    //! @return extracted high nibble from a byte
    [[nodiscard]] constexpr Byte high_nibble(Byte value) noexcept
    {
        return static_cast<Byte>((value >> 4) & 0x0F);
    }

    //!
    //! @param value Byte value
    //! @return extracted low nibble from a byte
    [[nodiscard]] constexpr Byte low_nibble(Byte value) noexcept
    {
        return static_cast<Byte>(value & 0x0F);
    }

    //!
    //! @param high high nibble
    //! @param low low nibble
    //! @return combines two bytes into a big-endian word
    [[nodiscard]] constexpr Word combine(Byte high, Byte low) noexcept
    {
        return static_cast<Word>((static_cast<Word>(high) << 8) | low);
    }

    //! 
    //! @param value Word value
    //! @return extracted high byte from a word
    [[nodiscard]] constexpr Byte high_byte(Word value) noexcept
    {
        return static_cast<Byte>((value >> 8) & 0xFF);
    }

    //!
    //! @param value Word value
    //! @return extracted low byte from a word
    [[nodiscard]] constexpr Byte low_byte(Word value) noexcept
    {
        return static_cast<Byte>(value & 0xFF);
    }

    /// 
    /// @param value Byte value
    /// @param bit_position bit position
    /// @return true/false if the value has a bit set on bit_position
    [[nodiscard]] constexpr bool is_bit_set(Byte value, int bit_position) noexcept
    {
        return (value & (1 << bit_position)) != 0;
    }

    /// 
    /// @param value Byte
    /// @return get msb
    [[nodiscard]] constexpr bool msb(Byte value) noexcept
    {
        return static_cast<Byte>((value >> 7) & 0x01);
    }

    /// 
    /// @param value Byte
    /// @return get lsb
    [[nodiscard]] constexpr bool lsb(Byte value) noexcept
    {
        return static_cast<Byte>(value & 0x01);
    }
}
