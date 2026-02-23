#include "raylib.h"
#include "core/board.hpp"
#include "core/game_record.hpp"
#include "engine/stockfish.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <mutex>

// Forward declaration of the platform-specific clipboard function
std::string GetClipboardTextFallback();

// Constants
const int SCREEN_WIDTH = 1000; // Expanded for UI
const int SCREEN_HEIGHT = 800;
const int BOARD_SIZE = 640;
const int SQUARE_SIZE = BOARD_SIZE / 8;
const int BOARD_OFFSET_X = 40;
const int BOARD_OFFSET_Y = (SCREEN_HEIGHT - BOARD_SIZE) / 2;

// Colors - High Contrast Theme
const Color COLOR_BG = { 43, 45, 48, 255 };        // IntelliJ-like Dark Gray
const Color COLOR_LIGHT = { 235, 236, 208, 255 };  // Cream / Lichess Light
const Color COLOR_DARK = { 119, 149, 86, 255 };   // Green / Lichess Dark
const Color COLOR_HIGHLIGHT = { 255, 246, 126, 180 }; // Yellow highlight
const Color COLOR_SELECTED = { 186, 202, 43, 200 };   // Lime-ish green for selection
const Color COLOR_TEXT_MAIN = { 205, 205, 205, 255 }; 
const Color COLOR_TEXT_DIM = { 150, 150, 150, 255 };

struct AnimState {
    bool active = false;
    Chess::Piece piece;
    Vector2 startPos;
    Vector2 endPos;
    float progress; // 0.0 to 1.0
    float speed = 8.0f; 
};

// Helper to parse PGN and populate record
void LoadPgnToRecord(const std::string& pgn, Chess::Board& board, Chess::GameRecord& record) {
    board.loadPgn(pgn);
    std::vector<Chess::Move> moves = board.getHistoryMoves();
    record.reset();
    board.reset();
    for (const auto& m : moves) {
        std::string san = board.moveToSan(m);
        record.addMove(m, san);
        board.makeMove(m);
    }
}

void DrawBoardBackground(int selectedSq, bool isFlipped) {
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            Color c = ((r + f) % 2 == 0) ? COLOR_DARK : COLOR_LIGHT;
            int drawR = isFlipped ? r : (7 - r);
            int drawF = isFlipped ? (7 - f) : f;
            int drawX = BOARD_OFFSET_X + drawF * SQUARE_SIZE;
            int drawY = BOARD_OFFSET_Y + drawR * SQUARE_SIZE;
            DrawRectangle(drawX, drawY, SQUARE_SIZE, SQUARE_SIZE, c);
        }
    }
    
    if (selectedSq != -1) {
        int r = selectedSq / 8;
        int f = selectedSq % 8;
        int drawR = isFlipped ? r : (7 - r);
        int drawF = isFlipped ? (7 - f) : f;
        DrawRectangle(BOARD_OFFSET_X + drawF * SQUARE_SIZE, BOARD_OFFSET_Y + drawR * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, COLOR_SELECTED);
    }
}

void DrawPieces(const Chess::Board& board, const AnimState& anim, bool isFlipped) {
    for (int i = 0; i < 64; i++) {
        Chess::Piece p = board.getPiece(i);
        if (p == Chess::NO_PIECE) continue;
        
        int r = i / 8;
        int f = i % 8;
        
        if (anim.active) {
             int destF = (int)((anim.endPos.x - BOARD_OFFSET_X)/SQUARE_SIZE);
             int destR = (int)((anim.endPos.y - BOARD_OFFSET_Y)/SQUARE_SIZE);
             int realF = isFlipped ? (7 - destF) : destF;
             int realR = isFlipped ? destR : (7 - destR);
             int destSq = realR * 8 + realF;
             if (i == destSq) continue; 
        }

        int drawR = isFlipped ? r : (7 - r);
        int drawF = isFlipped ? (7 - f) : f;
        int drawX = BOARD_OFFSET_X + drawF * SQUARE_SIZE;
        int drawY = BOARD_OFFSET_Y + drawR * SQUARE_SIZE;
        
        const char* text = "?";
        switch(Chess::typeOf(p)) {
            case Chess::PAWN: text = "P"; break;
            case Chess::KNIGHT: text = "N"; break;
            case Chess::BISHOP: text = "B"; break;
            case Chess::ROOK: text = "R"; break;
            case Chess::QUEEN: text = "Q"; break;
            case Chess::KING: text = "K"; break;
            default: break;
        }
        Color textColor = (Chess::colorOf(p) == Chess::White) ? WHITE : BLACK;
        DrawText(text, drawX + 35, drawY + 25, 40, textColor);
    }
    
    if (anim.active) {
        Vector2 pos;
        pos.x = anim.startPos.x + (anim.endPos.x - anim.startPos.x) * anim.progress;
        pos.y = anim.startPos.y + (anim.endPos.y - anim.startPos.y) * anim.progress;
        
        const char* text = "?";
         switch(Chess::typeOf(anim.piece)) {
            case Chess::PAWN: text = "P"; break;
            case Chess::KNIGHT: text = "N"; break;
            case Chess::BISHOP: text = "B"; break;
            case Chess::ROOK: text = "R"; break;
            case Chess::QUEEN: text = "Q"; break;
            case Chess::KING: text = "K"; break;
            default: break;
        }
        Color textColor = (Chess::colorOf(anim.piece) == Chess::White) ? WHITE : BLACK;
        DrawText(text, (int)pos.x + 35, (int)pos.y + 25, 40, textColor);
    }
}

void DrawSidePanel(bool showDialog, bool isAnalysisActive, int& scroll, const Chess::GameRecord& gameRecord, const std::string& currentEval, const std::string& bestMoveSan, std::mutex& evalMutex, Chess::Side turn) {
    int infoX = SCREEN_WIDTH - 280;
    
    std::string displayEval, displayBestMove;
    {
        std::lock_guard<std::mutex> lock(evalMutex);
        displayEval = currentEval;
        displayBestMove = bestMoveSan;
    }

    DrawText("Analysis", infoX, 20, 30, COLOR_TEXT_MAIN);
    DrawText(("Eval: " + displayEval).c_str(), infoX, 70, 20, COLOR_TEXT_MAIN);
    DrawText(("Best: " + displayBestMove).c_str(), infoX, 100, 20, COLOR_TEXT_MAIN);
    
    if (turn == Chess::White) DrawText("White to Move", infoX, 150, 20, COLOR_TEXT_DIM);
    else DrawText("Black to Move", infoX, 150, 20, COLOR_TEXT_DIM);
    
    if (!isAnalysisActive) {
        if (!showDialog) {
            int pasteButtonY = 420;
            bool mouseOverPaste = CheckCollisionPointRec(GetMousePosition(), { (float)infoX, (float)pasteButtonY, 150, 40 });
            DrawRectangle(infoX, pasteButtonY, 150, 40, mouseOverPaste ? COLOR_SELECTED : COLOR_DARK);
            DrawText("Paste PGN", infoX + 25, pasteButtonY + 10, 20, COLOR_BG);
        }
    } else {
        // Draw Move Table
        int tableY = 200;
        int tableWidth = 260;
        int tableHeight = 400;
        DrawRectangle(infoX, tableY, tableWidth, tableHeight, Fade(BLACK, 0.3f));
        
        DrawText("White", infoX + 40, tableY + 10, 20, COLOR_TEXT_MAIN);
        DrawText("Black", infoX + tableWidth / 2 + 40, tableY + 10, 20, COLOR_TEXT_MAIN);
        DrawLine(infoX + tableWidth / 2, tableY, infoX + tableWidth / 2, tableY + tableHeight, Fade(COLOR_TEXT_DIM, 0.5f));
        DrawLine(infoX, tableY + 35, infoX + tableWidth, tableY + 35, Fade(COLOR_TEXT_DIM, 0.5f));
        
        int itemsY = tableY + 45;
        int rowHeight = 25;
        int visibleRows = (tableHeight - 45) / rowHeight;
        int numRows = (gameRecord.sanMoves.size() + 1) / 2;
        
        if (numRows > visibleRows) {
            int currentRow = gameRecord.currentIndex / 2;
            if (currentRow > scroll + visibleRows - 1) scroll = currentRow - visibleRows + 1;
            if (currentRow < scroll) scroll = currentRow;
            if (scroll < 0) scroll = 0;
            if (scroll > numRows - visibleRows) scroll = numRows - visibleRows;
        } else {
            scroll = 0;
        }
        
        for (int i = 0; i < visibleRows; i++) {
            int rowIndex = scroll + i;
            if (rowIndex >= numRows) break;
            
            int moveNum = rowIndex + 1;
            int whiteMoveIdx = rowIndex * 2;
            int blackMoveIdx = rowIndex * 2 + 1;
            
            DrawText(std::to_string(moveNum).c_str(), infoX + 5, itemsY + i * rowHeight, 18, COLOR_TEXT_DIM);
            
            if (whiteMoveIdx == gameRecord.currentIndex - 1) DrawRectangle(infoX + 30, itemsY + i * rowHeight - 2, tableWidth / 2 - 30, rowHeight, Fade(COLOR_SELECTED, 0.5f));
            if (blackMoveIdx == gameRecord.currentIndex - 1) DrawRectangle(infoX + tableWidth / 2 + 5, itemsY + i * rowHeight - 2, tableWidth / 2 - 10, rowHeight, Fade(COLOR_SELECTED, 0.5f));

            if (whiteMoveIdx < gameRecord.sanMoves.size()) {
                DrawText(gameRecord.sanMoves[whiteMoveIdx].c_str(), infoX + 40, itemsY + i * rowHeight, 18, COLOR_TEXT_MAIN);
            }
            if (blackMoveIdx < gameRecord.sanMoves.size()) {
                DrawText(gameRecord.sanMoves[blackMoveIdx].c_str(), infoX + tableWidth / 2 + 30, itemsY + i * rowHeight, 18, COLOR_TEXT_MAIN);
            }
        }

        // Draw Playback Controls at bottom right
        int btnW = 40;
        int btnH = 30;
        int startX = infoX + (tableWidth - (5 * btnW + 4 * 5)) / 2; // Center 5 buttons
        int btnY = tableY + tableHeight + 20;

        Vector2 mp = GetMousePosition();
        const char* labels[] = {"|<", "<", "||", ">", ">|"};
        for (int i = 0; i < 5; i++) {
            int bx = startX + i * (btnW + 5);
            bool hover = CheckCollisionPointRec(mp, { (float)bx, (float)btnY, (float)btnW, (float)btnH });
            DrawRectangle(bx, btnY, btnW, btnH, hover ? COLOR_TEXT_DIM : Fade(BLACK, 0.5f));
            DrawText(labels[i], bx + 12 - (i==2?4:0), btnY + 8, 14, COLOR_TEXT_MAIN);
        }
    }
}

void DrawPasteDialog(bool& showDialog, std::string& dialogText, bool& submitPastedPgn, Vector2 mousePos) {
    if (!showDialog) return;
    
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.6f));
    
    int dialogW = 600;
    int dialogH = 400;
    int dialogX = (SCREEN_WIDTH - dialogW) / 2;
    int dialogY = (SCREEN_HEIGHT - dialogH) / 2;
    
    DrawRectangle(dialogX, dialogY, dialogW, dialogH, COLOR_BG);
    DrawRectangleLines(dialogX, dialogY, dialogW, dialogH, COLOR_TEXT_DIM);
    
    DrawText("Paste PGN (Ctrl+V)", dialogX + 20, dialogY + 20, 20, COLOR_TEXT_MAIN);
    
    bool overX = CheckCollisionPointRec(mousePos, { (float)(dialogX + dialogW - 40), (float)(dialogY + 10), 30, 30 });
    DrawText("X", dialogX + dialogW - 30, dialogY + 15, 20, overX ? RED : COLOR_TEXT_DIM);
    if (overX && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        showDialog = false;
        dialogText = "";
    }
    
    DrawRectangle(dialogX + 20, dialogY + 60, dialogW - 40, dialogH - 140, Fade(BLACK, 0.3f));
    std::string preview = dialogText;
    if (preview.length() > 500) preview = preview.substr(0, 500) + "...";
    
    DrawText("Pasted text preview:", dialogX + 25, dialogY + 65, 15, COLOR_TEXT_DIM);
    // Cheap text wrap rendering
    int dY = 85;
    for (size_t i = 0; i < preview.length(); i += 80) {
        DrawText(preview.substr(i, 80).c_str(), dialogX + 25, dialogY + dY, 10, COLOR_TEXT_MAIN);
        dY += 15;
        if (dY > dialogH - 100) break;
    }
    
    int btnW = 120;
    int btnH = 40;
    int btnX = dialogX + (dialogW - btnW) / 2;
    int btnY = dialogY + dialogH - 60;
    bool overBtn = CheckCollisionPointRec(mousePos, { (float)btnX, (float)btnY, (float)btnW, (float)btnH });
    if (dialogText.empty()) overBtn = false; 
    
    DrawRectangle(btnX, btnY, btnW, btnH, overBtn ? COLOR_SELECTED : (dialogText.empty() ? Fade(COLOR_DARK, 0.5f) : COLOR_DARK));
    DrawText("Analyze", btnX + 22, btnY + 10, 20, dialogText.empty() ? COLOR_TEXT_DIM : COLOR_BG);
    
    if (overBtn && !dialogText.empty() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        submitPastedPgn = true;
        showDialog = false;
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess Analysis App (Stockfish Enabled)");
    SetTargetFPS(60);

    Chess::Board board;
    board.reset();
    
    Chess::GameRecord gameRecord;
    Engine::StockfishClient engine("stockfish.exe");
    
    if (!engine.start()) {
        std::cerr << "Warning: Could not start stockfish.exe. Analysis disabled." << std::endl;
    }
    
    std::string currentEval = "N/A";
    std::string bestMoveSan = "";
    std::mutex evalMutex;
    
    engine.setEvalCallback([&](std::string score, std::string bestMove) {
        std::lock_guard<std::mutex> lock(evalMutex);
        currentEval = score;
        bestMoveSan = bestMove; 
    });

    AnimState anim;
    
    bool gameOver = false;
    int selectedSq = -1;
    bool isBoardFlipped = false;
    bool isAnalysisActive = false;
    bool showPasteDialog = false;
    std::string dialogPgnText = "";
    bool submitPastedPgn = false;
    int tableScroll = 0;
    std::string initialFen = board.getFen();
    
    auto triggerAnalysis = [&]() {
        engine.stopAnalysis();
        engine.setPosition(board.getFen());
        engine.go(20); 
    };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 mousePos = GetMousePosition();
        
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();
            if (droppedFiles.count > 0) {
                char* text = LoadFileText(droppedFiles.paths[0]);
                if (text) {
                    dialogPgnText = text;
                    showPasteDialog = true;
                    UnloadFileText(text);
                }
            }
            UnloadDroppedFiles(droppedFiles);
            selectedSq = -1;
        }
        
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        if (anim.active) {
            anim.progress += dt * anim.speed;
            if (anim.progress >= 1.0f) {
                anim.active = false;
            }
        }

        if (showPasteDialog && IsKeyPressed(KEY_V) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
            std::string content = "";
            const char* clipboardText = GetClipboardText();
            if (clipboardText) content = clipboardText;
            else content = GetClipboardTextFallback();
            if (!content.empty()) dialogPgnText = content;
        }

        if (submitPastedPgn) {
            submitPastedPgn = false;
            if (dialogPgnText.find("[Event") != std::string::npos || dialogPgnText.find("1.") != std::string::npos) {
                initialFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
                LoadPgnToRecord(dialogPgnText, board, gameRecord);
            } else {
                initialFen = dialogPgnText;
                board.loadFen(dialogPgnText);
                gameRecord.reset();
            }
            isAnalysisActive = true;
            triggerAnalysis();
            selectedSq = -1;
        }

        if (!anim.active && !showPasteDialog) {
            if (IsKeyPressed(KEY_RIGHT) && gameRecord.hasNext()) {
                Chess::Move m = gameRecord.next();
                anim.active = true;
                anim.piece = board.getPiece(m.from);
                anim.progress = 0.0f;
                int f1 = m.from % 8; int r1 = m.from / 8;
                int f2 = m.dest % 8; int r2 = m.dest / 8;
                int drawR1 = isBoardFlipped ? r1 : (7 - r1);
                int drawF1 = isBoardFlipped ? (7 - f1) : f1;
                int drawR2 = isBoardFlipped ? r2 : (7 - r2);
                int drawF2 = isBoardFlipped ? (7 - f2) : f2;
                anim.startPos = {(float)(BOARD_OFFSET_X + drawF1 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + drawR1 * SQUARE_SIZE)};
                anim.endPos = {(float)(BOARD_OFFSET_X + drawF2 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + drawR2 * SQUARE_SIZE)};
                board.makeMove(m);
                triggerAnalysis();
                selectedSq = -1;
            }
            else if (IsKeyPressed(KEY_LEFT) && gameRecord.hasPrev()) {
                board.undoMove();
                gameRecord.prev();
                triggerAnalysis();
                selectedSq = -1;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !showPasteDialog) {
            bool clickedUI = false;
            
            // Top Right Buttons
            if (CheckCollisionPointRec(mousePos, { SCREEN_WIDTH - 240, 10, 110, 30 })) {
                 isBoardFlipped = !isBoardFlipped;
                 clickedUI = true;
            } else if (CheckCollisionPointRec(mousePos, { SCREEN_WIDTH - 120, 10, 110, 30 })) {
                 board.loadFen(initialFen);
                 gameRecord.reset();
                 isAnalysisActive = false;
                 triggerAnalysis();
                 selectedSq = -1;
                 clickedUI = true;
            }
            // Paste Button
            else if (!isAnalysisActive && CheckCollisionPointRec(mousePos, { SCREEN_WIDTH - 280, 420, 150, 40 })) {
                showPasteDialog = true;
                clickedUI = true;
            }
            // Playback controls
            else if (isAnalysisActive) {
                int tableY = 200;
                int tableWidth = 260;
                int btnW = 40;
                int btnH = 30;
                int infoX = SCREEN_WIDTH - 280;
                int startX = infoX + (tableWidth - (5 * btnW + 4 * 5)) / 2;
                int btnY = tableY + 400 + 20;
                
                for (int i=0; i<5; i++) {
                    int bx = startX + i * (btnW + 5);
                    if (CheckCollisionPointRec(mousePos, { (float)bx, (float)btnY, (float)btnW, (float)btnH })) {
                        clickedUI = true;
                        if (i == 0) {
                            while(gameRecord.hasPrev()) { board.undoMove(); gameRecord.prev(); }
                            triggerAnalysis();
                        } else if (i == 1) {
                            if(gameRecord.hasPrev()) { board.undoMove(); gameRecord.prev(); triggerAnalysis(); }
                        } else if (i == 2) {
                            
                        } else if (i == 3 && !anim.active) {
                            if(gameRecord.hasNext()) {
                                Chess::Move m = gameRecord.next();
                                anim.active = true;
                                anim.piece = board.getPiece(m.from);
                                anim.progress = 0.0f;
                                int f1 = m.from % 8; int r1 = m.from / 8;
                                int f2 = m.dest % 8; int r2 = m.dest / 8;
                                int drawR1 = isBoardFlipped ? r1 : (7 - r1);
                                int drawF1 = isBoardFlipped ? (7 - f1) : f1;
                                int drawR2 = isBoardFlipped ? r2 : (7 - r2);
                                int drawF2 = isBoardFlipped ? (7 - f2) : f2;
                                anim.startPos = {(float)(BOARD_OFFSET_X + drawF1 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + drawR1 * SQUARE_SIZE)};
                                anim.endPos = {(float)(BOARD_OFFSET_X + drawF2 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + drawR2 * SQUARE_SIZE)};
                                board.makeMove(m);
                                triggerAnalysis();
                            }
                        } else if (i == 4) {
                            while(gameRecord.hasNext()) { board.makeMove(gameRecord.next()); }
                            triggerAnalysis();
                        }
                        selectedSq = -1;
                    }
                }
            }

            if (!clickedUI && !anim.active) {
                if (mousePos.x >= BOARD_OFFSET_X && mousePos.x < BOARD_OFFSET_X + BOARD_SIZE &&
                    mousePos.y >= BOARD_OFFSET_Y && mousePos.y < BOARD_OFFSET_Y + BOARD_SIZE) {
                    
                    int uiF = (mousePos.x - BOARD_OFFSET_X) / SQUARE_SIZE;
                    int uiR = (mousePos.y - BOARD_OFFSET_Y) / SQUARE_SIZE;
                    int file = isBoardFlipped ? (7 - uiF) : uiF;
                    int rank = isBoardFlipped ? uiR : (7 - uiR);
                    int sq = (7 - rank) * 8 + file;
                    
                    if (selectedSq == -1) {
                        Chess::Piece p = board.getPiece(sq);
                        if (p != Chess::NO_PIECE && Chess::colorOf(p) == board.getTurn()) {
                            selectedSq = sq;
                        }
                    } else {
                        Chess::Move m;
                        m.from = selectedSq;
                        m.dest = sq;
                        m.promotion = Chess::NO_PIECE_TYPE;
                        
                        Chess::Piece p = board.getPiece(selectedSq);
                        if (Chess::typeOf(p) == Chess::PAWN) {
                             int r = sq / 8;
                             if (r == 0 || r == 7) m.promotion = Chess::QUEEN; 
                        }
                        
                        std::vector<Chess::Move> legal = board.getLegalMoves();
                        bool found = false;
                        for (const auto& lm : legal) {
                            if (lm.from == m.from && lm.dest == m.dest && (m.promotion == Chess::NO_PIECE_TYPE || lm.promotion == m.promotion)) {
                                m = lm; 
                                found = true; 
                                break;
                            }
                        }
                        
                        if (found) {
                            if (gameRecord.hasNext()) {
                                 Chess::Move nextRec = gameRecord.moves[gameRecord.currentIndex];
                                 if (nextRec.from != m.from || nextRec.dest != m.dest || nextRec.promotion != m.promotion) {
                                     gameRecord.addMove(m, board.moveToSan(m)); 
                                 } else {
                                     gameRecord.currentIndex++;
                                 }
                            } else {
                                 gameRecord.addMove(m, board.moveToSan(m));
                            }

                            anim.active = true;
                            anim.piece = p;
                            anim.progress = 0.0f;
                            int f1 = m.from % 8; int r1 = m.from / 8;
                            int f2 = m.dest % 8; int r2 = m.dest / 8;
                            int drawR1 = isBoardFlipped ? r1 : (7 - r1);
                            int drawF1 = isBoardFlipped ? (7 - f1) : f1;
                            int drawR2 = isBoardFlipped ? r2 : (7 - r2);
                            int drawF2 = isBoardFlipped ? (7 - f2) : f2;
                            anim.startPos = {(float)(BOARD_OFFSET_X + drawF1 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + drawR1 * SQUARE_SIZE)};
                            anim.endPos = {(float)(BOARD_OFFSET_X + drawF2 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + drawR2 * SQUARE_SIZE)};

                            board.makeMove(m);
                            triggerAnalysis();
                            selectedSq = -1;
                        } else {
                            Chess::Piece target = board.getPiece(sq);
                            if (target != Chess::NO_PIECE && Chess::colorOf(target) == board.getTurn()) {
                                selectedSq = sq;
                            } else {
                                selectedSq = -1;
                            }
                        }
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(COLOR_BG);
        
        // Draw Top Right Buttons
        DrawRectangle(SCREEN_WIDTH - 240, 10, 110, 30, CheckCollisionPointRec(mousePos, { SCREEN_WIDTH - 240, 10, 110, 30 }) ? COLOR_SELECTED : COLOR_DARK);
        DrawText("Flip Board", SCREEN_WIDTH - 230, 16, 16, COLOR_TEXT_MAIN);

        DrawRectangle(SCREEN_WIDTH - 120, 10, 110, 30, CheckCollisionPointRec(mousePos, { SCREEN_WIDTH - 120, 10, 110, 30 }) ? RED : COLOR_DARK);
        DrawText("Reload", SCREEN_WIDTH - 90, 16, 16, COLOR_TEXT_MAIN);

        DrawBoardBackground(selectedSq, isBoardFlipped);
        DrawPieces(board, anim, isBoardFlipped);
        DrawSidePanel(showPasteDialog, isAnalysisActive, tableScroll, gameRecord, currentEval, bestMoveSan, evalMutex, board.getTurn());
        DrawPasteDialog(showPasteDialog, dialogPgnText, submitPastedPgn, mousePos);
        
        EndDrawing();
    }
    
    engine.stop();
    CloseWindow();
    return 0;
}
