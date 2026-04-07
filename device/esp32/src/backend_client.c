#include "backend_client.h"
#include "local_config.h"

#include <stdio.h>
#include <string.h>

#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "backend-client";

bool register_device(const char *id, const char *name) {

    char payload[128];

    snprintf(payload, sizeof(payload), "{\"id\":\"%s\",\"name\":\"%s\"}", id, name);

    esp_http_client_config_t config = {
        .url = BACKEND_URL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, strlen(payload));

    esp_err_t err = esp_http_client_perform(client);

    int status_code = esp_http_client_get_status_code(client);

    if (err == ESP_OK && status_code >= 200 && status_code < 300) {
        ESP_LOGI(TAG, "Device registration succeeded, status=%d", status_code);
        esp_http_client_cleanup(client);
        return true;
    }

    ESP_LOGE(TAG, "Device registration failed, err=%s status=%d", esp_err_to_name(err), status_code);
    esp_http_client_cleanup(client);

    return false;
}

bool send_heartbeat(const char *id) {
    char url[160];

    snprintf(url, sizeof(url), "%s/%s/heartbeat", BACKEND_URL, id);

    esp_http_client_config_t config = {
        .url = url,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);

    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);

    if (err == ESP_OK && status_code == 204) {
        ESP_LOGI(TAG, "Heartbeat succeeded, status=%d", status_code);
        esp_http_client_cleanup(client);
        return true;
    }

    ESP_LOGE(TAG, "Heartbeat failed, err=%s status=%d", esp_err_to_name(err), status_code);
    esp_http_client_cleanup(client);
    return false;
}

