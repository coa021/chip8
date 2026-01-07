cmake_minimum_required(VERSION 3.28)
project(chip8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# raylib fetch
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(OPENGL_VERSION "3.3" CACHE STRING "" FORCE)

FetchContent_MakeAvailable(raylib)

# catch2 fetch
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)


# headers
set(CORE_HEADERS
        include/core/types.hpp
        include/core/memory.hpp
        include/core/instruction.hpp
)
set(UTIL_HEADERS
        include/utils/result.hpp
        include/utils/logger.hpp
        include/utils/rom_loader.hpp
        include/utils/config.hpp
)

add_executable(chip8
        src/main.cpp
        ${CORE_HEADERS}
        ${UTIL_HEADERS}
)

target_link_libraries(chip8 PRIVATE raylib)
target_include_directories(chip8 PRIVATE ${CMAKE_SOURCE_DIR}/include)


add_executable(tests
        tests/testing.cpp
        tests/test_types.cpp
        tests/test_result.cpp
        tests/test_logger.cpp
        tests/test_memory.cpp
        tests/test_rom_loader.cpp
        tests/test_instruction.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

enable_testing()
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(tests)


add_custom_command(TARGET chip8 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/roms
        $<TARGET_FILE_DIR:chip8>/roms
)