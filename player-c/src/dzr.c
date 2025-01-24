#ifndef DZR_H
#define DZR_H

#include "dzr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct window_t {
    WINDOW *window;
    MENU *menu;
    int y;
    int x;
    int starty;
    int startx;
    char *label;
};

void create_win(window_t *w);

void create_menu(window_t *w, ITEM **items, Menu_Options options);

void destroy_menu(window_t *w, ITEM **items);

void drive_menu(window_t *w, int key);

void addLabel(WINDOW *win, char *str);
void free_window(window_t *w);

void type_search(window_t *win, char *str);

char * search_input(window_t *win);


// https://pubs.opengroup.org/onlinepubs/7908799/xcurses/intovix.html

int main(void) { // int argc, char **argv

    // init screen and sets up screen
    INIT_CURSES();

    window_t *playlist_w = calloc(1, sizeof(window_t));
    playlist_w->label = "Playlist";
    playlist_w->y = layout->yMax - 3;
    playlist_w->x = layout->xDiv;
    playlist_w->starty = 0;
    playlist_w->startx = 0;

    window_t *painel_w = calloc(1, sizeof(window_t));
    painel_w->label = "Painel";
    painel_w->y = layout->yDiv;
    painel_w->x = layout->xMax - layout->xDiv;
    painel_w->starty = 0;
    painel_w->startx = layout->xDiv;
    
    window_t *search_w = calloc(1, sizeof(window_t));
    search_w->label = "Search";
    search_w->y = 3;
    search_w->x = painel_w->x / 3;
    search_w->starty = 1;
    search_w->startx = painel_w->x - 2;
    

    create_win(playlist_w);
    
    create_win(painel_w);

    // create menus
    int nchoices = 100;
    ITEM **items = NULL;

    for (int i = 0; i < nchoices; i++) {
        items = (ITEM **) realloc(items, sizeof(ITEM *) * (i + 1));
        char *str = malloc(sizeof(char) * 20);
        sprintf(str, "%d item", i);
        items[i] = new_item(str, "abcd");
    }
    items = (ITEM **)realloc(items, sizeof(ITEM *) * (nchoices + 1));
    items[nchoices] = NULL;

    create_menu(painel_w, items, O_ONEVALUE);   

    // refresh();

    int ch;
    while ((ch = getch()) != CTRL_D) {
        LOGGING("key: %d char: %c", ch, ch);
        switch (ch) {
        case KEY_UP: {
            COMMAND("up");
            drive_menu(painel_w, REQ_UP_ITEM);
            break;
        }
        case KEY_DOWN: {
            COMMAND("down");
            drive_menu(painel_w, REQ_DOWN_ITEM);
            break;
        }
        case KEY_NPAGE: {
            COMMAND("previous page");
            drive_menu(painel_w, REQ_SCR_DPAGE);
            break;
        }
        case KEY_PPAGE: {
            COMMAND("next page");
            drive_menu(painel_w, REQ_SCR_UPAGE);
            break;
        }
        case KEY_LEFT: {
            COMMAND("left");
            break;
        }
        case KEY_RIGHT: {
            COMMAND("right");
            break;
        }
        case ' ': {
            COMMAND("backspace");
            ITEM *it = current_item(playlist_w->menu);
            int index = item_index(it);
            LOGGING("selecting item %d", index);
            menu_driver(playlist_w->menu, REQ_TOGGLE_ITEM);
            break;
        }
        case KEY_F(1): {
            COMMAND("f1");
            
            break;
        }
        case ':': {
            COMMAND("command");
            ch = getch();
            switch (ch) {
                case 't':{
                    COMMAND("track");
                    char* track = search_input(search_w);
                    LOGGING("track: %s", track);

                    break;
                }
                case 'a':{
                    COMMAND("artist");
                    break;
                }
                case 'b':{
                    COMMAND("album");
                    break;
                }
                case 'p':{
                    COMMAND("playlist");
                    break;
                }
                case 'u':{
                    COMMAND("user");
                    break;
                }
                case 'r':{
                    COMMAND("radio");
                    break;
                }
                default:{
                    COMMAND("invalid");
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
    }

    LOGGING("exiting ...");
    
    destroy_menu(playlist_w, items);
    free_window(playlist_w);
    free_window(painel_w);

    logging('0', NULL);

    endwin();
    exit(0);
    return 0;
}

void clear_and_write(window_t * w, char *str) {
    wclear(w->window);
    box(w->window, 0, 0);
    addLabel( w->window, w->label);
    mvwaddstr(w->window, 1, 3, str);
    wrefresh(w->window);
}

char * search_input(window_t *w) {
    create_win(w);
    char *track = calloc(100, sizeof(char));
    type_search(w, track);
    wclear(w->window);
    delwin(w->window);
    wrefresh(w->window);
    refresh();
    return track;
}

void type_search(window_t *win, char * str) {
    int i = 0;
    int c;
    while ((c = getch()) != '\n') {
        if (c == KEY_BACKSPACE) {
            if (i > 0) {
                i--;
                str[i] = '\0';
                clear_and_write(win, str);
            }
            continue;
        }
        str[i++] = c;
        clear_and_write(win, str);
    }
    str[i] = '\0';
}

void create_win(window_t *w) {
    w->window = newwin(w->y, w->x, w->starty, w->startx);
    CHECK_WINDOW(w->window);
    box(w->window, 0, 0);
    addLabel(w->window, w->label);
    wrefresh(w->window);
    refresh();
}

void addLabel(WINDOW *win, char *str) {
    size_t len = strlen(str);
    char *nstr = malloc(len + 3);
    sprintf(nstr, "| %.*s |", (int)len, str);
    int x = getmaxx(win) * 0.10;
    mvwaddstr(win, 0, x, nstr);
    wrefresh(win);
    free(nstr);
}

void destroy_menu(window_t *w, ITEM **items) {
    if(w->menu != NULL) {
        unpost_menu(w->menu);
        free_menu(w->menu);
        for(int i = 0; items[i] != NULL; i++) {
            free_item(items[i]);
        }
    }
}

void create_menu(window_t *w, ITEM **items, Menu_Options options) {
    w->menu = new_menu((ITEM **)items);
    set_menu_win(w->menu, w->window);
    set_menu_sub(w->menu, derwin(w->window, getmaxy(w->window) - 1, 10, 1, 1));
    set_menu_format(w->menu, getmaxy(w->window) - 3, 0);
    set_menu_mark(w->menu, " * ");

    menu_opts_off(w->menu, options);
    menu_opts_off(w->menu, O_SHOWDESC);
    post_menu(w->menu);
    wrefresh(w->window);
    refresh();
}

void drive_menu(window_t *w, int key) {
    menu_driver(w->menu, key);
    wrefresh(w->window);
    refresh();
}

void logging(char t, char *str, ...) {
   
    static window_t *logging;

    static window_t *command;

    if(logging == NULL) {
        logging = calloc(1, sizeof(window_t));
        logging->label = "Logging";
        logging->y = layout->yMax - layout->yDiv;
        logging->x = layout->xMax - layout->xDiv;
        logging->starty = layout->yDiv;
        logging->startx = layout->xDiv;
    }

    if(command == NULL) {
        command = calloc(1, sizeof(window_t));
        command->label = "Command";
        command->y = layout->yMax - layout->yDiv;
        command->x = layout->xDiv;
        command->starty = layout->yDiv;
        command->startx = 0;
    }
    if (str == NULL) {
        if(logging->window != NULL) {
            free_window(logging);
            LOG("Destroyed logging window");
        }
        if(command->window != NULL) {
            free_window(command);
            LOG("Destroyed command window");
        }
        return;
    } else {
        if(logging->window == NULL) {
            create_win(logging);
            LOG("Created logging window");
        }
        if(command->window == NULL) {
            create_win(command);
            LOG("Created command window");
        }

    }
    va_list args;
    va_start(args, str);

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), str, args);
    va_end(args);

    if(t == 'C') {
        clear_and_write(command, buffer);
    }
    if(t == 'L') {
        clear_and_write(logging, buffer);
    }
    
    refresh();
    LOG("%s", buffer);
}

void free_window(window_t *w) {
    if (w->window != NULL) {
        delwin(w->window);
        free(w->label);
        free(w);
    }
}

#endif // DZR_H
