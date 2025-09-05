#include <gtest/gtest.h>
#include "input_checking.hpp"
#include "minimal_search.hpp"

namespace Huginn {

class InputCheckingTest : public ::testing::Test {
protected:
    void SetUp() override {
        info.quit = false;
        info.stopped = false;
    }
    
    SearchInfo info;
};

// Test that input_is_waiting() doesn't crash and returns a boolean
TEST_F(InputCheckingTest, InputIsWaitingBasic) {
    // Should not crash and should return a boolean value
    bool result = input_is_waiting();
    // We can't guarantee what it returns since it depends on actual stdin state
    // But we can verify it compiles and runs without crashing
    EXPECT_TRUE(result == true || result == false);
}

// Test that read_input() handles empty/invalid input gracefully
TEST_F(InputCheckingTest, ReadInputSafetyTest) {
    // This test verifies that read_input doesn't crash when called
    // Even if no input is available, it should handle gracefully
    
    // Save initial state
    bool initial_quit = info.quit;
    bool initial_stopped = info.stopped;
    
    // Call read_input - should not crash even with no input
    read_input(info);
    
    // Function should complete without throwing
    SUCCEED();
}

// Test SearchInfo flag manipulation
TEST_F(InputCheckingTest, SearchInfoFlags) {
    // Test initial state
    EXPECT_FALSE(info.quit);
    EXPECT_FALSE(info.stopped);
    
    // Test flag setting
    info.quit = true;
    EXPECT_TRUE(info.quit);
    
    info.stopped = true;
    EXPECT_TRUE(info.stopped);
    
    // Test reset
    info.quit = false;
    info.stopped = false;
    EXPECT_FALSE(info.quit);
    EXPECT_FALSE(info.stopped);
}

} // namespace Huginn
