/**
 * @file ultra_move.hpp
 * @brief Ultra-optimized move representation - pure 64-bit coordinates
 * 
 * Revolutionary move system eliminating ALL coordinate conversions.
 * Target: Zero conversion overhead, maximum cache efficiency.
 * 
 * @author MTDuke71
 * @version Ultra 1.0
 */
#pragma once
#include <cstdint>
#include <array>

namespace UltraEngine {

// ============================================================================
// ULTRA-COMPACT MOVE ENCODING
// Pure 64-bit coordinates (0-63) - NO 120-square conversions anywhere!
// ============================================================================

/**
 * @brief Ultra-compact 22-bit move encoding
 * 
 * Bits 0-5:   From square (6 bits: 0-63) 
 * Bits 6-11:  To square (6 bits: 0-63)
 * Bits 12-14: Captured piece (3 bits: 0=None, 1=Pawn, 2=Knight, 3=Bishop, 4=Rook, 5=Queen, 6=King)
 * Bits 15-17: Promotion piece (3 bits: 0=None, 2=Knight, 3=Bishop, 4=Rook, 5=Queen)
 * Bit 18:     En passant flag
 * Bit 19:     Double pawn push flag  
 * Bit 20:     Castling flag
 * Bit 21:     Reserved for future use
 */
class UltraMove {
private:
    uint32_t data_;

public:
    // ========================================================================
    // CONSTRUCTORS - All constexpr for compile-time optimization
    // ========================================================================
    
    constexpr UltraMove() : data_(0) {}
    
    constexpr UltraMove(int from64, int to64, int captured = 0, int promoted = 0, 
                       bool ep = false, bool pawn_start = false, bool castle = false)
        : data_((from64 & 0x3F) | 
                ((to64 & 0x3F) << 6) |
                ((captured & 0x7) << 12) |
                ((promoted & 0x7) << 15) |
                (ep ? (1 << 18) : 0) |
                (pawn_start ? (1 << 19) : 0) |
                (castle ? (1 << 20) : 0)) {}

    // ========================================================================
    // ULTRA-FAST ACCESSORS - All constexpr and inline
    // ========================================================================
    
    [[nodiscard]] constexpr int from() const noexcept { return data_ & 0x3F; }
    [[nodiscard]] constexpr int to() const noexcept { return (data_ >> 6) & 0x3F; }
    [[nodiscard]] constexpr int captured() const noexcept { return (data_ >> 12) & 0x7; }
    [[nodiscard]] constexpr int promoted() const noexcept { return (data_ >> 15) & 0x7; }
    [[nodiscard]] constexpr bool is_ep() const noexcept { return data_ & (1 << 18); }
    [[nodiscard]] constexpr bool is_pawn_start() const noexcept { return data_ & (1 << 19); }
    [[nodiscard]] constexpr bool is_castle() const noexcept { return data_ & (1 << 20); }
    
    // Derived properties for fast move classification
    [[nodiscard]] constexpr bool is_capture() const noexcept { return captured() != 0; }
    [[nodiscard]] constexpr bool is_promotion() const noexcept { return promoted() != 0; }
    [[nodiscard]] constexpr bool is_quiet() const noexcept { return !is_capture() && !is_promotion(); }
    [[nodiscard]] constexpr bool is_special() const noexcept { return is_ep() || is_pawn_start() || is_castle(); }
    
    // Raw data access for high-performance scenarios
    [[nodiscard]] constexpr uint32_t raw() const noexcept { return data_; }
    
    // ========================================================================
    // COMPARISON OPERATORS
    // ========================================================================
    
    constexpr bool operator==(const UltraMove& other) const noexcept { return data_ == other.data_; }
    constexpr bool operator!=(const UltraMove& other) const noexcept { return data_ != other.data_; }
    constexpr bool operator<(const UltraMove& other) const noexcept { return data_ < other.data_; }
    
    // ========================================================================
    // FACTORY METHODS - For clean move creation
    // ========================================================================
    
    static constexpr UltraMove quiet(int from64, int to64) {
        return UltraMove(from64, to64);
    }
    
    static constexpr UltraMove capture(int from64, int to64, int captured_piece) {
        return UltraMove(from64, to64, captured_piece);
    }
    
    static constexpr UltraMove promotion(int from64, int to64, int promoted_piece, int captured = 0) {
        return UltraMove(from64, to64, captured, promoted_piece);
    }
    
    static constexpr UltraMove en_passant(int from64, int to64) {
        return UltraMove(from64, to64, 1, 0, true); // Captures pawn via en passant
    }
    
    static constexpr UltraMove pawn_double(int from64, int to64) {
        return UltraMove(from64, to64, 0, 0, false, true);
    }
    
    static constexpr UltraMove castle(int from64, int to64) {
        return UltraMove(from64, to64, 0, 0, false, false, true);
    }
};

// ============================================================================
// ULTRA-FAST MOVE LIST
// Stack-allocated, cache-friendly move storage
// ============================================================================

class UltraMoveList {
private:
    std::array<UltraMove, 256> moves_;
    int count_;

public:
    constexpr UltraMoveList() : count_(0) {}
    
    // ========================================================================
    // MOVE ADDITION - Ultra-fast, all inline
    // ========================================================================
    
    constexpr void add(const UltraMove& move) noexcept {
        moves_[count_++] = move;
    }
    
    constexpr void add_quiet(int from, int to) noexcept {
        moves_[count_++] = UltraMove::quiet(from, to);
    }
    
    constexpr void add_capture(int from, int to, int captured) noexcept {
        moves_[count_++] = UltraMove::capture(from, to, captured);
    }
    
    constexpr void add_promotion(int from, int to, int promoted, int captured = 0) noexcept {
        moves_[count_++] = UltraMove::promotion(from, to, promoted, captured);
    }
    
    constexpr void add_en_passant(int from, int to) noexcept {
        moves_[count_++] = UltraMove::en_passant(from, to);
    }
    
    constexpr void add_pawn_double(int from, int to) noexcept {
        moves_[count_++] = UltraMove::pawn_double(from, to);
    }
    
    constexpr void add_castle(int from, int to) noexcept {
        moves_[count_++] = UltraMove::castle(from, to);
    }
    
    // ========================================================================
    // ACCESS AND ITERATION
    // ========================================================================
    
    constexpr void clear() noexcept { count_ = 0; }
    [[nodiscard]] constexpr int size() const noexcept { return count_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return count_ == 0; }
    
    constexpr UltraMove& operator[](int i) noexcept { return moves_[i]; }
    constexpr const UltraMove& operator[](int i) const noexcept { return moves_[i]; }
    
    constexpr UltraMove* begin() noexcept { return moves_.data(); }
    constexpr UltraMove* end() noexcept { return moves_.data() + count_; }
    constexpr const UltraMove* begin() const noexcept { return moves_.data(); }
    constexpr const UltraMove* end() const noexcept { return moves_.data() + count_; }
};

// ============================================================================
// PIECE TYPE CONSTANTS - Ultra-clean, zero-based indexing
// ============================================================================

namespace PieceType {
    constexpr int None = 0;
    constexpr int Pawn = 1;
    constexpr int Knight = 2;
    constexpr int Bishop = 3;
    constexpr int Rook = 4;
    constexpr int Queen = 5;
    constexpr int King = 6;
}

namespace Color {
    constexpr int White = 0;
    constexpr int Black = 1;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Convert between different square representations (if needed for compatibility)
[[nodiscard]] constexpr int square_64_to_file(int sq64) noexcept { return sq64 & 7; }
[[nodiscard]] constexpr int square_64_to_rank(int sq64) noexcept { return sq64 >> 3; }
[[nodiscard]] constexpr int file_rank_to_square_64(int file, int rank) noexcept { return rank * 8 + file; }

// Square name conversion for debugging
const char* square_64_to_string(int sq64) noexcept;

} // namespace UltraEngine