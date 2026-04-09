
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

struct termios orig_term;

//=== escape sequences
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define CLEAR_SCREEN "\033[2J"
#define BLOCK "\u2588"
#define CURSOR_POS "\033[%d;%dH"
#define UP "\033[A"
#define DOWN "\033[B"
#define RIGHT "\033[C"
#define LEFT "\033[D"

//=== unicode characters
#define TOP_LEFT "\u250C"   
#define TOP_RIGHT "\u2510"
#define BOTTOM_LEFT "\u2514"
#define BOTTOM_RIGHT "\u2518" 
#define LINE_HORIZONTAL "\u2500"
#define LINE_VERTICAL "\u2502"


//=== global settings
#define MAX_SNAKE_LENGTH 64
#define MAX_FOOD 64
#define WORLD_WIDTH 64  // columns
#define WORLD_HEIGHT 16 // rows

typedef struct {
  int body[MAX_SNAKE_LENGTH][2];
  char* body_chars[MAX_SNAKE_LENGTH];
  char next_dir;
  char curr_dir;
  int curr_len;
} Snake;


typedef struct {
  int curr_foods;
  int coords[MAX_FOOD][2];
} Food;

void gen_food(Food* food);

typedef struct {
  Snake snake;
  Food food;
  int width;
  int height;
  int top_left[2];
  int score;
  int game_over;
} World;

