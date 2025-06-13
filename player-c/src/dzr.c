#include "dzr.h"
#include "logging.h"
#include "requests.h"

#include <ncursesw/curses.h>
#include <ncursesw/menu.h>
#include <ncursesw/panel.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vadefs.h>
#include <wchar.h>
#include <locale.h>

ITEM ** selected_items = NULL; 

int update_menu(window_t *w, ITEM ** items);

int create_menu(window_t *w, ITEM ** items, Menu_Options_t options);

int destroy_menu(window_t *w);

int destroy_items(ITEM ** items);

int drive_menu(window_t *w, int key);

int addLabel(WINDOW *win, char *str);

char *search_input(const char* label);

char *int_to_string(char* format, int i);

char *filter_chars(char *str);

int search_api(char *path, window_t *w);

int create_win(window_t *w);
int free_window(window_t *w);
int clear_and_write(window_t *w, char *str);

int do_command(int ch, ...);

char *type_search(char ch){
    switch (ch) {
        case 't':
            return "track";
        case 'b':
            return "album";
        case 'a':
            return "artist";
        case 'p':
            return "playlist";
        case 'u':
            return "user";
        case 'g':
            return "genre";
        case 'r':
            return "radio";
        default:
            return "none";
    }
}
// https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/windows.html
// https://pubs.opengroup.org/onlinepubs/7908799/xcurses/intovix.html
// http://graysoftinc.com/terminal-tricks/curses-windows-pads-and-panels
// https://sploitfun.wordpress.com/2015/02/10/understanding-glibc-malloc

int main(int argc, char **argv) {

    TRACE("main: entering");

    for(int i = 0; i < argc; i++){
        TRACE("argument %d: %s", i, argv[i]);
    }
    
    initscr();
    noecho();
    raw();
    keypad(stdscr, TRUE);
    setlocale(LC_ALL, "US");
    TRACE("Curses initialized ...");
    
    int yMax = getmaxy(stdscr);
    int xMax = getmaxx(stdscr);

    window_t *playlist_w = malloc(sizeof(window_t));
    strcpy(playlist_w->label, "Playlist");

    playlist_w->y = yMax;
    playlist_w->x = xMax * 0.20;
    playlist_w->starty = 0;
    playlist_w->startx = 0;
    playlist_w->menu = NULL;
    
    window_t *painel_w = malloc(sizeof(window_t));

    strcpy(painel_w->label, "Painel");
    
    painel_w->y = yMax;
    painel_w->x = xMax * 0.80;
    painel_w->starty = 0;
    painel_w->startx = playlist_w->x;
    painel_w->menu = NULL;

    TRACE("main: creating playlist window");
    if(create_win(playlist_w) != OK){
        TRACE("Error creating playlist window");
        goto exit;
    }

    TRACE("main: creating painel window");
    if(create_win(painel_w) != OK){
        TRACE("Error creating painel window");
        goto exit;
    }
    
    TRACE("main: searching for input");
    search_input("INIT");

    TRACE("main: updating panels");
    update_panels();

    TRACE("main: updating display");
    doupdate();

    int ch;
    while ((ch = getch()) != CTRL_D) {
        TRACE("main: key: %d char: %c", ch, ch);
        do_command(ch, painel_w);

        TRACE("main: updating panels");
        update_panels();

        TRACE("main: updating display");
        doupdate();
    }

    exit:

    TRACE("main: exiting ...");

    search_api("KILL", NULL);
    search_input("KILL");
    if(playlist_w != NULL){
        free_window(playlist_w);
    }
    if(painel_w != NULL){
        free_window(painel_w);
    }
    endwin();
    exit(0);
    return 0;
}

int search_api(char *path, window_t *w) {
    TRACE("search_api: Entered");
    
    typedef struct {
        char * name;
        int id;
    } response_items;

    typedef struct {
        char * path;
        char * next;
        int cur_size;
        int total;
        response_items ** items;
    } response_internals;

    static response_internals* response_internal;

    if(!response_internal){
        response_internal = malloc(sizeof(response_internals));
        if(!response_internal){
            TRACE("search_api: Error allocating memory for response_internal");
            return ERR;
        }
        response_internal->path = path;
        response_internal->next = NULL;
        response_internal->cur_size = 0;
        response_internal->total = 0;
        response_internal->items = NULL;
    }
    
    if(strcmp(path, "KILL") == 0){
        if(response_internal){
            if(response_internal->path)
                free(response_internal->path);
            if(response_internal->next)
                free(response_internal->next);
            if(response_internal->items){
                for(int i = 0;response_internal->items[i] != NULL; i++){
                    if(response_internal->items[i]->name){
                        free(response_internal->items[i]->name);
                        response_internal->items[i]->name = NULL;
                    }
                    free(response_internal->items[i]);
                    response_internal->items[i] = NULL;
                }
                free(response_internal->items);
                response_internal->items = NULL;
            }
            free(response_internal);
            response_internal = NULL;
        }
        return OK;
    }

    buffer_t *response_data = NULL;

    if(response_internal->path && strcmp(path, response_internal->path) != 0){
        if(strcmp(path, "MORE") == 0){
            if(response_internal && response_internal->next){
                response_data = http_get(response_internal->next);
            }
        }else{
            response_internal->cur_size = 0;
            response_internal->path = path;
            response_internal->next = NULL;
            if (w->menu != NULL) {
                TRACE("search_api: Resetting menu");
                destroy_menu(w);
            }
        }
    }
    
    if (response_internal->cur_size > 0 && response_internal->total > 0 && response_internal->cur_size > response_internal->total - 1) {
        TRACE("search_api: Search reach to total");
        return 1;    
    }

    if (!response_data) { 
        char *input = search_input(path);
        TRACE("search_api: Searching for %s", input);
        if (!input || strlen(input) == 0) {
            return OK;
        }
        response_data = api_url_search(path, input);
        free(input);
    }

    
    if (!response_data) {
        TRACE("search_api: Error getting response data");
        return ERR;
    }
    

    cJSON *json = cJSON_ParseWithLength(response_data->data, response_data->size);
    if (!json) {
        TRACE("search_api: Error parsing json");
        free(response_data->data);
        free(response_data);
        return ERR;
    }

    if(cJSON_HasObjectItem(json, "total")){
        cJSON *total = cJSON_GetObjectItem(json, "total");
        if(total){
            int total_int = cJSON_GetNumberValue(total);
            response_internal->total = total_int;
        }
    }

    if(cJSON_HasObjectItem(json, "next")){
        cJSON *next = cJSON_GetObjectItem(json, "next");
        if(next){
            char *next_url = cJSON_GetStringValue(next);
            if(next_url){
                if(response_internal->next){
                    free(response_internal->next);
                }
                response_internal->next = malloc(strlen(next_url) + 1);
                strcpy(response_internal->next, next_url);
            }
        }
    }

#define CHECK(json_field)                                                                                                                            \
    if (!json_field) {                                                                                                                               \
        cJSON_Delete(json);                                                                                                                          \
        free(response_data->data);                                                                                                                   \
        free(response_data);                                                                                                                         \
        TRACE("search_api: Error in %s", #json_field);                                                                                               \
        return ERR;                                                                                                                                  \
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    CHECK(data);

   
    int size = cJSON_GetArraySize(data);

    if(!response_internal->items){
        response_internal->items = malloc(sizeof(response_items *) * (size + 1));
        CHECK(response_internal->items);
    }else{
        void *tmp = realloc(response_internal->items, sizeof(response_items *) * (response_internal->cur_size + size + 1));
        CHECK(tmp);
        response_internal->items = tmp;

    }
    
    typedef struct {
        char *artist;
        char *title;
        char *nb_tracks;
    } names_t;
    

    for (int i = 0; i < size; i++) {

        names_t names;
        names.artist = NULL;
        names.title = NULL;
        names.nb_tracks = NULL;

        cJSON *c_item = cJSON_GetArrayItem(data, i);
        CHECK(c_item);
        
        if (cJSON_HasObjectItem(c_item, "artist")) {
            cJSON *c_artist = cJSON_GetObjectItem(c_item, "artist");
            CHECK(c_artist);
            if (cJSON_HasObjectItem(c_artist, "name")) {
                cJSON *c_name = cJSON_GetObjectItem(c_artist, "name");
                CHECK(c_name);
                names.artist = filter_chars(cJSON_GetStringValue(c_name));
            }
        }
        if (cJSON_HasObjectItem(c_item, "title")) {
            cJSON *c_title = cJSON_GetObjectItem(c_item, "title");
            CHECK(c_title);
            names.title = filter_chars(cJSON_GetStringValue(c_title));
        }
        if (cJSON_HasObjectItem(c_item, "nb_tracks")) {
            cJSON *c_nb_tracks = cJSON_GetObjectItem(c_item, "nb_tracks");
            CHECK(c_nb_tracks);
            int nb_tracks = (int)cJSON_GetNumberValue(c_nb_tracks);
            char *nb_tracks_str = int_to_string("(%d)", nb_tracks);
            CHECK(nb_tracks_str);
            names.nb_tracks = malloc(strlen(nb_tracks_str) + 1);
            strcpy(names.nb_tracks, nb_tracks_str);
        }

        char * name_item = NULL;

        if(names.artist && names.title){ // Artist - Title
            name_item = calloc(strlen(names.artist) + strlen(names.title) + strlen(" - ") + 1, sizeof(char));
            strcat(name_item, names.artist);
            strcat(name_item, " - ");
            strcat(name_item, names.title);
        }
        if(names.title && names.nb_tracks){ // Title (nb_tracks)
            name_item = calloc(strlen(names.title) + strlen(names.nb_tracks) + strlen(" ") + 1, sizeof(char));
            strcat(name_item, names.title);
            strcat(name_item, " ");
            strcat(name_item, names.nb_tracks);
        }
        
        
        cJSON *c_id = cJSON_GetObjectItem(c_item, "id");
        CHECK(c_id);

        int id_item = (int) cJSON_GetNumberValue(c_id);

        response_internal->items[response_internal->cur_size] = malloc(sizeof(response_items));
        response_internal->items[response_internal->cur_size]->name = filter_chars(name_item);
        response_internal->items[response_internal->cur_size]->id = id_item;
    
        response_internal->cur_size++;
    }
    
    response_internal->items[response_internal->cur_size] = NULL;
    
    ITEM **items = malloc(sizeof(ITEM *) * (response_internal->cur_size + 2));
    
    int i = 0;
    for(; response_internal->items[i] != NULL; i++){
        char * id_str = int_to_string(NULL, response_internal->items[i]->id);
        items[i] = new_item(response_internal->items[i]->name, id_str);
    }
    if (!(response_internal->cur_size > 0 && response_internal->total > 0 && response_internal->cur_size > response_internal->total - 1)) {
        if(response_internal->next){

            items[i] = new_item("-- MORE --", "MORE");
            i++;
        }
    }
    items[i] = NULL;

    update_menu(w, items);

    cJSON_Delete(json);
    free(response_data->data);
    free(response_data);
    return 1;
}

char *int_to_string(char *format, int i) {
    int len = snprintf(NULL, 0, format ? format : "%d", i);
    char *str = calloc(len + 1, sizeof(char));
    snprintf(str, len + 1, format ? format : "%d", i);
    return (char *)str;
}

int clear_and_write(window_t *w, char *str) {
    if (w == NULL) {
        TRACE("Invalid parameters in clear_and_write");
        return ERR;
    }

    WINDOW *window = panel_window(w->panel);
    if( window == NULL){
        TRACE("window is null");
        return ERR;
    }

    TRACE("Clearing window in clear_and_write");
    if (wclear(window) != OK) {
        TRACE("Error clearing window in clear_and_write");
        return ERR;
    }

    TRACE("Drawing box in clear_and_write");
    if (box(window, 0, 0) != OK) {
        TRACE("Error drawing box in clear_and_write");
        return ERR;
    }

    TRACE("Adding label in clear_and_write");
    if(addLabel(window, w->label) != OK){
        TRACE("Error adding label in clear_and_write");
        return ERR;
    }

    if(str && strlen(str) > 0){
        TRACE("Adding string in clear_and_write");
        if (mvwaddstr(window, 1, 3, str) != OK) {
            TRACE("Error adding string in clear_and_write");
            return ERR;
        }
    }
    return OK;
}

int is_chars(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
        (c >= '0' && c <= '9') || c == ' ' || c == '.' || 
        c == ',' || c == '!' || c == '?' || c == '-' || 
        c == '_' || c == '[' || c == ']' || c == '{' || 
        c == '}' || c == '(' || c == ')') {
        return 1;
    }
    return 0;
}

char *search_input(const char *label) {
    static window_t *search_window;

    TRACE("search_input: called with label %s", label);

    if (strcmp(label, "KILL") == 0) {
        TRACE("search_input: KILL command received, freeing window");
        free_window(search_window);
        return NULL;
    }

    if (strcmp(label, "INIT") == 0) {
        TRACE("search_input: INIT command received, initializing window");
        search_window = calloc(1, sizeof(window_t));
        if (!search_window) {
            TRACE("search_input: Error allocating memory for search_window");
            return NULL;
        }

        search_window->y = 3;
        search_window->x = getmaxx(stdscr) * 0.80;
        search_window->starty = (getmaxy(stdscr) / 2) - (search_window->y / 2);
        search_window->startx = (getmaxx(stdscr) / 2) - (search_window->x / 2);

        if (create_win(search_window) != OK) {
            TRACE("search_input: Error creating window");
            free(search_window);
            return NULL;
        }

        hide_panel(search_window->panel);
        update_panels();
        doupdate();
        return NULL;
    }

    TRACE("search_input: Setting window label");
    sprintf(search_window->label, "Search %s", label);
    clear_and_write(search_window, NULL);

    TRACE("search_input: Showing window panel");
    top_panel(search_window->panel);
    show_panel(search_window->panel);
    update_panels();
    doupdate();

    char *input = malloc(1);
    if (!input) {
        TRACE("search_input: Error allocating memory for input");
        return NULL;
    }
    input[0] = '\0';

    int i = 0;
    int c;
    TRACE("search_input: Waiting for user input");
    while ((c = getch()) != '\n') {
        if (c == KEY_BACKSPACE) {
            if (i > 0) {
                i--;
                input[i] = '\0';
                char *tmp = realloc(input, i + 1);
                if (tmp) {
                    input = tmp;
                }
                TRACE("search_input: Backspace detected, updating input");
                clear_and_write(search_window, input);
                update_panels();
                doupdate();
            }
            continue;
        }
        if (is_chars(c)) {
            char *tmp = realloc(input, i + 2);
            if (tmp) {
                input = tmp;
                input[i++] = c;
                input[i] = '\0';
                TRACE("search_input: Adding character '%c' to input", c);
                clear_and_write(search_window, input);
                update_panels();
                doupdate();
            } else {
                TRACE("search_input: Error reallocating memory for input");
                free(input);
                free_window(search_window);
                return NULL;
            }
        }
    }

    TRACE("search_input: Input complete, hiding window panel");
    clear_and_write(search_window, NULL);
    hide_panel(search_window->panel);
    update_panels();
    doupdate();

    TRACE("search_input: Returning user input");
    return input;
}

char *filter_chars(char *input_string) {
    if (!input_string) {
        return NULL;
    }

    char *output_string = input_string;
    while (*output_string) {
        if (!is_chars(*output_string)) {
            *output_string = '_';
        }
        output_string++;
    }

    return input_string;
}

int update_menu(window_t *w, ITEM ** items){
    if(w == NULL){
        TRACE("update_menu: unable update menu");
        return ERR;
    }
    if(w->menu){
        TRACE("update_menu: destroying menu");
        if(destroy_menu(w) != OK){
            TRACE("update_menu: error create menu");
            return ERR;
        };
    }

    TRACE("update_menu: creating menu");
    if(create_menu(w, items, GLOBAL_MENU_OPTIONS) != OK){
        TRACE("update_menu: error create menu");
        return ERR;
    };
    if(w->menu == NULL){
        TRACE("update_menu: menu doesn't exist");
        return ERR;
    }
    return OK;
}

int create_win(window_t *w) {
    TRACE("create_win: Creating window");
    WINDOW *window = newwin(w->y, w->x, w->starty, w->startx);
    if (!window) {
        TRACE("create_win: Unable to create window");
        return ERR;
    }

    TRACE("create_win: Drawing box in window");
    if (box(window, 0, 0) != OK) {
        TRACE("create_win: Error drawing box");
        return ERR;
    }

    TRACE("create_win: Adding label to window");
    if (addLabel(window, w->label) != OK) {
        TRACE("create_win: Error adding label");
        return ERR;
    }

    TRACE("create_win: Creating panel for window");
    w->panel = new_panel(window);
    if (!w->panel) {
        TRACE("create_win: Error creating panel");
        return ERR;
    }

    TRACE("create_win: Window created successfully");
    return OK;
}

int addLabel(WINDOW *win, char *str) {
    TRACE("addLabel: Started");
    if (win == NULL || str == NULL) {
        TRACE("addLabel: Invalid parameters");
        return ERR;
    }

    TRACE("addLabel: Allocating memory for nstr");
    size_t len = strlen(str);
    char *nstr = malloc(len + 3);
    if (nstr == NULL) {
        TRACE("addLabel: Error allocating memory for nstr");
        return ERR;
    }

    TRACE("addLabel: Formatting string");
    sprintf(nstr, " %.*s ", (int)len, str);
    int x = getmaxx(win) * 0.10;
    if (mvwaddstr(win, 0, x, nstr) != OK) {
        TRACE("addLabel: Error adding string");
        free(nstr);
        return ERR;
    }

    TRACE("addLabel: Freeing memory for nstr");
    free(nstr);
    TRACE("addLabel: Ended");
    return OK;
}

int destroy_menu(window_t *w) {
    if (w == NULL || w->menu == NULL) {
        TRACE("destroy_menu: Invalid parameters");
        return ERR;
    }

    TRACE("destroy_menu: Unposting menu");
    if (unpost_menu(w->menu) != E_OK) {
        TRACE("destroy_menu: Error unposting menu");
        return ERR;
    }
    
    ITEM ** items = menu_items(w->menu);

    TRACE("destroy_menu: Freeing menu");
    if (free_menu(w->menu) != E_OK) {
        TRACE("destroy_menu: Error freeing menu");
        return ERR;
    }
    w->menu = NULL;

    if (items == NULL) {
        TRACE("destroy_menu: No items to free");
        return OK;
    }

    destroy_items(items);

    return OK;
}

int destroy_items(ITEM ** items) {
    if (items == NULL) {
        TRACE("destroy_items: Invalid parameters");
        return ERR;
    }

    TRACE("destroy_items: Freeing menu items");
    for (int i = 0; items[i] != NULL; i++) {
        TRACE("destroy_items: Freeing item %d", i);
        if (free_item(items[i]) != E_OK) {
            TRACE("destroy_items: Error freeing item %d", i);
            return ERR;
        }
        items[i] = NULL;
    }
    free(items);
    items = NULL;
    TRACE("destroy_items: Ended");
    return OK;
}

int create_menu(window_t *w, ITEM ** items, Menu_Options_t options) {
    TRACE("create_menu: Started");
    if (w == NULL || items == NULL) {
        TRACE("create_menu: Invalid parameters");
        return ERR;
    }

    TRACE("create_menu: Creating new menu");
    w->menu = new_menu(items);
    if (w->menu == NULL) {
        TRACE("create_menu: Error creating new menu");
        return ERR;
    }
    WINDOW *window = panel_window(w->panel);

    if(window == NULL){
        TRACE("create_menu: Window is null");
        return ERR;
    }

    TRACE("create_menu: Setting menu window");
    if (set_menu_win(w->menu, window) != E_OK) {
        TRACE("create_menu: Error setting menu window");
        free_menu(w->menu);
        w->menu = NULL;
        return ERR;
    }

    TRACE("create_menu: Setting menu subwindow");
    if (set_menu_sub(w->menu, derwin(window, getmaxy(window) - 2, getmaxx(window) - 2, 1, 1)) != E_OK) {
        TRACE("create_menu: Error setting menu subwindow");
        free_menu(w->menu);
        w->menu = NULL;
        return ERR;
    }

    TRACE("create_menu: Setting menu format");
    if (set_menu_format(w->menu, getmaxy(window) - 3, 1) != E_OK) {
        TRACE("create_menu: Error setting menu format");
        free_menu(w->menu);
        w->menu = NULL;
        return ERR;
    }

    TRACE("create_menu: Setting menu mark");
    if (set_menu_mark(w->menu, " * ") != E_OK) {
        TRACE("create_menu: Error setting menu mark");
        free_menu(w->menu);
        w->menu = NULL;
        return ERR;
    }

    TRACE("create_menu: Setting menu options");
    menu_opts_off(w->menu, options.off);
    menu_opts_on(w->menu, options.on);

    TRACE("create_menu: Posting menu");
    if (post_menu(w->menu) != E_OK) {
        TRACE("create_menu: Error posting menu");
        free_menu(w->menu);
        w->menu = NULL;
        return ERR;
    }
    TRACE("create_menu: Ended");
    return OK;
}

int drive_menu(window_t *w, int key) {
    TRACE("drive_menu: Starting");
    if (w == NULL || w->menu == NULL) {
        TRACE("drive_menu: Menu does not exist on window %s", (w ? w->label : ""));
        return ERR;
    }
    int i = 0;
    TRACE("drive_menu: Driving menu");
    if ((i = menu_driver(w->menu, key)) != E_OK) {
        TRACE("drive_menu: Error driving menu");
        if(i == E_REQUEST_DENIED){
            return E_REQUEST_DENIED;
        }
        return ERR;
    }
    TRACE("drive_menu: Ended");
    return OK;
}

int free_window(window_t *w) {
    if (w == NULL) {
        TRACE("free_window: Window is NULL");
        return OK;
    }

    TRACE("free_window: Freeing menu");
    if (w->menu != NULL) {
        if(destroy_menu(w) != OK){
            TRACE("free_window: Error destroying menu");
            return ERR;
        }
    }

    WINDOW *window = panel_window(w->panel);
    if(window == NULL){
        TRACE("free_window: window is null");
        return ERR;
    }

    TRACE("free_window: Freeing window");
    if (window != NULL) {
        if(delwin(window) != OK){
            TRACE("free_window: Error deleting window");
            return ERR;
        }
    }
    if(w->panel != NULL){
        if(del_panel(w->panel) != OK){
            TRACE("free_window: Error deleting panel");
            return ERR;
        }
    }
    TRACE("free_window: Freeing memory");
    free(w);

    TRACE("free_window: Ended");
    return OK;
}

// commands

void up_command(va_list args){
    TRACE("main: up");
    window_t *painel_w  = va_arg(args, window_t *);
    drive_menu(painel_w, REQ_UP_ITEM);
}

void down_command(va_list args){
    TRACE("main: down");
    window_t *painel_w  = va_arg(args, window_t *);
    drive_menu(painel_w, REQ_DOWN_ITEM);
}

void page_up_command(va_list args){
    TRACE("main: previous page");
    window_t *painel_w  = va_arg(args, window_t *);
    drive_menu(painel_w, REQ_SCR_DPAGE);
}

void page_down_command(va_list args){
    TRACE("main: next page");
    window_t *painel_w  = va_arg(args, window_t *);
    drive_menu(painel_w, REQ_SCR_UPAGE);
}

void home_command(va_list args){
    TRACE("main: home");
    window_t *painel_w  = va_arg(args, window_t *);
    drive_menu(painel_w, REQ_FIRST_ITEM);
}

void end_command(va_list args){
    TRACE("main: end");
    window_t *painel_w  = va_arg(args, window_t *);
    drive_menu(painel_w, REQ_LAST_ITEM);
}

void left_command(va_list args){
    TRACE("main: left %p", args);
}

void right_command(va_list args){
    TRACE("main: right %p", args);
}

void select_command(va_list args){
    static int index_item = 0;
    TRACE("main: select");
    window_t *painel_w  = va_arg(args, window_t *);
    ITEM *it = current_item(painel_w->menu);
    // const char *name = item_name(it);
    const char *sel =  item_description(it);
    if(sel && strcmp(sel, "MORE") == 0){
        TRACE("main: more");
        if (!search_api("MORE", painel_w)) {
            TRACE("main: Error searching more");
        }
        int item_c = item_count(painel_w->menu);

        for(int j = 0; j < item_c; j++){
            for (int i = 0; i < index_item; i++){
                
            }
            drive_menu(painel_w, REQ_DOWN_ITEM);
        }
    }else{
        index_item++;
        void *tmp = realloc(selected_items, index_item * sizeof(ITEM *));
        if(!tmp){
            TRACE("main: select: error in alocation selected_items");
            return;
        }
        selected_items = tmp;
        selected_items[index_item] = it;
        int index = item_index(it);
        TRACE("main: selecting item %d", index);
        drive_menu(painel_w, REQ_TOGGLE_ITEM);
    }

}

void search(va_list args){
     TRACE("main: search");
    window_t *painel_w  = va_arg(args, window_t *);
    int ch = getch();
    char * type = type_search(ch);
    TRACE("main: %s", type);
    if(ch == 't' || ch == 'b' || ch == 'a' || ch == 'p'){
        if (!search_api(type, painel_w)) {
            TRACE("main: Error searching track");
        }
    }
}

static command_t commands[] = {
    {
        UP, up_command
    },
    {
        DOWN, down_command
    },
    {
        PAGE_UP, page_up_command
    },
    {
        PAGE_DOWN, page_down_command
    },
    {
        HOME, home_command
    },
    {
        END, end_command
    },
    {
        LEFT, left_command
    },
    {
        RIGHT, right_command
    },
    {
        SELECT, select_command
    },
    {
        COMMAND, search
    },
    {
        0, NULL
    }
};


int do_command(int ch, ...){
    TRACE("do_command init");
    for(int i=0;; i++){
        if(commands[i].key == 0){
            TRACE("do_command end");
            break;
        }
        if(commands[i].key == ch ){
            TRACE("do_command exec");
            va_list args;
            va_start(args);
            commands[i].func(args);
            va_end(args);
        }
    }
    return OK;
}