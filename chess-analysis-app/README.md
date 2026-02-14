# Chess Analysis App - Frontend

This is the React-based frontend for the Chess Analysis App. It uses Vite for development and building.

## Development

All commands must be run from within the `chess-analysis-app` directory.

To start the development server:

```bash
npm install
npm run dev
```

### Important Note on Asset Loading

The application uses `import.meta.env.BASE_URL` to resolve paths for static assets like the Stockfish Web Worker (`stockfish.js`). This ensures that the worker loads correctly both in local development (root path `/`) and when deployed to GitHub Pages (subdirectory path `/Chess-Engine/`).

## Available Scripts

- `npm run dev`: Starts the development server.
- `npm run build`: Builds the app for production.
- `npm run lint`: Runs ESLint to check for code quality.
- `npm run preview`: Previews the production build locally.

## Project Structure

- `src/`: Contains the React source code.
  - `App.jsx`: Main application component. Handles game state, navigation, and integrates the UI with the engine.
  - `engine.js`: Wrapper class for the Stockfish WASM worker. Manages UCI communication.
  - `uci-parser.js`: Utility to parse UCI output strings (info depth, score, pv, etc.).
  - `App.css`: Application-specific styles.
- `public/`: Static assets served directly.
  - `stockfish.js`: The Stockfish chess engine compiled to WebAssembly (WASM).
  - `stockfish.wasm`: The accompanying WASM binary.
- `eslint.config.js`: ESLint configuration using the new Flat Config system.
- `OPTIMIZATION_LOG.md`: A record of performance improvements and architectural decisions (e.g., PGN loading optimization).

For more information about the project features and roadmap, please refer to the [root README](../README.md).
