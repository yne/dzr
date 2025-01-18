#ifndef DZR_H
#define DZR_H

#include "dzr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct window_t {
    WINDOW *window;
    window_t *subwindow;
    int y;
    int x;
    int starty;
    int startx;
    char *label;
};

void create_win(window_t *w);

void addLabel(WINDOW *win, char *str);
void free_window(window_t *w);
void logging(char *str, ...);

int main(void) { // int argc, char **argv

    // init screen and sets up screen
    INIT_CURSES();

    window_t *playlist_w = calloc(1, sizeof(window_t));
    playlist_w->label = "Playlist";
    playlist_w->y = layout->yMax;
    playlist_w->x = layout->xDiv;
    playlist_w->starty = 0;
    playlist_w->startx = 0;
    playlist_w->subwindow = NULL;

    window_t *painel_w = calloc(1, sizeof(window_t));
    painel_w->label = "Painel";
    painel_w->y = layout->yDiv;
    painel_w->x = layout->xMax - layout->xDiv;
    painel_w->starty = 0;
    painel_w->startx = layout->xDiv;

    painel_w->subwindow = calloc(1, sizeof(window_t));
    painel_w->subwindow->label = "Search (track[t]/artist[a]/album[b]/playlist[p]/user[u]/radio[r])";
    painel_w->subwindow->y = 3;
    painel_w->subwindow->x = getmaxx(painel_w->window) - 2;
    painel_w->subwindow->starty = 1;
    painel_w->subwindow->startx = layout->xDiv + 1;

    create_win(playlist_w);
    
    create_win(painel_w);


    // create menus
    int nchoices = getmaxy(playlist_w->window) - 5;
    ITEM **items = NULL;

    for (int i = 0; i < nchoices; i++) {
        items = (ITEM **)realloc(items, sizeof(ITEM *) * (i + 1));
        char *str = malloc(sizeof(char) * 20);
        sprintf(str, "%d item", i);
        items[i] = new_item(str, "abcd");
    }
    items = (ITEM **)realloc(items, sizeof(ITEM *) * (nchoices + 1));
    items[nchoices] = NULL;

    MENU *menu = new_menu((ITEM **)items);

    set_menu_win(menu, playlist_w->window);
    set_menu_sub(menu, subwin(playlist_w->window, getmaxy(playlist_w->window) - 1,
                              getmaxx(playlist_w->window) - 1, 1, 1));
    set_menu_format(menu, nchoices, 0);
    set_menu_mark(menu, " * ");
    menu_opts_off(menu, O_ONEVALUE);

    post_menu(menu);
    // refresh();
    wrefresh(playlist_w->window);

    int ch;
    while ((ch = getch()) != CTRL_D) {
        logging("ch: %d %c", ch, ch);
        switch (ch) {
        case KEY_DOWN: {
            logging("down");
            menu_driver(menu, REQ_DOWN_ITEM);
            refresh();
            wrefresh(playlist_w->window);
            break;
        }
        case KEY_UP: {
            logging("up");
            menu_driver(menu, REQ_UP_ITEM);
            refresh();
            wrefresh(playlist_w->window);
            break;
        }
        case KEY_LEFT: {
            logging("left");
            break;
        }
        case KEY_RIGHT: {
            logging("right");
            break;
        }
        case ' ': {
            logging("backspace");
            ITEM *it = current_item(menu);
            int index = item_index(it);
            logging("selecting item %d", index);
            menu_driver(menu, REQ_TOGGLE_ITEM);

            break;
        }
        case KEY_F(1): {
            logging("f1");
            break;
        }
        case ':': {
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
            break;
        }
        default:
            break;
        }
    }

    logging("exiting ...");
    unpost_menu(menu);
    free_menu(menu);

    FREE_ITEMS(items, nchoices + 1);

    free_window(playlist_w);
    free_window(painel_w);


    logging(NULL);
    endwin();
    exit(0);
    return 0;
}


void create_win(window_t *w) {
    w->window = newwin(w->y, w->x, w->starty, w->startx);
    CHECK_WINDOW(w->window);
    box(w->window, 0, 0);
    addLabel(w->window, w->label);
    refresh();
    wrefresh(w->window);
    if (w->subwindow != NULL) {
        window_t *subw = &w->subwindow;
        subw->window = derwin(w->window, subw->y, subw->x, subw->starty, subw->startx);
        printf("subw: %p\n", (void *) subw->window);
        CHECK_WINDOW(subw->window);
        box(subw->window, 0, 0);
        addLabel(subw->window, subw->label);
        refresh();
        wrefresh(subw->window);
    }
}

void addLabel(WINDOW *win, char *str) {
    char *nstr = malloc(sizeof(char) * (strlen(str) + 2));
    sprintf(nstr, "| %s |", str);
    mvwaddstr(win, 0, 1, nstr);
    wrefresh(win);
    free(nstr);
}

void logging(char *str, ...) {
   
    static window_t *logging;

    if(logging == NULL) {
        logging = calloc(1, sizeof(window_t));
        logging->label = "Logging";
        logging->y = layout->yMax - layout->yDiv;
        logging->x = layout->xMax - layout->xDiv;
        logging->starty = layout->yDiv;
        logging->startx = layout->xDiv;
    }

    if (str == NULL) {
        if (logging->window != NULL) {
            delwin(logging->window);
            LOG("Destroyed logging window");
        }
        return;
    } else {
        if (logging->window == NULL) {
            create_win(logging);
            LOG("Created logging window");
        }
    }
    va_list args;
    va_start(args, str);

    wclear(logging->window);
    box(logging->window, 0, 0);
    addLabel(logging->window, "Logging");

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), str, args);
    mvwaddstr(logging->window, 1, 3, buffer);
    wrefresh(logging->window);

    LOG("%s", buffer);

    va_end(args);
}

void free_window(window_t *w) {
    if (w->window != NULL) {
        delwin(w->window);
        free(w->label);
        free(w);
    }
    if(w->subwindow != NULL) {
        free_window(w->subwindow);
    }
}

#endif // DZR_H
