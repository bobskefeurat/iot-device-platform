#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

alias espusb="$SCRIPT_DIR/reconnect-esp32-usb.sh"
alias tkill='tmux kill-session -t iot'
alias treset="$SCRIPT_DIR/reset-session.sh"
alias menu='print_terminal_help'

print_terminal_aliases() {
    :
}
