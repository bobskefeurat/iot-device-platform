#include "app/runtime.h"

#include "features/moisture/moisture_runtime.h"
#include "ui/device_console.h"

void device_runtime_init(void) {
    moisture_runtime_init();
    device_console_init();
}

void device_runtime_start(void) {
    moisture_runtime_start();
    device_console_start();
}
