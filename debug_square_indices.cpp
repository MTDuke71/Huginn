/**
 * @file debug_square_indices.cpp
 * @brief Debug square index calculations to verify move coordinates
 */

#include <iostream>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/init.hpp"

using namespace std;

string square_to_string(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

int main() {
    cout << "=== Square Index Debug ===" << endl;
    
    // Test square indices
    cout << "Square index mapping:" << endl;
    cout << "a1 = 0: " << square_to_string(0) << endl;
    cout << "h1 = 7: " << square_to_string(7) << endl;
    cout << "a2 = 8: " << square_to_string(8) << endl;
    cout << "h2 = 15: " << square_to_string(15) << endl;
    cout << "a8 = 56: " << square_to_string(56) << endl;
    cout << "h8 = 63: " << square_to_string(63) << endl;
    
    cout << "\nTesting specific squares:" << endl;
    cout << "h2 (rank 2, file h): should be 15 = " << square_to_string(15) << endl;
    cout << "h3 (rank 3, file h): should be 23 = " << square_to_string(23) << endl;
    cout << "h4 (rank 4, file h): should be 31 = " << square_to_string(31) << endl;
    cout << "a2 (rank 2, file a): should be 8 = " << square_to_string(8) << endl;
    cout << "a4 (rank 4, file a): should be 24 = " << square_to_string(24) << endl;
    
    // Check what I used in the debug tool
    cout << "\nWhat I used in debug tool:" << endl;
    cout << "h2 = 55: " << square_to_string(55) << " (should be h8!)" << endl;
    cout << "h3 = 47: " << square_to_string(47) << " (should be h6!)" << endl;
    cout << "h4 = 39: " << square_to_string(39) << " (should be h5!)" << endl;
    cout << "a2 = 8: " << square_to_string(8) << " (correct)" << endl;
    cout << "a4 = 24: " << square_to_string(24) << " (correct)" << endl;
    
    return 0;
}