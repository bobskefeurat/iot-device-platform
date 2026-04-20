#include "ui/device_console.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "drivers/serial_console.h"
#include "ui/device_menu_actions_adapter.h"
#include "ui/device_menu.h"

static const char *TAG = "PlantStation";

static void serial_command_task(void *arg) {
    (void)arg;

    char command_buffer[64];
    device_menu_actions_t menu_actions;
    device_menu_actions_adapter_build(&menu_actions);

    ESP_LOGI(TAG, "Serial menu ready");
    device_menu_print_main();

    while (1) {
        if (!serial_console_read_command(command_buffer, sizeof(command_buffer), pdMS_TO_TICKS(100))) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        device_menu_status_t menu_status;
        menu_actions.get_status(&menu_status);
        device_menu_handle_command(command_buffer, &menu_status, &menu_actions);
    }
}

void device_console_init(void) {
    device_menu_init();
    serial_console_init();
}

void device_console_start(void) {
    xTaskCreate(serial_command_task, "serial_command_task", 4096, NULL, 5, NULL);
}
