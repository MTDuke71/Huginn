Huginn

Huginn: Odin’s Thought, in Every Move

MSYS2 UCRT64 C++ Build Instructions
Prerequisites

Install MSYS2: https://www.msys2.org/

Open MSYS2 MinGW UCRT64 shell (not the plain MSYS shell).

Update & install toolchain:

pacman -Syu
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain cmake ninja

Build (CMake + Ninja)
# In the MSYS2 MinGW UCRT64 shell
cd /d/repos/Huginn
mkdir -p build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build build

Run the program
./build/huginn.exe

Running Tests (CTest)
# Build test binary
cmake --build build --target huginn_tests
# Run tests
ctest --test-dir build --output-on-failure -V

Convenience: check target

After the first configure, you can just:

cmake --build build --target check


This builds huginn_tests and then runs ctest.

GoogleTest Options
Option A: Vendored (recommended for Windows/MinGW)

Add googletest source at external/googletest/.

In your top-level CMakeLists.txt:

# Vendored googletest
set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)  # robust on MinGW
add_subdirectory(external/googletest)

enable_testing()
include(GoogleTest)

add_executable(huginn_tests
  test/test_board120.cpp
  test/test_chess_types.cpp
  test/test_perft.cpp
  test/test_main.cpp    # provides main()
)
target_compile_features(huginn_tests PRIVATE cxx_std_17)
target_include_directories(huginn_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(huginn_tests PRIVATE GTest::gtest)

# Discover at test time (prevents NOT_BUILT placeholders)
set(GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(huginn_tests DISCOVERY_TIMEOUT 30)

# Convenience target
add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --test-dir ${CMAKE_BINARY_DIR}
  DEPENDS huginn_tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

Option B: Use MSYS2’s packaged googletest
pacman -S --needed mingw-w64-ucrt-x86_64-gtest


Then:

find_package(GTest CONFIG REQUIRED)
target_link_libraries(huginn_tests PRIVATE GTest::gtest)  # or GTest::gtest_main if you omit test_main.cpp


Avoid mixing system gtest headers with vendored libs (or vice-versa).

PowerShell (alternate workflow)

If you prefer PowerShell outside MSYS2:

# Clean build
Remove-Item -Recurse -Force build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target huginn
cmake --build build --target huginn_tests
ctest --test-dir build -C Debug --output-on-failure -V

Notes & Tips

Generator: Using Ninja keeps toolchains consistent and avoids Visual Studio/MinGW mismatches.

Cleaning: If builds get weird, delete the build dir and reconfigure.

MSYS2: rm -rf build

PowerShell: Remove-Item -Recurse -Force build

Where to add tests: test/*.cpp (not src/). Targets: huginn (app) and huginn_tests (tests).

Debug asserts: Enable in Debug builds; they’re your defensive-programming guardrails.