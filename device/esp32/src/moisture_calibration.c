#include "moisture_calibration.h"

#include "moisture_sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "Moisture Calibration";

bool moisture_calibration_load(uint16_t *dry_adc, uint16_t *wet_adc) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("calib_ns", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No calibration namespace in NVS yet");
        return false;
    }

    err = nvs_get_u16(nvs_handle, "dry_adc", dry_adc);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Missing dry calibration value");
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_get_u16(nvs_handle, "wet_adc", wet_adc);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Missing wet calibration value");
        nvs_close(nvs_handle);
        return false;
    }

    nvs_close(nvs_handle);
    return true;
}

bool moisture_calibration_clear(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("calib_ns", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No calibration namespace to clear");
        return false;
    }

    err = nvs_erase_key(nvs_handle, "dry_adc");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Failed to erase dry calibration value: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_erase_key(nvs_handle, "wet_adc");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Failed to erase wet calibration value: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit calibration erase: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Calibration values cleared from NVS");
    return true;
}

uint8_t moisture_calibration_get_moisture_percent(uint16_t mean_moisture_adc) {
    uint16_t dry_adc = 0;
    uint16_t wet_adc = 0;

    if (!moisture_calibration_load(&dry_adc, &wet_adc)) {
        ESP_LOGW(TAG, "Using fallback full-scale mapping because calibration is unavailable");
        return (uint8_t)((mean_moisture_adc * 100U) / 4095U);
    }

    if (dry_adc == wet_adc) {
        ESP_LOGW(TAG, "Invalid calibration: dry and wet ADC values are identical");
        return 0;
    }

    int32_t percent = 0;

    if (dry_adc > wet_adc) {
        percent = ((int32_t)(dry_adc - mean_moisture_adc) * 100) / (dry_adc - wet_adc);
    } else {
        percent = ((int32_t)(mean_moisture_adc - dry_adc) * 100) / (wet_adc - dry_adc);
    }

    if (percent < 0) {
        percent = 0;
    } else if (percent > 100) {
        percent = 100;
    }

    ESP_LOGI(TAG, "Calibration mapping: dry=%u wet=%u adc=%u moisture=%ld%%",
             dry_adc, wet_adc, mean_moisture_adc, percent);

    return (uint8_t)percent;
}

void moisture_calibration_run(adc_channel_t channel, int sample_count, int delay_ms) {
    ESP_LOGI(TAG, "Starting dry calibration...");
    ESP_LOGI(TAG, "Please ensure the sensor is in dry soil.");

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "3...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "2...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "1...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    uint16_t dry_value = measure_average_adc_value(channel, sample_count, delay_ms);
    ESP_LOGI(TAG, "Calibration complete. Dry value: %d", dry_value);

    ESP_LOGI(TAG, "Starting wet calibration...");
    ESP_LOGI(TAG, "Please ensure the sensor is in wet soil.");

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "3...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "2...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "1...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    uint16_t wet_value = measure_average_adc_value(channel, sample_count, delay_ms);
    ESP_LOGI(TAG, "Calibration complete. Wet value: %d", wet_value);

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("calib_ns", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_set_u16(nvs_handle, "dry_adc", dry_value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save dry calibration value");
    }

    err = nvs_set_u16(nvs_handle, "wet_adc", wet_value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save wet calibration value");
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit calibration values");
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Calibration saved to NVS: dry=%u wet=%u", dry_value, wet_value);
}
