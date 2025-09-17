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
add_executable(debug_fresh_vs_sequence debug_fresh_vs_sequence.cpp ${COMMON_SOURCES})
target_compile_features(debug_fresh_vs_sequence PRIVATE cxx_std_17)
target_include_directories(debug_fresh_vs_sequence PRIVATE src)
target_compile_definitions(debug_fresh_vs_sequence PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_next_en_passant_bug debug_next_en_passant_bug.cpp ${COMMON_SOURCES})
target_compile_features(debug_next_en_passant_bug PRIVATE cxx_std_17)
target_include_directories(debug_next_en_passant_bug PRIVATE src)
target_compile_definitions(debug_next_en_passant_bug PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(analyze_d2c1_depth3 analyze_d2c1_depth3.cpp ${COMMON_SOURCES})
target_compile_features(analyze_d2c1_depth3 PRIVATE cxx_std_17)
target_include_directories(analyze_d2c1_depth3 PRIVATE src)
target_compile_definitions(analyze_d2c1_depth3 PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_move_generation_discrepancy test_move_generation_discrepancy.cpp ${COMMON_SOURCES})
target_compile_features(test_move_generation_discrepancy PRIVATE cxx_std_17)
target_include_directories(test_move_generation_discrepancy PRIVATE src)
target_compile_definitions(test_move_generation_discrepancy PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(find_missing_moves find_missing_moves.cpp ${COMMON_SOURCES})
target_compile_features(find_missing_moves PRIVATE cxx_std_17)
target_include_directories(find_missing_moves PRIVATE src)
target_compile_definitions(find_missing_moves PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(compare_position_implementations compare_position_implementations.cpp ${COMMON_SOURCES})
target_compile_features(compare_position_implementations PRIVATE cxx_std_17)
target_include_directories(compare_position_implementations PRIVATE src)
target_compile_definitions(compare_position_implementations PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_vice_kiwipete test_vice_kiwipete.cpp ${COMMON_SOURCES})
target_compile_features(test_vice_kiwipete PRIVATE cxx_std_17)
target_include_directories(test_vice_kiwipete PRIVATE src)
target_compile_definitions(test_vice_kiwipete PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_depth3 test_depth3.cpp ${COMMON_SOURCES})
target_compile_features(test_depth3 PRIVATE cxx_std_17)
target_include_directories(test_depth3 PRIVATE src)
target_compile_definitions(test_depth3 PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(analyze_f3f5_tree analyze_f3f5_tree.cpp ${COMMON_SOURCES})
target_compile_features(analyze_f3f5_tree PRIVATE cxx_std_17)
target_include_directories(analyze_f3f5_tree PRIVATE src)
target_compile_definitions(analyze_f3f5_tree PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_attack_detection test_attack_detection.cpp ${COMMON_SOURCES})
target_compile_features(test_attack_detection PRIVATE cxx_std_17)
target_include_directories(test_attack_detection PRIVATE src)
target_compile_definitions(test_attack_detection PRIVATE USE_PAWN_LOOKUP_TABLES)

# End of CMakeLists.txt    
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
    )
else()
    set(HUGINN_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
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

# ---- Huginn2 Perft Test ----
add_huginn_executable(perft_huginn2
    SOURCES
        perft_huginn2.cpp
        ${ENGINE_SOURCES}
        src/bitboard_benchmark.cpp  # Add bitboard performance framework
        src/bitboard_movegen.cpp    # Add actual bitboard move generation
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# Set preprocessor define for perft test to enable bitboard features
target_compile_definitions(perft_huginn2 PRIVATE BITBOARD_ENGINE=1)

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

# Evaluation benchmark for new piece list approach
add_executable(eval_benchmark_new eval_benchmark_new.cpp ${COMMON_SOURCES})
target_compile_features(eval_benchmark_new PRIVATE cxx_std_17)
target_include_directories(eval_benchmark_new PRIVATE src)
target_compile_definitions(eval_benchmark_new PRIVATE USE_KNIGHT_LOOKUP_TABLES USE_KING_LOOKUP_TABLES USE_PAWN_LOOKUP_TABLES)

# Attack detection benchmark for Phase 1 bitboard migration
add_executable(attack_benchmark benchmark/attack_benchmark.cpp ${COMMON_SOURCES})
target_compile_features(attack_benchmark PRIVATE cxx_std_17)
target_include_directories(attack_benchmark PRIVATE src)
target_compile_definitions(attack_benchmark PRIVATE USE_KNIGHT_LOOKUP_TABLES USE_KING_LOOKUP_TABLES USE_PAWN_LOOKUP_TABLES)

# Pawn move generation benchmark for Phase 2 bitboard migration
add_executable(pawn_movegen_benchmark pawn_movegen_benchmark.cpp ${COMMON_SOURCES})
target_compile_features(pawn_movegen_benchmark PRIVATE cxx_std_20)
target_include_directories(pawn_movegen_benchmark PRIVATE src)
target_compile_definitions(pawn_movegen_benchmark PRIVATE USE_KNIGHT_LOOKUP_TABLES USE_KING_LOOKUP_TABLES USE_PAWN_LOOKUP_TABLES)

# Native bitboard benchmark for conversion overhead elimination
add_executable(native_bitboard_benchmark native_bitboard_benchmark.cpp ${COMMON_SOURCES})
target_compile_features(native_bitboard_benchmark PRIVATE cxx_std_20)
target_include_directories(native_bitboard_benchmark PRIVATE src)
target_compile_definitions(native_bitboard_benchmark PRIVATE USE_KNIGHT_LOOKUP_TABLES USE_KING_LOOKUP_TABLES USE_PAWN_LOOKUP_TABLES)

# End of CMakeLists.txt
set(DEBUG_SOURCES
    src/bitboard.cpp
    src/bitboard_movegen.cpp
    src/init.cpp
    src/position.cpp
    src/zobrist.cpp
    src/attack_detection.cpp
    src/movegen_enhanced.cpp
    src/move.cpp
    src/board120.cpp
    src/chess_types.cpp
    src/evaluation.cpp
    src/knight_lookup_tables.cpp
    src/king_lookup_tables.cpp
    src/pawn_lookup_tables.cpp
)


# Huginn2 bitboard perft suite (first 2 positions)
add_executable(perft_suite_huginn2 perft/perft_suite_huginn2.cpp ${DEBUG_SOURCES})
target_compile_features(perft_suite_huginn2 PRIVATE cxx_std_17)
target_include_directories(perft_suite_huginn2 PRIVATE src)
target_compile_definitions(perft_suite_huginn2 PRIVATE USE_KNIGHT_LOOKUP_TABLES USE_KING_LOOKUP_TABLES USE_PAWN_LOOKUP_TABLES BITBOARD_ENGINE=1)

add_executable(test_occupied_bitboard test_occupied_bitboard.cpp ${COMMON_SOURCES})
target_compile_features(test_occupied_bitboard PRIVATE cxx_std_17)
target_include_directories(test_occupied_bitboard PRIVATE src)
target_compile_definitions(test_occupied_bitboard PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(kiwipete_systematic kiwipete_systematic.cpp ${COMMON_SOURCES})
target_compile_features(kiwipete_systematic PRIVATE cxx_std_17)
target_include_directories(kiwipete_systematic PRIVATE src)
target_compile_definitions(kiwipete_systematic PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(simple_f3f5_test simple_f3f5_test.cpp ${COMMON_SOURCES})
target_compile_features(simple_f3f5_test PRIVATE cxx_std_17)
target_include_directories(simple_f3f5_test PRIVATE src)
target_compile_definitions(simple_f3f5_test PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_position_state debug_position_state.cpp ${COMMON_SOURCES})
target_compile_features(debug_position_state PRIVATE cxx_std_17)
target_include_directories(debug_position_state PRIVATE src)
target_compile_definitions(debug_position_state PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_bitboard_position debug_bitboard_position.cpp ${COMMON_SOURCES})
target_compile_features(debug_bitboard_position PRIVATE cxx_std_17)
target_include_directories(debug_bitboard_position PRIVATE src)
target_compile_definitions(debug_bitboard_position PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(simple_bitboard_debug simple_bitboard_debug.cpp ${COMMON_SOURCES})
target_compile_features(simple_bitboard_debug PRIVATE cxx_std_17)
target_include_directories(simple_bitboard_debug PRIVATE src)
target_compile_definitions(simple_bitboard_debug PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_specific_fen test_specific_fen.cpp ${COMMON_SOURCES})
target_compile_features(test_specific_fen PRIVATE cxx_std_17)
target_include_directories(test_specific_fen PRIVATE src)
target_compile_definitions(test_specific_fen PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(verify_f3f5 verify_f3f5.cpp ${SOURCE_FILES})
# target_link_libraries(verify_f3f5 ${PROJECT_NAME}_lib)

add_executable(analyze_f3f5 analyze_f3f5.cpp)
target_compile_features(analyze_f3f5 PRIVATE cxx_std_17)
target_include_directories(analyze_f3f5 PRIVATE src)
target_compile_definitions(analyze_f3f5 PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_attack_detection_detailed test_attack_detection_detailed.cpp ${COMMON_SOURCES})
target_compile_features(test_attack_detection_detailed PRIVATE cxx_std_17)
target_include_directories(test_attack_detection_detailed PRIVATE src)
target_compile_definitions(test_attack_detection_detailed PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_attack_tables test_attack_tables.cpp ${COMMON_SOURCES})
target_compile_features(test_attack_tables PRIVATE cxx_std_17)
target_include_directories(test_attack_tables PRIVATE src)
target_compile_definitions(test_attack_tables PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_pawn_attacks test_pawn_attacks.cpp ${COMMON_SOURCES})
target_compile_features(test_pawn_attacks PRIVATE cxx_std_17)
target_include_directories(test_pawn_attacks PRIVATE src)
target_compile_definitions(test_pawn_attacks PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_knight_attacks debug_knight_attacks.cpp ${COMMON_SOURCES})
target_compile_features(debug_knight_attacks PRIVATE cxx_std_17)
target_include_directories(debug_knight_attacks PRIVATE src)
target_compile_definitions(debug_knight_attacks PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_knight_table test_knight_table.cpp ${COMMON_SOURCES})
target_compile_features(test_knight_table PRIVATE cxx_std_17)
target_include_directories(test_knight_table PRIVATE src)
target_compile_definitions(test_knight_table PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(e5_debug e5_debug.cpp ${COMMON_SOURCES})
target_compile_features(e5_debug PRIVATE cxx_std_17)
target_include_directories(e5_debug PRIVATE src)
target_compile_definitions(e5_debug PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(compare_moves compare_moves.cpp ${COMMON_SOURCES})
target_compile_features(compare_moves PRIVATE cxx_std_17)
target_include_directories(compare_moves PRIVATE src)
target_compile_definitions(compare_moves PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_f3f5_bug test_f3f5_bug.cpp ${COMMON_SOURCES})
target_compile_features(test_f3f5_bug PRIVATE cxx_std_17)
target_include_directories(test_f3f5_bug PRIVATE src)
target_compile_definitions(test_f3f5_bug PRIVATE USE_PAWN_LOOKUP_TABLES)
add_executable(verify_bitboard_correct verify_bitboard_correct.cpp ${COMMON_SOURCES})
target_compile_features(verify_bitboard_correct PRIVATE cxx_std_17)
target_include_directories(verify_bitboard_correct PRIVATE src)
target_compile_definitions(verify_bitboard_correct PRIVATE USE_PAWN_LOOKUP_TABLES)
add_executable(debug_state_corruption debug_state_corruption.cpp ${COMMON_SOURCES})
target_compile_features(debug_state_corruption PRIVATE cxx_std_17)
target_include_directories(debug_state_corruption PRIVATE src)
target_compile_definitions(debug_state_corruption PRIVATE USE_PAWN_LOOKUP_TABLES)
add_executable(compare_move_vs_fen compare_move_vs_fen.cpp ${COMMON_SOURCES})
target_compile_features(compare_move_vs_fen PRIVATE cxx_std_17)
target_include_directories(compare_move_vs_fen PRIVATE src)
target_compile_definitions(compare_move_vs_fen PRIVATE USE_PAWN_LOOKUP_TABLES)
add_executable(debug_original_kiwipete debug_original_kiwipete.cpp ${COMMON_SOURCES})
target_compile_features(debug_original_kiwipete PRIVATE cxx_std_17)
target_include_directories(debug_original_kiwipete PRIVATE src)
target_compile_definitions(debug_original_kiwipete PRIVATE USE_PAWN_LOOKUP_TABLES)
add_executable(test_position_perft test_position_perft.cpp ${COMMON_SOURCES})
target_compile_features(test_position_perft PRIVATE cxx_std_17)
target_include_directories(test_position_perft PRIVATE src)
target_compile_definitions(test_position_perft PRIVATE USE_PAWN_LOOKUP_TABLES)
add_executable(kiwipete_depth5_divide kiwipete_depth5_divide.cpp ${COMMON_SOURCES})
target_compile_features(kiwipete_depth5_divide PRIVATE cxx_std_17)
target_include_directories(kiwipete_depth5_divide PRIVATE src)
target_compile_definitions(kiwipete_depth5_divide PRIVATE USE_PAWN_LOOKUP_TABLES)
add_executable(compare_depth5_reference compare_depth5_reference.cpp ${COMMON_SOURCES})
target_compile_features(compare_depth5_reference PRIVATE cxx_std_17)
target_include_directories(compare_depth5_reference PRIVATE src)
target_compile_definitions(compare_depth5_reference PRIVATE USE_PAWN_LOOKUP_TABLES)
add_executable(check_move_order_pattern check_move_order_pattern.cpp ${COMMON_SOURCES})
target_compile_features(check_move_order_pattern PRIVATE cxx_std_17)
target_include_directories(check_move_order_pattern PRIVATE src)
target_compile_definitions(check_move_order_pattern PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_e5_knight debug_e5_knight.cpp ${COMMON_SOURCES})
target_compile_features(debug_e5_knight PRIVATE cxx_std_17)
target_include_directories(debug_e5_knight PRIVATE src)
target_compile_definitions(debug_e5_knight PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(compare_positions compare_positions.cpp ${COMMON_SOURCES})
target_compile_features(compare_positions PRIVATE cxx_std_17)
target_include_directories(compare_positions PRIVATE src)
target_compile_definitions(compare_positions PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_e5f7_specific debug_e5f7_specific.cpp ${COMMON_SOURCES})
target_compile_features(debug_e5f7_specific PRIVATE cxx_std_17)
target_include_directories(debug_e5f7_specific PRIVATE src)
target_compile_definitions(debug_e5f7_specific PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(fen_integrity_checker fen_integrity_checker.cpp ${COMMON_SOURCES})
target_compile_features(fen_integrity_checker PRIVATE cxx_std_17)
target_include_directories(fen_integrity_checker PRIVATE src)
target_compile_definitions(fen_integrity_checker PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_unmake_detailed debug_unmake_detailed.cpp ${COMMON_SOURCES})
target_compile_features(debug_unmake_detailed PRIVATE cxx_std_17)
target_include_directories(debug_unmake_detailed PRIVATE src)
target_compile_definitions(debug_unmake_detailed PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_knight_captures_pawn debug_knight_captures_pawn.cpp ${COMMON_SOURCES})
target_compile_features(debug_knight_captures_pawn PRIVATE cxx_std_17)
target_include_directories(debug_knight_captures_pawn PRIVATE src)
target_compile_definitions(debug_knight_captures_pawn PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(detailed_move_comparison detailed_move_comparison.cpp ${COMMON_SOURCES})
target_compile_features(detailed_move_comparison PRIVATE cxx_std_17)
target_include_directories(detailed_move_comparison PRIVATE src)
target_compile_definitions(detailed_move_comparison PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_castling_rights debug_castling_rights.cpp ${COMMON_SOURCES})
target_compile_features(debug_castling_rights PRIVATE cxx_std_17)
target_include_directories(debug_castling_rights PRIVATE src)
target_compile_definitions(debug_castling_rights PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(analyze_castling_differences analyze_castling_differences.cpp ${COMMON_SOURCES})
target_compile_features(analyze_castling_differences PRIVATE cxx_std_17)
target_include_directories(analyze_castling_differences PRIVATE src)
target_compile_definitions(analyze_castling_differences PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(verify_castling_moves verify_castling_moves.cpp ${COMMON_SOURCES})
target_compile_features(verify_castling_moves PRIVATE cxx_std_17)
target_include_directories(verify_castling_moves PRIVATE src)
target_compile_definitions(verify_castling_moves PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(perft_depth1_knight_positions perft_depth1_knight_positions.cpp ${COMMON_SOURCES})
target_compile_features(perft_depth1_knight_positions PRIVATE cxx_std_17)
target_include_directories(perft_depth1_knight_positions PRIVATE src)
target_compile_definitions(perft_depth1_knight_positions PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(debug_knight_capture_specific debug_knight_capture_specific.cpp ${COMMON_SOURCES})
target_compile_features(debug_knight_capture_specific PRIVATE cxx_std_17)
target_include_directories(debug_knight_capture_specific PRIVATE src)
target_compile_definitions(debug_knight_capture_specific PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_knight_perft_specific test_knight_perft_specific.cpp ${COMMON_SOURCES})
target_compile_features(test_knight_perft_specific PRIVATE cxx_std_17)
target_include_directories(test_knight_perft_specific PRIVATE src)
target_compile_definitions(test_knight_perft_specific PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_cold_vs_warm_perft test_cold_vs_warm_perft.cpp ${COMMON_SOURCES})
target_compile_features(test_cold_vs_warm_perft PRIVATE cxx_std_17)
target_include_directories(test_cold_vs_warm_perft PRIVATE src)
target_compile_definitions(test_cold_vs_warm_perft PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_fen_integrity test_fen_integrity.cpp ${COMMON_SOURCES})
target_compile_features(test_fen_integrity PRIVATE cxx_std_17)
target_include_directories(test_fen_integrity PRIVATE src)
target_compile_definitions(test_fen_integrity PRIVATE USE_PAWN_LOOKUP_TABLES)

add_executable(test_depth4_early_exit test_depth4_early_exit.cpp ${COMMON_SOURCES})
target_compile_features(test_depth4_early_exit PRIVATE cxx_std_17)
target_include_directories(test_depth4_early_exit PRIVATE src)
target_compile_definitions(test_depth4_early_exit PRIVATE USE_PAWN_LOOKUP_TABLES)

# Debug tool for +82 node difference
add_executable(debug_82_difference debug_82_difference.cpp ${COMMON_SOURCES})
target_compile_features(debug_82_difference PRIVATE cxx_std_17)
target_include_directories(debug_82_difference PRIVATE src)
target_compile_definitions(debug_82_difference PRIVATE USE_PAWN_LOOKUP_TABLES)

# Find exact source of depth-4 bug
add_executable(find_depth4_bug find_depth4_bug.cpp ${COMMON_SOURCES})
target_compile_features(find_depth4_bug PRIVATE cxx_std_17)
target_include_directories(find_depth4_bug PRIVATE src)
target_compile_definitions(find_depth4_bug PRIVATE USE_PAWN_LOOKUP_TABLES)

# Debug the h7->g6 move that causes +82 nodes
add_executable(debug_h7g6_move debug_h7g6_move.cpp ${COMMON_SOURCES})
target_compile_features(debug_h7g6_move PRIVATE cxx_std_17)
target_include_directories(debug_h7g6_move PRIVATE src)
target_compile_definitions(debug_h7g6_move PRIVATE USE_PAWN_LOOKUP_TABLES)

# Compare move lists after h7->g6 to find the discrepancy
add_executable(compare_move_lists compare_move_lists.cpp ${COMMON_SOURCES})
target_compile_features(compare_move_lists PRIVATE cxx_std_17)
target_include_directories(compare_move_lists PRIVATE src)
target_compile_definitions(compare_move_lists PRIVATE USE_PAWN_LOOKUP_TABLES)

# Divide analysis from position after h7->g6
add_executable(divide_after_h7g6 divide_after_h7g6.cpp ${COMMON_SOURCES})
target_compile_features(divide_after_h7g6 PRIVATE cxx_std_17)
target_include_directories(divide_after_h7g6 PRIVATE src)
target_compile_definitions(divide_after_h7g6 PRIVATE USE_PAWN_LOOKUP_TABLES)

# Comparison tool for VICE vs BitboardPosition
add_executable(compare_vice_vs_bitboard compare_vice_vs_bitboard.cpp ${COMMON_SOURCES})
target_compile_features(compare_vice_vs_bitboard PRIVATE cxx_std_17)
target_include_directories(compare_vice_vs_bitboard PRIVATE src)
target_compile_definitions(compare_vice_vs_bitboard PRIVATE USE_PAWN_LOOKUP_TABLES)

# Analyze specific moves to understand the +2 systematic overcounting
add_executable(analyze_specific_moves analyze_specific_moves.cpp ${COMMON_SOURCES})
target_compile_features(analyze_specific_moves PRIVATE cxx_std_17)
target_include_directories(analyze_specific_moves PRIVATE src)
target_compile_definitions(analyze_specific_moves PRIVATE USE_PAWN_LOOKUP_TABLES)

# Deep analysis of moves to find where +2 overcounting occurs
add_executable(deep_move_analysis deep_move_analysis.cpp ${COMMON_SOURCES})
target_compile_features(deep_move_analysis PRIVATE cxx_std_17)
target_include_directories(deep_move_analysis PRIVATE src)
target_compile_definitions(deep_move_analysis PRIVATE USE_PAWN_LOOKUP_TABLES)

# Find all submoves that cause +1 errors leading to +2 total
add_executable(find_all_plus1_submoves find_all_plus1_submoves.cpp ${COMMON_SOURCES})
target_compile_features(find_all_plus1_submoves PRIVATE cxx_std_17)
target_include_directories(find_all_plus1_submoves PRIVATE src)
target_compile_definitions(find_all_plus1_submoves PRIVATE USE_PAWN_LOOKUP_TABLES)

# Debug pawn promotion moves to understand the systematic bug
add_executable(debug_promotion_moves debug_promotion_moves.cpp ${COMMON_SOURCES})
target_compile_features(debug_promotion_moves PRIVATE cxx_std_17)
target_include_directories(debug_promotion_moves PRIVATE src)
target_compile_definitions(debug_promotion_moves PRIVATE USE_PAWN_LOOKUP_TABLES)

# Debug promotion move perft and undo functionality
add_executable(debug_promotion_perft debug_promotion_perft.cpp ${COMMON_SOURCES})
target_compile_features(debug_promotion_perft PRIVATE cxx_std_17)
target_include_directories(debug_promotion_perft PRIVATE src)
target_compile_definitions(debug_promotion_perft PRIVATE USE_PAWN_LOOKUP_TABLES)

# Test promotion move matching and undo correctness
add_executable(test_promotion_matching test_promotion_matching.cpp ${COMMON_SOURCES})
target_compile_features(test_promotion_matching PRIVATE cxx_std_17)
target_include_directories(test_promotion_matching PRIVATE src)
target_compile_definitions(test_promotion_matching PRIVATE USE_PAWN_LOOKUP_TABLES)

# Deep perft test with performance timing for depths 1-5
add_executable(deep_perft_test deep_perft_test.cpp ${COMMON_SOURCES})
target_compile_features(deep_perft_test PRIVATE cxx_std_17)
target_include_directories(deep_perft_test PRIVATE src)
target_compile_definitions(deep_perft_test PRIVATE USE_PAWN_LOOKUP_TABLES)
