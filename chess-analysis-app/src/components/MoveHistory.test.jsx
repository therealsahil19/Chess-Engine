import React from 'react';
import { render, screen } from '@testing-library/react';
import MoveHistory from './MoveHistory';
import { describe, it, expect } from 'vitest';

describe('MoveHistory Component', () => {
    const mockMoveHistory = [
        { san: 'e4', from: 'e2', to: 'e4', color: 'w' },
        { san: 'e5', from: 'e7', to: 'e5', color: 'b' }
    ];
    const mockAnalysisResults = {};

    it('should render without crashing when analysisResults is provided', () => {
        render(
            <MoveHistory
                moveHistory={mockMoveHistory}
                currentMoveIndex={-1}
                jumpToMove={() => {}}
                analysisResults={mockAnalysisResults}
            />
        );
        expect(screen.getByText('e4')).toBeInTheDocument();
    });
});
