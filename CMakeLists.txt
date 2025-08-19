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

# ---- Library ----
add_library(huginn_lib
  src/move_do.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
  src/attacks.cpp
  src/movegen_simple.cpp
)
target_include_directories(huginn_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(huginn_lib PUBLIC cxx_std_17)

# ---- App ----
add_executable(huginn
  src/main.cpp
)
target_link_libraries(huginn huginn_lib)
target_include_directories(huginn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

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
  test/debug_e5d7_position.cpp  # <-- debug test for e5d7 position
  test/debug_a2a4_position.cpp  # <-- debug test for a2a4 position
)
target_link_libraries(huginn_tests PRIVATE GTest::gtest huginn_lib)
target_include_directories(huginn_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

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