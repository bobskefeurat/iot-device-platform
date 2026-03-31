#pragma once

#include <stdint.h>
#include "esp_adc/adc_oneshot.h"


void moisture_sensor_init(adc_channel_t channel);

uint16_t measure_average_adc_value(adc_channel_t channel, int sample_count, int delay_ms);
