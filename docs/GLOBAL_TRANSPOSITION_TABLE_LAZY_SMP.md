# Global Transposition Table Implementation for Lazy SMP

## Overview

Based on the VICE chess engine video tutorial about moving the hash table to a global variable for lazy SMP (Symmetric Multi-Processing) support, this document outlines the implementation changes needed in Huginn.

## Why Global Transposition Table?

### Current Problem
- Each `MinimalEngine` instance has its own `TranspositionTable tt_table` member
- When launching multiple search threads, each would get a copy of the entire hash table
- This is inefficient in memory usage and prevents threads from sharing search results

### Lazy SMP Benefits
- **Shared Knowledge**: All search threads access the same transposition table
- **Memory Efficiency**: Single table instead of N copies for N threads
- **Better Search Quality**: Threads benefit from each other's discoveries
- **Scalability**: Essential for multi-threaded search performance

## Implementation Plan

### Phase 1: Global Table Infrastructure

1. **Create Global Table Manager**
   ```cpp
   // global_transposition_table.hpp
   namespace Huginn {
       extern std::unique_ptr<TranspositionTable> g_transposition_table;
       void init_global_transposition_table(size_t size_mb = 64);
       TranspositionTable& get_transposition_table();
       void cleanup_global_transposition_table();
   }
   ```

2. **Thread-Safe Table Design**
   ```cpp
   class TranspositionTable {
       // Note: In lazy SMP, multiple threads may write to same entry
       // This is acceptable as any valid entry is useful
       void store(uint64_t zobrist_key, int score, uint8_t depth, uint8_t node_type, uint32_t best_move = 0);
       bool probe(uint64_t zobrist_key, int& score, uint8_t& depth, uint8_t node_type, uint32_t& best_move) const;
   };
   ```

### Phase 2: Engine Refactoring

1. **Remove Instance Variable**
   ```cpp
   class MinimalEngine {
   private:
       // Remove this:
       // TranspositionTable tt_table;
       
       // Use global table via get_transposition_table()
   };
   ```

2. **Update All References**
   ```cpp
   // Before:
   tt_table.probe(pos.zobrist_key, tt_score, tt_depth, tt_node_type, tt_best_move);
   
   // After:
   get_transposition_table().probe(pos.zobrist_key, tt_score, tt_depth, tt_node_type, tt_best_move);
   ```

### Phase 3: Initialization Integration

1. **Engine Startup**
   ```cpp
   void init() {
       Zobrist::init_zobrist();
       EvalParams::init_evaluation_masks();
       init_global_transposition_table(64);  // 64MB default
       initialized = true;
   }
   ```

2. **Engine Shutdown**
   ```cpp
   void cleanup() {
       cleanup_global_transposition_table();
       initialized = false;
   }
   ```

### Phase 4: Multi-Threading Preparation

1. **Lazy SMP Structure**
   ```cpp
   class LazySMP {
       std::vector<std::thread> search_threads;
       // No need for shared_ptr<TranspositionTable> - it's global
       
       void start_parallel_search(Position& pos, int depth) {
           for (int i = 0; i < num_threads; ++i) {
               search_threads.emplace_back([&, i]() {
                   Position local_pos = pos;
                   MinimalEngine engine;  // Each thread gets own engine
                   // All engines share global transposition table automatically
                   engine.alpha_beta(local_pos, depth + (i % 3), -INFINITE, INFINITE, info);
               });
           }
       }
   };
   ```

## Files Modified

### New Files
- `src/global_transposition_table.hpp` - Global table interface
- `src/global_transposition_table.cpp` - Global table implementation

### Modified Files
- `src/minimal_search.hpp` - Remove tt_table member, add include
- `src/minimal_search.cpp` - Replace all tt_table.* with get_transposition_table().*
- `src/init.hpp` - Add cleanup() function declaration
- `src/init.cpp` - Add global table initialization and cleanup
- `src/transposition_table.hpp` - Add thread-safety comments

## Thread Safety Considerations

### Safe Operations
- **Reads**: Multiple threads can safely read from different table entries
- **Writes**: Multiple threads writing to the same entry is acceptable in lazy SMP
  - Any valid entry provides search benefit
  - Collisions are rare due to large table size
  - Race conditions don't corrupt data structure

### Statistics Accuracy
- Hit/miss counters will be approximate in multi-threaded environment
- This is acceptable as statistics are primarily for tuning, not correctness

## Performance Impact

### Expected Benefits
- **Memory**: Reduces memory usage from N×table_size to 1×table_size
- **Cache**: Better cache utilization from shared table
- **Search Quality**: Threads learn from each other's work

### Minimal Overhead
- Function call overhead for get_transposition_table() is negligible
- Global access is faster than member access through indirection

## Testing Strategy

1. **Single-threaded verification**: Ensure no performance regression
2. **Multi-threaded testing**: Verify shared table benefits
3. **Perft validation**: Confirm search correctness maintained
4. **Performance benchmarking**: Measure multi-threaded scaling

## Current Status

- ✅ Architecture designed for lazy SMP compatibility
- 🔄 Implementation in progress (partial files created)
- ⏳ Testing phase pending completion
- ⏳ Multi-threading integration planned

## Next Steps

1. Complete file-by-file refactoring to use global table
2. Add CMake targets for new source files
3. Test single-threaded performance parity
4. Implement lazy SMP threading framework
5. Performance tuning and optimization

This implementation follows the VICE tutorial approach while adapting to Huginn's modern C++ architecture and coding standards.
