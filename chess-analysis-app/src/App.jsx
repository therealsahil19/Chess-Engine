import { useState, useEffect, useRef } from 'react';
import { Chess } from 'chess.js';
import { Chessboard } from 'react-chessboard';
import Engine from './engine';
import './App.css';

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

  // Initialize Engine
  useEffect(() => {
    const newEngine = new Engine();
    setEngine(newEngine);
    return () => newEngine.quit();
  }, []);

  useEffect(() => {
    setFen(game.fen());
  }, [game]);

  const safeGameMutate = (modify) => {
    setGame((g) => {
      const update = new Chess(g.fen());
      modify(update);
      return update;
    });
  };

  const onDrop = (sourceSquare, targetSquare) => {
    let move = null;
    safeGameMutate((g) => {
      try {
        move = g.move({ from: sourceSquare, to: targetSquare, promotion: 'q' });
      } catch(e) { /* invalid move */ }
    });

    if (move) {
        const newHistory = [...moveHistory.slice(0, currentMoveIndex + 1), move];
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
      const history = tempGame.history({ verbose: true });
      setMoveHistory(history);
      const newGame = new Chess();
      setGame(newGame);
      setCurrentMoveIndex(-1);
      setAnalysisResults({});
      setIsAnalyzing(false);
    } catch (error) {
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
  const goToStart = () => { setGame(new Chess()); setCurrentMoveIndex(-1); };
  const goBack = () => {
    if (currentMoveIndex >= 0) {
        safeGameMutate((g) => g.undo());
        setCurrentMoveIndex(prev => prev - 1);
    }
  };
  const goForward = () => {
    if (currentMoveIndex < moveHistory.length - 1) {
        const nextMove = moveHistory[currentMoveIndex + 1];
        safeGameMutate((g) => g.move(nextMove));
        setCurrentMoveIndex(prev => prev + 1);
    }
  };
  const goToEnd = () => {
    const newGame = new Chess();
    for (const move of moveHistory) newGame.move(move);
    setGame(newGame);
    setCurrentMoveIndex(moveHistory.length - 1);
  };
  const jumpToMove = (index) => {
    const newGame = new Chess();
    for(let j=0; j<=index; j++) newGame.move(moveHistory[j]);
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
    analyzeStep(-1, depth);
  };

  const analyzeStep = (index, depth) => {
    const tempGame = new Chess();
    for (let i = 0; i <= index; i++) tempGame.move(moveHistory[i]);
    const fen = tempGame.fen();

    engine.analyzePosition(fen, depth, (result) => {
        setAnalysisResults(prev => ({ ...prev, [index]: result }));
        const nextIndex = index + 1;
        setAnalysisProgress(Math.round(((index + 2) / (moveHistory.length + 1)) * 100));
        if (nextIndex < moveHistory.length) {
            analyzeStep(nextIndex, depth);
        } else {
            setIsAnalyzing(false);
        }
    });
  };

  // Classification Helpers
  const getNormScore = (res, index) => {
      if (!res) return 0;
      let val = res.scoreVal;
      if (res.scoreType === 'mate') {
          val = (val > 0 ? 10000 : -10000) - (val * 100);
      }
      const modifier = (index + 1) % 2 === 0 ? 1 : -1;
      return val * modifier;
  };

  const getClassForMove = (index) => {
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

  const getBestMoveSan = (index) => {
      const res = analysisResults[index - 1];
      if (!res || !res.bestMove) return null;

      const tempGame = new Chess();
      for(let i=0; i<index; i++) tempGame.move(moveHistory[i]);

      try {
        const move = tempGame.move({
            from: res.bestMove.substring(0, 2),
            to: res.bestMove.substring(2, 4),
            promotion: res.bestMove.length > 4 ? res.bestMove[4] : 'q'
        });
        return move.san;
      } catch (e) {
          return res.bestMove;
      }
  };

  const currentClassification = getClassForMove(currentMoveIndex);
  const bestMoveSuggestion = (currentClassification && (currentClassification.label === 'Mistake' || currentClassification.label === 'Blunder'))
      ? getBestMoveSan(currentMoveIndex) : null;

  // Arrows for Best Move
  const arrows = [];
  const currentAnalysis = analysisResults[currentMoveIndex];
  if (currentAnalysis && currentAnalysis.bestMove) {
      const from = currentAnalysis.bestMove.substring(0, 2);
      const to = currentAnalysis.bestMove.substring(2, 4);
      arrows.push([from, to]);
  }

  return (
    <div className="app-container">
      <div className="board-container">
        <Chessboard
            position={fen}
            onPieceDrop={onDrop}
            customArrows={arrows}
            customArrowColor="rgba(46, 204, 113, 0.8)"
        />

        <div className="navigation-controls">
            <button onClick={goToStart} disabled={currentMoveIndex === -1}>&lt;&lt;</button>
            <button onClick={goBack} disabled={currentMoveIndex === -1}>&lt;</button>
            <button onClick={goForward} disabled={currentMoveIndex === moveHistory.length - 1}>&gt;</button>
            <button onClick={goToEnd} disabled={currentMoveIndex === moveHistory.length - 1}>&gt;&gt;</button>
        </div>

        {bestMoveSuggestion && (
            <div className="feedback-panel">
                <span className={`class-badge ${currentClassification.className}`}>{currentClassification.label}</span>
                <span className="suggestion">Best was: {bestMoveSuggestion}</span>
            </div>
        )}
      </div>

      <div className="controls-container">
        <h2>Chess Analysis</h2>

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
            <button onClick={startAnalysis} disabled={isAnalyzing || moveHistory.length === 0}>
                {isAnalyzing ? `Analyzing... ${analysisProgress}%` : 'Analyze Game'}
            </button>
        </div>

        <div className="pgn-input-section">
          {!moveHistory.length && (
              <>
                <textarea
                    placeholder="Paste PGN here..."
                    value={pgnInput}
                    onChange={(e) => setPgnInput(e.target.value)}
                    rows={5}
                />
                <div className="button-group">
                    <button onClick={handlePgnLoad}>Load PGN</button>
                    <button onClick={handleReset}>Reset</button>
                </div>
              </>
          )}
          {moveHistory.length > 0 && <button onClick={handleReset}>New Game</button>}
        </div>

        <div className="move-history">
            <strong>Moves:</strong>
            <div className="move-list">
                {moveHistory.map((move, i) => {
                    const cls = getClassForMove(i);
                    return (
                        <span
                            key={i}
                            className={`move-item ${i === currentMoveIndex ? 'current' : ''} ${cls ? cls.className : ''}`}
                            onClick={() => jumpToMove(i)}
                            title={cls ? cls.label : ''}
                        >
                            {i % 2 === 0 ? <span className="move-num">{(i/2) + 1}.</span> : ''}
                            {move.san}
                            {cls && (
                                <span className="move-annotation">
                                    {cls.label === 'Best' && '★'}
                                    {cls.label === 'Excellent' && ''}
                                    {cls.label === 'Good' && ''}
                                    {cls.label === 'Inaccuracy' && '?!'}
                                    {cls.label === 'Mistake' && '?'}
                                    {cls.label === 'Blunder' && '??'}
                                </span>
                            )}
                        </span>
                    );
                })}
            </div>
        </div>
      </div>
    </div>
  );
}

export default App;
