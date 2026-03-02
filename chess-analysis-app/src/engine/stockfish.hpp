#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace Engine {

struct EngineResult {
    float centipawns = 0.0f;
    std::string best_move;
};

class StockfishClient {
public:
    StockfishClient(const std::string& path = "stockfish.exe");
    ~StockfishClient();

    bool start();
    void stop();
    
    // Non-blocking commands
    void sendCommand(const std::string& cmd);
    void setPosition(const std::string& fen, const std::vector<std::string>& moves = {});
    void go(int depth = 20);
    void stopAnalysis();

    // Blocking analysis for Game Review
    EngineResult analyzePosition(const std::string& fen, int depth);

    // Callbacks
    using EvalCallback = std::function<void(const std::string& score, const std::string& bestMove)>;
    void setEvalCallback(EvalCallback cb);

private:
    std::string exePath;
    std::atomic<bool> isRunning;
    std::thread outputThread;
    
    EvalCallback onEval;
    std::mutex callbackMutex;
    
    // Sync analysis state
    std::condition_variable syncCv;
    bool syncMode = false;
    EngineResult syncResult;

    // Use void* to avoid including windows.h in header (Raylib conflict)
    void* hChildStd_IN_Wr = nullptr;
    void* hChildStd_OUT_Rd = nullptr;
    void* hProcess = nullptr;

    void readOutputLoop();
    void parseOutput(const std::string& line);
};

} // namespace Engine
