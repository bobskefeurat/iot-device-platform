#include <stdio.h>

static char applied_config_id[28];
static int heartbeat_interval = 30;

const char *get_applied_config_id() {
    return applied_config_id;
}

void set_applied_config_id(const char *new_applied_config_id) {
    snprintf(
        applied_config_id, 
        sizeof(applied_config_id), 
        "%s", 
        new_applied_config_id
    );
}

int get_heartbeat_interval() {
    return heartbeat_interval;
}

void set_heartbeat_interval(int new_heartbeat_interval) {
    heartbeat_interval = new_heartbeat_interval;
}

