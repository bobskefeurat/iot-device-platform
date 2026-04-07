#pragma once

#include <stdbool.h>

bool register_device(const char *id, const char *name);

bool send_heartbeat(const char *id);

