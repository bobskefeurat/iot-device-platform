#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_mac.h"

#include "device_menu.h"
#include "device_info.h"
#include "moisture_calibration.h"
#include "moisture_sensor.h"
#include "serial_console.h"
#include "wifi_manager.h"
#include "backend_client.h"

#define MOISTURE_SENSOR_PIN ADC_CHANNEL_7

#define HEARTBEAT_INTERVAL_MS 30000

static const char *TAG = "PlantStation";
static volatile bool calibration_in_progress = false;
static volatile bool live_reading_active = false;
static uint16_t latest_mean_adc = 0;
static uint8_t latest_moisture_percent = 0;

static void update_measurement(void) {
    uint16_t mean_adc = measure_average_adc_value(MOISTURE_SENSOR_PIN, 10, 20);
    uint8_t moisture_percent = moisture_calibration_get_moisture_percent(mean_adc);

    latest_mean_adc = mean_adc;
    latest_moisture_percent = moisture_percent;
}

static void get_menu_status(device_menu_status_t *status) {
    if (status == NULL) {
        return;
    }

    status->calibration_in_progress = calibration_in_progress;
    status->latest_mean_adc = latest_mean_adc;
    status->latest_moisture_percent = latest_moisture_percent;
}

static void start_live_reading(void) {
    live_reading_active = true;
}

static void stop_live_reading(void) {
    live_reading_active = false;
}

static void moisture_sensor_reading_task(void *arg) {
    (void)arg;

    const component_t *components = get_device_components();
    component_t moisture_sensor = components[0];
    const char *moisture_sensor_local_id = moisture_sensor.component_local_id;
    
    while (1) {
        if (!live_reading_active || calibration_in_progress) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        update_measurement();
        ESP_LOGI(TAG, "Live reading: adc=%u moisture=%u%%", latest_mean_adc, latest_moisture_percent);

        if (!wifi_manager_is_connected()) {
            continue;
        }
        send_measurement(get_device_id(), moisture_sensor_local_id, latest_mean_adc, latest_moisture_percent);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

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


static void calibration_task(void *arg) {
    (void)arg;

    moisture_calibration_run(MOISTURE_SENSOR_PIN, 200, 20);
    calibration_in_progress = false;
    ESP_LOGI(TAG, "Calibration flow finished");
    device_menu_print_main();
    vTaskDelete(NULL);
}

static void start_calibration(void) {
    if (calibration_in_progress) {
        ESP_LOGW(TAG, "Calibration is already running");
        return;
    }

    calibration_in_progress = true;
    BaseType_t task_created = xTaskCreate(calibration_task, "calibration_task", 4096, NULL, 5, NULL);
    if (task_created != pdPASS) {
        calibration_in_progress = false;
        ESP_LOGE(TAG, "Failed to start calibration task");
        return;
    }

    ESP_LOGI(TAG, "Calibration started. Follow the prompts in the serial monitor.");
}

static void serial_command_task(void *arg) {
    (void)arg;
    char command_buffer[64];
    device_menu_actions_t menu_actions = {
        .start_live_reading = start_live_reading,
        .stop_live_reading = stop_live_reading,
        .refresh_measurement = update_measurement,
        .start_calibration = start_calibration,
        .clear_calibration = moisture_calibration_clear,
        .load_calibration = moisture_calibration_load,
        .get_status = get_menu_status,
        .show_wifi_status = wifi_manager_print_status,
    };

    ESP_LOGI(TAG, "Serial menu ready");
    device_menu_print_main();

    while (1) {
        if (!serial_console_read_command(command_buffer, sizeof(command_buffer), pdMS_TO_TICKS(100))) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        device_menu_status_t menu_status;
        get_menu_status(&menu_status);
        device_menu_handle_command(command_buffer, &menu_status, &menu_actions);
    }
}

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

    moisture_sensor_init(MOISTURE_SENSOR_PIN);
    device_menu_init();
    serial_console_init();
    wifi_manager_init();
}

void app_main(void) {
    
    system_init();
    setup_device_info();

    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    while(!wifi_manager_is_connected()){
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    bool registered = register_device(
        get_device_id(),
        get_device_name(),
        get_device_components(),
        get_components_count()
    );

    if (registered) {
        ESP_LOGI(TAG, "Device registration succeeded");
    } else {
        ESP_LOGE(TAG, "Device registration failed");
    }

    xTaskCreate(serial_command_task, "serial_command_task", 4096, NULL, 5, NULL);
    xTaskCreate(moisture_sensor_reading_task, "moisture_sensor_reading_task", 4096, NULL, 5, NULL);

    if (registered) {
        xTaskCreate(heartbeat_task, "heartbeat_task", 4096, NULL, 5, NULL);
    } else {
        ESP_LOGE(TAG, "Device not registered");
    }

    ESP_LOGI(TAG, "PlantStation started");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
