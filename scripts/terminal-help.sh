#!/usr/bin/env bash

print_terminal_help() {
    cat <<'EOF'

========================================
Terminal quick reference
========================================
Tmux
  Ctrl+b w    list windows
  Ctrl+b n    next window
  Ctrl+b p    previous window
  Ctrl+b ,    rename current window
  Ctrl+b d    detach session
  Ctrl+b %    split vertically
  Ctrl+b "    split horizontally
  Ctrl+b z    zoom/unzoom current pane
  Ctrl+b x    close current pane
  Ctrl+b [    enter copy mode
  Space       start selection
  Enter       copy selection
  Ctrl+b ]    paste copied text

Session
  tmux ls     list sessions
  tmux attach -t iot
               reattach to session
  tkill       kill tmux session "iot"
  treset      recreate and reattach session "iot"

Helpers
  espusb      reconnect ESP32 USB from WSL
  menu        show this menu
========================================

EOF
}
