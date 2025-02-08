#ifndef DZR_H
#define DZR_H

#ifdef __linux__
  #include <ncurses.h> 
  #include <menu.h>
  #include <panel.h>
#else
  #include <ncursesw/curses.h>
  #include <ncursesw/form.h>
  #include <ncursesw/menu.h>
  #include <ncursesw/panel.h>
#endif



#include <stdio.h>
#include <stdlib.h>

#include <locale.h>

#include "logging.h"

#define STRINGFY(x) x

#define CHECK_WINDOW(x)                                                        \
  do {                                                                         \
    if (!(x)) {                                                                \
      endwin();                                                                \
      DEBUG("Unable to create window %s", #x);                                 \
      exit(1);                                                                 \
    }                                                                          \
  } while (0);

typedef struct window_t {
  WINDOW *window;
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

static inline int yMax(){
  return getmaxy(stdscr);
}

static inline int xMax(){
  return getmaxx(stdscr);
}

static inline int yDiv(){
  return getmaxy(stdscr) - 3;
}

static inline int xDiv(){
  return getmaxx(stdscr) / 4;
}

#endif // DZR_H