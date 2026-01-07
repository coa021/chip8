#pragma once
#include "types.hpp"

#include <chrono>
#include <functional>


namespace chip8 {

struct TimerState {
  Byte delay_timer{0};
  Byte sound_timer{0};

  constexpr bool is_sound_active() const noexcept { return sound_timer > 0; }
};

class Timers {
public:
  using Clock = std::chrono::high_resolution_clock;
  using Duration = std::chrono::duration<double>;
  using TimePoint = Clock::time_point;

  using SoundCallback = std::function<void(bool playing)>;

  static constexpr double FREQUENCY_HZ{60.0};
  static constexpr Duration TICK_PERIOD{1.0 / FREQUENCY_HZ};

  Timers() : m_Last_tick{Clock::now()} {
  }

  Byte delay() const noexcept { return m_State.delay_timer; }
  Byte sound() const noexcept { return m_State.sound_timer; }
  const TimerState &state() const noexcept { return m_State; }


  void set_delay(Byte value) noexcept { m_State.delay_timer = value; }

  void set_sound(Byte value) noexcept {
    const bool was_active{m_State.is_sound_active()};
    m_State.sound_timer = value;
    const bool is_active{m_State.is_sound_active()};

    if (was_active != is_active && m_Sound_callback)
      m_Sound_callback(is_active);
  }


  int update() noexcept {
    const auto now{Clock::now()};
    const Duration elapsed{now - m_Last_tick};

    const int ticks{static_cast<int>(elapsed / TICK_PERIOD)};

    if (ticks > 0) {
      m_Last_tick += std::chrono::duration<Clock::duration>(
          Duration{ticks * TICK_PERIOD.count()});

      decrement_timers(ticks);
    }
    return ticks;
  }

  void tick() noexcept {
    decrement_timers(1);
  }


  bool is_sound_playing() const noexcept { return m_State.is_sound_active(); }

  void set_sound_callback(SoundCallback callback) {
    m_Sound_callback = std::move(callback);
  }

  void reset() noexcept {
    const bool was_active{m_State.is_sound_active()};
    m_State.delay_timer = 0;
    m_State.sound_timer = 0;
    m_Last_tick = Clock::now();

    if (was_active && m_Sound_callback)
      m_Sound_callback(false);
  }

  Duration time_since_tick() const noexcept {
    return Clock::now() - m_Last_tick;
  }

  Duration time_until_tick() const noexcept {
    const Duration elapsed{time_since_tick()};
    if (elapsed >= TICK_PERIOD)
      return Duration::zero();
    return TICK_PERIOD - elapsed;
  }

private:
  void decrement_timers(int ticks) noexcept {
    const bool was_sound_active{m_State.is_sound_active()};

    // decrement delay timer
    if (m_State.delay_timer > 0)
      m_State.delay_timer = static_cast<Byte>(std::max(
          0, static_cast<int>(m_State.delay_timer) - ticks));

    // decrement sound timer
    if (m_State.sound_timer > 0)
      m_State.sound_timer = static_cast<Byte>(std::max(
          0, static_cast<int>(m_State.sound_timer) - ticks));

    const bool is_sound_active{m_State.is_sound_active()};

    if (was_sound_active != is_sound_active && m_Sound_callback)
      m_Sound_callback(is_sound_active);
  }

  TimerState m_State;
  TimePoint m_Last_tick;
  SoundCallback m_Sound_callback;
};


}