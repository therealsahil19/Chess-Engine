#include "../src/core/board.hpp"
#include "../src/core/types.hpp"
#include "../src/core/game_record.hpp"
#include "../src/engine/stockfish.hpp"
#include "../src/engine/game_reviewer.hpp"
#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <cassert>

using namespace Chess;

#define EXPECT_TRUE(cond) if (!(cond)) { std::cerr << "FAIL: " << #cond << " at " << __LINE__ << std::endl; exit(1); }
#define EXPECT_FALSE(cond) if (cond) { std::cerr << "FAIL: " << #cond << " at " << __LINE__ << std::endl; exit(1); }
template<typename T1, typename T2>
void expect_eq(const T1& a, const T2& b, const char* a_str, const char* b_str, int line) {
    if (a != b) {
        std::cerr << "FAIL: " << a_str << " != " << b_str;
        if constexpr (std::is_enum_v<T1> || std::is_integral_v<T1>) {
            std::cerr << " (" << static_cast<long long>(a);
        } else {
            std::cerr << " (" << a;
        }
        std::cerr << " != ";
        if constexpr (std::is_enum_v<T2> || std::is_integral_v<T2>) {
            std::cerr << static_cast<long long>(b);
        } else {
            std::cerr << b;
        }
        std::cerr << ") at " << line << std::endl;
        exit(1);
    }
}

#define EXPECT_EQ(a, b) expect_eq(a, b, #a, #b, __LINE__)

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

void test_isCheckmate() {
    Board b;
    // Fool's mate for white (Black delivers mate)
    b.loadFen("rnb1kbnr/pppp1ppp/8/4p3/5PPq/8/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    EXPECT_TRUE(b.isCheckmate());
    
    b.reset();
    EXPECT_FALSE(b.isCheckmate());
}

void test_addMove() {
    GameRecord gr;
    Board b;
    b.reset();
    Move m1 = b.parseSan("e4");
    b.makeMove(m1);
    gr.addMove(m1, "e4");
    
    Move m2 = b.parseSan("e5");
    b.makeMove(m2);
    gr.addMove(m2, "e5");
    
    EXPECT_EQ(gr.moves.size(), 2);
    
    gr.prev(); // index becomes 1
    b.undoMove();
    
    Move m3 = b.parseSan("d4");
    gr.addMove(m3, "d4");
    
    EXPECT_EQ(gr.moves.size(), 2);
    EXPECT_EQ(gr.sanMoves[1], "d4");
}

void test_loadPgn() {
    Board b;
    std::string pgn = "1. e4 e5 2. Nf3 Nc6 3. Bc4 Bc5";
    b.loadPgn(pgn);
    EXPECT_EQ(b.getFen(), "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
}

void test_stockfish_integration() {
#ifdef _WIN32
    Engine::StockfishClient sf("../chess-analysis-app/stockfish.exe");
    bool started = sf.start();
    EXPECT_TRUE(started);
    
    bool callbackFired = false;
    sf.setEvalCallback([&](const std::string& score, const std::string& bestMove) {
        if (!score.empty()) callbackFired = true;
    });
    
    sf.sendCommand("uci");
    sf.stopAnalysis(); 
    
    sf.setPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    sf.go(1);
    
    // Wait for the background thread to parse output and call callback
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    sf.stop();
    EXPECT_TRUE(callbackFired);
#endif
}

void test_game_reviewer_classification() {
    GameReviewer gr;

    // Normal evaluation cases (eval_before >= -300)
    EXPECT_EQ(gr.classifyMove(2.0f, 10.0f), MoveClassification::Best);
    EXPECT_EQ(gr.classifyMove(10.0f, 10.0f), MoveClassification::Excellent);
    EXPECT_EQ(gr.classifyMove(30.0f, 10.0f), MoveClassification::Good);
    EXPECT_EQ(gr.classifyMove(70.0f, 10.0f), MoveClassification::Inaccuracy);
    EXPECT_EQ(gr.classifyMove(150.0f, 10.0f), MoveClassification::Mistake);
    EXPECT_EQ(gr.classifyMove(250.0f, 10.0f), MoveClassification::Blunder);

    // Bad evaluation adjustment (eval_before < -300, cp_loss is halved)
    // 250 -> 125, which should be Mistake (< 200)
    EXPECT_EQ(gr.classifyMove(250.0f, -400.0f), MoveClassification::Mistake);
    // 50 -> 25, which should be Good (< 50)
    EXPECT_EQ(gr.classifyMove(50.0f, -400.0f), MoveClassification::Good);
}

void test_game_reviewer_summary() {
    std::vector<MoveReview> reviews;
    // White moves at even ply: 0, 2
    reviews.push_back({0, 10.0f, 5.0f, 5.0f, "e2e4", MoveClassification::Best}); // White
    reviews.push_back({1, 5.0f, -20.0f, 25.0f, "e7e5", MoveClassification::Good}); // Black
    reviews.push_back({2, -20.0f, -150.0f, 130.0f, "d2d4", MoveClassification::Mistake}); // White
    reviews.push_back({3, -150.0f, -150.0f, 0.0f, "d7d5", MoveClassification::GameEnd}); // Black

    ReviewSummary white_summary = computeSummary(reviews, true);
    EXPECT_EQ(white_summary.mistakes, 1);
    EXPECT_EQ(white_summary.good_moves, 1); // Best is categorized under good_moves in computeSummary
    EXPECT_EQ(white_summary.blunders, 0);

    ReviewSummary black_summary = computeSummary(reviews, false);
    EXPECT_EQ(black_summary.good_moves, 1);
    EXPECT_EQ(black_summary.inaccuracies, 0);
    // GameEnd should be skipped
}

void test_move_serialization() {
    // Test Move::fromString
    Move m1 = Move::fromString("e2e4");
    EXPECT_EQ(m1.from, stringToSquare("e2"));
    EXPECT_EQ(m1.dest, stringToSquare("e4"));
    EXPECT_EQ(m1.promotion, NO_PIECE_TYPE);

    Move m2 = Move::fromString("a7a8q");
    EXPECT_EQ(m2.from, stringToSquare("a7"));
    EXPECT_EQ(m2.dest, stringToSquare("a8"));
    EXPECT_EQ(m2.promotion, QUEEN);

    Move m3 = Move::fromString("a7a8r");
    EXPECT_EQ(m3.promotion, ROOK);

    Move m4 = Move::fromString("a7a8b");
    EXPECT_EQ(m4.promotion, BISHOP);

    Move m5 = Move::fromString("a7a8n");
    EXPECT_EQ(m5.promotion, KNIGHT);

    Move m6 = Move::fromString("abc");
    EXPECT_TRUE(m6.isNull());

    Move m7 = Move::fromString("a7a8x");
    EXPECT_EQ(m7.promotion, NO_PIECE_TYPE);

    // Test Move::toString
    Move m8;
    m8.from = stringToSquare("e2");
    m8.dest = stringToSquare("e4");
    m8.promotion = NO_PIECE_TYPE;
    EXPECT_EQ(m8.toString(), "e2e4");

    Move m9;
    m9.from = stringToSquare("a7");
    m9.dest = stringToSquare("a8");
    m9.promotion = QUEEN;
    EXPECT_EQ(m9.toString(), "a7a8q");

    Move m10; // Null move
    EXPECT_EQ(m10.toString(), "0000");
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
    test_isCheckmate();
    test_addMove();
    test_loadPgn();
    test_stockfish_integration();
    test_game_reviewer_classification();
    test_game_reviewer_summary();
    test_move_serialization();
    std::cout << "All tests passed!\n";
    return 0;
}
