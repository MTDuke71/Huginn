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
option(ENABLE_SEARCH_INTEGRITY_ASSERTS "Enable BACKLOG #37 search make/unmake integrity diagnostics" OFF)
option(ENABLE_SEE_ORDER_SPLIT "Enable BACKLOG #6 SEE good/bad capture ordering split" ON)

if(ENABLE_DEBUG_ASSERTIONS OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    message(STATUS "Debug assertions enabled")
endif()

if(ENABLE_SEARCH_INTEGRITY_ASSERTS)
    add_compile_definitions(ENABLE_SEARCH_INTEGRITY_ASSERTS=1)
    message(STATUS "Search position integrity diagnostics enabled")
endif()

# BACKLOG #6: forward as an explicit 0/1 so -DENABLE_SEE_ORDER_SPLIT=0 builds
# the pre-t24 arm (the source default in search.cpp is the shipped default, 1).
if(ENABLE_SEE_ORDER_SPLIT)
    add_compile_definitions(ENABLE_SEE_ORDER_SPLIT=1)
else()
    add_compile_definitions(ENABLE_SEE_ORDER_SPLIT=0)
endif()

# BACKLOG #44 follow-up: root two-fold draw-avoidance (see the
# ENABLE_ROOT_TWOFOLD_AVOID block in src/search.cpp). Forward as an explicit
# 0/1 ALWAYS — never rely on a bare if(DEFINED) skip to silently fall through
# to the source default. A reused/incrementally-reconfigured build directory
# can leave a CACHE-variable-backed guard like this compiled as if 0 even
# when unset; see BACKLOG #50-queue notes (the same bug hit
# ENABLE_FUTILITY_PV_GUARD).
option(ENABLE_ROOT_TWOFOLD_AVOID "BACKLOG #44 follow-up: root two-fold draw-avoidance (shipped default)" ON)
if(ENABLE_ROOT_TWOFOLD_AVOID)
    add_compile_definitions(ENABLE_ROOT_TWOFOLD_AVOID=1)
    message(STATUS "Root two-fold draw-avoidance enabled")
else()
    add_compile_definitions(ENABLE_ROOT_TWOFOLD_AVOID=0)
    message(STATUS "Root two-fold draw-avoidance disabled (pre-t24 arm)")
endif()

# BACKLOG #52: check-aware quiescence (see ENABLE_QSEARCH_CHECK_EVASIONS in
# src/search.cpp). CANDIDATE — source default 0; -DENABLE_QSEARCH_CHECK_EVASIONS=ON
# builds the SPRT arm. Forwarded as an explicit 0/1 (same cache-variable
# rationale as ENABLE_ROOT_TWOFOLD_AVOID above).
option(ENABLE_QSEARCH_CHECK_EVASIONS "BACKLOG #52: check-aware quiescence (candidate arm)" ON)  # candidate branch default
if(ENABLE_QSEARCH_CHECK_EVASIONS)
    add_compile_definitions(ENABLE_QSEARCH_CHECK_EVASIONS=1)
    message(STATUS "Check-aware quiescence enabled (BACKLOG #52 candidate arm)")
else()
    add_compile_definitions(ENABLE_QSEARCH_CHECK_EVASIONS=0)
endif()

# BACKLOG #53: rule-50-aware TT eligibility (see ENABLE_RULE50_TT_GUARD in
# src/search.cpp). CANDIDATE — source default 0; -DENABLE_RULE50_TT_GUARD=ON
# builds the SPRT arm.
option(ENABLE_RULE50_TT_GUARD "BACKLOG #53: rule-50-aware TT eligibility guard (candidate arm)" OFF)
if(ENABLE_RULE50_TT_GUARD)
    add_compile_definitions(ENABLE_RULE50_TT_GUARD=1)
    message(STATUS "Rule-50 TT guard enabled (BACKLOG #53 candidate arm)")
else()
    add_compile_definitions(ENABLE_RULE50_TT_GUARD=0)
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
    # NOTE: MSVC emits C4717 ("function will cause runtime stack overflow")
    # for Fathom's tb_pop_count / tb_lsb / tb_pop_lsb helper functions in
    # tbprobe.c. Cause: tbchess.c's `#define popcount tb_pop_count` macro
    # leaks into tbprobe.c via `#include "tbchess.c"` at tbprobe.c:422 and
    # rewrites the helper bodies into self-recursive calls. We never call
    # these helpers (grep confirms zero call sites in src/), so the
    # warnings flag dead code that the linker drops in Release. Defining
    # TB_NO_HELPER_API here fails the standalone tbchess.c compilation
    # because tbchess.c also uses the popcount→tb_pop_count macro and
    # would lose the external definition. Leaving the warnings as
    # documented noise rather than patching upstream Fathom or refactoring
    # the build.
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
    src/movegen_bb.cpp
    src/init.cpp
    src/position.cpp
    src/zobrist.cpp
    src/movegen.cpp
    src/attack_detection.cpp
    src/attack_tables.cpp
    src/magic_bitboards.cpp
    src/chess_types.cpp
    src/evaluation.cpp
    src/input_checking.cpp
    src/search.cpp
    src/pvtable.cpp
    src/polyglot_book.cpp
    src/syzygy_tablebase.cpp
    src/see.cpp
    # Bundle Fathom (empty when ENABLE_FATHOM=OFF) so any target using
    # COMMON_SOURCES picks up the Syzygy symbols that syzygy_tablebase.cpp
    # references. Without this, perft_suite / mirror_eval_test link-fail.
    ${FATHOM_SOURCES}
)

set(ENGINE_SOURCES
    ${COMMON_SOURCES}
    ${FATHOM_SOURCES}
    src/uci.cpp
    src/uci_utils.cpp
    src/search.cpp
    src/pvtable.cpp
)

# ---- Performance Critical Sources (for assembly analysis) ----
set(PERFORMANCE_CRITICAL_SOURCES
    src/bitboard.cpp
    src/movegen.cpp
    src/attack_detection.cpp
    src/position.cpp
    src/search.cpp
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

# ---- Huginn 2.0 Engine ----
add_huginn_executable(huginn
    SOURCES
        src/main.cpp
        ${ENGINE_SOURCES}
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Mirror Evaluation Test ----
add_huginn_executable(mirror_eval_test
    SOURCES
        mirror_test/mirror_eval_test.cpp
        ${COMMON_SOURCES}
        src/search.cpp
        src/pvtable.cpp
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Perft Suite ----
add_huginn_executable(perft_suite
    SOURCES
        perft/perft_suite.cpp
        ${COMMON_SOURCES}
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)

# ---- Texel tuner (#9) ----
# Built with -DHUGINN_TUNING so the eval parameter tables become mutable inline
# globals the tuner can optimize (release engine keeps them constexpr). Not part
# of the default build's critical path; build explicitly: --target huginn_tuner
add_huginn_executable(huginn_tuner
    SOURCES
        tools/texel/tuner.cpp
        ${COMMON_SOURCES}
    INCLUDE_DIRS
        ${HUGINN_INCLUDE_DIRS}
)
target_compile_definitions(huginn_tuner PRIVATE HUGINN_TUNING)

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
        test/test_chess_types.cpp
        test/test_perft.cpp
        test/test_main.cpp
        test/test_zobrist.cpp
        test/test_bitboard.cpp
        test/test_init.cpp
        test/test_board.cpp
        test/test_fen.cpp
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
        test/test_movegen.cpp
        test/test_evaluation_symmetry.cpp
    test/test_uci_time_and_stop_gtest.cpp
    test/test_uci_time_param_gtest.cpp
    test/test_uci_position_parsing.cpp
    test/test_uci_go_parsing.cpp
    test/test_input_checking.cpp
    test/test_see.cpp
    test/test_search_determinism.cpp
    test/test_pv_display.cpp
    test/test_position_integrity.cpp
    test/test_eval_outposts.cpp
    test/test_root_twofold_avoid.cpp
    test/test_audit_criticals.cpp
    )

    add_executable(huginn_tests
    ${TEST_SOURCES}
    ${ENGINE_SOURCES}
    src/search.cpp
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

# End of CMakeLists.txt




















