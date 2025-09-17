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
      # Set preprocessor define for optimized perft test to enable bitboard features
target_compile_definitions(perft_optimized_v4 PRIVATE BITBOARD_ENGINE=1)

# ---- Huginn2 Perft Optimized V5 Testarget_compile_target_compile_definitions(debug_move_comparison PRIVATE ULTRA_ENGINE=1)

# ---- Debug Board Display ----
add_huginn_executable(debug_board_display
    SOURCES
        debug_board_display.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

target_compile_definitions(debug_board_display PRIVATE ULTRA_ENGINE=1)

# ---- Debug Pawn Captures ----
add_huginn_executable(debug_pawn_captures
    SOURCES
        debug_pawn_captures.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

target_compile_definitions(debug_pawn_captures PRIVATE ULTRA_ENGINE=1)

# ---- Test Castling Debug ----

# ---- Debug Board State ----
add_huginn_executable(debug_board_state
    SOURCES
        debug_board_state.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

target_compile_definitions(debug_board_state PRIVATE ULTRA_ENGINE=1)

# ---- Test Castling Debug -------
add_huginn_executable(perft_optimized_v5
    SOURCES
        perft/perft_optimized_v5.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for optimized perft test to enable bitboard features
target_compile_definitions(perft_optimized_v5 PRIVATE BITBOARD_ENGINE=1)

# ---- Mirror Evaluation Test ----t(CMAKE_FIND_ROOT_PATH "")
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

# ---- Fathom Tablebase Option ----
option(ENABLE_FATHOM "Enable Fathom Syzygy tablebase support (requires C11 compiler)" OFF)

# ---- Toolchain Detection and Configuration ----
message(STATUS "=== Compiler Detection ===")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Using MSVC toolchain")
    # MSVC specific settings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(USE_KNIGHT_LOOKUP_TABLES)
    add_compile_definitions(USE_KING_LOOKUP_TABLES)
    add_compile_definitions(USE_PAWN_LOOKUP_TABLES)
    # Disabled optimizations for performance testing
    # add_compile_definitions(USE_RAZORING)
    # add_compile_definitions(USE_MULTI_CUT)
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
    # GCC specific settings
    add_compile_definitions(USE_KNIGHT_LOOKUP_TABLES)
    add_compile_definitions(USE_KING_LOOKUP_TABLES)
    add_compile_definitions(USE_PAWN_LOOKUP_TABLES)
    # Disabled optimizations for performance testing
    # add_compile_definitions(USE_RAZORING)
    # add_compile_definitions(USE_MULTI_CUT)
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
    # Fallback compile definitions for other compilers
    add_compile_definitions(USE_KNIGHT_LOOKUP_TABLES)
    add_compile_definitions(USE_KING_LOOKUP_TABLES)
    add_compile_definitions(USE_PAWN_LOOKUP_TABLES)
    # Disabled optimizations for performance testing
    # add_compile_definitions(USE_RAZORING)
    # add_compile_definitions(USE_MULTI_CUT)
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

# ---- Fathom Syzygy tablebase library ----
if(ENABLE_FATHOM)
    message(STATUS "Fathom tablebase support enabled")
    add_compile_definitions(FATHOM_AVAILABLE=1)
    set(FATHOM_SOURCES
        fathom/src/tbprobe.c
        fathom/src/tbchess.c
    )
    # Ensure C11 standard is used for Fathom
    set(CMAKE_C_STANDARD 17)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    
    # For MSVC, compile Fathom as C++ to avoid atomics issues
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set_source_files_properties(
            fathom/src/tbprobe.c
            fathom/src/tbchess.c
            PROPERTIES
            LANGUAGE CXX
            COMPILE_FLAGS "/TP"  # Force compilation as C++
        )
    else()
        # Set C-specific properties for Fathom source files on other compilers
        set_source_files_properties(
            fathom/src/tbprobe.c
            fathom/src/tbchess.c
            PROPERTIES
            LANGUAGE C
            COMPILE_FLAGS ""
        )
    endif()
else()
    message(STATUS "Fathom tablebase support disabled - using stub implementation")
    add_compile_definitions(FATHOM_AVAILABLE=0)
    set(FATHOM_SOURCES)
endif()

# ---- Source file lists ----
set(COMMON_SOURCES
    src/bitboard.cpp
    src/bitboard_movegen.cpp
    src/bitboard_position.cpp
    src/bitboard_movegen_pure.cpp
    src/bitboard_perft_optimized.cpp
    src/bitboard_attacks.cpp
    src/init.cpp
    src/position.cpp
    src/zobrist.cpp
    src/movegen_enhanced.cpp
    src/attack_detection.cpp
    src/attack_tables.cpp
    src/pawn_movegen_bb.cpp
    src/pawn_movegen_native_bb.cpp
    src/move.cpp
    src/board120.cpp
    src/chess_types.cpp
    src/evaluation.cpp
    src/input_checking.cpp
    src/minimal_search.cpp
    src/pvtable.cpp
    src/polyglot_book.cpp
    src/knight_lookup_tables.cpp
    src/king_lookup_tables.cpp
    src/pawn_lookup_tables.cpp
    src/syzygy_tablebase.cpp
    src/epd_parser.cpp
)

set(ENGINE_SOURCES
    ${COMMON_SOURCES}
    ${FATHOM_SOURCES}
    src/board.cpp

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
    src/minimal_search.cpp
)

# ---- Huginn 1.2 - Main Executable ----
if(ENABLE_FATHOM)
    set(HUGINN_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/fathom/src
        ${CMAKE_CURRENT_SOURCE_DIR}/ultra_engine
    )
else()
    set(HUGINN_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/ultra_engine
    )
endif()

add_huginn_executable(huginn
    SOURCES
        src/main.cpp
        ${ENGINE_SOURCES}
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Huginn2: Bitboard Migration Engine ----
add_huginn_executable(huginn2
    SOURCES
        src/main.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for huginn2 to enable bitboard features
target_compile_definitions(huginn2 PRIVATE BITBOARD_ENGINE=1)

# ---- Ultra Engine Sliding Piece Performance Test ----
add_huginn_executable(test_sliding_performance
    SOURCES
        test_sliding_performance.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Basic Ultra Engine Test ----
add_huginn_executable(test_basic_ultra
    SOURCES
        test_basic_ultra.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- UltraAttacks Only Test ----
add_huginn_executable(test_attacks_only
    SOURCES
        test_attacks_only.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Ultra Engine Pawn Performance Test ----
add_huginn_executable(test_pawn_performance
    SOURCES
        test_pawn_performance.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Ultra Engine Position System Test ----
add_huginn_executable(test_position_system
    SOURCES
        test_position_system.cpp
        ultra_engine/ultra_attacks.cpp
        ultra_engine/ultra_position.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Ultra Engine Special Moves Test ----
add_huginn_executable(test_special_moves
    SOURCES
        test_special_moves.cpp
        ultra_engine/ultra_attacks.cpp
        ultra_engine/ultra_position.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Ultra Engine Special Moves Test (Simple) ----
add_huginn_executable(test_special_moves_simple
    SOURCES
        test_special_moves_simple.cpp
        ultra_engine/ultra_attacks.cpp
        ultra_engine/ultra_position.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Huginn2 Perft Test ----
add_huginn_executable(perft_huginn2
    SOURCES
        perft/perft_suite_huginn2.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for perft test to enable bitboard features
target_compile_definitions(perft_huginn2 PRIVATE BITBOARD_ENGINE=1)

# ---- Huginn2 Perft Optimized V1 Test ----
add_huginn_executable(perft_optimized_v1
    SOURCES
        perft/perft_optimized_v1.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for optimized perft test to enable bitboard features
target_compile_definitions(perft_optimized_v1 PRIVATE BITBOARD_ENGINE=1)

# ---- Huginn2 Perft Optimized V2 Test ----
add_huginn_executable(perft_optimized_v2
    SOURCES
        perft/perft_optimized_v2.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for optimized perft test to enable bitboard features
target_compile_definitions(perft_optimized_v2 PRIVATE BITBOARD_ENGINE=1)

# ---- Huginn2 Perft Optimized V3 Test ----
add_huginn_executable(perft_optimized_v3
    SOURCES
        perft/perft_optimized_v3.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for optimized perft test to enable bitboard features
target_compile_definitions(perft_optimized_v3 PRIVATE BITBOARD_ENGINE=1)

# ---- Huginn2 Perft Optimized V4 Test ----
add_huginn_executable(perft_optimized_v4
    SOURCES
        perft/perft_optimized_v4.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for optimized perft test to enable bitboard features
target_compile_definitions(perft_optimized_v4 PRIVATE BITBOARD_ENGINE=1)

# ---- Huginn2 Perft Optimized V5 Test ----
add_huginn_executable(perft_optimized_v5
    SOURCES
        perft/perft_optimized_v5.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for optimized perft test to enable bitboard features
target_compile_definitions(perft_optimized_v5 PRIVATE BITBOARD_ENGINE=1)

# ---- Huginn2 Perft Optimized V6 Test ----
add_huginn_executable(perft_optimized_v6
    SOURCES
        perft/perft_optimized_v6.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for optimized perft test to enable bitboard features
target_compile_definitions(perft_optimized_v6 PRIVATE BITBOARD_ENGINE=1)

# ---- Huginn2 Perft Optimized V7 Test ----
add_huginn_executable(perft_optimized_v7
    SOURCES
        perft/perft_optimized_v7.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for optimized perft test to enable bitboard features
target_compile_definitions(perft_optimized_v7 PRIVATE BITBOARD_ENGINE=1)

# ---- TRUE UltraEngine Perft Test ----
add_huginn_executable(perft_ultra_engine
    SOURCES
        perft/perft_ultra_engine.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
        src/epd_parser.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for UltraEngine perft test
target_compile_definitions(perft_ultra_engine PRIVATE ULTRA_ENGINE=1)

# ---- Debug UltraPosition ----
add_huginn_executable(debug_ultra_position
    SOURCES
        debug_ultra_position.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

target_compile_definitions(debug_ultra_position PRIVATE ULTRA_ENGINE=1)

# ---- Debug UltraEngine Moves Detailed ----
add_huginn_executable(debug_ultra_moves_detailed
    SOURCES
        debug_ultra_moves_detailed.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

target_compile_definitions(debug_ultra_moves_detailed PRIVATE ULTRA_ENGINE=1)

# ---- Debug King Moves ----
add_huginn_executable(debug_king_moves
    SOURCES
        debug_king_moves.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

target_compile_definitions(debug_king_moves PRIVATE ULTRA_ENGINE=1)

# ---- Debug Move Comparison ----
add_huginn_executable(debug_move_comparison
    SOURCES
        debug_move_comparison.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

target_compile_definitions(debug_move_comparison PRIVATE ULTRA_ENGINE=1)

# ---- Debug FEN Parsing ----
add_huginn_executable(debug_fen_parsing
    SOURCES
        debug_fen_parsing.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

target_compile_definitions(debug_fen_parsing PRIVATE ULTRA_ENGINE=1)

# ---- Test Castling Debug ----
add_huginn_executable(test_castling_debug
    SOURCES
        test_castling_debug.cpp
        ultra_engine/ultra_position.cpp
        ultra_engine/ultra_move.cpp
        ultra_engine/ultra_attacks.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

target_compile_definitions(test_castling_debug PRIVATE ULTRA_ENGINE=1)

# ---- Ultra Engine Move System Test ----
add_executable(test_ultra_move
    ultra_engine/test_ultra_move.cpp
    ultra_engine/ultra_move.cpp
)

# Ultra engine requires C++17 for constexpr features
target_compile_features(test_ultra_move PRIVATE cxx_std_17)

# Optimize ultra engine test for maximum performance
if(MSVC)
    target_compile_options(test_ultra_move PRIVATE 
        /O2 /Ob2 /Oi /Ot /GL /DNDEBUG
        /fp:fast /arch:AVX2
    )
    target_link_options(test_ultra_move PRIVATE /LTCG)
endif()

# Include ultra engine directory
target_include_directories(test_ultra_move PRIVATE ultra_engine)

# ---- Ultra Engine Attack System Test ----
add_executable(test_ultra_attacks
    ultra_engine/test_ultra_attacks.cpp
    ultra_engine/ultra_attacks.cpp
    ultra_engine/ultra_move.cpp
)

# Ultra engine requires C++17 for constexpr features
target_compile_features(test_ultra_attacks PRIVATE cxx_std_17)

# Optimize ultra engine test for maximum performance
if(MSVC)
    target_compile_options(test_ultra_attacks PRIVATE 
        /O2 /Ob2 /Oi /Ot /GL /DNDEBUG
        /fp:fast /arch:AVX2
    )
    target_link_options(test_ultra_attacks PRIVATE /LTCG)
endif()

# Include ultra engine directory
target_include_directories(test_ultra_attacks PRIVATE ultra_engine)

# ---- Ultra Engine Knight Performance Test ----
add_executable(test_knight_performance
    ultra_engine/test_knight_performance.cpp
    ultra_engine/ultra_attacks.cpp
    ultra_engine/ultra_move.cpp
)

# Ultra engine requires C++17 for constexpr features
target_compile_features(test_knight_performance PRIVATE cxx_std_17)

# Optimize ultra engine test for maximum performance
if(MSVC)
    target_compile_options(test_knight_performance PRIVATE 
        /O2 /Ob2 /Oi /Ot /GL /DNDEBUG
        /fp:fast /arch:AVX2
    )
    target_link_options(test_knight_performance PRIVATE /LTCG)
endif()

# Include ultra engine directory
target_include_directories(test_knight_performance PRIVATE ultra_engine)

# ---- Mirror Evaluation Test ----
add_huginn_executable(mirror_eval_test
    SOURCES
        mirror_test/mirror_eval_test.cpp
        ${COMMON_SOURCES}
        src/minimal_search.cpp
        src/pvtable.cpp
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- Perft Suite VICE ----
add_huginn_executable(perft_suite_vice
    SOURCES
        perft/perft_suite_vice.cpp
        ${COMMON_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_huginn_executable(fast_perft_test
    SOURCES
        perft/fast_perft_test.cpp
        ${COMMON_SOURCES}
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
        test/test_knight_lookup.cpp
        test/test_king_lookup.cpp
        test/test_evaluation_symmetry.cpp
        test/debug_lookup_integration.cpp
    test/test_uci_time_and_stop_gtest.cpp
    test/test_uci_time_param_gtest.cpp
    test/test_uci_position_parsing.cpp
    test/test_uci_go_parsing.cpp
    test/test_input_checking.cpp
    )

    add_executable(huginn_tests
    ${TEST_SOURCES}
    ${ENGINE_SOURCES}
    src/minimal_search.cpp
    )

    target_compile_features(huginn_tests PRIVATE cxx_std_17)
    target_include_directories(huginn_tests PRIVATE 
        ${HUGINN_INCLUDE_DIRS}
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

# COMMENTED OUT TO SPEED UP BUILDS
# # Move generation comparison benchmark
# add_executable(movegen_comparison benchmark/movegen_comparison.cpp ${COMMON_SOURCES})
# target_compile_features(movegen_comparison PRIVATE cxx_std_17)
# target_include_directories(movegen_comparison PRIVATE src)
# target_compile_definitions(movegen_comparison PRIVATE USE_KNIGHT_LOOKUP_TABLES USE_KING_LOOKUP_TABLES USE_PAWN_LOOKUP_TABLES)

# # Evaluation benchmark for performance testing
# add_executable(eval_benchmark_old eval_benchmark_old.cpp ${COMMON_SOURCES})
# target_compile_features(eval_benchmark_old PRIVATE cxx_std_17)
# target_include_directories(eval_benchmark_old PRIVATE src)
# target_compile_definitions(eval_benchmark_old PRIVATE USE_KNIGHT_LOOKUP_TABLES USE_KING_LOOKUP_TABLES USE_PAWN_LOOKUP_TABLES)

# Attack detection benchmark for Phase 1 bitboard migration
add_executable(attack_benchmark benchmark/attack_benchmark.cpp ${COMMON_SOURCES})
target_compile_features(attack_benchmark PRIVATE cxx_std_17)
target_include_directories(attack_benchmark PRIVATE src)
target_compile_definitions(attack_benchmark PRIVATE USE_KNIGHT_LOOKUP_TABLES USE_KING_LOOKUP_TABLES USE_PAWN_LOOKUP_TABLES)

# Add ultra_engine directory to include paths globally
include_directories(${CMAKE_SOURCE_DIR}/ultra_engine)

# End of CMakeLists.txt






















