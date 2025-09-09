#include "search_thread.hpp"
#include "global_transposition_table.hpp"
#include "uci_utils.hpp"  // For move_to_string
#include <iostream>

namespace Huginn {
    
    bool SearchThreadManager::start_search(const Position& pos, const SearchInfo& info, std::function<void()> callback) {
        // Clean up any completed previous search thread
        if (search_thread && !search_running.load()) {
            if (search_thread->joinable()) {
                search_thread->join();
            }
            search_thread.reset();
        }
        
        // Don't start new search if one is already running
        if (search_running.load()) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(search_mutex);
        
        // Copy search parameters
        search_position = pos;
        search_info = info;
        completion_callback = callback;
        
        // Reset search state
        engine->reset();
        
        // Mark search as running
        search_running.store(true);
        
        // Create search thread (VICE Part 100 concept)
        // Equivalent to: thrd_create(&search_thread, search_function, data)
        try {
            search_thread = std::make_unique<std::thread>(search_thread_function, this);
            return true;
        } catch (const std::exception& e) {
            search_running.store(false);
            std::cerr << "Failed to create search thread: " << e.what() << std::endl;
            return false;
        }
    }
    
    void SearchThreadManager::stop_search() {
        if (!search_running.load()) {
            return;
        }
        
        // Signal search to stop
        if (engine) {
            engine->stop();
        }
        
        // Wait for search thread to complete (VICE Part 100)
        // Equivalent to: thrd_join(search_thread, NULL)
        if (search_thread && search_thread->joinable()) {
            search_thread->join();
        }
        
        search_thread.reset();
        search_running.store(false);
    }
    
    void SearchThreadManager::search_thread_function(SearchThreadManager* manager) {
        if (!manager || !manager->engine) {
            return;
        }
        
        try {
            // VICE Part 85: Increment age for new search
            increment_tt_age();
            
            // Execute the search in this thread
            // This is where the actual search happens while main thread listens for input
            S_MOVE best_move = manager->engine->searchPosition(
                manager->search_position, 
                manager->search_info
            );
            
            // Output best move (always output, regardless of stop condition)
            std::cout << "bestmove ";
            if (best_move.move != 0) {
                std::cout << manager->engine->move_to_uci(best_move);
            } else {
                std::cout << "0000";  // No legal moves (checkmate/stalemate)
            }
            std::cout << std::endl;
            std::cout.flush();  // Ensure immediate output
            
        } catch (const std::exception& e) {
            std::cerr << "Search thread error: " << e.what() << std::endl;
        }
        
        // Mark search as completed
        manager->search_running.store(false);
        
        // Call completion callback if provided
        if (manager->completion_callback) {
            manager->completion_callback();
        }
    }
}
