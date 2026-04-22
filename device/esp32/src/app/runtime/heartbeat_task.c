#include "app/runtime/heartbeat_task.h"

#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "app/runtime/wifi_wait.h"
#include "device/device_config.h"
#include "device/device_identity.h"
#include "network/backend/backend_client.h"
#include "network/backend/backend_messages.h"

static const char *TAG = "PlantStation";

static void heartbeat_task(void *arg) {
    (void)arg;

    size_t response_buffer_size = 96;
    char response_buffer[response_buffer_size];

    size_t config_id_buffer_size = 64;
    char config_id_buffer[config_id_buffer_size];

    while (1) {
        wait_for_wifi_connection();

        bool heartbeat_sent = send_heartbeat(get_device_id(), response_buffer, response_buffer_size);
        bool value_extracted = false;

        if (heartbeat_sent) {
            value_extracted = extract_backend_field_value(
                response_buffer,
                "desired_config_id",
                config_id_buffer,
                config_id_buffer_size
            );
        }

        if (heartbeat_sent && value_extracted) {
            int comparison = strcmp(get_applied_config_id(), config_id_buffer);
            if (comparison != 0) {
                //TODO start config task
            }
        }

        vTaskDelay(pdMS_TO_TICKS(get_heartbeat_interval()));
    }
}

bool heartbeat_task_start(void) {
    BaseType_t task_created = xTaskCreate(heartbeat_task, "heartbeat_task", 4096, NULL, 5, NULL);

    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Failed to start heartbeat task");
        return false;
    }

    return true;
}
