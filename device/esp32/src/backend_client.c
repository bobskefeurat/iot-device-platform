#include "backend_client.h"
#include "local_config.h"

#include <stdio.h>
#include <string.h>

#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "backend-client";

static bool build_register_device_payload(
    char *payload,
    size_t payload_size,
    const char *id,
    const char *name,
    const component_t *device_components,
    size_t component_count
) {
    char components_json[512];
    size_t offset = 0;
    size_t remaining = sizeof(components_json) - offset;
    int written = snprintf(
        components_json + offset,
        remaining,
        "["
    );

    if (written < 0 || (size_t)written >= remaining) {
        return false;
    }

    offset += (size_t)written;

    for (size_t i = 0; i < component_count; i++) {
        if (i > 0) {
            remaining = sizeof(components_json) - offset;
            written = snprintf(
                components_json + offset,
                remaining,
                ","
            );

            if (written < 0 || (size_t)written >= remaining) {
                return false;
            }

            offset += (size_t)written;
        }

        remaining = sizeof(components_json) - offset;
        written = snprintf(
            components_json + offset,
            remaining,
            "{\"component_local_id\":\"%s\",\"model_name\":\"%s\",\"component_type\":\"%s\"}",
            device_components[i].component_local_id,
            device_components[i].model_name,
            device_components[i].component_type
        );

        if (written < 0 || (size_t)written >= remaining) {
            return false;
        }

        offset += (size_t)written;
    }

    remaining = sizeof(components_json) - offset;
    written = snprintf(
        components_json + offset,
        remaining,
        "]"
    );

    if (written < 0 || (size_t)written >= remaining) {
        return false;
    }

    written = snprintf(
        payload,
        payload_size,
        "{\"id\":\"%s\",\"name\":\"%s\",\"device_components\":%s}",
        id,
        name,
        components_json
    );

    if (written < 0 || (size_t)written >= payload_size) {
        return false;
    }

    return true;
}

static bool build_measurement_payload(
    char *payload,
    size_t payload_size,
    const char *component_local_id,
    int mean_adc,
    int moisture_percent
) {

    int written = snprintf(
        payload,
        payload_size,
        "{\"component_local_id\":\"%s\",\"mean_adc\":%d,\"moisture_percent\":%d}",
        component_local_id,
        mean_adc,
        moisture_percent
    );

    if (written < 0 || (size_t)written >= payload_size ) {
        return false;
    }
    return true;
}


bool register_device(
    const char *id, 
    const char *name, 
    const component_t *device_components,
    size_t component_count) {

    char payload[768];

    if (!build_register_device_payload(payload, sizeof(payload), id, name, device_components, component_count)) {
        ESP_LOGE(TAG, "Failed to build device registration payload");
        return false;
    }

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

bool send_measurement(
    const char *id,
    const char *component_local_id,
    int mean_adc,
    int moisture_percent
) {

    char url[160];

    snprintf(url, sizeof(url), "%s/%s/measurements", BACKEND_URL, id);

    char payload[160];

    if (!build_measurement_payload(payload, sizeof(payload), component_local_id, mean_adc, moisture_percent)) {
        ESP_LOGE(TAG, "Failed to build measurement payload");
        return false;
    }

    esp_http_client_config_t config = {
        .url = url,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, strlen(payload));

    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);

    if (err == ESP_OK && status_code == 204) {
        ESP_LOGI(TAG, "Measurement sent, status=%d", status_code);
        esp_http_client_cleanup(client);
        return true;
    }

    ESP_LOGE(TAG, "Measurement not sent, err=%s status=%d", esp_err_to_name(err), status_code);
    esp_http_client_cleanup(client);
    return false;
}

