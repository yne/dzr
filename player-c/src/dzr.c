#ifndef DZR_H
#define DZR_H

#include "dzr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#endif

#define DEBUG

#ifdef DEBUG
#define LOG(fmt, ...) fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif

#define EXIT 17

#define CTRL_D 4

#define NCURSES_TRACE 1

#define INIT_CURSES()                                                          \
    do {                                                                       \
        initscr();                                                             \
        noecho();                                                              \
        raw();                                                                 \
        keypad(stdscr, TRUE);                                                  \
    } while (0);

#define CHECK_WINDOW(x)                                                        \
    do {                                                                       \
        if (!(x)) {                                                            \
            endwin();                                                          \
            exit(1);                                                           \
        }                                                                      \
    } while (0);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

WINDOW *create_win(char *label, int y, int x, int starty, int startx);
WINDOW *create_subwin(WINDOW *parent, char *label, int y, int x, int starty,
                      int startx);

void addLabel(WINDOW *win, char *str);
void logging(char *str, ...);

WINDOW *logWin;

int main(int argc, char **argv) {

    // init screen and sets up screen
    INIT_CURSES();

    // get screen size
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    int xDiv = xMax / 4;
    int yDiv = yMax - 3;

    // create windows
    CHECK_WINDOW(
        logWin = create_win("Logging", yMax - yDiv, xMax - xDiv, yDiv, xDiv));
    logging("");

    WINDOW *playlist;
    CHECK_WINDOW(playlist = create_win("Playlist", yMax, xDiv, 0, 0));
    WINDOW *searchMenu;
    CHECK_WINDOW(searchMenu = create_win("Painel", yDiv, xMax - xDiv, 0, xDiv));
    WINDOW *searchInput;
    CHECK_WINDOW(
        searchInput = create_subwin(
            searchMenu,
            "Search (track[t]/artist[a]/album[b]/playlist[p]/user[u]/radio[r])",
            3, getmaxx(searchMenu) - 2, 1, xDiv + 1));

    // create menus
    int nchoices = 10;
    ITEM **items = NULL;

    for (int i = 0; i < nchoices; i++) {
        items = (ITEM **)realloc(items, sizeof(ITEM *) * (i + 1));
        items[i] = new_item("abcd", "abcd");
    }
    items = (ITEM **)realloc(items, sizeof(ITEM *) * (nchoices + 1));
    items[nchoices] = NULL;

    MENU *menu = new_menu((ITEM **)items);

    set_menu_win(menu, playlist);
    set_menu_sub(menu, subwin(playlist, getmaxy(playlist) - 1,
                              getmaxx(playlist) - 1, 1, 1));
    set_menu_format(menu, 8, 1);
    set_menu_mark(menu, " * ");

    post_menu(menu);
    // refresh();
    wrefresh(playlist);

    int ch;
    while ((ch = getch()) != CTRL_D) {
        logging("ch: %d %c", ch, ch);
        switch (ch) {
        case KEY_DOWN:
            logging("down");
            menu_driver(menu, REQ_DOWN_ITEM);
            refresh();
            wrefresh(playlist);

            break;
        case KEY_UP:
            logging("up");
            menu_driver(menu, REQ_UP_ITEM);
            refresh();
            wrefresh(playlist);
            break;
        default:
            break;
        }
        if (ch == ':') {
            logging("command");
            ch = getch();
            switch (ch) {
            case 't':
                logging("track");
                char track[100];
                getstr(track);
                logging(track);
                // free(track);
                break;
            case 'a':
                logging("artist");
                break;
            case 'b':
                logging("album");
                break;
            case 'p':
                logging("playlist");
                break;
            case 'u':
                logging("user");
                break;
            case 'r':
                logging("radio");
                break;
            default:
                logging("invalid");
                break;
            }
        }
    }

    logging("exiting ...");

    delwin(playlist);
    delwin(searchMenu);
    delwin(logWin);
    delwin(searchInput);

    endwin();
    exit(0);
    return 0;
}

WINDOW *create_win(char *label, int y, int x, int starty, int startx) {
    WINDOW *win = newwin(y, x, starty, startx);
    box(win, 0, 0);
    addLabel(win, label);
    refresh();
    wrefresh(win);
    return win;
}

WINDOW *create_subwin(WINDOW *parent, char *label, int y, int x, int starty,
                      int startx) {
    WINDOW *win = subwin(parent, y, x, starty, startx);
    box(win, 0, 0);
    addLabel(win, label);
    refresh();
    wrefresh(win);
    return win;
}

void addLabel(WINDOW *win, char *str) {
    char *nstr = malloc(sizeof(char) * (strlen(str) + 2));
    sprintf(nstr, "| %s |", str);
    mvwaddstr(win, 0, 1, nstr);
    wrefresh(win);
    free(nstr);
}
void logging(char *str, ...) {
    va_list args;
    va_start(args, str);

    wclear(logWin);
    box(logWin, 0, 0);
    addLabel(logWin, "Logging");

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), str, args);
    mvwaddstr(logWin, 1, 3, buffer);
    wrefresh(logWin);

    LOG("%s", buffer);

    va_end(args);
}

#endif // DZR_H