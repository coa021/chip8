#include "catch2/catch_test_macros.hpp"
#include "core/cpu.hpp"
#include "core/memory.hpp"

using namespace chip8;

class CpuTestClass {
protected:
  Memory memory;
  Timers timers;
  Cpu cpu{memory, timers};

  void load_program(std::initializer_list<Byte> bytes) {
    std::vector<Byte> data(bytes);
    memory.load_rom(data);
  }

  void run(int n) {
    for (int i{0}; i < n; ++i)
      cpu.step();
  }
};

TEST_CASE_METHOD(CpuTestClass, "CPU inits correctly", "[cpu]") {
  REQUIRE(cpu.pc().get() == constants::PROGRAM_START);
  REQUIRE(cpu.index().get() == 0);
  for (int i = 0; i < 16; ++i) {
    REQUIRE(cpu.reg(RegisterIndex{static_cast<Byte>(i)}).get() == 0);
  }
}

TEST_CASE_METHOD(CpuTestClass, "LD Vx, byte", "[cpu][instruction]") {
  load_program({0x6A, 0x42});
  run(1);
  REQUIRE(cpu.reg(RegisterIndex{0xA}).get() == 0x42);
}

TEST_CASE_METHOD(CpuTestClass, "ADD Vx, byte", "[cpu][instruction]") {
  load_program({0x60, 0x10, 0x70, 0x20});
  run(2);
  REQUIRE(cpu.reg(RegisterIndex{0}).get() == 0x30);
}

TEST_CASE_METHOD(CpuTestClass, "JP addr", "[cpu][instruction]") {
  load_program({0x1A, 0xBC});
  run(1);
  REQUIRE(cpu.pc().get() == 0x0ABC);
}

TEST_CASE_METHOD(CpuTestClass, "CALL and RET", "[cpu][instruction]") {
  load_program({
      0x22, 0x06, // CALL 0x206   // 0x200
      0x00, 0x00, // 0x202
      0x00, 0x00, // 0x204
      0x00, 0xEE // RET at 0x206  // 0x206
  });

  run(1);
  REQUIRE(cpu.pc().get() == 0x206);
  run(1);
  REQUIRE(cpu.pc().get() == 0x202);
}

TEST_CASE_METHOD(CpuTestClass, "SE Vx byte skip", "[cpu][instruction]") {
  load_program({0x30, 0x00});
  run(1);
  REQUIRE(cpu.pc().get() == constants::PROGRAM_START + 4);
}

TEST_CASE_METHOD(CpuTestClass, "SE Vx byte no skip", "[cpu][instruction]") {
  load_program({0x30, 0xFF});
  run(1);
  REQUIRE(cpu.pc().get() == constants::PROGRAM_START + 2);
}

TEST_CASE_METHOD(CpuTestClass, "ADD Vx, Vy with carry", "[cpu][instruction]") {
  load_program({0x60, 0xFF, 0x61, 0x02, 0x80, 0x14});
  run(3);
  REQUIRE(cpu.reg(RegisterIndex{0}).get() == 0x01);
  REQUIRE(cpu.vf().get() == 1);
}

TEST_CASE_METHOD(CpuTestClass, "SUB Vx, Vy no borrow", "[cpu][instruction]") {
  load_program({0x60, 0x20, 0x61, 0x10, 0x80, 0x15});
  run(3);
  REQUIRE(cpu.reg(RegisterIndex{0}).get() == 0x10);
  REQUIRE(cpu.vf().get() == 1);
}

TEST_CASE_METHOD(CpuTestClass, "SHR sets VF to LSB", "[cpu][instruction]") {
  load_program({0x60, 0x05, 0x80, 0x06});
  run(2);
  REQUIRE(cpu.reg(RegisterIndex{0}).get() == 0x02);
  REQUIRE(cpu.vf().get() == 1);
}

TEST_CASE_METHOD(CpuTestClass, "LD I, addr", "[cpu][instruction]") {
  load_program({0xA1, 0x23});
  run(1);
  REQUIRE(cpu.index().get() == 0x123);
}

TEST_CASE_METHOD(CpuTestClass, "BCD conversion", "[cpu][instruction]") {
  load_program({0x60, 0xFF, 0xA3, 0x00, 0xF0, 0x33});
  run(3);
  REQUIRE(memory.read(Address{0x300}) == 2);
  REQUIRE(memory.read(Address{0x301}) == 5);
  REQUIRE(memory.read(Address{0x302}) == 5);
}

TEST_CASE("Shift quirk mode", "[cpu][quirk]") {
  Memory memory;
  Timers timers;
  CpuConfig config;
  config.shift_quirk = true;
  Cpu cpu{memory, timers, config};

  std::vector<Byte> prog = {0x60, 0x0F, 0x61, 0xF0, 0x80, 0x16};
  memory.load_rom(prog);

  cpu.step();
  cpu.step();
  cpu.step();

  // needs to shift V0, not V1
  REQUIRE(cpu.reg(RegisterIndex{0}).get() == 0x07);
}

TEST_CASE("Load/store quirk mode", "[cpu][quirk]") {
  Memory memory;
  Timers timers;
  CpuConfig config;
  config.load_store_quirk = true;
  Cpu cpu{memory, timers, config};

  std::vector<Byte> prog = {0xA3, 0x00, 0x60, 0xAA, 0xF0, 0x55};
  memory.load_rom(prog);

  cpu.step();
  cpu.step();
  cpu.step();

  // with quirk, I unchanged
  REQUIRE(cpu.index().get() == 0x300);
}

TEST_CASE("Load/store without quirk modifies I", "[cpu][quirk]") {
  Memory memory;
  Timers timers;
  Cpu cpu{memory, timers};

  std::vector<Byte> prog = {0xA3, 0x00, 0x60, 0xAA, 0xF0, 0x55};
  memory.load_rom(prog);

  cpu.step();
  cpu.step();
  cpu.step();

  // without quirk, I = 0x300 + 0 + 1 = 0x301
  REQUIRE(cpu.index().get() == 0x301);
}
