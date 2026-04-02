#!/usr/bin/env bash

print_esp32_help() {
    cat <<'EOF'

========================================
ESP32 quick reference
========================================
  eb
      build firmware

  eu
      flash device

  em
      open serial monitor

  ecfg
      open ESP-IDF menuconfig

  pwd
      confirm you are in device/esp32
========================================

EOF
}
