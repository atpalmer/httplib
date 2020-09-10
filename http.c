#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include "http.h"

CURLcode http_curl_error_code = CURLE_OK;

#define WRITE_BUFFER_INIT   ((struct http_write_buffer){0})

static size_t write_callback(void *contents, size_t size, size_t count, void *_buffer) {
    struct http_write_buffer *buffer = (struct http_write_buffer *)_buffer;
    size_t new_size = size * count;
    char *new_memory = realloc(buffer->data, buffer->size + new_size + 1);
    if(new_memory == NULL)
        return 0;
    buffer->data = new_memory;
    memcpy(&buffer->data[buffer->size], contents, new_size);
    buffer->size += new_size;
    buffer->data[buffer->size] = '\0';
    return new_size;
}


struct http_response *_http_curl_perform(CURL *curl) {
    struct http_response *result = malloc(sizeof *result);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &result->headers);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result->content);

    result->headers = WRITE_BUFFER_INIT;
    result->content = WRITE_BUFFER_INIT;
    result->curl = curl;

    CURLcode curl_code = curl_easy_perform(curl);
    if(curl_code != CURLE_OK)
        goto fail;
    return result;

fail:
    http_curl_error_code = curl_code;
    curl_easy_cleanup(curl);
    free(result);
    return NULL;
}

struct http_response *http_request(const char *method, const char *url, struct http_opts *opts) {
    if(!method)
        return NULL;
    if(!url)
        return NULL;

    CURL *curl = curl_easy_init();

    /* required arguments */
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* default options */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  /*
                                                          * TODO: reset content & headers?
                                                          * Implement custom logic to follow redirects (with CURLINFO_REDIRECT_URL)?
                                                          * Return linked list of http_response structs? (See Python requests API.)
                                                          */

    /* overrides */
    if(opts) {
        curl_easy_setopt(curl, CURLOPT_PROXY, opts->proxy);
        curl_easy_setopt(curl, CURLOPT_COOKIE, opts->cookies);
        if(opts->cert) {
            curl_easy_setopt(curl, CURLOPT_SSLCERT, opts->cert->cert);
            curl_easy_setopt(curl, CURLOPT_SSLKEY, opts->cert->key);
            curl_easy_setopt(curl, CURLOPT_KEYPASSWD, opts->cert->password);
        }
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, opts->timeout_secs);
        if(opts->auth) {
            switch(opts->auth->type) {
            case HTTP_AUTHTYPE_BASIC:
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
                curl_easy_setopt(curl, CURLOPT_USERNAME, ((struct http_auth_basic *)opts->auth)->username);
                curl_easy_setopt(curl, CURLOPT_PASSWORD, ((struct http_auth_basic *)opts->auth)->password);
                break;
            case HTTP_AUTHTYPE_BEARER:
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
                curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, ((struct http_auth_bearer *)opts->auth)->token);
                break;
            }
        }
    }

    return _http_curl_perform(curl);
}

void http_response_free(struct http_response *this) {
    curl_easy_cleanup(this->curl);
    if(this->headers.data)
        free(this->headers.data);
    if(this->content.data)
        free(this->content.data);
    free(this);
}
