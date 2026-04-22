#include "network/http_client.h"

#include <string.h>

#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "http-client";

//--------------------------------RESPONSE-HANDLING--------------------------------

static bool read_response_body(
    esp_http_client_handle_t client,
    char *response_buffer,
    size_t buffer_size
) {
    if (response_buffer == NULL || buffer_size == 0) {
        return true;
    }

    int bytes_read = esp_http_client_read_response(client, response_buffer, (buffer_size - 1));

    if (bytes_read > 0) {
        response_buffer[bytes_read] = '\0';
    }

    return (bytes_read >= 0) && esp_http_client_is_complete_data_received(client);
}

//--------------------------------POST-EXECUTION--------------------------------

//Core HTTP POST function. Lets the public wrapper API stay small and specific.

static bool perform_post_request(
    const char *url,
    const char *payload,
    bool has_json_payload,
    char *response_buffer,
    size_t buffer_size,
    int min_status_code,
    int max_status_code
) {

    esp_http_client_config_t config = {
        .url = url,
    };

    if ((response_buffer != NULL) && (buffer_size > 0)) {
        response_buffer[0] = '\0';
    }

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);

    if (has_json_payload) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, payload, strlen(payload));
    }

    esp_err_t err = esp_http_client_perform(client);

    int status_code = esp_http_client_get_status_code(client);
    bool response_complete = read_response_body(client, response_buffer, buffer_size);

    bool success = (
        (err == ESP_OK) && 
        (status_code >= min_status_code) && 
        (status_code <= max_status_code) &&
        response_complete
    );

    if (!success) {
        ESP_LOGE(TAG, "POST failed, url=%s err=%s status=%d", url, esp_err_to_name(err), status_code);
    }

    esp_http_client_cleanup(client);
    return success;
}

//--------------------------------PUBLIC-WRAPPERS--------------------------------

bool http_client_post_json(
    const char *url, 
    const char *payload, 
    int expected_status_code
) {
    return perform_post_request(url, payload, true, NULL, 0, expected_status_code, expected_status_code);
}

bool http_client_post_json_with_status_range(
    const char *url,
    const char *payload,
    int min_status_code,
    int max_status_code
) {
    return perform_post_request(url, payload, true, NULL, 0, min_status_code, max_status_code);
}

bool http_client_post(
    const char *url, 
    int expected_status_code
) {
    return perform_post_request(url, NULL, false, NULL, 0, expected_status_code, expected_status_code);
}

bool http_client_post_with_status_range(
    const char *url, 
    int min_status_code, 
    int max_status_code
) {
    return perform_post_request(url, NULL, false, NULL, 0, min_status_code, max_status_code);
}

bool http_client_post_with_response(
    const char *url, 
    char *response_buffer, 
    size_t buffer_size, 
    int expected_status_code
) {
    return perform_post_request(url, NULL, false, response_buffer, buffer_size, expected_status_code, expected_status_code);
}
bool http_client_post_with_response_and_status_range(
    const char *url, 
    char *response_buffer, 
    size_t buffer_size, 
    int min_status_code, 
    int max_status_code
) {
    return perform_post_request(url, NULL, false, response_buffer, buffer_size, min_status_code, max_status_code);
}
