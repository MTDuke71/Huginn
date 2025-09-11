/**
 * @file move.hpp
 * @brief Chess move representation with compact bit-packed encoding
 * 
 * This module implements the core move representation for the Huginn chess engine.
 * Moves are encoded as 25-bit integers with separate score fields for move ordering.
 * The encoding supports all chess move types including captures, promotions, 
 * castling, and en passant.
 */

#pragma once
#include <cstdint>
#include "board120.hpp"
#include "chess_types.hpp"

/**
 * @section Move Encoding Format
 * 
 * Move encoding uses a 25-bit layout within a 32-bit integer:
 * 
 * Bits  0-6:   Source square (7 bits, supports 0-127 for 120-square board)
 * Bits  7-13:  Destination square (7 bits, supports 0-127 for 120-square board)  
 * Bits 14-17:  Captured piece type (4 bits, PieceType enum values 0-15)
 * Bit  18:     En passant capture flag (1 bit)
 * Bit  19:     Pawn double-push flag (1 bit)
 * Bits 20-23:  Promoted piece type (4 bits, PieceType enum values 0-15)
 * Bit  24:     Castle move flag (1 bit)
 * Bits 25-31:  Unused (reserved for future extensions)
 */

// Bitmasks for move encoding/decoding - extract specific bit ranges
constexpr int MOVE_FROM_MASK     = 0x0000007F;  // Bits 0-6:   source square
constexpr int MOVE_TO_MASK       = 0x00003F80;  // Bits 7-13:  destination square
constexpr int MOVE_CAPTURED_MASK = 0x0003C000;  // Bits 14-17: captured piece type
constexpr int MOVE_ENPASSANT     = 0x00040000;  // Bit 18:     en passant flag
constexpr int MOVE_PAWNSTART     = 0x00080000;  // Bit 19:     pawn double-push flag
constexpr int MOVE_PROMOTED_MASK = 0x00F00000;  // Bits 20-23: promoted piece type
constexpr int MOVE_CASTLE        = 0x01000000;  // Bit 24:     castle move flag

// Bit shift positions for encoding operations
constexpr int MOVE_FROM_SHIFT       = 0;   // No shift needed for bits 0-6
constexpr int MOVE_TO_SHIFT         = 7;   // Shift destination to bits 7-13
constexpr int MOVE_CAPTURED_SHIFT   = 14;  // Shift captured type to bits 14-17
constexpr int MOVE_ENPASSANT_SHIFT  = 18;  // Shift en passant flag to bit 18
constexpr int MOVE_PAWNSTART_SHIFT  = 19;  // Shift pawn start flag to bit 19
constexpr int MOVE_PROMOTED_SHIFT   = 20;  // Shift promoted type to bits 20-23
constexpr int MOVE_CASTLE_SHIFT     = 24;  // Shift castle flag to bit 24

// Forward declaration for S_MOVE structure
struct S_MOVE;

/**
 * @brief Enhanced chess move structure with bit-packed encoding and move ordering score
 * 
 * S_MOVE represents a complete chess move with:
 * - Compact 25-bit encoding of move data (source, destination, special flags)
 * - Separate 32-bit score field for move ordering in search algorithms
 * - Complete decoding interface for extracting move components
 * - Convenience methods for move classification and comparison
 * 
 * The structure is optimized for:
 * - Memory efficiency (8 bytes total vs alternatives using multiple fields)
 * - Fast encoding/decoding using bitwise operations
 * - Move ordering in search algorithms via score field
 * - Compatibility with existing integer-based move representations
 */
struct S_MOVE {
    int move;   ///< Bit-packed move data (25 bits used, 7 bits reserved)
    int score;  ///< Move ordering score for search algorithms (higher = better)
    
    /**
     * @brief Default constructor - creates an invalid/null move
     * @post move == 0 && score == 0
     */
    constexpr S_MOVE() : move(0), score(0) {}
    
    /**
     * @brief Constructor with full move specification
     * @param from        Source square index (0-127, typically 0-119 for 120-square board)
     * @param to          Destination square index (0-127, typically 0-119 for 120-square board)
     * @param captured    Type of piece captured (PieceType::None if no capture)
     * @param en_passant  True if this is an en passant capture
     * @param pawn_start  True if this is a pawn's initial double-step move
     * @param promoted    Type of piece to promote to (PieceType::None if not promotion)
     * @param castle      True if this is a castling move
     * @post score == 0 (must be set separately for move ordering)
     */
    constexpr S_MOVE(int from, int to, PieceType captured = PieceType::None, 
           bool en_passant = false, bool pawn_start = false, 
           PieceType promoted = PieceType::None, bool castle = false) 
           : score(0) {
        // Inline encoding for maximum performance in hot paths
        move = (from & 0x7F) |
               ((to & 0x7F) << MOVE_TO_SHIFT) |
               ((int(captured) & 0xF) << MOVE_CAPTURED_SHIFT) |
               (en_passant ? MOVE_ENPASSANT : 0) |
               (pawn_start ? MOVE_PAWNSTART : 0) |
               ((int(promoted) & 0xF) << MOVE_PROMOTED_SHIFT) |
               (castle ? MOVE_CASTLE : 0);
    }
    
    /**
     * @brief Static move encoding function
     * 
     * Combines all move components into a single 25-bit integer using bitwise operations.
     * This function is implemented in move.cpp to keep the header interface clean.
     * 
     * @param from        Source square index (0-127)
     * @param to          Destination square index (0-127)
     * @param captured    Type of piece captured (default: PieceType::None)
     * @param en_passant  En passant capture flag (default: false)
     * @param pawn_start  Pawn double-push flag (default: false)
     * @param promoted    Type of piece to promote to (default: PieceType::None)
     * @param castle      Castle move flag (default: false)
     * @return            Encoded 25-bit move as integer
     */
    static int encode_move(int from, int to, PieceType captured = PieceType::None,
                          bool en_passant = false, bool pawn_start = false,
                          PieceType promoted = PieceType::None, bool castle = false);
    
    // ========================================================================
    // MOVE DECODING INTERFACE
    // Fast bitwise extraction of move components using precomputed masks
    // ========================================================================
    
    /**
     * @brief Extract source square from encoded move
     * @return Source square index (0-127)
     * @complexity O(1) - simple bitwise AND operation
     */
    [[nodiscard]] constexpr int get_from() const noexcept {
        return move & MOVE_FROM_MASK;
    }
    
    /**
     * @brief Extract destination square from encoded move
     * @return Destination square index (0-127)
     * @complexity O(1) - bitwise AND + right shift
     */
    [[nodiscard]] constexpr int get_to() const noexcept {
        return (move & MOVE_TO_MASK) >> MOVE_TO_SHIFT;
    }
    
    /**
     * @brief Extract captured piece type from encoded move
     * @return PieceType of captured piece (PieceType::None if no capture)
     * @complexity O(1) - bitwise AND + right shift + enum cast
     */
    [[nodiscard]] constexpr PieceType get_captured() const noexcept {
        return PieceType((move & MOVE_CAPTURED_MASK) >> MOVE_CAPTURED_SHIFT);
    }
    
    /**
     * @brief Check if this is an en passant capture
     * @return True if en passant capture, false otherwise
     * @complexity O(1) - bitwise AND + comparison
     */
    [[nodiscard]] constexpr bool is_en_passant() const noexcept {
        return (move & MOVE_ENPASSANT) != 0;
    }
    
    /**
     * @brief Check if this is a pawn's initial double-step move
     * @return True if pawn double-push, false otherwise
     * @complexity O(1) - bitwise AND + comparison
     */
    [[nodiscard]] constexpr bool is_pawn_start() const noexcept {
        return (move & MOVE_PAWNSTART) != 0;
    }
    
    /**
     * @brief Extract promoted piece type from encoded move
     * @return PieceType of promotion target (PieceType::None if not promotion)
     * @complexity O(1) - bitwise AND + right shift + enum cast
     */
    [[nodiscard]] constexpr PieceType get_promoted() const noexcept {
        return PieceType((move & MOVE_PROMOTED_MASK) >> MOVE_PROMOTED_SHIFT);
    }
    
    /**
     * @brief Check if this is a castling move
     * @return True if castling move, false otherwise
     * @complexity O(1) - bitwise AND + comparison
     */
    [[nodiscard]] constexpr bool is_castle() const noexcept {
        return (move & MOVE_CASTLE) != 0;
    }
    
    // ========================================================================
    // MOVE CLASSIFICATION INTERFACE
    // High-level move type detection for search and evaluation algorithms
    // ========================================================================
    
    /**
     * @brief Check if this move captures any piece
     * @return True if move captures a piece (normal capture or en passant)
     * @note En passant is considered a capture even though get_captured() may return None
     * @optimization Uses bitwise operations to avoid multiple function calls
     */
    [[nodiscard]] constexpr bool is_capture() const noexcept {
        // Fast check: either captured bits are set OR en passant bit is set
        return (move & (MOVE_CAPTURED_MASK | MOVE_ENPASSANT)) != 0;
    }
    
    /**
     * @brief Check if this move promotes a pawn
     * @return True if pawn promotion move, false otherwise
     * @optimization Direct bit check is faster than get_promoted() != None
     */
    [[nodiscard]] constexpr bool is_promotion() const noexcept {
        return (move & MOVE_PROMOTED_MASK) != 0;
    }
    
    /**
     * @brief Check if this is a quiet (non-tactical) move
     * @return True if move has no special effects (no capture, promotion, castle, or en passant)
     * @note Quiet moves are typically ordered last in search algorithms
     * @optimization Single bitwise operation checks all tactical flags at once
     */
    [[nodiscard]] constexpr bool is_quiet() const noexcept {
        // All tactical move bits: capture, promotion, castle, en passant, pawn start
        constexpr int TACTICAL_MASK = MOVE_CAPTURED_MASK | MOVE_PROMOTED_MASK | 
                                    MOVE_CASTLE | MOVE_ENPASSANT | MOVE_PAWNSTART;
        return (move & TACTICAL_MASK) == 0;
    }
    
    // ========================================================================
    // COMPARISON OPERATORS
    // Support for move ordering in search algorithms and container operations
    // ========================================================================
    
    /**
     * @brief Less-than comparison based on move score
     * @param other Move to compare against
     * @return True if this move's score is less than other's score
     * @note Used for sorting moves in ascending order (worst first)
     */
    [[nodiscard]] constexpr bool operator<(const S_MOVE& other) const noexcept {
        return score < other.score;
    }
    
    /**
     * @brief Greater-than comparison based on move score
     * @param other Move to compare against
     * @return True if this move's score is greater than other's score
     * @note Used for sorting moves in descending order (best first)
     */
    [[nodiscard]] constexpr bool operator>(const S_MOVE& other) const noexcept {
        return score > other.score;
    }
    
    /**
     * @brief Equality comparison based on move encoding
     * @param other Move to compare against
     * @return True if both moves have identical encoding (ignores score)
     * @note Score differences don't affect move equality
     */
    [[nodiscard]] constexpr bool operator==(const S_MOVE& other) const noexcept {
        return move == other.move;
    }
    
    /**
     * @brief Inequality comparison based on move encoding
     * @param other Move to compare against
     * @return True if moves have different encoding (ignores score)
     */
    [[nodiscard]] constexpr bool operator!=(const S_MOVE& other) const noexcept {
        return move != other.move;
    }
  
    // ========================================================================
    // LEGACY COMPATIBILITY INTERFACE
    // Support for test code and legacy systems using integer move representation
    // ========================================================================
    
    /**
     * @brief Assignment from integer (legacy compatibility)
     * @param value Integer move encoding to assign
     * @return Reference to this move object
     * @post score is reset to 0, move field contains the assigned value
     * @note Used for compatibility with existing test code
     */
    S_MOVE& operator=(int value) {
        move = value;
        score = 0;
        return *this;
    }
    
    /**
     * @brief Equality comparison with integer (legacy compatibility)
     * @param value Integer move encoding to compare against
     * @return True if this move's encoding equals the integer value
     * @note Ignores score field, only compares move encoding
     */
    bool operator==(int value) const {
        return move == value;
    }
};

// ============================================================================
// MOVE FACTORY FUNCTIONS
// Convenient constructors for common move types with descriptive names
// ============================================================================

/**
 * @brief Create a simple quiet move (no capture, promotion, or special effects)
 * @param from Source square index
 * @param to   Destination square index
 * @return S_MOVE representing the quiet move
 * @note Most common move type - piece simply moves from one square to another
 * @optimization Directly constructs move encoding without function call overhead
 */
[[nodiscard]] constexpr inline S_MOVE make_move(int from, int to) noexcept {
    S_MOVE move;
    move.move = (from & 0x7F) | ((to & 0x7F) << MOVE_TO_SHIFT);
    move.score = 0;
    return move;
}

/**
 * @brief Create a capture move
 * @param from     Source square index
 * @param to       Destination square index  
 * @param captured Type of piece being captured
 * @return S_MOVE representing the capture
 * @note Does not handle en passant captures (use make_en_passant instead)
 * @optimization Directly constructs move encoding for better performance
 */
[[nodiscard]] constexpr inline S_MOVE make_capture(int from, int to, PieceType captured) noexcept {
    S_MOVE move;
    move.move = (from & 0x7F) | 
                ((to & 0x7F) << MOVE_TO_SHIFT) |
                ((int(captured) & 0xF) << MOVE_CAPTURED_SHIFT);
    move.score = 0;
    return move;
}

/**
 * @brief Create an en passant capture move
 * @param from Source square index (attacking pawn position)
 * @param to   Destination square index (square behind captured pawn)
 * @return S_MOVE representing the en passant capture
 * @note The captured pawn is always a pawn, and it's not on the destination square
 */
[[nodiscard]] constexpr inline S_MOVE make_en_passant(int from, int to) noexcept {
    S_MOVE move;
    move.move = (from & 0x7F) | 
                ((to & 0x7F) << MOVE_TO_SHIFT) |
                ((int(PieceType::Pawn) & 0xF) << MOVE_CAPTURED_SHIFT) |
                MOVE_ENPASSANT;
    move.score = 0;
    return move;
}

/**
 * @brief Create a pawn double-push move (initial two-square pawn advance)
 * @param from Source square index (pawn's starting rank)
 * @param to   Destination square index (two squares forward)
 * @return S_MOVE representing the pawn double-push
 * @note Sets the pawn_start flag for en passant detection on next move
 */
[[nodiscard]] constexpr inline S_MOVE make_pawn_start(int from, int to) noexcept {
    S_MOVE move;
    move.move = (from & 0x7F) | 
                ((to & 0x7F) << MOVE_TO_SHIFT) |
                MOVE_PAWNSTART;
    move.score = 0;
    return move;
}

/**
 * @brief Create a pawn promotion move
 * @param from     Source square index (pawn on 7th rank)
 * @param to       Destination square index (8th rank)
 * @param promoted Type of piece to promote to (Queen, Rook, Bishop, or Knight)
 * @param captured Type of piece captured during promotion (optional)
 * @return S_MOVE representing the promotion
 * @note Can combine promotion with capture (promotion captures)
 */
[[nodiscard]] constexpr inline S_MOVE make_promotion(int from, int to, PieceType promoted, PieceType captured = PieceType::None) noexcept {
    S_MOVE move;
    move.move = (from & 0x7F) | 
                ((to & 0x7F) << MOVE_TO_SHIFT) |
                ((int(captured) & 0xF) << MOVE_CAPTURED_SHIFT) |
                ((int(promoted) & 0xF) << MOVE_PROMOTED_SHIFT);
    move.score = 0;
    return move;
}

/**
 * @brief Create a castling move
 * @param from Source square index (king's position)
 * @param to   Destination square index (king's target square)
 * @return S_MOVE representing the castling move
 * @note Only represents king movement; rook movement is handled separately
 * @note Both kingside and queenside castling use this function
 */
[[nodiscard]] constexpr inline S_MOVE make_castle(int from, int to) noexcept {
    S_MOVE move;
    move.move = (from & 0x7F) | 
                ((to & 0x7F) << MOVE_TO_SHIFT) |
                MOVE_CASTLE;
    move.score = 0;
    return move;
}

