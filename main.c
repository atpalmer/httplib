#include <stdio.h>
#include <stdlib.h>
#include "http.h"

#define HTTPBIN_URL_COOKIES                         "https://httpbin.org/cookies"
#define HTTPBIN_URL_BASIC_AUTH(user, password)      "https://httpbin.org/basic-auth/" user "/" password

#define HTTP_AUTH_BASIC_INIT(user, password)        {{HTTP_AUTHTYPE_BASIC}, (user), (password)}

void cookies(void) {
    struct http_opts opts = {
        .cookies = "key1=value1;key2=value2",
        .timeout_secs = 1,
    };
    struct http_response *resp = http_request("GET", HTTPBIN_URL_COOKIES, &opts);
    if(!resp) {
        printf("Error Code: %d\n", http_curl_error_code);
        printf("Error Text: %s\n", http_curl_error_text());
    }
    printf("CONTENT (%lu):\n%s\n\n", http_response_content_length(resp), http_response_content(resp));
    printf("HEADERS (%lu):\n%s\n\n", http_response_headers_length(resp), http_response_headers(resp));
    printf("URL: %s\n", http_response_url(resp));
    printf("Status Code: %d\n", http_response_status_code(resp));
    printf("Elapsed Seconds: %f\n", http_response_elapsed(resp));
    http_response_free(resp);
}

void auth_basic(void) {
    struct http_auth_basic auth = HTTP_AUTH_BASIC_INIT("me", "p4$$w0rd");
    struct http_opts opts = {
        .timeout_secs = 1,
        .auth = (struct http_auth *)&auth,
    };
    struct http_response *resp = http_request("GET", HTTPBIN_URL_BASIC_AUTH("me", "p4$$w0rd"), &opts);
    if(!resp) {
        printf("Error Code: %d\n", http_curl_error_code);
        printf("Error Text: %s\n", http_curl_error_text());
    }
    printf("CONTENT (%lu):\n%s\n\n", http_response_content_length(resp), http_response_content(resp));
    printf("HEADERS (%lu):\n%s\n\n", http_response_headers_length(resp), http_response_headers(resp));
    printf("URL: %s\n", http_response_url(resp));
    printf("Status Code: %d\n", http_response_status_code(resp));
    printf("Elapsed Seconds: %f\n", http_response_elapsed(resp));
    http_response_free(resp);
}

int main(void) {
    /* cookies(); */
    auth_basic();
}
