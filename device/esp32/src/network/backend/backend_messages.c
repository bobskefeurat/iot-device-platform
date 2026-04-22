#include "network/backend/backend_messages.h"

#include <stdio.h>
#include <string.h>

//---------------------OUTBOUND MESSAGES---------------------

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

//---------------------INBOUND MESSAGES---------------------

bool extract_backend_field_value(
    const char *response_buffer,
    const char *field_name,
    char *field_value,
    size_t field_value_size
) {

    if(
        (response_buffer == NULL)||
        (field_name == NULL)||
        (field_value == NULL)||
        (field_value_size == 0)
    ) {
        return false;
    }

    field_value[0] = '\0';

    const char *field_position = strstr(response_buffer, field_name);
    if (field_position == NULL) {
        return false;
    }

    const char *value_start = strchr(field_position, ':');
    if (value_start == NULL) {
        return false;
    }

    while (
        (*value_start == ':')||
        (*value_start == ' ')||
        (*value_start == '\"')
    ) {
        value_start++;
    }
    
    const char *value_end = value_start;
    while (
        (*value_end != '\0')&&
        (*value_end != '\"')&&
        (*value_end != '}')
    ) {
        value_end++;
    }

    size_t value_length = (size_t)(value_end - value_start);
    if (value_length >= field_value_size) {
        return false;
    }

    memcpy(field_value, value_start, value_length);
    field_value[value_length] = '\0';

    return true;
}