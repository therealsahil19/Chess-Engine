#pragma once
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include "stockfish.hpp"
#include "../core/game_record.hpp"

namespace Chess {

enum class MoveClassification {
    Brilliant,
    Best,
    Excellent,
    Good,
    Inaccuracy,
    Mistake,
    Blunder,
    Book,
    GameEnd
};

struct MoveReview {
    int ply;
    float eval_before;
    float eval_after;
    float cp_loss;
    std::string best_move_uci;
    MoveClassification classification;
};

struct ReviewSummary {
    int blunders = 0;
    int mistakes = 0;
    int inaccuracies = 0;
    int good_moves = 0;
    float accuracy = 0.0f;
};

class GameReviewer {
public:
    void startReview(const std::vector<std::string>& fens, Engine::StockfishClient& engine, int depth = 18, const std::string& game_result = "");
    bool isReviewComplete() const;
    float getProgress() const;
    const std::vector<MoveReview>& getResults() const { return results_; }
    MoveClassification classifyMove(float cp_loss, float eval_before);

private:
    std::vector<MoveReview> results_;
    bool complete_ = false;
    float progress_ = 0.0f;
};

// Summary computation
ReviewSummary computeSummary(const std::vector<MoveReview>& reviews, bool white);

} // namespace Chess
