#include "board.hpp"
#include <vector>
#include <cmath>

namespace Chess {

// Direction offsets for sliding pieces
// Rooks: N, E, S, W
// Bishops: NE, SE, SW, NW
const int RookDir[4] = {8, 1, -8, -1};
const int BishopDir[4] = {9, -7, -9, 7};

// Helper to check board boundaries properly
bool isSquareValid(int sq) {
    return sq >= 0 && sq < 64;
}

// Check if moving 'step' from 'current' crosses board edge
// For 1D representation, we need to be careful about file wrapping.
// We can check if file index changes by more than 1 or wraps.
bool isWrap(int from, int destSq) {
    int f1 = from % 8;
    int f2 = destSq % 8;
    // If we moved horizontally/diagonally, file diff should be small
    // Actually, simpler approach:
    // Precompute distances to edge for every square/direction? 
    // Or just robust coordinate math.
    int r1 = from / 8;
    int r2 = destSq / 8;
    int dr = r2 - r1;
    int df = f2 - f1;
    // If it's a valid sliding move, abs(dr) == abs(df) (diag) or one is 0 (straight)
    if (abs(dr) == abs(df)) return false; // Diag OK
    if (dr == 0 || df == 0) return false; // Straight OK
    return true; // Invalid jump (wrap)
}

// Improved direction logic
// Directions: N=8, S=-8, E=1, W=-1, NE=9, NW=7, SE=-7, SW=-9
// We need to check if we cross file boundaries for E/W components.
// E (+1): file cannot be 7
// W (-1): file cannot be 0
// NE (+9): file != 7, rank != 7
// NW (+7): file != 0, rank != 7
// etc.

void addSlidingMoves(const Board& b, int from, const int* dirs, int numDirs, std::vector<Move>& moves) {
    Piece p = b.getPiece(from);
    Side us = colorOf(p);
    
    for (int d = 0; d < numDirs; d++) {
        int dir = dirs[d];
        int targetSq = from;
        
        while (true) {
            // Check boundary before adding
            int file = targetSq % 8;
            int rank = targetSq / 8;
            
            // Check if adding dir would wrap
            // E/W/NE/SE/NW/SW components
            if ( (dir == 1 || dir == 9 || dir == -7) && file == 7 ) break; // East-ward blocked
            if ( (dir == -1 || dir == -9 || dir == 7) && file == 0 ) break; // West-ward blocked
            if ( (dir == 8 || dir == 9 || dir == 7) && rank == 7 ) break; // North-ward blocked
            if ( (dir == -8 || dir == -9 || dir == -7) && rank == 0 ) break; // South-ward blocked

            targetSq += dir;
            if (!isSquareValid(targetSq)) break;

            Piece target = b.getPiece(targetSq);
            if (target == NO_PIECE) {
                moves.push_back({from, targetSq, NO_PIECE_TYPE});
            } else {
                if (colorOf(target) != us) {
                    moves.push_back({from, targetSq, NO_PIECE_TYPE}); // Capture
                }
                break; // Blocked by any piece
            }
        }
    }
}

// Re-implementing generatePseudoLegalMoves to include sliding pieces
std::vector<Move> Board::generatePseudoLegalMoves() const {
    std::vector<Move> moves;
    moves.reserve(50); 
    
    for (int from = 0; from < 64; from++) {
        Piece p = board[from];
        if (p == NO_PIECE || colorOf(p) != turn) continue;
        
        PieceType pt = typeOf(p);
        int r = from / 8;
        int c = from % 8;
        int direction = (turn == White) ? 1 : -1;

        if (pt == PAWN) {
            // Pawn moves... (simplified)
            int forward = from + 8 * direction;
            if (isSquareValid(forward) && board[forward] == NO_PIECE) {
                // Promotion?
                 if ((turn == White && r == 6) || (turn == Black && r == 1)) {
                    moves.push_back({from, forward, QUEEN}); // Add all promos
                    moves.push_back({from, forward, KNIGHT}); 
                    moves.push_back({from, forward, ROOK}); 
                    moves.push_back({from, forward, BISHOP}); 
                 } else {
                    moves.push_back({from, forward, NO_PIECE_TYPE});
                    // Double push
                    if ((turn == White && r == 1) || (turn == Black && r == 6)) {
                        int forward2 = forward + 8 * direction;
                        if (isSquareValid(forward2) && board[forward2] == NO_PIECE) {
                            moves.push_back({from, forward2, NO_PIECE_TYPE});
                        }
                    }
                 }
            }
            // Captures
            for (int dx : {-1, 1}) {
                if (c + dx >= 0 && c + dx <= 7) {
                    int capSq = from + 8 * direction + dx;
                    if (isSquareValid(capSq)) {
                        Piece target = board[capSq];
                        if (target != NO_PIECE && colorOf(target) != turn) {
                             if ((turn == White && r == 6) || (turn == Black && r == 1)) {
                                moves.push_back({from, capSq, QUEEN}); // ...
                             } else {
                                moves.push_back({from, capSq, NO_PIECE_TYPE});
                             }
                        }
                        // En Passant
                        if (capSq == enPassantSquare) {
                            moves.push_back({from, capSq, NO_PIECE_TYPE});
                        }
                    }
                }
            }
        }
        else if (pt == KNIGHT) {
            // Knight logic
            const int KnightOffsets[] = {-17, -15, -10, -6, 6, 10, 15, 17};
            for (int offset : KnightOffsets) {
                int targetSq = from + offset;
                if (!isSquareValid(targetSq)) continue;
                // Check wrap
                int dr = abs((targetSq/8) - r);
                int dc = abs((targetSq%8) - c);
                if (dr + dc == 3) {
                     Piece target = board[targetSq];
                     if (target == NO_PIECE || colorOf(target) != turn) {
                         moves.push_back({from, targetSq, NO_PIECE_TYPE});
                     }
                }
            }
        }
        else if (pt == KING) {
             const int KingOffsets[] = {-9, -8, -7, -1, 1, 7, 8, 9};
             for (int offset : KingOffsets) {
                int targetSq = from + offset;
                if (!isSquareValid(targetSq)) continue;
                int dr = abs((targetSq/8) - r);
                int dc = abs((targetSq%8) - c);
                if (dr <= 1 && dc <= 1) {
                     Piece target = board[targetSq];
                     if (target == NO_PIECE || colorOf(target) != turn) {
                         moves.push_back({from, targetSq, NO_PIECE_TYPE});
                     }
                }
             }
        }
        else if (pt == BISHOP || pt == QUEEN) {
            addSlidingMoves(*this, from, BishopDir, 4, moves);
        }
        
        if (pt == ROOK || pt == QUEEN) {
            addSlidingMoves(*this, from, RookDir, 4, moves);
        }
    }
    return moves;
}

std::vector<Move> Board::getLegalMoves() const {
    std::vector<Move> pseudo = generatePseudoLegalMoves();
    std::vector<Move> legal;
    
    for (const auto& m : pseudo) {
        Board copy = *this;
        if (copy.makeMove(m)) {
            legal.push_back(m);
        }
    }
    
    return legal;
}

} // namespace Chess
