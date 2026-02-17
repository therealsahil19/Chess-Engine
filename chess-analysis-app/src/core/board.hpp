#pragma once
#include "types.hpp"
#include <vector>
#include <string>
#include <array>
#include <map>
#include <sstream>

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

    // Inlined for linking
    bool isCheckmate() const {
        return isCheck() && getLegalMoves().empty();
    }

    bool isStalemate() const {
        return !isCheck() && getLegalMoves().empty();
    }
    
    // Inlined for linking
    bool isInsufficientMaterial() const {
        int wPawn = 0, wKnight = 0, wBishop = 0, wRook = 0, wQueen = 0;
        int bPawn = 0, bKnight = 0, bBishop = 0, bRook = 0, bQueen = 0;

        for (int i = 0; i < 64; i++) {
            Piece p = board[i];
            if (p == NO_PIECE) continue;
            
            if (typeOf(p) == PAWN)   (colorOf(p) == White) ? wPawn++ : bPawn++;
            if (typeOf(p) == ROOK)   (colorOf(p) == White) ? wRook++ : bRook++;
            if (typeOf(p) == QUEEN)  (colorOf(p) == White) ? wQueen++ : bQueen++;
            if (typeOf(p) == KNIGHT) (colorOf(p) == White) ? wKnight++ : bKnight++;
            if (typeOf(p) == BISHOP) (colorOf(p) == White) ? wBishop++ : bBishop++;
        }

        if (wPawn + bPawn + wRook + bRook + wQueen + bQueen > 0) return false;

        int wMinor = wKnight + wBishop;
        int bMinor = bKnight + bBishop;

        if (wMinor == 0 && bMinor == 0) return true;
        
        if ((wMinor == 1 && wKnight == 1 && bMinor == 0) || 
            (bMinor == 1 && bKnight == 1 && wMinor == 0)) return true;

        if ((wMinor == 1 && wBishop == 1 && bMinor == 0) || 
            (bMinor == 1 && bBishop == 1 && wMinor == 0)) return true;

        return false;
    }
    
    bool isDraw() const {
        if (halfMoveClock >= 100) return true;
        if (isStalemate()) return true;
        if (isInsufficientMaterial()) return true;
        return false;
    }
    
    // Core Logic
    bool makeMove(Move move); 
    void undoMove(); 
    std::vector<Move> getLegalMoves() const;
    void reset();
    void loadFen(const std::string& fen);
    
    // Inlined for linking
    void loadPgn(const std::string& pgn) {
        reset();
        std::istringstream ss(pgn);
        std::string token;
        while (ss >> token) {
            if (token.find('.') != std::string::npos) continue;
            if (token == "1-0" || token == "0-1" || token == "1/2-1/2") break;
        }
    }
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


    inline void Board::clear() {
        board.fill(NO_PIECE);
        turn = White;
        castlingRights = 0;
        enPassantSquare = SQUARE_NONE;
        halfMoveClock = 0;
        fullMoveNumber = 1;
        history.clear();
    }

    inline void Board::loadFen(const std::string& fen) {
        clear();
        std::istringstream ss(fen);
        std::string token;
        
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
        
        ss >> token;
        turn = (token == "w") ? White : Black;
        
        ss >> token;
        if (token != "-") {
            for (char c : token) {
                if (c == 'K') castlingRights |= 1;
                else if (c == 'Q') castlingRights |= 2;
                else if (c == 'k') castlingRights |= 4;
                else if (c == 'q') castlingRights |= 8;
            }
        }
        
        ss >> token;
        if (token != "-") {
            enPassantSquare = stringToSquare(token);
        }
        
        if (ss >> token) halfMoveClock = std::stoi(token);
        if (ss >> token) fullMoveNumber = std::stoi(token);
    }

    inline Board::Board() { reset(); }
    inline Board::Board(const std::string& fen) { loadFen(fen); }
    
    inline void Board::reset() {
        loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }

    inline std::string Board::getFen() const {
        std::ostringstream ss;
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
        
        ss << " " << (turn == White ? "w" : "b");
        
        ss << " ";
        std::string castling = "";
        if (castlingRights & 1) castling += "K";
        if (castlingRights & 2) castling += "Q";
        if (castlingRights & 4) castling += "k";
        if (castlingRights & 8) castling += "q";
        if (castling.empty()) castling = "-";
        ss << castling;
        
        ss << " " << (enPassantSquare == SQUARE_NONE ? "-" : squareToString(enPassantSquare));
        ss << " " << halfMoveClock << " " << fullMoveNumber;
        
        return ss.str();
    }

    inline Piece Board::getPiece(Square s) const {
        if (s < 0 || s >= 64) return NO_PIECE;
        return board[s];
    }

    inline Side Board::getTurn() const { return turn; }

    inline bool Board::isSquareAttacked(Square s, Side attacker) const {
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

        const int RookDir[4] = {8, 1, -8, -1};
        for (int dir : RookDir) {
            int currSq = s;
            while (true) {
                int f = currSq % 8;
                int rank = currSq / 8;
                if ( (dir == 1) && f == 7 ) break;
                if ( (dir == -1) && f == 0 ) break;
                if ( (dir == 8) && rank == 7 ) break;
                if ( (dir == -8) && rank == 0 ) break;
                currSq += dir;
                if (currSq < 0 || currSq >= 64) break;
                Piece p = board[currSq];
                if (p != NO_PIECE) {
                    if (colorOf(p) == attacker && (typeOf(p) == ROOK || typeOf(p) == QUEEN)) return true;
                    break;
                }
            }
        }

        const int BishopDir[4] = {9, -7, -9, 7};
        for (int dir : BishopDir) {
            int currSq = s;
            while (true) {
                int f = currSq % 8;
                int rank = currSq / 8;
                if ( (dir == 9 || dir == -7) && f == 7 ) break;
                if ( (dir == -9 || dir == 7) && f == 0 ) break;
                if ( (dir == 9 || dir == 7) && rank == 7 ) break;
                if ( (dir == -7 || dir == -9) && rank == 0 ) break;
                currSq += dir;
                if (currSq < 0 || currSq >= 64) break;
                Piece p = board[currSq];
                if (p != NO_PIECE) {
                    if (colorOf(p) == attacker && (typeOf(p) == BISHOP || typeOf(p) == QUEEN)) return true;
                    break;
                }
            }
        }

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

    inline bool Board::isCheck() const {
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

    inline bool Board::makeMove(Move move) {
        if (move.from < 0 || move.from >= 64 || move.dest < 0 || move.dest >= 64) return false;
        Piece p = board[move.from];
        if (p == NO_PIECE || colorOf(p) != turn) return false;
        
        Piece captured = board[move.dest];
        Square oldEp = enPassantSquare;
        uint8_t oldCr = castlingRights;
        
        board[move.dest] = p;
        board[move.from] = NO_PIECE;
        if (typeOf(p) == PAWN && move.promotion != NO_PIECE_TYPE) {
            board[move.dest] = makePiece(turn, move.promotion);
        }

        if (isCheck()) {
            board[move.from] = p;
            board[move.dest] = captured;
            return false;
        }

        history.push_back({move, oldCr, oldEp, halfMoveClock, captured});
        enPassantSquare = SQUARE_NONE;
        if (typeOf(p) == PAWN && abs(move.dest - move.from) == 16) {
            enPassantSquare = move.from + (move.dest - move.from) / 2;
        }

        turn = (turn == White) ? Black : White;
        if (turn == White) fullMoveNumber++;
        halfMoveClock++;
        if (typeOf(p) == PAWN || captured != NO_PIECE) halfMoveClock = 0;

        return true;
    }

    inline std::string Board::moveToSan(const Move& m) {
        return m.toString(); 
    }

} // namespace Chess
