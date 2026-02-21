# Chess Analysis App (Native C++)

A high-performance, native desktop chess application built with C++17 and [Raylib](https://www.raylib.com/), integrated with the Stockfish chess engine.

## Features

- **Core Chess Engine**:
  - Complete implementation of chess rules (Castling, En Passant, Promotion).
  - Legal move generation and validation.
  - Game End Detection: Checkmate, Stalemate, Insufficient Material, 50-Move Rule.
- **Native GUI**:
  - Fast, hardware-accelerated rendering using Raylib.
  - Interactive board with click-to-move functionality.
  - Real-time move animation.
- **Stockfish Integration**:
  - Automated analysis of current board positions using the Stockfish engine.
  - Display of evaluation (cp/mate) and best move suggestions.
- **File Support**:
  - **PGN Loading**: Drag & drop `.pgn` files to parse and replay games automatically.
  - **FEN Loading**: Drag & drop `.fen` files or text files containing FEN strings to load positions.
  - **Clipboard Support**: "Paste PGN" button to load games directly from clipboard.

## Platform Support

**Windows Only**
This application currently relies on the Win32 API (`CreateProcess`, pipes) for communicating with the Stockfish engine. Linux and macOS are not currently supported.

## Prerequisites

- **CMake** (3.20 or newer)
- **C++ Compiler** with C++17 support (MSVC recommended for Windows).
- **Stockfish Executable**: You must have `stockfish.exe` available. A compatible binary is provided in the `stockfish/` directory.

## Build Instructions

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/yourusername/Chess-Engine.git
    cd Chess-Engine
    ```

2.  **Navigate to the app source**:
    ```bash
    cd chess-analysis-app
    ```

3.  **Configure**:
    ```bash
    cmake -S . -B build
    ```
    *Note: Raylib will be automatically fetched and built.*

4.  **Build**:
    ```bash
    cmake --build build --config Release
    ```

5.  **Setup Stockfish**:
    Copy `stockfish-windows-x86-64-avx2.exe` from the `../stockfish/` directory (or your own `stockfish.exe`) to the directory where the built executable is located (e.g., `chess-analysis-app/build/Release/`) and rename it to `stockfish.exe`.

6.  **Run**:
    Execute `ChessApp.exe` from the build directory.

## Controls

-   **Left Click**: Select a piece / Move to a valid square.
-   **Click on Own Piece**: Reselect different piece.
-   **Drag & Drop**: Drop a `.pgn` or `.fen` file onto the window to load it.
-   **Arrow Keys**:
    -   `Right`: Next move in loaded game record.
    -   `Left`: Previous move (undo).
-   **F11**: Toggle Fullscreen.
-   **Paste PGN Button**: Click to load game from clipboard.

## Project Structure

-   `chess-analysis-app/src/core/`: Chess logic (Board representation, Move generation, Rules).
    -   `board.hpp`: Header-only implementation of the Board logic.
    -   `move_gen.cpp`: Move generation logic.
    -   `types.hpp`: Basic types (Bitboard, Piece, Square).
-   `chess-analysis-app/src/engine/`: Stockfish integration.
    -   `stockfish.cpp`: Win32 process management for the engine.
-   `chess-analysis-app/src/main.cpp`: Entry point, Game Loop, and Raylib GUI rendering.
-   `chess-analysis-app/CMakeLists.txt`: Build configuration.
