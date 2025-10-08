
#ifndef LOGGING_H
#define LOGGING_H

#include <stdlib.h> 

#include <stdio.h>


#include <stdarg.h>

#ifdef __linux__
  #include <ncurses.h>
#else
  #include <ncursesw/curses.h>
#endif

static inline char * format_string(char* fmt, ...){
  
  if (!fmt) 
    return NULL; // Check for null pointer reference on format string
  
  va_list args;
  va_start(args, fmt);
  int needed = vsnprintf(NULL, 0, fmt, args) + 1;
  va_end(args); // Reset args before using it in another vsnprintf
  
  if (needed <= 0) 
    return NULL; // Check for vsnprintf failure
  
  char *buf = malloc(needed);
  if (!buf) 
    return NULL; // Check for memory allocation failure
  
  va_start(args, fmt); // Restart args for the actual formatting
  vsnprintf(buf, needed, fmt, args);
  va_end(args);
  return buf;
}

#define DEBUG_CURSES(fmt, ...)                          \
do {                                                    \
  if (stdscr) {                                         \
      char * buf = format_string(fmt, ##__VA_ARGS__);   \
      mvaddstr(getmaxy(stdscr) - 2, 2, buf);            \
      wrefresh(stdscr);                                 \
      free(buf);                                        \
  }                                                     \
} while(0)                                              \

#define TRACE_MODE 1

#if TRACE_MODE
  #define TRACE(fmt, ...)                                                        \
    do {                                                                         \
      fprintf(stderr, "TRACE: " fmt "\n", ##__VA_ARGS__);                        \
      fflush(stderr);                                                            \
      DEBUG_CURSES(fmt, ##__VA_ARGS__);                                          \
    } while (0)
#else
  #define TRACE(fmt, ...)
#endif

#endif /* LOGGING_H */
