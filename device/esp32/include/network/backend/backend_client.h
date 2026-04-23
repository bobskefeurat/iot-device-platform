#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "device/device_components.h"

bool register_device(
    const char *id, 
    const char *name,
    const component_t *device_components,
    size_t component_count
);

bool send_heartbeat(
    const char *id, 
    char *response_buffer, 
    size_t buffer_size
);

bool send_measurement(
    const char *id,
    const char *component_local_id,
    int mean_adc,
    int moisture_percent
);

bool sync_device_config(
    const char *id,
    const char *applied_config_id,
    char *response_buffer,
    size_t buffer_size
);
