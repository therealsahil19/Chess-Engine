# Chess Engine & Analysis App

A modern, web-based chess analysis application that leverages Stockfish (WASM) to provide deep insights into your games.

> **Note:** This application is optimized for Desktop/PC browsers. Mobile responsiveness is not currently supported.

## Features

- **Interactive Chessboard**: Smooth piece movement and board interaction using `react-chessboard`.
- **Engine Analysis**: Powered by `stockfish.js` running in a Web Worker for high performance without blocking the UI.
- **Move Classification**: Automatically evaluates moves based on centipawn loss (cp):
    - ★ **Best**: The engine's top choice (0 cp loss).
    - **Excellent**: Near-perfect move (≤ 20 cp loss).
    - **Good**: Solid move (≤ 50 cp loss).
    - **Inaccuracy (?! )**: Slight mistake that drops some advantage (≤ 100 cp loss).
    - **Mistake (?)**: Significant drop in evaluation (≤ 300 cp loss).
    - **Blunder (??)**: Critical error that changes the game state significantly (> 300 cp loss).
- **Analysis Panel**: Detailed engine feedback including:
    - **Evaluation**: Current score in centipawns or mate distance.
    - **Bound Display**: Indicates if the score is a lower (≥) or upper (≤) bound, useful during partial search depths.
    - **Depth**: The current search depth of the engine.
    - **Principal Variation (PV)**: The best line of play calculated by the engine.
- **Reasoning Modes**:
    - **Low Reasoning (Fast)**: Quick evaluation at depth 12.
    - **High Reasoning (Deep)**: Thorough analysis at depth 18.
    - **Custom**: Specify your desired analysis depth (1-30).
- **PGN Support**: Load games from PGN strings for full analysis. Includes a collapsible input section.
- **Game Navigation**: Easily navigate through moves with start, back, forward, and end controls.
- **Visual Feedback**: Best move arrows and move-by-move evaluation badges.
- **Feedback Panel**: Displays the engine's best move suggestion when a Mistake or Blunder is played, helping users learn from their errors.
- **Evaluation Bar**: Vertical bar showing the current advantage, visually representing who is winning.
- **3-Column Layout**: Optimized UI with Evaluation Bar, Chessboard, and Analysis Panel side-by-side for a comprehensive view.
- **Bound Display**: Clearly indicates if the score is a lower (≥) or upper (≤) bound relative to the side to move, providing context during partial search depths.

## Architecture Overview

The application is built with a clear separation of concerns:

- **`App.jsx`**: The main React component that handles application state (game history, analysis results, UI interaction). It manages the `chess.js` game instance and orchestrates the analysis flow. Optimized PGN loading uses a backward undo loop (O(N) * StatePop) to bypass expensive move validation, significantly improving performance for long games.
- **`engine.js`**: A wrapper class for the `stockfish.js` Web Worker. It handles communication (sending UCI commands, receiving messages) and keeps the UI thread responsive. It implements lazy parsing, only invoking the parser when necessary.
- **`uci-parser.js`**: A utility module dedicated to parsing raw UCI (Universal Chess Interface) messages from the engine. It extracts structured data like score, depth, bounds, and PV lines, using defensive programming to handle malformed inputs.
- **`verification/`**: Contains Python scripts (like `verify_bounds.py`) using Playwright for end-to-end verification of engine logic and UI elements.

## Technologies Used

- **React** (v19): Frontend framework.
- **Vite** (v7): Build tool and dev server.
- **chess.js**: Move validation and game logic.
- **react-chessboard**: Interactive UI component.
- **stockfish.js**: WASM version of the Stockfish chess engine.
- **ESLint**: Flat Config system (`eslint.config.js`) for code linting.

## Known Issues

- **Headless Mode Stability**: The `stockfish.js` engine worker may exhibit instability or fail to respond to subsequent commands after the first move when running in headless environments (like Playwright CI). Verification scripts may require a graphical environment or specific configuration adjustments.

## Getting Started

### Prerequisites

- Node.js (v18 or higher recommended)
- npm or yarn

### Installation

1. Clone the repository.
2. Navigate to the app directory:
   ```bash
   cd chess-analysis-app
   ```
3. Install dependencies:
   ```bash
   npm install
   ```

### Running Locally

To start the development server:
```bash
npm run dev
```

### Building for Production

To create a production build:
```bash
npm run build
```

## Deployment

The project is configured to automatically deploy to GitHub Pages using GitHub Actions.

- **Workflow**: `.github/workflows/deploy.yml`
- **Trigger**: Pushes to the `main` branch.
- **Process**: Builds the React application and uploads the `chess-analysis-app/dist` artifact to the `gh-pages` environment.

## Future Roadmap

- [ ] "Brilliant" move classification logic.
- [ ] Multi-PV (multiple principal variations) support.
- [x] Evaluation bar visualization.
- [ ] Opening book integration.
- [x] GitHub Actions deployment configuration.

## License

MIT
