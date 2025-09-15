#include <iostream>
extern "C" {
    #include "fathom/src/tbprobe.h"
}

int main() {
    std::cout << "Testing Fathom library integration..." << std::endl;
    
    // Test initialization with real tablebase path
    bool initialized = tb_init("d:\\TB\\");
    std::cout << "tb_init result: " << (initialized ? "success" : "failed") << std::endl;
    
    // Test getting tablebase size
    unsigned largest = TB_LARGEST;
    std::cout << "Largest available tablebase: " << largest << " pieces" << std::endl;
    
    // Clean up
    tb_free();
    
    std::cout << "Fathom library test completed successfully!" << std::endl;
    return 0;
}