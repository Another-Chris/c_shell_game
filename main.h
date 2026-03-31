
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

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

//=== global settings
#define MAX_SNAKE_LENGTH 64

typedef struct {
  int body[MAX_SNAKE_LENGTH][2];
  int curr_len;
  int curr_dir;
} Snake;


