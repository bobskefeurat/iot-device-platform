#pragma once

#include <stdbool.h>

void wifi_manager_init(void);
bool wifi_manager_is_connected(void);
void wifi_manager_print_status(void);
