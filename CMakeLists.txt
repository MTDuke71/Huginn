cmake_minimum_required(VERSION 3.24)
project(Huginn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Debug Mode Configuration ----
option(ENABLE_DEBUG_ASSERTIONS "Enable debug assertions with file/line info" OFF)
if(ENABLE_DEBUG_ASSERTIONS OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    message(STATUS "Debug assertions enabled")
endif()

# ---- App ----
add_executable(huginn
  src/main.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
)
target_include_directories(huginn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(huginn PRIVATE cxx_std_17)

# ---- Tests ----
find_package(GTest CONFIG REQUIRED)  # using MSYS2 packaged googletest

include(CTest)       # adds BUILD_TESTING and CTest config
enable_testing()
include(GoogleTest)

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
  test/test_pawn_bitboard.cpp       # <-- combined pawn bitboard tests
  test/test_debug_validation.cpp    # <-- debug validation function tests
  test/test_fen_generation.cpp      # <-- FEN generation function tests
  test/test_sq_attacked.cpp         # <-- SqAttacked function tests
  test/test_sq_attacked_performance.cpp  # <-- SqAttacked performance tests
  test/test_s_move.cpp              # <-- S_MOVE structure tests
  test/test_rook_movegen.cpp        # <-- rook move generation tests
  test/test_bishop_queen_movegen.cpp # <-- bishop and queen move generation tests
  test/test_king_movegen.cpp        # <-- king move generation tests
  test/test_pawn_movegen.cpp        # <-- pawn move generation tests
  test/test_comprehensive_pawn.cpp  # <-- comprehensive pawn functionality demo
  test/test_all_piece_movegen.cpp   # <-- all piece move generation integration tests
  test/test_castling.cpp            # <-- castling move generation tests
  test/test_legal_moves.cpp         # <-- legal move validation tests
  test/test_complete_movegen.cpp    # <-- complete move generation system demo
  src/bitboard.cpp       # <-- bitboard implementation for tests
  src/init.cpp           # <-- engine initialization for tests
  src/board.cpp          # <-- board management functions for tests
  src/position.cpp       # <-- position implementation with Zobrist updates for tests
  src/zobrist.cpp        # <-- zobrist implementation for tests
  src/debug.cpp          # <-- debug and validation functions for tests
)
target_compile_features(huginn_tests PRIVATE cxx_std_17)
target_include_directories(huginn_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(huginn_tests PRIVATE GTest::gtest)  # keep test_main.cpp, so no gtest_main

# Discover tests at test time (prevents NOT_BUILT placeholders)
set(GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(huginn_tests DISCOVERY_TIMEOUT 30)

# Convenience: build tests + run them
add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --test-dir ${CMAKE_BINARY_DIR}
  DEPENDS huginn_tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ---- Perft Suite Demo ----
add_executable(perft_suite_demo
  debug/perft_suite_demo.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
)
target_include_directories(perft_suite_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(perft_suite_demo PRIVATE cxx_std_17)

# ---- Kiwipete Debug Tool ----
add_executable(debug_kiwipete
  debug/debug_kiwipete.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
)
target_include_directories(debug_kiwipete PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_kiwipete PRIVATE cxx_std_17)

# ---- Move Comparison Tool ----
add_executable(compare_moves
  debug/compare_moves.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
)
target_include_directories(compare_moves PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(compare_moves PRIVATE cxx_std_17)

# ---- Debug a2a4 Tool ----
add_executable(debug_a2a4
  debug/debug_a2a4.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
)
target_include_directories(debug_a2a4 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_a2a4 PRIVATE cxx_std_17)

# ---- Debug Opening Tool ----
add_executable(debug_opening
  debug/debug_opening.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
)
target_include_directories(debug_opening PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_opening PRIVATE cxx_std_17)

# ---- Kiwipete Perft Compare Tool ----
add_executable(kiwipete_perft_compare
  debug/kiwipete_perft_compare.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
)
target_include_directories(kiwipete_perft_compare PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(kiwipete_perft_compare PRIVATE cxx_std_17)

# ---- Debug En Passant Tool ----
add_executable(debug_en_passant
  debug/debug_en_passant.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
)
target_include_directories(debug_en_passant PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_en_passant PRIVATE cxx_std_17)

# ---- Test En Passant Tool ----
add_executable(test_en_passant
  debug/test_en_passant.cpp
  src/bitboard.cpp       # <-- bitboard implementation
  src/init.cpp           # <-- engine initialization
  src/board.cpp          # <-- board management functions
  src/position.cpp       # <-- position implementation with Zobrist updates
  src/zobrist.cpp        # <-- zobrist implementation
  src/debug.cpp          # <-- debug and validation functions
)
target_include_directories(test_en_passant PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(test_en_passant PRIVATE cxx_std_17)

# ---- Debug Kiwipete a2a4 Perft Tool ----
add_executable(debug_kiwipete_a2a4_perft
  debug/debug_kiwipete_a2a4_perft.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
)
target_include_directories(debug_kiwipete_a2a4_perft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_kiwipete_a2a4_perft PRIVATE cxx_std_17)

# ---- Debug Kiwipete Perft Tool ----
add_executable(debug_kiwipete_perft
  debug/debug_kiwipete_perft.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
)
target_include_directories(debug_kiwipete_perft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_kiwipete_perft PRIVATE cxx_std_17)

# ---- Debug Kiwipete Perft Compare Tool ----
add_executable(debug_kiwipete_perft_compare
  debug/debug_kiwipete_perft_compare.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
)
target_include_directories(debug_kiwipete_perft_compare PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_kiwipete_perft_compare PRIVATE cxx_std_17)

# ---- Debug Kiwipete Castle Tool ----
add_executable(debug_kiwipete_castle
  debug/debug_kiwipete_castle.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
)
target_include_directories(debug_kiwipete_castle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_kiwipete_castle PRIVATE cxx_std_17)

# Include debug demo
include(debug_demo.cmake)
