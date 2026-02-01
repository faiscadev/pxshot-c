/**
 * @file usage.c
 * @brief Get API usage statistics example
 */

#include <pxshot.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *api_key = getenv("PXSHOT_API_KEY");
    if (!api_key) {
        fprintf(stderr, "Error: PXSHOT_API_KEY environment variable not set\n");
        return 1;
    }
    
    printf("Pxshot C SDK v%s\n\n", pxshot_version());
    
    /* Create client */
    pxshot_client_t *client = pxshot_new(api_key);
    if (!client) {
        fprintf(stderr, "Error: Failed to create client\n");
        return 1;
    }
    
    /* Get usage statistics */
    pxshot_usage_t *usage = NULL;
    pxshot_response_t *resp = pxshot_get_usage(client, &usage);
    
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
    
    /* Display usage */
    printf("API Usage Statistics\n");
    printf("====================\n");
    printf("Screenshots: %d / %d\n", usage->screenshots_used, usage->screenshots_limit);
    printf("Storage: %d / %d bytes\n", usage->storage_used_bytes, usage->storage_limit_bytes);
    if (usage->period_start) {
        printf("Period: %s to %s\n", usage->period_start, usage->period_end);
    }
    
    /* Cleanup */
    pxshot_usage_free(usage);
    pxshot_response_free(resp);
    pxshot_free(client);
    
    return 0;
}
