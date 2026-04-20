#include "device/device_identity.h"

#include <stdio.h>
#include <stdint.h>

#include "esp_check.h"
#include "esp_mac.h"

static char id[18];
static char name[28];

void build_device_id(void) {
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));

    snprintf(id, sizeof(id), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void set_device_name(const char *new_name) {
    snprintf(name, sizeof(name), "%s", new_name);
}

const char *get_device_id(void) {
    return id;
}

const char *get_device_name(void) {
    return name;
}
