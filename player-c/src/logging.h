#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

#define DEBUG_MODE 1

#if DEBUG_MODE
#define DEBUG(fmt, ...)                                                        \
  do {                                                                         \
    fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__);                        \
    fflush(stderr);                                                            \
  } while (0)
#else
#define DEBUG(fmt, ...)
#endif

#endif /* LOGGING_H */
