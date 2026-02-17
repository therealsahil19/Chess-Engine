# Native C++ Chess Analysis App

A high-performance, native desktop chess application built with C++17 and [Raylib](https://www.raylib.com/).

## Features

- **Core Chess Engine**:
  - Complete implementation of chess rules (Castling, En Passant, Promotion).
  - Legal move generation and validation.
  - Game End Detection: Checkmate, Stalemate, Insufficient Material, 50-Move Rule.
- **Native GUI**:
  - Fast, hardware-accelerated rendering using Raylib.
  - Interactive board with click-to-move functionality.
- **File Support**:
  - **PGN Loading**: Drag & drop `.pgn` files to parse and replay games automatically.
  - **FEN Loading**: Drag & drop `.fen` files or text files containing FEN strings to load positions.
  - **Drag & Drop**: Native OS file drag and drop integration.

## Prerequisites

- **CMake** (3.20 or newer)
- **C++ Compiler** with C++17 support (MSVC, GCC, or Clang)
- **Raylib**: Automatically fetched and built via CMake.

## Build Instructions

1.  **Configure**:
    ```bash
    cmake -S . -B build
    ```

2.  **Build**:
    ```bash
    cmake --build build --config Release
    ```

3.  **Run**:
    The executable will be located in `build/Release/ChessApp.exe` (Windows) or `build/ChessApp` (Linux/Mac).

## Controls

-   **Left Click**: Select a piece / Move to a valid square.
-   **Click on Own Piece**: Reselect different piece.
-   **Drag & Drop**: Drop a `.pgn` or `.fen` file onto the window to load it.
-   **Restart**: Click on the board when game is over to reset (or re-launch app).

## Project Structure

-   `src/core/`: Chess logic (Board representation, Move generation, Rules).
    -   `board.hpp`: Header-only implementation of the Board logic (to resolve linking issues).
    -   `move_gen.cpp`: Move generation logic.
    -   `types.hpp`: Basic types (Bitboard, Piece, Square).
-   `src/main.cpp`: Entry point, Game Loop, and Raylib GUI rendering.
-   `CMakeLists.txt`: Build configuration.
