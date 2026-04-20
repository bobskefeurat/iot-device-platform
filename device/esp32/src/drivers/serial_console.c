#include "drivers/serial_console.h"

#include <stdio.h>
#include <stddef.h>

void serial_console_init(void) {
    // Use ESP-IDF's default console on UART0 to avoid conflicting with log output.
}

bool serial_console_read_command(char *buffer, size_t buffer_size, TickType_t timeout_ticks) {
    static size_t command_length = 0;
    int received_byte = 0;

    if (buffer == NULL || buffer_size == 0) {
        return false;
    }

    (void)timeout_ticks;
    received_byte = getchar();

    if (received_byte == EOF) {
        return false;
    }

    if (received_byte == '\r' || received_byte == '\n') {
        buffer[command_length] = '\0';
        command_length = 0;
        return true;
    }

    if (received_byte == '\b' || received_byte == 127) {
        if (command_length > 0) {
            command_length--;
        }
        return false;
    }

    if (command_length < buffer_size - 1) {
        buffer[command_length++] = (char)received_byte;
    }

    return false;
}
