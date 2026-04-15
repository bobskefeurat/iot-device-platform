#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "device_info.h"

bool register_device(
    const char *id, 
    const char *name,
    const component_t *device_components,
    size_t component_count);

bool send_heartbeat(const char *id);

bool send_measurement(
    const char *id,
    const char *component_local_id,
    int mean_adc,
    int moisture_percent
);

