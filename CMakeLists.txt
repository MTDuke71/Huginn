cmake_minimum_required(VERSION 3.24)
project(Huginn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Debug Mode Configuration ----
option(ENABLE_DEBUG_ASSERTIONS "Enable debug assertions with file/line info" OFF)
if(ENABLE_DEBUG_ASSERTIONS OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    message(STATUS "Debug assertions enabled")
endif()

# ---- App ----
add_executable(huginn
  src/main.cpp
  src/move_do.cpp        # <-- add this file per our earlier step
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
)
target_include_directories(huginn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(huginn PRIVATE cxx_std_17)

# ---- Tests ----
find_package(GTest CONFIG REQUIRED)  # using MSYS2 packaged googletest

include(CTest)       # adds BUILD_TESTING and CTest config
enable_testing()
include(GoogleTest)

add_executable(huginn_tests
  test/test_board120.cpp
  test/test_chess_types.cpp
  test/test_perft.cpp
  test/test_main.cpp
  test/test_zobrist.cpp
  test/test_bitboard.cpp    # <-- bitboard tests
  test/test_init.cpp        # <-- initialization tests
  test/test_board.cpp       # <-- board management tests
  test/test_offboard_detection.cpp  # <-- offboard detection demo
  test/test_fen.cpp         # <-- FEN parsing tests
  test/test_print_position.cpp      # <-- position printing tests
  test/test_incremental_updates.cpp # <-- incremental update performance tests
  test/test_zobrist_incremental.cpp # <-- zobrist incremental XOR update tests
  test/test_material_tracking.cpp   # <-- material score tracking tests
  test/test_pawn_bitboard.cpp       # <-- combined pawn bitboard tests
  test/test_debug_validation.cpp    # <-- debug validation function tests
  test/test_fen_generation.cpp      # <-- FEN generation function tests
  test/test_sq_attacked.cpp         # <-- SqAttacked function tests
  test/test_sq_attacked_performance.cpp  # <-- SqAttacked performance tests
  src/move_do.cpp        # reuse the implementation in tests
  src/bitboard.cpp       # <-- bitboard implementation for tests
  src/init.cpp           # <-- engine initialization for tests
  src/board.cpp          # <-- board management functions for tests
  src/position.cpp       # <-- position implementation with Zobrist updates for tests
  src/zobrist.cpp        # <-- zobrist implementation for tests
  src/debug.cpp          # <-- debug and validation functions for tests
)
target_compile_features(huginn_tests PRIVATE cxx_std_17)
target_include_directories(huginn_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(huginn_tests PRIVATE GTest::gtest)  # keep test_main.cpp, so no gtest_main

# Discover tests at test time (prevents NOT_BUILT placeholders)
set(GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(huginn_tests DISCOVERY_TIMEOUT 30)

# Convenience: build tests + run them
add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --test-dir ${CMAKE_BINARY_DIR}
  DEPENDS huginn_tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Include debug demo
include(debug_demo.cmake)
