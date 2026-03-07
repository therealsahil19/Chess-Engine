#include "../src/core/board.hpp"
#include <iostream>
#include <chrono>
#include <vector>

using namespace Chess;

int main() {
    Board b;
    std::vector<std::string> moves = {"e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "O-O", "h3", "Na5", "Bc2", "c5", "d4", "Qc7"};

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        b.reset();
        for (const auto& san : moves) {
            Move m = b.parseSan(san);
            if (!m.isNull()) {
                b.makeMove(m);
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Time for 1000 iterations: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}
