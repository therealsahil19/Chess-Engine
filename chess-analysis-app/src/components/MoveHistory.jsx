import React, { useMemo } from 'react';
import { CLASSIFICATION_LABELS } from './constants';
import { computeClassification } from './utils';

const MoveHistory = ({ moveHistory, currentMoveIndex, jumpToMove, analysisResults }) => {

    const moveClassifications = useMemo(() => {
        const classifications = {};
        for (let i = 0; i < moveHistory.length; i++) {
            classifications[i] = computeClassification(i, moveHistory, analysisResults);
        }
        return classifications;
    }, [moveHistory, analysisResults]);

    const moveRows = useMemo(() => {
        const rows = [];
        for (let i = 0; i < moveHistory.length; i += 2) {
            rows.push({
                num: Math.floor(i / 2) + 1,
                white: { move: moveHistory[i], index: i },
                black: moveHistory[i + 1] ? { move: moveHistory[i + 1], index: i + 1 } : null
            });
        }
        return rows;
    }, [moveHistory]);

    return (
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
                                            {whiteClass.label === CLASSIFICATION_LABELS.BEST ? '★' : ''}
                                            {whiteClass.label === CLASSIFICATION_LABELS.INACCURACY ? '?!' : ''}
                                            {whiteClass.label === CLASSIFICATION_LABELS.MISTAKE ? '?' : ''}
                                            {whiteClass.label === CLASSIFICATION_LABELS.BLUNDER ? '??' : ''}
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
                                            {blackClass.label === CLASSIFICATION_LABELS.BEST ? '★' : ''}
                                            {blackClass.label === CLASSIFICATION_LABELS.INACCURACY ? '?!' : ''}
                                            {blackClass.label === CLASSIFICATION_LABELS.MISTAKE ? '?' : ''}
                                            {blackClass.label === CLASSIFICATION_LABELS.BLUNDER ? '??' : ''}
                                        </span>
                                    )}
                                </td>
                            </tr>
                        );
                    })}
                </tbody>
            </table>
        </div>
    );
};

export default MoveHistory;
