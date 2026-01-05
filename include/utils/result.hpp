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

  /// Transforms the success value using func
  /// @tparam F Callable
  /// @param func Transformation function
  /// @return New Result with transformed value or original error
  template <typename F>
    requires std::invocable<F, T &>
  [[nodiscard]] auto
  map(F &&func) & -> Result<std::invoke_result_t<F, T &>, E> {
    using U = std::invoke_result_t<F, T &>;
    if (is_ok())
      return Result<U, E>{
          std::invoke(std::forward<F>(func), std::get<0>(m_Data))};

    return Result<U, E>{error_tag{}, std::get<1>(m_Data)};
  }

  template <typename F>
    requires std::invocable<F, T &&>
  [[nodiscard]] auto
  map(F &&func) && -> Result<std::invoke_result_t<F, T &&>, E> {
    using U = std::invoke_result_t<F, T &&>;
    if (is_ok())
      return Result<U, E>{
          std::invoke(std::forward<F>(func), std::get<0>(std::move(m_Data)))};

    return Result<U, E>{error_tag{}, std::get<1>(std::move(m_Data))};
  }

  /// Transforms the error using func
  /// @tparam F Callable
  /// @param func Eror transformation func
  /// @return New Result with original value or transformed error
  template <typename F>
    requires std::invocable<F, E &>
  [[nodiscard]] auto map_err(
      F &&func) & -> Result<T, std::invoke_result_t<F, E &>> {
    using U = std::invoke_result_t<F, E &>;
    if (is_err())
      return Result<T, U>{error_tag{},
                          std::invoke(std::forward<F>(func),
                                      std::get<1>(m_Data))};

    return Result<T, U>{std::get<0>(m_Data)};
  }

  /// Chains operations that return Result, flattens nested Results
  /// @tparam F Callable
  /// @param func Function returning a new Result
  /// @return Result returned by func or original error
  template <typename F>
    requires std::invocable<F, T &>
  [[nodiscard]] auto and_then(F &&func) & -> std::invoke_result_t<F, T &> {
    if (is_ok())
      return std::invoke(std::forward<F>(func), std::get<0>(m_Data));

    using ReturnType = std::invoke_result_t<F, T &>;
    return ReturnType{error_tag{}, std::get<1>(m_Data)};
  }

  template <typename F>
    requires std::invocable<F, T &&>
  [[nodiscard]] auto and_then(F &&func) && -> std::invoke_result_t<F, T &&> {
    if (is_ok())
      return std::invoke(std::forward<F>(func), std::get<0>(std::move(m_Data)));

    using ReturnType = std::invoke_result_t<F, T &&>;
    return ReturnType{error_tag{}, std::get<1>(std::move(m_Data))};
  }

  /// Handles errors by calling func
  /// @tparam F Callable
  /// @param func Error handler returning a new Result
  /// @return Original Ok, or the Result returned by func
  template <typename F>
    requires std::invocable<F, T &> && std::same_as<
               std::invoke_result_t<F, E &>, Result<T, E>>
  [[nodiscard]] Result<T, E> or_else(F &&func) & {
    if (is_err())
      return std::invoke(std::forward<F>(func), std::get<1>(m_Data));

    return *this;
  }

  /// Calls func with the value if Result is Ok, does nothig if Error
  /// @tparam F Callable that accepts const T&
  /// @param func Function to call
  /// @return Reference to this Result for chainig
  template <typename F>
    requires std::invocable<F, const T &>
  Result &inspect(F &&func) & {
    if (is_ok())
      std::invoke(std::forward<F>(func), std::get<0>(m_Data));

    return *this;
  }

  /// Calls func with the error if Error, does nothing if Ok
  /// @tparam F Callable
  /// @param func Function to call only if Error
  /// @return Reference to *this for chaining
  template <typename F>
    requires std::invocable<F, const E &>
  Result &inspect_err(F &&func) & {
    if (is_err())
      std::invoke(std::forward<F>(func), std::get<1>(m_Data));

    return *this;
  }

private:
  std::variant<T, E> m_Data;
};

// void specialization
template <typename E>
class Result<void, E> {
public:
  struct success_tag {
  };

  struct error_tag {
  };

  Result() : m_Has_error{} {
  }

  Result(success_tag) : m_Has_error{false} {
  }

  Result(E error) : m_Error{std::move(error)}, m_Has_error{true} {
  }

  Result(error_tag, E error) : m_Error{std::move(error)}, m_Has_error{true} {
  }

  [[nodiscard]] constexpr bool is_ok() const noexcept { return !m_Has_error; }
  [[nodiscard]] constexpr bool is_err() const noexcept { return m_Has_error; }

  [[nodiscard]] E &error() & {
    if (!m_Has_error)
      throw std::runtime_error(
          "Attempted to access error of successful Result");

    return m_Error;
  }

  [[nodiscard]] const E &error() const & {
    if (!m_Has_error)
      throw std::runtime_error(
          "Attempted to access error of successful Result");
    return m_Error;
  }

  template <typename F>
    requires std::invocable<F>
  [[nodiscard]] auto and_then(F &&func) -> std::invoke_result_t<F> {
    if (is_ok())
      return std::invoke(std::forward<F>(func));

    using ReturnType = std::invoke_result_t<F>;
    return ReturnType{error_tag{}, m_Error};
  }

  template <typename F>
    requires std::invocable<F, const E &>
  Result &inspect_err(F &&func) {
    if (is_err())
      std::invoke(std::forward<F>(func), m_Error);
    return *this;
  }

private:
  E m_Error;
  bool m_Has_error;
};

// Factory
// success result
template <typename T>
[[nodiscard]] constexpr auto Ok(T &&value) {
  return Result<std::decay_t<T>>{std::forward<T>(value)};
}

// void success result
template <typename T>
[[nodiscard]] inline auto Ok() { return Result<void>{}; };

// error result
template <typename T, typename E>
[[nodiscard]] constexpr auto Err(E &&error) {
  return Result<T, std::decay_t<E>>{
      typename Result<T, std::decay_t<E>>::error_tag{}, std::forward<E>(error)};
}


// error result, deduced from error type
template <typename E>
[[nodiscard]] constexpr auto Err(E &&error) {
  return Result<void, std::decay_t<E>>{
      typename Result<void, std::decay_t<E>>::error_tag{},
      std::forward<E>(error)};
}


}