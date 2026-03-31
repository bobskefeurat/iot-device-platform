#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"

void serial_console_init(void);
bool serial_console_read_command(char *buffer, size_t buffer_size, TickType_t timeout_ticks);
