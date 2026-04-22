#include "app/runtime/wifi_wait.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "network/wifi_manager.h"

static const char *TAG = "PlantStation";

void wait_for_wifi_connection(void) {
    while (!wifi_manager_is_connected()) {
        ESP_LOGI(TAG, "Waiting for WiFi connection...");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
