#include "syscalls.h"
#include <stdint.h>
#include "shell.h"
#include "lib.h"

//numeros syscalls
#define SYS_WRITE 0
#define SYS_READ 1
#define SYS_CLEAR_SCREEN 2
#define SYS_SLEEP 3
#define SYS_DRAW_RECT 4
#define SYS_PLAY_BEEP 8
#define SYS_FONT_SIZE 9

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define ESC 27

#define PLAYERS_SIZE 40
#define BALL_SIZE   16
#define HOLE_SIZE   58

#define L1_OB_W 100
#define L1_OB_H 100 //tamaño obstaculos nivel 1
#define L1_OBS  2 //cantidad de obstaculos del nivel 1

#define L2_OB_W 60
#define L2_OB_H 60
#define L2_OBS  4

#define PLAYER_SPEED 8
#define BALL_SPEED   8

typedef struct {
    int x;
    int y;
} coords;

static coords *obstacles;
static int obstacle_count;
static int obstacle_w;
static int obstacle_h;


//posicion de los obstaculos y con _sys_drawRect le damos color y tamaño
static coords level1_obstacles[L1_OBS] = {
    {SCREEN_WIDTH/3 - L1_OB_W/2, SCREEN_HEIGHT/2 - L1_OB_H/2},
    {2*SCREEN_WIDTH/3 - L1_OB_W/2, SCREEN_HEIGHT/2 - L1_OB_H/2}
};

static coords level2_obstacles[L2_OBS] = {
    {SCREEN_WIDTH/2 - L2_OB_W/2, SCREEN_HEIGHT/2 - 150},
    {SCREEN_WIDTH/4 - L2_OB_W/2, SCREEN_HEIGHT/3},
    {3*SCREEN_WIDTH/4 - L2_OB_W/2, SCREEN_HEIGHT/3},
    {SCREEN_WIDTH/2 - L2_OB_W/2, SCREEN_HEIGHT/2 + 80}
};

//implementamos el score
static void draw_score(int p1, int p2) {
    char buf[32];
    print("P1: ");
    int len = int_to_str(p1, buf);
    buf[len] = '\0';
    print(buf);
    print("    P2: ");
    len = int_to_str(p2, buf);
    buf[len] = '\0';
    print(buf);
    print("\n");
}

//para ver cuantos jugadores juegan. Si juega uno solo, el otro se mueve solo (siguiendo a la pelota en el eje y)
static int ask_players() {
    const char *msg = "Jugadores (1/2): ";
    print(msg);
    char c = 0;
    while (read(&c, 1) <= 0 || (c != '1' && c != '2')) {}
    print("\n");
    return c - '0';
}

static void show_victory(int last, int players) {
    clearScreen();
    changeFontSize(3);
    const char *msg;
    if (players == 1) {
        msg = "GANASTE!";
    } else {
        msg = (last == 1) ? "EL JUGADOR 1 GANO" : "EL JUGADOR 2 GANO";
    }
    print(msg);
    print("\n");

    //cacion
    playBeep(8, 349.23, 400);
    playBeep(8, 349.23, 300);
    playBeep(8, 349.23, 100);
    playBeep(8, 466.16, 400);
    playBeep(8, 466.16, 400);
    playBeep(8, 523.25, 400);
    playBeep(8, 523.25, 400);
    playBeep(8, 349.23, 600);
    playBeep(8, 587.33, 200);
    playBeep(8, 466.16, 400);
    playBeep(8, 466.16, 400);
    playBeep(8, 587.33, 300);
    playBeep(8, 466.16, 100);
    playBeep(8, 392.00, 400);
    playBeep(8, 622.25, 800);
    playBeep(8, 523.25, 300);
    playBeep(8, 440.00, 100);
    changeFontSize(1);
    _sys_sleep(SYS_SLEEP, 60);
}

static int box_collider(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return !(x1 + w1 < x2 || x2 + w2 < x1 || y1 + h1 < y2 || y2 + h2 < y1);
}

static void VInRange(int *v, int min, int max) {
    if (*v < min) *v = min;
    if (*v > max) *v = max;
}

void pongis_game() {
    int players = ask_players();
    
    coords p1 = {100, SCREEN_HEIGHT/2 - PLAYERS_SIZE/2};
    coords p2 = {SCREEN_WIDTH-100-PLAYERS_SIZE, SCREEN_HEIGHT/2 - PLAYERS_SIZE/2};
    
    int score1 = 0;
    int score2 = 0;
    int last = 0;
    char running = 1;

    for (int level = 1; level <= 2 && running; level++) {
        if (level == 1) {
            obstacles     = level1_obstacles;
            obstacle_count = L1_OBS;
            obstacle_w     = L1_OB_W;
            obstacle_h     = L1_OB_H;
        } else {
            obstacles     = level2_obstacles;
            obstacle_count = L2_OBS;
            obstacle_w     = L2_OB_W;
            obstacle_h     = L2_OB_H;
        }

        coords ball = { SCREEN_WIDTH/2 - BALL_SIZE/2, SCREEN_HEIGHT/2 - BALL_SIZE/2 };
        coords hole = { SCREEN_WIDTH/2 - HOLE_SIZE/2, 60 };
        int vx = 0, vy = 0;
        char levelWon = 0;

        while (running && !levelWon) {
            coords pos1 = p1, pos2 = p2; //para que podamos volver atras si se chocan con obstaculos
            char c = 0;
            while (read(&c, 1) > 0) {
                //con q o ESC salimos del juego
                if (c == ESC || c == 'q') running = 0;
                if (c == 'w') p1.y -= PLAYER_SPEED;
                if (c == 's') p1.y += PLAYER_SPEED;
                if (c == 'a') p1.x -= PLAYER_SPEED;
                if (c == 'd') p1.x += PLAYER_SPEED;
                if (players == 2) {
                    if (c == 'i') p2.y -= PLAYER_SPEED;
                    if (c == 'k') p2.y += PLAYER_SPEED;
                    if (c == 'j') p2.x -= PLAYER_SPEED;
                    if (c == 'l') p2.x += PLAYER_SPEED;
                }
            }
            if (!running) break;

            if (players == 1) {
                // el segundo jugador se mueve solo (sube y baja segun la pos de la pelota)
                if (p2.y + PLAYERS_SIZE/2 < ball.y)
                    p2.y += PLAYER_SPEED;
                else if (p2.y + PLAYERS_SIZE/2 > ball.y + BALL_SIZE)
                    p2.y -= PLAYER_SPEED;
            }

            //jugadores no pueden salirse de la pantalla
            VInRange(&p1.x, 0, SCREEN_WIDTH-PLAYERS_SIZE);
            VInRange(&p1.y, 0, SCREEN_HEIGHT-PLAYERS_SIZE);
            VInRange(&p2.x, 0, SCREEN_WIDTH-PLAYERS_SIZE);
            VInRange(&p2.y, 0, SCREEN_HEIGHT-PLAYERS_SIZE);

            for (int i = 0; i < obstacle_count; i++) {
                if (box_collider(p1.x,p1.y,PLAYERS_SIZE,PLAYERS_SIZE, obstacles[i].x,obstacles[i].y,obstacle_w,obstacle_h))
                    p1 = pos1;
                if (box_collider(p2.x,p2.y,PLAYERS_SIZE,PLAYERS_SIZE, obstacles[i].x,obstacles[i].y,obstacle_w,obstacle_h))
                    p2 = pos2;
            }

            coords coords_ball = ball;
            ball.x += vx;
            if (ball.x <= 0 || ball.x +BALL_SIZE >= SCREEN_WIDTH) {
                ball.x = coords_ball.x;
                vx = -vx;
            }
            for (int i = 0; i < obstacle_count; i++) {
                if (box_collider(ball.x,ball.y,BALL_SIZE,BALL_SIZE, obstacles[i].x,obstacles[i].y,obstacle_w,obstacle_h)) {
                    ball.x = coords_ball.x;
                    vx = -vx;
                    break;
                }
            }

            coords_ball = ball;
            ball.y += vy;
            if (ball.y <= 0 || ball.y+BALL_SIZE >= SCREEN_HEIGHT) {
                ball.y = coords_ball.y;
                vy = -vy;
            }
            for (int i = 0; i < obstacle_count; i++) {
                if (box_collider(ball.x,ball.y,BALL_SIZE,BALL_SIZE, obstacles[i].x,obstacles[i].y,obstacle_w,obstacle_h)) {
                    ball.y = coords_ball.y;
                    vy = -vy;
                    break;
                }
            }

            if (box_collider(ball.x,ball.y,BALL_SIZE,BALL_SIZE, p1.x,p1.y,PLAYERS_SIZE,PLAYERS_SIZE)) {
                vx = (ball.x < p1.x ? -BALL_SPEED : BALL_SPEED);
                vy = (ball.y < p1.y ? -BALL_SPEED : BALL_SPEED);
                last = 1;
                playBeep(SYS_PLAY_BEEP, 40, 80);
            }
            if (box_collider(ball.x,ball.y,BALL_SIZE,BALL_SIZE, p2.x,p2.y,PLAYERS_SIZE,PLAYERS_SIZE)) {
                vx = (ball.x < p2.x ? -BALL_SPEED : BALL_SPEED);
                vy = (ball.y < p2.y ? -BALL_SPEED : BALL_SPEED);
                last = 2;
                playBeep(SYS_PLAY_BEEP, 60, 80);
            }

            if (box_collider(ball.x,ball.y,BALL_SIZE,BALL_SIZE, hole.x,hole.y,HOLE_SIZE,HOLE_SIZE)) {
                playBeep(SYS_PLAY_BEEP, 600, 90);
                if (last == 1) score1++;
                else if (last == 2) score2++;
                levelWon = 1;
            }

            clearScreen();
            draw_score(score1, score2);

            _sys_drawRect(SYS_DRAW_RECT, 0x00FF00, p1.x, p1.y, PLAYERS_SIZE, PLAYERS_SIZE);
            _sys_drawRect(SYS_DRAW_RECT, 0xFF0000, p2.x, p2.y, PLAYERS_SIZE, PLAYERS_SIZE);
            _sys_drawRect(SYS_DRAW_RECT, 0xFFFFFF, ball.x, ball.y, BALL_SIZE, BALL_SIZE);
            _sys_drawRect(SYS_DRAW_RECT, 0x0000FF, hole.x, hole.y, HOLE_SIZE, HOLE_SIZE);
            
            for (int i = 0; i < obstacle_count; i++) {
                //ponemos los obstaculos segun el nivel
                _sys_drawRect(SYS_DRAW_RECT, 0x888888,
                              obstacles[i].x, obstacles[i].y,
                              obstacle_w, obstacle_h);
            }

            _sys_sleep(SYS_SLEEP, 2);
        }
        if (!running) break;
    }
    if (running) show_victory(last, players);
}