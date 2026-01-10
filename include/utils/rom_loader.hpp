#pragma once
#include "result.hpp"
#include "core/types.hpp"

#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace chip8 {

class RomData {
public:
  RomData() = default;

  explicit RomData(std::vector<Byte> data) : m_Data{std::move(data)} {
  }

  const Byte *data() const noexcept { return m_Data.data(); }
  std::size_t size() const noexcept { return m_Data.size(); }
  bool empty() const noexcept { return m_Data.empty(); }

  std::span<const Byte> as_span() const noexcept {
    return std::span<const Byte>(m_Data);
  }

  auto begin() noexcept { return m_Data.begin(); }
  auto end() noexcept { return m_Data.end(); }

private:
  std::vector<Byte> m_Data;
};


class RomLoader {
public:
  static constexpr std::size_t MAX_ROM_SIZE{
      constants::MEMORY_SIZE - constants::PROGRAM_START};
  static constexpr std::array<std::string_view, 4> SUPPORTED_EXTENSIONS{
      {".ch8", ".c8", ".rom", ".bin"}};

  static Result<RomData> load(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path))
      return Result<RomData>{
          Error::io(std::format("File not found: {}", path.string()))};

    const auto file_size{std::filesystem::file_size(path)};
    if (file_size == 0)
      return Result<RomData>{Error::io("ROM file is empty")};
    if (file_size > MAX_ROM_SIZE)
      return Result<RomData>{
          Error::io(std::format("ROM too large: {} bytes (max: {} bytes)",
                                file_size, MAX_ROM_SIZE))};

    std::ifstream file(path, std::ios::binary);
    if (!file)
      return Result<RomData>{
          Error::io(std::format("Failed to open file: {}", path.string()))};

    std::vector<Byte> data(file_size);
    file.read(reinterpret_cast<char *>(data.data()),
              static_cast<std::streamsize>(file_size));

    if (!file)
      return Result<RomData>{Error::io("Failed to read ROM data")};

    return Result<RomData>{RomData{std::move(data)}};
  }

  static Result<RomData> load(const std::string &path) {
    return load(std::filesystem::path(path));
  }

  static bool is_supported_extension(const std::filesystem::path &path) {
    const auto ext{path.extension().string()};
    std::string lower_ext;
    lower_ext.reserve(ext.size());
    for (char c : ext)
      lower_ext.push_back(
          static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

    return std::ranges::find(SUPPORTED_EXTENSIONS, lower_ext) !=
           SUPPORTED_EXTENSIONS.end();
  }


  struct RomInfo {
    std::string filename;
    std::size_t size_bytes{0};
    std::size_t instruction_cound{0};
    bool valid{false};
  };

  static RomInfo get_info(const std::filesystem::path &path) {
    RomInfo info{};
    info.filename = path.filename().string();
    info.valid = false;
    if (!std::filesystem::exists(path))
      return info;

    info.size_bytes = std::filesystem::file_size(path);
    info.instruction_cound = info.size_bytes / 2;
    info.valid = info.size_bytes > 0 && info.size_bytes <= MAX_ROM_SIZE;

    return info;
  }

private
:

};

}