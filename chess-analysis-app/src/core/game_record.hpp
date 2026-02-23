#pragma once
#include "board.hpp"
#include <vector>
#include <map>
#include <string>

namespace Chess {

class GameRecord {
public:
    GameRecord() : currentIndex(0) {}

    void addMove(Move m, const std::string& san = "") {
        // If we differ from history (we are in past), truncate future
        if (currentIndex < moves.size()) {
            moves.resize(currentIndex);
            sanMoves.resize(currentIndex);
        }
        moves.push_back(m);
        sanMoves.push_back(san);
        currentIndex++;
    }

    void reset() {
        moves.clear();
        sanMoves.clear();
        evaluations.clear();
        currentIndex = 0;
    }

    bool hasNext() const { return currentIndex < moves.size(); }
    bool hasPrev() const { return currentIndex > 0; }

    Move next() {
        if (!hasNext()) return Move(); // Null move
        return moves[currentIndex++];
    }

    Move prev() {
        if (!hasPrev()) return Move();
        return moves[--currentIndex];
    }
    
    // Get full history for Stockfish "position startpos moves ..."
    std::vector<std::string> getMoveStrings(const Board& startBoard) const {
         // This is tricky if startpos is not standard.
         // For now assume standard startpos or handle FEN + moves.
         // Let's just return algebraic moves if possible, or coordinate notation.
         // Stockfish speaks coordinate notation (e2e4) well.
         std::vector<std::string> moveStrs;
         // We might need a temp board to generate correct notations if we want SAN, 
         // but for engine communication, coordinate notation (which we can derive from Move) is best.
         // `Move` struct has indices, `squareToString` helper exists in board.hpp logic?
         // We need to implement `moveToCoord` helper.
         return moveStrs;
    }
    
    // Store eval
    void setEval(int moveIndex, const std::string& eval) {
        evaluations[moveIndex] = eval;
    }
    
    std::string getEval(int moveIndex) const {
        auto it = evaluations.find(moveIndex);
        if (it != evaluations.end()) return it->second;
        return "";
    }

    // Public members for direct access if needed, or keeping simple
    std::vector<Move> moves;
    std::vector<std::string> sanMoves;
    int currentIndex;
    std::map<int, std::string> evaluations;
};

} // namespace Chess
