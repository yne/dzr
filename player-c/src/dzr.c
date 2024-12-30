#ifndef DZR_H
#define DZR_H

#include "dzr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ncurses/curses.h>
#include <ncurses/form.h>
#include <ncurses/menu.h>
#include <ncurses/panel.h>


#define EXIT 17

#define CTRL_D 4

#define NCURSES_TRACE 1

#define INIT_CURSES() initscr();  noecho();  raw();

#define CHECK_WINDOW(x) if(!(x)) { endwin(); exit(1); }

static inline WINDOW *create_win(int y, int x, int starty, int startx) {
    WINDOW *win = newwin(y, x, starty, startx);
    box(win, 0, 0);
    refresh();
    wrefresh(win);
    return win;
}

static inline WINDOW *create_subwin(WINDOW *parent, int y, int x, int starty, int startx) {
    WINDOW *win = subwin(parent, y, x, starty, startx);
    box(win, 0, 0);
    refresh();
    wrefresh(win);
    return win;
}

WINDOW *logWin;

void addLabel(WINDOW *win, char *str) {
    char *nstr = malloc(sizeof(char) * (strlen(str) + 2));
    sprintf(nstr, "| %s |", str); 
    mvwaddstr(win, 0, 1, nstr);
    wrefresh(win);
    free(nstr);
}

void logging(char *str) { 
    wclear(logWin);
    box(logWin, 0, 0);
    addLabel(logWin, "Logging");
    mvwaddstr(logWin, 1, 3, str);
    wrefresh(logWin);
}

int main(int argc, char **argv) {
    // init screen and sets up screen
    INIT_CURSES();


    int yMax,xMax;
    getmaxyx(stdscr, yMax, xMax);

    int xDiv = xMax / 4;
    int yDiv = yMax - 3;
    CHECK_WINDOW(logWin = create_win(yMax - yDiv, xMax - xDiv, yDiv, xDiv));
    logging("");

    WINDOW *playlist;
    CHECK_WINDOW(playlist = create_win(yMax, xDiv, 0, 0));
    WINDOW *searchMenu;
    CHECK_WINDOW(searchMenu = create_win(yDiv, xMax - xDiv, 0, xDiv));
    WINDOW *searchInput;
    CHECK_WINDOW(searchInput = create_subwin(searchMenu, 3, getmaxx(searchMenu) - 2, 1, xDiv + 1));

    addLabel(playlist, "Playlist");
    addLabel(searchMenu, "Painel");
    
    addLabel(searchInput, "Search (track[t]/artist[a]/album[b]/playlist[p]/user[u]/radio[r])");

    int ch;
    
    MENU *menu;
    CHECK_WINDOW(menu = new_menu((ITEM **)NULL));

    while ((ch = getch()) != CTRL_D){
        if(ch == ':'){
            ch = getch();
            switch(ch) {
                case 't': logging("track"); break;
                case 'a': logging("artist"); break;
                case 'b': logging("album"); break;
                case 'p': logging("playlist"); break;
                case 'u': logging("user"); break;
                case 'r': logging("radio"); break;
                default: break;
            }   
        }
    }

    delwin(playlist);
    delwin(searchMenu);
    delwin(logWin);
    delwin(searchInput);

    endwin();
    exit(0);
    return 0;
}

#endif // DZR_H