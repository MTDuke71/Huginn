#include "src/transposition_table.hpp"
#include <iostream>
#include <random>
#include <thread>
#include <vector>

const int MATE = 29000;

// Simulate the UCI score formatting
std::string format_uci_score(int score) {
    if (score > MATE - 100) {
        int mate_in_plies = MATE - score;
        int mate_in_moves = (mate_in_plies + 1) / 2;
        return "mate " + std::to_string(mate_in_moves);
    } else if (score < -MATE + 100) {
        int mate_in_plies = MATE + score;
        int mate_in_moves = (mate_in_plies + 1) / 2;
        return "mate -" + std::to_string(mate_in_moves);
    } else {
        return "cp " + std::to_string(score);
    }
}

void test_concurrent_access(TranspositionTable& tt, int thread_id) {
    std::random_device rd;
    std::mt19937 gen(rd() + thread_id);
    std::uniform_int_distribution<int> score_dist(-2000, 2000);  // Normal position scores
    std::uniform_int_distribution<uint64_t> key_dist(1, 0xFFFFFFFFFFFFFFFFULL);
    
    for (int i = 0; i < 10000; i++) {
        uint64_t key = key_dist(gen);
        int original_score = score_dist(gen);
        
        // Store the score
        tt.store(key, original_score, 10, 2, 0x12345678);
        
        // Immediately try to retrieve it
        int retrieved_score;
        uint8_t depth, flag;
        uint32_t move;
        
        if (tt.probe(key, retrieved_score, depth, flag, move)) {
            // Check for corruption into mate range
            if ((retrieved_score < -MATE + 100 || retrieved_score > MATE - 100) && 
                (original_score > -MATE + 100 && original_score < MATE - 100)) {
                
                std::cout << "THREAD " << thread_id << " FOUND CORRUPTION!" << std::endl;
                std::cout << "  Original: " << original_score << " (" << format_uci_score(original_score) << ")" << std::endl;
                std::cout << "  Retrieved: " << retrieved_score << " (" << format_uci_score(retrieved_score) << ")" << std::endl;
                std::cout << "  Key: 0x" << std::hex << key << std::dec << std::endl;
                std::cout << "  This would show as: " << format_uci_score(retrieved_score) << std::endl;
                std::cout << std::endl;
            }
        }
    }
}

int main() {
    std::cout << "=== Reproducing -M499 on Normal Positions ===" << std::endl;
    
    TranspositionTable tt(1024 * 16);  // Larger table
    
    // First test: Single-threaded to rule out race conditions
    std::cout << "\n1. Single-threaded test:" << std::endl;
    
    bool found_single_threaded = false;
    for (int score = -3000; score <= 3000; score += 10) {
        uint64_t key = static_cast<uint64_t>(score + 10000) * 0x123456789ABCDEFULL;
        
        tt.store(key, score, 12, 1, 0x87654321);
        
        int retrieved;
        uint8_t depth, flag;
        uint32_t move;
        
        if (tt.probe(key, retrieved, depth, flag, move)) {
            if ((retrieved < -MATE + 100 || retrieved > MATE - 100) && 
                (score > -MATE + 100 && score < MATE - 100)) {
                
                std::cout << "SINGLE-THREADED CORRUPTION!" << std::endl;
                std::cout << "  Score " << score << " -> " << retrieved << std::endl;
                std::cout << "  UCI: " << format_uci_score(retrieved) << std::endl;
                found_single_threaded = true;
            }
        }
    }
    
    if (!found_single_threaded) {
        std::cout << "✓ No corruption in single-threaded test" << std::endl;
    }
    
    // Second test: Multi-threaded to test race conditions
    std::cout << "\n2. Multi-threaded test (checking for race conditions):" << std::endl;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(test_concurrent_access, std::ref(tt), i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Multi-threaded test completed." << std::endl;
    
    // Third test: Direct bit manipulation to see what causes -M499
    std::cout << "\n3. Testing what normal score could become -M499:" << std::endl;
    
    // If UCI shows "mate -499", the internal score is approximately:
    // mate_in_moves = 499 -> mate_in_plies ≈ 997 -> score ≈ -(29000 - 997) = -28003
    
    int target_corrupted = -28003;
    std::cout << "Target corrupted score for -M499: " << target_corrupted << std::endl;
    
    // Work backwards: what packed value would give this score?
    uint16_t target_adjusted = static_cast<uint16_t>(target_corrupted + 32768);
    std::cout << "Target adjusted value: " << target_adjusted << " (0x" << std::hex << target_adjusted << std::dec << ")" << std::endl;
    
    // What normal score, when corrupted, could give this adjusted value?
    for (int test_score = -3000; test_score <= 3000; test_score += 50) {
        uint16_t normal_adjusted = static_cast<uint16_t>(test_score + 32768);
        
        // Check if any single bit flip could turn normal_adjusted into target_adjusted
        for (int bit = 0; bit < 16; bit++) {
            uint16_t flipped = normal_adjusted ^ (1 << bit);
            if (flipped == target_adjusted) {
                int resulting_score = static_cast<int16_t>(flipped - 32768);
                std::cout << "BIT FLIP SCENARIO:" << std::endl;
                std::cout << "  Normal score " << test_score << " (0x" << std::hex << normal_adjusted << std::dec << ")" << std::endl;
                std::cout << "  Bit " << bit << " flipped -> 0x" << std::hex << flipped << std::dec << std::endl;
                std::cout << "  Resulting score: " << resulting_score << std::endl;
                std::cout << "  UCI output: " << format_uci_score(resulting_score) << std::endl;
                std::cout << std::endl;
            }
        }
    }
    
    return 0;
}
