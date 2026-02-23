#include "types.hpp"
#include <array>

namespace Chess {

std::string squareToString(Square s) {
    if (s < 0 || s >= 64) return "-";
    static const std::string sqStrings[64] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
    };
    return sqStrings[s];
}

Square stringToSquare(std::string s) {
    if (s.length() != 2) return SQUARE_NONE;
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return SQUARE_NONE;
    return rank * 8 + file;
}

std::string Move::toString() const {
    if (isNull()) return "0000";
    std::string s = squareToString(from) + squareToString(dest);
    if (promotion != NO_PIECE_TYPE) {
        char p = ' ';
        switch(promotion) {
            case QUEEN: p = 'q'; break;
            case ROOK: p = 'r'; break;
            case BISHOP: p = 'b'; break;
            case KNIGHT: p = 'n'; break;
            default: break;
        }
        if (p != ' ') s += p;
    }
    return s;
}

Move Move::fromString(const std::string& s) {
    Move m;
    if (s.length() < 4) return m;
    m.from = stringToSquare(s.substr(0, 2));
    m.dest = stringToSquare(s.substr(2, 2));
    if (s.length() > 4) {
        switch(s[4]) {
            case 'q': m.promotion = QUEEN; break;
            case 'r': m.promotion = ROOK; break;
            case 'b': m.promotion = BISHOP; break;
            case 'n': m.promotion = KNIGHT; break;
            default: m.promotion = NO_PIECE_TYPE; break;
        }
    }
    return m;
}

} // namespace Chess
