cmake_minimum_required(VERSION 3.24)

# Create a separate debug demo executable
add_executable(debug_demo
  demos/debug_demo.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
)
target_include_directories(debug_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(debug_demo PRIVATE cxx_std_17)

# Create a separate castling validation demo executable
add_executable(castling_demo
  demos/castling_demo.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
)
target_include_directories(castling_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(castling_demo PRIVATE cxx_std_17)

# Create a separate FEN generation demo executable
add_executable(fen_demo
  demos/fen_demo.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
)
target_include_directories(fen_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(fen_demo PRIVATE cxx_std_17)

# Create a separate SqAttacked demo executable
add_executable(sq_attacked_demo
  demos/sq_attacked_demo.cpp
  src/bitboard.cpp
  src/init.cpp
  src/board.cpp
  src/position.cpp
  src/zobrist.cpp
  src/debug.cpp
)
target_include_directories(sq_attacked_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(sq_attacked_demo PRIVATE cxx_std_17)
