// Quick test for VICE Part 68 position parsing requirements
#include "src/uci.hpp"
#include <sstream>
#include <iostream>

void test_position_command(const std::string& command) {
    std::cout << "\n=== Testing: " << command << " ===" << std::endl;
    
    UCIInterface uci;
    
    // Manually parse the command like UCIInterface::run() would
    std::istringstream iss(command);
    std::string token;
    std::vector<std::string> tokens;
    while (iss >> token) tokens.push_back(token);
    
    if (!tokens.empty() && tokens[0] == "position") {
        try {
            uci.handle_position(tokens);
            std::cout << "✓ Command processed successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✗ Error: " << e.what() << std::endl;
        }
    }
}

int main() {
    // Test cases from VICE Part 68
    test_position_command("position startpos");
    test_position_command("position startpos moves e2e4");
    test_position_command("position startpos moves e2e4 e7e5");
    test_position_command("position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    test_position_command("position fen rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 moves e7e5");
    
    std::cout << "\nAll VICE Part 68 position parsing tests completed." << std::endl;
    return 0;
}
