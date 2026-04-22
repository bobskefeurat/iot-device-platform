#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app/platform.h"
#include "app/startup.h"

void app_main(void) {
    system_init();
    device_startup_run();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
