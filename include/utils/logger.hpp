#pragma once
#include <format>
#include <iostream>
#include <mutex>
#include <source_location>
#include <string_view>

namespace chip8 {

enum class LogLevel {
  Trace, Debug, Info, Warning, Error, Fatal, Off
};

[[nodiscard]] constexpr std::string_view log_level_string(
    LogLevel level) noexcept {
  switch (level) {
  case LogLevel::Trace:
    return "TRACE";
  case LogLevel::Debug:
    return "DEBUG";
  case LogLevel::Info:
    return "INFO";
  case LogLevel::Warning:
    return "WARNING";
  case LogLevel::Error:
    return "ERROR";
  case LogLevel::Fatal:
    return "FATAL";
  case LogLevel::Off:
    return "OFF";
  }
  return "Unknown";
}

struct LogConfig {
  LogLevel min_level{LogLevel::Info};
  bool show_timestamp{true};
  bool show_level{true};
  bool show_location{true};

  std::ostream *output{&std::cerr};
};


class Logger {
public:
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  // Meyers singleton
  static Logger &instance() {
    static Logger logger;
    return logger;
  }

  void configure(const LogConfig &cfg) {
    std::lock_guard lock(m_Mutex);
    m_Config = cfg;
  }

  void set_level(LogLevel level) {
    std::lock_guard lock(m_Mutex);
    m_Config.min_level = level;
  }

  [[nodiscard]] LogLevel level() const { return m_Config.min_level; }

  template <typename... Args>
  void log(LogLevel level, std::source_location loc,
           std::format_string<Args...> fmt, Args &&... args) {
    if (level < m_Config.min_level)
      return;

    std::lock_guard lock{m_Mutex};

    auto &out{*m_Config.output};

    // timestamp
    if (m_Config.show_timestamp) {
      const auto now{std::chrono::system_clock::now()};
      const auto time{std::chrono::system_clock::to_time_t(now)};
      const auto ms{
          std::chrono::duration_cast<std::chrono::milliseconds>(
              now.time_since_epoch()) % 1000};

      std::tm tm_buf{};
      localtime_s(&tm_buf, &time);

      out << std::format("{:02}:{:02}:{:02}.{:03} ",
                         tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec,
                         ms.count());
    }
    // level
    if (m_Config.show_level)
      out << '[' << log_level_string(level) << "] ";
    // location
    if (m_Config.show_location)
      out << std::format("{}:{}", loc.file_name(), loc.line());
    // message
    out << std::format(fmt, std::forward<Args>(args)...) << '\n';
    // flush if error
    if (level >= LogLevel::Error)
      out.flush();
  }

  template <typename... Args>
  void trace(std::source_location loc, std::format_string<Args...> fmt,
             Args &&... args) {
    log(LogLevel::Trace, loc, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void debug(std::source_location loc, std::format_string<Args...> fmt,
             Args &&... args) {
    log(LogLevel::Debug, loc, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void info(std::source_location loc, std::format_string<Args...> fmt,
            Args &&... args) {
    log(LogLevel::Info, loc, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void warning(std::source_location loc, std::format_string<Args...> fmt,
               Args &&... args) {
    log(LogLevel::Warning, loc, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(std::source_location loc, std::format_string<Args...> fmt,
             Args &&... args) {
    log(LogLevel::Error, loc, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void fatal(std::source_location loc, std::format_string<Args...> fmt,
             Args &&... args) {
    log(LogLevel::Fatal, loc, fmt, std::forward<Args>(args)...);
  }

private:
  Logger() = default;
  ~Logger() = default;
  LogConfig m_Config{};
  std::mutex m_Mutex{};
};


class ScopedLogLevel {
public:
  explicit ScopedLogLevel(LogLevel level) : m_Previous_level{
      Logger::instance().level()} {
    Logger::instance().set_level(level);
  }

  ~ScopedLogLevel() {
    Logger::instance().set_level(m_Previous_level);
  }

  ScopedLogLevel(const ScopedLogLevel &) = delete;
  ScopedLogLevel &operator=(const ScopedLogLevel &) = delete;

private:
  LogLevel m_Previous_level;
};

// macros for automatic source location
#define LOG_TRACE(...) ::chip8::Logger::instance().trace(std::source_location::current(), __VA_ARGS__)
#define LOG_DEBUG(...) ::chip8::Logger::instance().debug(std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(...) ::chip8::Logger::instance().info(std::source_location::current(), __VA_ARGS__)
#define LOG_WARNING(...) ::chip8::Logger::instance().warning(std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR(...) ::chip8::Logger::instance().error(std::source_location::current(), __VA_ARGS__)
#define LOG_FATAL(...) ::chip8::Logger::instance().fatal(std::source_location::current(), __VA_ARGS__)

}