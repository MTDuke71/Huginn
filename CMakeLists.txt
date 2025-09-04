cmake_minimum_required(VERSION 3.22)

# Prefer MSVC but allow fallback
if(WIN32)
    # Aggressive MSYS2 interference prevention
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Clear all MSYS2-related environment variables
        unset(ENV{MSYSTEM})
        unset(ENV{MSYS2_PATH_TYPE})
        unset(ENV{PKG_CONFIG_PATH})
        unset(ENV{ACLOCAL_PATH})
        unset(ENV{MANPATH})
        
        # Force CMake to not use any non-Windows paths
        set(CMAKE_FIND_ROOT_PATH "")
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
        set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
        
        # Clear system paths that might include MSYS2
        set(CMAKE_SYSTEM_INCLUDE_PATH "")
        set(CMAKE_SYSTEM_LIBRARY_PATH "")
        
        # Explicitly exclude MSYS2 paths from include search
        set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "")
        set(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES "")
        
        # Remove any MSYS2 paths from PATH during build
        string(REPLACE "C:/msys64/ucrt64/bin;" "" CMAKE_PROGRAM_PATH "$ENV{PATH}")
        string(REPLACE "C:/msys64/ucrt64/include;" "" CMAKE_INCLUDE_PATH "$ENV{INCLUDE}")
        string(REPLACE "C:/msys64/ucrt64/lib;" "" CMAKE_LIBRARY_PATH "$ENV{LIB}")
        
        message(STATUS "Using MSVC toolchain with aggressive MSYS2 interference prevention")
    else()
        message(STATUS "Not using MSVC - MSYS2 interference prevention disabled")
    endif()
endif()

project(Huginn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
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
    
    # Suppress specific warnings that are common in this codebase and safe
    add_compile_options(
        /wd4244  # Conversion warnings (int64 to int) - safe in our context
        /wd4267  # Size_t conversion warnings - safe in our context
    )
    
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
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using GCC compiler")
    # GCC optimizations for Release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(
            -O3                 # Maximum optimization
            -march=native       # Optimize for current CPU
            -mtune=native       # Tune for current CPU  
            -flto               # Link-time optimization
            -ffast-math         # Fast math optimizations
            -funroll-loops      # Loop unrolling
            -finline-functions  # Aggressive inlining
        )
        add_link_options(-flto)  # LTO for linking
    endif()
else()
    message(WARNING "Untested compiler: ${CMAKE_CXX_COMPILER_ID}. Proceeding anyway...")
endif()

# ---- Build Type Validation ----
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# ---- Compiler Optimizations ----
# MSVC-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Multi-config generator (Visual Studio) with MSVC
    if(CMAKE_CONFIGURATION_TYPES)
        # Configuration-specific flags using generator expressions
        add_compile_options(
            $<$<CONFIG:Release>:/O2>          # Full optimization
            $<$<CONFIG:Release>:/Oi>          # Enable intrinsic functions
            $<$<CONFIG:Release>:/Ot>          # Favor speed over size
            $<$<CONFIG:Release>:/Oy>          # Omit frame pointers
            $<$<CONFIG:Release>:/GT>          # Enable fiber-safe thread-local storage
            $<$<CONFIG:Release>:/GL>          # Whole program optimization
            $<$<CONFIG:Release>:/arch:AVX2>   # Enable AVX2 instructions
            $<$<CONFIG:Release>:/fp:fast>     # Fast floating point
            $<$<CONFIG:Release>:/GS>
            $<$<CONFIG:Release>:/guard:cf>
            $<$<CONFIG:Debug>:/Od>
            $<$<CONFIG:Debug>:/Zi>
        )
        
        # Global warning suppression for all configurations
        add_compile_options(/wd4244 /wd4267)  # int/size_t conversions
        message(STATUS "Multi-config generator: Advanced MSVC optimizations enabled")
        message(STATUS "MSVC warning suppression enabled for safe conversions")
    else()
        # Single-config generator (Ninja, Make) with MSVC
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_compile_options(/O2)          # Full optimization
            add_compile_options(/Oi)          # Enable intrinsic functions  
            add_compile_options(/Ot)          # Favor speed over size
            add_compile_options(/Oy)          # Omit frame pointers
            add_compile_options(/GT)          # Enable fiber-safe thread-local storage
            add_compile_options(/GL)          # Whole program optimization
            add_compile_options(/arch:AVX2)   # Enable AVX2 instructions
            add_compile_options(/fp:fast)     # Fast floating point
            # Add security features for MSVC
            add_compile_options(/GS /guard:cf)  # Buffer security check + Control Flow Guard
            
            # Link-time optimizations
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF")
            
            message(STATUS "Release optimizations enabled with advanced MSVC flags")
            message(STATUS "AVX2, intrinsics, and whole program optimization enabled")
        else()
            # Debug build specific options
            add_compile_options(/Od /Zi)
            message(STATUS "Debug build: optimizations disabled, debug symbols enabled")
        endif()
    endif()
endif()

# ---- Debug Mode Configuration ----
option(ENABLE_DEBUG_ASSERTIONS "Enable debug assertions with file/line info" OFF)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer (Debug builds only)" OFF)

if(ENABLE_DEBUG_ASSERTIONS OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    message(STATUS "Debug assertions enabled")
endif()

# ---- Sanitizers for enhanced debugging ----
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Sanitizers are not available with MSVC. Consider using static analysis tools instead.")
endif()

# ---- Find Threads (required for all targets) ----
find_package(Threads REQUIRED)

# ---- Find GTest with FetchContent ----
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable testing
include(CTest)
enable_testing()
include(GoogleTest)

set(GTEST_FOUND TRUE)
set(GTEST_LIBRARIES gtest)
set(GTEST_MAIN_LIBRARIES gtest_main)
message(STATUS "GoogleTest configured via FetchContent")

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
        
        # MSVC assembly generation with proper flags
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/asm/${filename}.asm
            COMMAND ${CMAKE_CXX_COMPILER} 
                /FA /Fa${CMAKE_BINARY_DIR}/asm/${filename}.asm 
                /c ${filepath} 
                /I${CMAKE_CURRENT_SOURCE_DIR}/src 
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
    src/uci.cpp
    src/uci_utils.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
)

# ---- Performance Critical Sources (for assembly analysis) ----
set(PERFORMANCE_CRITICAL_SOURCES
    src/bitboard.cpp
    src/movegen_enhanced.cpp
    src/attack_detection.cpp
    src/position.cpp
    src/evaluation.cpp
    src/minimal_search.cpp
)

# ---- Huginn 1.1 - Main Executable ----
add_huginn_executable(huginn
    SOURCES
        src/main.cpp
        src/evaluation.cpp
        ${ENGINE_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- Move Generation Profiler ----


# ---- Assembly Generation Target ----
if(GENERATE_ASSEMBLY)
    # Create assembly directory
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/asm)
    
    # Add assembly generation for performance-critical files
    add_assembly_target(huginn_assembly "${PERFORMANCE_CRITICAL_SOURCES}")
    
    # Create a target that generates all assembly files
    set(asm_extension ".asm")  # MSVC assembly extension
    
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
        test/test_knight_optimization.cpp
        test/test_evaluation_symmetry.cpp
    )

    add_executable(huginn_tests
        ${TEST_SOURCES}
        ${ENGINE_SOURCES}
        src/evaluation.cpp
        src/minimal_search.cpp
    )

    target_compile_features(huginn_tests PRIVATE cxx_std_17)
    target_include_directories(huginn_tests PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src 
    )
    target_link_libraries(huginn_tests PRIVATE ${GTEST_LIBRARIES} Threads::Threads)

    set_target_properties(huginn_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Always include GoogleTest before discovering tests
    include(GoogleTest)
    gtest_discover_tests(
        huginn_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        DISCOVERY_TIMEOUT 30
    )

    # Convenience target to build and run tests
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS huginn_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        COMMENT "Building and running tests"
    )

    message(STATUS "Tests enabled - use 'cmake --build . --target check' to run")
else()
    message(STATUS "Tests disabled - GTest not found")
endif()

# ---- Minimal Engine ----
add_huginn_executable(huginn_minimal
    SOURCES
        src/minimal_main.cpp
        src/minimal_search.cpp
        src/pvtable.cpp
        ${COMMON_SOURCES}
        src/movegen_enhanced.cpp
        src/board.cpp
        src/position.cpp
        src/attack_detection.cpp
        src/debug.cpp
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- Demo executables ----
set(DEMO_NAMES
    perft_suite_demo
    perft_suite_vice
    opening_analyzer
    king_safety_test
    king_walking_specific_test
    move_selection_test
    f6_penalty_test
    systematic_move_tester
    self_play_tester
    critical_position_test
    timing_test
    perft6_huginn
    repetition_detection_demo
)

foreach(demo_name ${DEMO_NAMES})
    if(demo_name STREQUAL "perft_suite_demo")
        set(demo_source "perft/perft_suite_demo.cpp")
        set(demo_sources ${COMMON_SOURCES} src/board.cpp src/debug.cpp)
    elseif(demo_name STREQUAL "perft_suite_vice")
        set(demo_source "perft/perft_suite_vice.cpp")
        set(demo_sources ${COMMON_SOURCES} src/board.cpp src/debug.cpp)
    elseif(demo_name STREQUAL "self_play_tester")
        set(demo_source "demos/${demo_name}.cpp")
        set(demo_sources ${ENGINE_SOURCES} src/minimal_search.cpp src/pvtable.cpp)  # Add minimal search + PVTable for self-play
    else()
        set(demo_source "demos/${demo_name}.cpp")
        set(demo_sources ${ENGINE_SOURCES})
    endif()
    
    add_huginn_executable(${demo_name}
        SOURCES
            ${demo_source}
            ${demo_sources}
            src/evaluation.cpp
            src/minimal_search.cpp
        INCLUDE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
endforeach()

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

# Test executable for VICE Part 57 - clearForSearch
add_executable(test_clearforsearch 
    test_clearforsearch.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_clearforsearch PRIVATE cxx_std_17)
target_include_directories(test_clearforsearch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_clearforsearch PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Demo executable for iterative deepening showcase
add_executable(demo_iterative_deepening 
    demo_iterative_deepening.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(demo_iterative_deepening PRIVATE cxx_std_17)
target_include_directories(demo_iterative_deepening PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
# Test for FEN parsing
add_executable(test_fen 
    debug/test_fen.cpp
    ${COMMON_SOURCES}
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_fen PRIVATE cxx_std_17)
target_include_directories(test_fen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
# Debug WAC.1 search issue
add_executable(debug_wac1 
    debug/debug_wac1.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(debug_wac1 PRIVATE cxx_std_17)
target_include_directories(debug_wac1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
# Test UCI depth command (bypass time management)
add_executable(test_uci_depth 
    debug/test_uci_depth.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_uci_depth PRIVATE cxx_std_17)
target_include_directories(test_uci_depth PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_uci_depth PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# VICE Part 60: Basic Search Testing with move ordering statistics
add_executable(test_vice_part60 
    debug/test_vice_part60.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_vice_part60 PRIVATE cxx_std_17)
target_include_directories(test_vice_part60 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_vice_part60 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# VICE Part 60: Basic Search Testing
add_executable(test_basic_search 
    test_basic_search.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_basic_search PRIVATE cxx_std_17)
target_include_directories(test_basic_search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_basic_search PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Debug test for basic search issues
add_executable(debug_test_basic 
    debug_test_basic.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(debug_test_basic PRIVATE cxx_std_17)
target_include_directories(debug_test_basic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(debug_test_basic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Evaluation Analysis Tool
add_executable(eval_analysis 
    debug/eval_analysis.cpp
    ${COMMON_SOURCES}
    src/position.cpp
    src/evaluation.cpp
    src/movegen_enhanced.cpp
    src/board.cpp
    src/attack_detection.cpp
)
target_compile_features(eval_analysis PRIVATE cxx_std_17)
target_include_directories(eval_analysis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(eval_analysis PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Development vs Pawn Move Test
add_executable(development_test 
    debug/development_test.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/position.cpp
    src/evaluation.cpp
    src/movegen_enhanced.cpp
    src/board.cpp
    src/attack_detection.cpp
)
target_compile_features(development_test PRIVATE cxx_std_17)
target_include_directories(development_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(development_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Deep Search Evaluation Test
add_executable(deep_search_test 
    debug/deep_search_test.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/position.cpp
    src/evaluation.cpp
    src/movegen_enhanced.cpp
    src/board.cpp
    src/attack_detection.cpp
)
target_compile_features(deep_search_test PRIVATE cxx_std_17)
target_include_directories(deep_search_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(deep_search_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


# MVV-LVA Move Ordering Test
add_executable(test_mvv_lva 
    debug/test_mvv_lva.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_mvv_lva PRIVATE cxx_std_17)
target_include_directories(test_mvv_lva PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_mvv_lva PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# MVV-LVA Search Efficiency Test
add_executable(test_mvv_lva_search 
    debug/test_mvv_lva_search.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_mvv_lva_search PRIVATE cxx_std_17)
target_include_directories(test_mvv_lva_search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_mvv_lva_search PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Unified Search System Test
add_executable(test_unified_search 
    debug/test_unified_search.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_unified_search PRIVATE cxx_std_17)
target_include_directories(test_unified_search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_unified_search PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Legacy alpha_beta Warning Test
add_executable(test_legacy_warning 
    debug/test_legacy_warning.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_legacy_warning PRIVATE cxx_std_17)
target_include_directories(test_legacy_warning PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_legacy_warning PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# VICE Part 62: Pick Move Test - Move Ordering Improvement
add_executable(test_pick_move 
    debug/test_pick_move.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_pick_move PRIVATE cxx_std_17)
target_include_directories(test_pick_move PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_pick_move PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# VICE Part 62: Pick Move Comparison Test
add_executable(test_pick_move_comparison 
    debug/test_pick_move_comparison.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    ${COMMON_SOURCES}
    src/movegen_enhanced.cpp
    src/board.cpp
    src/position.cpp
    src/attack_detection.cpp
    src/debug.cpp
)
target_compile_features(test_pick_move_comparison PRIVATE cxx_std_17)
target_include_directories(test_pick_move_comparison PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_pick_move_comparison PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ======================================
# CTest Registration
# ======================================
# Register all test executables with CTest

# Core Tests
add_test(NAME test_basic_search COMMAND test_basic_search)
add_test(NAME test_mvv_lva COMMAND test_mvv_lva)
add_test(NAME test_mvv_lva_search COMMAND test_mvv_lva_search) 
add_test(NAME test_unified_search COMMAND test_unified_search)
add_test(NAME test_legacy_warning COMMAND test_legacy_warning)

# VICE Part 62 Tests
add_test(NAME test_pick_move COMMAND test_pick_move)
add_test(NAME test_pick_move_comparison COMMAND test_pick_move_comparison)

# Set test properties
set_tests_properties(test_basic_search test_mvv_lva test_mvv_lva_search test_unified_search 
                     test_legacy_warning test_pick_move test_pick_move_comparison
    PROPERTIES
    TIMEOUT 120  # 2 minutes timeout for each test
)
