#include "epd_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<TestPosition> parse_epd_file(const std::string& filename) {
    std::vector<TestPosition> positions;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return positions;
    }

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        TestPosition pos;
        
        // Read FEN string up to the first semicolon
        std::getline(iss, pos.fen, ';');

        // Trim trailing whitespace from FEN
        size_t end = pos.fen.find_last_not_of(" \t\n\r");
        if (end != std::string::npos) {
            pos.fen = pos.fen.substr(0, end + 1);
        }

        std::string segment;
        while (std::getline(iss, segment, ';')) {
            std::size_t depth_pos = segment.find("D");
            if (depth_pos != std::string::npos) {
                std::string depth_str = segment.substr(depth_pos + 1);
                std::size_t space_pos = depth_str.find(" ");
                if (space_pos != std::string::npos) {
                    int depth = std::stoi(depth_str.substr(0, space_pos));
                    uint64_t nodes = std::stoull(depth_str.substr(space_pos + 1));
                    pos.perft_results.push_back({depth, nodes});
                }
            }
        }
        positions.push_back(pos);
    }

    return positions;
}
