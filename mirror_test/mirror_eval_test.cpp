// Mirror evaluation test for all positions in mirror.epd
#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace Huginn;

std::string extract_fen_from_epd(const std::string& epd_line) {
    // EPD format: "fen_part bm move; id "name";"
    // We need the first 4 parts of the FEN (position, side, castling, ep)
    std::istringstream iss(epd_line);
    std::string board, side, castling, ep;
    
    if (!(iss >> board >> side >> castling >> ep)) {
        return "";  // Invalid format
    }
    
    // Reconstruct FEN with default halfmove and fullmove
    return board + " " + side + " " + castling + " " + ep + " 0 1";
}

int main() {
    std::cout << "=== Mirror Evaluation Test for all positions in mirror.epd ===" << std::endl;
    
    init();
    MinimalEngine engine;
    
    // Open input file
    std::ifstream epd_file("test/mirror.epd");
    if (!epd_file.is_open()) {
        std::cerr << "Error: Could not open test/mirror.epd file!" << std::endl;
        return 1;
    }
    
    // Open output file
    std::ofstream output_file("mirror_test/mirror_eval_results.txt");
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not create output file!" << std::endl;
        return 1;
    }
    
    // Write header
    output_file << "Mirror Evaluation Test Results" << std::endl;
    output_file << "=============================" << std::endl;
    output_file << "Position | Original Eval | Mirrored Eval | Difference | Status" << std::endl;
    output_file << "---------|---------------|---------------|------------|--------" << std::endl;
    
    std::string line;
    int position_count = 0;
    int symmetric_count = 0;
    int asymmetric_count = 0;
    
    while (std::getline(epd_file, line)) {
        if (line.empty() || line[0] == '#') continue;  // Skip empty lines and comments
        
        position_count++;
        
        // Extract FEN from EPD line
        std::string fen = extract_fen_from_epd(line);
        if (fen.empty()) {
            std::cout << "Warning: Could not parse EPD line " << position_count << std::endl;
            continue;
        }
        
        std::cout << "Processing position " << position_count << "..." << std::endl;
        
        try {
            // Set up position
            Position pos;
            pos.set_from_fen(fen);
            
            // Evaluate original position
            int eval1 = engine.evalPosition(pos);
            
            // Create and evaluate mirrored position
            Position mirrored = engine.mirrorBoard(pos);
            int eval2 = engine.evalPosition(mirrored);
            
            // For symmetric evaluation, both positions should evaluate to the same
            // value from their respective side's perspective (no correction needed)
            int eval2_corrected = eval2;
            
            // Calculate difference
            int diff = eval1 - eval2_corrected;
            bool is_symmetric = (diff == 0);
            
            if (is_symmetric) {
                symmetric_count++;
            } else {
                asymmetric_count++;
            }
            
            // Write to output file
            output_file << std::setw(8) << position_count << " | ";
            output_file << std::setw(13) << eval1 << " | ";
            output_file << std::setw(13) << eval2_corrected << " | ";
            output_file << std::setw(10) << diff << " | ";
            output_file << (is_symmetric ? "PASS" : "FAIL") << std::endl;
            
            // Also write the FEN for reference
            output_file << "         FEN: " << fen << std::endl;
            
            if (!is_symmetric) {
                output_file << "         Mirrored FEN: " << mirrored.to_fen() << std::endl;
            }
            output_file << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Error processing position " << position_count << ": " << e.what() << std::endl;
            output_file << std::setw(8) << position_count << " | ERROR: " << e.what() << std::endl;
            output_file << "         FEN: " << fen << std::endl << std::endl;
        }
    }
    
    // Write summary
    output_file << std::endl;
    output_file << "=============================" << std::endl;
    output_file << "SUMMARY" << std::endl;
    output_file << "=============================" << std::endl;
    output_file << "Total positions tested: " << position_count << std::endl;
    output_file << "Symmetric (PASS): " << symmetric_count << std::endl;
    output_file << "Asymmetric (FAIL): " << asymmetric_count << std::endl;
    output_file << "Success rate: " << (position_count > 0 ? (100.0 * symmetric_count / position_count) : 0) << "%" << std::endl;
    
    std::cout << std::endl;
    std::cout << "=== Processing Complete ===" << std::endl;
    std::cout << "Total positions: " << position_count << std::endl;
    std::cout << "Symmetric: " << symmetric_count << std::endl;
    std::cout << "Asymmetric: " << asymmetric_count << std::endl;
    std::cout << "Results written to: mirror_test/mirror_eval_results.txt" << std::endl;
    
    epd_file.close();
    output_file.close();
    
    return 0;
}
