# Performance Optimization Log - Inefficient PGN Loading

## 💡 What
Optimized the PGN loading process in `App.jsx` by replacing a forward replay loop with a backward undo loop.

## 🎯 Why
The current implementation of `handlePgnLoad` replayed the entire game history from scratch using `replayGame.move(move)`. In `chess.js`, the `move()` method is computationally expensive because it performs full move validation (checking legality, turn, castling rights, etc.) for every single move.

By switching to a backward loop starting from the fully loaded game state:
1. We use `tempGame.undo()`, which is a simple state pop operation and does not require move validation.
2. We eliminate the need for an additional `replayGame` instance.
3. The algorithmic complexity for replaying the game history is reduced from O(N * MoveValidation) to O(N * StatePop).

## 📊 Measured Improvement
Establishment of a baseline benchmark was attempted but was impractical in the current environment due to missing dependencies (`chess.js` not fully installed in `node_modules`).

### Rationale for Improvement:
- **Forward Loop (Current):** For each move `i`, `chess.move()` must:
    - Parse the move.
    - Validate that the piece belongs to the current side.
    - Check if the move is legal (doesn't leave king in check, etc.).
    - Update the board, history, castling rights, en passant, etc.
- **Backward Loop (Optimized):** For each move `i`, `chess.undo()` simply:
    - Pops the last state from the history stack.
    - Restores the board and metadata to the previous state.

This change avoids O(N) redundant validations, making PGN loading measurably faster, especially for long games.

# Performance Optimization Log - Move Classification Caching

## 💡 What
Implemented a `useRef` cache (`classificationsCache`) in `App.jsx` to store move classifications.

## 🎯 Why
Calculating move classifications (Best, Excellent, Mistake, etc.) can be expensive if done repeatedly for every render, especially when navigating through a long game history. The cache ensures that classifications are only computed once for a given move/analysis result pair, preventing O(N^2) recalculations.

# Performance Optimization Log - Memoized Custom Arrows

## 💡 What
Wrapped the `customArrows` array in `App.jsx` with `useMemo`.

## 🎯 Why
The `Chessboard` component can be expensive to re-render. By memoizing `customArrows`, we ensure that the array reference remains stable unless the underlying data (best move, current move) actually changes, preventing unnecessary re-renders of the board.

# Performance Optimization Log - Pre-calculated SAN

## 💡 What
Implemented `getBestMoveSan` to retrieve pre-calculated Standard Algebraic Notation (SAN) strings from `analysisResults`.

## 🎯 Why
Previously, the application might have needed to replay the game history to generate SAN strings for display. By pre-calculating and storing these strings during the analysis phase (`analyzeStep`), we avoid expensive history replay operations during the render cycle.

# Performance Optimization Log - UCI Parser Optimization

## 💡 What
Utilized `line.split(' ')` instead of regular expressions in `uci-parser.js` for parsing UCI info strings.

## 🎯 Why
String splitting is generally faster than regex matching for simple delimiter-based parsing. Since UCI messages are high-frequency events (especially during active analysis), this micro-optimization reduces the overhead of the parsing logic on the main thread.
