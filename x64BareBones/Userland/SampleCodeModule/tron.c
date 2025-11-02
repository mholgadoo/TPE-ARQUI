// tron.c - TRON Light Cycles Game
#include "syscalls.h"
#include <stdint.h>
#include "shell.h"
#include "lib.h"
#include "tron.h"

// Syscalls
#define SYS_WRITE 0
#define SYS_READ 1
#define SYS_CLEAR_SCREEN 2
#define SYS_SLEEP 3
#define SYS_DRAW_RECT 4
#define SYS_PLAY_BEEP 8
#define SYS_PUT_CHAR 16

// Screen dimensions
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// Game constants
#define MAX_GRID_WIDTH 128
#define MAX_GRID_HEIGHT 96
#define DEFAULT_CELL_SIZE 8
#define DEFAULT_SPEED 10
#define DEFAULT_MAX_SCORE 10
#define HUD_HEIGHT 30

// Colors
#define COLOR_BLACK 0x000000
#define COLOR_PLAYER1 0x00FFFF  // Cyan
#define COLOR_PLAYER2 0xFF00FF  // Magenta
#define COLOR_WALL 0x808080     // Gray
#define COLOR_TEXT 0xFFFFFF     // White

// Grid values
#define EMPTY 0
#define WALL 1
#define P1_TRAIL 2
#define P2_TRAIL 3

// Directions
#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

// Game modes
#define MODE_SOLO 0
#define MODE_VERSUS 1

// Game states
#define STATE_PLAYING 0
#define STATE_PAUSED 1
#define STATE_ROUND_END 2

// Turbo settings
#define TURBO_DURATION 30       // Turbo lasts 30 ticks
#define TURBO_COOLDOWN 60       // 60 ticks cooldown after use
#define TURBO_MAX_CHARGE 30     // Max turbo charge

typedef struct {
    int x;
    int y;
    int dir;
    int next_dir;
    int alive;
    int turbo_active;           // Is turbo currently active
    int turbo_charge;           // Remaining turbo charge (ticks)
    int turbo_cooldown;         // Cooldown before next turbo use
} Player;

typedef struct {
    int mode;              // solo or versus
    int speed;             // ticks per second
    int cell_size;         // pixel size of each cell
    int grid_width;        // arena width in cells
    int grid_height;       // arena height in cells
    int max_score;         // points to win
    int score1;
    int score2;
    int state;
    int fps;               // current FPS
    Player p1;
    Player p2;
    uint8_t grid[MAX_GRID_HEIGHT][MAX_GRID_WIDTH];
} GameState;

static GameState game;

// Direction vectors
static const int dx[] = {0, 1, 0, -1};
static const int dy[] = {-1, 0, 1, 0};

// ============================================================================
// Utility Functions
// ============================================================================

static int str_starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str++ != *prefix++) return 0;
    }
    return 1;
}

static int str_to_int(const char *str) {
    int result = 0;
    int sign = 1;
    
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

static void skip_spaces(const char **str) {
    while (**str == ' ') (*str)++;
}

// ============================================================================
// Grid Functions
// ============================================================================

static void init_grid() {
    for (int y = 0; y < game.grid_height; y++) {
        for (int x = 0; x < game.grid_width; x++) {
            game.grid[y][x] = EMPTY;
        }
    }
}

static int check_collision(int x, int y) {
    // Check boundaries
    if (x < 0 || x >= game.grid_width || y < 0 || y >= game.grid_height) {
        return 1;
    }
    // Check if cell is occupied
    return game.grid[y][x] != EMPTY;
}

static void place_trail(int x, int y, int player) {
    if (x >= 0 && x < game.grid_width && y >= 0 && y < game.grid_height) {
        game.grid[y][x] = (player == 1) ? P1_TRAIL : P2_TRAIL;
    }
}

// ============================================================================
// Drawing Functions
// ============================================================================

static void draw_cell(int x, int y, uint32_t color) {
    int px = x * game.cell_size;
    int py = HUD_HEIGHT + y * game.cell_size;
    _sys_drawRect(SYS_DRAW_RECT, color, px, py, game.cell_size, game.cell_size);
}

// Helper to draw text using kernel's putChar
static void draw_text(const char *text, int x, int y, uint32_t color) {
    int offset = 0;
    while (*text) {
        _sys_putChar(SYS_PUT_CHAR, *text, x + offset, y, color);
        offset += 8;  // Font width is 8 pixels
        text++;
    }
}

static void draw_hud() {
    // Clear HUD area - FIXED POSITION
    _sys_drawRect(SYS_DRAW_RECT, COLOR_BLACK, 0, 0, SCREEN_WIDTH, HUD_HEIGHT);
    
    // Build text strings
    char buf[100];
    
    // P1 score
    buf[0] = 'P'; buf[1] = '1'; buf[2] = ':'; buf[3] = ' ';
    int len = int_to_str(game.score1, buf + 4);
    buf[4 + len] = '\0';
    draw_text(buf, 10, 5, COLOR_PLAYER1);
    
    // P2 score
    buf[0] = 'P'; buf[1] = '2'; buf[2] = ':'; buf[3] = ' ';
    len = int_to_str(game.score2, buf + 4);
    buf[4 + len] = '\0';
    draw_text(buf, 120, 5, COLOR_PLAYER2);
    
    // Speed
    buf[0] = 'S'; buf[1] = 'p'; buf[2] = 'd'; buf[3] = ':'; buf[4] = ' ';
    len = int_to_str(game.speed, buf + 5);
    buf[5 + len] = '\0';
    draw_text(buf, 240, 5, 0xFFFF00);
    
    // FPS
    buf[0] = 'F'; buf[1] = 'P'; buf[2] = 'S'; buf[3] = ':'; buf[4] = ' ';
    len = int_to_str(game.fps, buf + 5);
    buf[5 + len] = '\0';
    draw_text(buf, 360, 5, 0x00FF00);
    
    // Draw turbo bars at FIXED positions
    int bar_width = 100;
    int bar_height = 6;
    int bar_y = 20;
    
    // P1 turbo bar (left side) - FIXED
    int p1_x = 10;
    // Background
    _sys_drawRect(SYS_DRAW_RECT, 0x404040, p1_x, bar_y, bar_width, bar_height);
    // Charge level (cyan if ready, gray if cooldown)
    int p1_fill = (game.p1.turbo_charge * bar_width) / TURBO_MAX_CHARGE;
    uint32_t p1_color = (game.p1.turbo_cooldown > 0) ? 0x606060 : COLOR_PLAYER1;
    if (p1_fill > 0) {
        _sys_drawRect(SYS_DRAW_RECT, p1_color, p1_x, bar_y, p1_fill, bar_height);
    }
    // Active indicator (brighter)
    if (game.p1.turbo_active) {
        _sys_drawRect(SYS_DRAW_RECT, 0xFFFFFF, p1_x, bar_y, bar_width, bar_height);
    }
    
    // P2 turbo bar (right side) - FIXED - only in versus mode
    if (game.mode == MODE_VERSUS) {
        int p2_x = SCREEN_WIDTH - bar_width - 10;
        // Background
        _sys_drawRect(SYS_DRAW_RECT, 0x404040, p2_x, bar_y, bar_width, bar_height);
        // Charge level (magenta if ready, gray if cooldown)
        int p2_fill = (game.p2.turbo_charge * bar_width) / TURBO_MAX_CHARGE;
        uint32_t p2_color = (game.p2.turbo_cooldown > 0) ? 0x606060 : COLOR_PLAYER2;
        if (p2_fill > 0) {
            _sys_drawRect(SYS_DRAW_RECT, p2_color, p2_x, bar_y, p2_fill, bar_height);
        }
        // Active indicator (brighter)
        if (game.p2.turbo_active) {
            _sys_drawRect(SYS_DRAW_RECT, 0xFFFF00, p2_x, bar_y, bar_width, bar_height);
        }
    }
}

static void draw_game() {
    // Clear playing area
    _sys_drawRect(SYS_DRAW_RECT, COLOR_BLACK, 0, HUD_HEIGHT, 
                  SCREEN_WIDTH, SCREEN_HEIGHT - HUD_HEIGHT);
    
    // Draw grid
    for (int y = 0; y < game.grid_height; y++) {
        for (int x = 0; x < game.grid_width; x++) {
            uint32_t color = COLOR_BLACK;
            switch (game.grid[y][x]) {
                case P1_TRAIL:
                    color = COLOR_PLAYER1;
                    break;
                case P2_TRAIL:
                    color = COLOR_PLAYER2;
                    break;
            }
            if (color != COLOR_BLACK) {
                draw_cell(x, y, color);
            }
        }
    }
    
    // Draw player heads (brighter)
    if (game.p1.alive) {
        draw_cell(game.p1.x, game.p1.y, 0xFFFFFF);
    }
    // In solo mode, P2 is just an obstacle (draw differently)
    if (game.mode == MODE_SOLO) {
        draw_cell(game.p2.x, game.p2.y, 0xFF0000);  // Red obstacle
    } else if (game.p2.alive) {
        draw_cell(game.p2.x, game.p2.y, 0xFFFF00);  // Yellow head
    }
    
    draw_hud();
}

// ============================================================================
// Game Logic
// ============================================================================

static void init_players() {
    // Player 1 starts on the left
    game.p1.x = game.grid_width / 4;
    game.p1.y = game.grid_height / 2;
    game.p1.dir = DIR_RIGHT;
    game.p1.next_dir = DIR_RIGHT;
    game.p1.alive = 1;
    game.p1.turbo_active = 0;
    game.p1.turbo_charge = TURBO_MAX_CHARGE;
    game.p1.turbo_cooldown = 0;
    
    // Player 2 starts on the right
    game.p2.x = 3 * game.grid_width / 4;
    game.p2.y = game.grid_height / 2;
    game.p2.dir = DIR_LEFT;
    game.p2.next_dir = DIR_LEFT;
    game.p2.turbo_active = 0;
    game.p2.turbo_charge = TURBO_MAX_CHARGE;
    game.p2.turbo_cooldown = 0;
    
    // In solo mode, P2 doesn't move (alive=0 means it won't update)
    game.p2.alive = (game.mode == MODE_VERSUS) ? 1 : 0;
    
    // Place initial trails
    place_trail(game.p1.x, game.p1.y, 1);
    if (game.mode == MODE_SOLO) {
        // In solo mode, P2 is just a stationary obstacle
        place_trail(game.p2.x, game.p2.y, 2);
    } else {
        place_trail(game.p2.x, game.p2.y, 2);
    }
}

static void reset_round() {
    init_grid();
    init_players();
    game.state = STATE_PLAYING;
}

static int is_opposite_dir(int dir1, int dir2) {
    return (dir1 + 2) % 4 == dir2;
}

// AI removed - in solo mode, P2 is just a stationary obstacle

static void update_turbo(Player *p) {
    // Update turbo cooldown
    if (p->turbo_cooldown > 0) {
        p->turbo_cooldown--;
    }
    
    // Update turbo charge if active
    if (p->turbo_active) {
        p->turbo_charge--;
        if (p->turbo_charge <= 0) {
            p->turbo_active = 0;
            p->turbo_charge = 0;
            p->turbo_cooldown = TURBO_COOLDOWN;
        }
    } else {
        // Recharge turbo when not on cooldown
        if (p->turbo_cooldown == 0 && p->turbo_charge < TURBO_MAX_CHARGE) {
            p->turbo_charge++;
        }
    }
}

static void update_players() {
    // Update directions (no 180° turn)
    if (!is_opposite_dir(game.p1.dir, game.p1.next_dir)) {
        game.p1.dir = game.p1.next_dir;
    }
    if (!is_opposite_dir(game.p2.dir, game.p2.next_dir)) {
        game.p2.dir = game.p2.next_dir;
    }
    
    // Update turbo states
    update_turbo(&game.p1);
    update_turbo(&game.p2);
    
    // Move players (with turbo = 2 steps)
    int p1_steps = (game.p1.alive && game.p1.turbo_active) ? 2 : 1;
    int p2_steps = (game.p2.alive && game.p2.turbo_active) ? 2 : 1;
    
    int p1_collision = 0;
    int p2_collision = 0;
    
    // Move P1
    for (int step = 0; step < p1_steps && game.p1.alive; step++) {
        game.p1.x += dx[game.p1.dir];
        game.p1.y += dy[game.p1.dir];
        
        if (check_collision(game.p1.x, game.p1.y)) {
            p1_collision = 1;
            game.p1.alive = 0;
            break;
        }
        place_trail(game.p1.x, game.p1.y, 1);
    }
    
    // Move P2
    for (int step = 0; step < p2_steps && game.p2.alive; step++) {
        game.p2.x += dx[game.p2.dir];
        game.p2.y += dy[game.p2.dir];
        
        if (check_collision(game.p2.x, game.p2.y)) {
            p2_collision = 1;
            game.p2.alive = 0;
            break;
        }
        place_trail(game.p2.x, game.p2.y, 2);
    }
    
    // Handle scoring
    if (p1_collision || p2_collision) {
        if (p1_collision && p2_collision) {
            // Both crashed - no points
            _sys_playBeep(SYS_PLAY_BEEP, 200, 100);
        } else if (p1_collision) {
            // P1 crashed, P2 wins
            game.score2++;
            _sys_playBeep(SYS_PLAY_BEEP, 880, 150);
        } else {
            // P2 crashed, P1 wins
            game.score1++;
            _sys_playBeep(SYS_PLAY_BEEP, 440, 150);
        }
        
        game.state = STATE_ROUND_END;
    }
}

static void handle_input() {
    char c;
    while (read(&c, 1) > 0) {
        // Player 1 controls (WASD)
        if (c == 'w' || c == 'W') game.p1.next_dir = DIR_UP;
        if (c == 's' || c == 'S') game.p1.next_dir = DIR_DOWN;
        if (c == 'a' || c == 'A') game.p1.next_dir = DIR_LEFT;
        if (c == 'd' || c == 'D') game.p1.next_dir = DIR_RIGHT;
        
        // Player 1 turbo (E)
        if ((c == 'e' || c == 'E') && game.p1.turbo_charge > 0 && game.p1.turbo_cooldown == 0) {
            game.p1.turbo_active = 1;
            _sys_playBeep(SYS_PLAY_BEEP, 1000, 50);  // Turbo sound
        }
        
        // Player 2 controls (IJKL - same as pongis)
        if (c == 'i' || c == 'I') game.p2.next_dir = DIR_UP;
        if (c == 'k' || c == 'K') game.p2.next_dir = DIR_DOWN;
        if (c == 'j' || c == 'J') game.p2.next_dir = DIR_LEFT;
        if (c == 'l' || c == 'L') game.p2.next_dir = DIR_RIGHT;
        
        // Player 2 turbo (O)
        if ((c == 'o' || c == 'O') && game.p2.turbo_charge > 0 && game.p2.turbo_cooldown == 0) {
            game.p2.turbo_active = 1;
            _sys_playBeep(SYS_PLAY_BEEP, 1200, 50);  // Turbo sound
        }
        
        // Global controls
        if (c == 'p' || c == 'P') {
            game.state = (game.state == STATE_PAUSED) ? STATE_PLAYING : STATE_PAUSED;
            if (game.state == STATE_PAUSED) {
                print("=== PAUSED ===\n");
            }
        }
        if (c == 'r' || c == 'R') {
            reset_round();
        }
        if (c == 'q' || c == 'Q') {
            game.state = -1; // Signal to quit
            return;
        }
    }
}

static void show_winner() {
    clearScreen();
    changeFontSize(3);
    
    if (game.score1 >= game.max_score) {
        print("PLAYER 1 WINS!\n");
    } else {
        print("PLAYER 2 WINS!\n");
    }
    
    // Victory sound
    _sys_playBeep(SYS_PLAY_BEEP, 523, 200);
    _sys_sleep(SYS_SLEEP, 10);
    _sys_playBeep(SYS_PLAY_BEEP, 659, 200);
    _sys_sleep(SYS_SLEEP, 10);
    _sys_playBeep(SYS_PLAY_BEEP, 784, 400);
    
    changeFontSize(1);
    _sys_sleep(SYS_SLEEP, 60);
}

// ============================================================================
// Player Selection
// ============================================================================

static int ask_players() {
    print("Numero de jugadores (1/2): ");
    char c = 0;
    while (1) {
        if (read(&c, 1) > 0) {
            if (c == '1') {
                print("1\n");
                return 1;
            } else if (c == '2') {
                print("2\n");
                return 2;
            }
            // Ignore other characters including escape sequences
        }
    }
}

// ============================================================================
// Main Game Loop
// ============================================================================

static void game_loop() {
    uint64_t tick_duration_ms = 1000 / game.speed;
    uint64_t last_tick = bench_start();
    
    // FPS counter - sliding window of 60 frames
    #define FPS_WINDOW 60
    uint64_t frame_times[FPS_WINDOW];
    int frame_index = 0;
    int frame_count = 0;
    uint64_t last_frame_time = bench_start();
    
    // Initialize FPS
    game.fps = 0;
    
    while (game.state >= 0) {
        // Handle input
        handle_input();
        
        if (game.state == STATE_PAUSED) {
            _sys_sleep(SYS_SLEEP, 2);
            continue;
        }
        
        if (game.state == STATE_ROUND_END) {
            // Check for game end
            if (game.score1 >= game.max_score || game.score2 >= game.max_score) {
                show_winner();
                break;
            }
            
            // Wait and reset
            _sys_sleep(SYS_SLEEP, 40); // 1 second
            reset_round();
            draw_game();
            last_tick = bench_start();
            last_frame_time = bench_start();
            continue;
        }
        
        // Calculate elapsed time for game tick
        uint64_t current = bench_start();
        uint64_t elapsed = bench_stop(last_tick);
        uint64_t elapsed_ms = cycles_to_ms(elapsed);
        
        if (elapsed_ms >= tick_duration_ms) {
            // Update game
            update_players();
            
            // Calculate FPS
            uint64_t frame_elapsed = bench_stop(last_frame_time);
            frame_times[frame_index] = frame_elapsed;
            frame_index = (frame_index + 1) % FPS_WINDOW;
            if (frame_count < FPS_WINDOW) {
                frame_count++;
            }
            
            // Calculate average FPS over window
            uint64_t total_cycles = 0;
            for (int i = 0; i < frame_count; i++) {
                total_cycles += frame_times[i];
            }
            uint64_t avg_ms = cycles_to_ms(total_cycles / frame_count);
            if (avg_ms > 0) {
                game.fps = 1000 / avg_ms;
            }
            
            draw_game();
            last_tick = current;
            last_frame_time = bench_start();
        } else {
            // Sleep for a short time
            _sys_sleep(SYS_SLEEP, 1);
        }
    }
}

// ============================================================================
// Initialization and Entry Point
// ============================================================================

static void parse_args(const char *args) {
    // Set defaults
    game.mode = MODE_VERSUS;
    game.speed = DEFAULT_SPEED;
    game.cell_size = DEFAULT_CELL_SIZE;
    game.max_score = DEFAULT_MAX_SCORE;
    
    // Parse arguments
    while (*args) {
        skip_spaces(&args);
        if (*args == '\0') break;
        
        if (str_starts_with(args, "-mode")) {
            args += 5;
            skip_spaces(&args);
            if (str_starts_with(args, "solo")) {
                game.mode = MODE_SOLO;
                args += 4;
            } else if (str_starts_with(args, "versus")) {
                game.mode = MODE_VERSUS;
                args += 6;
            }
        }
        else if (str_starts_with(args, "-speed")) {
            args += 6;
            skip_spaces(&args);
            game.speed = str_to_int(args);
            if (game.speed < 1) game.speed = 1;
            if (game.speed > 60) game.speed = 60;
            while (*args >= '0' && *args <= '9') args++;
        }
        else if (str_starts_with(args, "-cell")) {
            args += 5;
            skip_spaces(&args);
            game.cell_size = str_to_int(args);
            if (game.cell_size < 4) game.cell_size = 4;
            if (game.cell_size > 32) game.cell_size = 32;
            while (*args >= '0' && *args <= '9') args++;
        }
        else if (str_starts_with(args, "-max_score")) {
            args += 10;
            skip_spaces(&args);
            game.max_score = str_to_int(args);
            if (game.max_score < 1) game.max_score = 1;
            while (*args >= '0' && *args <= '9') args++;
        }
        else {
            // Skip unknown argument
            while (*args && *args != ' ' && *args != '-') args++;
        }
    }
    
    // Calculate grid dimensions
    game.grid_width = SCREEN_WIDTH / game.cell_size;
    game.grid_height = (SCREEN_HEIGHT - HUD_HEIGHT) / game.cell_size;
    
    if (game.grid_width > MAX_GRID_WIDTH) game.grid_width = MAX_GRID_WIDTH;
    if (game.grid_height > MAX_GRID_HEIGHT) game.grid_height = MAX_GRID_HEIGHT;
}

void tron_game(const char *args) {
    // Parse arguments (may override mode)
    parse_args(args);
    
    // Clear screen
    clearScreen();
    
    // Ask for number of players (unless -mode was specified)
    int has_mode_flag = 0;
    const char *p = args;
    while (*p) {
        if (str_starts_with(p, "-mode")) {
            has_mode_flag = 1;
            break;
        }
        p++;
    }
    
    if (!has_mode_flag) {
        print("=== TRON LIGHT CYCLES ===\n\n");
        int players = ask_players();
        game.mode = (players == 1) ? MODE_SOLO : MODE_VERSUS;
        print("\n");
    }
    
    // Initialize game state
    game.score1 = 0;
    game.score2 = 0;
    game.state = STATE_PLAYING;
    
    // Show instructions
    print("=== INSTRUCCIONES ===\n");
    print("Player 1: W/A/S/D | Turbo: E\n");
    if (game.mode == MODE_VERSUS) {
        print("Player 2: I/J/K/L | Turbo: O\n");
    } else {
        print("Mode: SOLO (evita el obstaculo!)\n");
    }
    print("P: Pause | R: Reset | Q: Quit\n");
    print("Turbo: 2x velocidad (recarga auto)\n\n");
    print("Presiona ESPACIO para empezar...\n");
    
    // Wait for SPACE key specifically to avoid arrow key issues
    char c = 0;
    while (1) {
        if (read(&c, 1) > 0 && c == ' ') {
            break;
        }
    }
    
    // Clear any remaining input
    while (read(&c, 1) > 0) {}
    
    clearScreen();
    
    // Initialize round
    reset_round();
    draw_game();
    
    // Start game loop
    game_loop();
    
    // Return to shell
    clearScreen();
    print("Gracias por jugar TRON!\n");
}

