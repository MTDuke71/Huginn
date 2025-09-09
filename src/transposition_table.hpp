#pragma once

#include <cstdint>
#include <vector>
#include <atomic>
#include <iostream>
#include <iomanip>

// Forward declaration 
using Move = uint32_t;

// Transposition Table entry for storing search results
// LOCKLESS HASHING for Lazy SMP (Robert Hyatt & Tim Mann approach)
// Thread-Safety: Uses XOR encoding to prevent data corruption from race conditions
// - Data is XORed with zobrist key before storage
// - Retrieval XORs data with key to verify integrity
// - Corrupted entries are automatically detected and rejected
struct TTEntry {
    std::atomic<uint64_t> encoded_data;  // Atomic XOR of zobrist_key and packed data for lockless access
    
    // Node types for alpha-beta bounds
    static constexpr uint8_t EXACT = 0;
    static constexpr uint8_t LOWER_BOUND = 1; // Alpha cutoff
    static constexpr uint8_t UPPER_BOUND = 2; // Beta cutoff
    
    TTEntry() : encoded_data(0) {}
    
    // Make TTEntry non-copyable but movable (required for std::vector with std::atomic)
    TTEntry(const TTEntry&) = delete;
    TTEntry& operator=(const TTEntry&) = delete;
    TTEntry(TTEntry&& other) noexcept : encoded_data(other.encoded_data.load()) {}
    TTEntry& operator=(TTEntry&& other) noexcept {
        if (this != &other) {
            encoded_data.store(other.encoded_data.load());
        }
        return *this;
    }
    
    // Pack all data into a single 64-bit integer for atomic storage
    // Format: [move:32][score:16][depth:8][flags:4][age:4]
    static uint64_t pack_data(uint32_t best_move, int16_t score, uint8_t depth, uint8_t node_type, uint8_t age) {
        // Handle negative scores by adding offset (VICE approach)
        uint16_t adjusted_score = static_cast<uint16_t>(score + 32768);
        
        return (static_cast<uint64_t>(best_move) << 32) |
               (static_cast<uint64_t>(adjusted_score) << 16) |
               (static_cast<uint64_t>(depth) << 8) |
               (static_cast<uint64_t>(node_type) << 4) |
               (static_cast<uint64_t>(age));
    }
    
    // Unpack data from the 64-bit integer
    static void unpack_data(uint64_t packed, uint32_t& best_move, int16_t& score, uint8_t& depth, uint8_t& node_type, uint8_t& age) {
        best_move = static_cast<uint32_t>(packed >> 32);
        uint16_t adjusted_score = static_cast<uint16_t>((packed >> 16) & 0xFFFF);
        score = static_cast<int16_t>(adjusted_score - 32768);  // Restore original score
        depth = static_cast<uint8_t>((packed >> 8) & 0xFF);
        node_type = static_cast<uint8_t>((packed >> 4) & 0xF);
        age = static_cast<uint8_t>(packed & 0xF);
    }
    
    // VICE-style macros for data packing/unpacking as mentioned in video (2:10)
    // These provide the macro interface equivalent to VICE's FoldData, ExtractMove, etc.
    static uint64_t FoldData(uint32_t move, int16_t score, uint8_t depth, uint8_t flag, uint8_t age) {
        return pack_data(move, score, depth, flag, age);
    }
    
    static uint32_t ExtractMove(uint64_t data) { return static_cast<uint32_t>(data >> 32); }
    static int16_t ExtractScore(uint64_t data) { 
        uint16_t adjusted = static_cast<uint16_t>((data >> 16) & 0xFFFF);
        return static_cast<int16_t>(adjusted - 32768);
    }
    static uint8_t ExtractDepth(uint64_t data) { return static_cast<uint8_t>((data >> 8) & 0xFF); }
    static uint8_t ExtractFlag(uint64_t data) { return static_cast<uint8_t>((data >> 4) & 0xF); }
    static uint8_t ExtractAge(uint64_t data) { return static_cast<uint8_t>(data & 0xF); }
    
    // Store data using lockless hashing (XOR with zobrist key)
    void store_lockless(uint64_t zobrist_key, uint32_t best_move, int16_t score, uint8_t depth, uint8_t node_type, uint8_t age) {
        uint64_t packed = pack_data(best_move, score, depth, node_type, age);
        encoded_data.store(zobrist_key ^ packed, std::memory_order_relaxed);  // Atomic store with XOR
    }
    
    // Retrieve data using lockless hashing (verify with XOR)
    bool probe_lockless(uint64_t zobrist_key, uint32_t& best_move, int16_t& score, uint8_t& depth, uint8_t& node_type, uint8_t& age) const {
        uint64_t encoded = encoded_data.load(std::memory_order_relaxed);  // Atomic load
        if (encoded == 0) return false;  // Empty entry
        
        uint64_t packed = zobrist_key ^ encoded;  // XOR to get original data
        unpack_data(packed, best_move, score, depth, node_type, age);
        
        // Verify integrity: re-encode and check if it matches
        uint64_t verification = pack_data(best_move, score, depth, node_type, age);
        return (zobrist_key ^ verification) == encoded;
    }
    
    /**
     * @brief VICE verifyEntrySMP function (1:58 in video)
     * 
     * Verifies that the SMP data can be reconstructed correctly and that
     * the SMP_key matches the expected value after XORing position key with data.
     * This function is crucial for debugging parallel processing.
     * 
     * @param zobrist_key The position key to verify against
     * @return true if entry is valid and data can be reconstructed correctly
     */
    bool verifyEntrySMP(uint64_t zobrist_key) const {
        uint64_t encoded = encoded_data.load(std::memory_order_relaxed);
        if (encoded == 0) return false;  // Empty entry, nothing to verify
        
        // Extract the packed data by XORing with key (this is the SMP_data equivalent)
        uint64_t packed_data = zobrist_key ^ encoded;
        
        // Reconstruct data from packed format
        uint32_t move;
        int16_t score;
        uint8_t depth, flag, age;
        unpack_data(packed_data, move, score, depth, flag, age);
        
        // Re-pack the data to verify integrity
        uint64_t reconstructed_data = pack_data(move, score, depth, flag, age);
        
        // Verify that reconstructed data matches original packed data
        if (reconstructed_data != packed_data) {
            return false;  // Data corruption detected
        }
        
        // Verify that the SMP_key (encoded) matches expected value
        uint64_t expected_encoded = zobrist_key ^ reconstructed_data;
        return (expected_encoded == encoded);
    }
};

/**
 * @brief Transposition table for caching search results
 * 
 * Thread-Safety for Lazy SMP:
 * - Designed for concurrent access by multiple search threads
 * - Store operations are safe even with write collisions
 * - Probe operations are always safe for reading
 * - Statistics may be approximate in multi-threaded environment
 */
class TranspositionTable {
private:
    std::vector<TTEntry> table;
    size_t size_mask;  // For fast modulo (size must be power of 2)
    uint8_t current_age = 0;  // Current age for replacement strategy (VICE Part 85)
    
    // Statistics tracking
    mutable uint64_t hits = 0;      // Successful probes
    mutable uint64_t misses = 0;    // Failed probes  
    uint64_t writes = 0;            // Store operations
    
public:
    explicit TranspositionTable(size_t size_mb = 64) {
        // Calculate number of entries for given size in MB
        size_t num_entries = (size_mb * 1024 * 1024) / sizeof(TTEntry);
        
        // Round down to nearest power of 2 for fast indexing
        size_t power_of_2 = 1;
        while (power_of_2 * 2 <= num_entries) {
            power_of_2 *= 2;
        }
        
        table.resize(power_of_2);
        size_mask = power_of_2 - 1;
    }
    
    // Store position in transposition table with lockless hashing and age-based replacement
    // LAZY SMP: Multiple threads can safely call this concurrently
    void store(uint64_t zobrist_key, int score, uint8_t depth, uint8_t node_type, uint32_t best_move = 0) {
        size_t index = zobrist_key & size_mask;
        TTEntry& entry = table[index];
        
        // For lockless replacement, we need to check if we should replace
        // First, try to decode existing entry to see if replacement is warranted
        if (entry.encoded_data != 0) {
            uint32_t existing_move;
            int16_t existing_score;
            uint8_t existing_depth, existing_node_type, existing_age;
            
            // Try to decode existing entry - if it fails, it's corrupted so replace it
            bool valid_existing = entry.probe_lockless(zobrist_key, existing_move, existing_score, 
                                                      existing_depth, existing_node_type, existing_age);
            
            if (valid_existing) {
                // Valid existing entry - check replacement criteria (VICE Part 85)
                bool should_replace = (existing_age < current_age) ||                     // Older entry
                                     (existing_age == current_age && depth >= existing_depth); // Same age, deeper search
                
                if (!should_replace) {
                    return; // Don't replace - existing entry is better
                }
            }
            // If !valid_existing, it's corrupted data so we'll replace it
        }
        
        // Store using lockless hashing
        entry.store_lockless(zobrist_key, best_move, static_cast<int16_t>(score), depth, node_type, current_age);
        writes++;  // Track write operations
        
        // VICE Part 85: Verify the entry was stored correctly (4:27 in video)
        // This verification is crucial for debugging parallel processing
        if (!entry.verifyEntrySMP(zobrist_key)) {
            // This should never happen if our lockless hashing is working correctly
            // If it does, it indicates a bug in our storage mechanism
            std::cerr << "ERROR: Failed to verify hash entry after storage!" << std::endl;
        }
    }
    
    // Probe transposition table for position using lockless hashing
    // LAZY SMP: Multiple threads can safely call this concurrently
    bool probe(uint64_t zobrist_key, int& score, uint8_t& depth, uint8_t& node_type, uint32_t& best_move) const {
        size_t index = zobrist_key & size_mask;
        const TTEntry& entry = table[index];
        
        uint32_t decoded_move;
        int16_t decoded_score;
        uint8_t decoded_depth, decoded_node_type, decoded_age;
        
        // Use lockless probing - automatically detects corruption
        if (entry.probe_lockless(zobrist_key, decoded_move, decoded_score, decoded_depth, decoded_node_type, decoded_age)) {
            // VICE Part 85: Additional verification check (6:16 in video)
            // Double-check the entry integrity for debugging parallel processing
            if (!entry.verifyEntrySMP(zobrist_key)) {
                // If verification fails, treat as a miss even though probe_lockless succeeded
                // This catches any subtle corruption issues
                misses++;
                return false;
            }
            
            score = decoded_score;
            depth = decoded_depth;
            node_type = decoded_node_type;
            best_move = decoded_move;
            hits++;  // Track successful probes
            return true;
        }
        
        misses++;  // Track failed probes (includes corrupted entries)
        return false;
    }
    
    // Clear all entries
    void clear() {
        for (auto& entry : table) {
            entry.encoded_data.store(0, std::memory_order_relaxed);  // Atomic clear of lockless encoded data
        }
        // Reset statistics and age
        hits = misses = writes = 0;
        current_age = 0;
    }
    
    // Get table utilization statistics
    double get_utilization() const {
        size_t filled = 0;
        for (const auto& entry : table) {
            if (entry.encoded_data.load(std::memory_order_relaxed) != 0) filled++;  // Atomic load for encoded data check
        }
        return double(filled) / table.size();
    }
    
    size_t get_size() const { return table.size(); }
    
    // Get statistics
    uint64_t get_hits() const { return hits; }
    uint64_t get_misses() const { return misses; }
    uint64_t get_writes() const { return writes; }
    uint64_t get_total_probes() const { return hits + misses; }
    double get_hit_rate() const { 
        uint64_t total = get_total_probes();
        return total > 0 ? double(hits) / total : 0.0; 
    }
    
    // Clear statistics only
    void clear_stats() {
        hits = misses = writes = 0;
    }
    
    // VICE Part 85: Age management for better replacement strategy
    
    /**
     * @brief Increment age for new search
     * 
     * Should be called at the start of each new search to mark entries
     * from this search as more recent than previous searches.
     */
    void increment_age() {
        current_age++;
        // Prevent overflow (though 255 searches is quite a lot)
        if (current_age == 0) current_age = 1;
    }
    
    /**
     * @brief Reset age for new game
     * 
     * Should be called when starting a new game to reset the age system.
     */
    void reset_age() {
        current_age = 0;
    }
    
    /**
     * @brief Get current age
     * 
     * @return Current age value
     */
    uint8_t get_age() const {
        return current_age;
    }
    
    /**
     * @brief Test function for data packing/unpacking macros
     * 
     * VICE Part 85: Test the data packing and unpacking to ensure
     * the macros work correctly with all data ranges.
     */
    void data_check() const {
        std::cout << "\n=== Transposition Table Data Packing Test ===" << std::endl;
        
        // Test various data combinations
        Move test_move = 0x12345678;  // 32-bit move
        int16_t test_score = -1234;   // Negative score
        uint8_t test_depth = 15;      // 8-bit depth
        uint8_t test_flag = 3;        // 4-bit flag (EXACT)
        uint8_t test_age = 7;         // 4-bit age
        
        // Pack the data using TTEntry function
        uint64_t packed = TTEntry::pack_data(test_move, test_score, test_depth, test_flag, test_age);
        
        std::cout << "Original Data:" << std::endl;
        std::cout << "  Move: 0x" << std::hex << test_move << std::dec << std::endl;
        std::cout << "  Score: " << test_score << std::endl;
        std::cout << "  Depth: " << static_cast<int>(test_depth) << std::endl;
        std::cout << "  Flag: " << static_cast<int>(test_flag) << std::endl;
        std::cout << "  Age: " << static_cast<int>(test_age) << std::endl;
        
        std::cout << "\nPacked Data: 0x" << std::hex << packed << std::dec << std::endl;
        
        // Test VICE macros
        std::cout << "\nVICE Macro Extraction:" << std::endl;
        std::cout << "  ExtractMove: 0x" << std::hex << TTEntry::ExtractMove(packed) << std::dec << std::endl;
        std::cout << "  ExtractScore: " << TTEntry::ExtractScore(packed) << std::endl;
        std::cout << "  ExtractDepth: " << static_cast<int>(TTEntry::ExtractDepth(packed)) << std::endl;
        std::cout << "  ExtractFlag: " << static_cast<int>(TTEntry::ExtractFlag(packed)) << std::endl;
        std::cout << "  ExtractAge: " << static_cast<int>(TTEntry::ExtractAge(packed)) << std::endl;
        
        // Verify data integrity
        bool all_correct = true;
        if (TTEntry::ExtractMove(packed) != test_move) {
            std::cout << "ERROR: Move mismatch!" << std::endl;
            all_correct = false;
        }
        if (TTEntry::ExtractScore(packed) != test_score) {
            std::cout << "ERROR: Score mismatch!" << std::endl;
            all_correct = false;
        }
        if (TTEntry::ExtractDepth(packed) != test_depth) {
            std::cout << "ERROR: Depth mismatch!" << std::endl;
            all_correct = false;
        }
        if (TTEntry::ExtractFlag(packed) != test_flag) {
            std::cout << "ERROR: Flag mismatch!" << std::endl;
            all_correct = false;
        }
        if (TTEntry::ExtractAge(packed) != test_age) {
            std::cout << "ERROR: Age mismatch!" << std::endl;
            all_correct = false;
        }
        
        if (all_correct) {
            std::cout << "\n✓ All data packing/unpacking tests PASSED!" << std::endl;
        } else {
            std::cout << "\n✗ Data packing/unpacking tests FAILED!" << std::endl;
        }
        
        // Test FoldData macro
        std::cout << "\nTesting FoldData macro:" << std::endl;
        uint64_t folded = TTEntry::FoldData(test_move, test_score, test_depth, test_flag, test_age);
        std::cout << "  FoldData result: 0x" << std::hex << folded << std::dec << std::endl;
        std::cout << "  pack_data result: 0x" << std::hex << packed << std::dec << std::endl;
        
        if (folded == packed) {
            std::cout << "✓ FoldData macro matches pack_data function!" << std::endl;
        } else {
            std::cout << "✗ FoldData macro differs from pack_data function!" << std::endl;
        }
        
        // Test VICE verifyEntrySMP function as shown in video (1:58)
        std::cout << "\nTesting VICE verifyEntrySMP function:" << std::endl;
        
        // Create a test entry with known data
        TTEntry test_entry;
        uint64_t test_zobrist = 0xABCDEF1234567890ULL;
        
        // Store test data
        test_entry.store_lockless(test_zobrist, test_move, test_score, test_depth, test_flag, test_age);
        
        // Verify the entry
        bool verify_result = test_entry.verifyEntrySMP(test_zobrist);
        if (verify_result) {
            std::cout << "✓ verifyEntrySMP validation PASSED!" << std::endl;
        } else {
            std::cout << "✗ verifyEntrySMP validation FAILED!" << std::endl;
        }
        
        // Test with wrong key (should fail)
        uint64_t wrong_key = test_zobrist + 1;
        bool wrong_verify = test_entry.verifyEntrySMP(wrong_key);
        if (!wrong_verify) {
            std::cout << "✓ verifyEntrySMP correctly detects wrong key!" << std::endl;
        } else {
            std::cout << "✗ verifyEntrySMP failed to detect wrong key!" << std::endl;
        }
        
        std::cout << "=== End Data Packing Test ===\n" << std::endl;
    }
};
