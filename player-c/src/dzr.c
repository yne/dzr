#include "dzr.h"
#include "requests.h"

#include <string.h>
#include <wchar.h>

typedef enum {
    CTRL_D = 4,
    COMMAND = ':',
    UP = KEY_UP,
    DOWN = KEY_DOWN,
    LEFT = KEY_LEFT,
    RIGHT = KEY_RIGHT,
    ENTER = 10,
    PAGE_UP = KEY_NPAGE,
    PAGE_DOWN = KEY_PPAGE,
    HOME = KEY_HOME,
    END = KEY_END,
    SELECT = ' ',

    TRACK = 't',
    ALBUM = 'b',
    ARTIST = 'a',
    PLAYLIST = 'p',
    USER = 'u',
    GENRE = 'g',
    RADIO = 'r'
} Commands;

typedef struct {
    Menu_Options on;
    Menu_Options off;
} Menu_Options_Seeting;

int create_menu(window_t *w, Menu_Options_Seeting options);

int destroy_menu(window_t *w);

int destroy_itens(window_t *w);

int drive_menu(window_t *w, int key);

int addLabel(WINDOW *win, char *str);

char *search_input(const char* label);

char *int_to_string(char* format, int i);

char *filter_chars(char *str);

int search_api(char *path, window_t *w);

int create_win(window_t *w);
int free_window(window_t *w);
int clear_and_write(window_t *w, char *str);

// https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/windows.html
// https://pubs.opengroup.org/onlinepubs/7908799/xcurses/intovix.html
// http://graysoftinc.com/terminal-tricks/curses-windows-pads-and-panels
// https://sploitfun.wordpress.com/2015/02/10/understanding-glibc-malloc


int main(void) { // int argc, char **argv

    TRACE("main: entering");
    init_curses();
    
    int yMax = getmaxy(stdscr);
    int xMax = getmaxx(stdscr);

    // Menu_Options_Seeting playlist_options = {
    //     .on = O_ONEVALUE,
    //     .off = O_SHOWDESC
    // };

    window_t *playlist_w = calloc(1, sizeof(window_t));
    if(!playlist_w){
        TRACE("Error allocating memory for playlist window");
        goto exit;
    }
    if(!strcpy(playlist_w->label, "Playlist")){
        TRACE("Error copying label to playlist window");
        goto exit;
    }

    playlist_w->y = yMax;
    playlist_w->x = xMax * 0.20;
    playlist_w->starty = 0;
    playlist_w->startx = 0;
    
    window_t *painel_w = calloc(1, sizeof(window_t));
    if(!painel_w){
        TRACE("Error allocating memory for painel window");
        goto exit;
    }
    if(!strcpy(painel_w->label, "Painel")){
        TRACE("Error copying label to painel window");
        goto exit;
    }
    painel_w->y = yMax;
    painel_w->x = xMax * 0.80;
    painel_w->starty = 0;
    painel_w->startx = playlist_w->x;

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
        switch (ch) {
            case UP: {
                TRACE("main: up");
                drive_menu(painel_w, REQ_UP_ITEM);
                break;
            }
            case DOWN: {
                TRACE("main: down");
                drive_menu(painel_w, REQ_DOWN_ITEM);
                break;
            }
            case PAGE_UP: {
                TRACE("main: previous page");
                drive_menu(painel_w, REQ_SCR_DPAGE);
                break;
            }
            case PAGE_DOWN: {
                TRACE("main: next page");
                drive_menu(painel_w, REQ_SCR_UPAGE);
                break;
            }
            case HOME: {
                TRACE("main: home");
                drive_menu(painel_w, REQ_FIRST_ITEM);
                break;
            }
            case END: {
                TRACE("main: end");
                drive_menu(painel_w, REQ_LAST_ITEM);
                break;
            }
            case LEFT: {
                TRACE("main: left");
                break;
            }
            case RIGHT: {
                TRACE("main: right");
                break;
            }
            case SELECT: {
                TRACE("main: select");
                ITEM *it = current_item(painel_w->menu);
                int index = item_index(it);
                TRACE("main: selecting item %d", index);
                menu_driver(painel_w->menu, REQ_TOGGLE_ITEM);
                break;
            }
            case COMMAND: {
                TRACE("main: command");
                ch = getch();
                switch (ch) {
                    case TRACK: {
                        TRACE("main: track");
                        if (!search_api("track", painel_w)) {
                            TRACE("main: Error searching track");
                        }
                        break;
                    }
                    case ARTIST: {
                        TRACE("main: artist");
                        if (!search_api("artist", painel_w)) {
                            TRACE("main: Error searching track");
                        }
                        break;
                    }
                    case ALBUM: {
                        TRACE("main: album");
                        if (!search_api("album", painel_w)) {
                            TRACE("main: Error searching track");
                        }
                        break;
                    }
                    case PLAYLIST: {
                        TRACE("main: playlist");
                        if (!search_api("playlist", painel_w)) {
                            TRACE("main: Error searching track");
                        }
                        break;
                    }
                    case USER: {
                        TRACE("main: user");
                        TRACE("main: Not implemented");
                        break;
                    }
                    case RADIO: {
                        TRACE("main: radio");
                        TRACE("main: Not implemented");
                        break;
                    }
                    default: {
                        TRACE("main: invalid");
                        break;
                    }
                }
                break;
            }
            default:
                break;
        }
        TRACE("main: updating panels");
        update_panels();

        TRACE("main: updating display");
        doupdate();
    }

    exit:

    TRACE("main: exiting ...");

    search_api("KILL", NULL);
    search_input("KILL");
    free_window(playlist_w);
    free_window(painel_w);
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
        char * previous;
        int cur_size;
        int total;
        response_items ** items;
    } response_internals;

    static response_internals* response_internal;

    if(!response_internal){
        response_internal = calloc(1, sizeof(response_internals));
        if(!response_internal){
            TRACE("search_api: Error allocating memory for response_internal");
            return ERR;
        }
        response_internal->path = path;
        response_internal->next = NULL;
        response_internal->previous = NULL;
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
            if(response_internal->previous)
                free(response_internal->previous);
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
        response_internal->cur_size = 0;
        response_internal->path = path;
        response_internal->next = NULL;
        response_internal->previous = NULL;
        if (w->items != NULL || w->menu != NULL) {
            TRACE("search_api: Resetting menu");
            destroy_menu(w);
        }
    }
    
    if (response_internal->cur_size > 0 && response_internal->total > 0 && response_internal->cur_size > response_internal->total - 1) {
        TRACE("search_api: Search reach to total");
        return 1;    
    }

    if(response_internal && response_internal->next){
        response_data = http_get(response_internal->next);
    }else{

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
                response_internal->next = calloc(strlen(next_url) + 1, sizeof(char));
                strcpy(response_internal->next, next_url);
            }
        }
    }

    if(cJSON_HasObjectItem(json, "prev")){
        cJSON *previous = cJSON_GetObjectItem(json, "prev");
        if(previous){
            char *previous_url = cJSON_GetStringValue(previous);
            if(previous_url){
                response_internal->previous = calloc(strlen(previous_url) + 1, sizeof(char));
                strcpy(response_internal->previous, previous_url);

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
            char *nb_tracks_str = int_to_string(" (%d)", nb_tracks);
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
            name_item = calloc(strlen(names.title) + strlen(names.nb_tracks) + 1, sizeof(char));
            strcat(name_item, names.title);
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
    
    ITEM **items = malloc(sizeof(ITEM *) * (response_internal->cur_size + 1));
    for(int i = 0; i < response_internal->cur_size; i++){
        char * id_str = int_to_string(NULL, response_internal->items[i]->id);
        items[i] = new_item(response_internal->items[i]->name, id_str);
    }

    if(w->menu && w->items){
        destroy_itens(w);
    }
    w->items = items;
    
    if(!w->menu){
        Menu_Options_Seeting setting = {
            .on = O_ONEVALUE,
            .off = O_SHOWDESC
        };
        create_menu(w, setting);
    }else{
        int i = 0;
        i = unpost_menu(w->menu);
        i = set_menu_items(w->menu, w->items);
        i = post_menu(w->menu);
        i++;
    }

    

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
    if (w == NULL || w->window == NULL) {
        TRACE("Invalid parameters in clear_and_write");
        return ERR;
    }

    TRACE("Clearing window in clear_and_write");
    if (wclear(w->window) != OK) {
        TRACE("Error clearing window in clear_and_write");
        return ERR;
    }

    TRACE("Drawing box in clear_and_write");
    if (box(w->window, 0, 0) != OK) {
        TRACE("Error drawing box in clear_and_write");
        return ERR;
    }

    TRACE("Adding label in clear_and_write");
    if(addLabel(w->window, w->label) != OK){
        TRACE("Error adding label in clear_and_write");
        return ERR;
    }

    if(str && strlen(str) > 0){
        TRACE("Adding string in clear_and_write");
        if (mvwaddstr(w->window, 1, 3, str) != OK) {
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

int create_win(window_t *w) {
    TRACE("create_win: Creating window");
    w->window = newwin(w->y, w->x, w->starty, w->startx);
    if (!w->window) {
        TRACE("create_win: Unable to create window");
        return ERR;
    }

    TRACE("create_win: Drawing box in window");
    if (box(w->window, 0, 0) != OK) {
        TRACE("create_win: Error drawing box");
        return ERR;
    }

    TRACE("create_win: Adding label to window");
    if (addLabel(w->window, w->label) != OK) {
        TRACE("create_win: Error adding label");
        return ERR;
    }

    TRACE("create_win: Creating panel for window");
    w->panel = new_panel(w->window);
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

    TRACE("destroy_menu: Freeing menu");
    if (free_menu(w->menu) != E_OK) {
        TRACE("destroy_menu: Error freeing menu");
        return ERR;
    }
    w->menu = NULL;

    if (w->items == NULL) {
        TRACE("destroy_menu: No items to free");
        return OK;
    }

    destroy_itens(w);

    return OK;
}

int destroy_itens(window_t *w) {
    if (w == NULL || w->items == NULL) {
        TRACE("destroy_itens: Invalid parameters");
        return ERR;
    }

    TRACE("destroy_itens: Freeing menu items");
    for (int i = 0; w->items[i] != NULL; i++) {
        TRACE("destroy_itens: Freeing item %d", i);
        if (free_item(w->items[i]) != E_OK) {
            TRACE("destroy_itens: Error freeing item %d", i);
            return ERR;
        }
        w->items[i] = NULL;
    }
    free(w->items);
    w->items = NULL;
    TRACE("destroy_itens: Ended");
    return OK;
}

int create_menu(window_t *w, Menu_Options_Seeting options) {
    TRACE("create_menu: Started");
    if (w == NULL || w->items == NULL) {
        TRACE("create_menu: Invalid parameters");
        return ERR;
    }

    TRACE("create_menu: Creating new menu");
    w->menu = new_menu(w->items);
    if (w->menu == NULL) {
        TRACE("create_menu: Error creating new menu");
        return ERR;
    }

    TRACE("create_menu: Setting menu window");
    if (set_menu_win(w->menu, w->window) != E_OK) {
        TRACE("create_menu: Error setting menu window");
        free_menu(w->menu);
        w->menu = NULL;
        return ERR;
    }

    TRACE("create_menu: Setting menu subwindow");
    if (set_menu_sub(w->menu, derwin(w->window, getmaxy(w->window) - 2, getmaxx(w->window) - 2, 1, 1)) != E_OK) {
        TRACE("create_menu: Error setting menu subwindow");
        free_menu(w->menu);
        w->menu = NULL;
        return ERR;
    }

    TRACE("create_menu: Setting menu format");
    if (set_menu_format(w->menu, getmaxy(w->window) - 3, 1) != E_OK) {
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

    TRACE("free_window: Freeing window");
    if (w->window != NULL) {
        if(delwin(w->window) != OK){
            TRACE("free_window: Error deleting window");
            return ERR;
        }
    }

    TRACE("free_window: Freeing memory");
    free(w);

    TRACE("free_window: Ended");
    return OK;
}
