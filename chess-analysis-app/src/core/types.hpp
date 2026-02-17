#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

namespace Chess {

// Typedefs for clarity
using Bitboard = uint64_t;
using Square = int;

// Constants
constexpr int SQUARE_NONE = -1;
constexpr int NUM_SQUARES = 64;

enum Side {
    White = 0,
    Black = 1,
    Side_NB = 2
};

enum PieceType {
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5,
    NO_PIECE_TYPE = 6
};

enum Piece {
    W_PAWN = 0, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    NO_PIECE = 16
};

struct Move {
    Square from = SQUARE_NONE;
    Square dest = SQUARE_NONE;
    PieceType promotion = NO_PIECE_TYPE;
    
    // Flags could be added here (capture, castle, etc.) but let's keep it simple for now
    bool isNull() const { return from == SQUARE_NONE; }
    
    // For string conversion e.g. "e2e4"
    std::string toString() const;
    static Move fromString(const std::string& s);
};

// Helper functions
std::string squareToString(Square s);
Square stringToSquare(std::string s);

inline Side colorOf(Piece p) {
    if (p == NO_PIECE) return Side_NB;
    return (p < 8) ? White : Black;
}

inline PieceType typeOf(Piece p) {
    if (p == NO_PIECE) return NO_PIECE_TYPE;
    return static_cast<PieceType>(p % 8);
}

inline Piece makePiece(Side c, PieceType pt) {
    return static_cast<Piece>((c == White ? 0 : 8) + pt);
}


} // namespace Chess
