import { CLASSIFICATION_THRESHOLDS, CLASSIFICATION_LABELS } from './constants';

export const getNormScore = (res, index) => {
    if (!res) return 0;
    let val = res.scoreVal;
    if (res.scoreType === 'mate') {
        val = (val > 0 ? 10000 : -10000) - (val * 100);
    }
    // If index is (move number - 1), then (index + 1) is the move number.
    // Odd move number = White, Even = Black.
    // If index is (move number - 1), then (index + 1) is the move number.
    // Odd move number (1, 3...) -> White played.
    // Even move number (2, 4...) -> Black played.
    // Result for index 0 (Move 1): Black to move. Score relative to Side to Move (Black).
    // If Black is winning (+), result is relative to Black.
    // We want score relative to White.
    // If Black +, White -.
    // So modifier -1.
    // (0 + 1) % 2 = 1. -> -1. Correct.
    // Result for index 1 (Move 2): White to move. Score relative to White.
    // modifier 1.
    // (1 + 1) % 2 = 0. -> 1. Correct.
    const modifier = (index + 1) % 2 === 0 ? 1 : -1;
    return val * modifier;
};

export const computeClassification = (index, moveHistory, analysisResults) => {
    if (index === -1) return null; // Start pos
    const prevRes = analysisResults[index - 1];
    const currRes = analysisResults[index];
    if (!prevRes || !currRes) return null;

    const playedMoveObj = moveHistory[index];
    const playedMoveLan = playedMoveObj.from + playedMoveObj.to + (playedMoveObj.promotion || '');
    if (playedMoveLan === prevRes.bestMove) return { label: CLASSIFICATION_LABELS.BEST, className: 'class-best' };

    const prevScore = getNormScore(prevRes, index - 1);
    const currScore = getNormScore(currRes, index);

    let loss = 0;
    if (playedMoveObj.color === 'w') {
        loss = prevScore - currScore;
    } else {
        loss = currScore - prevScore;
    }

    if (loss <= CLASSIFICATION_THRESHOLDS.BEST) return { label: CLASSIFICATION_LABELS.BEST, className: 'class-best' };
    if (loss <= CLASSIFICATION_THRESHOLDS.EXCELLENT) return { label: CLASSIFICATION_LABELS.EXCELLENT, className: 'class-excellent' };
    if (loss <= CLASSIFICATION_THRESHOLDS.GOOD) return { label: CLASSIFICATION_LABELS.GOOD, className: 'class-good' };
    if (loss <= CLASSIFICATION_THRESHOLDS.INACCURACY) return { label: CLASSIFICATION_LABELS.INACCURACY, className: 'class-inaccuracy' };
    if (loss <= CLASSIFICATION_THRESHOLDS.MISTAKE) return { label: CLASSIFICATION_LABELS.MISTAKE, className: 'class-mistake' };
    return { label: CLASSIFICATION_LABELS.BLUNDER, className: 'class-blunder' };
};
