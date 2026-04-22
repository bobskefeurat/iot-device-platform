#include "app/device_startup.h"

#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "app/device_runtime.h"
#include "device/device_components.h"
#include "device/device_identity.h"
#include "device/device_setup.h"
#include "device/device_config.h"
#include "network/backend/backend_client.h"
#include "network/backend/backend_messages.h"
#include "network/wifi_manager.h"

#define WIFI_RETRY_DELAY_MS 500

static const char *TAG = "PlantStation";

static void heartbeat_task(void *arg) {
    (void)arg;

    size_t response_buffer_size = 96;
    char response_buffer[response_buffer_size];

    size_t config_id_buffer_size = 64;
    char config_id_buffer[config_id_buffer_size];

    while (1) {
        
        while (!wifi_manager_is_connected()) {
            vTaskDelay(pdMS_TO_TICKS(WIFI_RETRY_DELAY_MS));
        }

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

static void wait_for_wifi_connection(void) {
    ESP_LOGI(TAG, "Waiting for WiFi connection...");

    while (!wifi_manager_is_connected()) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static bool register_current_device(void) {
    return register_device(
        get_device_id(),
        get_device_name(),
        get_device_components(),
        get_components_count()
    );
}

void device_startup_run(void) {
    setup_device_info();
    wait_for_wifi_connection();

    bool registered = register_current_device();

    if (registered) {
        ESP_LOGI(TAG, "Device registration succeeded");
    } else {
        ESP_LOGE(TAG, "Device registration failed");
    }

    device_runtime_start();

    if (registered) {
        xTaskCreate(heartbeat_task, "heartbeat_task", 4096, NULL, 5, NULL);
    } else {
        ESP_LOGE(TAG, "Device not registered");
    }

    ESP_LOGI(TAG, "PlantStation started");
}
