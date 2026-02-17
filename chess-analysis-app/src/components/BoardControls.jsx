import React from 'react';

const BoardControls = ({
    goToStart,
    goBack,
    goForward,
    goToEnd,
    currentMoveIndex,
    totalMoves
}) => {
    return (
        <div className="navigation-controls">
            <button onClick={goToStart} disabled={currentMoveIndex === -1} aria-label="Go to start">«</button>
            <button onClick={goBack} disabled={currentMoveIndex === -1} aria-label="Go back">‹</button>
            <button onClick={goForward} disabled={currentMoveIndex === totalMoves - 1} aria-label="Go forward">›</button>
            <button onClick={goToEnd} disabled={currentMoveIndex === totalMoves - 1} aria-label="Go to end">»</button>
        </div>
    );
};

export default BoardControls;
