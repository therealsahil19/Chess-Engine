import React from 'react';

const AnalysisStats = ({ currentAnalysis, currentMoveIndex, evalData }) => {
    if (!currentAnalysis) return null;

    return (
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
    );
};

export default AnalysisStats;
