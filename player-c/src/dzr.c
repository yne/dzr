#include "dzr.h"
#include "requests.h"

#include <string.h>
#include <wchar.h>


enum Commands {
    CTRL_D = 4,
    COMMAND = ':',
    UP = KEY_UP,
    DOWN = KEY_DOWN,
    LEFT = KEY_LEFT,
    RIGHT = KEY_RIGHT,
    ENTER = 10,
    PAGE_UP = KEY_NPAGE,
    PAGE_DOWN = KEY_PPAGE,
    SELECT = ' ',

    TRACK = 't',
    ALBUM = 'b',
    ARTIST = 'a',
    PLAYLIST = 'p',
    USER = 'u',
    GENRE = 'g',
    RADIO = 'r'
};

typedef struct {
    Menu_Options on;
    Menu_Options off;
} Menu_Options_Seeting;

int create_menu(window_t *w, Menu_Options_Seeting options);

int destroy_menu(window_t *w);

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

int main(void) { // int argc, char **argv

    init_curses();
    
    int yMax = getmaxy(stdscr);
    int xMax = getmaxx(stdscr);

    // Menu_Options_Seeting playlist_options = {
    //     .on = O_ONEVALUE,
    //     .off = O_SHOWDESC
    // };

    window_t *playlist_w = calloc(1, sizeof(window_t));
    if(!playlist_w){
        DEBUG("Error allocating memory for playlist window");
        goto exit;
    }
    if(!strcpy(playlist_w->label, "Playlist")){
        DEBUG("Error copying label to playlist window");
        goto exit;
    }

    playlist_w->y = yMax;
    playlist_w->x = xMax * 0.20;
    playlist_w->starty = 0;
    playlist_w->startx = 0;
    
    window_t *painel_w = calloc(1, sizeof(window_t));
    if(!painel_w){
        DEBUG("Error allocating memory for painel window");
        goto exit;
    }
    if(!strcpy(painel_w->label, "Painel")){
        DEBUG("Error copying label to painel window");
        goto exit;
    }
    painel_w->y = yMax;
    painel_w->x = xMax * 0.80;
    painel_w->starty = 0;
    painel_w->startx = playlist_w->x;

    if(create_win(playlist_w) != OK){
        DEBUG("Error creating playlist window");
        goto exit;
    }

    if(create_win(painel_w) != OK){
        DEBUG("Error creating painel window");
        goto exit;
    }
    
    search_input("INIT");

    update_panels();
    doupdate();

    int ch;
    while ((ch = getch()) != CTRL_D) {
        
        DEBUG("key: %d char: %c", ch, ch);
        switch (ch) {
            case UP: {
                DEBUG("up");
                drive_menu(painel_w, REQ_UP_ITEM);
                break;
            }
            case DOWN: {
                DEBUG("down");
                drive_menu(painel_w, REQ_DOWN_ITEM);
                break;
            }
            case PAGE_UP: {
                DEBUG("previous page");
                drive_menu(painel_w, REQ_SCR_DPAGE);
                break;
            }
            case PAGE_DOWN: {
                DEBUG("next page");
                drive_menu(painel_w, REQ_SCR_UPAGE);
                break;
            }
            case LEFT: {
                DEBUG("left");
                break;
            }
            case RIGHT: {
                DEBUG("right");
                break;
            }
            case SELECT: {
                DEBUG("backspace");
                ITEM *it = current_item(painel_w->menu);
                int index = item_index(it);
                DEBUG("selecting item %d", index);
                menu_driver(painel_w->menu, REQ_TOGGLE_ITEM);
                break;
            }
            case KEY_F(1): {
                DEBUG("f1");

                break;
            }
            case COMMAND: {
                DEBUG("command");
                ch = getch();
                switch (ch) {
                    case TRACK: {
                        DEBUG("track");
                        if (!search_api("track", painel_w)) {
                            DEBUG("Error searching track");
                        }
                        break;
                    }
                    case ARTIST: {
                        DEBUG("artist");
                        if (!search_api("artist", painel_w)) {
                            DEBUG("Error searching track");
                        }
                        break;
                    }
                    case ALBUM: {
                        DEBUG("album");
                        if (!search_api("album", painel_w)) {
                            DEBUG("Error searching track");
                        }
                        break;
                    }
                    case PLAYLIST: {
                        DEBUG("playlist");
                        if (!search_api("playlist", painel_w)) {
                            DEBUG("Error searching track");
                        }
                        break;
                    }
                    case USER: {
                        DEBUG("user");
                        DEBUG("Not implemented");
                        break;
                    }
                    case RADIO: {
                        DEBUG("radio");
                        DEBUG("Not implemented");
                        break;
                    }
                    default: {
                        DEBUG("invalid");
                        break;
                    }
                }
                break;
            }
            default:
                break;
        }
        update_panels();
        doupdate();
    }

    exit:

    DEBUG("exiting ...");

    search_api("KILL", NULL);
    search_input("KILL");
    free_window(playlist_w);
    free_window(painel_w);
    endwin();
    exit(0);
    return 0;
}

int search_api(char *path, window_t *w) {
    
    typedef struct  {
        char * path;
        char * next;
        char * previous;
        int cur_size;
        int total;
    } response_internals;

    static response_internals * response_internal;

    if(!response_internal){
        response_internal = malloc(sizeof(response_internals));
        if(!response_internal){
            DEBUG("Error allocating memory for response_internal");
            return ERR;
        }
        response_internal->path = path;
        response_internal->next = NULL;
        response_internal->previous = NULL;
        response_internal->cur_size = 0;
        response_internal->total = 0;
    }
    if(strcmp(path, "KILL") == 0){
        free(response_internal);
        response_internal = NULL;
        return OK;
    }

    buffer_t *response_data = NULL;

    if(response_internal->path && strcmp(path, response_internal->path) != 0){
        response_internal->cur_size = 0;
        response_internal->path = path;
        response_internal->next = NULL;
        response_internal->previous = NULL;
    }

    if(response_internal && response_internal->next){
        response_data = http_get(response_internal->next);
    }else{

        char *input = search_input(path);
    
        if (!input || strlen(input) == 0) {
            return OK;
        }
        
        
        if (w->items != NULL || w->menu != NULL) {
            destroy_menu(w);
        }
        
        response_data = api_url_search(path, input);
        free(input);
    }
    
    if (!response_data) {
        return ERR;
    }

    cJSON *json = cJSON_ParseWithLength(response_data->data, response_data->size);
    if (!json) {
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
                response_internal->next = next_url;
            }
        }
    }

    if(cJSON_HasObjectItem(json, "prev")){
        cJSON *previous = cJSON_GetObjectItem(json, "prev");
        if(previous){
            char *previous_url = cJSON_GetStringValue(previous);
            if(previous_url){
                response_internal->previous = previous_url;
            }
        }
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (!data) {
        cJSON_Delete(json);
        free(response_data->data);
        free(response_data);
        return ERR;
    }

    int size = cJSON_GetArraySize(data);
    if(!w->items){
        w->items = malloc(sizeof(ITEM *) * (size + 1));
    }else{
        void *tmp = realloc(w->items, sizeof(ITEM *) * ( response_internal->cur_size + size + 1));
        if(!tmp){
            cJSON_Delete(json);
            free(response_data->data);
            free(response_data);
            return ERR;
        }
        w->items = tmp;

    }
    if (!w->items) {
        cJSON_Delete(json);
        free(response_data->data);
        free(response_data);
        return ERR;
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
        if (!c_item) {
            cJSON_Delete(json);
            free(response_data->data);
            free(response_data);
            return ERR;
        }
        if (cJSON_HasObjectItem(c_item, "artist")) {
            cJSON *c_artist = cJSON_GetObjectItem(c_item, "artist");
            if (!c_artist) {
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return ERR;
            }
            if (cJSON_HasObjectItem(c_artist, "name")) {
                cJSON *c_name = cJSON_GetObjectItem(c_artist, "name");
                if (!c_name) {
                    cJSON_Delete(json);
                    free(response_data->data);
                    free(response_data);
                    return ERR;
                }
                char *artist_name = cJSON_GetStringValue(c_name);
                if (!artist_name) {
                    cJSON_Delete(json);
                    free(response_data->data);
                    free(response_data);
                    return ERR;
                }
                names.artist = artist_name;
            }
        }
        if (cJSON_HasObjectItem(c_item, "title")) {
            cJSON *c_title = cJSON_GetObjectItem(c_item, "title");
            if (!c_title) {
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return ERR;
            }
            char *title = cJSON_GetStringValue(c_title);
            if (!title) {
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return ERR;
            }
            names.title = title;
        }
        if (cJSON_HasObjectItem(c_item, "nb_tracks")) {
            cJSON *c_nb_tracks = cJSON_GetObjectItem(c_item, "nb_tracks");
            if (!c_nb_tracks) {
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return ERR;
            }
            int nb_tracks = (int)cJSON_GetNumberValue(c_nb_tracks);
            char *nb_tracks_str = int_to_string(" (%d)", nb_tracks);
            if (!nb_tracks_str) {
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return ERR;
            }
            names.nb_tracks = nb_tracks_str;
        }

        cJSON *c_id = cJSON_GetObjectItem(c_item, "id");
        if (!c_id) {
            cJSON_Delete(json);
            free(response_data->data);
            free(response_data);
            return ERR;
        }

        char *id_item = int_to_string(NULL, (int)cJSON_GetNumberValue(c_id));
        if (!id_item) {
            cJSON_Delete(json);
            free(response_data->data);
            free(response_data);
            return ERR;
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
        w->items[response_internal->cur_size] = new_item(filter_chars(name_item), id_item);
        
        response_internal->cur_size++;

    }

    w->items[response_internal->cur_size + 1] = NULL;
    
    Menu_Options_Seeting options = {.on = O_ONEVALUE, .off = O_SHOWDESC};
    create_menu(w, options);

    cJSON_Delete(json);
    free(response_data->data);
    free(response_data);
    return 1;
}


char *int_to_string(char *format, int i) {
    char *str = calloc(10, sizeof(char));
    sprintf(str, format ? format : "%d", i);
    return (char *)str;
}

int clear_and_write(window_t *w, char *str) {
    if (w == NULL || w->window == NULL) {
        DEBUG("Invalid parameters in clear_and_write");
        return ERR;
    }

    if (wclear(w->window) != OK) {
        DEBUG("Error clearing window in clear_and_write");
        return ERR;
    }
    if (box(w->window, 0, 0) != OK) {
        DEBUG("Error drawing box in clear_and_write");
        return ERR;
    }

    if(addLabel(w->window, w->label) != OK){
        DEBUG("Error adding label in clear_and_write");
        return ERR;
    }

    if(str && strlen(str) > 0){
        if (mvwaddstr(w->window, 1, 3, str) != OK) {
            DEBUG("Error adding string in clear_and_write");
            return ERR;
        }
    }
    return OK;
}

int is_chars(char c) {
    static const char valid_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .,!?-_[]{}()";
    return strchr(valid_chars, c) != NULL;
}

char *search_input(const char *label) {
    static window_t *search_window;

    if (strcmp(label, "KILL") == 0) {
        free_window(search_window);
        return NULL;
    }

    if (strcmp(label, "INIT") == 0) {
        search_window = calloc(1, sizeof(window_t));
        if (!search_window) {
            return NULL;
        }

        search_window->y = 3;
        search_window->x = getmaxx(stdscr) * 0.80;
        search_window->starty = (getmaxy(stdscr) / 2) - (search_window->y / 2);
        search_window->startx = (getmaxx(stdscr) / 2) - (search_window->x / 2);

        if (create_win(search_window) != OK) {
            free(search_window);
            return NULL;
        }

        hide_panel(search_window->panel);
        update_panels();
        doupdate();
        return NULL;
    }

    sprintf(search_window->label, "Search %s", label);
    clear_and_write(search_window, NULL);

    top_panel(search_window->panel);
    show_panel(search_window->panel);
    update_panels();
    doupdate();

    char *input = malloc(1);
    if (!input) {
        free_window(search_window);
        return NULL;
    }
    input[0] = '\0';

    int i = 0;
    int c;
    while ((c = getch()) != '\n') {
        if (c == KEY_BACKSPACE) {
            if (i > 0) {
                i--;
                input[i] = '\0';
                char *tmp = realloc(input, i + 1);
                if (tmp) {
                    input = tmp;
                }
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
                clear_and_write(search_window, input);
                update_panels();
                doupdate();
            } else {
                free(input);
                free_window(search_window);
                return NULL;
            }
        }
    }

    clear_and_write(search_window, NULL);
    hide_panel(search_window->panel);
    update_panels();
    doupdate();

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
    w->window = newwin(w->y, w->x, w->starty, w->startx);
    if(!w->window){
        DEBUG("Unable to create window"); 
        return ERR;
    }
    if(box(w->window, 0, 0) != OK){
        DEBUG("Error drawing box in create_win");
        return ERR;
    }
    if(addLabel(w->window, w->label) != OK){
        DEBUG("Error adding label in create_win");
        return ERR;
    }
    w->panel = new_panel(w->window);
    if(!w->panel){
        DEBUG("Error creating panel in create_win");
        return ERR;
    }
    return OK;
}

int addLabel(WINDOW *win, char *str) {
    if (win == NULL || str == NULL) {
        DEBUG("Invalid parameters in addLabel");
        return ERR;
    }

    size_t len = strlen(str);
    char *nstr = malloc(len + 3);
    if (nstr == NULL) {
        DEBUG("Error allocating memory for nstr in addLabel");
        return ERR;
    }

    sprintf(nstr, " %.*s ", (int)len, str);
    int x = getmaxx(win) * 0.10;
    if (mvwaddstr(win, 0, x, nstr) != OK) {
        DEBUG("Error adding string in addLabel");
        free(nstr);
        return ERR;
    }
    free(nstr);
    return OK;
}

int destroy_menu(window_t *w) {
    if (w == NULL || w->menu == NULL) {
        return ERR;
    }

    if (unpost_menu(w->menu) != E_OK) {
        DEBUG("Error unposting menu");
        return ERR;
    }
    if (free_menu(w->menu) != E_OK) {
        DEBUG("Error freeing menu");
        return ERR;
    }
    w->menu = NULL;

    if (w->items == NULL) {
        return OK;
    }

    for (int i = 0; w->items[i] != NULL; i++) {
        if (free_item(w->items[i]) != E_OK) {
            DEBUG("Error freeing item %d", i);
            return ERR;
        }
        w->items[i] = NULL;
    }
    return OK;
}

int create_menu(window_t *w, Menu_Options_Seeting options) {
    if (w == NULL || w->items == NULL) {
        DEBUG("Invalid parameters in create_menu");
        return ERR;
    }
    w->menu = new_menu((ITEM **)w->items);
    if (w->menu == NULL) {
        DEBUG("Error creating new menu");
        return ERR;
    }

    set_menu_win(w->menu, w->window);
    set_menu_sub(w->menu, derwin(w->window, getmaxy(w->window) - 2, getmaxx(w->window) - 2, 1, 1));
    set_menu_format(w->menu, getmaxy(w->window) - 3, 1);
    set_menu_mark(w->menu, " * ");

    menu_opts_off(w->menu, options.off);
    menu_opts_on(w->menu, options.on);

    if (post_menu(w->menu) != E_OK) {
        DEBUG("Error posting menu");
        free_menu(w->menu);
        w->menu = NULL;
        return ERR;
    }
    return OK;
}

int drive_menu(window_t *w, int key) {
    if (w == NULL || w->menu == NULL) {
        DEBUG("Menu does not exist on window %s", (w ? w->label : ""));
        return ERR;
    }
    int i = 0;
    if ((i = menu_driver(w->menu, key)) != E_OK) {
        if(i == E_REQUEST_DENIED){
            return E_REQUEST_DENIED;
        }
        DEBUG("Error driving menu %d", i);
        return ERR;
    }
    return OK;
}

int free_window(window_t *w) {
    if (w != NULL) {
        if (w->menu != NULL) {
            if(destroy_menu(w) != OK){
                DEBUG("Error destroying menu");
                return ERR;
            }
        }
        if (w->window != NULL) {
            if(delwin(w->window) != OK){
                DEBUG("Error deleting window");
                return ERR;
            }
        }
        free(w);
    }
    return OK;
}
