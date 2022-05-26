#include <libc.h>
#define MAX_LENGTH 25 * 80

char buff[24];

int pid;

enum Direction {
  UP,
  RIGHT,
  DOWN,
  LEFT
};

struct Position {
  int x;
  int y;
};

char (*screen)[80][2];

enum Direction snake[MAX_LENGTH];
struct Position snake_position;

int check_death(struct Position pos, char screen[25][80][2]) {
  if (pos.y >= 25 || pos.y < 0 || pos.x >= 80 || pos.x < 0 || screen[pos.y][pos.x][0] == '@')
    return 1;
  else
    return 0;
}

void print_game_over() {
  char *over = "Game Over";
  for (int i = i; i < 11; ++i) {
    screen[12][i + 35][0] = over[i];
    screen[12][i + 35][1] = 4;
  }
}

void print_food(char screen[25][80][2], struct Position food_position) {
  screen[food_position.y][food_position.x][0] = 'c';
  screen[food_position.y][food_position.x][1] = 12;
}

void reset_screen(char screen[25][80][2]) {
  for (int i = 0; i < 25; ++i)
    for (int j = 0; j < 80; ++j)
      screen[i][j][0] = '\0';
}

int print_snake(char screen[25][80][2], struct Position food_position, int length, int *eaten) {
  struct Position pos = snake_position;

  int game_over = check_death(pos, screen);
  if (game_over)
    return 1;

  if (pos.x == food_position.x && pos.y == food_position.y)
    *eaten = 1;

  screen[pos.y][pos.x][0] = '@';
  screen[pos.y][pos.x][1] = 2;

  for (int i = 1; i < length; ++i) {
    switch (snake[i]) {
      case UP:
      ++pos.y;
      break;
    case RIGHT:
      --pos.x;
      break;
    case DOWN:
      --pos.y;
      break;
    case LEFT:
      ++pos.x;
      break;
    }

    game_over = check_death(pos, screen);
    if (game_over)
      return 1;

    screen[pos.y][pos.x][0] = '@';
    screen[pos.y][pos.x][1] = 2;
  }

  return 0;
}

void register_input() {
  char c;
  if (get_key(&c) >= 0) {
    if (snake[0] != UP && snake[0] != DOWN) {
      if (c == 'w')
        snake[0] = UP;
      else if (c == 's')
        snake[0] = DOWN;
    } else if (snake[0] != RIGHT && snake[0] != LEFT) {
      if (c == 'd')
        snake[0] = RIGHT;
      else if (c == 'a')
        snake[0] = LEFT;
    }
  }
}

void screen_callback(char *screen_buffer) {
  char (*buffer)[80][2] = (char (*)[80][2]) screen_buffer;
  for (int i = 0; i < 25; ++i)
    for (int j = 0; j < 80; ++j)
      for (int k = 0; k < 2; ++k)
        buffer[i][j][k] = screen[i][j][k];
}

void screen_callback_wrapper(void (*callback_function)(char *), char *screen_buffer) {
  callback_function(screen_buffer);
  return_from_callback();
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  system_info_screen_callback_wrapper(screen_callback_wrapper);

  screen = (char (*)[80][2]) get_screen();

  int aux = 0;
  int time = gettime();

  int length = 1;
  snake_position.x = 39;
  snake_position.y = 12;
  snake[0] = LEFT;

  int eaten = 0;
  struct Position food_position;
  food_position.x = time * 231 % 79;
  food_position.y = time * 42 % 24;

  int game_over = 0;

  set_screen_callback(screen_callback);
  while (!game_over) {
    time = gettime();
    if (eaten) {
      ++length;
      snake[length - 1] = snake[length - 2];
      food_position.x = time * 231 % 79;
      food_position.y = time * 1001 % 24;
      eaten = 0;
    }

    register_input();

    if (time % 80 == 0 && aux != time) {
      enum Direction dir = snake[0];
      switch (dir) {
        case UP:
          --snake_position.y;
          break;
        case RIGHT:
          ++snake_position.x;
          break;
        case DOWN:
          ++snake_position.y;
          break;
        case LEFT:
          --snake_position.x;
          break;
      }

      char (*aux_screen)[80][2] = get_screen();

      for (int i = 1; i < length; ++i)
        snake[length - i] = snake[length - i - 1];

      reset_screen(aux_screen);

      print_food(aux_screen, food_position);
      game_over = print_snake(aux_screen, food_position, length, &eaten);

      char (*tmp_screen)[80][2] = screen;
      screen = aux_screen;
      remove_screen(tmp_screen);

      aux = time;
    }
  }

  print_game_over();

  while (1) { }
}
