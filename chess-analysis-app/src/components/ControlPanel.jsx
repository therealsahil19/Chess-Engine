import React from 'react';
import { ANALYSIS_MODES } from './constants';

const ControlPanel = ({
    analysisMode,
    setAnalysisMode,
    isAnalyzing,
    customDepth,
    setCustomDepth,
    moveHistoryLength,
    startAnalysis,
    analysisProgress,
    isPgnCollapsed,
    setIsPgnCollapsed,
    pgnInput,
    setPgnInput,
    handlePgnLoad,
    handleReset
}) => {
    return (
        <>
            <div className="settings-section">
                <div className="analysis-settings">
                    <label>Mode:
                        <select value={analysisMode} onChange={(e) => setAnalysisMode(e.target.value)} disabled={isAnalyzing}>
                            <option value={ANALYSIS_MODES.LOW}>Low Reasoning (Fast)</option>
                            <option value={ANALYSIS_MODES.HIGH}>High Reasoning (Deep)</option>
                            <option value={ANALYSIS_MODES.CUSTOM}>Custom</option>
                        </select>
                    </label>
                    {analysisMode === ANALYSIS_MODES.CUSTOM && (
                        <input
                            type="number"
                            value={customDepth}
                            onChange={(e) => setCustomDepth(parseInt(e.target.value))}
                            min="1" max="30"
                        />
                    )}
                    <button className="analyze-button" onClick={startAnalysis} disabled={isAnalyzing || moveHistoryLength === 0}>
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
        </>
    );
};

export default ControlPanel;
