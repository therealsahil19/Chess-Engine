#include "stockfish.hpp"
#include <iostream>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Engine {

StockfishClient::StockfishClient(const std::string& path) : exePath(path), isRunning(false) {}

StockfishClient::~StockfishClient() {
    stop();
}

bool StockfishClient::start() {
#ifdef _WIN32
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hOutRd, hOutWr;
    HANDLE hInRd, hInWr;

    // Create a pipe for the child process's STDOUT.
    if (!CreatePipe(&hOutRd, &hOutWr, &saAttr, 0)) return false;
    if (!SetHandleInformation(hOutRd, HANDLE_FLAG_INHERIT, 0)) return false;

    // Create a pipe for the child process's STDIN.
    if (!CreatePipe(&hInRd, &hInWr, &saAttr, 0)) return false;
    if (!SetHandleInformation(hInWr, HANDLE_FLAG_INHERIT, 0)) return false;

    // Create the child process.
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hOutWr;
    siStartInfo.hStdOutput = hOutWr;
    siStartInfo.hStdInput = hInRd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process.
    std::string commandLine = "\"" + exePath + "\"";
    std::vector<char> cmd(commandLine.begin(), commandLine.end());
    cmd.push_back(0);

    if (!CreateProcessA(NULL, 
        cmd.data(),     
        NULL,          
        NULL,          
        TRUE,          
        CREATE_NO_WINDOW, 
        NULL,          
        NULL,          
        &siStartInfo,  
        &piProcInfo    
    )) {
        CloseHandle(hOutRd); CloseHandle(hOutWr);
        CloseHandle(hInRd); CloseHandle(hInWr);
        return false;
    }

    // Store handles
    hProcess = (void*)piProcInfo.hProcess;
    CloseHandle(piProcInfo.hThread);
    
    // Close handles we don't need
    CloseHandle(hOutWr);
    CloseHandle(hInRd);
    
    // Store ours
    hChildStd_OUT_Rd = (void*)hOutRd;
    hChildStd_IN_Wr = (void*)hInWr;
    
    // Initialize others to null to be safe? 
    // We reused local vars for creation to avoid casting mess in CreatePipe

    isRunning = true;
    outputThread = std::thread(&StockfishClient::readOutputLoop, this);
    
    sendCommand("uci");
    sendCommand("isready");

    return true;
#else
    return false; // Not implemented for non-Windows
#endif
}

void StockfishClient::stop() {
    if (!isRunning) return;
    
    sendCommand("quit");
    isRunning = false;
    
    if (outputThread.joinable()) outputThread.join();

#ifdef _WIN32
    if (hProcess) {
        WaitForSingleObject((HANDLE)hProcess, 1000);
        CloseHandle((HANDLE)hProcess);
        hProcess = nullptr;
    }
    if (hChildStd_IN_Wr) {
        CloseHandle((HANDLE)hChildStd_IN_Wr);
        hChildStd_IN_Wr = nullptr;
    }
    if (hChildStd_OUT_Rd) {
        CloseHandle((HANDLE)hChildStd_OUT_Rd);
         hChildStd_OUT_Rd = nullptr;
    }
#endif
}

void StockfishClient::sendCommand(const std::string& cmd) {
    if (!isRunning) return;
    std::string fullCmd = cmd;
    if (fullCmd.empty() || fullCmd.back() != '\n') {
        fullCmd += "\n";
    }
#ifdef _WIN32
    DWORD dwWritten;
    WriteFile((HANDLE)hChildStd_IN_Wr, fullCmd.c_str(), fullCmd.size(), &dwWritten, NULL);
#endif
}

void StockfishClient::setPosition(const std::string& fen, const std::vector<std::string>& moves) {
    std::stringstream ss;
    ss << "position fen " << fen;
    if (!moves.empty()) {
        ss << " moves";
        for (const auto& m : moves) {
            ss << " " << m;
        }
    }
    sendCommand(ss.str());
}

void StockfishClient::go(int depth) {
    sendCommand("go depth " + std::to_string(depth));
}

void StockfishClient::stopAnalysis() {
    sendCommand("stop");
}

void StockfishClient::setEvalCallback(EvalCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    onEval = cb;
}

void StockfishClient::readOutputLoop() {
#ifdef _WIN32
    const int BUFSIZE = 4096;
    CHAR chBuf[BUFSIZE]; 
    DWORD dwRead; 
    std::string buffer = "";

    while (isRunning) { 
        if (!ReadFile((HANDLE)hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL) || dwRead == 0) break; 
        
        buffer.append(chBuf, dwRead);
        
        size_t pos = 0;
        size_t nextPos;
        while ((nextPos = buffer.find('\n', pos)) != std::string::npos) {
            std::string_view line(buffer.data() + pos, nextPos - pos);
            if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
            
            if (line.compare(0, 4, "info") == 0 && line.find("score") != std::string_view::npos) {
                parseOutput(std::string(line));
            }
            pos = nextPos + 1;
        }
        buffer.erase(0, pos);
    } 
#endif
}

void StockfishClient::parseOutput(const std::string& line) {
    // Example: info depth 10 seldepth 14 multipv 1 score cp 32 nodes 1234 nps 4321 hashfull 0 tbhits 0 time 5 pv e2e4 e7e5
    // info depth 20 ... score mate 3 ...
    
    if (line.rfind("info", 0) == 0 && line.find("score") != std::string::npos) {
        std::string score = "";
        std::string bestMove = ""; // "pv" usually follows
        
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            if (token == "score") {
                std::string type;
                int val;
                iss >> type >> val;
                if (type == "cp") {
                    score = (val > 0 ? "+" : "") + std::to_string((float)val / 100.0f);
                } else if (type == "mate") {
                    score = "#" + std::to_string(val);
                }
            }
             else if (token == "pv") {
                 iss >> bestMove; // First move of PV
             }
        }
        
        // Callback
        std::lock_guard<std::mutex> lock(callbackMutex);
        if (onEval) onEval(score, bestMove);
    }
}

} // namespace Engine
