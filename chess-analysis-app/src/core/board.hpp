#pragma once
#include "types.hpp"
#include <vector>
#include <string>
#include <array>
#include <map>

namespace Chess {

class Board {
public:
    Board();
    Board(const std::string& fen);

    // Getters
    std::string getFen() const;
    Piece getPiece(Square s) const;
    Side getTurn() const;
    bool isCheck() const;
    bool isCheckmate() const;
    bool isStalemate() const;
    bool isInsufficientMaterial() const;
    bool isDraw() const;
    
    // Core Logic
    bool makeMove(Move move); 
    void undoMove(); 
    std::vector<Move> getLegalMoves() const;
    void reset();
    void loadFen(const std::string& fen);
    void loadPgn(const std::string& pgn);
    std::string moveToSan(const Move& m);

private:
    std::array<Piece, 64> board;
    Side turn;
    int halfMoveClock;
    int fullMoveNumber;
    
    // Castling rights: 4 bits (White K, White Q, Black K, Black Q)
    uint8_t castlingRights;
    
    Square enPassantSquare;
    
    // History for undo (store relevant state)
    struct State {
        Move move;
        uint8_t castlingRights;
        Square enPassantSquare;
        int halfMoveClock;
        Piece capturedPiece;
    };
    std::vector<State> history;
    // Helpers
    void clear();
    void setFen(const std::string& fen);
    bool isSquareAttacked(Square s, Side attacker) const;
    std::vector<Move> generatePseudoLegalMoves() const;
};

} // namespace Chess
