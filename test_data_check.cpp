#include "src/transposition_table.hpp"

int main() {
    // Create a transposition table instance
    GlobalTranspositionTable tt(1024);  // Small size for testing
    
    std::cout << "Running VICE data packing tests..." << std::endl;
    
    // Run the data check test
    tt.data_check();
    
    return 0;
}
