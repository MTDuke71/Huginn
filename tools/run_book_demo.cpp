#include "init.hpp"
#include "minimal_search.hpp"
#include "position.hpp"
#include <iostream>
#include <fstream>
#include <vector>

using namespace Huginn;

int main() {
    init();
    MinimalEngine engine;
    
    // Try multiple possible locations for the book file
    std::vector<std::string> possible_paths = {
        "src/performance.bin",           // From project root
        "performance.bin",               // Same directory as executable
        "../../../src/performance.bin", // From release bin directory to project src
        "../../src/performance.bin"     // From debug bin directory to project src
    };
    
    std::string book_path;
    bool found = false;
    
    for (const auto& path : possible_paths) {
        std::ifstream test_file(path);
        if (test_file.good()) {
            book_path = path;
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cerr << "Could not find performance.bin in any of these locations:" << std::endl;
        for (const auto& path : possible_paths) {
            std::cerr << "  " << path << std::endl;
        }
        return 1;
    }
    
    std::cout << "Loading book: " << book_path << std::endl;
    if (!engine.load_opening_book(book_path)) {
        std::cerr << "Failed to load book: " << book_path << std::endl;
        return 1;
    }

    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << "Starting position FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" << std::endl;
    
    // Check our Polyglot key generation
    uint64_t our_key = engine.opening_book.get_polyglot_key(pos);
    std::cout << "Our Polyglot key: 0x" << std::hex << our_key << std::dec << std::endl;
    std::cout << "Expected key:     0x463b96181691fc9c" << std::endl;
    
    if (our_key == 0x463b96181691fc9cULL) {
        std::cout << "✓ Key matches Polyglot specification!" << std::endl;
    } else {
        std::cout << "✗ Key does not match Polyglot specification" << std::endl;
    }
    
    std::cout << "Checking if position is in book..." << std::endl;
    if (engine.is_in_opening_book(pos)) {
        std::cout << "✓ Position is in the opening book!" << std::endl;
    } else {
        std::cout << "✗ Position is not in the opening book" << std::endl;
    }

    engine.print_book_moves(pos);
    
    // Debug: Let's look at the raw book entries for this position
    auto book_moves = engine.opening_book.get_all_book_moves(pos);
    std::cout << "\nDebug: Raw book entries:" << std::endl;
    for (const auto& [move, weight] : book_moves) {
        std::cout << "  Move: from=" << move.get_from() << " to=" << move.get_to() 
                  << " (120-square format)" << std::endl;
        
        // Also print file/rank interpretation
        int from_file = (move.get_from() % 10) - 1;
        int from_rank = (move.get_from() / 10) - 2;
        int to_file = (move.get_to() % 10) - 1;
        int to_rank = (move.get_to() / 10) - 2;
        
        char from_file_char = 'a' + from_file;
        char to_file_char = 'a' + to_file;
        int from_rank_num = from_rank + 1;
        int to_rank_num = to_rank + 1;
        
        std::cout << "    = " << from_file_char << from_rank_num 
                  << to_file_char << to_rank_num << std::endl;
    }
    
    return 0;
}
