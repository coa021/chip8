#pragma once
#include <cstdint>
#include <format>
#include <functional>
#include <source_location>
#include <string>
#include <string_view>
#include <variant>


namespace chip8 {
// for curiosity, reimplementing std::expected or rather rust's Result<T,E> type
// why not?

class Error {
public:
  enum class Category {
    None,
    IO,
    Memory,
    InvalidOpcode,
    StackError,
    Config,
    Graphics,
    Audio,
    Input,
    Runtime
  };

  constexpr Error() noexcept : m_Category{Category::None} {
  }

  Error(Category category, std::string message,
        std::source_location loc = std::source_location::current())
    : m_Category{category},
      m_Message{std::move(message)},
      m_File{loc.file_name()},
      m_Function{loc.function_name()},
      m_Line{loc.line()} {
  }

  // factory for categories
  [[nodiscard]] static Error io(std::string msg,
                                std::source_location loc =
                                    std::source_location::current()) {
    return Error{Category::IO, std::move(msg), loc};
  }

  [[nodiscard]] static Error memory(std::string msg,
                                    std::source_location loc =
                                        std::source_location::current()) {
    return Error{Category::Memory, std::move(msg), loc};
  }

  [[nodiscard]] static Error opcode(std::string msg,
                                    std::source_location loc =
                                        std::source_location::current()) {
    return Error{Category::InvalidOpcode, std::move(msg), loc};
  }

  [[nodiscard]] static Error stack(std::string msg,
                                   std::source_location loc =
                                       std::source_location::current()) {
    return Error{Category::StackError, std::move(msg), loc};
  }

  [[nodiscard]] static Error config(std::string msg,
                                    std::source_location loc =
                                        std::source_location::current()) {
    return Error{Category::Config, std::move(msg), loc};
  }

  [[nodiscard]] static Error graphics(std::string msg,
                                      std::source_location loc =
                                          std::source_location::current()) {
    return Error{Category::Graphics, std::move(msg), loc};
  }

  [[nodiscard]] static Error audio(std::string msg,
                                   std::source_location loc =
                                       std::source_location::current()) {
    return Error{Category::Audio, std::move(msg), loc};
  }

  [[nodiscard]] static Error input(std::string msg,
                                   std::source_location loc =
                                       std::source_location::current()) {
    return Error{Category::Input, std::move(msg), loc};
  }

  [[nodiscard]] static Error runtime(std::string msg,
                                     std::source_location loc =
                                         std::source_location::current()) {
    return Error{Category::Runtime, std::move(msg), loc};
  }

  // accessors
  [[nodiscard]] constexpr Category category() const noexcept {
    return m_Category;
  }

  [[nodiscard]] const std::string &message() const noexcept {
    return m_Message;
  }

  [[nodiscard]] std::string_view file() const noexcept {
    return m_File;
  }

  [[nodiscard]] std::string_view function() const noexcept {
    return m_Function;
  }

  [[nodiscard]] constexpr std::uint32_t line() const noexcept {
    return m_Line;
  }

  /// check if this represents an actual error
  /// @return t/f
  [[nodiscard]] constexpr bool is_error() const noexcept {
    return m_Category != Category::None;
  }

  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return is_error();
  }

  [[nodiscard]] constexpr std::string_view category_string() const noexcept {
    switch (m_Category) {
    case Category::None:
      return "None";
    case Category::IO:
      return "IO";
    case Category::Memory:
      return "Memory";
    case Category::InvalidOpcode:
      return "Opcode";
    case Category::StackError:
      return "Stack";
    case Category::Config:
      return "Config";
    case Category::Graphics:
      return "Graphics";
    case Category::Audio:
      return "Audio";
    case Category::Input:
      return "Input";
    case Category::Runtime:
      return "Runtime";
    }
    return "Unknown";
  }

  [[nodiscard]] std::string format() const {
    if (!is_error())
      return "No error";

    return std::format("[{}] {} ({}:{} in {})", category_string(), m_Message,
                       m_File, m_Line, m_Function);
  }

private:
  Category m_Category;
  std::string m_Message;
  std::string_view m_File;
  std::string_view m_Function;
  std::uint32_t m_Line{0};
};

template <typename T, typename E = Error>
class Result {
public:
  struct success_tag {
  };

  struct error_tag {
  };

  explicit Result(T value) : m_Data{std::in_place_index<0>, std::move(value)} {
  }

  Result(success_tag, T value) : m_Data{std::in_place_index<0>,
                                        std::move(value)} {
  }

  Result(error_tag, E error) : m_Data{std::in_place_index<1>,
                                      std::move(error)} {
  }

  explicit Result(E error) requires(!std::same_as<T, E>)
    : m_Data{std::in_place_index<1>, std::move(error)} {
  }

  // check success/failure
  [[nodiscard]] constexpr bool is_ok() const noexcept {
    return m_Data.index() == 0;
  }

  [[nodiscard]] constexpr bool is_err() const noexcept {
    return m_Data.index() == 1;
  }

  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return is_ok();
  }

  // access the value, throws if error
  [[nodiscard]] T &value() & {
    if (is_err())
      throw std::runtime_error("Attempted to access value of error Result");
    return std::get<0>(m_Data);
  }

  [[nodiscard]] const T &value() const & {
    if (is_err())
      throw std::runtime_error("Attempted to access value of error Result");
    return std::get<0>(m_Data);
  }

  [[nodiscard]] T &&value() && {
    if (is_err())
      throw std::runtime_error("Attempted to access value of error Result");
    return std::get<0>(std::move(m_Data));
  }

  [[nodiscard]] E &error() & {
    if (is_ok())
      throw std::runtime_error(
          "Attempted to access error of successful Result");
    return std::get<1>(m_Data);
  }

  [[nodiscard]] const E &error() const & {
    if (is_ok())
      throw std::runtime_error(
          "Attempted to access error of successful Result");
    return std::get<1>(m_Data);
  }

  [[nodiscard]] E &&error() && {
    if (is_ok())
      throw std::runtime_error(
          "Attempted to access error of successful Result");
    return std::get<1>(std::move(m_Data));
  }

  // get value or default
  [[nodiscard]] T value_or(T default_value) const & {
    if (is_ok())
      return std::get<0>(m_Data);

    return default_value;
  }

  [[nodiscard]] T value_or(T default_value) && {
    if (is_ok())
      return std::get<0>(std::move(m_Data));

    return default_value;
  }

  // pointer like access
  [[nodiscard]] T *operator->() {
    return is_ok() ? &std::get<0>(m_Data) : nullptr;
  }

  [[nodiscard]] const T *operator->() const {
    return is_ok() ? &std::get<0>(m_Data) : nullptr;
  }

  [[nodiscard]] T &operator*() { return value(); }
  [[nodiscard]] const T &operator*() const & { return value(); }
  [[nodiscard]] T &&operator*() && { return std::move(*this).value(); }

  // map, transform success value propagate errors
  template <typename F>
    requires std::invocable<F, T &>
  [[nodiscard]] auto map(F &&func) & -> Result<std::invoke_result<F, T &>, E> {
    using U = std::invoke_result_t<F, T &>;
    if (is_ok())
      return Result<U, E>{
          std::invoke(std::forward<F>(func), std::get<0>(m_Data))};

    return Result<U, E>{error_tag{}, std::get<1>(m_Data)};
  }

private:
  std::variant<T, E> m_Data;
};

}