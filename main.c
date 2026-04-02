#include "main.h"

void gen_food_coords(World* world, int* fx, int* fy) {
  int width = world->width;
  int height = world->height;

  // [0, height-1]
  int randx = rand() % height; 
  int randy = rand() % width;

  if (randx == 0) {
    randx = 1;
  }
  if (randy == 0) {
    randy = 1;
  }
  
  for (int i = 0; i < world->snake.curr_len; ++i) {
    int sx = world->snake.body[i][0];
    int sy = world->snake.body[i][1];
    if (sx == randx && sy == randy) {
      gen_food_coords(world, fx, fy);
      return;
    }
  }

  *fx = randx;
  *fy = randy;
}

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
  move_cursor(topx + height, topy);
  printf(BOTTOM_LEFT);
  move_cursor(topx, topy + width);
  printf(TOP_RIGHT);
  move_cursor(topx + height, topy + width);
  printf(BOTTOM_RIGHT);

  for (int row = 1; row < height; ++row) {
    move_cursor(topx + row, topy);
    printf(LINE_VERTICAL);
    move_cursor(topx + row, topy + width);
    printf(LINE_VERTICAL);
  }

  for (int col = 1; col < width; ++col) {
    move_cursor(topx, topy + col);
    printf(LINE_HORIZONTAL);
    move_cursor(topx + height, topy + col);
    printf(LINE_HORIZONTAL);
  }

  //=== food
  for (int i = 0; i < world.food.curr_foods; ++i) {
    int fx = world.food.coords[i][0];
    int fy = world.food.coords[i][1];
    move_cursor(topx + fx + 1, topy + fy + 1);
    printf("*");
  }


  //=== snake
  Snake snake = world.snake;
  for (int i = 0; i < snake.curr_len; ++i) {
    move_cursor(topx + snake.body[i][0] + 1, topy + snake.body[i][1] + 1);
    printf("%s", snake.body_chars[i]);
  }
  fflush(stdout);
}

void update(World *world) {

  // determine the char && position
  int head_next[2] = {0};
  head_next[0] = world->snake.body[0][0];
  head_next[1] = world->snake.body[0][1];
  switch (world->snake.next_dir) {
  case 'R':
    world->snake.body_chars[0] = ">";
    head_next[1]++;
    break;
  case 'L':
    world->snake.body_chars[0] = "<";
    head_next[1]--;
    break;
  case 'U':
    world->snake.body_chars[0] = "^";
    head_next[0]--;
    break;
  case 'D':
    world->snake.body_chars[0] = "v";
    head_next[0]++;
    break;
  }

  // eat food
  for (int i = 0; i < world->food.curr_foods; ++i) {
    int fx = world->food.coords[i][0];
    int fy = world->food.coords[i][1];
    if (head_next[0] == fx && head_next[1] == fy) {
      world->snake.curr_len++;
      int new_fx, new_fy;
      gen_food_coords(world, &new_fx, &new_fy);
      world->food.coords[i][0] = new_fx;
      world->food.coords[i][1] = new_fy;
    }
  }

  // move chars
  for (int i = world->snake.curr_len - 1; i >= 2; --i) {
    world->snake.body_chars[i] = world->snake.body_chars[i - 1];
  }

  // change snake characters
  char curr_dir = world->snake.curr_dir;
  char next_dir = world->snake.next_dir;
  if (world->snake.curr_len > 1) {
    if (curr_dir != next_dir) {
      switch (curr_dir) {
      case 'R':
        if (next_dir == 'U') {
          world->snake.body_chars[1] = BOTTOM_RIGHT;
        } else if (next_dir == 'D') {
          world->snake.body_chars[1] = TOP_RIGHT;
        }
        break;
      case 'D':
        if (next_dir == 'L') {
          world->snake.body_chars[1] = BOTTOM_RIGHT;
        } else if (next_dir == 'R') {
          world->snake.body_chars[1] = BOTTOM_LEFT;
        }
        break;
      case 'U':
        if (next_dir == 'L') {
          world->snake.body_chars[1] = TOP_RIGHT;
        } else if (next_dir == 'R') {
          world->snake.body_chars[1] = TOP_LEFT;
        }
        break;
      case 'L':
        if (next_dir == 'D') {
          world->snake.body_chars[1] = TOP_LEFT;
        } else if (next_dir == 'U') {
          world->snake.body_chars[1] = BOTTOM_LEFT;
        }
        break;
      }
    } else {
      if (curr_dir == 'R' || curr_dir == 'L') {
        world->snake.body_chars[1] = LINE_HORIZONTAL;
      } else if (curr_dir == 'U' || curr_dir == 'D') {
        world->snake.body_chars[1] = LINE_VERTICAL;
      }
    }
  }

  // change current snake directions
  world->snake.curr_dir = world->snake.next_dir;

  // change positions for each body
  int *tail = NULL;
  int *head = NULL;
  for (int i = world->snake.curr_len - 1; i >= 1; --i) {
    tail = world->snake.body[i];
    head = world->snake.body[i - 1];
    tail[0] = head[0];
    tail[1] = head[1];
  }

  head[0] = head_next[0];
  head[1] = head_next[1];
}



void init_world(World *world) {

  // arena
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    perror("ioctl");
  }

  world->width = WORLD_WIDTH;
  world->height = WORLD_HEIGHT;
  world->top_left[0] = (w.ws_row - WORLD_HEIGHT)/2;
  world->top_left[1] = (w.ws_col - WORLD_WIDTH)/2;

  // snake
  world->snake.curr_dir = 'R';
  world->snake.next_dir = 'R';
  world->snake.body_chars[0] = ">";
  world->snake.body_chars[1] = LINE_VERTICAL;
  world->snake.body[0][0] = 0;
  world->snake.body[0][1] = 1;
  world->snake.body[1][0] = 0;
  world->snake.body[1][0] = 0;
  world->snake.curr_len = 2;

  // foods
  world->food.curr_foods = 2;

  int fx, fy;
  gen_food_coords(world, &fx, &fy);
  world->food.coords[0][0] = fx;
  world->food.coords[0][1] = fy;
  gen_food_coords(world, &fx, &fy);
  world->food.coords[1][0] = fx;
  world->food.coords[1][1] = fy;
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

    // TODO: handle escape sequence of input
    switch (key) {
    case 'a':
      world.snake.next_dir = 'L';
      break;
    case 'd':
      world.snake.next_dir = 'R';
      break;
    case 'w':
      world.snake.next_dir = 'U';
      break;
    case 's':
      world.snake.next_dir = 'D';
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
