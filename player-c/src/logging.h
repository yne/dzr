#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __linux__
  #include <ncurses.h>
#else
  #include <ncursesw/curses.h>
#endif

static inline void DEBUG_CURSES(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int needed = vsnprintf(NULL, 0, fmt, args) + 1;
  char *buf = malloc(needed);
  if (!buf) {
    va_end(args);
    return;
  }
  vsnprintf(buf, needed, fmt, args);
  mvaddstr(getmaxy(stdscr) - 2, 2, buf);
  va_end(args);
  wrefresh(stdscr);
}

#define TRACE_MODE 1

#if TRACE_MODE
  #define TRACE(fmt, ...)                                                        \
    do {                                                                         \
      fprintf(stderr, "TRACE: " fmt "\n", ##__VA_ARGS__);                        \
      fflush(stderr);                                                            \
      /*LOG("DEBUG: " fmt , ##__VA_ARGS__);*/                                    \
    } while (0)
#else
  #define TRACE(fmt, ...)
#endif


#endif /* LOGGING_H */
