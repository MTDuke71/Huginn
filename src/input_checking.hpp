#pragma once

#include "minimal_search.hpp"

namespace Huginn {

// VICE Part 70: Input checking functions for interrupt detection during search

// Check if input is waiting on stdin without blocking
// Returns true if input is available to read
bool input_is_waiting();

// Read and process input if available 
// Sets info->quit to true if "quit" command is received
// Sets info->stopped to true if any input is detected (to interrupt search)
void read_input(SearchInfo& info);

} // namespace Huginn
