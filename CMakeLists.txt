cmake_minimum_required(VERSION 3.24)

# Prevent CMake from adding MSYS2 paths when using MSVC
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Clear any MSYS2-related variables that might interfere
    unset(ENV{MSYSTEM})
    unset(ENV{MSYS2_PATH_TYPE})
    
    # Explicitly prevent MSYS2 paths from being added
    set(CMAKE_FIND_ROOT_PATH "")
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    
    # Ensure we use Windows SDK paths, not MSYS2
    set(CMAKE_SYSTEM_INCLUDE_PATH "")
    set(CMAKE_SYSTEM_LIBRARY_PATH "")
endif()

project(Huginn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Assembly Generation Option ----
option(GENERATE_ASSEMBLY "Generate assembly listings for analysis" OFF)

# ---- Toolchain Detection and Configuration ----
message(STATUS "=== Compiler Detection ===")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Using MSVC toolchain")
    # MSVC specific settings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/W3)
    
    # Assembly generation for MSVC
    if(GENERATE_ASSEMBLY)
        add_compile_options(/FA)  # Generate assembly listings
        add_compile_options(/Fa${CMAKE_BINARY_DIR}/asm/)  # Output directory for .asm files
        message(STATUS "Assembly generation enabled - files will be in ${CMAKE_BINARY_DIR}/asm/")
    endif()
    
    # Ensure we don't pick up MSYS2 includes
    if(CMAKE_CXX_COMPILER_INCLUDE_DIRS)
        list(FILTER CMAKE_CXX_COMPILER_INCLUDE_DIRS EXCLUDE REGEX "msys64")
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using GNU/Clang toolchain")
    # GCC/Clang specific settings
    add_compile_options(-Wall -Wextra)
    
    # Assembly generation for GCC/Clang
    if(GENERATE_ASSEMBLY)
        add_compile_options(-S)  # Generate assembly
        add_compile_options(-fverbose-asm)  # Make assembly more readable
        message(STATUS "Assembly generation enabled")
    endif()
else()
    message(STATUS "Using unknown toolchain: ${CMAKE_CXX_COMPILER_ID}")
endif()

# ---- Compiler Optimizations for Release Builds ----
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-O3 -march=native)
        message(STATUS "Release optimizations enabled (-O3 -march=native)")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/O2)
        message(STATUS "Release optimizations enabled (/O2)")
    endif()
endif()

# ---- Debug Mode Configuration ----
option(ENABLE_DEBUG_ASSERTIONS "Enable debug assertions with file/line info" OFF)
if(ENABLE_DEBUG_ASSERTIONS OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    message(STATUS "Debug assertions enabled")
endif()

# ---- Find Threads (required for all targets) ----
find_package(Threads REQUIRED)

# ---- Find GTest with proper toolchain handling ----
set(GTEST_FOUND FALSE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # For MSVC, prefer vcpkg or CONFIG mode
    find_package(GTest CONFIG QUIET)
    if(GTest_FOUND)
        set(GTEST_FOUND TRUE)
        set(GTEST_LIBRARIES GTest::gtest)
        set(GTEST_MAIN_LIBRARIES GTest::gtest_main)
        message(STATUS "GTest found via CONFIG mode (likely vcpkg)")
    endif()
else()
    # For GCC/MinGW, try both CONFIG and MODULE modes
    find_package(GTest CONFIG QUIET)
    if(GTest_FOUND)
        set(GTEST_FOUND TRUE)
        if(TARGET GTest::gtest)
            set(GTEST_LIBRARIES GTest::gtest)
            set(GTEST_MAIN_LIBRARIES GTest::gtest_main)
        elseif(TARGET gtest)
            set(GTEST_LIBRARIES gtest)
            set(GTEST_MAIN_LIBRARIES gtest_main)
        endif()
        message(STATUS "GTest found via CONFIG mode")
    else()
        # Fallback to MODULE mode
        find_package(GTest MODULE QUIET)
        if(GTEST_FOUND)
            set(GTEST_LIBRARIES ${GTEST_LIBRARIES})
            set(GTEST_MAIN_LIBRARIES ${GTEST_MAIN_LIBRARIES})
            message(STATUS "GTest found via MODULE mode")
        endif()
    endif()
endif()

if(GTEST_FOUND)
    message(STATUS "GTest libraries: ${GTEST_LIBRARIES}")
    include(CTest)
    enable_testing()
    if(COMMAND gtest_discover_tests)
        include(GoogleTest)
    endif()
else()
    message(WARNING "GTest not found - tests will be skipped")
    message(STATUS "To install GTest:")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message(STATUS "  Use vcpkg: vcpkg install gtest:x64-windows")
    else()
        message(STATUS "  Use MSYS2: pacman -S mingw-w64-ucrt-x86_64-gtest")
    endif()
endif()

# ---- Function to create executables with common settings ----
function(add_huginn_executable target_name)
    cmake_parse_arguments(ARG "" "" "SOURCES;INCLUDE_DIRS" ${ARGN})
    
    add_executable(${target_name} ${ARG_SOURCES})
    target_compile_features(${target_name} PRIVATE cxx_std_17)
    target_link_libraries(${target_name} PRIVATE Threads::Threads)
    
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${target_name} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()
    
    # Set output directory to avoid conflicts
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    # Create assembly output directory for MSVC if assembly generation is enabled
    if(GENERATE_ASSEMBLY AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/asm)
    endif()
endfunction()

# ---- Function to add assembly generation for specific files ----
function(add_assembly_target target_name source_files)
    if(NOT GENERATE_ASSEMBLY)
        return()
    endif()
    
    foreach(source_file ${source_files})
        get_filename_component(filename ${source_file} NAME_WE)
        get_filename_component(filepath ${source_file} ABSOLUTE)
        
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            # MSVC assembly generation with proper flags
            add_custom_command(
                OUTPUT ${CMAKE_BINARY_DIR}/asm/${filename}.asm
                COMMAND ${CMAKE_CXX_COMPILER} 
                    /FA /Fa${CMAKE_BINARY_DIR}/asm/${filename}.asm 
                    /c ${filepath} 
                    /I${CMAKE_CURRENT_SOURCE_DIR}/src 
                    /I${CMAKE_CURRENT_SOURCE_DIR}/Engine3_src
                    /std:c++17
                    /O2
                    /D_CRT_SECURE_NO_WARNINGS
                    /EHsc
                    /DWIN32
                    /D_WINDOWS
                DEPENDS ${source_file}
                COMMENT "Generating assembly for ${filename}"
                VERBATIM
            )
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            # GCC/Clang assembly generation
            add_custom_command(
                OUTPUT ${CMAKE_BINARY_DIR}/asm/${filename}.s
                COMMAND ${CMAKE_CXX_COMPILER} 
                    -S -fverbose-asm -O2 -std=c++17 
                    -I${CMAKE_CURRENT_SOURCE_DIR}/src 
                    -I${CMAKE_CURRENT_SOURCE_DIR}/Engine3_src 
                    -o ${CMAKE_BINARY_DIR}/asm/${filename}.s 
                    ${filepath}
                DEPENDS ${source_file}
                COMMENT "Generating assembly for ${filename}"
                VERBATIM
            )
        endif()
    endforeach()
endfunction()

# ---- Source file lists ----
set(COMMON_SOURCES
    src/bitboard.cpp
    src/init.cpp
    src/position.cpp
    src/zobrist.cpp
    src/movegen_enhanced.cpp
    src/attack_detection.cpp
    src/move.cpp
    src/board120.cpp
    src/chess_types.cpp
)

set(ENGINE_SOURCES
    ${COMMON_SOURCES}
    src/board.cpp
    src/debug.cpp
    src/evaluation.cpp
    src/search.cpp
    src/uci.cpp
)

# ---- Performance Critical Sources (for assembly analysis) ----
set(PERFORMANCE_CRITICAL_SOURCES
    src/bitboard.cpp
    src/movegen_enhanced.cpp
    src/attack_detection.cpp
    src/position.cpp
    src/evaluation.cpp
    src/search.cpp
    Engine3_src/hybrid_evaluation.cpp
    Engine3_src/simple_search.cpp
)

# ---- Huginn 1.1 (Engine3) - Main Executable ----
add_huginn_executable(huginn
    SOURCES
        huginn3_main.cpp
        Engine3_src/simple_search.cpp
        Engine3_src/hybrid_evaluation.cpp
        ${COMMON_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine3_src
)

# ---- Legacy Huginn (Original Engine) ----
add_huginn_executable(huginn_legacy
    SOURCES
        src/main.cpp
        ${ENGINE_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- UCI Interface ----
add_huginn_executable(huginn_uci
    SOURCES
        src/uci_main.cpp
        ${ENGINE_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- Position Display Demo ----
add_huginn_executable(huginn_demo
    SOURCES
        src/demo_main.cpp
        ${COMMON_SOURCES}
        src/board.cpp
        src/debug.cpp
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- Move Generation Profiler ----
add_huginn_executable(movegen_profiler
    SOURCES
        src/movegen_profiler_main.cpp
        src/movegen_profiler.cpp
        ${COMMON_SOURCES}
        src/board.cpp
        src/debug.cpp
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- Profiler Validation Test ----
add_huginn_executable(profiler_test
    SOURCES
        src/profiler_test.cpp
        src/movegen_profiler.cpp
        ${COMMON_SOURCES}
        src/board.cpp
        src/debug.cpp
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- Assembly Generation Target ----
if(GENERATE_ASSEMBLY)
    # Create assembly directory
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/asm)
    
    # Add assembly generation for performance-critical files
    add_assembly_target(huginn_assembly "${PERFORMANCE_CRITICAL_SOURCES}")
    
    # Create a target that generates all assembly files
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(asm_extension ".asm")
    else()
        set(asm_extension ".s")
    endif()
    
    set(asm_files "")
    foreach(source_file ${PERFORMANCE_CRITICAL_SOURCES})
        get_filename_component(filename ${source_file} NAME_WE)
        list(APPEND asm_files ${CMAKE_BINARY_DIR}/asm/${filename}${asm_extension})
    endforeach()
    
    add_custom_target(generate_assembly
        DEPENDS ${asm_files}
        COMMENT "Generating all assembly files"
    )
    
    message(STATUS "Assembly generation target 'generate_assembly' created")
    message(STATUS "Use: cmake --build . --target generate_assembly")
endif()

# ---- Tests ----
if(GTEST_FOUND)
    set(TEST_SOURCES
        test/test_board120.cpp
        test/test_chess_types.cpp
        test/test_perft.cpp
        test/test_main.cpp
        test/test_zobrist.cpp
        test/test_bitboard.cpp
        test/test_init.cpp
        test/test_board.cpp
        test/test_offboard_detection.cpp
        test/test_fen.cpp
        test/test_print_position.cpp
        test/test_incremental_updates.cpp
        test/test_zobrist_incremental.cpp
        test/test_material_tracking.cpp
        test/test_debug_validation.cpp
        test/test_fen_generation.cpp
        test/test_sq_attacked.cpp
        test/test_sq_attacked_performance.cpp
        test/test_s_move.cpp
        test/test_piece_movegen.cpp
        test/test_pawn_consolidated.cpp
        test/test_castling.cpp
        test/test_legal_moves.cpp
        test/test_complete_movegen.cpp
        test/test_enhanced_movegen.cpp
        test/test_pawn_optimization.cpp
        test/test_king_optimization.cpp
        test/test_knight_optimization.cpp
        test/test_sliding_piece_optimization.cpp
        test/test_evaluation_symmetry.cpp
    )

    add_executable(huginn_tests
        ${TEST_SOURCES}
        ${ENGINE_SOURCES}
        Engine3_src/hybrid_evaluation.cpp
        Engine3_src/simple_search.cpp
    )
    
    target_compile_features(huginn_tests PRIVATE cxx_std_17)
    target_include_directories(huginn_tests PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src 
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine3_src
    )
    target_link_libraries(huginn_tests PRIVATE ${GTEST_LIBRARIES} Threads::Threads)
    
    set_target_properties(huginn_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Discover tests if GoogleTest module is available
    if(COMMAND gtest_discover_tests)
        set(GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
        gtest_discover_tests(huginn_tests DISCOVERY_TIMEOUT 30)
    endif()

    # Convenience target to build and run tests
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --test-dir ${CMAKE_BINARY_DIR}
        DEPENDS huginn_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building and running tests"
    )
    
    message(STATUS "Tests enabled - use 'cmake --build . --target check' to run")
else()
    message(STATUS "Tests disabled - GTest not found")
endif()

# ---- Demo executables ----
set(DEMO_NAMES
    perft_suite_demo
    opening_analyzer
    king_safety_test
    king_walking_specific_test
    move_selection_test
    f6_penalty_test
    systematic_move_tester
    self_play_tester
    critical_position_test
    timing_test
)

foreach(demo_name ${DEMO_NAMES})
    if(demo_name STREQUAL "perft_suite_demo")
        set(demo_source "perft/perft_suite_demo.cpp")
        set(demo_sources ${COMMON_SOURCES} src/board.cpp src/debug.cpp)
    else()
        set(demo_source "demos/${demo_name}.cpp")
        set(demo_sources ${ENGINE_SOURCES})
    endif()
    
    add_huginn_executable(${demo_name}
        SOURCES
            ${demo_source}
            ${demo_sources}
        INCLUDE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
endforeach()

# ---- Debug Symmetry Executable ----
add_huginn_executable(debug_symmetry
    SOURCES
        debug/debug_symmetry.cpp
        ${ENGINE_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- Summary ----
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
if(GENERATE_ASSEMBLY)
    message(STATUS "Assembly generation: Enabled")
else()
    message(STATUS "Assembly generation: Disabled (use -DGENERATE_ASSEMBLY=ON to enable)")
endif()
if(GTEST_FOUND)
    message(STATUS "Testing: Enabled")
else()
    message(STATUS "Testing: Disabled (GTest not found)")
endif()
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "=====================================")
