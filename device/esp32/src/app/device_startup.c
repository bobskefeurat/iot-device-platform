#include "app/device_startup.h"

#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "app/device_runtime.h"
#include "device/device_components.h"
#include "device/device_identity.h"
#include "device/device_setup.h"
#include "network/backend_client.h"
#include "network/wifi_manager.h"

#define HEARTBEAT_INTERVAL_MS 30000

static const char *TAG = "PlantStation";

static void heartbeat_task(void *arg) {
    (void)arg;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_INTERVAL_MS));

        if (!wifi_manager_is_connected()) {
            continue;
        }

        send_heartbeat(get_device_id());
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
