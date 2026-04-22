#include "app/startup.h"

#include <stdbool.h>
#include "esp_log.h"

#include "app/runtime.h"
#include "app/runtime/wifi_wait.h"
#include "device/device_components.h"
#include "device/device_identity.h"
#include "device/device_setup.h"
#include "network/backend/backend_client.h"

static const char *TAG = "PlantStation";

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
    device_runtime_init();
    wait_for_wifi_connection();

    bool registered = register_current_device();

    if (registered) {
        ESP_LOGI(TAG, "Device registration succeeded");
    } else {
        ESP_LOGE(TAG, "Device registration failed");
    }

    device_runtime_start();

    if (registered) {
        if (!heartbeat_task_start()) {
            ESP_LOGE(TAG, "Heartbeat task failed to start");
        }
    } else {
        ESP_LOGE(TAG, "Device not registered");
    }

    ESP_LOGI(TAG, "PlantStation started");
}
