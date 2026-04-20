#include "ui/device_menu_actions_adapter.h"

#include "features/moisture/moisture_calibration_flow.h"
#include "features/moisture/moisture_calibration_store.h"
#include "features/moisture/moisture_runtime.h"
#include "network/wifi_manager.h"

void device_menu_actions_adapter_build(device_menu_actions_t *actions) {
    if (actions == NULL) {
        return;
    }

    *actions = (device_menu_actions_t){
        .start_live_reading = moisture_runtime_start_live_reading,
        .stop_live_reading = moisture_runtime_stop_live_reading,
        .refresh_measurement = moisture_runtime_refresh_measurement,
        .start_calibration = moisture_runtime_start_calibration,
        .clear_calibration = moisture_calibration_clear,
        .load_calibration = moisture_calibration_load,
        .get_status = moisture_runtime_get_status,
        .show_wifi_status = wifi_manager_print_status,
    };
}
