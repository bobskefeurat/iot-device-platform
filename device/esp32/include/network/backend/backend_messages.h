#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "device/device_components.h"

bool build_register_device_payload(
    char *payload,
    size_t payload_size,
    const char *id,
    const char *name,
    const component_t *device_components,
    size_t component_count
);

bool build_measurement_payload(
    char *payload,
    size_t payload_size,
    const char *component_local_id,
    int mean_adc,
    int moisture_percent
);

bool build_config_sync_payload(
    char *payload,
    size_t payload_size,
    const char *applied_config_id
);

bool extract_backend_field_value(
    const char *response_buffer,
    const char *field_name,
    char *field_value,
    size_t field_value_size
);
