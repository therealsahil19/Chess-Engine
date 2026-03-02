# Chess Analysis App (Native C++)

A high-performance, native desktop chess application built with C++17 and [Raylib](https://www.raylib.com/), seamlessly integrated with the renowned Stockfish chess engine.

## Features

### Core Chess Engine
- **Robust Rules Implementation**: Complete support for core chess rules including Castling, En Passant, and Pawn Promotion.
- **Accurate Move Generation**: Fully verified pseudo-legal and legal move generation, backed by perft testing.
- **Game State Detection**: Native detection for Checkmate, Stalemate, and Insufficient Material draws.
- **FEN and PGN Parsing**: Native capability to parse Forsyth-Edwards Notation (FEN) state and Standard Algebraic Notation (SAN) for move logging.

### Native GUI with Modern Aesthetics
- **Hardware-Accelerated Rendering**: Fast and smooth UI powered by Raylib.
- **High-Contrast Dark Theme**: A sleek Lichess/IntelliJ-inspired dark UI.
- **High-Resolution Textures**: Utilizes 128x128px high-quality PNG textures for chess pieces, providing crisp visuals.
- **Interactive Board**: Intuitive click-to-move animations, board flipping, and piece selection highlighting.
- **Dynamic Board Flipping**: Toggleable board orientation for both White and Black perspectives.

### Advanced Stockfish Integration
- **Automated Deep Analysis**: Connects automatically to the `stockfish.exe` engine via standard Windows pipes.
- **Real-Time Evaluation Bar**: A dynamic visual evaluation bar tied directly to Stockfish's centipawn/mate score.
- **Best Move Indicators**: Displays numerical evaluation and best move suggestions directly in the right-side analysis panel.
- **Multi-Threaded**: Runs the engine synchronously in a background thread, ensuring smooth UI performance during deep calculations.

### Comprehensive Game Controls
- **Interactive Move Navigation**: On-screen playback controls (`|<`, `<`, `||`, `>`, `>|`) to effortlessly step forwards and backwards through game history.
- **Move History Table**: A fully scrollable "Move Table" displaying the standard algebraic notation (SAN) for both players.
- **Keyboard Shortcuts**: Arrow keys for quick move undo/redo, and `F11` for fullscreen mode.

### Advanced Input & File Support
- **Drag & Drop Loading**: Drag a `.pgn` or `.fen` file straight into the application window to load game history or positions.
- **Intelligent Paste Dialog**: A dedicated "Paste PGN" overlay. Use standard `Ctrl+V` to parse PGN/FEN strings from your clipboard directly into the application with wrapped text previews.

## Platform Support

**Windows Only**
This application specifically relies on the Win32 API (`CreateProcess`, `CreatePipe`, `SECURITY_ATTRIBUTES`) for managing and communicating with the external Stockfish engine child process.

## Prerequisites

- **CMake** (3.20 or newer)
- **C++ Compiler** with C++17 support (MSVC recommended for Windows).
- **Stockfish Executable**: You must have `stockfish.exe` available. A compatible binary should be provided in the project root or `stockfish/` directory.

## Build Instructions

1. **Clone the repository**:
    ```bash
    git clone https://github.com/yourusername/Chess-Engine.git
    cd Chess-Engine
    ```

2. **Navigate to the application source**:
    ```bash
    cd chess-analysis-app
    ```

3. **Configure the build using CMake**:
    ```bash
    cmake -S . -B build
    ```
    *Note: Raylib will be automatically fetched and built from source via CMake FetchContent.*

4. **Build the application**:
    ```bash
    cmake --build build --config Release
    ```

5. **Setup Stockfish**:
    Ensure the `stockfish.exe` binary is available in the output directory (e.g., `chess-analysis-app/build/Release/`). A custom CMake post-build step automatically copies `stockfish.exe` to the target directory if it exists in the source root. The build step also copies the `textures/` directory.

6. **Run**:
    Execute `ChessApp.exe` from the built directory!

## Testing

The project incorporates a robust suite of unit tests for the core chess logic. Testing includes standard FEN setups, check states, material draw rules, and 'Perft' (Performance Test) node counting for move generation verification.

> **Important:** Please check the [tests.md](tests.md) document each time a test is made or updated. It contains vital information on current test coverage, missing coverage, and recommendations for test efficiency.

1. **Build Tests**:
    ```bash
    cmake --build build --config Release --target ChessTests
    ```

2. **Run Tests**:
    ```bash
    .\build\Release\ChessTests.exe
    ```

## Controls Overview

- **Left Click**: Select a piece / Move to a valid square / Interact with UI buttons.
- **Click on Own Piece**: Reselect different piece.
- **Drag & Drop**: Drop a `.pgn` or `.fen` file onto the window.
- **Arrow Keys**:
  - `Right`: Next move in the game record.
  - `Left`: Previous move (undo).
- **F11**: Toggle Fullscreen.
- **Scroll Wheel**: Scroll up and down inside the Move History table.
- **Paste PGN Dialog**: Click the "Paste PGN" button, press `Ctrl+V` to load text from your clipboard, and click "Analyze" to execute it.
- **Media Controls**: Navigate forwards, backwards, to start, or to the end of the move list directly from the GUI.

## Project Structure

- `chess-analysis-app/src/core/`: The foundational chess logic independent of any graphical or engine concerns.
  - `board.hpp` / `game_record.hpp`: Board representation, game history tracking, and piece movement.
  - `move_gen.cpp` / `move_utils.hpp`: Move generation, validation, and SAN/FEN conversion helpers.
  - `types.hpp` / `types.cpp`: Primitive chess types (Square, Move, Piece, Side).
- `chess-analysis-app/src/engine/`: Interface for external engine communication.
  - `stockfish.cpp` / `stockfish.hpp`: Win32 native child process management and standard I/O pipe reading.
- `chess-analysis-app/src/main.cpp`: The central entry point, rendering game loop (Raylib), and application state management.
- `chess-analysis-app/tests/`: Unit testing suite including `test_runner.cpp`.
- `chess-analysis-app/CMakeLists.txt`: Project definitions, FetchContent, and target building.
- `textures/`: High-resolution visual assets.
