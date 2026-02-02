# Chess Engine & Analysis App

A modern, web-based chess analysis application that leverages Stockfish (WASM) to provide deep insights into your games.

## Features

- **Interactive Chessboard**: Smooth piece movement and board interaction using `react-chessboard`.
- **Engine Analysis**: Powered by `stockfish.js` running in a Web Worker for high performance without blocking the UI.
- **Move Classification**: Automatically evaluates moves and classifies them as:
    - ★ **Best**: The engine's top choice.
    - **Excellent**: Near-perfect move.
    - **Good**: Solid move.
    - **Inaccuracy (?! )**: Slight mistake that drops some advantage.
    - **Mistake (?)**: Significant drop in evaluation.
    - **Blunder (??)**: Critical error that changes the game state significantly.
- **Reasoning Modes**:
    - **Low Reasoning (Fast)**: Quick evaluation at depth 12.
    - **High Reasoning (Deep)**: Thorough analysis at depth 18.
    - **Custom**: Specify your desired analysis depth.
- **PGN Support**: Load games from PGN strings for full analysis.
- **Game Navigation**: Easily navigate through moves with start, back, forward, and end controls.
- **Visual Feedback**: Best move arrows and move-by-move evaluation badges.

## Technologies Used

- **React**: Frontend framework.
- **Vite**: Build tool and dev server.
- **chess.js**: Move validation and game logic.
- **react-chessboard**: Interactive UI component.
- **stockfish.js**: WASM version of the Stockfish chess engine.

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

## Future Roadmap

- [ ] "Brilliant" move classification logic.
- [ ] Multi-PV (multiple principal variations) support.
- [ ] Evaluation bar visualization.
- [ ] Opening book integration.
- [x] GitHub Actions deployment configuration.

## License

MIT
