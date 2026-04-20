#pragma once

#include "ui/device_menu.h"

void moisture_runtime_init(void);
void moisture_runtime_start(void);
void moisture_runtime_refresh_measurement(void);
void moisture_runtime_start_live_reading(void);
void moisture_runtime_stop_live_reading(void);
void moisture_runtime_start_calibration(void);
void moisture_runtime_get_status(device_menu_status_t *status);
