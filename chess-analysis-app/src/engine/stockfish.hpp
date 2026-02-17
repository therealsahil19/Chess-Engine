#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <mutex>

namespace Engine {

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

    // Callbacks
    using EvalCallback = std::function<void(const std::string& score, const std::string& bestMove)>;
    void setEvalCallback(EvalCallback cb);

private:
    std::string exePath;
    std::atomic<bool> isRunning;
    std::thread outputThread;
    
    EvalCallback onEval;
    std::mutex callbackMutex;

    // Use void* to avoid including windows.h in header (Raylib conflict)
    void* hChildStd_IN_Rd = nullptr;
    void* hChildStd_IN_Wr = nullptr;
    void* hChildStd_OUT_Rd = nullptr;
    void* hChildStd_OUT_Wr = nullptr;
    void* hProcess = nullptr;

    void readOutputLoop();
    void parseOutput(const std::string& line);
};

} // namespace Engine
