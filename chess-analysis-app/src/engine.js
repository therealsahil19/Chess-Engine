import { parseInfo } from './uci-parser';

export default class Engine {
  constructor(onMessage) {
    const workerPath = `${import.meta.env.BASE_URL}stockfish.js`;
    this.stockfish = new Worker(workerPath);
    this.latestInfo = {};

    this.stockfish.onmessage = (event) => {
      const line = event.data;

      // Parse info lines for depth, score, pv
      if (line.startsWith('info') && line.includes('score')) {
        this.latestInfo = parseInfo(line);
      }

      // When bestmove is returned, the analysis for this position is done
      if (line.startsWith('bestmove')) {
        const bestMove = line.split(' ')[1];
        if (this.onResult) {
            this.onResult({
                bestMove,
                ...this.latestInfo
            });
            this.onResult = null; // Clear callback
        }
      }

      if (onMessage) {
        onMessage(line);
      }
    };

    this.stockfish.postMessage("uci");
  }

  analyzePosition(fen, depth, callback) {
    this.latestInfo = {};
    this.onResult = callback;
    this.stockfish.postMessage(`position fen ${fen}`);
    this.stockfish.postMessage(`go depth ${depth}`);
  }

  analyzePositionTime(fen, time, callback) {
    this.latestInfo = {};
    this.onResult = callback;
    this.stockfish.postMessage(`position fen ${fen}`);
    this.stockfish.postMessage(`go movetime ${time}`);
  }

  stop() {
    this.stockfish.postMessage("stop");
  }

  quit() {
    this.stockfish.postMessage("quit");
  }

  sendCommand(command) {
    this.stockfish.postMessage(command);
  }
}
