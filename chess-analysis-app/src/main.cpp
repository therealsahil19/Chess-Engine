#include "raylib.h"
#include "core/board.hpp"
#include <iostream>
#include <string>

// Constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int BOARD_SIZE = 640;
const int SQUARE_SIZE = BOARD_SIZE / 8;
const int BOARD_OFFSET_X = (SCREEN_WIDTH - BOARD_SIZE) / 2;
const int BOARD_OFFSET_Y = (SCREEN_HEIGHT - BOARD_SIZE) / 2;

// Colors
const Color COLOR_LIGHT = {240, 217, 181, 255};
const Color COLOR_DARK = {181, 136, 99, 255};
const Color COLOR_HIGHLIGHT = {255, 255, 0, 100};

int main() {
    // 1. Initialize Window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess Analysis App (C++ Native)");
    SetTargetFPS(60);

    // 2. Initialize Game
    Chess::Board board;
    board.reset();

    // 3. Load Resources
    // TODO: Load textures here
    // Texture2D piecesTexture = LoadTexture("assets/pieces.png");

    // 4. Game Loop
    // Game State variables
    bool gameOver = false;
    std::string gameStatus = "";

    // 4. Game Loop
    while (!WindowShouldClose()) {
        // --- Update ---
        
        // Drag & Drop Handling
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();
            if (droppedFiles.count > 0) {
                // Determine if FEN or PGN (basic check)
                std::string path = droppedFiles.paths[0];
                char* text = LoadFileText(path.c_str());
                if (text) {
                    std::string content(text);
                    // If it contains moves (1. e4), assume PGN, else FEN
                    // Very naive check
                    if (content.find("1.") != std::string::npos || content.find("[Event") != std::string::npos) {
                        board.loadPgn(content);
                    } else {
                        // Assume FEN
                         board.loadFen(content);
                    }
                    UnloadFileText(text);
                    
                    // Reset game state
                    gameOver = false;
                    gameStatus = "";
                }
            }
            UnloadDroppedFiles(droppedFiles);
        }

        if (!gameOver) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePos = GetMousePosition();
                // Basic coordinate mapping
                if (mousePos.x >= BOARD_OFFSET_X && mousePos.x < BOARD_OFFSET_X + BOARD_SIZE &&
                    mousePos.y >= BOARD_OFFSET_Y && mousePos.y < BOARD_OFFSET_Y + BOARD_SIZE) {
                    
                    int file = (mousePos.x - BOARD_OFFSET_X) / SQUARE_SIZE;
                    int rank = 7 - (int)((mousePos.y - BOARD_OFFSET_Y) / SQUARE_SIZE);
                    int sq = rank * 8 + file;

                    static int selectedSq = -1;
                    
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
                        
                        // Check promotion (simplified: always Queen for now if pawn reaches end)
                        Chess::Piece p = board.getPiece(selectedSq);
                        if (Chess::typeOf(p) == Chess::PAWN) {
                            int r = sq / 8;
                            if (r == 0 || r == 7) m.promotion = Chess::QUEEN;
                        }

                        // Validate against legal moves
                        std::vector<Chess::Move> legal = board.getLegalMoves();
                        bool isLegal = false;
                        for (const auto& lm : legal) {
                            if (lm.from == m.from && lm.dest == m.dest) {
                                m = lm; // Copy correct promotion if needed
                                isLegal = true;
                                break;
                            }
                        }

                        if (isLegal) {
                            board.makeMove(m);
                            selectedSq = -1;
                            
                            // Check Game End
                            if (board.isCheckmate()) {
                                gameOver = true;
                                gameStatus = (board.getTurn() == Chess::White ? "Black Wins (Checkmate)" : "White Wins (Checkmate)");
                            } else if (board.isStalemate()) {
                                gameOver = true;
                                gameStatus = "Draw (Stalemate)";
                            } else if (board.isInsufficientMaterial()) {
                                gameOver = true;
                                gameStatus = "Draw (Insufficient Material)";
                            } else if (board.isDraw()) {
                                gameOver = true;
                                gameStatus = "Draw (50-move rule)";
                            }
                        } else {
                            // If clicked on own piece, change selection
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
        } else {
            // Restart on click?
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                board.reset();
                gameOver = false;
                gameStatus = "";
            }
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw Board
        for (int r = 0; r < 8; r++) {
            for (int f = 0; f < 8; f++) {
                Color c = ((r + f) % 2 == 0) ? COLOR_DARK : COLOR_LIGHT; 
                int drawX = BOARD_OFFSET_X + f * SQUARE_SIZE;
                int drawY = BOARD_OFFSET_Y + (7 - r) * SQUARE_SIZE;
                DrawRectangle(drawX, drawY, SQUARE_SIZE, SQUARE_SIZE, c);
                
                // Highlight last move or check? (TODO)
            }
        }

        // Draw Pieces
        for (int i = 0; i < 64; i++) {
            Chess::Piece p = board.getPiece(i);
            if (p != Chess::NO_PIECE) {
                int r = i / 8;
                int f = i % 8;
                int drawX = BOARD_OFFSET_X + f * SQUARE_SIZE;
                int drawY = BOARD_OFFSET_Y + (7 - r) * SQUARE_SIZE;
                
                // Debug Text for pieces
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
                DrawText(text, drawX + 25, drawY + 20, 40, textColor);
            }
        }
        
        // Game Over Overlay
        if (gameOver) {
            DrawRectangle(0, SCREEN_HEIGHT / 2 - 50, SCREEN_WIDTH, 100, Fade(BLACK, 0.8f));
            int textWidth = MeasureText(gameStatus.c_str(), 40);
            DrawText(gameStatus.c_str(), (SCREEN_WIDTH - textWidth) / 2, SCREEN_HEIGHT / 2 - 20, 40, RED);
            
            const char* sub = "Click to Restart";
            int subWidth = MeasureText(sub, 20);
            DrawText(sub, (SCREEN_WIDTH - subWidth) / 2, SCREEN_HEIGHT / 2 + 30, 20, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
