cmake_minimum_required(VERSION 3.24)

# Create a separate debug demo executable
add_executable(debug_demo
  src/debug_demo.cpp
)
target_link_libraries(debug_demo PRIVATE huginn_lib)

# Create a separate castling validation demo executable
add_executable(castling_demo
  src/castling_demo.cpp
)
target_link_libraries(castling_demo PRIVATE huginn_lib)

# Create a separate FEN generation demo executable
add_executable(fen_demo
  src/fen_demo.cpp
)
target_link_libraries(fen_demo PRIVATE huginn_lib)

# Create a separate SqAttacked demo executable
add_executable(sq_attacked_demo
  src/sq_attacked_demo.cpp
)
target_link_libraries(sq_attacked_demo PRIVATE huginn_lib)