#ifndef PARALLEL_MOVEGEN_HPP
#define PARALLEL_MOVEGEN_HPP

#include "movegen_enhanced.hpp"
#include "position.hpp"
#include "move.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

// Parallel move generation class
class ParallelMoveGenerator {
public:
    static void generate_all_moves_parallel(const Position& pos, S_MOVELIST& list);
    static void generate_legal_moves_parallel(Position& pos, S_MOVELIST& list);
    
private:
    // Helper structures for parallel generation
    struct PieceThreadData {
        const Position* pos;
        S_MOVELIST moves;
        Color us;
        bool completed;
        
        PieceThreadData() : pos(nullptr), us(Color::White), completed(false) {
            moves.count = 0;  // Direct clear - faster than function call
        }
    };
    
    // Worker functions for each piece type
    static void generate_pawn_worker(PieceThreadData* data);
    static void generate_knight_worker(PieceThreadData* data);
    static void generate_bishop_worker(PieceThreadData* data);
    static void generate_rook_worker(PieceThreadData* data);
    static void generate_queen_worker(PieceThreadData* data);
    static void generate_king_worker(PieceThreadData* data);
    
    // Legal move validation workers
    struct LegalityThreadData {
        const Position* original_pos;
        const S_MOVE* moves_to_check;
        int start_index;
        int end_index;
        std::vector<S_MOVE> legal_moves;
        bool completed;
        
        LegalityThreadData() : original_pos(nullptr), moves_to_check(nullptr), 
                              start_index(0), end_index(0), completed(false) {}
    };
    
    static void check_legality_worker(LegalityThreadData* data);
    
    // Utility functions
    static void merge_move_lists(S_MOVELIST& target, const std::vector<S_MOVELIST>& sources);
    static int get_optimal_thread_count();
};

// Configuration for parallel processing
struct ParallelConfig {
    static constexpr int MIN_MOVES_FOR_PARALLEL_LEGAL = 20;  // Only parallelize legal checking if enough moves
    static constexpr int MAX_THREADS = 6;  // One per piece type
    static constexpr int LEGAL_MOVES_PER_THREAD = 10;  // Chunk size for legal move checking
    
    static bool use_parallel_generation;
    static bool use_parallel_legal;
    static int thread_count;
    
    static void auto_configure();
};

#endif // PARALLEL_MOVEGEN_HPP
