// init.hpp
#pragma once

/**
 * @namespace Huginn
 * @brief Contains core initialization functions for the Huginn engine.
 */
namespace Huginn {

/**
 * @brief Initializes all engine subsystems.
 * 
 * This function must be called once at program startup before using any engine functionality.
 * It sets up all necessary subsystems required for the engine to operate correctly.
 */
    void init();
    
/**
 * @brief Cleans up all engine subsystems.
 * 
 * This function should be called at program shutdown to properly clean up resources.
 * It's safe to call this multiple times.
 */
    void cleanup();
    
/**
 * @brief Checks if the engine has been properly initialized.
 * 
 * @return true if the engine has been initialized, false otherwise.
 */
    bool is_initialized();
} // namespace Huginn
