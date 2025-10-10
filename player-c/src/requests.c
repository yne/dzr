#include "requests.h"
#include "logging.h"
#include <string.h>
#include <stdlib.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t total_size = size * nmemb;
    buffer_t *b = (buffer_t *)stream;
    char *tmp = realloc(b->data, b->size + total_size + 1);
    if (!tmp) {
        TRACE("Error reallocating memory");
        return 0;
    }

    b->data = tmp;
    memcpy(&b->data[b->size], ptr, total_size);
    b->size += total_size;
    b->data[b->size] = 0;
    return total_size;
}

void curl_set_proxy(CURL *curl, char *proxy) {
    TRACE("Setting proxy with address: %s", proxy);

    char *hostport = strchr(proxy, '@') ? strchr(proxy, '@') + 1 : proxy;
    char *colon = strchr(hostport, ':');

    TRACE("Determined hostport: %s", hostport);
    curl_easy_setopt(curl, CURLOPT_PROXY, colon ? hostport : proxy);
    if (colon) {
        int port = atoi(colon + 1);
        TRACE("Setting proxy port: %d", port);
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, port);
    }
    
    if (strchr(proxy, '@')) {
        TRACE("Setting proxy user credentials.");
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxy);
    }

    TRACE("Proxy setup complete.");
}

/**
 * @brief Envia uma solicitação HTTP GET para um url.
 *
 * @param url URL a ser enviada a solicitação.
 * @return buffer_t* com a resposta da solicitação.
 */
buffer_t *http_get(const char *url) {
    TRACE("Initializing http_get");

    buffer_t *response_data = malloc(sizeof(buffer_t));
    if (!response_data) {
        TRACE("Error allocating memory for response_data");
        return NULL;
    }
    response_data->data = NULL;
    response_data->size = 0;

    CURL *curl = curl_easy_init();
    if (!curl) {
        TRACE("Error initializing CURL");
        free(response_data);
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);

    char *proxy = getenv(HTTP_PROXY_ENV);
    if (proxy) {
        TRACE("Setting proxy for cURL");
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);

    TRACE("Performing cURL request");
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        TRACE("curl_easy_perform() on URL %s failed: %s", url, curl_easy_strerror(res));
        free(response_data->data);
        free(response_data);
        response_data = NULL;
    } else {
        TRACE("curl_easy_perform() on URL %s completed successfully", url);
    }

    TRACE("Cleaning up cURL");
    curl_easy_cleanup(curl);

    return response_data;
}

/**
 * @brief Envia uma solicita o HTTP POST para um url, com um JSON e headers.
 *
 * @param url URL a ser enviada a solicita o.
 * @param headers Headers a serem incluídos na solicita o.
 * @param json JSON a ser enviado na solicita o.
 * @return buffer_t* com a resposta da solicita o.
 */
buffer_t *http_post(const char *url, struct curl_slist *headers, cJSON *json) {
    TRACE("Initializing http_post");

    buffer_t *response_data = malloc(sizeof(buffer_t));
    if (!response_data) {
        TRACE("Error allocating memory for response_data");
        return NULL;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        TRACE("Error initializing CURL");
        free(response_data);
        return NULL;
    }

    char *proxy = getenv(HTTP_PROXY_ENV);
    if (proxy) {
        TRACE("Setting proxy for cURL");
        curl_set_proxy(curl, proxy);
    }

    TRACE("Setting url: %s", url);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    TRACE("Setting headers");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    TRACE("Setting POST");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    TRACE("Setting timeout");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
    TRACE("Setting nosignal");
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); // for EINTR

    char *json_str = cJSON_Print(json);
    if (!json_str) {
        TRACE("Error printing JSON");
        free(response_data);
        curl_easy_cleanup(curl);
        return NULL;
    }
    TRACE("Setting post fields");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(json_str));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);

    TRACE("Setting write function");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    TRACE("Setting write data");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);

    TRACE("Performing cURL request");
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        TRACE("curl_easy_perform() on URL %s failed: %s", url, curl_easy_strerror(res));
        free(response_data->data);
        free(response_data);
        response_data = NULL;
    } else {
        TRACE("curl_easy_perform() on URL %s completed successfully", url);
    }

    TRACE("Freeing JSON string");
    free(json_str);
    TRACE("Cleaning up cURL");
    curl_easy_cleanup(curl);

    return response_data;
}

/**
 * @brief Retorna um buffer com a resposta a uma solicita o HTTP GET para
 *        a URL <code>API_URL/search/<i>path</i>?q=<i>query</i>&limit=500</code>.
 *
 * @param path Parte da URL entre "search/" e o '?' da query.
 * @param query Query a ser enviada na solicita o.
 * @return buffer_t* com a resposta da solicita o.
 */
buffer_t *api_url_search(const char *path, const char *query) {
    char *encoded_query = curl_easy_escape(NULL, query, strlen(query));
    char url[1024];
    int needed = snprintf(url, sizeof(url), "%s/search/%s?q=%s", API_URL, path, encoded_query);
    if (needed >= (int) sizeof(url)) {
        TRACE("Error building URL");
        free(encoded_query);
        return NULL;
    }
    free(encoded_query);
    return http_get(url);
}

/**
 * @brief Retorna um buffer com a resposta a uma solicita o HTTP GET para
 *        a URL <code>API_URL/<i>path</i>/<i>id</i></code>.
 *
 * @param path Parte da URL entre "https://api.deezer.com" e o <i>id</i>.
 * @param id   ID da entidade a ser retornada.
 * @return buffer_t* com a resposta da solicita o.
 */
buffer_t *api_url_id(const char *path, const char *id) {
    char url[strlen(API_URL) + strlen(path) + strlen(id) + 3];
    sprintf(url, "%s/%s/%s", API_URL, path, id);
    return http_get(url);
}

