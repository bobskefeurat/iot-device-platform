#include "device/device_setup.h"

#include "device/device_components.h"
#include "device/device_identity.h"

static const component_t default_components[] = {
    {
        .component_local_id = "moisture_sensor_1",
        .model_name = "Capacitive Soil Moisture Sensor V2.0.0",
        .component_type = "sensor"
    },
};

static const size_t default_component_count = sizeof(default_components) / sizeof(default_components[0]);

void setup_device_info(void) {
    build_device_id();
    set_device_name("ESP32 Plantstation");

    for (size_t i = 0; i < default_component_count; i++) {
        add_component(default_components[i]);
    }
}
