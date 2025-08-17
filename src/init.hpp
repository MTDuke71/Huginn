// init.hpp
#pragma once

namespace Huginn {
    // Initialize all engine subsystems
    // Call once at program startup before using any engine functionality
    void init();
    
    // Check if engine has been properly initialized
    bool is_initialized();
} // namespace Huginn
