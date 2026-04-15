#include "device_info.h"

#include <stdio.h>
#include <stdint.h>

#include "esp_check.h"
#include "esp_mac.h"

#define MAX_COMPONENTS 8

static char id[18];
static char name[28];

static size_t component_count = 0;
static component_t device_components[MAX_COMPONENTS];

static component_t default_components[] = {
        {
            .component_local_id = "moisture_sensor_1",
            .model_name = "Capacitive Soil Moisture Sensor V2.0.0",
            .component_type = "sensor"
        },
    };
const static size_t default_component_count = sizeof(default_components) / sizeof(default_components[0]);

void build_device_id() {

    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));

    snprintf(id, sizeof(id), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void setup_device_info() {

    build_device_id();
    set_device_name("ESP32 Plantstation");

    for (size_t i = 0; i < default_component_count; i++) {
        add_component(default_components[i]);
    }    
}

void set_device_name(const char *new_name) {
    snprintf(name, sizeof(name), "%s", new_name);
}

bool add_component(component_t new_component) {

    if (component_count >= MAX_COMPONENTS) {
        return false;
    }

    device_components[component_count] = new_component;
    component_count++;

    return true;
}

const char* get_device_id() {return id;}

const char* get_device_name() {return name;}

const component_t *get_device_components() {return device_components;}

size_t get_components_count() {return component_count;}
