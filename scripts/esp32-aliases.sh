#!/usr/bin/env bash

alias eb='pio run'
alias eu='pio run -t upload'
alias em='pio device monitor'
alias ecfg='pio run -t menuconfig'
alias menu='print_esp32_aliases'

print_esp32_aliases() {
    cat <<'EOF'

========================================
ESP32 aliases
========================================
  eb     pio run
  eu     pio run -t upload
  em     pio device monitor
  ecfg   pio run -t menuconfig
  menu   show this menu
========================================

EOF
}
