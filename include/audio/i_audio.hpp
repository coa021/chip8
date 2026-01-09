#pragma once


namespace chip8 {


class IAudio {
public:
  virtual ~IAudio() = default;

  virtual bool initialize() = 0;
  virtual void shutdown() = 0;
  virtual void start_beep() = 0;
  virtual void stop_beep() = 0;

  virtual bool is_playing() const = 0;
  virtual void set_frequency(float freq) = 0;
  virtual void set_volume(float volume) = 0;

protected:
  IAudio() = default;
  IAudio(const IAudio &) = delete;
  IAudio &operator=(const IAudio &) = delete;
  IAudio(IAudio &&) = delete;
  IAudio &operator=(IAudio &&) = delete;


};


}