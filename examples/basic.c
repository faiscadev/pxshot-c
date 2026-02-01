/**
 * @file basic.c
 * @brief Basic screenshot capture example
 * 
 * Captures a screenshot and saves it to a file.
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
    const char *output = argc > 2 ? argv[2] : "screenshot.png";
    
    printf("Pxshot C SDK v%s\n", pxshot_version());
    printf("Capturing screenshot of: %s\n", url);
    
    /* Create client */
    pxshot_client_t *client = pxshot_new(api_key);
    if (!client) {
        fprintf(stderr, "Error: Failed to create client\n");
        return 1;
    }
    
    /* Configure screenshot options */
    pxshot_screenshot_opts_t opts = {
        .url = url,
        .format = PXSHOT_FORMAT_PNG,
        .width = 1280,
        .height = 720,
        .full_page = false,
        .wait_until = PXSHOT_WAIT_LOAD
    };
    
    /* Capture screenshot */
    pxshot_response_t *resp = pxshot_screenshot(client, &opts);
    
    if (resp->error != PXSHOT_OK) {
        fprintf(stderr, "Error: %s", pxshot_error_string(resp->error));
        if (resp->error_message) {
            fprintf(stderr, " - %s", resp->error_message);
        }
        if (resp->http_status) {
            fprintf(stderr, " (HTTP %d)", resp->http_status);
        }
        fprintf(stderr, "\n");
        pxshot_response_free(resp);
        pxshot_free(client);
        return 1;
    }
    
    /* Save to file */
    FILE *fp = fopen(output, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Failed to open output file: %s\n", output);
        pxshot_response_free(resp);
        pxshot_free(client);
        return 1;
    }
    
    size_t written = fwrite(resp->data, 1, resp->data_len, fp);
    fclose(fp);
    
    if (written != resp->data_len) {
        fprintf(stderr, "Error: Failed to write all data\n");
        pxshot_response_free(resp);
        pxshot_free(client);
        return 1;
    }
    
    printf("Screenshot saved to: %s (%zu bytes)\n", output, resp->data_len);
    
    /* Cleanup */
    pxshot_response_free(resp);
    pxshot_free(client);
    
    return 0;
}
