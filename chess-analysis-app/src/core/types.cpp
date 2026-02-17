#include "types.hpp"
#include <array>

namespace Chess {

std::string squareToString(Square s) {
    if (s < 0 || s >= 64) return "-";
    int rank = s / 8;
    int file = s % 8;
    // Standard notation: file 'a'..'h', rank '1'..'8'
    // But internally usually rank 0 is 8th rank or 1st rank? Let's use standard Little-Endian Rank-File mapping
    // Rank 0 = 1st rank (a1, b1...), Rank 7 = 8th rank
    // File 0 = a, File 7 = h
    // Square = rank * 8 + file
    
    char f = 'a' + file;
    char r = '1' + rank;
    return {f, r};
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
    std::string s = squareToString(from) + squareToString(to);
    if (promotion != NO_PIECE_TYPE && promotion != NO_PIECE_TYPE) {
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
    m.to = stringToSquare(s.substr(2, 2));
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
