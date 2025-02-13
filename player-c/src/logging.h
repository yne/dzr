#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdarg.h>

#ifdef __linux__
  #include <ncurses.h>
#else
  #include <ncursesw/curses.h>
#endif

static inline void LOG(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int needed = vsnprintf(NULL, 0, fmt, args) + 1;
  char *buf = malloc(needed);
  if (!buf) {
    va_end(args);
    return;
  }
  vsnprintf(buf, needed, fmt, args);
  
  mvaddstr(getmaxy(stdscr), 2, buf);
  va_end(args);
  wrefresh(stdscr);
}

#define DEBUG_MODE 1

#if DEBUG_MODE
#define DEBUG(fmt, ...)                                                        \
  do {                                                                         \
    fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__);                        \
    fflush(stderr);                                                            \
    LOG("DEBUG: " fmt , ##__VA_ARGS__);                                        \
  } while (0)
#else
#define DEBUG(fmt, ...)
#endif


#endif /* LOGGING_H */
