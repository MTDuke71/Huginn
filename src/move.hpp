#pragma once
#include <cstdint>
#include "board120.hpp"
#include "chess_types.hpp"

// Move encoding bit layout (25 bits total in int move):
// Bits 0-6:   from square (7 bits, 0-127, supports 120-square notation)
// Bits 7-13:  to square (7 bits, 0-127, supports 120-square notation)  
// Bits 14-17: captured piece type (4 bits, 0-15, PieceType enum)
// Bit 18:     en passant capture flag (1 bit)
// Bit 19:     pawn start (double push) flag (1 bit)
// Bits 20-23: promoted piece type (4 bits, 0-15, PieceType enum)
// Bit 24:     castle move flag (1 bit)

// Bitmasks for move encoding/decoding
constexpr int MOVE_FROM_MASK     = 0x0000007F;  // Bits 0-6
constexpr int MOVE_TO_MASK       = 0x00003F80;  // Bits 7-13
constexpr int MOVE_CAPTURED_MASK = 0x0003C000;  // Bits 14-17
constexpr int MOVE_ENPASSANT     = 0x00040000;  // Bit 18
constexpr int MOVE_PAWNSTART     = 0x00080000;  // Bit 19
constexpr int MOVE_PROMOTED_MASK = 0x00F00000;  // Bits 20-23
constexpr int MOVE_CASTLE        = 0x01000000;  // Bit 24

// Bit shift positions
constexpr int MOVE_FROM_SHIFT     = 0;
constexpr int MOVE_TO_SHIFT       = 7;
constexpr int MOVE_CAPTURED_SHIFT = 14;
constexpr int MOVE_ENPASSANT_SHIFT = 18;
constexpr int MOVE_PAWNSTART_SHIFT = 19;
constexpr int MOVE_PROMOTED_SHIFT = 20;
constexpr int MOVE_CASTLE_SHIFT   = 24;

// Forward declaration
struct S_MOVE;

// Enhanced move structure with packed encoding and score for move ordering
struct S_MOVE {
    int move;   // Packed move data (25 bits used)
    int score;  // Move score for ordering/evaluation
    
    // Default constructor
    S_MOVE() : move(0), score(0) {}
    
    // Constructor with move encoding
    S_MOVE(int from, int to, PieceType captured = PieceType::None, 
           bool en_passant = false, bool pawn_start = false, 
           PieceType promoted = PieceType::None, bool castle = false) 
           : score(0) {
        move = encode_move(from, to, captured, en_passant, pawn_start, promoted, castle);
    }
    
    // Static move encoding function - declaration only
    static int encode_move(int from, int to, PieceType captured = PieceType::None,
                          bool en_passant = false, bool pawn_start = false,
                          PieceType promoted = PieceType::None, bool castle = false);
    
    // Decoding functions using bitmasks and bit shifts
    int get_from() const {
        return move & MOVE_FROM_MASK;
    }
    
    int get_to() const {
        return (move & MOVE_TO_MASK) >> MOVE_TO_SHIFT;
    }
    
    PieceType get_captured() const {
        return PieceType((move & MOVE_CAPTURED_MASK) >> MOVE_CAPTURED_SHIFT);
    }
    
    bool is_en_passant() const {
        return (move & MOVE_ENPASSANT) != 0;
    }
    
    bool is_pawn_start() const {
        return (move & MOVE_PAWNSTART) != 0;
    }
    
    PieceType get_promoted() const {
        return PieceType((move & MOVE_PROMOTED_MASK) >> MOVE_PROMOTED_SHIFT);
    }
    
    bool is_castle() const {
        return (move & MOVE_CASTLE) != 0;
    }
    
    // Convenience functions
    bool is_capture() const {
        return get_captured() != PieceType::None || is_en_passant();
    }
    
    bool is_promotion() const {
        return get_promoted() != PieceType::None;
    }
    
    bool is_quiet() const {
        return !is_capture() && !is_promotion() && !is_castle() && !is_en_passant();
    }
    
    // Comparison operators for move ordering
    bool operator<(const S_MOVE& other) const {
        return score < other.score;
    }
    
    bool operator>(const S_MOVE& other) const {
        return score > other.score;
    }
    
    bool operator==(const S_MOVE& other) const {
        return move == other.move;
    }
    
    bool operator!=(const S_MOVE& other) const {
        return move != other.move;
    }
  
    // Allow assignment from int for test compatibility (assigns to move field)
    S_MOVE& operator=(int value) {
        move = value;
        score = 0;
        return *this;
    }
    
    // Allow comparison with int for test compatibility
    bool operator==(int value) const {
        return move == value;
    }
};

// Convenience functions for creating moves
inline S_MOVE make_move(int from, int to) {
    return S_MOVE(from, to);
}

inline S_MOVE make_capture(int from, int to, PieceType captured) {
    return S_MOVE(from, to, captured);
}

inline S_MOVE make_en_passant(int from, int to) {
    return S_MOVE(from, to, PieceType::Pawn, true);
}

inline S_MOVE make_pawn_start(int from, int to) {
    return S_MOVE(from, to, PieceType::None, false, true);
}

inline S_MOVE make_promotion(int from, int to, PieceType promoted, PieceType captured = PieceType::None) {
    return S_MOVE(from, to, captured, false, false, promoted);
}

inline S_MOVE make_castle(int from, int to) {
    return S_MOVE(from, to, PieceType::None, false, false, PieceType::None, true);
}

