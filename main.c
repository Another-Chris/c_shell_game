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

void render(World world) {
  printf(CLEAR_SCREEN);

  //=== arena
  int topx = world.top_left[0];
  int topy = world.top_left[1];
  int width = world.width;
  int height = world.height;

  move_cursor(topx, topy);
  printf(TOP_LEFT);
  move_cursor(topx+height, topy);
  printf(BOTTOM_LEFT);
  move_cursor(topx, topy+width);
  printf(TOP_RIGHT);
  move_cursor(topx+height, topy+width);
  printf(BOTTOM_RIGHT);

  for (int row = 1; row < height; ++row) {
    move_cursor(topx+row, topy);
    printf(LINE_VERTICAL);
    move_cursor(topx+row, topy+width);
    printf(LINE_VERTICAL);
  }

  for (int col = 1; col < width; ++col) {
    move_cursor(topx, topy+col);
    printf(LINE_HORIZONTAL);
    move_cursor(topx+height, topy+col);
    printf(LINE_HORIZONTAL);
  }

  //=== food
  move_cursor(topx+world.food[0], topy+world.food[1]);
  printf("*");

  //=== snake
  Snake snake = world.snake;
  for (int i = 0; i < snake.curr_len; ++i) {
    move_cursor(topx+snake.body[i][0]+1, topy+snake.body[i][1]+1);
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


void update(World *world) {
  switch (world->snake.curr_dir) {
  case 'R':
    world->snake.body[0][1]++;
    break;
  case 'L':
    world->snake.body[0][1]--;
    break;
  case 'U':
    world->snake.body[0][0]--;
    break;
  case 'D':
    world->snake.body[0][0]++;
    break;
  }
}

void init_world(World* world) {

  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    perror("ioctl");
  }

  world->width = w.ws_col - 8;
  world->height = w.ws_row - 4;
  world->top_left[0] = 2;
  world->top_left[1] = 4;

  world->snake.curr_len = 1;
  world->snake.curr_dir = 'R';
  world->snake.body[0][0] = 0;
  world->snake.body[0][1] = 0;

  world->food[0] = WORLD_HEIGHT / 2;
  world->food[1] = WORLD_WIDTH / 2;
}






int main() {
  init_raw_mode();

  World world = {0};
  init_world(&world);

  long prev = get_ticks_ms();
  float dt_acc = 0;
  float time_thre = 300;

  printf(HIDE_CURSOR);  // hide cursor
  printf(CLEAR_SCREEN); // clear screen
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
        world.snake.curr_dir = 'L';
        break;
      case 'd':
        world.snake.curr_dir = 'R';
        break;
      case 'w':
        world.snake.curr_dir = 'U';
        break;
      case 's':
        world.snake.curr_dir = 'D';
        break;
    }

    if (dt_acc > time_thre) {
      update(&world);
      render(world);
      dt_acc = 0;
    }
  }

  return 0;
}
