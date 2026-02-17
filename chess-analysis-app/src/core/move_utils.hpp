#pragma once
#include "types.hpp"
#include <string>

namespace Chess {

inline std::string moveToCoord(const Move& m) {
    std::string s = squareToString(m.from) + squareToString(m.dest);
    if (m.promotion != NO_PIECE_TYPE) {
        char p;
        switch(m.promotion) {
            case QUEEN: p = 'q'; break;
            case ROOK: p = 'r'; break;
            case BISHOP: p = 'b'; break;
            case KNIGHT: p = 'n'; break;
            default: p = 'q'; break;
        }
        s += p;
    }
    return s;
}

}
