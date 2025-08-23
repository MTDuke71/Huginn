# Huginn Chess Engine Documentation

This directory contains all documentation for the Huginn chess engine project.

## Documentation Index

### 📚 Core Documentation
- [`API.md`](API.md) - Complete API reference and usage guide
- [`UCI_IMPLEMENTATION.md`](UCI_IMPLEMENTATION.md) - UCI interface implementation and usage
- [`README.md`](../README.md) - Main project overview (in root directory)

### 🏗️ Architecture & Design
- [`POSITION_AND_MOVEGEN_ARCHITECTURE.md`](POSITION_AND_MOVEGEN_ARCHITECTURE.md) - Core position and move generation architecture
- [`BITBOARD_IMPLEMENTATION.md`](BITBOARD_IMPLEMENTATION.md) - Bitboard system implementation details
- [`SQATTACKED_IMPLEMENTATION.md`](SQATTACKED_IMPLEMENTATION.md) - Square attack detection implementation

### 🚀 Performance Analysis
- [`PERFORMANCE_TRACKING.md`](PERFORMANCE_TRACKING.md) - Performance tracking system and results
- [`SQATTACKED_PERFORMANCE.md`](SQATTACKED_PERFORMANCE.md) - Square attack performance analysis
- [`MOVE_DECODING_ANALYSIS.md`](MOVE_DECODING_ANALYSIS.md) - Move decoding performance comparison
- [`DECODE_MOVE_ANALYSIS.md`](DECODE_MOVE_ANALYSIS.md) - Analysis of decode_move() removal impact

### 🔧 Optimization Documentation
- [`DECODE_MOVE_REMOVAL.md`](DECODE_MOVE_REMOVAL.md) - Complete modernization: decode_move() function removal
- [`assembly_analysis.md`](assembly_analysis.md) - Assembly code analysis for optimizations
- [`OPTIMIZATION_ANALYSIS.md`](OPTIMIZATION_ANALYSIS.md) - Comprehensive optimization analysis

### ✅ Validation & Testing
- [`DEBUG_VALIDATION.md`](DEBUG_VALIDATION.md) - Debug validation system documentation
- [`CASTLING_VALIDATION.md`](CASTLING_VALIDATION.md) - Castling move validation
- [`EN_PASSANT_VALIDATION.md`](EN_PASSANT_VALIDATION.md) - En passant move validation
- [`FEN_GENERATION.md`](FEN_GENERATION.md) - FEN string generation and parsing

### 📋 Project Management
- [`ToDo.md`](ToDo.md) - Current tasks and future improvements

## Quick Navigation

### For Developers
- Start with [`API.md`](API.md) for the public interface
- Check [`UCI_IMPLEMENTATION.md`](UCI_IMPLEMENTATION.md) for chess GUI integration
- Review [`POSITION_AND_MOVEGEN_ARCHITECTURE.md`](POSITION_AND_MOVEGEN_ARCHITECTURE.md) for core concepts
- Check [`PERFORMANCE_TRACKING.md`](PERFORMANCE_TRACKING.md) for optimization results

### For Performance Analysis
- [`PERFORMANCE_TRACKING.md`](PERFORMANCE_TRACKING.md) - Overall performance tracking system
- [`DECODE_MOVE_ANALYSIS.md`](DECODE_MOVE_ANALYSIS.md) - Why micro-optimizations show different results in real tests
- [`SQATTACKED_PERFORMANCE.md`](SQATTACKED_PERFORMANCE.md) - Attack detection performance

### For Implementation Details
- [`BITBOARD_IMPLEMENTATION.md`](BITBOARD_IMPLEMENTATION.md) - Bitboard system
- [`SQATTACKED_IMPLEMENTATION.md`](SQATTACKED_IMPLEMENTATION.md) - Attack detection
- [`CASTLING_VALIDATION.md`](CASTLING_VALIDATION.md) - Castling rules
- [`EN_PASSANT_VALIDATION.md`](EN_PASSANT_VALIDATION.md) - En passant rules

## Documentation Standards

All documentation in this project follows these standards:
- **Clear structure** with logical sections
- **Code examples** for technical concepts
- **Performance metrics** with concrete numbers
- **Cross-references** between related documents
- **Update tracking** with dates and versions

**📝 Note for Contributors**: All new .md documentation files should be created in the `docs/` directory to maintain project organization.

## Recent Updates

All documentation is actively maintained and reflects the current state of the chess engine implementation. 

**Latest**: The main `huginn.exe` executable is now a complete UCI chess engine, ready for immediate use with chess GUIs like Arena, Fritz, and ChessBase.

Check git history for detailed change tracking.
