# WSL Build Guide for Huginn Chess Engine

This guide documents how to build the Huginn chess engine using GCC in Windows Subsystem for Linux (WSL), including both native Linux builds and cross-compiled Windows executables.

## Performance Comparison

Based on testing with the perft suite demo (quick test mode):

| Build Type | Performance | Notes |
|------------|-------------|--------|
| **MSVC (Recommended)** | **28.6 seconds** | Best performance, native Windows toolchain |
| Historical GCC baseline | 22.5 seconds | Target performance (conditions unknown) |
| WSL GCC Linux | 57.8 seconds | GCC native Linux build in WSL |
| MinGW-w64 Windows .exe | 58.1 seconds | GCC cross-compiled Windows executable |

**Recommendation**: Use MSVC for production builds due to superior performance.

## Prerequisites

### 1. Install WSL2 with Ubuntu
```powershell
# Install WSL2 (if not already installed)
wsl --install -d Ubuntu

# Update to latest version
wsl --update
```

### 2. Install Build Tools in WSL
```bash
# Update package list
sudo apt update

# Install essential build tools
sudo apt install -y build-essential cmake

# For cross-compilation to Windows (optional)
sudo apt install -y mingw-w64

# Verify installations
gcc --version
cmake --version
```

## Building Native Linux Version in WSL

### 1. Configure the Build
```bash
cd /mnt/d/repos/Huginn_New/Huginn
mkdir -p build/wsl-gcc-release
cd build/wsl-gcc-release

# Configure with GCC optimizations
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS_RELEASE="-O3 -march=native -mtune=native -funroll-loops -DNDEBUG -fomit-frame-pointer -flto" \
      ../..
```

### 2. Build the Project
```bash
# Clean build
make clean

# Build with parallel jobs
make -j4

# Or build specific target
make -j4 perft_suite_demo
```

### 3. Test the Build
```bash
# Run performance test
cd /mnt/d/repos/Huginn_New/Huginn
echo '1' | ./build/wsl-gcc-release/bin/perft_suite_demo
```

## Cross-Compiling Windows Executable with MinGW-w64

### 1. Create Toolchain File
Create `mingw-toolchain.cmake`:
```cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-posix)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Override march=native for cross-compilation and add threading support with static linking
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mtune=generic -flto -ffast-math -funroll-loops -finline-functions -DNDEBUG -fomit-frame-pointer -pthread -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto -s -pthread -static-libgcc -static-libstdc++ -static")
```

### 2. Configure Cross-Compilation Build
```bash
cd /mnt/d/repos/Huginn_New/Huginn
mkdir -p build/mingw-w64-release
cd build/mingw-w64-release

# Configure with MinGW-w64 toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=../../mingw-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      ../..
```

### 3. Build Windows Executable
```bash
# Build the Windows .exe
make -j4 perft_suite_demo

# Verify it's a Windows executable
file bin/perft_suite_demo.exe
# Output: PE32+ executable (console) x86-64, for MS Windows

# Check dependencies (should only show system DLLs)
x86_64-w64-mingw32-objdump -p bin/perft_suite_demo.exe | grep 'DLL Name'
```

### 4. Test on Windows
```powershell
# Copy to Windows and test
Write-Output "1" | .\build\mingw-w64-release\bin\perft_suite_demo.exe
```

## Troubleshooting

### Threading Issues
If you encounter mutex/threading errors, ensure you're using the posix variant:
- Use `x86_64-w64-mingw32-g++-posix` instead of `x86_64-w64-mingw32-g++`
- Add `-pthread` flag to both compile and link options

### Performance Considerations
- WSL adds overhead compared to native builds
- Cross-compiled executables perform similarly to WSL native builds
- For best performance, use MSVC toolchain directly on Windows

### CMake Configuration
The project automatically detects GCC and applies appropriate optimizations:
```cmake
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using GCC compiler")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(
            -O3                 # Maximum optimization
            -march=native       # Optimize for current CPU (WSL only)
            -mtune=native       # Tune for current CPU  
            -flto               # Link-time optimization
            -ffast-math         # Fast math optimizations
            -funroll-loops      # Loop unrolling
            -finline-functions  # Aggressive inlining
        )
        add_link_options(-flto)  # LTO for linking
    endif()
```

## Build Artifacts

### WSL Linux Build
- Location: `build/wsl-gcc-release/bin/`
- Executable: `perft_suite_demo` (Linux ELF)
- Size: ~90KB (stripped)

### MinGW-w64 Windows Build
- Location: `build/mingw-w64-release/bin/`
- Executable: `perft_suite_demo.exe` (Windows PE32+)
- Size: ~1.1MB (statically linked)
- Dependencies: Only KERNEL32.dll and msvcrt.dll (system DLLs)

## Conclusion

While WSL provides a convenient way to use GCC toolchain on Windows, the performance results show that **MSVC remains the preferred toolchain** for the Huginn chess engine on Windows, delivering significantly better performance (28.6s vs 58.1s) for the same workload.

The WSL/GCC builds are useful for:
- Cross-platform compatibility testing
- Leveraging GCC-specific features or optimizations
- Development environments where MSVC is not available
- Creating portable, statically-linked Windows executables

For production use on Windows, continue using the MSVC build configuration for optimal performance.
