import React from 'react';

const EvaluationBar = ({ evalData }) => {
    return (
        <aside className="evaluation-bar-container" aria-label="Evaluation Bar">
            <div
                className="evaluation-bar-fill"
                style={{ height: `${evalData.percentage}%` }}
            />
            <span className={`evaluation-text ${evalData.side}`}>{evalData.text}</span>
        </aside>
    );
};

export default EvaluationBar;
