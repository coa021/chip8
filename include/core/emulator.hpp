#pragma once
#include "cpu.hpp"
#include "memory.hpp"
#include "timers.hpp"
#include "audio/beeper.hpp"
#include "graphics/Display.hpp"
#include "graphics/renderer.hpp"
#include "input/keyboard.hpp"
#include "input/raylib_key_provider.hpp"
#include "utils/config.hpp"
#include "utils/rom_loader.hpp"

#include <chrono>
#include <cstdint>

namespace chip8 {

enum class EmulatorState {
  Uninitialized,
  Ready,
  Running,
  Paused,
  Stopped,
  Error
};

struct EmulatorStats {
  uint64_t total_cycles{0};
  uint64_t frames_rendered{0};
  double average_fps{0.0};
  double cpu_utilization{0.0};
  std::chrono::steady_clock::time_point start_time;
};

class Emulator {
public:
  using Clock = std::chrono::high_resolution_clock;
  using Duration = std::chrono::duration<double>;

  explicit Emulator(const Config &config = {})
    : m_Config{config},
      m_Memory{},
      m_Timers{},
      m_Display{},
      m_Cpu{m_Memory, m_Timers,
            make_cpu_config(config)},
      m_Renderer{
          config.display_scale},
      m_Audio{},
      m_Keyboard{std::make_shared<RaylibKeyProvider>()} {
    setup_callbacks();
  }

  ~Emulator() {
    shutdown();
  }

  Emulator(const Emulator &) = delete;
  Emulator &operator=(const Emulator &) = delete;
  Emulator(Emulator &&) = delete;
  Emulator &operator=(const Emulator &&) = delete;

  Result<void> initialize() {
    if (m_State != EmulatorState::Uninitialized)
      return Ok();
    LOG_INFO("Initializing CHIP-8 interpreter");

    if (!m_Renderer.initialize()) {
      m_State = EmulatorState::Error;
      return Error::graphics("Failed to initialize renderer");
    }

    if (m_Config.audio_enabled) {
      if (!m_Audio.initialize()) {
        LOG_WARNING("Failed to initialize audio, continuing without sound");
      } else {
        m_Audio.set_frequency(m_Config.beep_frequency);
        m_Audio.set_frequency(m_Config.beep_volume);
      }
    }

    m_State = EmulatorState::Ready;
    LOG_INFO("Emulator initialized");

    return Ok();
  }

  void shutdown() {
    if (m_State == EmulatorState::Uninitialized)
      return;

    LOG_INFO("Shutting down");
    m_State = EmulatorState::Stopped;
    m_Audio.shutdown();
    m_Renderer.shutdown();

    m_State = EmulatorState::Uninitialized;
  }


  Result<void> load_rom(const std::filesystem::path &path) {
    LOG_INFO("Loading ROM: {}", path.string());

    auto rom_result{RomLoader::load(path)};
    if (!rom_result)
      return Error::io(rom_result.error().message());

    auto load_result{m_Memory.load_rom(rom_result->as_span())};
    if (!load_result)
      return load_result;

    m_Cpu.reset();
    m_Display.clear();
    m_Timers.reset();

    m_Current_ROM_path = path;
    m_State = EmulatorState::Ready;

    LOG_INFO("ROM loaded: {} bytes", rom_result->size());

    return Ok();
  }

  void run() {
    if (m_State != EmulatorState::Ready && m_State != EmulatorState::Paused) {
      LOG_WARNING("Cannot run, emulator not ready");
      return;
    }

    m_State = EmulatorState::Running;
    m_Stats.start_time = std::chrono::steady_clock::now();

    LOG_INFO("Emulator started");
  }

  void pause() {
    if (m_State == EmulatorState::Running) {
      m_State = EmulatorState::Paused;
      m_Audio.stop_beep();
      LOG_INFO("Emulator paused");
    }
  }

  void resume() {
    if (m_State == EmulatorState::Paused) {
      m_State = EmulatorState::Running;
      LOG_INFO("Emulator resumed");
    }
  }

  void toggle_pause() {
    if (m_State == EmulatorState::Running)
      pause();
    else if (m_State == EmulatorState::Paused)
      resume();
  }

  void stop() {
    m_State = EmulatorState::Stopped;
    m_Audio.stop_beep();
    LOG_INFO("Emulator stopped");
  }

  void reset() {
    m_Cpu.reset();
    m_Display.clear();
    m_Timers.reset();
    m_Audio.stop_beep();

    if (!m_Current_ROM_path.empty()) {
      m_Memory.clear_program_area();
      auto rom_result{RomLoader::load(m_Current_ROM_path)};
      if (rom_result)
        m_Memory.load_rom(rom_result->as_span());
    }

    m_State = EmulatorState::Ready;
    LOG_INFO("Emulator reset");
  }

  Result<void> update() {
    if (m_Renderer.should_close()) {
      m_State = EmulatorState::Stopped;
      return Ok();
    }

    handle_input();

    if (m_State == EmulatorState::Running) {
      const int cycles_per_frame{
          static_cast<int>(m_Config.cpu_frequency / 60.0)};

      for (int i{0}; i < cycles_per_frame; ++i) {
        auto result{m_Cpu.step()};
        if (!result) {
          LOG_ERROR("CPU Error: {}", result.error().message());
          m_State = EmulatorState::Paused;
          return result;
        }
        ++m_Stats.total_cycles;
      }

      m_Timers.update();
      update_audio(); // update audio based on sound timer
    }
    m_Audio.update(); // update audio stream
    m_Renderer.render_frame(m_Display.buffer());
    ++m_Stats.frames_rendered;

    return Ok();
  }


  EmulatorState state() const noexcept { return m_State; }
  bool is_running() const noexcept { return m_State == EmulatorState::Running; }
  bool is_paused() const noexcept { return m_State == EmulatorState::Paused; }

  bool should_quit() const noexcept {
    return m_State == EmulatorState::Stopped;
  }

  const EmulatorStats &stats() const noexcept { return m_Stats; }
  const Config &config() const noexcept { return m_Config; }
  const CpuState &cpu_state() const noexcept { return m_Cpu.state(); }

  const DisplayBuffer &display_buffer() const noexcept {
    return m_Display.buffer();
  }

  void toggle_fullscreen() { m_Renderer.toggle_fullscreen(); }

private:
  void setup_callbacks() {
    m_Cpu.set_draw([this](Byte x, Byte y, MemoryView sprite) -> bool {
      return m_Display.draw_sprite(x, y, sprite);
    });

    m_Cpu.set_clear_display([this]() { m_Display.clear(); });

    m_Cpu.set_key_check([this](KeyIndex key) -> bool {
      return m_Keyboard.is_key_pressed(key);
    });

    m_Cpu.set_key_wait([this]() -> std::optional<KeyIndex> {
      return m_Keyboard.poll_key_press();
    });

    m_Timers.set_sound_callback([this](bool playing) {
      if (playing)
        m_Audio.start_beep();
      else
        m_Audio.stop_beep();
    });
  }


  void handle_input() {
    m_Keyboard.update();

    // check custom keybinds
    if (m_Keyboard.is_quit_pressed())
      m_State = EmulatorState::Stopped;
    else if (m_Keyboard.is_pause_pressed())
      toggle_pause();
    else if (m_Keyboard.is_reset_pressed()) {
      reset();
      run();
    } else if (m_Keyboard.is_fullscreen_pressed())
      toggle_fullscreen();
  }

  void update_audio() {
    if (m_Timers.is_sound_playing()) {
      if (!m_Audio.is_playing())
        m_Audio.start_beep();
    } else {
      if (m_Audio.is_playing())
        m_Audio.stop_beep();
    }
  }

  static CpuConfig make_cpu_config(const Config &config) {
    return CpuConfig{
        .shift_quirk = config.shift_quirk,
        .load_store_quirk = config.load_store_quirk,
        .jump_quirk = config.jump_quirk,
        .frequency_hz = config.cpu_frequency
    };
  }


  Config m_Config;
  Memory m_Memory;
  Timers m_Timers;
  Display m_Display;
  Cpu m_Cpu;

  RaylibRenderer m_Renderer;
  Beeper m_Audio;
  Keyboard m_Keyboard;

  EmulatorState m_State{EmulatorState::Uninitialized};
  EmulatorStats m_Stats;
  std::filesystem::path m_Current_ROM_path;
};


}