cmake_minimum_required(VERSION 3.24) # or newer
project(Huginn)

set(CMAKE_CXX_STANDARD 17)

add_executable(huginn src/main.cpp)

# --- GoogleTest via FetchContent ---
find_package(GTest CONFIG REQUIRED)  # provided by MSYS2 package

enable_testing()
include(GoogleTest)

add_executable(huginn_tests
  test/test_board120.cpp
  test/test_chess_types.cpp
  test/test_perft.cpp
  test/test_main.cpp
  src/position.hpp
  src/move.hpp
  src/movegen.hpp
)

target_compile_features(huginn_tests PRIVATE cxx_std_17)
target_include_directories(huginn_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(huginn_tests PRIVATE GTest::gtest)  # or GTest::gtest_main if you remove test_main.cpp

set(GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(huginn_tests DISCOVERY_TIMEOUT 30)

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --test-dir ${CMAKE_BINARY_DIR}
    DEPENDS huginn_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)