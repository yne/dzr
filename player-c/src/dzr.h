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


typedef struct window_t {
  PANEL *panel;
  MENU *menu;
  int y;
  int x;
  int starty;
  int startx;
  char label[15];
} window_t;

typedef struct {
    Menu_Options on;
    Menu_Options off;
} Menu_Options_t;

const Menu_Options_t GLOBAL_MENU_OPTIONS = {
  .on = O_NONCYCLIC,
  .off = O_SHOWDESC | O_ONEVALUE
};

const Menu_Options_t GLOBAL_PLAYLIST_OPTIONS = {
  .on = O_ONEVALUE | O_NONCYCLIC,
  .off = O_SHOWDESC
};

typedef struct command_t {
    int key;
    void (* func)(va_list args);
} command_t; 

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