# Debug Assertions

This chess engine includes a comprehensive debug assertion system that provides detailed error information including file names and line numbers when assertions fail.

## Usage

### Two Types of Assertions

1. **CHESS_ASSERT(condition)** - Simple assertion for basic checks
2. **DEBUG_ASSERT(condition, message)** - Detailed assertion with custom error message

### Examples

```cpp
// Simple assertion
CHESS_ASSERT(is_playable(square));

// Detailed assertion with custom message
DEBUG_ASSERT(pCount[color][type] < MAX_PIECES_PER_TYPE, 
            "Too many pieces of this type on the board");
```

### Output Example

When an assertion fails in debug mode, you'll see detailed information:

```
ASSERTION FAILED: Too many pieces of this type on the board
  File: src/position.hpp
  Line: 42
  Function: add_piece_to_list
  Condition: pCount[color][type] < MAX_PIECES_PER_TYPE
```

## Enabling Debug Assertions

### Method 1: CMake Option (UCRT64)
```bash
# For MSYS2 UCRT64 environment
cmake -G Ninja -B build_debug -DENABLE_DEBUG_ASSERTIONS=ON \
    -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe
cmake --build build_debug
```

### Method 2: Debug Build Type (UCRT64)
```bash
# For MSYS2 UCRT64 environment  
cmake -G Ninja -B build_debug -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe
cmake --build build_debug
```

### Method 3: Manual Compiler Flag
```bash
g++ -DDEBUG -std=c++17 your_files.cpp
```

### Why specify the generator and compiler?

On Windows, CMake defaults to Visual Studio generator and MSVC compiler. To maintain consistency with your MSYS2 UCRT64 development environment, explicitly specify:
- `-G Ninja` for the same build system
- `-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe` for the same compiler

## Performance

- **Release Mode**: Assertions are completely compiled out (zero overhead)
- **Debug Mode**: Assertions are active and will terminate the program on failure

## Integration

The debug assertion system is integrated throughout the codebase:

- **Position class**: Validates piece list operations, move parameters
- **Board operations**: Validates square indices and piece placements  
- **Move making**: Validates move parameters and board state
- **Main engine**: Validates core chess engine invariants

## Best Practices

1. Use assertions for programming errors, not user input validation
2. Write clear, descriptive messages for DEBUG_ASSERT
3. Use CHESS_ASSERT for simple boolean checks
4. Place assertions at function entry points to validate parameters
5. Add assertions after complex operations to verify postconditions

## Current Assertions

The following areas currently have debug assertions:

- Square validity checks
- Piece list management
- Move parameter validation
- Board state consistency
- Array bounds checking
- Null pointer/invalid state detection
