/**
 * @file input_checking.hpp
 * @brief Cross-platform input detection interface for chess engine responsiveness
 * 
 * This header defines a clean API for non-blocking input checking that enables
 * the chess engine to remain responsive to user commands during search operations.
 * The interface abstracts away platform differences and provides UCI protocol
 * compliance for chess GUI integration.
 * 
 * @author Huginn Chess Engine Team
 * @date 2025
 * @version 1.1
 */

#pragma once

#include "minimal_search.hpp"

namespace Huginn {

/**
 * @brief Check for available input without blocking execution
 * 
 * Non-blocking function that detects pending input on standard input stream.
 * Enables responsive engine operation during search by allowing periodic
 * checks for user commands without halting computation.
 * 
 * @return true if input is available to read, false otherwise
 * 
 * @note Not thread-safe - call from main search thread only
 * @note O(1) operation suitable for frequent polling
 * 
 * @see read_input() for processing detected input
 * @see Implementation in input_checking.cpp for platform-specific details
 */
bool input_is_waiting();

/**
 * @brief Read and process input commands for engine control
 * 
 * Reads available input and processes UCI commands ("quit", "stop") to control
 * engine operation. Works with `input_is_waiting()` for responsive command
 * handling during search operations.
 * 
 * **Commands:**
 * - "quit": Complete engine shutdown (sets quit + stopped flags)
 * - "stop": Halt current search only (sets stopped flag)
 * - Other input: Also stops search for maximum GUI responsiveness
 * 
 * @param info SearchInfo reference for engine control flags
 *             - Modified: info.quit (for "quit" command)
 *             - Modified: info.stopped (for any input)
 * 
 * @note Not thread-safe - call from main thread only
 * @note Only reads when input_is_waiting() confirms availability
 * 
 * @see input_is_waiting() for input detection
 * @see Implementation in input_checking.cpp for detailed behavior
 */
void read_input(SearchInfo& info);

} // namespace Huginn
