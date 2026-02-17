#include "board.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Chess {

Board::Board() {
    reset();
}

Board::Board(const std::string& fen) {
    loadFen(fen);
}

void Board::reset() {
    loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::clear() {
    board.fill(NO_PIECE);
    turn = White;
    castlingRights = 0;
    enPassantSquare = SQUARE_NONE;
    halfMoveClock = 0;
    fullMoveNumber = 1;
    history.clear();
}

void Board::loadFen(const std::string& fen) {
    clear();
    std::istringstream ss(fen);
    std::string token;
    
    // 1. Piece placement
    ss >> token;
    int rank = 7;
    int file = 0;
    for (char c : token) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            file += (c - '0');
        } else {
            Side color = isupper(c) ? White : Black;
            PieceType pt;
            switch(tolower(c)) {
                case 'p': pt = PAWN; break;
                case 'n': pt = KNIGHT; break;
                case 'b': pt = BISHOP; break;
                case 'r': pt = ROOK; break;
                case 'q': pt = QUEEN; break;
                case 'k': pt = KING; break;
                default: pt = NO_PIECE_TYPE; break;
            }
            if (pt != NO_PIECE_TYPE) {
                board[rank * 8 + file] = makePiece(color, pt);
                file++;
            }
        }
    }
    
    // 2. Turn
    ss >> token;
    turn = (token == "w") ? White : Black;
    
    // 3. Castling
    ss >> token;
    if (token != "-") {
        for (char c : token) {
            if (c == 'K') castlingRights |= 1;
            else if (c == 'Q') castlingRights |= 2;
            else if (c == 'k') castlingRights |= 4;
            else if (c == 'q') castlingRights |= 8;
        }
    }
    
    // 4. En passant
    ss >> token;
    if (token != "-") {
        enPassantSquare = stringToSquare(token);
    }
    
    // 5. Halfmove clock
    if (ss >> token) halfMoveClock = std::stoi(token);
    
    // 6. Fullmove number
    if (ss >> token) fullMoveNumber = std::stoi(token);
}

std::string Board::getFen() const {
    std::ostringstream ss;
    // 1. Board
    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;
        for (int file = 0; file < 8; file++) {
            Piece p = board[rank * 8 + file];
            if (p == NO_PIECE) {
                empty++;
            } else {
                if (empty > 0) {
                    ss << empty;
                    empty = 0;
                }
                char c;
                switch(typeOf(p)) {
                    case PAWN: c = 'p'; break;
                    case KNIGHT: c = 'n'; break;
                    case BISHOP: c = 'b'; break;
                    case ROOK: c = 'r'; break;
                    case QUEEN: c = 'q'; break;
                    case KING: c = 'k'; break;
                    default: c = '?'; break;
                }
                if (colorOf(p) == White) c = toupper(c);
                ss << c;
            }
        }
        if (empty > 0) ss << empty;
        if (rank > 0) ss << "/";
    }
    
    // 2. Turn
    ss << " " << (turn == White ? "w" : "b");
    
    // 3. Castling
    ss << " ";
    std::string castling = "";
    if (castlingRights & 1) castling += "K";
    if (castlingRights & 2) castling += "Q";
    if (castlingRights & 4) castling += "k";
    if (castlingRights & 8) castling += "q";
    if (castling.empty()) castling = "-";
    ss << castling;
    
    // 4. En Passant
    ss << " " << (enPassantSquare == SQUARE_NONE ? "-" : squareToString(enPassantSquare));
    
    // 5. Clocks
    ss << " " << halfMoveClock << " " << fullMoveNumber;
    
    return ss.str();
}

// Stub implementations
Piece Board::getPiece(Square s) const {
    if (s < 0 || s >= 64) return NO_PIECE;
    return board[s];
}

Side Board::getTurn() const { return turn; }

bool Board::isCheckmate() const {
    return isCheck() && getLegalMoves().empty();
}

bool Board::isStalemate() const {
    return !isCheck() && getLegalMoves().empty();
}

bool Board::isInsufficientMaterial() const {
    int count = 0;
    for (int i = 0; i < 64; i++) {
        if (board[i] != NO_PIECE) count++;
    }
    return count <= 2;
}

bool Board::isDraw() const {
    if (halfMoveClock >= 100) return true;
    if (isStalemate()) return true;
    if (isInsufficientMaterial()) return true;
    return false;
}

bool Board::isSquareAttacked(Square s, Side attacker) const {
    // 1. Pawns
    int pDir = (attacker == White) ? -1 : 1; 
    for (int dx : {-1, 1}) {
        int file = Square(s) % 8;
        if (file + dx >= 0 && file + dx <= 7) {
            int pawnSq = s + (8 * pDir) + dx;
            if (pawnSq >= 0 && pawnSq < 64) {
                Piece p = board[pawnSq];
                if (typeOf(p) == PAWN && colorOf(p) == attacker) return true;
            }
        }
    }

    // 2. Knights
    const int KnightOffsets[] = {-17, -15, -10, -6, 6, 10, 15, 17};
    int r = Square(s) / 8;
    int c = Square(s) % 8;
    for (int offset : KnightOffsets) {
        int from = s + offset;
        if (from < 0 || from >= 64) continue;
        if (abs(from / 8 - r) + abs(from % 8 - c) == 3) {
            Piece p = board[from];
            if (typeOf(p) == KNIGHT && colorOf(p) == attacker) return true;
        }
    }

    // 3. Sliding (Rook/Queen)
    const int RookDir[4] = {8, 1, -8, -1};
    for (int dir : RookDir) {
        int to = s;
        while (true) {
            int f = to % 8;
            int rank = to / 8;
            if ( (dir == 1) && f == 7 ) break;
            if ( (dir == -1) && f == 0 ) break;
            if ( (dir == 8) && rank == 7 ) break;
            if ( (dir == -8) && rank == 0 ) break;
            to += dir;
            if (to < 0 || to >= 64) break;
            Piece p = board[to];
            if (p != NO_PIECE) {
                if (colorOf(p) == attacker && (typeOf(p) == ROOK || typeOf(p) == QUEEN)) return true;
                break;
            }
        }
    }

    // 4. Sliding (Bishop/Queen)
    const int BishopDir[4] = {9, -7, -9, 7};
    for (int dir : BishopDir) {
        int to = s;
        while (true) {
            int f = to % 8;
            int rank = to / 8;
            if ( (dir == 9 || dir == -7) && f == 7 ) break;
            if ( (dir == -9 || dir == 7) && f == 0 ) break;
            if ( (dir == 9 || dir == 7) && rank == 7 ) break;
            if ( (dir == -7 || dir == -9) && rank == 0 ) break;
            to += dir;
            if (to < 0 || to >= 64) break;
            Piece p = board[to];
            if (p != NO_PIECE) {
                if (colorOf(p) == attacker && (typeOf(p) == BISHOP || typeOf(p) == QUEEN)) return true;
                break;
            }
        }
    }

    // 5. King
    const int KingOffsets[] = {-9, -8, -7, -1, 1, 7, 8, 9};
    for (int offset : KingOffsets) {
        int from = s + offset;
        if (from < 0 || from >= 64) continue;
        if (abs(from / 8 - r) <= 1 && abs(from % 8 - c) <= 1) {
            Piece p = board[from];
            if (typeOf(p) == KING && colorOf(p) == attacker) return true;
        }
    }

    return false;
}

bool Board::isCheck() const {
    Square kingSq = -1;
    Piece ourKing = makePiece(turn, KING);
    for (int i = 0; i < 64; i++) {
        if (board[i] == ourKing) {
            kingSq = i;
            break;
        }
    }
    if (kingSq == -1) return false;
    return isSquareAttacked(kingSq, (turn == White ? Black : White));
}

bool Board::makeMove(Move move) {
    if (move.from < 0 || move.from >= 64 || move.to < 0 || move.to >= 64) return false;
    Piece p = board[move.from];
    if (p == NO_PIECE || colorOf(p) != turn) return false;
    
    Piece captured = board[move.to];
    Square oldEp = enPassantSquare;
    uint8_t oldCr = castlingRights;
    
    board[move.to] = p;
    board[move.from] = NO_PIECE;
    if (typeOf(p) == PAWN && move.promotion != NO_PIECE_TYPE) {
        board[move.to] = makePiece(turn, move.promotion);
    }

    if (isCheck()) {
        board[move.from] = p;
        board[move.to] = captured;
        return false;
    }

    history.push_back({move, oldCr, oldEp, halfMoveClock, captured});
    enPassantSquare = SQUARE_NONE;
    if (typeOf(p) == PAWN && abs(move.to - move.from) == 16) {
        enPassantSquare = move.from + (move.to - move.from) / 2;
    }

    turn = (turn == White) ? Black : White;
    if (turn == White) fullMoveNumber++;
    halfMoveClock++;
    if (typeOf(p) == PAWN || captured != NO_PIECE) halfMoveClock = 0;

    return true;
}

void Board::loadPgn(const std::string& pgn) {
    // Basic implementation: reset board and try to parse moves
    reset();
    // This requires a robust SAN parser which is hard.
    // For now, let's at least have the method.
}

std::string Board::moveToSan(const Move& m) {
    // Simple UCI for now, but placeholder for real SAN
    return m.toString(); 
}

} // namespace Chess
