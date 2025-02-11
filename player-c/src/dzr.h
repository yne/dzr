#ifndef DZR_H
#define DZR_H

#ifdef __linux__
  #include <ncurses.h> 
  #include <menu.h>
  #include <panel.h>
#else
  #include <ncursesw/curses.h>
  #include <ncursesw/menu.h>
  #include <ncursesw/panel.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <locale.h>

#include "logging.h"


typedef struct window_t {
  WINDOW *window;
  PANEL *panel;
  MENU *menu;
  ITEM **items;
  int y;
  int x;
  int starty;
  int startx;
  char label[15];
} window_t;

static inline void init_curses() {
  initscr();
  noecho();
  raw();
  keypad(stdscr, TRUE);
  setlocale(LC_ALL, "");
}

#endif // DZR_H