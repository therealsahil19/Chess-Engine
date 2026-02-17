#include "raylib.h"
#include "core/board.hpp"
#include "core/game_record.hpp"
#include "engine/stockfish.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

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
    record.reset();
    board.reset();
    
    std::string cleanPgn = pgn;
    // Simple cleanup
    std::replace(cleanPgn.begin(), cleanPgn.end(), '\n', ' ');
    std::replace(cleanPgn.begin(), cleanPgn.end(), '\r', ' ');
    
    std::string movesOnly;
    bool inTag = false;
    bool inComment = false;
    for (char c : cleanPgn) {
        if (c == '[') inTag = true;
        else if (c == '{') inComment = true;
        else if (c == ']') inTag = false;
        else if (c == '}') inComment = false;
        else if (!inTag && !inComment) movesOnly += c;
    }
    
    std::istringstream ss(movesOnly);
    std::string token;
    while (ss >> token) {
        if (token.find('.') != std::string::npos) continue; // "1."
        if (!isalpha(token[0])) continue; // Result "1-0" or bad token
        
        Chess::Move m = board.parseSan(token);
        if (!m.isNull()) {
            board.makeMove(m);
            record.addMove(m);
        }
    }
    
    // Reset to start for viewing
    board.reset();
    record.currentIndex = 0;
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
    
    // Engine Callback
    engine.setEvalCallback([&](std::string score, std::string bestMove) {
        currentEval = score;
        // Convert bestMove (coordinate) to SAN? Requires board state.
        // Thread safety issue: accessing board in callback? NO. Use generic storage.
        // Just store the coordinate string.
        bestMoveSan = bestMove; 
    });

    // Texture loading placeholder
    // Texture2D piecesTexture = LoadTexture("assets/pieces.png");

    AnimState anim;
    
    bool gameOver = false;
    int selectedSq = -1;
    
    // Auto-analysis triggering
    auto triggerAnalysis = [&]() {
        engine.stopAnalysis();
        engine.setPosition(board.getFen());
        engine.go(20); // Depth 20
    };

    while (!WindowShouldClose()) {
        // --- Update ---
        float dt = GetFrameTime();
        
        // Drag & Drop
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();
            if (droppedFiles.count > 0) {
                char* text = LoadFileText(droppedFiles.paths[0]);
                if (text) {
                    std::string content(text);
                    if (content.find("[Event") != std::string::npos || content.find("1.") != std::string::npos) {
                        LoadPgnToRecord(content, board, gameRecord);
                        triggerAnalysis();
                    } else {
                        // Assume FEN
                        board.loadFen(content);
                        gameRecord.reset();
                        triggerAnalysis();
                    }
                    UnloadFileText(text);
                }
            }
            UnloadDroppedFiles(droppedFiles);
            selectedSq = -1;
        }
        
        // Toggle Fullscreen
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        // Animation
        if (anim.active) {
            anim.progress += dt * anim.speed;
            if (anim.progress >= 1.0f) {
                anim.active = false;
                // Ensure board state is final (it already is, visual only)
            }
        }

        // Keyboard Navigation
        if (!anim.active) { // Block nav during animation? Or cancel?
            if (IsKeyPressed(KEY_RIGHT) && gameRecord.hasNext()) {
                Chess::Move m = gameRecord.next();
                
                // Start animation
                anim.active = true;
                anim.piece = board.getPiece(m.from);
                anim.progress = 0.0f;
                
                int f1 = m.from % 8; int r1 = m.from / 8;
                int f2 = m.dest % 8; int r2 = m.dest / 8;
                anim.startPos = {(float)(BOARD_OFFSET_X + f1 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + (7 - r1) * SQUARE_SIZE)};
                anim.endPos = {(float)(BOARD_OFFSET_X + f2 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + (7 - r2) * SQUARE_SIZE)};
                
                board.makeMove(m);
                triggerAnalysis();
                selectedSq = -1;
            }
            else if (IsKeyPressed(KEY_LEFT) && gameRecord.hasPrev()) {
                // For undo, we animate backwards? Or just snap? Snap is easier for now.
                // To animate: undo first, then anim from dest to start.
                // But `undoMove` restores piece at start.
                
                // Let's just snap for backward, or tricky logic.
                // Let's snap.
                board.undoMove();
                gameRecord.prev(); // Decrement index
                triggerAnalysis();
                selectedSq = -1;
            }
        }

        // Mouse Interaction
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !anim.active) {
            Vector2 mousePos = GetMousePosition();
            if (mousePos.x >= BOARD_OFFSET_X && mousePos.x < BOARD_OFFSET_X + BOARD_SIZE &&
                mousePos.y >= BOARD_OFFSET_Y && mousePos.y < BOARD_OFFSET_Y + BOARD_SIZE) {
                
                int file = (mousePos.x - BOARD_OFFSET_X) / SQUARE_SIZE;
                int rank = 7 - (int)((mousePos.y - BOARD_OFFSET_Y) / SQUARE_SIZE);
                int sq = rank * 8 + file;
                
                if (selectedSq == -1) {
                    Chess::Piece p = board.getPiece(sq);
                    if (p != Chess::NO_PIECE && Chess::colorOf(p) == board.getTurn()) {
                        selectedSq = sq;
                    }
                } else {
                    // Try move
                    Chess::Move m;
                    m.from = selectedSq;
                    m.dest = sq;
                    m.promotion = Chess::NO_PIECE_TYPE;
                    
                    Chess::Piece p = board.getPiece(selectedSq);
                    if (Chess::typeOf(p) == Chess::PAWN) {
                         int r = sq / 8;
                         if (r == 0 || r == 7) m.promotion = Chess::QUEEN; 
                    }
                    
                    // Validate
                    std::vector<Chess::Move> legal = board.getLegalMoves();
                    bool found = false;
                    for (const auto& lm : legal) {
                        if (lm.from == m.from && lm.dest == m.dest) {
                            m = lm; 
                            found = true; 
                            break;
                        }
                    }
                    
                    if (found) {
                        // User made a move -> deviation from record?
                        // Truncate future record
                        if (gameRecord.hasNext()) {
                             // Logic: if new move matches next record move, keep it?
                             // Else truncate.
                             Chess::Move nextRec = gameRecord.moves[gameRecord.currentIndex];
                             // Simple check: same from/dest/promo
                             if (nextRec.from != m.from || nextRec.dest != m.dest || nextRec.promotion != m.promotion) {
                                 // Diverged
                                 gameRecord.addMove(m); // Truncates implicitly by index check in addMove? NO.
                                 // My addMove logic: "if (currentIndex < moves.size()) moves.resize(currentIndex);"
                                 // So it handles divergence.
                             } else {
                                 // Matched record, just advance
                                 gameRecord.currentIndex++;
                             }
                        } else {
                             gameRecord.addMove(m);
                        }

                        // Animate
                        anim.active = true;
                        anim.piece = p;
                        anim.progress = 0.0f;
                        int f1 = m.from % 8; int r1 = m.from / 8;
                        int f2 = m.dest % 8; int r2 = m.dest / 8;
                        anim.startPos = {(float)(BOARD_OFFSET_X + f1 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + (7 - r1) * SQUARE_SIZE)};
                        anim.endPos = {(float)(BOARD_OFFSET_X + f2 * SQUARE_SIZE), (float)(BOARD_OFFSET_Y + (7 - r2) * SQUARE_SIZE)};

                        board.makeMove(m);
                        triggerAnalysis();
                        selectedSq = -1;
                    } else {
                        // Reselect?
                        Chess::Piece target = board.getPiece(sq);
                        if (target != Chess::NO_PIECE && Chess::colorOf(target) == board.getTurn()) {
                            selectedSq = sq;
                        } else {
                            selectedSq = -1;
                        }
                    }
                }
            }

            // "Paste PGN" Button Click Detection
            int pasteButtonX = SCREEN_WIDTH - 280;
            int pasteButtonY = 420;
            int pasteButtonWidth = 150;
            int pasteButtonHeight = 40;

            if (mousePos.x >= pasteButtonX && mousePos.x <= pasteButtonX + pasteButtonWidth &&
                mousePos.y >= pasteButtonY && mousePos.y <= pasteButtonY + pasteButtonHeight) {

                const char* clipboardText = GetClipboardText();
                if (clipboardText) {
                    std::string content(clipboardText);
                    if (content.find("[Event") != std::string::npos || content.find("1.") != std::string::npos) {
                        LoadPgnToRecord(content, board, gameRecord);
                        triggerAnalysis();
                    }
                    else if (content.length() > 20) { // Naive FEN length check
                        board.loadFen(content);
                        gameRecord.reset();
                        triggerAnalysis();
                    }
                    selectedSq = -1;
                }
            }
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground(COLOR_BG);
        
        // Draw Board
        for (int r = 0; r < 8; r++) {
            for (int f = 0; f < 8; f++) {
                Color c = ((r + f) % 2 == 0) ? COLOR_DARK : COLOR_LIGHT;
                int drawX = BOARD_OFFSET_X + f * SQUARE_SIZE;
                int drawY = BOARD_OFFSET_Y + (7 - r) * SQUARE_SIZE;
                DrawRectangle(drawX, drawY, SQUARE_SIZE, SQUARE_SIZE, c);
            }
        }
        
        // Highlight selection
        if (selectedSq != -1) {
            int r = selectedSq / 8;
            int f = selectedSq % 8;
            DrawRectangle(BOARD_OFFSET_X + f * SQUARE_SIZE, BOARD_OFFSET_Y + (7 - r) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, COLOR_SELECTED);
        }

        // Draw Pieces
        for (int i = 0; i < 64; i++) {
            // If animating, don't draw the piece at its NEW location (dest).
            // Main loop `makeMove` has already moved it to dest.
            // So if `anim.active` and `i == anim.piece` new loc...
            // Faster: Iterate board. If square == anim.dest, skip drawing it if it matches anim.piece?
            // Actually `makeMove` moves piece to dest.
            // So if we are animating a move from A->B. Piece is at B.
            // We want to draw it executing A->B.
            // So skip drawing at B.
            
            // Wait, what if B had a capture? B has the Mover now. Captured is gone.
            // We draw everything normally EXCEPT the moving piece at B.
            // Instead we draw the moving piece at `Lerp(A, B)`.
            
            Chess::Piece p = board.getPiece(i);
            if (p == Chess::NO_PIECE) continue;
            
            int r = i / 8;
            int f = i % 8;
            
            // If this is the piece we are animating (at destination)
            if (anim.active && i == (int)((anim.endPos.x - BOARD_OFFSET_X)/SQUARE_SIZE) + 8 * (7 - (int)((anim.endPos.y - BOARD_OFFSET_Y)/SQUARE_SIZE))) {
                 // Check if it's the right piece type? 
                 // Simple logic: If we are animating, we assume board state is post-move.
                 // The piece at `move.dest` is the one moving.
                 // We skip drawing it at grid position.
                 
                 // Re-calculate anim dest sq to be sure
                 int destSq = (int)((anim.endPos.x - BOARD_OFFSET_X)/SQUARE_SIZE) + 8 * (7 - (int)((anim.endPos.y - BOARD_OFFSET_Y)/SQUARE_SIZE));
                 if (i == destSq) continue; 
            }

            int drawX = BOARD_OFFSET_X + f * SQUARE_SIZE;
            int drawY = BOARD_OFFSET_Y + (7 - r) * SQUARE_SIZE;
            
            // Draw Text Placeholder
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
        
        // Draw Animated Piece
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
        
        // Draw INFO Panel
        int infoX = SCREEN_WIDTH - 280;
        DrawText("Analysis", infoX, 20, 30, COLOR_TEXT_MAIN);
        DrawText(("Eval: " + currentEval).c_str(), infoX, 70, 20, COLOR_TEXT_MAIN);
        DrawText(("Best: " + bestMoveSan).c_str(), infoX, 100, 20, COLOR_TEXT_MAIN);
        
        if (board.getTurn() == Chess::White) DrawText("White to Move", infoX, 150, 20, COLOR_TEXT_DIM);
        else DrawText("Black to Move", infoX, 150, 20, COLOR_TEXT_DIM);
        
        DrawText("Controls:", infoX, 300, 20, COLOR_TEXT_MAIN);
        DrawText("Arrow L/R: Navigate", infoX, 330, 15, COLOR_TEXT_DIM);
        DrawText("Drag PGN: Load Game", infoX, 355, 15, COLOR_TEXT_DIM);
        DrawText("F11: Full Screen", infoX, 380, 15, COLOR_TEXT_DIM);

        // Paste PGN Button
        int pasteButtonY = 420;
        bool mouseOverPaste = CheckCollisionPointRec(GetMousePosition(), { (float)infoX, (float)pasteButtonY, 150, 40 });
        DrawRectangle(infoX, pasteButtonY, 150, 40, mouseOverPaste ? COLOR_SELECTED : COLOR_DARK);
        DrawText("Paste PGN", infoX + 25, pasteButtonY + 10, 20, COLOR_BG);

        EndDrawing();
    }
    
    // Cleanup
    engine.stop();
    CloseWindow();
    return 0;
}
