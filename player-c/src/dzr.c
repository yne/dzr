#include "dzr.h"
#include <string.h>
#include <cjson/cJSON.h>

#define API_URL "https://api.deezer.com"
#define GW_URL "https://www.deezer.com/ajax/gw-light.php"
#define MEDIA_URL "https://media.deezer.com/v1/get_url"



enum Commands{
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

struct window_t {
    WINDOW *window;
    MENU *menu;
    ITEM **items;
    int y;
    int x;
    int starty;
    int startx;
    char label[15];
};

typedef struct {
    Menu_Options on;
    Menu_Options off;
} Menu_Options_Seeting;

void create_win(window_t *w);

void create_menu(window_t *w, Menu_Options_Seeting options);

void destroy_menu(window_t *w);

void drive_menu(window_t *w, int key);

void addLabel(WINDOW *win, char *str);

void free_window(window_t *w);

void type_search(window_t *win, char *str);

char * search_input();

buffer_t * api_url_search(const char *path, const char *query);

buffer_t * api_url_id(const char *path, const char *id);

buffer_t * http_get(char *url);

buffer_t * http_post(char *url, struct curl_slist * headers, cJSON *json);

// https://pubs.opengroup.org/onlinepubs/7908799/xcurses/intovix.html


int main(void) { // int argc, char **argv
    
    init_curses();

    // Menu_Options_Seeting playlist_options = {
    //     .on = O_ONEVALUE,
    //     .off = O_SHOWDESC
    // };

    Menu_Options_Seeting painel_options = {
        .on = O_ONEVALUE,
        .off = O_SHOWDESC
    };

    window_t *playlist_w = calloc(1, sizeof(window_t));
    strcpy(playlist_w->label, "Playlist");
    playlist_w->y = layout->yMax - 3;
    playlist_w->x = layout->xDiv;
    playlist_w->starty = 0;
    playlist_w->startx = 0;

    window_t *painel_w = calloc(1, sizeof(window_t));
    strcpy(painel_w->label, "Painel");
    painel_w->y = layout->yDiv;
    painel_w->x = layout->xMax - layout->xDiv;
    painel_w->starty = 0;
    painel_w->startx = layout->xDiv;

    create_win(playlist_w);
    
    create_win(painel_w);

    int ch;
    while ((ch = getch()) != CTRL_D) {
        LOGGING("key: %d char: %c", ch, ch);
        switch (ch) {
        case UP: {
            COMMAND("up");
            drive_menu(painel_w, REQ_UP_ITEM);
            break;
        }
        case DOWN: {
            COMMAND("down");
            drive_menu(painel_w, REQ_DOWN_ITEM);
            break;
        }
        case PAGE_UP: {
            COMMAND("previous page");
            drive_menu(painel_w, REQ_SCR_DPAGE);
            break;
        }
        case PAGE_DOWN: {
            COMMAND("next page");
            drive_menu(painel_w, REQ_SCR_UPAGE);
            break;
        }
        case LEFT: {
            COMMAND("left");
            break;
        }
        case RIGHT: {
            COMMAND("right");
            break;
        }
        case SELECT: {
            COMMAND("backspace");
            ITEM *it = current_item(painel_w->menu);
            int index = item_index(it);
            LOGGING("selecting item %d", index);
            menu_driver(painel_w->menu, REQ_TOGGLE_ITEM);
            break;
        }
        case KEY_F(1): {
            COMMAND("f1");
            
            break;
        }
        case COMMAND: {
            COMMAND("command");
            ch = getch();
            switch (ch) {
                case TRACK:{
                    COMMAND("track");
                    char* track = search_input();
                    if(strlen(track) == 0){
                        break;
                    }
                    buffer_t *response_data = api_url_search("track", track);
                    
                    cJSON *json = cJSON_ParseWithLength(response_data->data, response_data->size);
                    cJSON *data = cJSON_GetObjectItem(json, "data");
                    int size = cJSON_GetArraySize(data);
                    painel_w->items = malloc(sizeof(ITEM *) * size);
                    for (int i = 0; i < size; i++) {
                        cJSON *aItem = cJSON_GetArrayItem(data, i);
                        cJSON *title = cJSON_GetObjectItem(aItem, "title");
                        cJSON *artist = cJSON_GetObjectItem(aItem, "artist");
                        cJSON *artistName = cJSON_GetObjectItem(artist, "name");
                        cJSON *id = cJSON_GetObjectItem(aItem, "id");
                        char * name = cJSON_GetStringValue(title);
                        char * artist_name = cJSON_GetStringValue(artistName);
                        char * name_item = calloc(strlen(name) + strlen(artist_name) + 3, sizeof(char));
                        strcat(name_item, name);
                        strcat(name_item, " - ");
                        strcat(name_item, artist_name);

                        int need = snprintf(NULL, 0, "%0.f", cJSON_GetNumberValue(id));
                        char *id_item = calloc(need + 1, sizeof(char));
                        sprintf(id_item, "%0.f", cJSON_GetNumberValue(id));

                        painel_w->items[i] = new_item(name_item, id_item);
                        
                    }
                    create_menu(painel_w, painel_options);
                    free(track);
                    cJSON_free(json);
                    free(response_data->data);
                    free(response_data);
                    break;
                }
                case ARTIST:{
                    COMMAND("artist");
                    break;
                }
                case ALBUM:{
                    COMMAND("album");
                    break;
                }
                case PLAYLIST:{
                    COMMAND("playlist");
                    break;
                }
                case USER:{
                    COMMAND("user");
                    break;
                }
                case RADIO:{
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
    
    
    free_window(playlist_w);
    free_window(painel_w);

    logging('0', NULL);

    endwin();
    exit(0);
    return 0;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t total_size = size * nmemb;
    buffer_t *b = (buffer_t *) stream;
    char * tmp = realloc(b->data, b->size + total_size + 1);
    if(!tmp) {
        return 0;
    }

    b->data = tmp;
    memcpy(&b->data[b->size], ptr, total_size);
    b->size += total_size;
    b->data[b->size] = 0;
    return total_size;
}
buffer_t * http_get(char *url){
    
    buffer_t *response_data = malloc(sizeof(buffer_t));
    response_data->data = malloc(1);
    response_data->size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        char *proxy = getenv("HTTP_PROXY");
        if(proxy) {
            char *at = strchr(proxy, '@');
            char *userpwd = NULL;
            char *host = NULL;
            char *port = NULL;

            if(at) {
                userpwd = strndup(proxy, at - proxy);
                char *hostport = at + 1;
                char *colon = strchr(hostport, ':');

                if(colon) {
                    host = strndup(hostport, colon - hostport);
                    port = colon + 1;

                    curl_easy_setopt(curl, CURLOPT_PROXY, host);
                    curl_easy_setopt(curl, CURLOPT_PROXYPORT, atoi(port));
                } else {
                    curl_easy_setopt(curl, CURLOPT_PROXY, hostport);
                }
                curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, userpwd);
            } else {
                curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
            }

            free(userpwd);
            free(host);
            free(port);
            free(proxy);
        }
        #ifdef SKIP_PEER_VERIFICATION
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        #endif
        #ifdef SKIP_HOSTNAME_VERIFICATION
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        #endif
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            LOGGING("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return response_data;
}

buffer_t * http_post(char *url, struct curl_slist * headers, cJSON *json) {

    buffer_t *response_data = malloc(sizeof(buffer_t));
    response_data->data = malloc(1);
    response_data->size = 0;


    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        char *proxy = getenv("HTTP_PROXY");
        if(proxy) {
            char *at = strchr(proxy, '@');
            char *userpwd = NULL;
            char *host = NULL;
            char *port = NULL;

            if(at) {
                userpwd = strndup(proxy, at - proxy);
                char *hostport = at + 1;
                char *colon = strchr(hostport, ':');

                if(colon) {
                    host = strndup(hostport, colon - hostport);
                    port = colon + 1;

                    curl_easy_setopt(curl, CURLOPT_PROXY, host);
                    curl_easy_setopt(curl, CURLOPT_PROXYPORT, atoi(port));
                } else {
                    curl_easy_setopt(curl, CURLOPT_PROXY, hostport);
                }
                curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, userpwd);
            } else {
                curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
            }

            free(userpwd);
            free(host);
            free(port);
            free(proxy);
        }
        #ifdef SKIP_PEER_VERIFICATION
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        #endif
        #ifdef SKIP_HOSTNAME_VERIFICATION
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        #endif
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
        char * json_str = cJSON_Print(json);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(json_str));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);
        
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            LOGGING("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return response_data;
}

buffer_t * api_url_search(const char *path, const char *query) {
    int needed = snprintf(NULL, 0, "%s/search/%s?q=%s&limit=100000", API_URL, path, query);
    char *url = malloc(needed + 1);
    if (url) {
        snprintf(url, needed + 1, "%s/search/%s?q=%s&limit=100000", API_URL, path, query);
    }
    return http_get(url);
}

buffer_t * api_url_id(const char *path, const char *id) {
    int needed = snprintf(NULL, 0, "%s/%s/%s", API_URL, path, id);
    char *url = malloc(needed + 1);
    if (url) {
        snprintf(url, needed + 1, "%s/%s/%s", API_URL, path, id);
    }
    return http_get(url);
}

void clear_and_write(window_t *w, char *str) {
    if (w == NULL || str == NULL || w->window == NULL) {
        return;
    }

    wclear(w->window);
    if (box(w->window, 0, 0) == ERR) {
        return;
    }

    addLabel(w->window, w->label);

    if (mvwaddstr(w->window, 1, 3, str) == ERR) {
        return;
    }

    if (wrefresh(w->window) == ERR) {
        return;
    }
}

char * search_input() {

    window_t *w = calloc(1, sizeof(window_t));
    strcpy(w->label, "Search");
    w->y = 3;
    w->x = layout->yDiv;
    w->starty = 1;
    w->startx = layout->xDiv + layout->yDiv;

    create_win(w);
    char *track = calloc(100, sizeof(char));
    type_search(w, track);
    wclear(w->window);
    wrefresh(w->window);
    delwin(w->window);
    free(w);
    refresh();
    return track;
}

void type_search(window_t *win, char str[]) {
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
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' ' || c == '.' || c == ',' || c == '!' || c == '?') {
            str[i++] = c;
            clear_and_write(win, str);
        }
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
    if (win == NULL || str == NULL) {
        return;
    }

    size_t len = strlen(str);
    char *nstr = malloc(len + 3);
    if (nstr == NULL) {
        return;
    }

    sprintf(nstr, " %.*s ", (int)len, str);
    int x = getmaxx(win) * 0.10;
    if (mvwaddstr(win, 0, x, nstr) == ERR) {
        return;
    }
    if (wrefresh(win) == ERR) {
        return;
    }
    free(nstr);
}

void destroy_menu(window_t *w) {
    if (w == NULL || w->menu == NULL) {
        return;
    }

    unpost_menu(w->menu);
    free_menu(w->menu);
    w->menu = NULL;

    if (w->items == NULL) {
        return;
    }

    for (int i = 0; w->items[i] != NULL; i++) {
        free_item(w->items[i]);
        w->items[i] = NULL;
    }
}

void create_menu(window_t *w, Menu_Options_Seeting options) {
    if (w == NULL || w->items == NULL) {
        return;
    }

    w->menu = new_menu((ITEM **) w->items);
    if (w->menu == NULL) {
        return;
    }

    set_menu_win(w->menu, w->window);
    set_menu_sub(w->menu, derwin(w->window, getmaxy(w->window) - 1, getmaxx(w->window) - 1, 1, 1));
    set_menu_format(w->menu, getmaxy(w->window) - 3, 0);
    set_menu_mark(w->menu, " * ");

    menu_opts_off(w->menu, options.off);
    menu_opts_on(w->menu, options.on);

    if (post_menu(w->menu) != E_OK) {
        free_menu(w->menu);
        w->menu = NULL;
        return;
    }

    wrefresh(w->window);
    refresh();
}

void drive_menu(window_t *w, int key) {
    menu_driver(w->menu, key);
    wrefresh(w->window);
    refresh();
}

static void logging(char t, char *str, ...) {
    static window_t *logging = NULL;

    static window_t *command = NULL;

    if (str == NULL) {
        if (logging != NULL) {
            free_window(logging);
            logging = NULL;
            LOG("Destroyed logging window\n");
        }
        if (command != NULL) {
            free_window(command);
            command = NULL;
            LOG("Destroyed command window\n");
        }
        return;
    }

    if (logging == NULL) {
        logging = calloc(1, sizeof(window_t));
        if (logging == NULL) {
            LOG("Unable to allocate memory for logging window\n");
            return;
        }
        strcpy(logging->label, "Logging");
        logging->y = layout->yMax - layout->yDiv;
        logging->x = layout->xMax - layout->xDiv;
        logging->starty = layout->yDiv;
        logging->startx = layout->xDiv;
    }

    if (command == NULL) {
        command = calloc(1, sizeof(window_t));
        if (command == NULL) {
            LOG("Unable to allocate memory for command window\n");
            return;
        }
        strcpy(command->label, "Command");
        command->y = layout->yMax - layout->yDiv;
        command->x = layout->xDiv;
        command->starty = layout->yDiv;
        command->startx = 0;
    }

    va_list args;
    va_start(args, str);

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), str, args);
    va_end(args);
    if (t == 'C') {
        if (command->window == NULL) {
            create_win(command);
            LOG("Created command window\n");
        }
        clear_and_write(command, buffer);
    }
    if (t == 'L') {
        if (logging->window == NULL) {
            create_win(logging);
            LOG("Created logging window\n");
        }
        clear_and_write(logging, buffer);
    }
    
    refresh();
    LOG("%s", buffer);
}

void free_window(window_t *w) {
    if (w != NULL) {
        if(w->window != NULL){
            delwin(w->window);
        }
        if(w->menu != NULL){
            destroy_menu(w);
        }
        free(w);
    }
}

