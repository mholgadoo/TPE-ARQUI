// tron.h
#ifndef TRON_H
#define TRON_H

/**
 * @brief Start the TRON light cycles game
 * @param args Command line arguments (flags)
 * 
 * Supported flags:
 *   -mode {solo|versus}  - Game mode (default: versus)
 *   -speed N            - Ticks per second (default: 10, range: 1-60)
 *   -cell N             - Cell size in pixels (default: 8, range: 4-32)
 *   -max_score N        - Points to win (default: 10)
 * 
 * Controls:
 *   Player 1: W/A/S/D (up/left/down/right), E (turbo)
 *   Player 2: I/J/K/L (up/left/down/right), O (turbo)
 *   P: Pause/Unpause
 *   R: Reset current round
 *   Q: Quit to shell
 * 
 * Turbo:
 *   - Press E (P1) or O (P2) to activate turbo
 *   - Moves 2x faster (2 cells per tick)
 *   - Limited duration (30 ticks)
 *   - Auto-recharges when not on cooldown
 *   - Visual indicator: turbo bars in HUD
 */
void tron_game(const char *args);

#endif // TRON_H

