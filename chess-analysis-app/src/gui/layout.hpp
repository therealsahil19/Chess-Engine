#pragma once
#include "raylib.h"

namespace Gui {
namespace Layout {

// Screen dimensions
constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 960;

// Board dimensions
constexpr int BOARD_SIZE = 800;
constexpr int SQUARE_SIZE = BOARD_SIZE / 8;
constexpr int BOARD_OFFSET_X = 80;
constexpr int BOARD_OFFSET_Y = 80;

// Eval Bar
constexpr int EVAL_BAR_WIDTH = 20;
constexpr int EVAL_BAR_OFFSET_X = 40;

// Side Panel
constexpr int SIDE_PANEL_WIDTH = 320;
constexpr int INFO_X = SCREEN_WIDTH - SIDE_PANEL_WIDTH;

// Move Table
constexpr int TABLE_Y = 200;
constexpr int TABLE_WIDTH = 280;
constexpr int TABLE_HEIGHT = 600;

// Dialogs
constexpr int DIALOG_W = 600;
constexpr int DIALOG_H = 400;

// Button and Control Dimensions
constexpr int BTN_WIDTH = 110;
constexpr int BTN_HEIGHT = 40;
constexpr int BTN_MARGIN = 20;
constexpr int BTN_PLAYBACK_WIDTH = 80;
constexpr int BTN_PLAYBACK_HEIGHT = 50;
constexpr int BTN_PASTE_Y = 420;
constexpr int BTN_PASTE_W = 150;

// Colors - High Contrast Theme
constexpr Color COLOR_BG = { 43, 45, 48, 255 };        // IntelliJ-like Dark Gray
constexpr Color COLOR_LIGHT = { 235, 236, 208, 255 };  // Cream / Lichess Light
constexpr Color COLOR_DARK = { 119, 149, 86, 255 };   // Green / Lichess Dark
constexpr Color COLOR_HIGHLIGHT = { 255, 246, 126, 180 }; // Yellow highlight
constexpr Color COLOR_SELECTED = { 186, 202, 43, 200 };   // Lime-ish green for selection
constexpr Color COLOR_TEXT_MAIN = { 205, 205, 205, 255 };
constexpr Color COLOR_TEXT_DIM = { 150, 150, 150, 255 };

} // namespace Layout
} // namespace Gui
