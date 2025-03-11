/* This is the implementation of the game IO using ncurses,
 * while the game logic resides in main.c
 *
 * by Andrea Malatesti and Simone Guercini
 * Released under CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/)
 * plus a waiver of all other intellectual property. The goal of this work is to
 * be and remain completely in the public domain forever, available for any use
 * whatsoever.
 */


#include <curses.h>
#include "./io_api.h"
static int vis;
int init_game(char const * /**title*/, int /*size*/, int /*sidebar*/) {
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, 1);
  vis = curs_set(0);
  return 1;
}

void close_game(void) {
  endwin();
}


void draw_ship(int x, int y) {
  mvaddch(y, x, '@');
}

void draw_base(int x, int y, int frame) {
  mvaddch(y, x, "Bbhb"[frame]);
}

void draw_mineral(int x, int y, int frame) {
  mvaddch(y, x, "Mm#;"[frame]);
} 

void draw_laser(int x, int y, int frame) {
  mvaddch(y, x, ":/-\\"[frame]);
} 

void draw_explosion(int x, int y, int frame) {
  mvaddch(y, x, "oO#%"[frame]);
} 

void draw_space(void) {
  clear();
}

void draw_asteroid(int x, int y) {
  mvaddch(y, x, '0');
}

void screen_refresh(void) {
  refresh();
  napms(50);
}

enum Input get_input(void) {
  enum Input action = NONE;
  int in;
  while((in = getch()) != ERR) {
    switch(in) {
    case 'w':
    case 'k':
      action = UP;
      break;
    case 'a':
    case 'h':
      action = LEFT;
      break;
    case 's':
    case 'j':
      action = DOWN;
      break;
    case 'd':
    case 'l':
      action = RIGHT;
      break;
    case '\n':
      action = BUTTON_A;
      break;
    case ' ':
      action = BUTTON_B;
      break;
    case 'q':
      action = QUIT;
      break;
    }
  }
  return action;
}

void draw_path(int x, int y, enum PathType type) {
  mvaddch(y, x, ".xX!&"[type]);
}

void draw_arrow(int x, int y, int angle) {
  mvaddch(y, x, ">v<^"[angle]);
}

void draw_text(int x, int y, char* str) {
  mvaddstr(y, x, str);
}
