#include "app/system_init.h"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "app/device_runtime.h"
#include "network/wifi_manager.h"

void system_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("HTTP_CLIENT", ESP_LOG_WARN);
    esp_log_level_set("esp-tls", ESP_LOG_WARN);

    device_runtime_init();
    wifi_manager_init();
}
