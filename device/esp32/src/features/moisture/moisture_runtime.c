#include "features/moisture/moisture_runtime.h"

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "device/device_components.h"
#include "device/device_identity.h"
#include "drivers/moisture_sensor.h"
#include "features/moisture/moisture_calibration_flow.h"
#include "features/moisture/moisture_mapping.h"
#include "network/backend/backend_client.h"
#include "network/wifi_manager.h"

#define MOISTURE_SENSOR_PIN ADC_CHANNEL_7

static const char *TAG = "PlantStation";
static volatile bool calibration_in_progress = false;
static volatile bool live_reading_active = false;
static uint16_t latest_mean_adc = 0;
static uint8_t latest_moisture_percent = 0;

void moisture_runtime_refresh_measurement(void) {
    uint16_t mean_adc = measure_average_adc_value(MOISTURE_SENSOR_PIN, 10, 20);
    uint8_t moisture_percent = moisture_calibration_get_moisture_percent(mean_adc);

    latest_mean_adc = mean_adc;
    latest_moisture_percent = moisture_percent;
}

void moisture_runtime_get_status(device_menu_status_t *status) {
    if (status == NULL) {
        return;
    }

    status->calibration_in_progress = calibration_in_progress;
    status->latest_mean_adc = latest_mean_adc;
    status->latest_moisture_percent = latest_moisture_percent;
}

void moisture_runtime_start_live_reading(void) {
    live_reading_active = true;
}

void moisture_runtime_stop_live_reading(void) {
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

        moisture_runtime_refresh_measurement();
        ESP_LOGI(TAG, "Live reading: adc=%u moisture=%u%%", latest_mean_adc, latest_moisture_percent);

        if (!wifi_manager_is_connected()) {
            continue;
        }

        send_measurement(get_device_id(), moisture_sensor_local_id, latest_mean_adc, latest_moisture_percent);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void calibration_task(void *arg) {
    (void)arg;

    moisture_calibration_run(MOISTURE_SENSOR_PIN, 200, 20);
    calibration_in_progress = false;
    ESP_LOGI(TAG, "Calibration flow finished");
    vTaskDelete(NULL);
}

void moisture_runtime_start_calibration(void) {
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

void moisture_runtime_init(void) {
    moisture_sensor_init(MOISTURE_SENSOR_PIN);
}

void moisture_runtime_start(void) {
    xTaskCreate(moisture_sensor_reading_task, "moisture_sensor_reading_task", 4096, NULL, 5, NULL);
}
