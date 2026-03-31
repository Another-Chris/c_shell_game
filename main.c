#include "main.h"

void reset_term() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
  printf(SHOW_CURSOR); // show cursor
}

void cleanup() {
  reset_term();
  exit(0);
}

void init_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_term);
  atexit(reset_term);
  signal(SIGINT, cleanup);
  struct termios raw = orig_term;
  raw.c_lflag &= ~(ECHO | ICANON);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int get_key() {
  fd_set fds;
  struct timeval tv = {0, 0};
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
      return c;
    }
  }
  return -1;
}

long get_ticks_ms() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void move_cursor(int row, int col) { printf(CURSOR_POS, row, col); }

void render(Snake snake) {
  printf(CLEAR_SCREEN);

  for (int i = 0; i < snake.curr_len; ++i) {
    move_cursor(snake.body[i][0], snake.body[i][1]);
    switch (snake.curr_dir) {
    case 'R':
      printf(">");
      break;
    case 'L':
      printf("<");
      break;
    case 'U':
      printf("^");
      break;
    case 'D':
      printf("v");
      break;
    }
  }
  fflush(stdout);
}

void init_snake(Snake *snake) {
  snake->curr_len = 1;
  snake->curr_dir = 'R';
  snake->body[0][0] = 0;
  snake->body[0][1] = 0;
}

void update(Snake *snake) {
  switch (snake->curr_dir) {
  case 'R':
    snake->body[0][1]++;
    break;
  case 'L':
    snake->body[0][1]--;
    break;
  case 'U':
    snake->body[0][0]--;
    break;
  case 'D':
    snake->body[0][0]++;
    break;
  }
}

int main() {
  init_raw_mode();

  Snake snake = {0};
  init_snake(&snake);

  long prev = get_ticks_ms();
  float dt_acc = 0;
  float time_thre = 300;

  printf(HIDE_CURSOR);  // hide cursor
  printf(CLEAR_SCREEN); // clear screen
  fflush(stdout);

  move_cursor(0, 0);
  printf(BLOCK);
  fflush(stdout);

  int running = 1;
  while (running) {

    long now = get_ticks_ms();
    float dt = now - prev;
    prev = now;
    dt_acc += dt;

    int key = get_key();

    //TODO: handle escape sequence of input
    switch(key) {
      case 'a':
        snake.curr_dir = 'L';
        break;
      case 'd':
        snake.curr_dir = 'R';
        break;
      case 'w':
        snake.curr_dir = 'U';
        break;
      case 's':
        snake.curr_dir = 'D';
        break;
    }

    if (dt_acc > time_thre) {
      update(&snake);
      render(snake);
      dt_acc = 0;
    }
  }

  return 0;
}
