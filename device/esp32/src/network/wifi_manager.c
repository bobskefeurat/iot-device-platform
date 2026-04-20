#include "network/wifi_manager.h"
#include "config/local_config.h"

#include <stdio.h>
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/inet.h"

static const char *TAG = "WiFi";
static bool wifi_connected = false;
static char wifi_ip_address[16] = "not assigned";
static int32_t last_disconnect_reason = -1;

static void wifi_manager_event_handler(void *arg,
                                       esp_event_base_t event_base,
                                       int32_t event_id,
                                       void *event_data) {
    (void)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi started, connecting to access point");
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
        wifi_connected = false;
        last_disconnect_reason = event->reason;
        snprintf(wifi_ip_address, sizeof(wifi_ip_address), "%s", "not assigned");
        ESP_LOGW(TAG, "WiFi disconnected, reason=%ld, retrying", (long)last_disconnect_reason);
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        wifi_connected = true;
        last_disconnect_reason = 0;
        snprintf(wifi_ip_address, sizeof(wifi_ip_address), IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "WiFi connected, IP address: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void wifi_manager_init(void) {
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_manager_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_manager_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi station mode started");
}

bool wifi_manager_is_connected(void) {
    return wifi_connected;
}

void wifi_manager_print_status(void) {
    ESP_LOGI(TAG, "WiFi status: connected=%s ip=%s last_disconnect_reason=%ld",
             wifi_connected ? "yes" : "no",
             wifi_ip_address,
             (long)last_disconnect_reason);
}
