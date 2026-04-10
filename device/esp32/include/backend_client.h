#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    const char *component_local_id;
    const char *model_name;
    const char *component_type;
} backend_component_t;


bool register_device(
    const char *id, 
    const char *name,
    const backend_component_t *components,
    size_t component_count);

bool send_heartbeat(const char *id);

