#include "features/moisture/moisture_calibration_flow.h"

#include <inttypes.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/moisture_sensor.h"
#include "features/moisture/moisture_calibration_store.h"

static const char *TAG = "Moisture Calibration";

static uint16_t measure_calibration_point(const char *phase_message,
                                          const char *prompt_message,
                                          adc_channel_t channel,
                                          int sample_count,
                                          int delay_ms) {
    ESP_LOGI(TAG, "%s", phase_message);
    ESP_LOGI(TAG, "%s", prompt_message);

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "3...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "2...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "1...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    return measure_average_adc_value(channel, sample_count, delay_ms);
}

void moisture_calibration_run(adc_channel_t channel, int sample_count, int delay_ms) {
    uint16_t dry_value = measure_calibration_point(
        "Starting dry calibration...",
        "Please ensure the sensor is in dry soil.",
        channel,
        sample_count,
        delay_ms
    );
    ESP_LOGI(TAG, "Calibration complete. Dry value: %" PRIu16, dry_value);

    uint16_t wet_value = measure_calibration_point(
        "Starting wet calibration...",
        "Please ensure the sensor is in wet soil.",
        channel,
        sample_count,
        delay_ms
    );
    ESP_LOGI(TAG, "Calibration complete. Wet value: %" PRIu16, wet_value);

    if (!moisture_calibration_save(dry_value, wet_value)) {
        ESP_LOGE(TAG, "Failed to persist calibration values");
        return;
    }

    ESP_LOGI(TAG, "Calibration saved to NVS: dry=%" PRIu16 " wet=%" PRIu16, dry_value, wet_value);
}
