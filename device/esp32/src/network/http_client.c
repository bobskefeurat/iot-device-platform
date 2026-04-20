#include "network/http_client.h"

#include <string.h>

#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "http-client";

static bool perform_post_request(const char *url,
                                 const char *payload,
                                 bool has_json_payload,
                                 int min_status_code,
                                 int max_status_code) {
    esp_http_client_config_t config = {
        .url = url,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);

    if (has_json_payload) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, payload, strlen(payload));
    }

    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    bool success = err == ESP_OK && status_code >= min_status_code && status_code <= max_status_code;

    if (!success) {
        ESP_LOGE(TAG, "POST failed, url=%s err=%s status=%d", url, esp_err_to_name(err), status_code);
    }

    esp_http_client_cleanup(client);
    return success;
}

bool http_client_post_json(const char *url, const char *payload, int expected_status_code) {
    return perform_post_request(url, payload, true, expected_status_code, expected_status_code);
}

bool http_client_post_json_with_status_range(
    const char *url,
    const char *payload,
    int min_status_code,
    int max_status_code
) {
    return perform_post_request(url, payload, true, min_status_code, max_status_code);
}

bool http_client_post(const char *url, int expected_status_code) {
    return perform_post_request(url, NULL, false, expected_status_code, expected_status_code);
}

bool http_client_post_with_status_range(const char *url, int min_status_code, int max_status_code) {
    return perform_post_request(url, NULL, false, min_status_code, max_status_code);
}
