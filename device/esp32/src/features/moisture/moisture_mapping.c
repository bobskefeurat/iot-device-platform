#include "features/moisture/moisture_mapping.h"

#include <inttypes.h>

#include "esp_log.h"

#include "features/moisture/moisture_calibration_store.h"

static const char *TAG = "Moisture Calibration";

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

    ESP_LOGI(TAG, "Calibration mapping: dry=%" PRIu16 " wet=%" PRIu16 " adc=%" PRIu16 " moisture=%" PRId32 "%%",
             dry_adc, wet_adc, mean_moisture_adc, percent);

    return (uint8_t)percent;
}
