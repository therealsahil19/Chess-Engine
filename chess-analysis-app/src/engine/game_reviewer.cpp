#include "game_reviewer.hpp"
#include <cmath>
#include <algorithm>

namespace Chess {

void GameReviewer::startReview(const std::vector<std::string>& fens, Engine::StockfishClient& engine, int depth, const std::string& game_result) {
    complete_ = false;
    results_.clear();
    if (fens.size() < 2) {
        progress_ = 1.0f;
        complete_ = true;
        return;
    }
    
    results_.resize(fens.size() - 1);
    progress_ = 0.0f;

    std::thread([this, fens, &engine, depth, game_result]() {
        std::vector<float> evals(fens.size());
        std::vector<std::string> best_moves(fens.size());

        // Phase 1: get eval for every position
        for (size_t i = 0; i < fens.size(); ++i) {
            auto res = engine.analyzePosition(fens[i], depth);
            bool black_to_move = fens[i].find(" b ") != std::string::npos;
            evals[i] = black_to_move ? -res.centipawns : res.centipawns;
            best_moves[i] = res.best_move;
            progress_ = (float)i / fens.size() * 0.9f;
        }

        // Phase 2: classify each move
        for (size_t i = 0; i < results_.size(); ++i) {
            bool white_to_move = (fens[i].find(" w ") != std::string::npos);
            float before_white = evals[i];
            float after_white  = evals[i + 1];

            float before_mover = white_to_move ? before_white : -before_white;
            float after_mover  = white_to_move ? after_white  : -after_white;

            float cp_loss = white_to_move ? (before_white - after_white) : (after_white - before_white);
            cp_loss = std::max(0.0f, cp_loss);

            auto classification = classifyMove(cp_loss, before_mover);
            if (i == results_.size() - 1) {
                if ((white_to_move && game_result == "1-0") || (!white_to_move && game_result == "0-1")) {
                    classification = MoveClassification::GameEnd;
                }
            }

            results_[i] = {
                (int)i,
                before_white,
                after_white,
                cp_loss,
                best_moves[i],
                classification
            };
        }
        progress_ = 1.0f;
        complete_ = true;
    }).detach();
}

bool GameReviewer::isReviewComplete() const {
    return complete_;
}

float GameReviewer::getProgress() const {
    return progress_;
}

MoveClassification GameReviewer::classifyMove(float cp_loss, float eval_before) {
    if (eval_before < -300.0f) {
        cp_loss *= 0.5f;
    }

    if (cp_loss < 5)   return MoveClassification::Best;
    if (cp_loss < 20)  return MoveClassification::Excellent;
    if (cp_loss < 50)  return MoveClassification::Good;
    if (cp_loss < 100) return MoveClassification::Inaccuracy;
    if (cp_loss < 200) return MoveClassification::Mistake;
    return MoveClassification::Blunder;
}

ReviewSummary computeSummary(const std::vector<MoveReview>& reviews, bool white) {
    ReviewSummary s = {};
    float total_cp_loss = 0;
    int move_count = 0;

    for (size_t i = 0; i < reviews.size(); ++i) {
        bool is_white_move = (i % 2 == 0);
        if (is_white_move != white) continue;
        
        if (reviews[i].classification == MoveClassification::GameEnd) continue;

        move_count++;
        total_cp_loss += reviews[i].cp_loss;
        
        switch(reviews[i].classification) {
            case MoveClassification::Blunder:    s.blunders++;    break;
            case MoveClassification::Mistake:    s.mistakes++;    break;
            case MoveClassification::Inaccuracy: s.inaccuracies++; break;
            default:                             s.good_moves++;  break;
        }
    }
    
    float acpl = move_count ? (total_cp_loss / move_count) : 0.0f;
    s.accuracy = 103.1668f * std::exp(-0.00866f * acpl) - 3.1669f;
    s.accuracy = std::clamp(s.accuracy, 0.0f, 100.0f);
    
    return s;
}

} // namespace Chess
