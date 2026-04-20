#include "network/backend_payloads.h"

#include <stdio.h>

bool build_register_device_payload(
    char *payload,
    size_t payload_size,
    const char *id,
    const char *name,
    const component_t *device_components,
    size_t component_count
) {
    char components_json[512];
    size_t offset = 0;
    size_t remaining = sizeof(components_json) - offset;
    int written = snprintf(components_json + offset, remaining, "[");

    if (written < 0 || (size_t)written >= remaining) {
        return false;
    }

    offset += (size_t)written;

    for (size_t i = 0; i < component_count; i++) {
        if (i > 0) {
            remaining = sizeof(components_json) - offset;
            written = snprintf(components_json + offset, remaining, ",");

            if (written < 0 || (size_t)written >= remaining) {
                return false;
            }

            offset += (size_t)written;
        }

        remaining = sizeof(components_json) - offset;
        written = snprintf(
            components_json + offset,
            remaining,
            "{\"component_local_id\":\"%s\",\"model_name\":\"%s\",\"component_type\":\"%s\"}",
            device_components[i].component_local_id,
            device_components[i].model_name,
            device_components[i].component_type
        );

        if (written < 0 || (size_t)written >= remaining) {
            return false;
        }

        offset += (size_t)written;
    }

    remaining = sizeof(components_json) - offset;
    written = snprintf(components_json + offset, remaining, "]");

    if (written < 0 || (size_t)written >= remaining) {
        return false;
    }

    written = snprintf(
        payload,
        payload_size,
        "{\"id\":\"%s\",\"name\":\"%s\",\"device_components\":%s}",
        id,
        name,
        components_json
    );

    return written >= 0 && (size_t)written < payload_size;
}

bool build_measurement_payload(
    char *payload,
    size_t payload_size,
    const char *component_local_id,
    int mean_adc,
    int moisture_percent
) {
    int written = snprintf(
        payload,
        payload_size,
        "{\"component_local_id\":\"%s\",\"mean_adc\":%d,\"moisture_percent\":%d}",
        component_local_id,
        mean_adc,
        moisture_percent
    );

    return written >= 0 && (size_t)written < payload_size;
}
