# Huginn Build Environment Setup

This document explains how to properly configure your build environment to avoid header conflicts between MSYS2 and MSVC.

## Problem

The build system is currently picking up MSYS2 headers (`C:\msys64\ucrt64\include\stddef.h`) when using the MSVC compiler, causing redefinition errors for fundamental types like `uintptr_t`.

## Solutions

### Option 1: Use Visual Studio Developer Command Prompt (Recommended for MSVC)

1. Open "Developer Command Prompt for VS 2022" from the Start Menu
2. Navigate to your project directory
3. Use the build scripts:
   ```cmd
   # For Windows with MSVC
   build.bat
   ```

### Option 2: Clean Environment Variables

If using regular command prompt, ensure MSYS2 paths are not in your environment:

```cmd
# Temporarily remove MSYS2 from PATH
set PATH=%PATH:C:\msys64\ucrt64\bin;=%
set PATH=%PATH:C:\msys64\mingw64\bin;=%

# Clear package config paths
set PKG_CONFIG_PATH=
set CMAKE_PREFIX_PATH=
```

### Option 3: Use CMake Presets (Recommended)

Use the provided CMake presets which isolate toolchains:

```cmd
# Configure with MSVC
cmake --preset msvc-x64-debug

# Build
cmake --build --preset msvc-x64-debug

# Or for GCC (in MSYS2 environment)
cmake --preset gcc-x64-debug
cmake --build --preset gcc-x64-debug
```

### Option 4: Manual CMake Configuration

For MSVC (clean environment):
```cmd
mkdir build\msvc-debug
cd build\msvc-debug
cmake ..\.. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl
ninja
```

For GCC (MSYS2 environment):
```bash
mkdir -p build/gcc-debug
cd build/gcc-debug
cmake ../.. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
ninja
```

## Verification

After configuring, check that the correct compiler is being used:

```cmd
# Should show MSVC version
cl

# Should NOT show MSYS2 paths in includes
cmake --preset msvc-x64-debug --debug-output | grep "include"
```

## GTest Installation

### For MSVC: Use vcpkg
```cmd
# Install vcpkg if not already installed
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# Install GTest
.\vcpkg install gtest:x64-windows
```

### For GCC: Use MSYS2
```bash
# In MSYS2 terminal
pacman -S mingw-w64-ucrt-x86_64-gtest
```

## Troubleshooting

### If you see `uintptr_t` redefinition errors:
- You're mixing MSYS2 headers with MSVC compiler
- Use Option 1 (Developer Command Prompt) or Option 2 (clean environment)

### If CMake can't find the compiler:
- Ensure you're using the correct preset for your toolchain
- Check that the compiler is in your PATH

### If GTest is not found:
- For MSVC: Install via vcpkg and use `CMAKE_TOOLCHAIN_FILE`
- For GCC: Install via MSYS2 package manager