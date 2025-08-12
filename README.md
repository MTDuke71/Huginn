# Huginn
Huginn: Odin’s Thought, in Every Move

# MSYS2 ucrt64 C++ Build Instructions

## Prerequisites
- Install MSYS2 from https://www.msys2.org/
- Open the MSYS2 terminal and run:
  ```sh
  pacman -Syu
  pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain cmake
  ```
- Use the "MSYS2 MinGW UCRT64" terminal for building C++ projects.

## Build Instructions (CMake)
1. Open the "MSYS2 MinGW UCRT64" terminal.
2. Navigate to your project directory:
   ```sh
   cd /d/repos/Huginn
   ```
3. Create a build directory and run CMake:
   ```sh
   mkdir build
   cd build
   cmake -G "MSYS Makefiles" ..
   cmake --build .
   ```
4. Run the program:
   ```sh
   ./hello.exe
   ```

## Running Tests with CTest
1. After building, run:
   ```sh
   ctest
   ```
2. To see detailed output:
   ```sh
   ctest --verbose
   ```

## Notes
- GoogleTest is automatically downloaded and built via CMake.
- You can add more tests in `src/test.cpp`.
- For more advanced builds, edit `CMakeLists.txt`.
