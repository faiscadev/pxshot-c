/**
 * @file header_only.c
 * @brief Example using pxshot as a header-only library
 * 
 * To use pxshot as header-only, define PXSHOT_IMPLEMENTATION before
 * including the header in exactly ONE source file:
 * 
 *   #define PXSHOT_IMPLEMENTATION
 *   #include <pxshot.h>
 * 
 * All other source files can include the header normally.
 * 
 * Compile with:
 *   gcc -I../include header_only.c -lcurl -o header_only
 */

#define PXSHOT_IMPLEMENTATION
#include <pxshot.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *api_key = getenv("PXSHOT_API_KEY");
    if (!api_key) {
        fprintf(stderr, "Set PXSHOT_API_KEY environment variable\n");
        return 1;
    }
    
    const char *url = argc > 1 ? argv[1] : "https://example.com";
    
    printf("Header-only pxshot v%s\n", pxshot_version());
    printf("Capturing: %s\n", url);
    
    pxshot_client_t *client = pxshot_new(api_key);
    if (!client) {
        fprintf(stderr, "Failed to create client\n");
        return 1;
    }
    
    pxshot_screenshot_opts_t opts = {.url = url};
    pxshot_response_t *resp = pxshot_screenshot(client, &opts);
    
    if (resp->error == PXSHOT_OK) {
        printf("Success! Received %zu bytes\n", resp->data_len);
        
        /* Save to file */
        FILE *fp = fopen("screenshot.png", "wb");
        if (fp) {
            fwrite(resp->data, 1, resp->data_len, fp);
            fclose(fp);
            printf("Saved to screenshot.png\n");
        }
    } else {
        fprintf(stderr, "Error: %s\n", pxshot_error_string(resp->error));
        if (resp->error_message) {
            fprintf(stderr, "  %s\n", resp->error_message);
        }
    }
    
    pxshot_response_free(resp);
    pxshot_free(client);
    
    return resp->error == PXSHOT_OK ? 0 : 1;
}
