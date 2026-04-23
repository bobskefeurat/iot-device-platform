
#include "app/runtime/config_task.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "device/device_config.h"
#include "device/device_identity.h"
#include "network/backend/backend_client.h"
#include "network/backend/backend_messages.h"

typedef struct {
    char desired_config_id[64];
} config_sync_task_args_t;

static const char *TAG = "PlantStation";

static TaskHandle_t config_task_handle = NULL;

static void config_task(void *arg) {
    config_sync_task_args_t *config_sync_args = (config_sync_task_args_t *)arg;

    char response_buffer[128];
    char heartbeat_interval_buffer[16];
    char desired_config_id_buffer[64];
    char detail_buffer[32];

    bool sync_request_succeeded = sync_device_config(
        get_device_id(),
        get_applied_config_id(),
        response_buffer,
        sizeof(response_buffer)
    );

    if (!sync_request_succeeded) {
        ESP_LOGE(TAG, "Config sync request failed");
        goto cleanup;
    }

    bool config_synced = extract_backend_field_value(
        response_buffer,
        "message",
        heartbeat_interval_buffer,
        sizeof(heartbeat_interval_buffer)
    );

    if (config_synced && strcmp(heartbeat_interval_buffer, "config synced") == 0) {
        ESP_LOGI(TAG, "Config synced");
        goto cleanup;
    }

    bool heartbeat_interval_extracted = extract_backend_field_value(
        response_buffer,
        "heartbeat_interval",
        heartbeat_interval_buffer,
        sizeof(heartbeat_interval_buffer)
    );

    bool desired_config_id_extracted = extract_backend_field_value(
        response_buffer,
        "desired_config_id",
        desired_config_id_buffer,
        sizeof(desired_config_id_buffer)
    );

    if (!heartbeat_interval_extracted || !desired_config_id_extracted) {
        bool detail_extracted = extract_backend_field_value(
            response_buffer,
            "detail",
            detail_buffer,
            sizeof(detail_buffer)
        );

        if (detail_extracted) {
            ESP_LOGW(TAG, "Config sync aborted: %s", detail_buffer);
        } else {
            ESP_LOGE(TAG, "Config response missing expected fields");
        }
        goto cleanup;
    }

    int heartbeat_interval = atoi(heartbeat_interval_buffer);
    set_heartbeat_interval(heartbeat_interval);
    set_applied_config_id(desired_config_id_buffer);

    sync_request_succeeded = sync_device_config(
        get_device_id(),
        get_applied_config_id(),
        response_buffer,
        sizeof(response_buffer)
    );

    if (!sync_request_succeeded) {
        ESP_LOGE(TAG, "Config confirmation request failed");
        goto cleanup;
    }

    config_synced = extract_backend_field_value(
        response_buffer,
        "message",
        heartbeat_interval_buffer,
        sizeof(heartbeat_interval_buffer)
    );

    if (!config_synced || strcmp(heartbeat_interval_buffer, "config synced") != 0) {
        ESP_LOGE(TAG, "Config confirmation response was not synced");
        goto cleanup;
    }

    if (config_sync_args != NULL) {
        if (strcmp(config_sync_args->desired_config_id, desired_config_id_buffer) != 0) {
            ESP_LOGW(
                TAG,
                "Applied config id differs from requested desired id: requested=%s received=%s",
                config_sync_args->desired_config_id,
                desired_config_id_buffer
            );
        }
    }

    ESP_LOGI(TAG, "Applied new config and confirmed sync");

cleanup:
    free(config_sync_args);
    config_task_handle = NULL;
    vTaskDelete(NULL);

}

bool config_task_start(const char *desired_config_id) {
    if (desired_config_id == NULL) {
        return false;
    }

    if (config_task_handle != NULL) {
        return false;
    }

    config_sync_task_args_t *config_sync_args = malloc(sizeof(config_sync_task_args_t));
    if (config_sync_args == NULL) {
        return false;
    }

    memset(config_sync_args, 0, sizeof(*config_sync_args));
    snprintf(
        config_sync_args->desired_config_id,
        sizeof(config_sync_args->desired_config_id),
        "%s",
        desired_config_id
    );


    BaseType_t task_created = xTaskCreate(
        config_task,
        "config_task",
        4096,
        config_sync_args,
        5,
        &config_task_handle
    );

    if (task_created != pdPASS) {
        free(config_sync_args);
        config_task_handle = NULL;
        return false;
    }

    return true;
}
