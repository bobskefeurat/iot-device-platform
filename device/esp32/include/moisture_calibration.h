#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_adc/adc_oneshot.h"

bool moisture_calibration_load(uint16_t *dry_adc, uint16_t *wet_adc);
bool moisture_calibration_clear(void);
uint8_t moisture_calibration_get_moisture_percent(uint16_t mean_moisture_adc);
void moisture_calibration_run(adc_channel_t channel, int sample_count, int delay_ms);
