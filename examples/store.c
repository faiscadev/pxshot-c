/**
 * @file store.c
 * @brief Store screenshot and get URL example
 * 
 * Captures a screenshot, stores it remotely, and returns the URL.
 */

#include <pxshot.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *api_key = getenv("PXSHOT_API_KEY");
    if (!api_key) {
        fprintf(stderr, "Error: PXSHOT_API_KEY environment variable not set\n");
        return 1;
    }
    
    const char *url = argc > 1 ? argv[1] : "https://example.com";
    
    printf("Pxshot C SDK v%s\n", pxshot_version());
    printf("Capturing and storing screenshot of: %s\n", url);
    
    /* Create client */
    pxshot_client_t *client = pxshot_new(api_key);
    if (!client) {
        fprintf(stderr, "Error: Failed to create client\n");
        return 1;
    }
    
    /* Configure screenshot options with store=true */
    pxshot_screenshot_opts_t opts = {
        .url = url,
        .format = PXSHOT_FORMAT_PNG,
        .width = 1920,
        .height = 1080,
        .full_page = true,
        .store = true  /* Store remotely instead of returning bytes */
    };
    
    /* Capture screenshot */
    pxshot_response_t *resp = pxshot_screenshot(client, &opts);
    
    if (resp->error != PXSHOT_OK) {
        fprintf(stderr, "Error: %s", pxshot_error_string(resp->error));
        if (resp->error_message) {
            fprintf(stderr, " - %s", resp->error_message);
        }
        fprintf(stderr, "\n");
        pxshot_response_free(resp);
        pxshot_free(client);
        return 1;
    }
    
    /* Access stored image info */
    if (resp->stored) {
        printf("\nScreenshot stored successfully!\n");
        printf("  URL: %s\n", resp->stored->url);
        printf("  Expires: %s\n", resp->stored->expires_at);
        printf("  Dimensions: %dx%d\n", resp->stored->width, resp->stored->height);
        printf("  Size: %zu bytes\n", resp->stored->size_bytes);
    } else {
        fprintf(stderr, "Error: Expected stored response but got binary data\n");
        pxshot_response_free(resp);
        pxshot_free(client);
        return 1;
    }
    
    /* Cleanup */
    pxshot_response_free(resp);
    pxshot_free(client);
    
    return 0;
}
