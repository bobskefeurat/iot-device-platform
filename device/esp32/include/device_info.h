#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    const char *component_local_id;
    const char *model_name;
    const char *component_type;
} component_t;

void setup_device_info();

const char* get_device_id();

void set_device_name(const char *name);

const char* get_device_name();

bool add_component(component_t new_component);

const component_t *get_device_components();

size_t get_components_count();
