#include "dzr.h"
#include <cjson/cJSON.h>
#include <string.h>
#include <wchar.h>

#define HTTP_PROXY_ENV "HTTP_PROXY_O"

#define API_URL "https://api.deezer.com"
#define GW_URL "https://www.deezer.com/ajax/gw-light.php"
#define MEDIA_URL "https://media.deezer.com/v1/get_url"

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

char *search_input();

char *int_to_string(char *format, int i);

int search_api(const char *path, window_t *w, Menu_Options_Seeting options);

buffer_t *api_url_search(const char *path, const char *query);

buffer_t *api_url_id(const char *path, const char *id);

buffer_t *http_get(char *url);

buffer_t *http_post(char *url, struct curl_slist *headers, cJSON *json);

// https://pubs.opengroup.org/onlinepubs/7908799/xcurses/intovix.html

int main(void) { // int argc, char **argv

    init_curses();

    // Menu_Options_Seeting playlist_options = {
    //     .on = O_ONEVALUE,
    //     .off = O_SHOWDESC
    // };

    Menu_Options_Seeting painel_options = {.on = O_ONEVALUE, .off = O_SHOWDESC};

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
            case TRACK: {
                COMMAND("track");
                if (!search_api("track", painel_w, painel_options)) {
                    DEBUG("Error searching track");
                }
                break;
            }
            case ARTIST: {
                COMMAND("artist");
                if (!search_api("artist", painel_w, painel_options)) {
                    DEBUG("Error searching track");
                }
                break;
            }
            case ALBUM: {
                COMMAND("album");
                if (!search_api("album", painel_w, painel_options)) {
                    DEBUG("Error searching track");
                }
                break;
            }
            case PLAYLIST: {
                COMMAND("playlist");
                if (!search_api("playlist", painel_w, painel_options)) {
                    DEBUG("Error searching track");
                }
                break;
            }
            case USER: {
                COMMAND("user");
                DEBUG("Not implemented");
                break;
            }
            case RADIO: {
                COMMAND("radio");
                DEBUG("Not implemented");
                break;
            }
            default: {
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
    buffer_t *b = (buffer_t *)stream;
    char *tmp = realloc(b->data, b->size + total_size + 1);
    if (!tmp) {
        DEBUG("Error reallocating memory");
        return 0;
    }

    b->data = tmp;
    memcpy(&b->data[b->size], ptr, total_size);
    b->size += total_size;
    b->data[b->size] = 0;
    return total_size;
}

void curl_set_proxy(CURL *curl, char *proxy) {
    char *at = strchr(proxy, '@');
    char *userpwd = NULL;
    char *host = NULL;
    char *port = NULL;

    if (at) {
        // O trecho '@' existe, separa as credenciais do proxy
        size_t len = at - proxy;
        userpwd = (char *)malloc(len + 1); // Aloca espaço para as credenciais
        if (!userpwd) {
            DEBUG("Error allocating memory for userpwd");
            return;
        }
        strncpy(userpwd, proxy, len);
        userpwd[len] = '\0'; // Null-terminate the string

        // Pega o host e a porta após o '@'
        char *hostport = at + 1;
        char *colon = strchr(hostport, ':'); // Verifica se há uma porta

        if (colon) {
            // Separando o host da porta
            host = (char *)malloc(colon - hostport + 1);
            if (!host) {
                DEBUG("Error allocating memory for host");
                free(userpwd);
                return;
            }
            strncpy(host, hostport, colon - hostport);
            host[colon - hostport] = '\0'; // Null-terminate the string

            // Atribui o valor da porta
            port = (char *)malloc(strlen(colon + 1) + 1); // Aloca memória para a porta
            if (!port) {
                DEBUG("Error allocating memory for port");
                free(userpwd);
                free(host);
                return;
            }
            strcpy(port, colon + 1); // Copia a string após o ':'
            curl_easy_setopt(curl, CURLOPT_PROXY, host);
            curl_easy_setopt(curl, CURLOPT_PROXYPORT, atoi(port)); // Converte para inteiro
            free(port);                                            // Libera a memória de 'port' após usá-la
        } else {
            host = (char *)malloc(strlen(hostport) + 1); // Aloca memória para o host
            if (!host) {
                DEBUG("Error allocating memory for host");
                free(userpwd);
                return;
            }
            strcpy(host, hostport); // Copia o host
            curl_easy_setopt(curl, CURLOPT_PROXY, host);
        }

        // Configura as credenciais
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, userpwd);
    } else {
        // Não há credenciais, apenas o proxy sem '@'
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
    }

    // Libera as memórias alocadas
    free(userpwd);
    free(host);
}

buffer_t *http_get(char *url) {

    buffer_t *response_data = malloc(sizeof(buffer_t));
    if (!response_data) {
        DEBUG("Error allocating memory for response_data");
        return NULL;
    }
    response_data->data = malloc(1);
    if (!response_data->data) {
        DEBUG("Error allocating memory for response_data->data");
        free(response_data);
        return NULL;
    }
    response_data->size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        char *proxy = getenv(HTTP_PROXY_ENV);
        if (proxy) {
            curl_set_proxy(curl, proxy);
        }
#ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
#ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            DEBUG("curl_easy_perform() on URL %s failed: %s", url, curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    } else {
        DEBUG("Error initializing CURL");
        free(response_data->data);
        free(response_data);
        response_data = NULL;
    }
    curl_global_cleanup();
    return response_data;
}

buffer_t *http_post(char *url, struct curl_slist *headers, cJSON *json) {

    buffer_t *response_data = malloc(sizeof(buffer_t));
    if (!response_data) {
        DEBUG("Error allocating memory for response_data");
        return NULL;
    }
    response_data->data = malloc(1);
    if (!response_data->data) {
        DEBUG("Error allocating memory for response_data->data");
        free(response_data);
        return NULL;
    }
    response_data->size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        char *proxy = getenv(HTTP_PROXY_ENV);
        if (proxy) {
            curl_set_proxy(curl, proxy);
        }

#ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
#ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
        char *json_str = cJSON_Print(json);
        if (!json_str) {
            DEBUG("Error printing JSON");
            free(response_data->data);
            free(response_data);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return NULL;
        }
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(json_str));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            DEBUG("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        }
        free(json_str);
        curl_easy_cleanup(curl);
    } else {
        DEBUG("Error initializing CURL");
        free(response_data->data);
        free(response_data);
        response_data = NULL;
    }
    curl_global_cleanup();
    return response_data;
}

int search_api(const char *path, window_t *w, Menu_Options_Seeting options) {

    char *input = search_input();

    if (!input || strlen(input) == 0) {
        return 1;
    }

    if (w->items != NULL || w->menu != NULL) {
        destroy_menu(w);
    }

    buffer_t *response_data = api_url_search(path, input);
    if (!response_data) {
        free(input);
        return 0;
    }

    cJSON *json = cJSON_ParseWithLength(response_data->data, response_data->size);
    if (!json) {
        free(input);
        free(response_data->data);
        free(response_data);
        return 0;
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (!data) {
        free(input);
        cJSON_Delete(json);
        free(response_data->data);
        free(response_data);
        return 0;
    }

    int size = cJSON_GetArraySize(data);
    w->items = malloc(sizeof(ITEM *) * (size + 1));
    if (!w->items) {
        free(input);
        cJSON_Delete(json);
        free(response_data->data);
        free(response_data);
        return 0;
    }
    for (int i = 0; i < size; i++) {

        char *name_item = malloc(1);

        cJSON *c_item = cJSON_GetArrayItem(data, i);
        if (!c_item) {
            free(input);
            cJSON_Delete(json);
            free(response_data->data);
            free(response_data);
            return 0;
        }
        if (cJSON_HasObjectItem(c_item, "artist")) {
            cJSON *c_artist = cJSON_GetObjectItem(c_item, "artist");
            if (!c_artist) {
                free(input);
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return 0;
            }
            if (cJSON_HasObjectItem(c_artist, "name")) {
                cJSON *c_name = cJSON_GetObjectItem(c_artist, "name");
                if (!c_name) {
                    free(input);
                    cJSON_Delete(json);
                    free(response_data->data);
                    free(response_data);
                    return 0;
                }
                char *artist_name = cJSON_GetStringValue(c_name);
                if (!artist_name) {
                    free(input);
                    cJSON_Delete(json);
                    free(response_data->data);
                    free(response_data);
                    return 0;
                }
                void *tmp = realloc(name_item, (strlen(artist_name) * sizeof(char)) + (3 * sizeof(char)));
                if (!tmp) {
                    free(input);
                    cJSON_Delete(json);
                    free(response_data->data);
                    free(response_data);
                    return 0;
                }
                name_item = tmp;
                strcat(name_item, artist_name);
                strcat(name_item, " - ");
            }
        }
        if (cJSON_HasObjectItem(c_item, "title")) {
            cJSON *c_title = cJSON_GetObjectItem(c_item, "title");
            if (!c_title) {
                free(input);
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return 0;
            }
            char *title = cJSON_GetStringValue(c_title);
            if (!title) {
                free(input);
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return 0;
            }
            void *tmp = realloc(name_item,  (strlen(title) * sizeof(char)) + (3 * sizeof(char)));
            if (!tmp) {
                free(input);
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return 0;
            }
            name_item = tmp;
            strcat(name_item, title);
            strcat(name_item, " - ");
        }
        if (cJSON_HasObjectItem(c_item, "nb_tracks")) {
            cJSON *c_nb_tracks = cJSON_GetObjectItem(c_item, "nb_tracks");
            if (!c_nb_tracks) {
                free(input);
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return 0;
            }
            int nb_tracks = (int)cJSON_GetNumberValue(c_nb_tracks);
            char *nb_tracks_str = int_to_string(" (%d)", nb_tracks);
            if (!nb_tracks_str) {
                free(input);
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return 0;
            }
            void *tmp = realloc(name_item, strlen(nb_tracks_str) * sizeof(char));
            if (!tmp) {
                free(input);
                cJSON_Delete(json);
                free(response_data->data);
                free(response_data);
                return 0;
            }
            name_item = tmp;
            strcat(name_item, nb_tracks_str);
        }

        cJSON *c_id = cJSON_GetObjectItem(c_item, "id");
        if (!c_id) {
            free(input);
            cJSON_Delete(json);
            free(response_data->data);
            free(response_data);
            return 0;
        }

        char *id_item = int_to_string(NULL, (int)cJSON_GetNumberValue(c_id));
        if (!id_item) {
            free(input);
            cJSON_Delete(json);
            free(response_data->data);
            free(response_data);
            return 0;
        }
        w->items[i] = new_item(name_item, id_item);
    }

    w->items[size] = NULL;

    create_menu(w, options);

    free(input);
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

buffer_t *api_url_search(const char *path, const char *query) {
    char *encoded_query = curl_easy_escape(NULL, query, strlen(query));
    int needed = snprintf(NULL, 0, "%s/search/%s?q=%s&limit=100000", API_URL, path, encoded_query);
    char *url = malloc(needed + 1);
    buffer_t *result = NULL;
    if (url) {
        snprintf(url, needed + 1, "%s/search/%s?q=%s&limit=100000", API_URL, path, encoded_query);
        result = http_get(url);
        free(url);
    } else {
        DEBUG("Error allocating memory for url");
    }
    return result;
}

buffer_t *api_url_id(const char *path, const char *id) {
    int needed = snprintf(NULL, 0, "%s/%s/%s", API_URL, path, id);
    char *url = malloc(needed + 1);
    buffer_t *result = NULL;
    if (url) {
        snprintf(url, needed + 1, "%s/%s/%s", API_URL, path, id);
        result = http_get(url);
        free(url);
    } else {
        DEBUG("Error allocating memory for url");
    }
    return result;
}

void clear_and_write(window_t *w, char *str) {
    if (w == NULL || str == NULL || w->window == NULL) {
        DEBUG("Invalid parameters in clear_and_write");
        return;
    }

    wclear(w->window);
    if (box(w->window, 0, 0) == ERR) {
        DEBUG("Error drawing box in clear_and_write");
        return;
    }

    addLabel(w->window, w->label);

    if (mvwaddstr(w->window, 1, 3, str) == ERR) {
        DEBUG("Error adding string in clear_and_write");
        return;
    }

    if (wrefresh(w->window) == ERR) {
        DEBUG("Error refreshing window in clear_and_write");
        return;
    }
}

char *search_input() {
    window_t *w = calloc(1, sizeof(window_t));
    if (!w) {
        DEBUG("Error allocating memory for search window");
        return NULL;
    }

    strcpy(w->label, "Search");
    w->y = 3;
    w->x = getmaxx(stdscr) * 0.80;
    w->starty = (getmaxy(stdscr) / 2) - (w->y / 2);
    w->startx = (getmaxx(stdscr) / 2) - (w->x / 2);

    create_win(w);

    char *track = malloc(1);
    if (!track) {
        DEBUG("Error allocating memory for track");
        free_window(w);
        return NULL;
    }
    track[0] = '\0';

    int i = 0;
    int c;
    while ((c = getch()) != '\n') {
        if (c == KEY_BACKSPACE) {
            if (i > 0) {
                i--;
                track[i] = '\0';
                char *tmp = realloc(track, i + 1);
                if (tmp) {
                    track = tmp;
                }
                clear_and_write(w, track);
            }
            continue;
        }
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' ' || c == '.' || c == ',' || c == '!' || c == '?') {
            char *tmp = realloc(track, i + 2);
            if (tmp) {
                track = tmp;
                track[i++] = c;
                track[i] = '\0';
                clear_and_write(w, track);
            } else {
                DEBUG("Error reallocating memory for track");
                free(track);
                free_window(w);
                return NULL;
            }
        }
    }

    wclear(w->window);
    if (wrefresh(w->window) == ERR) {
        DEBUG("Error refreshing search window");
    }
    delwin(w->window);
    free(w);
    refresh();

    return track;
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
        DEBUG("Invalid parameters in addLabel");
        return;
    }

    size_t len = strlen(str);
    char *nstr = malloc(len + 3);
    if (nstr == NULL) {
        DEBUG("Error allocating memory for nstr in addLabel");
        return;
    }

    sprintf(nstr, " %.*s ", (int)len, str);
    int x = getmaxx(win) * 0.10;
    if (mvwaddstr(win, 0, x, nstr) == ERR) {
        DEBUG("Error adding string in addLabel");
        free(nstr);
        return;
    }
    if (wrefresh(win) == ERR) {
        DEBUG("Error refreshing window in addLabel");
        free(nstr);
        return;
    }
    free(nstr);
}

void destroy_menu(window_t *w) {
    if (w == NULL || w->menu == NULL) {
        return;
    }

    if (unpost_menu(w->menu) != E_OK) {
        DEBUG("Error unposting menu");
    }
    if (free_menu(w->menu) != E_OK) {
        DEBUG("Error freeing menu");
    }
    w->menu = NULL;

    if (w->items == NULL) {
        return;
    }

    for (int i = 0; w->items[i] != NULL; i++) {
        if (free_item(w->items[i]) != E_OK) {
            DEBUG("Error freeing item %d", i);
        }
        w->items[i] = NULL;
    }
}

void create_menu(window_t *w, Menu_Options_Seeting options) {
    if (w == NULL || w->items == NULL) {
        DEBUG("Invalid parameters in create_menu");
        return;
    }
    DEBUG("%p", (void *)*((&w->items + 1) - 1));
    w->menu = new_menu((ITEM **)w->items);
    if (w->menu == NULL) {
        DEBUG("Error creating new menu");
        return;
    }

    set_menu_win(w->menu, w->window);
    set_menu_sub(w->menu, derwin(w->window, getmaxy(w->window) - 1, getmaxx(w->window) - 1, 1, 1));
    set_menu_format(w->menu, getmaxy(w->window) - 3, 0);
    set_menu_mark(w->menu, " * ");

    menu_opts_off(w->menu, options.off);
    menu_opts_on(w->menu, options.on);

    if (post_menu(w->menu) != E_OK) {
        DEBUG("Error posting menu");
        free_menu(w->menu);
        w->menu = NULL;
        return;
    }

    if (wrefresh(w->window) == ERR) {
        DEBUG("Error refreshing window in create_menu");
    }
    if (refresh() == ERR) {
        DEBUG("Error refreshing screen in create_menu");
    }
}

void drive_menu(window_t *w, int key) {
    menu_driver(w->menu, key);
    wrefresh(w->window);
    refresh();
}

void free_window(window_t *w) {
    if (w != NULL) {
        if (w->window != NULL) {
            delwin(w->window);
        }
        if (w->menu != NULL) {
            destroy_menu(w);
        }
        free(w);
    }
}

static void logging(char t, char *str, ...) {
    static window_t *logging = NULL;

    static window_t *command = NULL;

    if (str == NULL) {
        if (logging != NULL) {
            free_window(logging);
            logging = NULL;
            DEBUG("Destroyed logging window\n");
        }
        if (command != NULL) {
            free_window(command);
            command = NULL;
            DEBUG("Destroyed command window\n");
        }
        return;
    }

    if (logging == NULL) {
        logging = calloc(1, sizeof(window_t));
        if (logging == NULL) {
            DEBUG("Unable to allocate memory for logging window\n");
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
            DEBUG("Unable to allocate memory for command window\n");
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
            DEBUG("Created command window\n");
        }
        clear_and_write(command, buffer);
    }
    if (t == 'L') {
        if (logging->window == NULL) {
            create_win(logging);
            DEBUG("Created logging window\n");
        }
        clear_and_write(logging, buffer);
    }

    refresh();
    DEBUG("%s", buffer);
}
