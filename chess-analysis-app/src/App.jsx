import { useState, useEffect, useMemo } from 'react';
import { Chess } from 'chess.js';
import { Chessboard } from 'react-chessboard';
import Engine from './engine';
import './App.css';

// Helper functions
const getNormScore = (res, index) => {
  if (!res) return 0;
  let val = res.scoreVal;
  if (res.scoreType === 'mate') {
    val = (val > 0 ? 10000 : -10000) - (val * 100);
  }
  const modifier = (index + 1) % 2 === 0 ? 1 : -1;
  return val * modifier;
};

const computeClassification = (index, moveHistory, analysisResults) => {
  if (index === -1) return null; // Start pos
  const prevRes = analysisResults[index - 1];
  const currRes = analysisResults[index];
  if (!prevRes || !currRes) return null;

  const playedMoveObj = moveHistory[index];
  const playedMoveLan = playedMoveObj.from + playedMoveObj.to + (playedMoveObj.promotion || '');
  if (playedMoveLan === prevRes.bestMove) return { label: 'Best', className: 'class-best' };

  const prevScore = getNormScore(prevRes, index - 1);
  const currScore = getNormScore(currRes, index);

  let loss = 0;
  if (playedMoveObj.color === 'w') {
    loss = prevScore - currScore;
  } else {
    loss = currScore - prevScore;
  }

  if (loss <= 0) return { label: 'Best', className: 'class-best' };
  if (loss <= 20) return { label: 'Excellent', className: 'class-excellent' };
  if (loss <= 50) return { label: 'Good', className: 'class-good' };
  if (loss <= 100) return { label: 'Inaccuracy', className: 'class-inaccuracy' };
  if (loss <= 300) return { label: 'Mistake', className: 'class-mistake' };
  return { label: 'Blunder', className: 'class-blunder' };
};

function App() {
  const [game, setGame] = useState(new Chess());
  const [fen, setFen] = useState(game.fen());
  const [pgnInput, setPgnInput] = useState('');
  const [engine, setEngine] = useState(null);

  // Navigation State
  const [moveHistory, setMoveHistory] = useState([]);
  const [currentMoveIndex, setCurrentMoveIndex] = useState(-1);

  // Analysis State
  const [isAnalyzing, setIsAnalyzing] = useState(false);
  const [analysisProgress, setAnalysisProgress] = useState(0);
  const [analysisResults, setAnalysisResults] = useState({});
  const [analysisMode, setAnalysisMode] = useState('low');
  const [customDepth, setCustomDepth] = useState(15);
  const [isPgnCollapsed, setIsPgnCollapsed] = useState(true);

  // Initialize Engine
  useEffect(() => {
    const newEngine = new Engine();
    setEngine(newEngine);
    return () => newEngine.quit();
  }, []);

  useEffect(() => {
    setFen(game.fen());
  }, [game]);

  const onDrop = (sourceSquare, targetSquare) => {
    const gameCopy = new Chess(game.fen());
    let move = null;
    try {
      move = gameCopy.move({ from: sourceSquare, to: targetSquare, promotion: 'q' });
    } catch { /* invalid move */ }

    if (move) {
        setGame(gameCopy);
        const newHistory = [...moveHistory.slice(0, currentMoveIndex + 1), { ...move, fen: gameCopy.fen() }];
        setMoveHistory(newHistory);
        setCurrentMoveIndex(prev => prev + 1);

        return true;
    }
    return false;
  };

  const handlePgnLoad = () => {
    try {
      const tempGame = new Chess();
      tempGame.loadPgn(pgnInput);
      const rawHistory = tempGame.history({ verbose: true });

      const history = new Array(rawHistory.length);
      for (let i = rawHistory.length - 1; i >= 0; i--) {
        history[i] = { ...rawHistory[i], fen: tempGame.fen() };
        tempGame.undo();
      }

      setMoveHistory(history);
      const newGame = new Chess();
      setGame(newGame);

      setCurrentMoveIndex(-1);
      setAnalysisResults({});
      setIsAnalyzing(false);
      setIsPgnCollapsed(true);
    } catch {
      alert('Invalid PGN');
    }
  };

  const handleReset = () => {
    setGame(new Chess());
    setMoveHistory([]);
    setCurrentMoveIndex(-1);
    setPgnInput('');
    setAnalysisResults({});
    setIsAnalyzing(false);
  };

  // Navigation
  const goToStart = () => { jumpToMove(-1); };
  const goBack = () => {
    if (currentMoveIndex >= 0) {
        jumpToMove(currentMoveIndex - 1);
    }
  };
  const goForward = () => {
    if (currentMoveIndex < moveHistory.length - 1) {
        jumpToMove(currentMoveIndex + 1);
    }
  };
  const goToEnd = () => {
    jumpToMove(moveHistory.length - 1);
  };
  const jumpToMove = (index) => {
    let newGame;
    if (index === -1) {
      newGame = new Chess();
    } else {
      newGame = new Chess(moveHistory[index].fen);
    }
    setGame(newGame);
    setCurrentMoveIndex(index);
  };

  // Analysis Logic
  const startAnalysis = () => {
    if (!engine || moveHistory.length === 0) return;
    setIsAnalyzing(true);
    setAnalysisResults({});
    setAnalysisProgress(0);
    let depth = 15;
    if (analysisMode === 'low') depth = 12;
    if (analysisMode === 'high') depth = 18;
    if (analysisMode === 'custom') depth = customDepth;

    // Pass a fresh game instance for incremental analysis
    analyzeStep(-1, depth, new Chess());
  };

  const analyzeStep = (index, depth, gameInstance) => {
    // Apply the move for the current index if applicable
    // This incrementally updates the game state instead of replaying from start
    if (index >= 0) {
      gameInstance.move(moveHistory[index]);
    }
    const fen = gameInstance.fen();

    engine.analyzePosition(fen, depth, (result) => {
        if (result.bestMove) {
            try {
                const move = gameInstance.move({
                    from: result.bestMove.substring(0, 2),
                    to: result.bestMove.substring(2, 4),
                    promotion: result.bestMove.length > 4 ? result.bestMove[4] : 'q'
                });
                result.bestMoveSan = move.san;
                gameInstance.undo();
            } catch {
                result.bestMoveSan = result.bestMove;
            }
        }

        setAnalysisResults(prev => ({ ...prev, [index]: result }));
        const nextIndex = index + 1;
        setAnalysisProgress(Math.round(((index + 2) / (moveHistory.length + 1)) * 100));
        if (nextIndex < moveHistory.length) {
            analyzeStep(nextIndex, depth, gameInstance);
        } else {
            setIsAnalyzing(false);
        }
    });
  };


  const getBestMoveSan = (index) => {
      const res = analysisResults[index - 1];
      if (!res || !res.bestMove) return null;
      return res.bestMoveSan || res.bestMove;
  };

  const getEvalData = () => {
    const res = analysisResults[currentMoveIndex];
    if (!res) return { percentage: 50, text: '0.0', side: 'top' };

    let score = res.scoreVal;
    if (res.scoreType === 'mate') {
      // Logic: if it's black to move (even index+1, or currentMoveIndex is even), and score > 0, white is mating.
      const sideToMove = (currentMoveIndex + 1) % 2 === 0 ? 'w' : 'b';
      const whiteScore = sideToMove === 'w' ? score : -score;
      return {
        percentage: whiteScore > 0 ? 100 : 0,
        text: `M${Math.abs(score)}`,
        side: whiteScore > 0 ? 'bottom' : 'top'
      };
    }

    const sideToMove = (currentMoveIndex + 1) % 2 === 0 ? 'w' : 'b';
    const whiteScore = sideToMove === 'w' ? score : -score;

    let percentage = 50 + (whiteScore / 10);
    percentage = Math.max(5, Math.min(95, percentage));

    return {
      percentage,
      text: (Math.abs(whiteScore) / 100).toFixed(1),
      side: whiteScore >= 0 ? 'bottom' : 'top'
    };
  };

  const moveClassifications = useMemo(() => {
    const classifications = {};
    moveHistory.forEach((_, index) => {
      classifications[index] = computeClassification(index, moveHistory, analysisResults);
    });
    return classifications;
  }, [moveHistory, analysisResults]);

  const currentClassification = moveClassifications[currentMoveIndex];
  const bestMoveSuggestion = (currentClassification && (currentClassification.label === 'Mistake' || currentClassification.label === 'Blunder'))
      ? getBestMoveSan(currentMoveIndex) : null;

  const evalData = getEvalData();

  // Arrows for Best Move
  const arrows = useMemo(() => {
    const currentAnalysis = analysisResults[currentMoveIndex];
    if (currentAnalysis && currentAnalysis.bestMove) {
        const from = currentAnalysis.bestMove.substring(0, 2);
        const to = currentAnalysis.bestMove.substring(2, 4);
        return [[from, to]];
    }
    return [];
  }, [analysisResults, currentMoveIndex]);

  // Move grouping for table
  const moveRows = useMemo(() => {
    const rows = [];
    for (let i = 0; i < moveHistory.length; i += 2) {
      rows.push({
        num: Math.floor(i / 2) + 1,
        white: { move: moveHistory[i], index: i },
        black: moveHistory[i+1] ? { move: moveHistory[i+1], index: i+1 } : null
      });
    }
    return rows;
  }, [moveHistory]);

  return (
    <main className="app-container">
      <aside className="evaluation-bar-container" aria-label="Evaluation Bar">
        <div
          className="evaluation-bar-fill"
          style={{ height: `${evalData.percentage}%` }}
        />
        <span className={`evaluation-text ${evalData.side}`}>{evalData.text}</span>
      </aside>

      <section className="board-container">
        <Chessboard
            position={fen}
            onPieceDrop={onDrop}
            customArrows={arrows}
            customArrowColor="rgba(129, 182, 76, 0.8)"
            boardOrientation="white"
        />

        <div className="navigation-controls">
            <button onClick={goToStart} disabled={currentMoveIndex === -1} aria-label="Go to start">«</button>
            <button onClick={goBack} disabled={currentMoveIndex === -1} aria-label="Go back">‹</button>
            <button onClick={goForward} disabled={currentMoveIndex === moveHistory.length - 1} aria-label="Go forward">›</button>
            <button onClick={goToEnd} disabled={currentMoveIndex === moveHistory.length - 1} aria-label="Go to end">»</button>
        </div>
      </section>

      <aside className="analysis-panel">
        <div className="panel-header">
          <h2>Analysis</h2>
        </div>

        {currentAnalysis && (
          <div className="stats-panel">
            <div className="stat-item">
              <span className="stat-label">Evaluation</span>
              <span className="stat-value">
                {currentAnalysis.bound && (
                  ((currentMoveIndex + 1) % 2 === 0)
                    ? (currentAnalysis.bound === 'lower' ? '≥ ' : '≤ ')
                    : (currentAnalysis.bound === 'lower' ? '≤ ' : '≥ ')
                )}
                {evalData.text} {currentAnalysis.scoreType === 'mate' ? '(Mate)' : ''}
              </span>
            </div>
            <div className="stat-item">
              <span className="stat-label">Depth</span>
              <span className="stat-value">{currentAnalysis.depth || 0}</span>
            </div>
            {currentAnalysis.pv && (
              <div className="pv-display" title="Principal Variation">
                {currentAnalysis.pv}
              </div>
            )}
          </div>
        )}

        <div className="move-history-container">
          <table className="move-table">
            <thead>
              <tr>
                <th>#</th>
                <th>White</th>
                <th>Black</th>
              </tr>
            </thead>
            <tbody>
              {moveRows.map((row) => {
                const whiteClass = moveClassifications[row.white.index];
                const blackClass = row.black ? moveClassifications[row.black.index] : null;

                return (
                  <tr key={row.num} className="move-row">
                    <td className="move-number">{row.num}.</td>
                    <td
                      className={`move-cell ${currentMoveIndex === row.white.index ? 'current' : ''}`}
                      onClick={() => jumpToMove(row.white.index)}
                    >
                      {row.white.move.san}
                      {whiteClass && (
                        <span className={`move-annotation ${whiteClass.className}`}>
                          {whiteClass.label === 'Best' ? '★' : ''}
                          {whiteClass.label === 'Inaccuracy' ? '?!' : ''}
                          {whiteClass.label === 'Mistake' ? '?' : ''}
                          {whiteClass.label === 'Blunder' ? '??' : ''}
                        </span>
                      )}
                    </td>
                    <td
                      className={`move-cell ${row.black && currentMoveIndex === row.black.index ? 'current' : ''}`}
                      onClick={() => row.black && jumpToMove(row.black.index)}
                    >
                      {row.black?.move.san}
                      {blackClass && (
                        <span className={`move-annotation ${blackClass.className}`}>
                          {blackClass.label === 'Best' ? '★' : ''}
                          {blackClass.label === 'Inaccuracy' ? '?!' : ''}
                          {blackClass.label === 'Mistake' ? '?' : ''}
                          {blackClass.label === 'Blunder' ? '??' : ''}
                        </span>
                      )}
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </div>

        <div className="settings-section">
          <div className="analysis-settings">
              <label>Mode:
                  <select value={analysisMode} onChange={(e) => setAnalysisMode(e.target.value)} disabled={isAnalyzing}>
                      <option value="low">Low Reasoning (Fast)</option>
                      <option value="high">High Reasoning (Deep)</option>
                      <option value="custom">Custom</option>
                  </select>
              </label>
              {analysisMode === 'custom' && (
                  <input
                      type="number"
                      value={customDepth}
                      onChange={(e) => setCustomDepth(parseInt(e.target.value))}
                      min="1" max="30"
                  />
              )}
              <button className="analyze-button" onClick={startAnalysis} disabled={isAnalyzing || moveHistory.length === 0}>
                  {isAnalyzing ? `Analyzing... ${analysisProgress}%` : 'Analyze Game'}
              </button>
          </div>
        </div>

        <div className="collapsible-section">
          <div className="collapsible-header" onClick={() => setIsPgnCollapsed(!isPgnCollapsed)}>
            <span>PGN / New Game</span>
            <span>{isPgnCollapsed ? '▼' : '▲'}</span>
          </div>
          {!isPgnCollapsed && (
            <div className="collapsible-content">
              <textarea
                  placeholder="Paste PGN here..."
                  value={pgnInput}
                  onChange={(e) => setPgnInput(e.target.value)}
                  rows={5}
              />
              <div className="button-group">
                  <button onClick={handlePgnLoad}>Load PGN</button>
                  <button onClick={handleReset}>Reset / New Game</button>
              </div>
            </div>
          )}
        </div>
      </aside>

      {bestMoveSuggestion && (
          <div className="feedback-panel" style={{ gridColumn: '2 / span 1' }}>
              <span className={`class-badge class-${currentClassification.label.toLowerCase()}`}>{currentClassification.label}</span>
              <span className="suggestion">Best was: {bestMoveSuggestion}</span>
          </div>
      )}
    </main>
  );
}

export default App;
