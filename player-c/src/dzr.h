
#ifdef __linux__
#include <form.h>
#include <menu.h>
#include <ncurses.h>
#include <panel.h>
#else
#include <ncurses/curses.h>
#include <ncurses/form.h>
#include <ncurses/menu.h>
#include <ncurses/panel.h>
#include <curl/curl.h>
#endif

#define DEBUG

#ifdef DEBUG
#define LOG(fmt, ...) \
    do { \
        fprintf(stderr, "DEBUG: " fmt "\n\n", ##__VA_ARGS__); \
        fflush(stderr); \
    } while (0)
#else
#define LOG(fmt, ...)
#endif

#define EXIT 17

#define CTRL_D 4

#define NCURSES_TRACE 1

void logging(char t, char *str, ...);

#define LOGGING(str, ...) logging('L', str, ##__VA_ARGS__);
#define COMMAND(str, ...) logging('C', str, ##__VA_ARGS__);

#define INIT_CURSES()                                                          \
    do {                                                                       \
        initscr();                                                             \
        noecho();                                                              \
        raw();                                                                 \
        keypad(stdscr, TRUE);                                                  \
        layout = (screen_layout_t *)calloc(1, sizeof(screen_layout_t));        \
        getmaxyx(stdscr, layout->yMax, layout->xMax);                          \
        layout->xDiv = layout->xMax / 4;                                       \
        layout->yDiv = layout->yMax - 3;                                       \
        LOGGING("");                                                           \
    } while (0);                                                               \

#define CHECK_WINDOW(x)                                                        \
    do {                                                                       \
        if (!(x)) {                                                            \
            endwin();                                                          \
            LOG("Unable to create window %s", #x);                             \
            exit(1);                                                           \
        }                                                                      \
    } while (0);                                                               \

typedef struct window_t window_t;

typedef struct {
    int xMax;
    int yMax;
    int xDiv;
    int yDiv;
} screen_layout_t;

screen_layout_t *layout;
