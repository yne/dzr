#include "requests.h"
#include "logging.h"
#include <string.h>
#include <stdlib.h>

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

