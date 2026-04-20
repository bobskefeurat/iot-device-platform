#pragma once

#include "esp_adc/adc_oneshot.h"

void moisture_calibration_run(adc_channel_t channel, int sample_count, int delay_ms);
