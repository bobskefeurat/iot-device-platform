#include "features/moisture/moisture_calibration_store.h"

#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"

static const char *TAG = "Moisture Calibration";
static const char *CALIBRATION_NAMESPACE = "calib_ns";
static const char *DRY_ADC_KEY = "dry_adc";
static const char *WET_ADC_KEY = "wet_adc";

bool moisture_calibration_load(uint16_t *dry_adc, uint16_t *wet_adc) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(CALIBRATION_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No calibration namespace in NVS yet");
        return false;
    }

    err = nvs_get_u16(nvs_handle, DRY_ADC_KEY, dry_adc);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Missing dry calibration value");
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_get_u16(nvs_handle, WET_ADC_KEY, wet_adc);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Missing wet calibration value");
        nvs_close(nvs_handle);
        return false;
    }

    nvs_close(nvs_handle);
    return true;
}

bool moisture_calibration_save(uint16_t dry_adc, uint16_t wet_adc) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(CALIBRATION_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return false;
    }

    err = nvs_set_u16(nvs_handle, DRY_ADC_KEY, dry_adc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save dry calibration value: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_set_u16(nvs_handle, WET_ADC_KEY, wet_adc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save wet calibration value: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit calibration values: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }

    nvs_close(nvs_handle);
    return true;
}

bool moisture_calibration_clear(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(CALIBRATION_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No calibration namespace to clear");
        return false;
    }

    err = nvs_erase_key(nvs_handle, DRY_ADC_KEY);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Failed to erase dry calibration value: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_erase_key(nvs_handle, WET_ADC_KEY);
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
