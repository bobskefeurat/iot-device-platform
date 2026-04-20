#include "ui/device_menu.h"

#include <string.h>
#include "esp_log.h"

static const char *TAG = "PlantStation";

typedef enum {
    MENU_STATE_MAIN = 0,
    MENU_STATE_LIVE_READING
} menu_state_t;

static menu_state_t current_menu_state = MENU_STATE_MAIN;

void device_menu_init(void) {
    current_menu_state = MENU_STATE_MAIN;
}

void device_menu_print_main(void) {
    ESP_LOGI(TAG, "Main menu");
    ESP_LOGI(TAG, "1. Start live readings");
    ESP_LOGI(TAG, "2. Show current status");
    ESP_LOGI(TAG, "3. Show calibration values");
    ESP_LOGI(TAG, "4. Start calibration");
    ESP_LOGI(TAG, "5. Clear calibration");
    ESP_LOGI(TAG, "6. Show WiFi status");
    ESP_LOGI(TAG, "h. Show this menu again");
}

void device_menu_print_live_controls(void) {
    ESP_LOGI(TAG, "Live readings started. Type q and press Enter to return to the menu.");
}

bool device_menu_is_live_reading_active(void) {
    return current_menu_state == MENU_STATE_LIVE_READING;
}

static void show_device_status(const device_menu_status_t *status, const device_menu_actions_t *actions) {
    uint16_t dry_adc = 0;
    uint16_t wet_adc = 0;
    bool has_calibration = actions->load_calibration(&dry_adc, &wet_adc);

    ESP_LOGI(TAG, "Status: adc=%u moisture=%u%% calibration=%s",
             status->latest_mean_adc,
             status->latest_moisture_percent,
             has_calibration ? "loaded" : "missing");

    if (has_calibration) {
        ESP_LOGI(TAG, "Calibration values: dry=%u wet=%u", dry_adc, wet_adc);
    }

    if (status->calibration_in_progress) {
        ESP_LOGI(TAG, "Calibration is currently running");
    }
}

static void show_calibration_values(const device_menu_actions_t *actions) {
    uint16_t dry_adc = 0;
    uint16_t wet_adc = 0;

    if (actions->load_calibration(&dry_adc, &wet_adc)) {
        ESP_LOGI(TAG, "Stored calibration: dry=%u wet=%u", dry_adc, wet_adc);
    } else {
        ESP_LOGW(TAG, "No calibration values stored");
    }
}

static void handle_main_menu_input(const char *command,
                                   const device_menu_status_t *status,
                                   const device_menu_actions_t *actions) {
    if (strcmp(command, "1") == 0) {
        if (status->calibration_in_progress) {
            ESP_LOGW(TAG, "Cannot start live readings while calibration is running");
            return;
        }

        current_menu_state = MENU_STATE_LIVE_READING;
        actions->start_live_reading();
        device_menu_print_live_controls();
    } else if (strcmp(command, "2") == 0) {
        device_menu_status_t current_status = *status;

        if (!status->calibration_in_progress) {
            actions->refresh_measurement();
            actions->get_status(&current_status);
        }

        show_device_status(&current_status, actions);
        device_menu_print_main();
    } else if (strcmp(command, "3") == 0) {
        show_calibration_values(actions);
        device_menu_print_main();
    } else if (strcmp(command, "4") == 0) {
        actions->start_calibration();
    } else if (strcmp(command, "5") == 0) {
        if (!actions->clear_calibration()) {
            ESP_LOGW(TAG, "Calibration could not be cleared");
        }

        device_menu_print_main();
    } else if (strcmp(command, "6") == 0) {
        actions->show_wifi_status();
        device_menu_print_main();
    } else if (strcmp(command, "h") == 0 || strcmp(command, "menu") == 0) {
        device_menu_print_main();
    } else if (command[0] != '\0') {
        ESP_LOGW(TAG, "Unknown menu selection: %s", command);
        device_menu_print_main();
    }
}

static void handle_live_menu_input(const char *command, const device_menu_actions_t *actions) {
    if (strcmp(command, "q") == 0) {
        current_menu_state = MENU_STATE_MAIN;
        actions->stop_live_reading();
        ESP_LOGI(TAG, "Stopped live readings");
        device_menu_print_main();
    } else if (strcmp(command, "h") == 0 || strcmp(command, "menu") == 0) {
        ESP_LOGI(TAG, "Live reading controls: q = stop readings and return to menu");
    } else if (command[0] != '\0') {
        ESP_LOGW(TAG, "Unknown live reading input: %s", command);
        ESP_LOGI(TAG, "Type q and press Enter to return to the menu");
    }
}

void device_menu_handle_command(const char *command,
                                const device_menu_status_t *status,
                                const device_menu_actions_t *actions) {
    if (current_menu_state == MENU_STATE_LIVE_READING) {
        handle_live_menu_input(command, actions);
    } else {
        handle_main_menu_input(command, status, actions);
    }
}
