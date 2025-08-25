#include "parallel_movegen.hpp"
#include "movegen_enhanced.hpp"
#include "attack_detection.hpp"
#include <algorithm>
#include <chrono>

// Static configuration variables
bool ParallelConfig::use_parallel_generation = false;
bool ParallelConfig::use_parallel_legal = false;
int ParallelConfig::thread_count = 1;

void ParallelConfig::auto_configure() {
    // Auto-detect optimal configuration based on hardware
    thread_count = std::min(static_cast<int>(std::thread::hardware_concurrency()), MAX_THREADS);
    if (thread_count < 2) {
        thread_count = 1;
        use_parallel_generation = false;
        use_parallel_legal = false;
        return;
    }
    
    // Enable parallel generation for multi-core systems
    use_parallel_generation = (thread_count >= 4);
    use_parallel_legal = (thread_count >= 2);
    
    // Clamp to reasonable limits
    if (thread_count > MAX_THREADS) {
        thread_count = MAX_THREADS;
    }
}

int ParallelMoveGenerator::get_optimal_thread_count() {
    if (ParallelConfig::thread_count <= 0) {
        ParallelConfig::auto_configure();
    }
    return ParallelConfig::thread_count;
}

void ParallelMoveGenerator::generate_all_moves_parallel(const Position& pos, S_MOVELIST& list) {
    list.count = 0;  // Direct clear - faster than function call
    
    // Check if parallel generation is beneficial
    if (!ParallelConfig::use_parallel_generation || get_optimal_thread_count() < 2) {
        // Fall back to sequential generation
        generate_all_moves(pos, list);
        return;
    }
    
    Color us = pos.side_to_move;
    
    // Prepare thread data for each piece type
    std::vector<PieceThreadData> thread_data(6);
    std::vector<std::thread> threads;
    
    for (auto& data : thread_data) {
        data.pos = &pos;
        data.us = us;
        data.completed = false;
    }
    
    // Launch worker threads for each piece type
    threads.emplace_back(generate_pawn_worker, &thread_data[0]);
    threads.emplace_back(generate_knight_worker, &thread_data[1]);
    threads.emplace_back(generate_bishop_worker, &thread_data[2]);
    threads.emplace_back(generate_rook_worker, &thread_data[3]);
    threads.emplace_back(generate_queen_worker, &thread_data[4]);
    threads.emplace_back(generate_king_worker, &thread_data[5]);
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // Merge results from all threads
    std::vector<S_MOVELIST> move_lists;
    for (const auto& data : thread_data) {
        if (data.completed && data.moves.count > 0) {
            move_lists.push_back(data.moves);
        }
    }
    
    merge_move_lists(list, move_lists);
}

void ParallelMoveGenerator::generate_legal_moves_parallel(Position& pos, S_MOVELIST& list) {
    // First generate all pseudo-legal moves
    S_MOVELIST pseudo_moves;
    
    if (ParallelConfig::use_parallel_generation) {
        generate_all_moves_parallel(pos, pseudo_moves);
    } else {
        generate_all_moves(pos, pseudo_moves);
    }
    
    list.count = 0;  // Direct clear - faster than function call
    
    // Check if parallel legal checking is beneficial
    if (!ParallelConfig::use_parallel_legal || 
        pseudo_moves.count < ParallelConfig::MIN_MOVES_FOR_PARALLEL_LEGAL ||
        get_optimal_thread_count() < 2) {
        // Fall back to sequential legal move generation
        generate_legal_moves_enhanced(pos, list);
        return;
    }
    
    // Calculate optimal number of threads and chunk size
    int num_threads = std::min(get_optimal_thread_count(), 
                              (pseudo_moves.count + ParallelConfig::LEGAL_MOVES_PER_THREAD - 1) / 
                              ParallelConfig::LEGAL_MOVES_PER_THREAD);
    
    if (num_threads < 2) {
        generate_legal_moves_enhanced(pos, list);
        return;
    }
    
    // Prepare thread data
    std::vector<LegalityThreadData> thread_data(num_threads);
    std::vector<std::thread> threads;
    
    int moves_per_thread = pseudo_moves.count / num_threads;
    int remaining_moves = pseudo_moves.count % num_threads;
    
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].original_pos = &pos;
        thread_data[i].moves_to_check = pseudo_moves.moves;
        thread_data[i].start_index = i * moves_per_thread + std::min(i, remaining_moves);
        thread_data[i].end_index = thread_data[i].start_index + moves_per_thread + (i < remaining_moves ? 1 : 0);
        thread_data[i].completed = false;
        
        threads.emplace_back(check_legality_worker, &thread_data[i]);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // Merge legal moves from all threads
    for (const auto& data : thread_data) {
        if (data.completed) {
            for (const auto& move : data.legal_moves) {
                list.add_quiet_move(move);
            }
        }
    }
}

// Worker functions for each piece type
void ParallelMoveGenerator::generate_pawn_worker(PieceThreadData* data) {
    try {
        generate_pawn_moves(*data->pos, data->moves, data->us);
        data->completed = true;
    } catch (...) {
        data->completed = false;
    }
}

void ParallelMoveGenerator::generate_knight_worker(PieceThreadData* data) {
    try {
        generate_knight_moves(*data->pos, data->moves, data->us);
        data->completed = true;
    } catch (...) {
        data->completed = false;
    }
}

void ParallelMoveGenerator::generate_bishop_worker(PieceThreadData* data) {
    try {
        generate_bishop_moves(*data->pos, data->moves, data->us);
        data->completed = true;
    } catch (...) {
        data->completed = false;
    }
}

void ParallelMoveGenerator::generate_rook_worker(PieceThreadData* data) {
    try {
        generate_rook_moves(*data->pos, data->moves, data->us);
        data->completed = true;
    } catch (...) {
        data->completed = false;
    }
}

void ParallelMoveGenerator::generate_queen_worker(PieceThreadData* data) {
    try {
        generate_queen_moves(*data->pos, data->moves, data->us);
        data->completed = true;
    } catch (...) {
        data->completed = false;
    }
}

void ParallelMoveGenerator::generate_king_worker(PieceThreadData* data) {
    try {
        generate_king_moves(*data->pos, data->moves, data->us);
        data->completed = true;
    } catch (...) {
        data->completed = false;
    }
}

void ParallelMoveGenerator::check_legality_worker(LegalityThreadData* data) {
    try {
        for (int i = data->start_index; i < data->end_index; ++i) {
            Position temp_pos = *data->original_pos;
            temp_pos.make_move_with_undo(data->moves_to_check[i]);
            
            // Check if our king is still in check after the move
            Color us = data->original_pos->side_to_move;
            if (!SqAttacked(temp_pos.king_sq[int(us)], temp_pos, !us)) {
                data->legal_moves.push_back(data->moves_to_check[i]);
            }
            temp_pos.undo_move();
        }
        data->completed = true;
    } catch (...) {
        data->completed = false;
    }
}

void ParallelMoveGenerator::merge_move_lists(S_MOVELIST& target, const std::vector<S_MOVELIST>& sources) {
    target.count = 0;  // Direct clear - faster than function call
    
    for (const auto& source : sources) {
        for (int i = 0; i < source.count; ++i) {
            target.add_quiet_move(source.moves[i]);
        }
    }
}
