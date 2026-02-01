# Pxshot C SDK

Official C SDK for the [Pxshot](https://pxshot.com) screenshot API.

## Features

- C11 standard
- Header-only mode available
- Clean memory management with documented ownership
- Comprehensive error handling
- CMake and pkg-config support
- Minimal dependencies (libcurl only; cJSON bundled)

## Installation

### CMake (Recommended)

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

### Header-Only Mode

Copy `include/pxshot.h` and `include/cJSON.h` to your project. In exactly ONE source file:

```c
#define PXSHOT_IMPLEMENTATION
#include "pxshot.h"
```

All other files can include the header normally:

```c
#include "pxshot.h"
```

Compile with `-lcurl`.

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `PXSHOT_BUILD_SHARED` | ON | Build shared library |
| `PXSHOT_BUILD_STATIC` | ON | Build static library |
| `PXSHOT_BUILD_EXAMPLES` | ON | Build example programs |
| `PXSHOT_USE_SYSTEM_CJSON` | OFF | Use system cJSON instead of bundled |
| `PXSHOT_HEADER_ONLY` | OFF | Install headers only |

## Quick Start

```c
#include <pxshot.h>
#include <stdio.h>

int main() {
    // Create client
    pxshot_client_t *client = pxshot_new("px_your_api_key");
    
    // Capture screenshot
    pxshot_screenshot_opts_t opts = {
        .url = "https://example.com",
        .width = 1280,
        .height = 720
    };
    
    pxshot_response_t *resp = pxshot_screenshot(client, &opts);
    
    if (resp->error == PXSHOT_OK) {
        // Save image
        FILE *fp = fopen("screenshot.png", "wb");
        fwrite(resp->data, 1, resp->data_len, fp);
        fclose(fp);
        printf("Saved %zu bytes\n", resp->data_len);
    } else {
        printf("Error: %s\n", pxshot_error_string(resp->error));
    }
    
    // Cleanup (always free response and client)
    pxshot_response_free(resp);
    pxshot_free(client);
    
    return 0;
}
```

## API Reference

### Client Lifecycle

```c
// Create with API key (uses default base URL)
pxshot_client_t *pxshot_new(const char *api_key);

// Create with full configuration
pxshot_config_t config = {
    .api_key = "px_...",
    .base_url = "https://api.pxshot.com",  // optional
    .timeout_ms = 30000                     // optional
};
pxshot_client_t *pxshot_new_with_config(&config);

// Free client (safe to pass NULL)
void pxshot_free(pxshot_client_t *client);
```

### Screenshot Capture

```c
pxshot_screenshot_opts_t opts = {
    .url = "https://example.com",      // required
    .format = PXSHOT_FORMAT_PNG,       // PNG, JPEG, WEBP
    .quality = 80,                     // 1-100 for JPEG/WebP
    .width = 1280,                     // viewport width
    .height = 720,                     // viewport height
    .full_page = false,                // capture full page
    .wait_until = PXSHOT_WAIT_LOAD,    // LOAD, DOMCONTENTLOADED, NETWORKIDLE
    .wait_for_selector = NULL,         // CSS selector to wait for
    .wait_for_timeout = 0,             // max wait time (ms)
    .device_scale_factor = 1.0,        // pixel ratio
    .store = false,                    // true = return URL, false = return bytes
    .block_ads = true                  // block ads and trackers
};

pxshot_response_t *pxshot_screenshot(client, &opts);
```

### Response Handling

```c
// Response contains either image bytes OR stored URL info
typedef struct {
    pxshot_error_t error;      // PXSHOT_OK on success
    int http_status;           // HTTP status code
    char *error_message;       // Human-readable error (may be NULL)
    
    // For store=false: raw image bytes
    uint8_t *data;
    size_t data_len;
    
    // For store=true: stored image info
    pxshot_stored_t *stored;
} pxshot_response_t;

// Stored image info
typedef struct {
    char *url;           // URL to access the image
    char *expires_at;    // ISO8601 expiration
    int width, height;   // Dimensions
    size_t size_bytes;   // File size
} pxshot_stored_t;

// Always free the response when done
void pxshot_response_free(pxshot_response_t *resp);
```

### Usage Statistics

```c
pxshot_usage_t *usage = NULL;
pxshot_response_t *resp = pxshot_get_usage(client, &usage);

if (resp->error == PXSHOT_OK) {
    printf("Screenshots: %d/%d\n", 
           usage->screenshots_used, 
           usage->screenshots_limit);
}

pxshot_usage_free(usage);
pxshot_response_free(resp);
```

### Error Handling

```c
typedef enum {
    PXSHOT_OK = 0,
    PXSHOT_ERR_INVALID_ARG,
    PXSHOT_ERR_OUT_OF_MEMORY,
    PXSHOT_ERR_CURL_INIT,
    PXSHOT_ERR_CURL_PERFORM,
    PXSHOT_ERR_HTTP_ERROR,
    PXSHOT_ERR_JSON_PARSE,
    PXSHOT_ERR_API_ERROR,
    PXSHOT_ERR_TIMEOUT,
    PXSHOT_ERR_UNKNOWN
} pxshot_error_t;

// Get human-readable error string
const char *pxshot_error_string(pxshot_error_t error);
```

## Memory Management

**Ownership rules:**
- Caller owns all returned pointers and must free them
- `pxshot_response_free()` frees the response and all its contents
- `pxshot_usage_free()` frees usage statistics
- `pxshot_free()` frees the client
- All functions are NULL-safe (passing NULL is a no-op)

## Examples

Build and run examples:

```bash
mkdir build && cd build
cmake ..
make

# Set your API key
export PXSHOT_API_KEY="px_your_api_key"

# Run examples
./example_basic https://example.com screenshot.png
./example_store https://example.com
./example_usage
```

## Thread Safety

The client can be used from multiple threads concurrently. Each request uses its own CURL easy handle internally.

## Dependencies

- **libcurl**: HTTP client (required)
- **cJSON**: JSON parsing (bundled, or use system with `-DPXSHOT_USE_SYSTEM_CJSON=ON`)

## License

MIT License - see [LICENSE](LICENSE)
