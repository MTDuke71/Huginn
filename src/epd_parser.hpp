#ifndef EPD_PARSER_HPP
#define EPD_PARSER_HPP

#include <string>
#include <vector>
#include <cstdint>

struct TestPosition {
    std::string fen;
    std::vector<std::pair<int, uint64_t>> perft_results;
};

std::vector<TestPosition> parse_epd_file(const std::string& filename);

#endif // EPD_PARSER_HPP
