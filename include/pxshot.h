/**
 * @file pxshot.h
 * @brief Pxshot Screenshot API - Official C SDK
 * @version 1.0.0
 * 
 * A simple, clean C11 SDK for the Pxshot screenshot API.
 * 
 * @example
 * @code
 * #include <pxshot.h>
 * 
 * int main() {
 *     pxshot_client_t *client = pxshot_new("px_your_api_key");
 *     pxshot_screenshot_opts_t opts = {.url = "https://example.com"};
 *     pxshot_response_t *resp = pxshot_screenshot(client, &opts);
 *     
 *     if (resp->error == PXSHOT_OK) {
 *         // Use resp->data and resp->data_len
 *     }
 *     
 *     pxshot_response_free(resp);
 *     pxshot_free(client);
 *     return 0;
 * }
 * @endcode
 */

#ifndef PXSHOT_H
#define PXSHOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Version information */
#define PXSHOT_VERSION_MAJOR 1
#define PXSHOT_VERSION_MINOR 0
#define PXSHOT_VERSION_PATCH 0
#define PXSHOT_VERSION_STRING "1.0.0"

/* Default API base URL */
#define PXSHOT_DEFAULT_BASE_URL "https://api.pxshot.com"

/**
 * @brief Error codes returned by SDK functions
 */
typedef enum {
    PXSHOT_OK = 0,              /**< Success */
    PXSHOT_ERR_INVALID_ARG,     /**< Invalid argument passed */
    PXSHOT_ERR_OUT_OF_MEMORY,   /**< Memory allocation failed */
    PXSHOT_ERR_CURL_INIT,       /**< Failed to initialize CURL */
    PXSHOT_ERR_CURL_PERFORM,    /**< CURL request failed */
    PXSHOT_ERR_HTTP_ERROR,      /**< HTTP error (check http_status) */
    PXSHOT_ERR_JSON_PARSE,      /**< Failed to parse JSON response */
    PXSHOT_ERR_API_ERROR,       /**< API returned an error */
    PXSHOT_ERR_TIMEOUT,         /**< Request timed out */
    PXSHOT_ERR_UNKNOWN          /**< Unknown error */
} pxshot_error_t;

/**
 * @brief Image format options
 */
typedef enum {
    PXSHOT_FORMAT_PNG = 0,      /**< PNG format (default) */
    PXSHOT_FORMAT_JPEG,         /**< JPEG format */
    PXSHOT_FORMAT_WEBP          /**< WebP format */
} pxshot_format_t;

/**
 * @brief Wait until page load condition
 */
typedef enum {
    PXSHOT_WAIT_LOAD = 0,       /**< Wait for 'load' event (default) */
    PXSHOT_WAIT_DOMCONTENTLOADED, /**< Wait for DOMContentLoaded */
    PXSHOT_WAIT_NETWORKIDLE     /**< Wait for network idle */
} pxshot_wait_until_t;

/**
 * @brief Opaque client handle
 * 
 * Created with pxshot_new(), freed with pxshot_free().
 * Thread-safe for concurrent requests.
 */
typedef struct pxshot_client pxshot_client_t;

/**
 * @brief Client configuration options
 */
typedef struct {
    const char *api_key;        /**< API key (required) */
    const char *base_url;       /**< Base URL (optional, defaults to https://api.pxshot.com) */
    long timeout_ms;            /**< Request timeout in milliseconds (0 = default 30s) */
} pxshot_config_t;

/**
 * @brief Screenshot request options
 * 
 * All fields except 'url' are optional. Zero-initialize for defaults:
 * @code
 * pxshot_screenshot_opts_t opts = {0};
 * opts.url = "https://example.com";
 * @endcode
 */
typedef struct {
    const char *url;            /**< URL to screenshot (required) */
    pxshot_format_t format;     /**< Image format (default: PNG) */
    int quality;                /**< JPEG/WebP quality 1-100 (0 = default 80) */
    int width;                  /**< Viewport width (0 = default 1280) */
    int height;                 /**< Viewport height (0 = default 720) */
    bool full_page;             /**< Capture full scrollable page */
    pxshot_wait_until_t wait_until; /**< Wait condition */
    const char *wait_for_selector;  /**< CSS selector to wait for */
    int wait_for_timeout;       /**< Max wait time in ms (0 = default) */
    double device_scale_factor; /**< Device pixel ratio (0 = default 1.0) */
    bool store;                 /**< Store image and return URL instead of bytes */
    bool block_ads;             /**< Block ads and trackers */
} pxshot_screenshot_opts_t;

/**
 * @brief Stored screenshot information (when store=true)
 */
typedef struct {
    char *url;                  /**< URL to access the stored image */
    char *expires_at;           /**< ISO8601 expiration timestamp */
    int width;                  /**< Image width in pixels */
    int height;                 /**< Image height in pixels */
    size_t size_bytes;          /**< Image size in bytes */
} pxshot_stored_t;

/**
 * @brief API response container
 * 
 * Ownership: The caller owns this struct and must free it with pxshot_response_free().
 * 
 * For screenshot requests:
 * - If store=false: data/data_len contain image bytes, stored is NULL
 * - If store=true: stored contains URL info, data is NULL
 */
typedef struct {
    pxshot_error_t error;       /**< Error code (PXSHOT_OK on success) */
    int http_status;            /**< HTTP status code (0 if request failed) */
    char *error_message;        /**< Human-readable error message (may be NULL) */
    
    /* Response data (mutually exclusive based on request type) */
    uint8_t *data;              /**< Binary response data (caller owns) */
    size_t data_len;            /**< Length of data in bytes */
    
    pxshot_stored_t *stored;    /**< Stored image info (for store=true) */
} pxshot_response_t;

/**
 * @brief Usage statistics
 */
typedef struct {
    int screenshots_used;       /**< Screenshots taken this period */
    int screenshots_limit;      /**< Screenshot limit for plan */
    int storage_used_bytes;     /**< Storage bytes used */
    int storage_limit_bytes;    /**< Storage limit for plan */
    char *period_start;         /**< Billing period start (ISO8601) */
    char *period_end;           /**< Billing period end (ISO8601) */
} pxshot_usage_t;

/* ============================================================================
 * Client Lifecycle
 * ============================================================================ */

/**
 * @brief Create a new Pxshot client with an API key
 * 
 * @param api_key Your Pxshot API key (must not be NULL)
 * @return New client instance, or NULL on failure
 * 
 * @note The client copies the API key, so the original can be freed.
 * @note Free with pxshot_free() when done.
 */
pxshot_client_t *pxshot_new(const char *api_key);

/**
 * @brief Create a new Pxshot client with full configuration
 * 
 * @param config Configuration options
 * @return New client instance, or NULL on failure
 */
pxshot_client_t *pxshot_new_with_config(const pxshot_config_t *config);

/**
 * @brief Free a Pxshot client and all associated resources
 * 
 * @param client Client to free (safe to pass NULL)
 */
void pxshot_free(pxshot_client_t *client);

/* ============================================================================
 * API Operations
 * ============================================================================ */

/**
 * @brief Capture a screenshot
 * 
 * @param client Pxshot client
 * @param opts Screenshot options (url is required)
 * @return Response containing image data or stored URL info
 * 
 * @note Caller must free the response with pxshot_response_free()
 */
pxshot_response_t *pxshot_screenshot(pxshot_client_t *client, 
                                      const pxshot_screenshot_opts_t *opts);

/**
 * @brief Get usage statistics
 * 
 * @param client Pxshot client
 * @param usage Output parameter for usage stats
 * @return Response with error info (data fields unused)
 * 
 * @note Caller must free both the response and usage with respective free functions
 */
pxshot_response_t *pxshot_get_usage(pxshot_client_t *client, pxshot_usage_t **usage);

/* ============================================================================
 * Memory Management
 * ============================================================================ */

/**
 * @brief Free a response structure
 * 
 * @param resp Response to free (safe to pass NULL)
 */
void pxshot_response_free(pxshot_response_t *resp);

/**
 * @brief Free a usage structure
 * 
 * @param usage Usage to free (safe to pass NULL)
 */
void pxshot_usage_free(pxshot_usage_t *usage);

/* ============================================================================
 * Utilities
 * ============================================================================ */

/**
 * @brief Get a human-readable error message for an error code
 * 
 * @param error Error code
 * @return Static string describing the error
 */
const char *pxshot_error_string(pxshot_error_t error);

/**
 * @brief Get the SDK version string
 * 
 * @return Version string (e.g., "1.0.0")
 */
const char *pxshot_version(void);

/* ============================================================================
 * Header-Only Implementation (optional)
 * 
 * Define PXSHOT_IMPLEMENTATION before including this header in exactly ONE
 * source file to include the implementation:
 * 
 *   #define PXSHOT_IMPLEMENTATION
 *   #include <pxshot.h>
 * 
 * ============================================================================ */

#ifdef PXSHOT_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>

/* Bundled cJSON (minimal subset) - or use system cJSON */
#ifndef PXSHOT_USE_SYSTEM_CJSON
#include "cJSON.h"
#else
#include <cjson/cJSON.h>
#endif

/* Internal client structure */
struct pxshot_client {
    char *api_key;
    char *base_url;
    long timeout_ms;
    CURL *curl;
};

/* CURL write callback data */
typedef struct {
    uint8_t *data;
    size_t len;
    size_t cap;
} pxshot_buffer_t;

/* Internal helpers */
static size_t pxshot_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    pxshot_buffer_t *buf = (pxshot_buffer_t *)userp;
    
    if (buf->len + realsize + 1 > buf->cap) {
        size_t newcap = (buf->cap == 0) ? 4096 : buf->cap * 2;
        while (newcap < buf->len + realsize + 1) newcap *= 2;
        uint8_t *newdata = (uint8_t *)realloc(buf->data, newcap);
        if (!newdata) return 0;
        buf->data = newdata;
        buf->cap = newcap;
    }
    
    memcpy(buf->data + buf->len, contents, realsize);
    buf->len += realsize;
    buf->data[buf->len] = 0;
    return realsize;
}

static char *pxshot_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *dup = (char *)malloc(len + 1);
    if (dup) memcpy(dup, s, len + 1);
    return dup;
}

static pxshot_response_t *pxshot_response_new(void) {
    pxshot_response_t *resp = (pxshot_response_t *)calloc(1, sizeof(pxshot_response_t));
    return resp;
}

static void pxshot_set_error(pxshot_response_t *resp, pxshot_error_t err, const char *msg) {
    resp->error = err;
    if (msg) resp->error_message = pxshot_strdup(msg);
}

static const char *pxshot_format_string(pxshot_format_t fmt) {
    switch (fmt) {
        case PXSHOT_FORMAT_JPEG: return "jpeg";
        case PXSHOT_FORMAT_WEBP: return "webp";
        default: return "png";
    }
}

static const char *pxshot_wait_until_string(pxshot_wait_until_t w) {
    switch (w) {
        case PXSHOT_WAIT_DOMCONTENTLOADED: return "domcontentloaded";
        case PXSHOT_WAIT_NETWORKIDLE: return "networkidle";
        default: return "load";
    }
}

/* Public API Implementation */

pxshot_client_t *pxshot_new(const char *api_key) {
    pxshot_config_t config = {
        .api_key = api_key,
        .base_url = NULL,
        .timeout_ms = 0
    };
    return pxshot_new_with_config(&config);
}

pxshot_client_t *pxshot_new_with_config(const pxshot_config_t *config) {
    if (!config || !config->api_key) return NULL;
    
    pxshot_client_t *client = (pxshot_client_t *)calloc(1, sizeof(pxshot_client_t));
    if (!client) return NULL;
    
    client->api_key = pxshot_strdup(config->api_key);
    client->base_url = pxshot_strdup(config->base_url ? config->base_url : PXSHOT_DEFAULT_BASE_URL);
    client->timeout_ms = config->timeout_ms > 0 ? config->timeout_ms : 30000;
    
    if (!client->api_key || !client->base_url) {
        pxshot_free(client);
        return NULL;
    }
    
    client->curl = curl_easy_init();
    if (!client->curl) {
        pxshot_free(client);
        return NULL;
    }
    
    return client;
}

void pxshot_free(pxshot_client_t *client) {
    if (!client) return;
    if (client->curl) curl_easy_cleanup(client->curl);
    free(client->api_key);
    free(client->base_url);
    free(client);
}

pxshot_response_t *pxshot_screenshot(pxshot_client_t *client,
                                      const pxshot_screenshot_opts_t *opts) {
    pxshot_response_t *resp = pxshot_response_new();
    if (!resp) return NULL;
    
    if (!client || !opts || !opts->url) {
        pxshot_set_error(resp, PXSHOT_ERR_INVALID_ARG, "client and opts->url are required");
        return resp;
    }
    
    /* Build JSON body */
    cJSON *body = cJSON_CreateObject();
    if (!body) {
        pxshot_set_error(resp, PXSHOT_ERR_OUT_OF_MEMORY, "failed to create JSON");
        return resp;
    }
    
    cJSON_AddStringToObject(body, "url", opts->url);
    cJSON_AddStringToObject(body, "format", pxshot_format_string(opts->format));
    
    if (opts->quality > 0)
        cJSON_AddNumberToObject(body, "quality", opts->quality);
    if (opts->width > 0)
        cJSON_AddNumberToObject(body, "width", opts->width);
    if (opts->height > 0)
        cJSON_AddNumberToObject(body, "height", opts->height);
    if (opts->full_page)
        cJSON_AddBoolToObject(body, "full_page", true);
    if (opts->wait_until != PXSHOT_WAIT_LOAD)
        cJSON_AddStringToObject(body, "wait_until", pxshot_wait_until_string(opts->wait_until));
    if (opts->wait_for_selector)
        cJSON_AddStringToObject(body, "wait_for_selector", opts->wait_for_selector);
    if (opts->wait_for_timeout > 0)
        cJSON_AddNumberToObject(body, "wait_for_timeout", opts->wait_for_timeout);
    if (opts->device_scale_factor > 0)
        cJSON_AddNumberToObject(body, "device_scale_factor", opts->device_scale_factor);
    if (opts->store)
        cJSON_AddBoolToObject(body, "store", true);
    
    char *json_str = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);
    
    if (!json_str) {
        pxshot_set_error(resp, PXSHOT_ERR_OUT_OF_MEMORY, "failed to serialize JSON");
        return resp;
    }
    
    /* Build URL */
    size_t url_len = strlen(client->base_url) + 32;
    char *url = (char *)malloc(url_len);
    if (!url) {
        free(json_str);
        pxshot_set_error(resp, PXSHOT_ERR_OUT_OF_MEMORY, "failed to allocate URL");
        return resp;
    }
    snprintf(url, url_len, "%s/v1/screenshot", client->base_url);
    
    /* Build auth header */
    size_t auth_len = strlen(client->api_key) + 32;
    char *auth_header = (char *)malloc(auth_len);
    if (!auth_header) {
        free(json_str);
        free(url);
        pxshot_set_error(resp, PXSHOT_ERR_OUT_OF_MEMORY, "failed to allocate header");
        return resp;
    }
    snprintf(auth_header, auth_len, "Authorization: Bearer %s", client->api_key);
    
    /* Setup CURL */
    CURL *curl = client->curl;
    curl_easy_reset(curl);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    pxshot_buffer_t buffer = {0};
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pxshot_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    free(auth_header);
    free(url);
    free(json_str);
    
    if (res != CURLE_OK) {
        free(buffer.data);
        if (res == CURLE_OPERATION_TIMEDOUT) {
            pxshot_set_error(resp, PXSHOT_ERR_TIMEOUT, curl_easy_strerror(res));
        } else {
            pxshot_set_error(resp, PXSHOT_ERR_CURL_PERFORM, curl_easy_strerror(res));
        }
        return resp;
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    resp->http_status = (int)http_code;
    
    if (http_code >= 400) {
        /* Try to parse error message from JSON */
        cJSON *err_json = cJSON_Parse((char *)buffer.data);
        if (err_json) {
            cJSON *msg = cJSON_GetObjectItem(err_json, "error");
            if (msg && cJSON_IsString(msg)) {
                resp->error_message = pxshot_strdup(msg->valuestring);
            }
            cJSON_Delete(err_json);
        }
        free(buffer.data);
        pxshot_set_error(resp, PXSHOT_ERR_HTTP_ERROR, 
                         resp->error_message ? resp->error_message : "HTTP error");
        return resp;
    }
    
    /* Check if response is JSON (stored) or binary (image) */
    char *content_type = NULL;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
    
    if (opts->store || (content_type && strstr(content_type, "application/json"))) {
        /* Parse stored response */
        cJSON *json = cJSON_Parse((char *)buffer.data);
        free(buffer.data);
        
        if (!json) {
            pxshot_set_error(resp, PXSHOT_ERR_JSON_PARSE, "failed to parse response JSON");
            return resp;
        }
        
        resp->stored = (pxshot_stored_t *)calloc(1, sizeof(pxshot_stored_t));
        if (!resp->stored) {
            cJSON_Delete(json);
            pxshot_set_error(resp, PXSHOT_ERR_OUT_OF_MEMORY, "failed to allocate stored struct");
            return resp;
        }
        
        cJSON *item;
        if ((item = cJSON_GetObjectItem(json, "url")) && cJSON_IsString(item))
            resp->stored->url = pxshot_strdup(item->valuestring);
        if ((item = cJSON_GetObjectItem(json, "expires_at")) && cJSON_IsString(item))
            resp->stored->expires_at = pxshot_strdup(item->valuestring);
        if ((item = cJSON_GetObjectItem(json, "width")) && cJSON_IsNumber(item))
            resp->stored->width = item->valueint;
        if ((item = cJSON_GetObjectItem(json, "height")) && cJSON_IsNumber(item))
            resp->stored->height = item->valueint;
        if ((item = cJSON_GetObjectItem(json, "size_bytes")) && cJSON_IsNumber(item))
            resp->stored->size_bytes = (size_t)item->valuedouble;
        
        cJSON_Delete(json);
    } else {
        /* Binary image data */
        resp->data = buffer.data;
        resp->data_len = buffer.len;
    }
    
    resp->error = PXSHOT_OK;
    return resp;
}

pxshot_response_t *pxshot_get_usage(pxshot_client_t *client, pxshot_usage_t **usage) {
    pxshot_response_t *resp = pxshot_response_new();
    if (!resp) return NULL;
    
    if (!client || !usage) {
        pxshot_set_error(resp, PXSHOT_ERR_INVALID_ARG, "client and usage are required");
        return resp;
    }
    
    *usage = NULL;
    
    /* Build URL */
    size_t url_len = strlen(client->base_url) + 32;
    char *url = (char *)malloc(url_len);
    if (!url) {
        pxshot_set_error(resp, PXSHOT_ERR_OUT_OF_MEMORY, "failed to allocate URL");
        return resp;
    }
    snprintf(url, url_len, "%s/v1/usage", client->base_url);
    
    /* Build auth header */
    size_t auth_len = strlen(client->api_key) + 32;
    char *auth_header = (char *)malloc(auth_len);
    if (!auth_header) {
        free(url);
        pxshot_set_error(resp, PXSHOT_ERR_OUT_OF_MEMORY, "failed to allocate header");
        return resp;
    }
    snprintf(auth_header, auth_len, "Authorization: Bearer %s", client->api_key);
    
    /* Setup CURL */
    CURL *curl = client->curl;
    curl_easy_reset(curl);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    
    pxshot_buffer_t buffer = {0};
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pxshot_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    free(auth_header);
    free(url);
    
    if (res != CURLE_OK) {
        free(buffer.data);
        pxshot_set_error(resp, PXSHOT_ERR_CURL_PERFORM, curl_easy_strerror(res));
        return resp;
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    resp->http_status = (int)http_code;
    
    if (http_code >= 400) {
        free(buffer.data);
        pxshot_set_error(resp, PXSHOT_ERR_HTTP_ERROR, "HTTP error");
        return resp;
    }
    
    /* Parse JSON response */
    cJSON *json = cJSON_Parse((char *)buffer.data);
    free(buffer.data);
    
    if (!json) {
        pxshot_set_error(resp, PXSHOT_ERR_JSON_PARSE, "failed to parse response JSON");
        return resp;
    }
    
    *usage = (pxshot_usage_t *)calloc(1, sizeof(pxshot_usage_t));
    if (!*usage) {
        cJSON_Delete(json);
        pxshot_set_error(resp, PXSHOT_ERR_OUT_OF_MEMORY, "failed to allocate usage struct");
        return resp;
    }
    
    cJSON *item;
    if ((item = cJSON_GetObjectItem(json, "screenshots_used")) && cJSON_IsNumber(item))
        (*usage)->screenshots_used = item->valueint;
    if ((item = cJSON_GetObjectItem(json, "screenshots_limit")) && cJSON_IsNumber(item))
        (*usage)->screenshots_limit = item->valueint;
    if ((item = cJSON_GetObjectItem(json, "storage_used_bytes")) && cJSON_IsNumber(item))
        (*usage)->storage_used_bytes = item->valueint;
    if ((item = cJSON_GetObjectItem(json, "storage_limit_bytes")) && cJSON_IsNumber(item))
        (*usage)->storage_limit_bytes = item->valueint;
    if ((item = cJSON_GetObjectItem(json, "period_start")) && cJSON_IsString(item))
        (*usage)->period_start = pxshot_strdup(item->valuestring);
    if ((item = cJSON_GetObjectItem(json, "period_end")) && cJSON_IsString(item))
        (*usage)->period_end = pxshot_strdup(item->valuestring);
    
    cJSON_Delete(json);
    resp->error = PXSHOT_OK;
    return resp;
}

void pxshot_response_free(pxshot_response_t *resp) {
    if (!resp) return;
    free(resp->error_message);
    free(resp->data);
    if (resp->stored) {
        free(resp->stored->url);
        free(resp->stored->expires_at);
        free(resp->stored);
    }
    free(resp);
}

void pxshot_usage_free(pxshot_usage_t *usage) {
    if (!usage) return;
    free(usage->period_start);
    free(usage->period_end);
    free(usage);
}

const char *pxshot_error_string(pxshot_error_t error) {
    switch (error) {
        case PXSHOT_OK: return "success";
        case PXSHOT_ERR_INVALID_ARG: return "invalid argument";
        case PXSHOT_ERR_OUT_OF_MEMORY: return "out of memory";
        case PXSHOT_ERR_CURL_INIT: return "failed to initialize CURL";
        case PXSHOT_ERR_CURL_PERFORM: return "CURL request failed";
        case PXSHOT_ERR_HTTP_ERROR: return "HTTP error";
        case PXSHOT_ERR_JSON_PARSE: return "JSON parse error";
        case PXSHOT_ERR_API_ERROR: return "API error";
        case PXSHOT_ERR_TIMEOUT: return "request timed out";
        default: return "unknown error";
    }
}

const char *pxshot_version(void) {
    return PXSHOT_VERSION_STRING;
}

#endif /* PXSHOT_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* PXSHOT_H */
