import { useState, useEffect, useMemo } from 'react';
import { Chess } from 'chess.js';
import { Chessboard } from 'react-chessboard';
import Engine from './engine';
import './App.css';

// Helper functions
import EvaluationBar from './components/EvaluationBar';
import BoardControls from './components/BoardControls';
import AnalysisStats from './components/AnalysisStats';
import MoveHistory from './components/MoveHistory';
import ControlPanel from './components/ControlPanel';
import { ANALYSIS_MODES, ANALYSIS_DEPTHS } from './components/constants';
import { getNormScore, computeClassification } from './components/utils';

function App() {
  const [game, setGame] = useState(new Chess());
  // fen state removed, derived from game.fen()
  const [pgnInput, setPgnInput] = useState('');
  const [engine, setEngine] = useState(null);

  // Navigation State
  const [moveHistory, setMoveHistory] = useState([]);
  const [currentMoveIndex, setCurrentMoveIndex] = useState(-1);

  // Analysis State
  const [isAnalyzing, setIsAnalyzing] = useState(false);
  const [analysisProgress, setAnalysisProgress] = useState(0);
  const [analysisResults, setAnalysisResults] = useState({});
  const [analysisMode, setAnalysisMode] = useState(ANALYSIS_MODES.LOW);
  const [customDepth, setCustomDepth] = useState(ANALYSIS_DEPTHS.DEFAULT);
  const [isPgnCollapsed, setIsPgnCollapsed] = useState(true);

  // Initialize Engine
  useEffect(() => {
    const newEngine = new Engine();
    setEngine(newEngine);
    return () => newEngine.quit();
  }, []);



  const onDrop = (sourceSquare, targetSquare) => {
    const gameCopy = new Chess(game.fen());
    const move = gameCopy.move({ from: sourceSquare, to: targetSquare, promotion: 'q' });


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
    if (pgnInput.length > 1000000) {
      alert('PGN too large');
      return;
    }
    try {
      const tempGame = new Chess();
      tempGame.loadPgn(pgnInput);
      const rawHistory = tempGame.history({ verbose: true });

      const history = rawHistory.map(move => ({
        ...move,
        fen: move.after || (() => { tempGame.move(move); const f = tempGame.fen(); return f; })()
      }));

      // If .after wasn't present, we might have advanced tempGame state, but let's assume we want to rely on the verbose history if possible. 
      // Actually, if 'after' is missing, the map above with side-effect is risky if we don't reset. 
      // Safest optimized way without assumption:
      if (!rawHistory[0]?.after) {
        // Fallback to reconstruction if needed, but let's try to use the fact that we can just play forward
        const reconstructionGame = new Chess();
        for (let i = 0; i < history.length; i++) {
          reconstructionGame.move(history[i]);
          history[i].fen = reconstructionGame.fen();
        }
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
    let depth = ANALYSIS_DEPTHS.DEFAULT;
    if (analysisMode === ANALYSIS_MODES.LOW) depth = ANALYSIS_DEPTHS.LOW;
    if (analysisMode === ANALYSIS_MODES.HIGH) depth = ANALYSIS_DEPTHS.HIGH;

    if (analysisMode === ANALYSIS_MODES.CUSTOM) {
      depth = Math.max(1, Math.min(30, customDepth || ANALYSIS_DEPTHS.DEFAULT));
    }

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
      const whiteScore = getNormScore(res, currentMoveIndex);
      return {
        percentage: whiteScore > 0 ? 100 : 0,
        text: `M${Math.abs(score)}`,
        side: whiteScore > 0 ? 'bottom' : 'top'
      };
    }

    const whiteScore = getNormScore(res, currentMoveIndex);

    let percentage = 50 + (whiteScore / 10);
    percentage = Math.max(5, Math.min(95, percentage));

    return {
      percentage,
      text: (Math.abs(whiteScore) / 100).toFixed(1),
      side: whiteScore >= 0 ? 'bottom' : 'top'
    };
  };



  const currentAnalysis = analysisResults[currentMoveIndex];

  const currentClassification = computeClassification(currentMoveIndex, moveHistory, analysisResults);
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

  // moveRows logic moved to MoveHistory component

  return (
    <main className="app-container">
      <EvaluationBar evalData={evalData} />

      <section className="board-container">
        <Chessboard
          position={game.fen()}
          onPieceDrop={onDrop}
          customArrows={arrows}
          customArrowColor="rgba(129, 182, 76, 0.8)"
          boardOrientation="white"
          customBoardStyle={{
            borderRadius: '4px',
            boxShadow: '0 5px 15px rgba(0, 0, 0, 0.5)',
            backgroundColor: '#edd6b0', // Light square color to fill gaps
          }}
        />

        <BoardControls
          goToStart={goToStart}
          goBack={goBack}
          goForward={goForward}
          goToEnd={goToEnd}
          currentMoveIndex={currentMoveIndex}
          totalMoves={moveHistory.length}
        />
      </section>

      <aside className="analysis-panel">
        <div className="panel-header">
          <h2>Analysis</h2>
        </div>

        <AnalysisStats
          currentAnalysis={currentAnalysis}
          currentMoveIndex={currentMoveIndex}
          evalData={evalData}
        />

        <MoveHistory
          moveHistory={moveHistory}
          currentMoveIndex={currentMoveIndex}
          jumpToMove={jumpToMove}
          analysisResults={analysisResults}
        />

        <ControlPanel
          analysisMode={analysisMode}
          setAnalysisMode={setAnalysisMode}
          isAnalyzing={isAnalyzing}
          customDepth={customDepth}
          setCustomDepth={setCustomDepth}
          moveHistoryLength={moveHistory.length}
          startAnalysis={startAnalysis}
          analysisProgress={analysisProgress}
          isPgnCollapsed={isPgnCollapsed}
          setIsPgnCollapsed={setIsPgnCollapsed}
          pgnInput={pgnInput}
          setPgnInput={setPgnInput}
          handlePgnLoad={handlePgnLoad}
          handleReset={handleReset}
        />
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
