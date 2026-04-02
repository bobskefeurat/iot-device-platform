#!/usr/bin/env bash

set -euo pipefail

SESSION_NAME="iot"
DETACH_ONLY=0
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BACKEND_WINDOW="backend"
GIT_WINDOW="git"
ESP32_WINDOW="esp32"
TERMINAL_WINDOW="terminal"
VENV_ACTIVATE="$REPO_ROOT/.venv/bin/activate"
GIT_ALIASES="$REPO_ROOT/scripts/git-aliases.sh"
TERMINAL_ALIASES="$REPO_ROOT/scripts/terminal-aliases.sh"
TERMINAL_HELP="$REPO_ROOT/scripts/terminal-help.sh"
ESP32_ALIASES="$REPO_ROOT/scripts/esp32-aliases.sh"
BACKEND_COMMAND="uvicorn backend.main:app --host 0.0.0.0 --port 8000 --reload"

while [ $# -gt 0 ]; do
    case "$1" in
        --detach-only)
            DETACH_ONLY=1
            ;;
        --session-name)
            SESSION_NAME="${2:-}"
            shift
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 1
            ;;
    esac
    shift
done

attach_or_switch() {
    if [ -n "${TMUX:-}" ]; then
        tmux switch-client -t "$SESSION_NAME"
    else
        tmux attach-session -t "$SESSION_NAME"
    fi
}

send_tmux_command() {
    local target="$1"
    local command="$2"
    tmux send-keys -t "$target" "$command" C-m
}

if tmux has-session -t "$SESSION_NAME" 2>/dev/null; then
    if [ "$DETACH_ONLY" -eq 0 ]; then
        attach_or_switch
    fi
    exit 0
fi

tmux new-session -d -s "$SESSION_NAME" -n "$TERMINAL_WINDOW" -c "$REPO_ROOT"

if [ -f "$VENV_ACTIVATE" ]; then
    send_tmux_command "$SESSION_NAME:$TERMINAL_WINDOW" "source \"$VENV_ACTIVATE\""
fi
send_tmux_command "$SESSION_NAME:$TERMINAL_WINDOW" "source \"$TERMINAL_ALIASES\""
send_tmux_command "$SESSION_NAME:$TERMINAL_WINDOW" "source \"$TERMINAL_HELP\""
send_tmux_command "$SESSION_NAME:$TERMINAL_WINDOW" "print_terminal_help"

tmux new-window -t "$SESSION_NAME" -n "$GIT_WINDOW" -c "$REPO_ROOT"

if [ -f "$VENV_ACTIVATE" ]; then
    send_tmux_command "$SESSION_NAME:$GIT_WINDOW" "source \"$VENV_ACTIVATE\""
fi
send_tmux_command "$SESSION_NAME:$GIT_WINDOW" "source \"$GIT_ALIASES\""
send_tmux_command "$SESSION_NAME:$GIT_WINDOW" "print_git_aliases"

tmux new-window -t "$SESSION_NAME" -n "$BACKEND_WINDOW" -c "$REPO_ROOT"

if [ -f "$VENV_ACTIVATE" ]; then
    send_tmux_command "$SESSION_NAME:$BACKEND_WINDOW" "source \"$VENV_ACTIVATE\""
fi
send_tmux_command "$SESSION_NAME:$BACKEND_WINDOW" "$BACKEND_COMMAND"

tmux new-window -t "$SESSION_NAME" -n "$ESP32_WINDOW" -c "$REPO_ROOT/device/esp32"
send_tmux_command "$SESSION_NAME:$ESP32_WINDOW" "source \"$ESP32_ALIASES\""
send_tmux_command "$SESSION_NAME:$ESP32_WINDOW" "print_esp32_aliases"
tmux select-window -t "$SESSION_NAME:$TERMINAL_WINDOW"

if [ "$DETACH_ONLY" -eq 0 ]; then
    attach_or_switch
fi
