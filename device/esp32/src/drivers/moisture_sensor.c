#include "drivers/moisture_sensor.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

static adc_oneshot_unit_handle_t adc_handle;
static const char *TAG = "Moisture Sensor";

void moisture_sensor_init(adc_channel_t channel) {

    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };

    adc_oneshot_new_unit(&init_cfg, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12
    };

    adc_oneshot_config_channel(adc_handle, channel, &chan_cfg);
}

uint16_t measure_average_adc_value(adc_channel_t channel, int sample_count, int delay_ms) {

    uint16_t total = 0;
    int raw = 0;

    for (int i = 0; i < sample_count; i++) {
        adc_oneshot_read(adc_handle, channel, &raw);
        total += raw;

        printf("%d,%d\n", i, raw);

        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }

    uint16_t mean_moisture_adc = total / sample_count;
    ESP_LOGI(TAG, "Mean Moisture ADC: %d", mean_moisture_adc);
    return mean_moisture_adc;
}
