#ifndef DZR_H
#define DZR_H

#ifdef __linux__
  #include <formw.h>
  #include <menuw.h>
  #include <ncursesw.h>
  #include <panelw.h>
#else
  #include <ncursesw/curses.h>
  #include <ncursesw/form.h>
  #include <ncursesw/menu.h>
  #include <ncursesw/panel.h>
#endif

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>

#include <locale.h>

#define STRINGFY(x) x

#define DEBUG 1

#if DEBUG
#define LOG(fmt, ...)                                                          \
  do {                                                                         \
    fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__);                        \
    fflush(stderr);                                                            \
  } while (0)
#else
#define LOG(fmt, ...)
#endif

static void logging(char t, char *str, ...);

#define LOGGING(str, ...) logging('L', str, ##__VA_ARGS__);
#define COMMAND(str, ...) logging('C', str, ##__VA_ARGS__);

#define CHECK_WINDOW(x)                                                        \
  do {                                                                         \
    if (!(x)) {                                                                \
      endwin();                                                                \
      LOG("Unable to create window %s", #x);                                   \
      exit(1);                                                                 \
    }                                                                          \
  } while (0);

typedef struct window_t window_t;

typedef struct {
  int xMax;
  int yMax;
  int xDiv;
  int yDiv;
} screen_layout_t;

typedef struct {
    char * data;
    size_t size;
} buffer_t;

screen_layout_t *layout;

static inline void init_curses() {
  initscr();
  noecho();
  raw();
  keypad(stdscr, TRUE);
  layout = (screen_layout_t *) calloc(1, sizeof(screen_layout_t));
  getmaxyx(stdscr, layout->yMax, layout->xMax);
  layout->xDiv = layout->xMax / 4;
  layout->yDiv = layout->yMax - 3;
  LOGGING("");
  COMMAND("");
  setlocale(LC_ALL, "");
}
#endif // DZR_H