# Chess Analysis App (Native C++)

A high-performance, native desktop chess application built with C++17 and [Raylib](https://www.raylib.com/), integrated with the Stockfish chess engine.

## Features

- **Core Chess Engine**:
  - Complete implementation of chess rules (Castling, En Passant, Promotion).
  - Legal move generation and validation.
  - Game End Detection: Checkmate, Stalemate, Insufficient Material, 50-Move Rule.
- **Native GUI with Modern Aesthetics**:
  - Fast, hardware-accelerated rendering using Raylib.
  - Custom dark theme UI (IntelliJ/Lichess inspired).
  - High-res piece textures (128x128px PNGs with built-in sub-sampling and square fitting).
  - Interactive board with slick click-to-move animations.
- **Advanced Stockfish Integration**:
  - Automated analysis of current board positions using the Stockfish engine.
  - Vertical dynamic evaluation bar tied directly to Stockfish's real-time centipawn/mate score.
  - Display of numerical evaluation and best move suggestions in the side panel.
- **Comprehensive Game Controls**:
  - `Flip Board` and `Reload` controls for board manipulation.
  - Interactive media playback buttons (`|<`, `<`, `||`, `>`, `>|`) to traverse the game history.
  - A scrollable "Move Table" displaying standard algebraic notation (SAN).
- **File & Input Support**:
  - **PGN Loading**: Drag & drop `.pgn` files to parse and replay games automatically.
  - **FEN Loading**: Drag & drop `.fen` files or text files containing FEN strings to load positions.
  - **Paste Dialog**: Advanced "Paste PGN" overlay dialog with preview wrapping. Supports standard `Ctrl+V` clipboard pasting.

## Platform Support

**Windows Only**
This application currently relies on the Win32 API (`CreateProcess`, pipes) for communicating with the Stockfish engine. Linux and macOS are not currently supported.

## Prerequisites

- **CMake** (3.20 or newer)
- **C++ Compiler** with C++17 support (MSVC recommended for Windows).
- **Stockfish Executable**: You must have `stockfish.exe` available. A compatible binary is provided in the `stockfish/` directory.

## Build Instructions

1. **Clone the repository**:

    ```bash
    git clone https://github.com/yourusername/Chess-Engine.git
    cd Chess-Engine
    ```

2. **Navigate to the app source**:

    ```bash
    cd chess-analysis-app
    ```

3. **Configure**:

    ```bash
    cmake -S . -B build
    ```

    *Note: Raylib will be automatically fetched and built.*

4. **Build**:

    ```bash
    cmake --build build --config Release
    ```

5. **Setup Stockfish**:
    Copy `stockfish-windows-x86-64-avx2.exe` from the `../stockfish/` directory (or your own `stockfish.exe`) to the directory where the built executable is located (e.g., `chess-analysis-app/build/Release/`) and rename it to `stockfish.exe`.

6. **Run**:
    Execute `ChessApp.exe` from the build directory.

## Testing

The project includes a suite of unit tests for the core chess logic to guarantee robustness (including Perft testing).

1. **Build Tests**:

    ```bash
    cmake --build build --config Release --target ChessTests
    ```

2. **Run Tests**:

    ```bash
    .\build\Release\ChessTests.exe
    ```

## Controls

- **Left Click**: Select a piece / Move to a valid square.
- **Click on Own Piece**: Reselect different piece.
- **Drag & Drop**: Drop a `.pgn` or `.fen` file onto the window to load it.
- **Arrow Keys**:
  - `Right`: Next move in loaded game record.
  - `Left`: Previous move (undo).
- **F11**: Toggle Fullscreen.
- **Scroll Wheel**: Scroll up and down inside the Move History table.
- **Paste PGN Dialog**: Click "Paste PGN", then press `Ctrl+V` to load text from your clipboard. Hit "Analyze" to load it onto the board.
- **Media Controls**: Navigate forwards, backwards, to start, or to end of move list.

## Project Structure

- `chess-analysis-app/src/core/`: Chess logic (Board representation, Move generation, Rules).
  - `board.hpp`: Header-only implementation of the Board logic.
  - `move_gen.cpp`: Move generation logic.
  - `types.hpp`: Basic types (Bitboard, Piece, Square).
- `chess-analysis-app/src/engine/`: Stockfish integration.
  - `stockfish.cpp`: Win32 process management for the engine.
- `chess-analysis-app/src/main.cpp`: Entry point, Game Loop, and Raylib GUI rendering.
- `chess-analysis-app/CMakeLists.txt`: Build configuration.
