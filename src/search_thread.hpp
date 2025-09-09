#pragma once

#include "minimal_search.hpp"
#include "position.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>

namespace Huginn {
    /**
     * @brief Search thread manager for UCI commands
     * 
     * Implements VICE Part 100 threading concept:
     * - Main thread handles UCI input
     * - Search thread executes position search
     * - Proper synchronization for stop/quit commands
     */
    class SearchThreadManager {
    private:
        std::unique_ptr<std::thread> search_thread;
        std::atomic<bool> search_running{false};
        std::mutex search_mutex;
        
        // Search parameters
        MinimalEngine* engine;
        Position search_position;
        SearchInfo search_info;
        
        // Callback for search completion
        std::function<void()> completion_callback;
        
    public:
        SearchThreadManager(MinimalEngine* eng) : engine(eng) {}
        
        ~SearchThreadManager() {
            stop_search();
        }
        
        /**
         * @brief Start search in separate thread (VICE Part 100)
         * 
         * Equivalent to thrd_create in tinycthread:
         * - Creates search thread with search function
         * - Passes position and search info to thread
         * - Main thread returns to UCI input listening
         */
        bool start_search(const Position& pos, const SearchInfo& info, std::function<void()> callback = nullptr);
        
        /**
         * @brief Stop search and wait for thread completion
         * 
         * Equivalent to thrd_join in tinycthread:
         * - Sets stop flag for search thread
         * - Waits for search thread to complete
         * - Called on "stop" or "quit" commands
         */
        void stop_search();
        
        /**
         * @brief Check if search is currently running
         */
        bool is_searching() const { return search_running.load(); }
        
        /**
         * @brief Wait for search to complete (non-blocking check)
         * @return true if search is still running, false if completed
         */
        bool wait_for_completion_check() {
            if (!search_running.load() && search_thread && search_thread->joinable()) {
                search_thread->join();
                search_thread.reset();
                return false; // Search completed
            }
            return search_running.load(); // Still running
        }
        
        /**
         * @brief Search thread function (VICE Part 100)
         * 
         * This is the function passed to thrd_create equivalent.
         * Executes the actual search and handles stop conditions.
         */
        static void search_thread_function(SearchThreadManager* manager);
    };
}
