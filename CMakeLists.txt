cmake_minimum_required(VERSION 3.24)
project(Huginn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Compiler Optimizations for Release Builds ----
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
        message(STATUS "Release optimizations enabled (-O3 -march=native)")
    elseif(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
        message(STATUS "Release optimizations enabled (/O2)")
    endif()
endif()

# ---- Debug Mode Configuration ----
option(ENABLE_DEBUG_ASSERTIONS "Enable debug assertions with file/line info" OFF)
if(ENABLE_DEBUG_ASSERTIONS OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    message(STATUS "Debug assertions enabled")
endif()

# ---- Huginn 1.1 (Engine3) - Main Executable ----
add_executable(huginn
  huginn3_main.cpp
  Engine3_src/simple_search.cpp
  Engine3_src/hybrid_evaluation.cpp
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
)
target_include_directories(huginn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/Engine3_src)
target_compile_features(huginn PRIVATE cxx_std_17)
find_package(Threads REQUIRED)
target_link_libraries(huginn PRIVATE Threads::Threads)

# ---- Legacy Huginn (Original Engine) ----
add_executable(huginn_legacy
  src/main.cpp
  src/uci.cpp            # <-- UCI interface implementation
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- position evaluation implementation
  src/search.cpp         # <-- search engine implementation
)
target_include_directories(huginn_legacy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(huginn_legacy PRIVATE cxx_std_17)
find_package(Threads REQUIRED)
target_link_libraries(huginn_legacy PRIVATE Threads::Threads)

# ---- UCI Interface ----
add_executable(huginn_uci
  src/uci_main.cpp
  src/uci.cpp            # <-- UCI interface implementation
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- position evaluation implementation
  src/search.cpp         # <-- search engine implementation
)
target_include_directories(huginn_uci PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(huginn_uci PRIVATE cxx_std_17)
target_link_libraries(huginn_uci PRIVATE Threads::Threads)

# ---- Position Display Demo ----
add_executable(huginn_demo
  src/demo_main.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
)
target_include_directories(huginn_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(huginn_demo PRIVATE cxx_std_17)
target_link_libraries(huginn_demo PRIVATE Threads::Threads)

# ---- Move Generation Profiler ----
add_executable(movegen_profiler
  src/movegen_profiler_main.cpp
  src/movegen_profiler.cpp # <-- detailed profiling implementation
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
)
target_include_directories(movegen_profiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(movegen_profiler PRIVATE cxx_std_17)
target_link_libraries(movegen_profiler PRIVATE Threads::Threads)

# ---- Profiler Validation Test ----
add_executable(profiler_test
  src/profiler_test.cpp
  src/movegen_profiler.cpp # <-- detailed profiling implementation
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
)
target_include_directories(profiler_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(profiler_test PRIVATE cxx_std_17)
target_link_libraries(profiler_test PRIVATE Threads::Threads)

# ---- Tests ----
find_package(GTest CONFIG QUIET)  # using MSYS2 packaged googletest

if(GTest_FOUND)
    include(CTest)       # adds BUILD_TESTING and CTest config
    enable_testing()
    include(GoogleTest)
else()
    message(WARNING "GTest not found, skipping tests")
endif()

if(GTest_FOUND)
add_executable(huginn_tests
  test/test_board120.cpp
  test/test_chess_types.cpp
  test/test_perft.cpp
  test/test_main.cpp
  test/test_zobrist.cpp
  test/test_bitboard.cpp    # <-- bitboard tests
  test/test_init.cpp        # <-- initialization tests
  test/test_board.cpp       # <-- board management tests
  test/test_offboard_detection.cpp  # <-- offboard detection demo
  test/test_fen.cpp         # <-- FEN parsing tests
  test/test_print_position.cpp      # <-- position printing tests
  test/test_incremental_updates.cpp # <-- incremental update performance tests
  test/test_zobrist_incremental.cpp # <-- zobrist incremental XOR update tests
  test/test_material_tracking.cpp   # <-- material score tracking tests
  test/test_debug_validation.cpp    # <-- debug validation function tests
  test/test_fen_generation.cpp      # <-- FEN generation function tests
  test/test_sq_attacked.cpp         # <-- SqAttacked function tests
  test/test_sq_attacked_performance.cpp  # <-- SqAttacked performance tests
  test/test_s_move.cpp              # <-- S_MOVE structure tests
  test/test_piece_movegen.cpp       # <-- consolidated piece move generation tests
  test/test_pawn_consolidated.cpp   # <-- consolidated pawn functionality tests
  test/test_castling.cpp            # <-- castling move generation tests
  test/test_legal_moves.cpp         # <-- legal move validation tests
  test/test_complete_movegen.cpp    # <-- complete move generation system demo
  test/test_enhanced_movegen.cpp    # <-- enhanced move generation system tests
  test/test_pawn_optimization.cpp   # <-- pawn promotion optimization tests
  test/test_king_optimization.cpp   # <-- king move generation optimization tests
  test/test_knight_optimization.cpp # <-- knight move generation optimization tests
  test/test_sliding_piece_optimization.cpp # <-- sliding piece move generation optimization tests
  test/test_evaluation_symmetry.cpp # <-- evaluation symmetry tests
  src/bitboard.cpp       # <-- bitboard implementation for tests
  src/init.cpp           # <-- engine initialization for tests
  src/board.cpp          # <-- board management functions for tests
  src/position.cpp       # <-- position implementation with Zobrist updates for tests
  src/zobrist.cpp        # <-- zobrist implementation for tests
  src/debug.cpp          # <-- debug and validation functions for tests
  src/movegen_enhanced.cpp # <-- enhanced move generation for tests
  src/attack_detection.cpp # <-- attack detection implementation for tests
  src/move.cpp           # <-- move structure implementation for tests
  src/board120.cpp       # <-- coordinate utilities implementation for tests
  src/chess_types.cpp    # <-- piece type utilities implementation for tests
  src/evaluation.cpp     # <-- position evaluation implementation for tests
  src/search.cpp         # <-- search engine implementation for tests
)
target_compile_features(huginn_tests PRIVATE cxx_std_17)
target_include_directories(huginn_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(huginn_tests PRIVATE GTest::gtest Threads::Threads)  # keep test_main.cpp, so no gtest_main

# Discover tests at test time (prevents NOT_BUILT placeholders)
set(GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(huginn_tests DISCOVERY_TIMEOUT 30)

# Convenience: build tests + run them
add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --test-dir ${CMAKE_BINARY_DIR}
  DEPENDS huginn_tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
endif()

# ---- Perft Suite Demo ----
add_executable(perft_suite_demo
  perft/perft_suite_demo.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
)
target_include_directories(perft_suite_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(perft_suite_demo PRIVATE cxx_std_17)

# ---- Opening Analyzer Demo ----
add_executable(opening_analyzer
  demos/opening_analyzer.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- evaluation system
  src/search.cpp         # <-- search engine
)
target_include_directories(opening_analyzer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(opening_analyzer PRIVATE cxx_std_17)

# ---- King Safety Test ----
add_executable(king_safety_test
  demos/king_safety_test.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- evaluation system
  src/search.cpp         # <-- search engine
)
target_include_directories(king_safety_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(king_safety_test PRIVATE cxx_std_17)

# ---- King Walking Specific Test ----
add_executable(king_walking_specific_test
  demos/king_walking_specific_test.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- evaluation system
  src/search.cpp         # <-- search engine
)
target_include_directories(king_walking_specific_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(king_walking_specific_test PRIVATE cxx_std_17)

# ---- Move Selection Test ----
add_executable(move_selection_test
  demos/move_selection_test.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- evaluation system
  src/search.cpp         # <-- search engine
)
target_include_directories(move_selection_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(move_selection_test PRIVATE cxx_std_17)

# ---- F6 Penalty Test ----
add_executable(f6_penalty_test
  demos/f6_penalty_test.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- evaluation system
  src/search.cpp         # <-- search engine
)
target_include_directories(f6_penalty_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(f6_penalty_test PRIVATE cxx_std_17)

# ---- Systematic Move Tester ----
add_executable(systematic_move_tester
  demos/systematic_move_tester.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- evaluation system
  src/search.cpp         # <-- search engine
)
target_include_directories(systematic_move_tester PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(systematic_move_tester PRIVATE cxx_std_17)

# ---- Self-Play Tester ----
add_executable(self_play_tester
  demos/self_play_tester.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- evaluation system
  src/search.cpp         # <-- search engine
)
target_include_directories(self_play_tester PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(self_play_tester PRIVATE cxx_std_17)

# ---- Critical Position Test ----
add_executable(critical_position_test
  demos/critical_position_test.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- evaluation system
  src/search.cpp         # <-- search engine
)
target_include_directories(critical_position_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(critical_position_test PRIVATE cxx_std_17)

# ---- Timing Test ----
add_executable(timing_test
  demos/timing_test.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
  src/movegen_enhanced.cpp # <-- enhanced move generation system
  src/attack_detection.cpp # <-- attack detection implementation
  src/move.cpp           # <-- move structure implementation
  src/board120.cpp       # <-- coordinate utilities implementation
  src/chess_types.cpp    # <-- piece type utilities implementation
  src/evaluation.cpp     # <-- evaluation system
  src/search.cpp         # <-- search engine
)
target_include_directories(timing_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(timing_test PRIVATE cxx_std_17)
add_executable(debug_symmetry debug/debug_symmetry.cpp src/evaluation.cpp src/position.cpp src/board.cpp src/init.cpp src/movegen_enhanced.cpp src/zobrist.cpp src/attack_detection.cpp src/move.cpp src/board120.cpp src/chess_types.cpp src/debug.cpp src/bitboard.cpp src/search.cpp src/uci.cpp)
target_include_directories(debug_symmetry PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_symmetry PRIVATE cxx_std_17)
