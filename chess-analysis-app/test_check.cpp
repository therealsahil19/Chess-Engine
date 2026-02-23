#include "src/core/board.hpp"
#include <iostream>

using namespace Chess;

int main() {
    Board b;
    b.loadFen("8/8/8/8/8/8/4K3/3qk3 w - - 0 1");
    std::cout << "FEN: " << b.getFen() << "\n";
    std::cout << "King sq: " << -1 << "\n";
    Piece ourKing = makePiece(White, KING);
    for (int i = 0; i < 64; i++) {
        if (b.getPiece(i) == ourKing) {
            std::cout << "Found King at " << i << "\n";
        }
        if (b.getPiece(i) == makePiece(Black, QUEEN)) {
            std::cout << "Found Black Queen at " << i << "\n";
        }
    }
    std::cout << "isCheck: " << b.isCheck() << "\n";
}
