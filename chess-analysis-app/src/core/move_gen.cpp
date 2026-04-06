#include "board.hpp"
#include <vector>
#include <cmath>

namespace Chess {

// Re-implementing generatePseudoLegalMoves using enumeration
std::vector<Move> Board::generatePseudoLegalMoves() const {
    std::vector<Move> moves;
    moves.reserve(50); 
    
    enumeratePseudoLegalMoves([&](const Move& m) {
        moves.push_back(m);
        return false; // Continue enumeration
    });

    return moves;
}

std::vector<Move> Board::getLegalMoves() const {
    std::vector<Move> legal;
    legal.reserve(40);
    
    enumeratePseudoLegalMoves([&](const Move& m) {
        Board copy = *this;
        if (copy.makeMove(m)) {
            legal.push_back(m);
        }
        return false; // Continue enumeration
    });
    
    return legal;
}

} // namespace Chess
