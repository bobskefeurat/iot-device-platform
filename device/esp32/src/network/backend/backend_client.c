#include "network/backend/backend_client.h"
#include "config/local_config.h"

#include <stdio.h>

#include "esp_log.h"

#include "network/backend/backend_messages.h"
#include "network/http_client.h"

static const char *TAG = "backend-client";

//--------------------------------BACKEND-OPERATIONS--------------------------------
// This layer defines what the device sends to the backend, while the HTTP layer handles how requests are sent.

bool register_device(
    const char *id, 
    const char *name, 
    const component_t *device_components,
    size_t component_count
) {

    char payload[768];

    if (!build_register_device_payload(payload, sizeof(payload), id, name, device_components, component_count)) {
        ESP_LOGE(TAG, "Failed to build device registration payload");
        return false;
    }

    return http_client_post_json_with_status_range(BACKEND_URL, payload, 200, 299);
}

bool send_heartbeat(const char *id, char *response_buffer, size_t buffer_size) {
    char url[160];
    snprintf(url, sizeof(url), "%s/%s/heartbeat", BACKEND_URL, id);

    return http_client_post_with_response(url, response_buffer, buffer_size, 200);
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

    return http_client_post_json(url, payload, 204);
}

bool sync_device_config(
    const char *id,
    const char *applied_config_id,
    char *response_buffer,
    size_t buffer_size
) {
    char url[160];
    snprintf(url, sizeof(url), "%s/%s/config/sync", BACKEND_URL, id);

    char payload[128];
    if (!build_config_sync_payload(payload, sizeof(payload), applied_config_id)) {
        ESP_LOGE(TAG, "Failed to build config sync payload");
        return false;
    }

    return http_client_post_json_with_response_and_status_range(
        url,
        payload,
        response_buffer,
        buffer_size,
        200,
        404
    );
}
