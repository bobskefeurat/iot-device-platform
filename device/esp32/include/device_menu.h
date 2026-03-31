#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool calibration_in_progress;
    uint16_t latest_mean_adc;
    uint8_t latest_moisture_percent;
} device_menu_status_t;

typedef struct {
    void (*start_live_reading)(void);
    void (*stop_live_reading)(void);
    void (*refresh_measurement)(void);
    void (*start_calibration)(void);
    bool (*clear_calibration)(void);
    bool (*load_calibration)(uint16_t *dry_adc, uint16_t *wet_adc);
    void (*get_status)(device_menu_status_t *status);
} device_menu_actions_t;

void device_menu_init(void);
void device_menu_print_main(void);
void device_menu_print_live_controls(void);
void device_menu_handle_command(const char *command,
                                const device_menu_status_t *status,
                                const device_menu_actions_t *actions);
bool device_menu_is_live_reading_active(void);
