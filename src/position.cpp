// position.cpp - Implementation file for Position class methods
#include "position.hpp"
#include "zobrist.hpp"

// Update Zobrist key incrementally for a move using XOR (much faster than recomputation)
void Position::update_zobrist_for_move(const S_MOVE& m, Piece moving, Piece captured) {
    Color moving_color = color_of(moving);
    PieceType moving_type = type_of(moving);
    
    // XOR out the moving piece from its original square
    int piece_index = int(moving_type) + (moving_color == Color::Black ? 6 : 0);
    zobrist_key ^= Zobrist::Piece[piece_index][m.get_from()];
    
    // XOR out captured piece (if any) from destination square
    if (!is_none(captured)) {
        Color captured_color = color_of(captured);
        PieceType captured_type = type_of(captured);
        int captured_index = int(captured_type) + (captured_color == Color::Black ? 6 : 0);
        zobrist_key ^= Zobrist::Piece[captured_index][m.get_to()];
    }
    
    // XOR in the piece at its new square
    if (m.is_promotion()) {
        // For promotion, the piece type changes
        int promoted_index = int(m.get_promoted()) + (moving_color == Color::Black ? 6 : 0);
        zobrist_key ^= Zobrist::Piece[promoted_index][m.get_to()];
    } else {
        // Regular move - same piece type
        zobrist_key ^= Zobrist::Piece[piece_index][m.get_to()];
    }
    
    // XOR the side to move (since it always flips)
    zobrist_key ^= Zobrist::Side;
    
    // TODO: Add castling rights and en passant XOR updates when those are implemented
}

// Compute and set the Zobrist key from current position
void Position::update_zobrist_key() {
    zobrist_key = Zobrist::compute(*this);
}
