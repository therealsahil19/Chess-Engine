import { describe, it, expect } from 'vitest';
import { getNormScore, computeClassification } from './utils';
import { CLASSIFICATION_LABELS } from './constants';

describe('utils', () => {
    describe('getNormScore', () => {
        it('returns 0 if no result', () => {
            expect(getNormScore(null, 0)).toBe(0);
        });

        it('returns flipped score for index 0 (White move, Black to move)', () => {
            // Index 0: After 1. e4. Black to move.
            // Stockfish: +50 (Black advantage).
            // Relative to White: -50.
            expect(getNormScore({ scoreVal: 50, scoreType: 'cp' }, 0)).toBe(-50);
        });

        it('returns direct score for index 1 (Black move, White to move)', () => {
            // Index 1: After 1... e5. White to move.
            // Stockfish: +50 (White advantage).
            // Relative to White: +50.
            expect(getNormScore({ scoreVal: 50, scoreType: 'cp' }, 1)).toBe(50);
        });

        it('handles mate score', () => {
            // Mate 5.
            // (5 > 0 ? 10000 : -10000) - 500 = 9500 (Base val).

            // Case: Index 1 (White to move). Mate +5 (White mates in 5).
            // modifier 1 -> 9500.
            expect(getNormScore({ scoreVal: 5, scoreType: 'mate' }, 1)).toBe(9500);

            // Case: Index 0 (Black to move). Mate +5 (Black mates in 5).
            // modifier -1 -> -9500. (White is being mated).
            expect(getNormScore({ scoreVal: 5, scoreType: 'mate' }, 0)).toBe(-9500);
        });
    });
});
