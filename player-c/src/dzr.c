#ifndef DZR_H
#define DZR_H

#include "dzr.h"
#include <stdio.h>
#include <stdlib.h>

#include <ncurses/curses.h>
#include <ncurses/form.h>
#include <ncurses/menu.h>
#include <ncurses/panel.h>


#define EXIT 17

#define CTRL_D 4

#define NCURSES_TRACE 1

#define INIT_CURSES initscr();  noecho();  raw();

#define CHECK_WINDOW(x) if(!(x)) { endwin(); exit(1); }

inline WINDOW *create_win(int y, int x, int starty, int startx) {
    WINDOW *win = newwin(y, x, starty, startx);
    box(win, 0, 0);
    refresh();
    wrefresh(win);
    return win;
}

WINDOW *create_win(int y, int x, int starty, int startx);

WINDOW *logWin;

int main(int argc, char **argv) {
    // init screen and sets up screen
    INIT_CURSES;


    int yMax,xMax;
    getmaxyx(stdscr, yMax, xMax);

    int xDiv = xMax / 4;
    int yDiv = yMax - 3;
    
    WINDOW *playlist;
    CHECK_WINDOW(playlist = create_win(yDiv, xDiv, 0, 0));
    WINDOW *searchMenu;
    CHECK_WINDOW(searchMenu = create_win(yDiv, xMax - xDiv, 0, xDiv));
    // FIELD *searchField;
    // CHECK_WINDOW(searchField = new_field(1, xMax - xDiv, 0, xDiv, 0,0));
    // WINDOW *logging = create_win(yMax - hDiv, xMax, yMax - vDiv, xMax -
    // hDiv);
    // if (!logging) {
    //     endwin();
    //     exit(1);
    // }

    int ch;
    do {
    // wprintw(searchMenu, "Search menu %i", ch);
    // wrefresh(searchMenu);
        
    }while ((ch = getch()) != CTRL_D);

    delwin(playlist);
    delwin(searchMenu);
    // delwin(logging);

    endwin();
    exit(0);
    return 0;
}

#endif // DZR_H