#include "../src/core/board.hpp"
#include "../src/core/types.hpp"
#include <iostream>
#include <cassert>

using namespace Chess;

#define EXPECT_TRUE(cond) if (!(cond)) { std::cerr << "FAIL: " << #cond << " at " << __LINE__ << std::endl; exit(1); }
#define EXPECT_FALSE(cond) if (cond) { std::cerr << "FAIL: " << #cond << " at " << __LINE__ << std::endl; exit(1); }
#define EXPECT_EQ(a, b) if ((a) != (b)) { std::cerr << "FAIL: " << #a << " != " << #b << " (" << (a) << " != " << (b) << ") at " << __LINE__ << std::endl; exit(1); }

// Test cases
void test_squareToString_and_stringToSquare() {
    EXPECT_EQ(squareToString(0), "a1");
    EXPECT_EQ(squareToString(7), "h1");
    EXPECT_EQ(squareToString(56), "a8");
    EXPECT_EQ(squareToString(63), "h8");

    EXPECT_EQ(stringToSquare("a1"), 0);
    EXPECT_EQ(stringToSquare("h1"), 7);
    EXPECT_EQ(stringToSquare("a8"), 56);
    EXPECT_EQ(stringToSquare("h8"), 63);
}

void test_isInsufficientMaterial() {
    Board b;
    b.loadFen("8/8/8/8/8/8/4K3/4k3 w - - 0 1");
    EXPECT_TRUE(b.isInsufficientMaterial());

    b.loadFen("8/8/8/8/8/8/4K3/3nk3 w - - 0 1");
    EXPECT_TRUE(b.isInsufficientMaterial());

    b.loadFen("8/8/8/8/8/8/4K3/3qk3 w - - 0 1");
    EXPECT_FALSE(b.isInsufficientMaterial());
}

void test_isCheck() {
    Board b;
    b.loadFen("8/8/8/8/8/8/4K3/3qk3 w - - 0 1");
    EXPECT_TRUE(b.isCheck());

    b.loadFen("8/8/8/8/8/8/4K3/7k w - - 0 1");
    EXPECT_FALSE(b.isCheck());
}

void test_loadFen() {
    Board b;
    b.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    EXPECT_EQ(b.getPiece(0), makePiece(White, ROOK));
    EXPECT_EQ(b.getPiece(63), makePiece(Black, ROOK));
}

void test_undoMove() {
    Board b;
    b.reset();
    std::string startFen = b.getFen();
    Move m = b.parseSan("e4");
    b.makeMove(m);
    EXPECT_FALSE(b.getFen() == startFen);
    b.undoMove();
    EXPECT_EQ(b.getFen(), startFen);
}

void test_parseSan_moveToSan() {
    Board b;
    b.reset();
    Move m = b.parseSan("e4");
    EXPECT_EQ(b.moveToSan(m), "e4");

    b.loadFen("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3");
    Move m2 = b.parseSan("Nxe5");
    EXPECT_EQ(m2.from, stringToSquare("f3"));
    EXPECT_EQ(m2.dest, stringToSquare("e5"));
}

void test_castling() {
    Board b;
    b.loadFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    auto moves = b.getLegalMoves();
    bool canCastleKingside = false;
    bool canCastleQueenside = false;
    for (const auto& m : moves) {
        if (m.from == 4 && m.dest == 6) canCastleKingside = true;
        if (m.from == 4 && m.dest == 2) canCastleQueenside = true;
    }
    EXPECT_TRUE(canCastleKingside);
    EXPECT_TRUE(canCastleQueenside);
}

uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1;
    uint64_t nodes = 0;
    auto moves = board.getLegalMoves();
    for (const auto& m : moves) {
        board.makeMove(m);
        nodes += perft(board, depth - 1);
        board.undoMove();
    }
    return nodes;
}

void test_perft() {
    Board b;
    b.reset();
    // Start position perft depths: 1:20, 2:400, 3:8902, 4:197281
    uint64_t p1 = perft(b, 1);
    EXPECT_EQ(p1, 20ULL);
    uint64_t p2 = perft(b, 2);
    EXPECT_EQ(p2, 400ULL);
    uint64_t p3 = perft(b, 3);
    EXPECT_EQ(p3, 8902ULL);
    uint64_t p4 = perft(b, 4);
    EXPECT_EQ(p4, 197281ULL);
}

int main() {
    std::cout << "Running tests...\n";
    test_squareToString_and_stringToSquare();
    test_isInsufficientMaterial();
    test_isCheck();
    test_loadFen();
    test_undoMove();
    test_parseSan_moveToSan();
    test_castling();
    test_perft();
    std::cout << "All tests passed!\n";
    return 0;
}
