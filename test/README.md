# Test Suite

This directory contains the comprehensive test suite for the Huginn chess engine.

## Test Categories

### Core Functionality Tests
- **Board representation and coordinate systems**
- **Move generation for all piece types** 
- **Position management and FEN parsing**
- **Attack detection and legal move validation**
- **Special moves**: Castling, en passant, promotions

### Optimization Tests  
- **Pawn optimization validation**
- **King optimization validation**
- **Knight optimization validation** 
- **Sliding piece optimization validation**

### Performance Tests
- **Perft validation** against standard test suites
- **Performance regression testing**

## Usage

### Running All Tests
```bash
# Build and run all tests
cmake --build build --target huginn_tests
.\build\huginn_tests.exe

# Run specific test categories
.\build\huginn_tests.exe --gtest_filter="PawnOptimization*"
```

## Test Framework

The test suite uses Google Test framework with:
- **42+ optimization tests** validating correctness
- **Comprehensive position coverage** including edge cases
- **Performance benchmarking** for optimization validation
- **Regression testing** to prevent performance degradation

All tests must pass before committing optimizations to ensure correctness is maintained.
