#include "serial_console.h"

#include <stddef.h>
#include <stdint.h>
#include "driver/uart.h"
#include "esp_err.h"

#define SERIAL_UART_PORT UART_NUM_0
#define SERIAL_RX_BUFFER_SIZE 256

void serial_console_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(SERIAL_UART_PORT, SERIAL_RX_BUFFER_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(SERIAL_UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(SERIAL_UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_flush_input(SERIAL_UART_PORT));
}

bool serial_console_read_command(char *buffer, size_t buffer_size, TickType_t timeout_ticks) {
    static size_t command_length = 0;
    uint8_t received_byte = 0;

    if (buffer == NULL || buffer_size == 0) {
        return false;
    }

    int bytes_read = uart_read_bytes(SERIAL_UART_PORT, &received_byte, 1, timeout_ticks);
    if (bytes_read <= 0) {
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
