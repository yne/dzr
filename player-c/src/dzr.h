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

typedef struct {
    Menu_Options on;
    Menu_Options off;
} Menu_Options_Seeting;

typedef struct command_t {
    int key;
    void (* func)(va_list args);
} command_t; 

static inline void init_curses() {
  initscr();
  noecho();
  raw();
  keypad(stdscr, TRUE);
  setlocale(LC_ALL, "");
  TRACE("Curses initialized ...");
}


#define CTRL_D  4
#define COMMAND  ':'
#define UP KEY_UP
#define DOWN KEY_DOWN
#define LEFT KEY_LEFT
#define RIGHT KEY_RIGHT
#define ENTER 10
#define PAGE_UP KEY_NPAGE
#define PAGE_DOWN KEY_PPAGE
#define HOME KEY_HOME
#define END KEY_END
#define SELECT ' '


#endif // DZR_H