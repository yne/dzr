#ifndef DZR_H
#define DZR_H

#include <iostream>
#include "dzr.h"
#include <ncurses/curses.h>
#include <ncurses/panel.h>
#include <ncurses/form.h>
#include <ncurses/menu.h>



int main(int argc, char** argv) {
    // init screen and sets up screen
    initscr();
    noecho();
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    WINDOW* pad = newwin(10, 20, 0, 0);
    
    box(pad, '|', '-');
    refresh();
    // refreshes the screen

    // while (true) {
    //     int c = getch();
    //     printw("%c", c);
    //     cbreak();   
    //     refresh();
    // }
    getch();
    // deallocates memory and ends ncurses
    endwin();
    exit(0);
    return 0;
}

#endif // DZR_H