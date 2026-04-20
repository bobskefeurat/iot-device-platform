#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *component_local_id;
    const char *model_name;
    const char *component_type;
} component_t;

bool add_component(component_t new_component);
const component_t *get_device_components(void);
size_t get_components_count(void);
