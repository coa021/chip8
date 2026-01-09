#pragma once
#include "i_audio.hpp"
#include "raylib.h"

#include <array>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace chip8 {

class Beeper : public IAudio {
public:
  static constexpr float DEFAULT_FREQUENCY{440.0f};
  static constexpr float DEFAULT_VOLUME{0.3f};
  static constexpr float SIGNED_SAMPLE_16BIT{32767.0f};

  static constexpr int SAMPLE_RATE{44100};
  static constexpr int BUFFER_SIZE{1024};

  Beeper() = default;

  ~Beeper() override {
    if (m_Initialized)
      shutdown();
  }

  bool initialize() override {
    if (m_Initialized)
      return true;

    InitAudioDevice();
    if (!IsAudioDeviceReady())
      return false;

    generate_wave();

    m_Stream = LoadAudioStream(SAMPLE_RATE, 16, 1);
    SetAudioStreamVolume(m_Stream, m_Volume);

    m_Initialized = true;
    return true;
  }

  void shutdown() override {
    if (!m_Initialized)
      return;

    stop_beep();
    UnloadAudioStream(m_Stream);
    CloseAudioDevice();

    m_Initialized = false;
  }

  void start_beep() override {
    if (!m_Initialized || m_Playing)
      return;
    UpdateAudioStream(m_Stream, m_Buffer.data(), BUFFER_SIZE);

    PlayAudioStream(m_Stream);
    m_Playing = true;
  }

  void stop_beep() override {
    if (!m_Initialized || !m_Playing)
      return;

    StopAudioStream(m_Stream);
    m_Playing = false;
  }

  bool is_playing() const override {
    return m_Playing && IsAudioStreamPlaying(m_Stream);
  }

  void set_frequency(float freq) override {
    m_Frequency = std::clamp(freq, 20.0f, 20000.0f);
    if (m_Initialized)
      generate_wave();
  }

  void set_volume(float volume) override {
    m_Volume = std::clamp(volume, 0.0f, 1.0f);
    if (m_Initialized)
      SetAudioStreamVolume(m_Stream, m_Volume);
  }

  void update() {
    if (!m_Initialized || !m_Playing)
      return;

    if (IsAudioStreamProcessed(m_Stream)) {
      UpdateAudioStream(m_Stream, m_Buffer.data(), BUFFER_SIZE);
    }
  }

private:
  void generate_wave() {
    const float period{static_cast<float>(SAMPLE_RATE) / m_Frequency};
    for (std::size_t i{0}; i < BUFFER_SIZE; ++i) {
      const float t{static_cast<float>(i) / period};
      const float phase{t - std::floor(t)};

      float sample;
      if (phase < 0.5f)
        sample = 1.0f;
      else
        sample = -1.0f;

      m_Buffer[i] = static_cast<std::int16_t>(
        sample * SIGNED_SAMPLE_16BIT * 0.5f);
    }
  }

  float m_Frequency{DEFAULT_FREQUENCY};
  float m_Volume{DEFAULT_VOLUME};
  bool m_Initialized{false};
  bool m_Playing{false};

  AudioStream m_Stream{};
  std::array<int16_t, BUFFER_SIZE> m_Buffer{};
};


}