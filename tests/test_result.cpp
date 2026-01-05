#include "catch2/catch_test_macros.hpp"
#include "utils/result.hpp"

TEST_CASE("Result success path", "[result]") {
  chip8::Result<int> r{chip8::Ok(42)};

  REQUIRE(r.is_ok());
  REQUIRE_FALSE(r.is_err());
  REQUIRE(r.value() == 42);
  REQUIRE(r.value_or(0) == 42);
}


TEST_CASE("Result error path", "[result]") {
  chip8::Result<int> r{chip8::Err<int>(chip8::Error::memory("out of bounds"))};

  REQUIRE(r.is_err());
  REQUIRE_FALSE(r.is_ok());
  REQUIRE(r.error().category() == chip8::Error::Category::Memory);
  REQUIRE(r.value_or(99) == 99);
}

TEST_CASE("Result<void>", "[result]") {
  chip8::Result<void> success{chip8::Ok()};
  chip8::Result<void> failure{
      chip8::Err<void>(chip8::Error::io("file not found"))};

  REQUIRE(success.is_ok());
  REQUIRE(failure.is_err());
}

TEST_CASE("Result::map transforms values", "[result]") {
  chip8::Result<int> r{chip8::Ok(12)};
  auto doubled{r.map([](int x) { return x * 2; })};

  REQUIRE(doubled.value() == 24);
}

TEST_CASE("Result::and_then chains operations", "[result]") {
  auto parse_positive{[](int x) -> chip8::Result<int> {
    if (x > 0)
      return chip8::Ok(x);
    return chip8::Err<int>(chip8::Error::config("must be positive"));
  }};

  auto good{chip8::Ok(10)};
  auto bad{chip8::Ok(-10)};

  REQUIRE(
      good.and_then(parse_positive).and_then([](int x){return chip8::Ok(x*x);}).
      value() == 100);
  REQUIRE(bad.and_then(parse_positive).is_err());
}

TEST_CASE("Result::or_else chains operations", "[result]") {
  chip8::Result<int, chip8::Error> err{chip8::Error::io("failed")};

  auto result{err.or_else([](const chip8::Error &) {
    return chip8::Result<int, chip8::Error>{999};
  })};

  REQUIRE(result.is_ok());
  REQUIRE(result.value() == 999);
}

TEST_CASE("Result::or_else passes through Ok", "[result]") {
  chip8::Result<int, chip8::Error> ok{10};
  bool called{false};

  auto result{ok.or_else([&](const chip8::Error &) {
    called = true;
    return chip8::Result<int, chip8::Error>{999};
  })};

  REQUIRE(called == false);
  REQUIRE(ok.value() == 10);
  REQUIRE(ok.is_ok());
}

TEST_CASE("Result::inspect calls function on Ok", "[result]") {
  chip8::Result<int, chip8::Error> ok{10};

  int captured{0};
  ok.inspect([&](const int &val) { captured = val; });

  REQUIRE(captured == 10);
}

TEST_CASE("Result::inspect_err calls function on Error", "[result]") {
  chip8::Result<int, chip8::Error> bad{chip8::Error::io("something broke")};

  std::string captured{};
  bad.inspect_err([&](const chip8::Error &e) { captured = e.message(); });

  REQUIRE(captured == "something broke");
}