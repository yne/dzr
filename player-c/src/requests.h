#ifndef REQUESTS_H
#define REQUESTS_H

#include <curl/curl.h>

#include <cjson/cJSON.h>

#define HTTP_PROXY_ENV "HTTP_PROXY_O"

#define API_URL "https://api.deezer.com"
#define GW_URL "https://www.deezer.com/ajax/gw-light.php"
#define MEDIA_URL "https://media.deezer.com/v1/get_url"

typedef struct {
    char * data;
    size_t size;
} buffer_t;

buffer_t *api_url_search(const char *path, const char *query);

buffer_t *api_url_id(const char *path, const char *id);

buffer_t *http_get(char *url);

buffer_t *http_post(char *url, struct curl_slist *headers, cJSON *json);

#endif /* REQUESTS_H */
