#include <iostream>

// Test if we can include Fathom headers
extern "C" {
    #include "fathom/src/tbprobe.h"
}

int main() {
    std::cout << "Fathom header included successfully!" << std::endl;
    std::cout << "tb_init test - available functions:" << std::endl;
    std::cout << "tb_init: " << (void*)tb_init << std::endl;
    std::cout << "tb_free: " << (void*)tb_free << std::endl;
    return 0;
}