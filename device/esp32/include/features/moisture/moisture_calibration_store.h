#pragma once

#include <stdbool.h>
#include <stdint.h>

bool moisture_calibration_load(uint16_t *dry_adc, uint16_t *wet_adc);
bool moisture_calibration_save(uint16_t dry_adc, uint16_t wet_adc);
bool moisture_calibration_clear(void);
