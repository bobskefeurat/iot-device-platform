#include "device/device_components.h"

#define MAX_COMPONENTS 8

static size_t component_count = 0;
static component_t device_components[MAX_COMPONENTS];

bool add_component(component_t new_component) {
    if (component_count >= MAX_COMPONENTS) {
        return false;
    }

    device_components[component_count] = new_component;
    component_count++;
    return true;
}

const component_t *get_device_components(void) {
    return device_components;
}

size_t get_components_count(void) {
    return component_count;
}
